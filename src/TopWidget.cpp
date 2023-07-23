// You may need to build the project (run Qt uic code generator) to get "ui_TopWidget.h" resolved

#include "TopWidget.h"
#include "ui_TopWidget.h"

namespace HL
{
	TopWidget::TopWidget(QWidget *parent) : QWidget(parent), ui(new Ui::TopWidget)
	{
		ui->setupUi(this);
	}

	TopWidget::~TopWidget()
	{
		delete ui;
	}

	void TopWidget::resizeEvent(QResizeEvent *event)
	{
		static bool first = true;
		if (first)
		{
			first = false;
			return;
		}
		ui->openGLWidget->setImage(QImage(":/icon/logo.png"));
		resizeEvent(event);
	}
}