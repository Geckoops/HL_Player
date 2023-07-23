#pragma once

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
namespace HL
{

	class LogoGLWidget : public QOpenGLWidget, protected QOpenGLFunctions
	{
		Q_OBJECT
	public:
		explicit LogoGLWidget(QWidget *parent = 0);

	public slots:
		void initializeGL() Q_DECL_OVERRIDE;
		void resizeGL(int w, int h) Q_DECL_OVERRIDE;
		void paintGL() Q_DECL_OVERRIDE;
		void setImage(const QImage &image);
		void initTextures();
		void initShaders();

	private:
		QVector<QVector3D> vertices;
		QVector<QVector2D> texCoords;
		QOpenGLShaderProgram program;
		QOpenGLTexture *texture;
		QMatrix4x4 projection;
	};

}
