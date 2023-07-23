#pragma once

#include <cstdint>
namespace HL
{
	class AudioPlay
	{
	public:
		virtual bool open() = 0;
		virtual void close() = 0;
		virtual void clear() = 0;
		virtual bool write(uint8_t *data, int size) = 0;
		virtual size_t bytesFree() = 0;
		// 返回缓冲中还没有播放的时间
		virtual int64_t getNoPlayMs() = 0;
		virtual bool isPause() = 0;
		AudioPlay() = default;
		virtual ~AudioPlay() = default;
	};
}
