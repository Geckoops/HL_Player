#pragma once

#include <QOpenGLFunctions>
#include <QtOpenGLWidgets/QOpenGLWidget>
#include <QOpenGLShaderProgram>
#include <QFile>
#include <mutex>
#include <VideoCall.h>
#include <QOpenGLTexture>

extern "C"
{
#include <libavutil/frame.h>
}

namespace HL
{
	class VideoWidget : public QOpenGLWidget, protected QOpenGLFunctions, public VideoCall
	{
		Q_OBJECT
	public:
		VideoWidget(QWidget *parent = nullptr);
		void init(int width, int height) override;
		void rePaint(AVFrame *frame) override;
		void initTexture();
		void initShaders();
		~VideoWidget();

	protected:
		// 初始化
		void initializeGL() override;
		// 绘制
		void paintGL() override;
		// 设置尺寸
		void resizeGL(int width, int height) override;

	public:
		QOpenGLShaderProgram m_program;
		QOpenGLTexture *texture = nullptr;
		QVector<QVector2D> vertices;
		QVector<QVector2D> texCoords;
		// shader中的yuv变量地址
		GLuint yuv[3]{0};
		// opengl中的texture地址
		GLuint texs[3]{0};
		int width = 240, height = 128;
		// 材质内存空间
		uint8_t *data[3]{nullptr, nullptr, nullptr};
		std::mutex mtx;
	};
}
