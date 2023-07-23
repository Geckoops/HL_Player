#pragma once

#include <QWidget>
#include <QDialog>
#include <QTimer>
#include "PlayWidget.h"
#include "ToolWidget.h"

QT_BEGIN_NAMESPACE
namespace Ui
{
	class RootWidget;
}
QT_END_NAMESPACE

namespace HL
{
	class RootWidget : public QWidget
	{
		Q_OBJECT

	public:
		explicit RootWidget(QWidget *parent = nullptr);
		~RootWidget() override;

	public slots:
		void ShowFullScreen();
		void ShowNormal();
		void ShowMinimized();
		void Close();
		void ShowFullScreenOrNormal();
		void finishedEvent();
		PlayWidget *GetPlayWidget() const;
		ToolWidget *GetToolWidget() const;
		QTimer *getHideTimer() const;

	protected:
		void resizeEvent(QResizeEvent *evt) override;
		void mouseDoubleClickEvent(QMouseEvent *evt) override;
		void mouseMoveEvent(QMouseEvent *event) override;
		void keyPressEvent(QKeyEvent *event) override;

	private:
		void BindAllEvent();

	private:
	public:
		Ui::RootWidget *ui;
		PlayWidget *pw;
		ToolWidget *tw;
		DemuxThread dt;
		QTimer *hideTimer;
		QTimer *playTimer;
		QTimer *twSliderTimer;
		QRect normalGeo;
	};

}
