#include "Resample.h"
#include "Util.h"
extern "C"
{
#include <libswresample/swresample.h>
}
bool HL::Resample::open(AVCodecParameters *para)
{
	if (para == nullptr)
		return false;
	std::lock_guard lg(mtx);
	AVChannelLayout out_ch_layout;
	av_channel_layout_default(&out_ch_layout, 2);
	const AVChannelLayout in_ch_layout = para->ch_layout;
	// swr_context 如果为空会自动分配空间
	swr_alloc_set_opts2(&m_swr_ctx,
						&out_ch_layout,
						m_format,
						para->sample_rate,
						&in_ch_layout,
						static_cast<AVSampleFormat>(para->format),
						para->sample_rate,
						0, nullptr);

	int error_code = swr_init(m_swr_ctx);
	if (error_code != 0)
	{
		HL::print_error_log("swr_init error!");
		avcodec_parameters_free(&para);
		return false;
	}
	m_sample_rate = para->sample_rate;
	m_ch_count = in_ch_layout.nb_channels;
	m_is_open = true;
	avcodec_parameters_free(&para);
	return true;
}
// 返回重采样后的大小, 不管成功与否都释放in_data空间
int64_t HL::Resample::resample(AVFrame *in_data, uint8_t *out_data)
{
	if (in_data == nullptr)
		return 0;
	if (out_data == nullptr)
	{
		HL::print_error_log("out_data is nullptr");
		av_frame_free(&in_data);
		return 0;
	}
	uint8_t *data[2]{out_data, nullptr};
	int64_t samples_per_ch = swr_convert(m_swr_ctx, data, in_data->nb_samples,
										 const_cast<const uint8_t **>(in_data->extended_data),
										 in_data->nb_samples);
	if (samples_per_ch <= 0)
	{
		HL::print_error_log("swr_convert error!");
		av_frame_free(&in_data);
		return samples_per_ch;
	}
	int outSize = samples_per_ch * in_data->ch_layout.nb_channels * av_get_bytes_per_sample(m_format);
	av_frame_free(&in_data);
	return outSize;
}

void HL::Resample::close()
{
	std::lock_guard lg(mtx);
	m_is_open = false;
	if (m_swr_ctx != nullptr)
		swr_free(&m_swr_ctx);
}

HL::Resample::~Resample()
{
	std::lock_guard lg(mtx);
	m_is_open = false;
	if (m_swr_ctx != nullptr)
		swr_free(&m_swr_ctx);
}

int HL::Resample::sample_rate()
{
	return m_sample_rate;
}

int HL::Resample::ch_count()
{
	return m_ch_count;
}

AVSampleFormat HL::Resample::format()
{
	return m_format;
}

bool HL::Resample::isOpen()
{
	return m_is_open;
}
