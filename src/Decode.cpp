#include "Decode.h"
#include "Util.h"

extern "C"
{
#include "libavcodec/avcodec.h"
}

bool HL::Decode::open(AVCodecParameters *para, int thread_count)
{
	if (para == nullptr)
		return false;
	close();
	const AVCodec *codec = avcodec_find_decoder(para->codec_id);
	if (codec == nullptr)
	{
		HL::print_error_log("codec not found error!");
		avcodec_parameters_free(&para);
		return false;
	}
	std::lock_guard lg(mtx);
	m_context = avcodec_alloc_context3(codec);
	avcodec_parameters_to_context(m_context, para);
	avcodec_parameters_free(&para);
	m_context->thread_count = thread_count;

	int error_code = avcodec_open2(m_context, nullptr, nullptr);
	if (error_code != 0)
	{
		HL::print_error_log("解码器打开失败");
		avcodec_free_context(&m_context);
		return false;
	}
	m_is_open = true;
	return true;
}

void HL::Decode::close()
{
	std::lock_guard lg(mtx);
	m_is_open = false;
	if (m_context != nullptr)
	{
		avcodec_close(m_context);
		avcodec_free_context(&m_context);
	}
	pts = 0;
}

void HL::Decode::clear()
{
	std::lock_guard lg(mtx);
	pts = 0;
	if (m_context != nullptr)
		avcodec_flush_buffers(m_context);
}

HL::Decode::~Decode()
{
	std::lock_guard lg(mtx);
	m_is_open = false;
	pts = 0;
	if (m_context != nullptr)
	{
		avcodec_close(m_context);
		avcodec_free_context(&m_context);
	}
}

// 发送到解码线程，不管成功与否都释放pkt空间（对象和媒体内容）
bool HL::Decode::send(AVPacket *pkt)
{
	if (pkt == nullptr or pkt->size <= 0 or pkt->data == nullptr)
	{
		if (pkt != nullptr)
			av_packet_free(&pkt);
		return false;
	}
	//	std::lock_guard lg(mtx);
	if (m_context == nullptr)
		return false;

	int error_code = avcodec_send_packet(m_context, pkt);

	av_packet_free(&pkt);
	if (error_code != 0)
		return false;
	return true;
}
// 获取解码数据，一次send可能需要多次Recv，获取缓冲中的数据Send NULL在Recv多次
// 每次复制一份，由调用者释放 av_frame_free
AVFrame *HL::Decode::recv()
{
	//	std::lock_guard lg(mtx);
	if (m_context == nullptr)
		return nullptr;
	AVFrame *frame = av_frame_alloc();
	int error_code = avcodec_receive_frame(m_context, frame);
	if (error_code != 0)
	{
		av_frame_free(&frame);
		return nullptr;
	}
	//	cerr << "["<<frame->linesize[0] << "] " << std::flush;
	pts = frame->pts;
	return frame;
}

bool HL::Decode::empty()
{
	return m_context->codec == nullptr;
}

bool HL::Decode::isNull()
{
	return m_context == nullptr;
}
bool HL::Decode::isOpen()
{
	return m_is_open;
}
AVCodecContext *HL::Decode::codecContext() const
{
	return m_context;
}
