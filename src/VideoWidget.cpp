#include "VideoWidget.h"
#include <QFile>
#include <QTimer>
#include <QtGlobal>
#define GET_STR(x) #x
static constexpr int A_VER = 3;
static constexpr int T_VER = 4;
static const char vString[] = GET_STR(
	attribute vec4 vertexIn;
	attribute vec2 textureIn;
	varying vec2 textureOut;

	void main() 
	{
		gl_Position = vertexIn;
		textureOut = textureIn;
	});

static const char tString[] = GET_STR(
	varying vec2 textureOut;
	uniform sampler2D tex_y;
	uniform sampler2D tex_u;
	uniform sampler2D tex_v;
	void main() 
	{
		vec3 yuv;
		vec3 rgb;
		yuv.x = texture2D(tex_y, textureOut).r;
		yuv.y = texture2D(tex_u, textureOut).r - 0.5;
		yuv.z = texture2D(tex_v, textureOut).r - 0.5;
		rgb = mat3(
				  1.0, 1.0, 1.0,
				  0.0, -0.39465, 2.03211,
				  1.13983, -0.58060, 0.0) *
			  yuv;
		gl_FragColor = vec4(rgb, 1.0);
	});

void HL::VideoWidget::initializeGL()
{

	qDebug() << "初始化";
	std::lock_guard lg(mtx);
	// 初始化opengl函数
	initializeOpenGLFunctions();
	// 背景设为黑色
	glClearColor(0, 0, 0, 0);
	glEnable(GL_TEXTURE_2D); // 设置纹理2D功能可用
	initTexture();
	initShaders();
}

void HL::VideoWidget::paintGL()
{
	std::lock_guard lg(mtx);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	for (int i = 0; i < 3; i++)
	{
		int den = (i == 0 ? 1 : 2);
		// 激活 0层材质
		glActiveTexture(GL_TEXTURE0 + i);
		// 0层材质绑定到 texs[0]即 Y通道
		glBindTexture(GL_TEXTURE_2D, texs[i]);
		// 修改材质内容(复制内存内容)
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width / den, height / den, GL_RED, GL_UNSIGNED_BYTE, data[i]);
		glUniform1i(yuv[i], i);
	}
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void HL::VideoWidget::resizeGL(int width, int height)
{
	std::lock_guard lg(mtx);
	qDebug() << width << height;
}

HL::VideoWidget::VideoWidget(QWidget *parent) : QOpenGLWidget(parent) {}

// 不管成不成狗都释放frame空间
void HL::VideoWidget::rePaint(AVFrame *frame)
{
	if (frame == nullptr)
		return;
	// frame -> yuv420p

	std::lock_guard lg(mtx);
	if (!data[0] || width * height == 0 || frame->width != this->width || frame->height != this->height)
	{
		av_frame_free(&frame);
		return;
	}
	if (width == frame->linesize[0])
	{
		memcpy(data[0], frame->data[0], width * height);
		memcpy(data[1], frame->data[1], width * height / 4);
		memcpy(data[2], frame->data[2], width * height / 4);
	}
	else
	{
		for (int i = 0; i < height; i++)
			memcpy(data[0] + width * i, frame->data[0] + frame->linesize[0] * i, width);
		for (int i = 0; i < height / 2; i++)
			memcpy(data[1] + width / 2 * i, frame->data[1] + frame->linesize[1] * i, width / 2);
		for (int i = 0; i < height / 2; i++)
			memcpy(data[2] + width / 2 * i, frame->data[2] + frame->linesize[2] * i, width / 2);
	}
	update();
	av_frame_free(&frame);
}

void HL::VideoWidget::init(int width, int height)
{
	qDebug() << width << height;
	std::lock_guard lg(mtx);
	this->width = width;
	this->height = height;
	delete data[0];
	delete data[1];
	delete data[2];
	/// 分配材质内存空间
	data[0] = new unsigned char[width * height];	 // Y
	data[1] = new unsigned char[width * height / 4]; // U
	data[2] = new unsigned char[width * height / 4]; // V
	if (texs[0])
	{
		glDeleteTextures(3, texs);
	}
	// 创建材质
	glGenTextures(3, texs);
	// YUV分别做，执行3次, UV的宽高是Y的一半, 因为是yuv420
	for (int i = 0; i < 3; i++)
	{
		int den = i ? 2 : 1;
		glBindTexture(GL_TEXTURE_2D, texs[i]);
		// 放大过滤算法为线性插值
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		// 缩小过滤算法为线性插值
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		// 设置材质显卡空间
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width / den, height / den, 0, GL_RED, GL_UNSIGNED_BYTE, 0);
	}
}

HL::VideoWidget::~VideoWidget()
{
	for (int i = 0; i < 3; i++)
		if (data[i])
			delete data[i];
}

void HL::VideoWidget::initTexture()
{
	texture = new QOpenGLTexture(QOpenGLTexture::Target2D);
	texture->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
	texture->setMagnificationFilter(QOpenGLTexture::Linear);
	// 重复使用纹理坐标
	// 纹理坐标(1.1, 1.2)与(0.1, 0.2)相同
	texture->setWrapMode(QOpenGLTexture::Repeat);
	// 设置纹理大小
	texture->setSize(this->width, this->height);
	// 分配储存空间
	texture->allocateStorage();
	texture->setData(QImage("/icon/logo.png"));
	texture->setLevelofDetailBias(-10); // 值越小，图像越清晰
}

void HL::VideoWidget::initShaders()
{
	// 添加片元shader
	qDebug() << m_program.addShaderFromSourceCode(QOpenGLShader::Fragment, tString);
	// 添加顶点shader
	qDebug() << m_program.addShaderFromSourceCode(QOpenGLShader::Vertex, vString);
	// 设置顶点坐标的变量
	m_program.bindAttributeLocation("vertexIn", A_VER);
	// 设置材质坐标
	m_program.bindAttributeLocation("textureIn", T_VER);
	// 编译shader
	qDebug() << "program.link() = " << m_program.link();
	qDebug() << "program.bind() = " << m_program.bind();

	// 顶点坐标
	vertices.append(QVector2D(-1, -1));
	vertices.append(QVector2D(1, -1));
	vertices.append(QVector2D(-1, 1));
	vertices.append(QVector2D(1, 1));

	// 材质坐标
	texCoords.append(QVector2D(0, 1));
	texCoords.append(QVector2D(1, 1));
	texCoords.append(QVector2D(0, 0));
	texCoords.append(QVector2D(1, 0));

	// 顶点
	glVertexAttribPointer(A_VER, 2, GL_FLOAT, 0, 0, vertices.constData());
	glEnableVertexAttribArray(A_VER);

	// 材质
	glVertexAttribPointer(T_VER, 2, GL_FLOAT, 0, 0, texCoords.constData());
	glEnableVertexAttribArray(T_VER);

	// 从shader 获取材质
	yuv[0] = m_program.uniformLocation("tex_y");
	yuv[1] = m_program.uniformLocation("tex_u");
	yuv[2] = m_program.uniformLocation("tex_v");
}
