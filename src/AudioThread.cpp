#include "AudioThread.h"
#include "Util.h"
#include "Slider.h"
#include "DemuxThread.h"
#include "PlayWidget.h"
#include <Qthread>
#include <QDebug>
#include "RecordListItem.h"
#include "ToolWidget.h"
#include "ui_ToolWidget.h"

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavfilter/avfilter.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavutil/opt.h>
}

namespace HL
{
	using namespace std::chrono_literals;

	bool AudioThread::open()
	{
		if (para == nullptr)
		{
			print_error_log("AudioThread->para 没有设置");
			return false;
		}
		std::lock_guard lg(mtx);
		pts = 0;
		if (decode == nullptr)
			decode = new Decode();
		if (resample == nullptr)
			resample = new Resample();
		if (audio_play == nullptr)
			audio_play = new QtAudioPlay();
		AVCodecParameters *tmp = avcodec_parameters_alloc();
		avcodec_parameters_copy(tmp, para);

		int ret = resample->open(tmp);
		if (!ret)
		{
			print_error_log("resample open 失败");
			avcodec_parameters_free(&para);
			return false;
		}

		QAudioFormat afmt;
		afmt.setSampleRate(resample->sample_rate());
		afmt.setChannelCount(resample->ch_count());
		afmt.setSampleFormat(Util::AVSampleFormat_to_QSampleFormat(resample->format()));
		audio_play->setFormat(afmt);

		ret = audio_play->open();
		if (!ret)
		{
			print_error_log("audio_play open 失败");
			avcodec_parameters_free(&para);
			return false;
		}

		ret = decode->open(Util::copyParameters(para));
		if (!ret)
		{
			print_error_log("audio decode open 失败");
			avcodec_parameters_free(&para);
			return false;
		}

		is_exit = false;
		setStatus(PlayStatus::PLAY);
		return true;
	}

	void AudioThread::run()
	{
		qDebug() << "at thread" << currentThreadId();
		open();
		auto item = RecordListItem::getItemByName(Global::tw->Ui()->twFileName->fullText);
		double seekPos = (double)item->pts / Global::dt->getTotalMs();
		Global::dt->Seek(seekPos);
		audio_play->setVolume(Global::volume / 100.0);
		if (Global::playBackSpeed == 0)
			setSpeed(1.0);
		else
			setSpeed(Global::playBackSpeed);
		//	uint8_t *pcm = new uint8_t[AUDIO_BUFFER_SIZE];
		while (!is_exit)
		{
			mtx.lock();
			if (isPause())
			{
				mtx.unlock();
				msleep(5);
				continue;
			}

			if (packs.empty() or !decode->isOpen() or !resample->isOpen() or !audio_play->isOpen())
			{
				mtx.unlock();
				msleep(1);
				continue;
			}
			AVPacket *pkt = packs.front();
			packs.pop();
			int ret = decode->send(pkt);
			if (!ret)
			{
				mtx.unlock();
				msleep(1);
				continue;
			}

			while (!is_exit)
			{
				AVFrame *frame = decode->recv();
				if (frame == nullptr)
				{
					//				mtx.unlock();
					break;
				}
				// 减去缓冲中未播放的时间
				pts = decode->pts - audio_play->getNoPlayMs();
				RecordListItem::setSubPts(Global::tw->Ui()->twFileName->fullText, pts);
				//			/*
				// !!!!! 新增开始
				// push the audio data from decoded frame into the filtergraph
				if (av_buffersrc_add_frame_flags(buffersrc_ctx, frame, AV_BUFFERSRC_FLAG_KEEP_REF) < 0)
				{
					av_frame_free(&frame);
					print_error_log("Error while feeding the audio filtergraph\n");
					break;
				}
				av_frame_free(&frame);
				// pull filtered audio from the filtergraph
				while (!is_exit and !isPause())
				{
					AVFrame *filt_frame = av_frame_alloc();
					ret = av_buffersink_get_frame(buffersink_ctx, filt_frame);
					if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
					{
						av_frame_free(&filt_frame);
						msleep(5);
						break;
					}
					if (ret < 0)
					{
						mtx.unlock();
						//					delete pcm;
						print_error_log("av_buffersink_get_frame error");
						av_frame_free(&filt_frame);
						return;
					}
					int size = filt_frame->ch_layout.nb_channels * filt_frame->nb_samples * av_get_bytes_per_sample(static_cast<AVSampleFormat>(filt_frame->format));
					// 如果是在播放状态
					while (!is_exit and !isPause())
					{
						if (size <= 0)
							break;
						if (audio_play->bytesFree() < size)
						{
							msleep(1);
							continue;
						}
						audio_play->write(filt_frame->data[0], size);
						break;
					}
					av_frame_free(&filt_frame);
				}
				//			!!!!! 新增结束
				//			*/
				//			 下面注释的是原本的代码
				//			int size = resample->resample(frame, pcm);
				//			qDebug() << size;
				//			// 如果是在播放状态
				//			while(!is_exit and !isPause()) {
				//				if(size <= 0) break;
				//				if(audio_play->bytesFree() < size) {
				//					msleep(1);
				//					continue;
				//				}
				//				audio_play->write(pcm, size);
				//				break;
				//			}
			}
			mtx.unlock();
		}
		//	delete pcm;
	}

