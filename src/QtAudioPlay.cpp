#include "QtAudioPlay.h"
#include "Util.h"
#include <QThread>
#include <QDebug>

namespace HL
{
	bool QtAudioPlay::open()
	{
		close();
		std::lock_guard lg(mtx);
		if (!m_fmt.isValid())
		{
			HL::print_error_log("音频格式无效");
			return false;
		}
		m_sink = new QAudioSink(m_device, m_fmt);
		if (m_sink->error() != 0)
		{
			print_error_log("m_sink 创建失败");
			return false;
		}
		// 获取流指针
		m_io = m_sink->start();
		if (m_io == nullptr)
		{
			close();
			return false;
		}
		m_is_open = true;
		setStatus(PlayStatus::PLAY);
		return true;
	}

	void QtAudioPlay::close()
	{
		std::lock_guard lg(mtx);
		m_is_open = false;
		setStatus(PlayStatus::STOP);
		if (m_sink != nullptr)
		{
			m_sink->stop();
			delete m_sink;
			m_sink = nullptr;
		}
		if (m_io != nullptr)
		{
			m_io->close();
			delete m_io;
			m_io = nullptr;
		}
	}

	QtAudioPlay::QtAudioPlay() : m_device(QMediaDevices::defaultAudioOutput()), m_vol_control(m_device)
	{
		m_fmt.setChannelCount(m_ch_count);
		m_fmt.setSampleRate(m_sample_rate);
		m_fmt.setSampleFormat(m_sample_fmt);
	}

	QtAudioPlay::~QtAudioPlay()
	{
		std::lock_guard lg(mtx);
		m_is_open = false;
		if (m_io != nullptr)
		{
			m_io->close();
			//		delete m_io;
			m_io = nullptr;
		}
		if (m_sink != nullptr)
		{
			m_sink->stop();
			delete m_sink;
			m_sink = nullptr;
		}
	}

	void QtAudioPlay::setFormat(const QAudioFormat &fmt)
	{
		m_fmt = fmt;
		m_sample_rate = fmt.sampleRate();
		m_sample_fmt = fmt.sampleFormat();
		m_ch_count = fmt.channelCount();
	}

	size_t QtAudioPlay::bytesFree()
	{
		std::lock_guard lg(mtx);
		if (m_sink == nullptr)
		{
			HL::print_error_log("m_sink 为初始化");
			return 0;
		}
		return m_sink->bytesFree();
	}

	bool QtAudioPlay::write(uint8_t *data, int size)
	{
		if (data == nullptr or size <= 0)
			return false;
		std::lock_guard lg(mtx);
		if (m_sink == nullptr or m_io == nullptr)
		{
			return false;
		}
		int len = m_io->write(reinterpret_cast<char *>(data), size);
		if (len != size)
		{
			print_error_log("实际写入的大小与源数据大小不一致");
			return false;
		}
		return true;
	}

	bool QtAudioPlay::isOpen()
	{
		return m_is_open;
	}

	int64_t QtAudioPlay::getNoPlayMs()
	{
		mtx.lock();
		if (!m_is_open)
		{
			mtx.unlock();
			return 0;
		}
		int64_t pts = 0;
		// 还未播放的字节数
		double size = m_sink->bufferSize() - m_sink->bytesFree();
		// 一秒字节大小
		assert(m_fmt.bytesPerSample() == 2);
		double secSize = m_sample_rate * m_fmt.bytesPerSample() * m_ch_count;
		if (secSize <= 0)
			pts = 0;
		else
			pts = size / secSize * 1000;
		mtx.unlock();
		return pts;
	}

	bool QtAudioPlay::isPause()
	{
		return status == PlayStatus::PAUSE;
	}

	void QtAudioPlay::setStatus(PlayStatus sta)
	{
		switch (sta)
		{
		case PlayStatus::PAUSE:
			m_sink->suspend();
			break;
		case PlayStatus::PLAY:
			m_sink->resume();
			break;
		}
		status = sta;
	}

	void QtAudioPlay::clear()
	{
		std::lock_guard lg(mtx);
		//	if(m_io != nullptr)
		//		m_io->reset();
		if (m_sink)
			m_sink->reset();
	}

	void QtAudioPlay::setVolume(double volume)
	{
		volume = std::max(0.0, volume);
		volume = std::min(1.0, volume);
		m_sink->setVolume(volume);
	}
}
