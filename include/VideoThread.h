#pragma once

#include <QThread>
#include "Decode.h"
#include <mutex>
#include <set>
#include "queue"
#include "Util.h"
#include "VideoCall.h"
struct AVFilterGraph;
class AVFilterContext;
namespace HL
{

	class VideoThread : public QThread
	{
	public:
		bool open();
		void close();
		void clear();
		void setParameters(AVCodecParameters *para_);
		void setWindow(VideoCall *widget);
		void setStatus(PlayStatus sta);
		bool isPause() const;
		void run() override;
		void push(AVPacket *pkt);
		int init_filter(QString subFilePath = "", int subStreamindex = -1);
		VideoCall *getVideoCall();
		Decode *getDecode();
		AVRational timeBase() const;
		VideoThread() = default;
		VideoThread(AVCodecParameters *para_, int w, int h);
		~VideoThread();
		// 同步时间又外部传入
		int64_t synpts = 0;

	private:
		std::queue<AVPacket *> packs;
		//	std::set<std::pair<int,AVPacket*>> packs;
		bool is_exit = false;
		Decode *decode = nullptr;
		AVCodecParameters *para = nullptr;
		AVFilterContext *buffersrcContext = nullptr;
		AVFilterContext *buffersinkContext = nullptr;
		VideoCall *video_widget = nullptr;
		PlayStatus status = PlayStatus::STOP;
		std::mutex mtx;

		// 滤波器用成员
		AVFilterGraph *filter_graph = nullptr;
		AVFilterContext *buffersrc_ctx = nullptr;
		AVFilterContext *buffersink_ctx = nullptr;
	};

}
