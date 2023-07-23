#pragma once

#include <cstdint>
#include <chrono>
#include <iostream>
#include <fstream>
#include <QFile>
#include "Reference.h"
#include <QAudioFormat>
#include <SubManager.h>
#include "SqliteHelper.h"
extern "C" 
{
#include <libavformat/avformat.h>
};



namespace HL {
using std::cerr;
#define print_error_log(str) cerr << __FILE__ << " " << __LINE__ << "\n" << str << std::endl
	class Util {
	public:
		static int64_t ratio_to_milliseconds(AVRational ratio, int64_t duration = 1);
		static QAudioFormat::SampleFormat AVSampleFormat_to_QSampleFormat(AVSampleFormat src);
		static AVCodecParameters* copyParameters(AVCodecParameters *src);
		static void getAllChildren(QObject *w,std::vector<QObject*> &res);
		static std::tuple<int,int,int> ms2hms(int64_t ms);
	};

enum class PlayStatus {
	PAUSE,
	PLAY,
	STOP
};

enum class Constant {
	 // 菜单窗口自动隐藏的待机时间
	 HIDDENTIME  = 3000,
	 // 跳转时考虑的pts毫秒数
	 PTSERRORRANGE = 100,
	 // 视频帧缓冲队列大小
	 MAXPACKETNUMBER = 30,
	 // 快进快退步长
	 STEPMSCOUNT = 5000
};

struct Global {
	inline static RootWidget *rw = nullptr;
	inline static PlayWidget *pw = nullptr;
	inline static DemuxThread *dt = nullptr;
	inline static ToolWidget *tw = nullptr;
	inline static QString lastFilePath = "";
	inline static QString currentFilePath = "";
	inline static double playBackSpeed = 0;
	inline static int volume = 100;
	inline static SubManager subManager;
	inline static SqliteHelper *sqliteHelper;
	inline static bool isVideo = false;
};
}
