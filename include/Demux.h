#pragma once

#include <cstdint>
#include <mutex>
#include <vector>
#include <optional>
#include "Reference.h"

namespace HL
{

	class Demux
	{
	public:
		Demux();
		virtual ~Demux();
		virtual bool open(const char *url);
		// 空间需要调用者释放 ，释放AVPacket对象空间，和数据空间 av_packet_free
		AVPacket *read_packet();
		// 获取视频参数  返回的空间需要清理  avcodec_parameters_free
		AVCodecParameters *copyVideoCodecParameters();
		AVCodecParameters *copyAudioCodecParameters();
		bool seek(double progress);
		void clear();
		void close();
		bool isAudio(AVPacket *pkt);
		bool isVideo(AVPacket *pkt);
		int width() const;
		int height() const;
		int64_t totalMs() const;
		AVRational audioTimeBase() const;
		AVRational videoTimeBase() const;

	private:
		AVFormatContext *ic = nullptr;
		int videoStreamIndex = -1;
		int audioStreamIndex = -1;
		int m_width = 0;
		int m_height = 0;
		int64_t totalLength = 0;
		std::mutex mtx;
	};

}

