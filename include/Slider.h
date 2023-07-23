#pragma once

#include <QSlider>
#include <QMouseEvent>
namespace HL
{
	class Slider : public QSlider
	{
		Q_OBJECT

	public:
		explicit Slider(QWidget *parent = nullptr);
		~Slider() override;
		void mousePressEvent(QMouseEvent *evt) override;

	protected:
		void keyPressEvent(QKeyEvent *ev) override;

	};

}

