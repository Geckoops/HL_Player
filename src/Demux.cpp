#include "Demux.h"
#include "Util.h"
#include "Reference.h"
#include <iostream>
#include <QFile>
#include <QDebug>
#include <set>
extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
}
struct AVDictionary
{
	int count;
	AVDictionaryEntry *elems;
};

HL::Demux::Demux()
{
	static bool isFirst = true;
	static std::mutex demux_mtx;
	demux_mtx.lock();
	if (isFirst)
	{
		avformat_network_init();
		isFirst = false;
	}
	demux_mtx.unlock();
}

HL::Demux::~Demux()
{
	std::lock_guard lg(mtx);
	if (ic != nullptr)
		avformat_close_input(&ic);
}

bool HL::Demux::open(const char *url)
{
	close();
	AVDictionary *opts = nullptr;
	// 设置rtsp流以tcp打开
	av_dict_set(&opts, "rtsp_transport", "tcp", 0);
	av_dict_set(&opts, "rtmp_transport", "tcp", 0);
	// 设置网络延时时间
	av_dict_set(&opts, "max_delay", "500", 0);
	std::lock_guard lk(mtx);
	if (avformat_open_input(&ic, url, nullptr, &opts) != 0)
	{
		HL::print_error_log("demux open error!");
		av_dict_free(&opts);
		return false;
	}
	av_dict_free(&opts);
	//	Global::subManager.subStreamIndexList.clear();
	//	Global::subManager.enable = false;
	//	Global::subManager.filePath = "";
	//	Global::subManager.currentSubStreamIndex = -1;
	Global::subManager.subList.clear();
	Global::subManager.enable = false;
	Global::subManager.curIndex = -1;

	avformat_find_stream_info(ic, nullptr);
	for (int i = 0; i < ic->nb_streams; ++i)
	{
		if (ic->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			if (videoStreamIndex == -1)
			{
				videoStreamIndex = i;
			}
		}
		else if (ic->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
		{
			if (audioStreamIndex == -1)
				audioStreamIndex = i;
		}
		//		else if(ic->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_SUBTITLE)
		//			Global::subManager.subStreamIndexList.emplace_back(i);
	}

	//	if(!Global::subManager.subStreamIndexList.empty())
	//		Global::subManager.currentSubStreamIndex = 0;
	//	Global::subManager.enable = true;
	auto &videoStream = ic->streams[videoStreamIndex];
	m_width = videoStream->codecpar->width;
	m_height = videoStream->codecpar->height;
	// 清空文件再写入，这样保证只有视频信息
	Global::subManager.clearFile();
	av_dump_format(ic, 0, url, 0);
	fflush(stderr);
	// 读取文件内容提取字幕信息
	Global::subManager.loadMetadata(url);
	if (Global::subManager.subList.size() > 0)
	{
		Global::subManager.enable = true;
		Global::subManager.curIndex = 0;
	}
	totalLength = Util::ratio_to_milliseconds(AVRational{1, AV_TIME_BASE}, ic->duration);
	std::set<QString> miscExtentions{
		"flac"};
	QString ext(url);
	ext = ext.right(ext.length() - ext.lastIndexOf('.') - 1);
	if (!miscExtentions.contains(ext))
	{
		Global::isVideo = true;
	}
	return true;
}

AVPacket *HL::Demux::read_packet()
{
	std::lock_guard lg(mtx);
	if (ic == nullptr)
	{
		HL::print_error_log("read_packet error! ic is nullptr");
		return nullptr;
	}
	AVPacket *pkt = av_packet_alloc();
	if (av_read_frame(ic, pkt) != 0)
	{
		// 播放完毕
		HL::print_error_log("packet 读取失败");
		av_packet_free(&pkt);
		return nullptr;
	}

	// pts 和 dts 转换单位
	auto &&ratio = ic->streams[pkt->stream_index]->time_base;
	pkt->pts = Util::ratio_to_milliseconds(ratio, pkt->pts);
	pkt->pts = Util::ratio_to_milliseconds(ratio, pkt->dts);
	//	std::cerr << pkt.pts() << " " << pkt.dts() << std::endl;
	return pkt;
}

AVCodecParameters *HL::Demux::copyVideoCodecParameters()
{
	std::lock_guard lg(mtx);
	if (!ic)
		return NULL;
	AVCodecParameters *pa = avcodec_parameters_alloc();
	avcodec_parameters_copy(pa, ic->streams[videoStreamIndex]->codecpar);
	return pa;
}

AVCodecParameters *HL::Demux::copyAudioCodecParameters()
{
	std::lock_guard lg(mtx);
	if (!ic)
		return NULL;
	AVCodecParameters *pa = avcodec_parameters_alloc();
	avcodec_parameters_copy(pa, ic->streams[audioStreamIndex]->codecpar);
	return pa;
}

bool HL::Demux::seek(double progress)
{
	progress = std::min(progress, 1.0);
	progress = std::max(progress, 0.0);
	std::lock_guard lg(mtx);
	if (ic == nullptr)
	{
		HL::print_error_log("seek error ! ic = nullptr");
		return false;
	}
	avformat_flush(ic);

	int64_t position = (long double)ic->streams[videoStreamIndex]->duration * progress;
	if (ic->streams[videoStreamIndex]->duration < 0)
	{
		position = static_cast<double>(ic->duration) / AV_TIME_BASE;
		position *= static_cast<double>(ic->streams[videoStreamIndex]->time_base.den) / ic->streams[videoStreamIndex]->time_base.num;
		position *= progress;
	}
	if (Global::isVideo)
	{
		if (av_seek_frame(ic, videoStreamIndex, position, AVSEEK_FLAG_FRAME | AVSEEK_FLAG_BACKWARD) < 0)
		{
			HL::print_error_log("seek error ! ic = nullptr");
			return false;
		}
	}
	else
	{
		if (av_seek_frame(ic, audioStreamIndex, position, AVSEEK_FLAG_FRAME | AVSEEK_FLAG_BACKWARD) < 0)
		{
			HL::print_error_log("seek error ! ic = nullptr");
			return false;
		}
	}

	return true;
}

void HL::Demux::clear()
{
	std::lock_guard lg(mtx);
	if (ic == nullptr)
	{
		HL::print_error_log("clear error ! ic = nullptr");
		return;
	}
	avformat_flush(ic);
}

void HL::Demux::close()
{
	std::lock_guard lg(mtx);
	if (ic == nullptr)
	{
		HL::print_error_log("ic = nullptr");
		return;
	}
	avformat_close_input(&ic);
	this->totalLength = 0;
	this->videoStreamIndex = this->audioStreamIndex = -1;
}
bool HL::Demux::isAudio(AVPacket *pkt)
{
	if (pkt == nullptr)
		return false;
	return pkt->stream_index == audioStreamIndex;
}
bool HL::Demux::isVideo(AVPacket *pkt)
{
	if (pkt == nullptr)
		return false;
	return pkt->stream_index == videoStreamIndex;
}
int HL::Demux::width() const
{
	return m_width;
}
int HL::Demux::height() const
{
	return m_height;
}
int64_t HL::Demux::totalMs() const
{
	return totalLength;
}
AVRational HL::Demux::audioTimeBase() const
{
	return ic->streams[audioStreamIndex]->time_base;
}

AVRational HL::Demux::videoTimeBase() const
{
	return ic->streams[videoStreamIndex]->time_base;
}
