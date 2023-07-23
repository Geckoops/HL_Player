#pragma once

#include "Reference.h"
#include <mutex>
extern "C"
{
#include <libavutil/samplefmt.h>
}
namespace HL
{
	class Resample
	{
	public:
		virtual bool open(AVCodecParameters *para);
		virtual void close();
		virtual int64_t resample(AVFrame *in_data, uint8_t *out_data);
		int sample_rate();
		int ch_count();
		AVSampleFormat format();
		bool isOpen();
		Resample() = default;
		~Resample();

	protected:
		int m_sample_rate = 0;
		int m_ch_count = 0;
		bool m_is_open = false;
		AVSampleFormat m_format = AVSampleFormat::AV_SAMPLE_FMT_S16;
		std::mutex mtx;
		SwrContext *m_swr_ctx = nullptr;
		//	AVSampleFormat m_out_fmt = AV_SAMPLE_FMT_S16;
	};
}

