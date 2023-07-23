#pragma once

#include "Reference.h"
#include <mutex>
#include <thread>
namespace HL
{
	class Decode
	{
	public:
		// 当前解码到的pts
		int64_t pts = 0;
		virtual bool open(AVCodecParameters *para, int thread_count = 1);
		virtual void close();
		virtual void clear();
		virtual bool send(AVPacket *pkt);
		virtual AVFrame *recv();
		bool empty();
		bool isNull();
		bool isOpen();
		AVCodecContext *codecContext() const;
		Decode() = default;
		virtual ~Decode();

	protected:
		AVCodecContext *m_context = nullptr;
		std::mutex mtx;
		bool m_is_open = false;
	};
}
