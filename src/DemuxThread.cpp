#include "Demux.h"
#include "VideoThread.h"
#include "AudioThread.h"
#include "Util.h"
#include "DemuxThread.h"
#include "ToolWidget.h"
#include "ui_ToolWidget.h"
#include "RecordListItem.h"
namespace HL
{
	bool DemuxThread::open(const char *url, VideoCall *widget)
	{
		if (url == nullptr or widget == nullptr or url[0] == '\0')
			return false;
		std::lock_guard lg(mtx);
		if (is_open)
			close();
		if (demux == nullptr)
			demux = new Demux();
		if (video_thread == nullptr)
			video_thread = new VideoThread();
		if (audio_thread == nullptr)
			audio_thread = new AudioThread();
		int ret = demux->open(url);
		if (ret == false)
		{
			print_error_log("demux open 失败!");
			return false;
		}
		widget->init(demux->width(), demux->height());
		audio_thread->setParameters(demux->copyAudioCodecParameters());
		video_thread->setParameters(demux->copyVideoCodecParameters());
		video_thread->setWindow(widget);
		totalMs = demux->totalMs();
		is_open = true;
		is_exit = false;
		setStatus(PlayStatus::PLAY);
		setVolume(Global::volume);
		return true;
	}

	void DemuxThread::run()
	{
		qDebug() << "dt thread" << currentThreadId();
		if (Global::isVideo and video_thread)
			video_thread->start();
		if (audio_thread)
			audio_thread->start();

		while (!is_exit)
		{
			mtx.lock();
			if (isPause())
			{
				mtx.unlock();
				msleep(5);
				continue;
			}
			if (demux == nullptr)
			{
				mtx.unlock();
				msleep(5);
				continue;
			}

			// 音视频同步
			if (Global::isVideo and video_thread and audio_thread)
			{
				pts = audio_thread->pts;
				video_thread->synpts = audio_thread->pts;
			}
			AVPacket *pkt = demux->read_packet();
			if (pkt == nullptr)
			{
				// 播放完毕
				emit finished();
				mtx.unlock();
				msleep(5);
				continue;
			}
			if (demux->isAudio(pkt))
			{
				if (audio_thread)
				{
					audio_thread->push(pkt); // 缓存满了会阻塞导致影响同步
				}
			}
			else if (Global::isVideo and demux->isVideo(pkt))
			{
				if (video_thread)
					video_thread->push(pkt);
			}
			mtx.unlock();
			// !!!此处需要微调参数,不sleep或者sleep太长都会导致卡顿
			// 不sleep会导致同步来不及做，sleep太长播放的时间就短了
			msleep(1);
		}
	}
	DemuxThread::~DemuxThread()
	{
		close();
	}
	void DemuxThread::close()
	{
		is_exit = true;
		wait();
		setStatus(PlayStatus::STOP);
		is_open = false;
		if (video_thread)
			video_thread->close(), delete video_thread, video_thread = nullptr;
		if (audio_thread)
			audio_thread->close(), delete audio_thread, audio_thread = nullptr;
		if (demux)
			delete demux, demux = nullptr;
	}
	void DemuxThread::setPts(int64_t pts_)
	{
		pts = pts_;
	}
	void DemuxThread::setTotalMs(int64_t ms)
	{
		totalMs = ms;
	}
	int64_t DemuxThread::getPts() const
	{
		return pts;
	}
	int64_t DemuxThread::getTotalMs() const
	{
		return totalMs;
	}
	void DemuxThread::setStatus(PlayStatus sta)
	{
		status = sta;
		if (video_thread)
			video_thread->setStatus(sta);
		if (audio_thread)
			audio_thread->setStatus(sta);
	}
	bool DemuxThread::isPause() const
	{
		return status == PlayStatus::PAUSE;
	}

	void DemuxThread::Seek(double pos)
	{
		if (isStop())
			return;
		mtx.lock();
		auto oldStatus = status;
		mtx.unlock();
		setStatus(PlayStatus::PAUSE);
		clear();
		mtx.lock();
		if (demux)
			demux->seek(pos);
		int64_t seekPts = pos * demux->totalMs();

		// 跳转到精确帧, 丢弃掉多余的帧, 有点问题最好不要使用
		while (!is_exit and true)
		{
			AVPacket *pkt = demux->read_packet();
			if (!pkt)
				break;
			if (demux->isAudio(pkt))
			{
				av_packet_free(&pkt);
			}
			else if (Global::isVideo and demux->isVideo(pkt))
			{
				// send packet一次几十毫秒，跳过非关键帧提升效率
				//			if(!(pkt->flags & AV_PKT_FLAG_KEY) and pkt->pts + static_cast<int>(Constant::PTSERRORRANGE)*50 < seekPts ) {
				//				continue;
				//			}

				//			if(pkt->pts + static_cast<int>(Constant::PTSERRORRANGE)*10 < seekPts)
				//				continue;

				bool ret = video_thread->getDecode()->send(pkt);
				if (!ret)
					break;
				AVFrame *frame = video_thread->getDecode()->recv();
				if (!frame)
					continue;
				//			std::cerr << seekPts << ' ' << frame->pts << std::endl;
				// 到达位置
				if (frame->pts >= seekPts)
				{
					this->pts = frame->pts;
					video_thread->getVideoCall()->rePaint(frame);
					break;
				}
				else
				{
					av_frame_free(&frame);
				}
			}
		}
		mtx.unlock();
		setStatus(oldStatus);
	}
	void DemuxThread::clear()
	{
		mtx.lock();
		if (demux)
			demux->clear();
		if (audio_thread)
			audio_thread->clear();
		if (video_thread)
			video_thread->clear();
		mtx.unlock();
	}
	bool DemuxThread::isPlay() const
	{
		return status == PlayStatus::PLAY;
	}
	bool DemuxThread::isStop() const
	{
		return status == PlayStatus::STOP;
	}
	bool DemuxThread::isOpen() const
	{
		return is_open;
	}
	AudioThread *DemuxThread::getAudioThread() const
	{
		return audio_thread;
	}
	void DemuxThread::setVolume(int value)
	{
		Global::volume = volume = value;
		Global::tw->Ui()->twVolumeSlider->setValue(value);
		if (isOpen())
			audio_thread->setVolume(value);
	}
	AVRational DemuxThread::audioTimeBase() const
	{
		return demux->audioTimeBase();
	}
	AVRational DemuxThread::videoTimeBase() const
	{
		return demux->videoTimeBase();
	}
	VideoThread *DemuxThread::getVideoThread() const
	{
		return video_thread;
	}
	int DemuxThread::getVolume() const
	{
		return Global::volume;
	}
} // HL