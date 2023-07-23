#include "Util.h"
#include <libavutil/rational.h>
#include <iostream>
#include <QWidget>
namespace HL
{
	int64_t Util::ratio_to_milliseconds(AVRational ratio, int64_t duration)
	{
		int64_t milliseconds = 1000 * duration * ratio.num / ratio.den;
		return milliseconds;
	}

	QAudioFormat::SampleFormat Util::AVSampleFormat_to_QSampleFormat(AVSampleFormat src)
	{
		static std::unordered_map<AVSampleFormat, QAudioFormat::SampleFormat> table{
			{AVSampleFormat::AV_SAMPLE_FMT_NONE, QAudioFormat::Unknown},
			{AV_SAMPLE_FMT_U8, QAudioFormat::UInt8},
			{AV_SAMPLE_FMT_S16, QAudioFormat::Int16},
			{AV_SAMPLE_FMT_S32, QAudioFormat::Int32},
			{AV_SAMPLE_FMT_FLT, QAudioFormat::Float},
			{AV_SAMPLE_FMT_U8P, QAudioFormat::UInt8},
			{AV_SAMPLE_FMT_S16P, QAudioFormat::Int16},
			{AV_SAMPLE_FMT_S32P, QAudioFormat::Int32},
			{AV_SAMPLE_FMT_FLTP, QAudioFormat::Float}};
		if (table.contains(src))
			return table[src];
		else
		{
			print_error_log("QtAudioPlay不支持该格式的音频");
			return QAudioFormat::Unknown;
		}
	}

	AVCodecParameters *Util::copyParameters(AVCodecParameters *src)
	{
		AVCodecParameters *dst = avcodec_parameters_alloc();
		avcodec_parameters_copy(dst, src);
		return dst;
	}

	void Util::getAllChildren(QObject *widget, std::vector<QObject *> &res)
	{
		if (widget == nullptr)
			return;
		res.push_back(widget);
		auto v = widget->children();
		for (auto u : v)
			getAllChildren(u, res);
	}

	std::tuple<int, int, int> Util::ms2hms(int64_t ms)
	{
		int hh = 0, mm = 0, ss = 0;
		ss = ms / 1000;
		mm += ss / 60;
		ss %= 60;
		hh += mm / 60;
		mm %= 60;
		return std::make_tuple(hh, mm, ss);
	}
}
