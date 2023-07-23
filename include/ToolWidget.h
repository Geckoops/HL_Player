#pragma once

#include <QWidget>
#include <QPushButton>
#include "DemuxThread.h"
#include "AudioThread.h"
QT_BEGIN_NAMESPACE
namespace Ui
{
	class ToolWidget;
}
QT_END_NAMESPACE

namespace HL
{

	class ToolWidget : public QWidget
	{
		Q_OBJECT

	public:
		explicit ToolWidget(QWidget *parent = nullptr);
		~ToolWidget() override;

		void timerEvent(QTimerEvent *evt) override;
		void Show();
		void Hide();
		Ui::ToolWidget *Ui();

	protected:
		void resizeEvent(QResizeEvent *event) override;
		void initSpeedMenu();
	public slots:
		void SliderPress();
		void SliderRelease();
		void SetTotalTime();
		void SetCurrentTime();
		void VideoSliderUpdate();
		void VideoStepForward();
		void VideoStepBackward();
		void VolumeUp();
		void VolumeDown();
		void FileInfoUpdate(const QString &fileName);
		void PlayStatusInfoUpdate(PlayStatus status);
		void ListShowOrHide();
		PlayStatus Status();
		void btnNextClick();
		void btnPrevClick();
		void btnSpeedClick();
		void speedActionClick();
		void btnSubClick();
		void VolumeSliderRelease();

	private:
		Ui::ToolWidget *ui;
		QMenu *speedMenu = nullptr;
		bool isSliderPress = false;
		bool isHide = false;
	};

}