	AudioThread::~AudioThread()
	{
		is_exit = true;
		if (decode)
			delete decode;
		if (audio_play)
			delete audio_play;
		if (resample)
			delete resample;
		if (para)
			avcodec_parameters_free(&para);
		wait();
	}

	void AudioThread::push(AVPacket *pkt)
	{
		if (pkt == nullptr)
			return;
		while (!is_exit)
		{
			mtx.lock();
			if (packs.size() < max_packs)
			{
				packs.push(pkt);
				mtx.unlock();
				break;
			}
			else
			{
				mtx.unlock();
				msleep(5);
				break;
			}
			qDebug() << "at packs" << packs.size();
		}
	}

	void AudioThread::setParameters(AVCodecParameters *para_)
	{
		para = para_;
	}

	void AudioThread::close()
	{
		is_exit = true;
		wait();
		clear();
		setStatus(PlayStatus::STOP);
		if (audio_play)
			delete audio_play, audio_play = nullptr;
		if (resample)
			delete resample, resample = nullptr;
		if (decode)
			delete decode, decode = nullptr;
		if (para)
			avcodec_parameters_free(&para), para = nullptr;
		pts = 0;
	}

	void AudioThread::setStatus(PlayStatus sta)
	{
		status = sta;
		if (audio_play)
			audio_play->setStatus(sta);
	}

	bool AudioThread::isPause() const
	{
		return status == PlayStatus::PAUSE;
	}

	void AudioThread::clear()
	{
		std::lock_guard lg(mtx);
		if (decode)
			decode->clear();
		if (audio_play)
			audio_play->clear();
		while (packs.size())
		{
			auto pkt = packs.front();
			packs.pop();
			av_packet_free(&pkt);
		}
	}

	QtAudioPlay *AudioThread::getAudioPlay() const
	{
		return audio_play;
	}

	void AudioThread::setVolume(int value)
	{
		Global::volume = value;
		if (audio_play)
			audio_play->setVolume(value / 100.0);
	}

