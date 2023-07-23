#include <QDebug>
#include "DemuxThread.h"
#include "VideoThread.h"
#include "RecordListItem.h"
#include "Util.h"
#include "ui_ToolWidget.h"
#include "ToolWidget.h"
extern "C"
{
#include <libavfilter/avfilter.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavutil/opt.h>
}
namespace HL
{
	bool VideoThread::open()
	{
		if (para == nullptr)
		{
			print_error_log("AudioThread->para 没有设置");
			return false;
		}
		if (video_widget == nullptr)
		{
			HL::print_error_log("视频窗口未设置");
			return false;
		}
		std::lock_guard lg(mtx);
		synpts = 0;
		//  初始化显示窗口, 材质创建不能跨线程，在这里(子线程创建会出错)
		//	video_widget->init(pix_width, pix_height);
		//  打开解码器
		if (decode == nullptr)
			decode = new Decode();

		int ret = decode->open(Util::copyParameters(para));
		if (!ret)
		{
			HL::print_error_log("audio decode open 失败");
			return false;
		}
		is_exit = false;
		setStatus(PlayStatus::PLAY);
		return true;
	}

	void VideoThread::setParameters(AVCodecParameters *para_)
	{
		para = para_;
	}

	void VideoThread::run()
	{
		qDebug() << "vt thread" << currentThreadId();
		open();
		//	int idx = Global::subManager.currentSubStreamIndex;
		//	init_filter(Global::subManager.filePath, -1);
		auto item = RecordListItem::getItemByName(Global::tw->Ui()->twFileName->fullText);
		if (item and item->subIndex != -1)
		{
			init_filter(item->subPath, item->subIndex);
		}
		else
		{
			init_filter("", -1);
		}
		//	int idx = Global::subManager.curIndex;
		//	if(idx != -1)
		//		init_filter(Global::subManager.subList[idx].path, Global::subManager.subList[idx].index);
		//	else
		//		init_filter("", -1);
		while (!is_exit)
		{
			mtx.lock();
			if (isPause())
			{
				mtx.unlock();
				msleep(5);
				continue;
			}
			if (packs.empty() or !decode->isOpen())
			{
				mtx.unlock();
				msleep(1);
				continue;
			}
			if (synpts < decode->pts)
			{
				mtx.unlock();
				msleep(1);
				continue;
			}

			AVPacket *pkt = packs.front();
			packs.pop();
			int ret = decode->send(pkt);
			//		AVPacket *pkt = packs.begin()->second;
			//		packs.erase(packs.begin());
			//		int ret = decode->send(pkt);
			if (!ret)
			{
				mtx.unlock();
				msleep(1);
				continue;
			}
			// 如果是在播放状态
			while (!is_exit and !isPause())
			{
				AVFrame *frame = decode->recv();
				if (frame == nullptr)
					break;
				// 滤波处理 begin
				if (av_buffersrc_add_frame_flags(buffersrc_ctx, frame, AV_BUFFERSRC_FLAG_KEEP_REF) < 0)
				{
					av_frame_free(&frame);
					print_error_log("Error while feeding the video filtergraph\n");
					break;
				}
				av_frame_free(&frame);
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
						print_error_log("av_buffersink_get_frame error");
						av_frame_free(&filt_frame);
						return;
					}
					if (video_widget != nullptr)
					{
						video_widget->rePaint(filt_frame);
						break;
					}
				}
				// 滤波处理 end
				// 显示视频
				//			if(video_widget != nullptr)  {
				//				video_widget->rePaint(frame);
				//			}
			}
			mtx.unlock();
		}
	}
	void VideoThread::push(AVPacket *pkt)
	{
		if (pkt == nullptr)
			return;
		while (!is_exit)
		{
			mtx.lock();
			if (packs.size() < static_cast<int>(Constant::MAXPACKETNUMBER))
			{
				packs.push(pkt);
				//			packs.insert({pkt->pts, pkt});
				mtx.unlock();
				break;
			}
			else
			{
				mtx.unlock();
				msleep(5);
				break;
			}
		}
	}

	VideoThread::~VideoThread()
	{
		is_exit = true;
		wait();
		if (decode)
			delete decode;
		if (para)
			avcodec_parameters_free(&para);
	}
	void VideoThread::setWindow(VideoCall *widget)
	{
		video_widget = widget;
	}
	VideoThread::VideoThread(AVCodecParameters *para_, int w, int h)
	{
		para = para_;
	}
	void VideoThread::close()
	{
		is_exit = true;
		wait();
		clear();
		//	std::lock_guard lg(mtx);
		setStatus(PlayStatus::STOP);
		if (decode)
			delete decode, decode = nullptr;
		if (para)
			avcodec_parameters_free(&para);
		if (video_widget)
			video_widget = nullptr;
		avfilter_graph_free(&filter_graph);
		synpts = 0;
	}
	void VideoThread::setStatus(PlayStatus sta)
	{
		status = sta;
	}
	bool VideoThread::isPause() const
	{
		return status == PlayStatus::PAUSE;
	}
	void VideoThread::clear()
	{
		std::lock_guard lg(mtx);
		if (decode)
			decode->clear();
		while (packs.size())
		{
			auto pkt = packs.front();
			packs.pop();
			av_packet_free(&pkt);
		}
	}
	Decode *VideoThread::getDecode()
	{
		return decode;
	}
	VideoCall *VideoThread::getVideoCall()
	{
		return video_widget;
	}

	int VideoThread::init_filter(QString subFilePath, int subStreamindex)
	{
		//	subStreamindex = 0;
		//	subFilePath = "E:/番剧/[BeanSub&LoliHouse] Vinland Saga S2 - 13/[BeanSub&LoliHouse] Vinland Saga S2 - 13 [WebRip 1080p HEVC-10bit AAC ASSx2].SC.ass";
		//	char fontDir[] = "E\\:/番剧/[BeanSub&LoliHouse] Vinland Saga S2 - 13/[BeanSub&LoliHouse] Vinland Saga S2 - Fonts";
		static char filters_descr[512];
		QString fmt = QString("scale=%1x%2").arg(para->width).arg(para->height);
		if (!subFilePath.isEmpty() and subStreamindex != -1)
		{
			//		subFilePath.replace('/', "\\");
			subFilePath.insert(subFilePath.indexOf(":"), "\\");
			fmt += QString(",subtitles=filename='%1':original_size=%2x%3").arg(subFilePath).arg(para->width).arg(para->height);
			fmt += QString(":stream_index=%1").arg(subStreamindex);
			//		fmt += QString(":fontsdir='%1'").arg(fontDir);
		}
		qDebug() << fmt;
		sprintf(filters_descr, fmt.toStdString().c_str(), para->width, para->height);
		const AVFilter *buffersrc = avfilter_get_by_name("buffer");
		const AVFilter *buffersink = avfilter_get_by_name("buffersink");
		AVFilterInOut *output = avfilter_inout_alloc();
		AVFilterInOut *input = avfilter_inout_alloc();
		auto release = [&output, &input]
		{
			avfilter_inout_free(&output);
			avfilter_inout_free(&input);
		};

		if (!output || !input)
		{
			release();
			return false;
		}
		char args[512];
		const enum AVPixelFormat out_pix_fmts[] = {AV_PIX_FMT_YUV420P, AV_PIX_FMT_NONE};
		AVRational time_base = timeBase();
		int ret = 0;
		do
		{
			filter_graph = avfilter_graph_alloc();
			if (!output || !input || !filter_graph)
			{
				ret = AVERROR(ENOMEM);
				break;
			}
			snprintf(args, sizeof(args),
					 "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
					 para->width, para->height, para->format,
					 time_base.num, time_base.den,
					 para->sample_aspect_ratio.num, para->sample_aspect_ratio.den);

			ret = avfilter_graph_create_filter(&buffersrc_ctx, buffersrc, "in",
											   args, NULL, filter_graph);
			if (ret < 0)
			{
				print_error_log("Cannot create video buffer source\n");
				break;
			}
			ret = avfilter_graph_create_filter(&buffersink_ctx, buffersink, "out",
											   NULL, NULL, filter_graph);
			if (ret < 0)
			{
				print_error_log("Cannot create video buffer sink\n");
				break;
			}
			ret = av_opt_set_int_list(buffersink_ctx, "pix_fmts", out_pix_fmts,
									  AV_PIX_FMT_NONE, AV_OPT_SEARCH_CHILDREN);
			if (ret < 0)
			{
				print_error_log("Cannot set output pixel format\n");
				break;
			}
			output->name = av_strdup("in");
			output->filter_ctx = buffersrc_ctx;
			output->pad_idx = 0;
			output->next = NULL;

			input->name = av_strdup("out");
			input->filter_ctx = buffersink_ctx;
			input->pad_idx = 0;
			input->next = NULL;
			qDebug() << filter_graph << "----" << filters_descr;
			ret = avfilter_graph_parse_ptr(filter_graph, filters_descr, &input, &output, nullptr);
			if (ret < 0)
			{
				print_error_log("video filter graph parse error");
				break;
			}
			ret = avfilter_graph_config(filter_graph, nullptr);
			if (ret < 0)
			{
				print_error_log("video filter graph config error");
				break;
			}

		} while (0);
		release();
		return ret;
	}
	AVRational VideoThread::timeBase() const
	{
		return Global::dt->videoTimeBase();
	}
}