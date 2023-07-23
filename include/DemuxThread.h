#pragma once

#include <QThread>
#include "VideoCall.h"
#include <mutex>
#include "Reference.h"
#include "Util.h"
namespace HL
{

	class DemuxThread : public QThread
	{
		Q_OBJECT
	public:
		bool open(const char *url, VideoCall *widget);
		void run() override;
		void close();
		void clear();
		void Seek(double pos);
		void setPts(int64_t pts_);
		void setTotalMs(int64_t ms);
		void setStatus(PlayStatus sta);
		bool isPause() const;
		bool isPlay() const;
		bool isStop() const;
		bool isOpen() const;
		int64_t getPts() const;
		int64_t getTotalMs() const;
		AudioThread *getAudioThread() const;
		VideoThread *getVideoThread() const;
		AVRational audioTimeBase() const;
		AVRational videoTimeBase() const;
		int getVolume() const;
		DemuxThread() = default;
		~DemuxThread();
	public slots:
		void setVolume(int value);
	signals:
		void finished();

	private:
		int64_t pts = 0;
		int64_t totalMs = 0;
		bool is_open = false;
		Demux *demux = nullptr;
		VideoThread *video_thread = nullptr;
		AudioThread *audio_thread = nullptr;
		bool is_exit = false;
		PlayStatus status = PlayStatus::STOP;
		int volume = 100;
		std::mutex mtx;
	};

}