	int AudioThread::init_filter(double speed)
	{
		// atempo = seepd
		static char filters_descr[] = "atempo=2.0,aformat=sample_fmts=s16:channel_layouts=stereo";
		sprintf(filters_descr, "atempo=%.1f,aformat=sample_fmts=s16:channel_layouts=stereo", speed);
		char args[512];
		const AVFilter *abuffersrc = avfilter_get_by_name("abuffer");
		const AVFilter *abuffersink = avfilter_get_by_name("abuffersink");
		AVFilterInOut *outputs = avfilter_inout_alloc();
		AVFilterInOut *inputs = avfilter_inout_alloc();
		const enum AVSampleFormat out_sample_fmts[] = {AV_SAMPLE_FMT_S16, AV_SAMPLE_FMT_NONE};
		const int64_t out_channel_layouts[] = {AV_CH_LAYOUT_STEREO, -1};
		const int out_sample_rates[] = {sampleRate(), -1};
		AVRational time_base = timeBase();
		AVCodecContext *dec_ctx = audioContext();

		int ret = 0;
		do
		{
			filter_graph = avfilter_graph_alloc();
			if (!outputs || !inputs || !filter_graph)
			{
				ret = AVERROR(ENOMEM);
				break;
			}

			/* buffer audio source: the decoded frames from the decoder will be inserted here. */
			if (!dec_ctx->channel_layout)
				dec_ctx->channel_layout = av_get_default_channel_layout(dec_ctx->channels);
			snprintf(args, sizeof(args),
					 "time_base=%d/%d:sample_rate=%d:sample_fmt=%s:channel_layout=0x%x",
					 time_base.num, time_base.den, dec_ctx->sample_rate,
					 av_get_sample_fmt_name(dec_ctx->sample_fmt), dec_ctx->channel_layout);
			ret = avfilter_graph_create_filter(&buffersrc_ctx, abuffersrc, "in",
											   args, NULL, filter_graph);
			if (ret < 0)
			{
				print_error_log("Cannot create audio buffer source\n");
				break;
			}

			/* buffer audio sink: to terminate the filter chain. */
			ret = avfilter_graph_create_filter(&buffersink_ctx, abuffersink, "out",
											   NULL, NULL, filter_graph);
			if (ret < 0)
			{
				print_error_log("Cannot create audio buffer sink\n");
				break;
			}

			ret = av_opt_set_int_list(buffersink_ctx, "sample_fmts", out_sample_fmts, -1,
									  AV_OPT_SEARCH_CHILDREN);
			if (ret < 0)
			{
				printf("Cannot set output sample format\n");
				break;
			}

			ret = av_opt_set_int_list(buffersink_ctx, "channel_layouts", out_channel_layouts, -1,
									  AV_OPT_SEARCH_CHILDREN);
			if (ret < 0)
			{
				printf("Cannot set output channel layout\n");
				break;
			}

			ret = av_opt_set_int_list(buffersink_ctx, "sample_rates", out_sample_rates, -1,
									  AV_OPT_SEARCH_CHILDREN);
			if (ret < 0)
			{
				printf("Cannot set output sample rate\n");
				break;
			}

			outputs->name = av_strdup("in");
			outputs->filter_ctx = buffersrc_ctx;
			outputs->pad_idx = 0;
			outputs->next = NULL;

			inputs->name = av_strdup("out");
			inputs->filter_ctx = buffersink_ctx;
			inputs->pad_idx = 0;
			inputs->next = NULL;

			if ((ret = avfilter_graph_parse_ptr(filter_graph, filters_descr,
												&inputs, &outputs, nullptr)) < 0)
				break;

			if ((ret = avfilter_graph_config(filter_graph, nullptr)) < 0)
				break;

		} while (0);

		avfilter_inout_free(&inputs);
		avfilter_inout_free(&outputs);

		return ret;
	}

	int AudioThread::sampleRate() const
	{
		return para->sample_rate;
	}

	AVRational AudioThread::timeBase() const
	{
		return Global::dt->audioTimeBase();
	}

	AVCodecContext *AudioThread::audioContext() const
	{
		return decode->codecContext();
	}

	void AudioThread::setSpeed(double speed)
	{
		//	if(speed == Global::playBackSpeed)
		//		return;
		if (Global::pw->Status() == PlayStatus::STOP)
		{
			Global::playBackSpeed = speed;
			return;
		}
		mtx.lock();
		Global::pw->setPlayStatus(PlayStatus::PAUSE);
		clear_filter();
		init_filter(speed);
		Global::playBackSpeed = speed;
		Global::pw->setPlayStatus(PlayStatus::PLAY);
		mtx.unlock();
	}

	void AudioThread::clear_filter()
	{
		avfilter_graph_free(&filter_graph);
	}
}