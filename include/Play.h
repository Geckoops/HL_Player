#pragma once

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui
{
	class Play;
}
QT_END_NAMESPACE

enum class PlayStatus
{
	PAUSE,
	PLAY,
	STOP
};

class Play : public QWidget
{
	Q_OBJECT
public:
	explicit Play(QWidget *parent = nullptr);
	~Play() override;
	// 定时器，进度条显示
	void timerEvent(QTimerEvent *evt) override;
	// 窗口尺寸变化
	void resizeEvent(QResizeEvent *evt) override;
	// 双击全屏
	void mouseDoubleClickEvent(QMouseEvent *evt) override;
	// 设置播放状态
	void setPlayStatus(PlayStatus sta);

protected:
	bool event(QEvent *event) override;
public slots:
	void OpenFile();
	void PlayOrPause();
	void SliderPress();
	void SliderRelease();

private:
	Ui::Play *ui;
	std::mutex mtx;
	PlayStatus status = PlayStatus::STOP;
	bool isSliderPress = false;
};

