#pragma once

#include <QThread>
#include "Decode.h"
#include "QtAudioPlay.h"
#include "Resample.h"
#include <mutex>
#include "queue"
#include "Play.h"
struct AVFilterGraph;
struct AVFilterContext;
namespace HL
{

	class AudioThread : public QThread
	{
	public:
		// 当前音频播放的pts
		int64_t pts = 0;
		bool open();
		void close();
		void clear();
		void setParameters(AVCodecParameters *para_);
		void run() override;
		void push(AVPacket *pkt);
		void setStatus(PlayStatus sta);
		bool isPause() const;
		int sampleRate() const;
		AVRational timeBase() const;
		QtAudioPlay *getAudioPlay() const;
		AVCodecContext *audioContext() const;
		int init_filter(double speed);
		void clear_filter();
		AudioThread() = default;
		~AudioThread();
	public slots:
		void setVolume(int value);
		void setSpeed(double speed);

	protected:
		std::queue<AVPacket *> packs;
		int max_packs = 100;
		int volume = 100;
		Decode *decode = nullptr;
		QtAudioPlay *audio_play = nullptr;
		Resample *resample = nullptr;
		std::mutex mtx;
		AVCodecParameters *para = nullptr;
		PlayStatus status = PlayStatus::STOP;
		bool is_exit = false;

		// 滤波器用成员
		AVFilterGraph *filter_graph = nullptr;
		AVFilterContext *buffersrc_ctx = nullptr;
		AVFilterContext *buffersink_ctx = nullptr;
#define AUDIO_BUFFER_SIZE 1024 * 1024
	};

}
