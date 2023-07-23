#include "Slider.h"
#include <iostream>

namespace HL
{
	Slider::Slider(QWidget *parent) : QSlider(parent)
	{
	}
	Slider::~Slider()
	{
	}
	void Slider::mousePressEvent(QMouseEvent *evt)
	{
		QSlider::mousePressEvent(evt);
		double pos = 1.0 * evt->pos().x() / width();
		setValue(pos * maximum());
		sliderReleased();
	}
	void Slider::keyPressEvent(QKeyEvent *ev)
	{
		//	QAbstractSlider::keyPressEvent(ev);
	}
}