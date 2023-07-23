#pragma once

#include "AudioPlay.h"
#include <QAudioFormat>
#include <QAudioDevice>
#include <QMediaDevices>
#include <QAudioOutput>
#include <QAudioSink>
#include <mutex>
#include "Util.h"
extern "C"
{
	// #include <libavutil/samplefmt.h>
};
namespace HL
{
	class QtAudioPlay : public AudioPlay
	{
	public:
		bool open() override;
		void close() override;
		void clear() override;
		bool write(uint8_t *data, int size);
		size_t bytesFree();
		int64_t getNoPlayMs() override;
		void setFormat(const QAudioFormat &fmt);
		bool isOpen();
		bool isPause() override;
		void setStatus(PlayStatus sta);
		void setVolume(double volume);
		QtAudioPlay();
		~QtAudioPlay();

	private:
		QAudioFormat::SampleFormat m_sample_fmt = QAudioFormat::SampleFormat::Unknown;
		int m_sample_rate = 0;
		int m_ch_count = 0;
		bool m_is_open = false;
		QAudioDevice m_device;
		QAudioOutput m_vol_control;
		QAudioFormat m_fmt;
		QAudioSink *m_sink = nullptr;
		QIODevice *m_io = nullptr;
		PlayStatus status = PlayStatus::STOP;
		std::mutex mtx;
	};

}
