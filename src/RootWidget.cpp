// You may need to build the project (run Qt uic code generator) to get "ui_RootWidget.h" resolved

#include <QStackedLayout>
#include <QWindow>
#include "RootWidget.h"
#include "ui_RootWidget.h"
#include "ui_ToolWidget.h"
#include "ui_PlayWidget.h"
#include "RecordListItem.h"
namespace HL
{

	RootWidget::RootWidget(QWidget *parent) : QWidget(parent), ui(new Ui::RootWidget),
											  hideTimer(new QTimer(this)), playTimer(new QTimer(this)),
											  twSliderTimer(new QTimer(this))
	{
		// 加载设置信息
		auto settings = HL::Global::sqliteHelper->selectTableSettings();
		HL::Global::volume = settings.volume;
		HL::Global::playBackSpeed = settings.speed;
		HL::Global::lastFilePath = settings.previousFile;

		ui->setupUi(this);
		HL::Global::rw = this;
		Global::tw = tw = new ToolWidget(this);
		Global::pw = pw = new PlayWidget(this);
		Global::dt = &dt;

		BindAllEvent();
		pw->setPlayStatus(PlayStatus::STOP);
		QStackedLayout layout;
		layout.addWidget(tw);
		layout.addWidget(pw);
		layout.setStackingMode(QStackedLayout::StackAll);
		this->setLayout(&layout);
		this->setGeometry(0, 0, 1024, 720);
		tw->setGeometry(this->geometry());
		pw->setGeometry(this->geometry());
		normalGeo = this->geometry();
		pw->stackUnder(tw);
		this->grabKeyboard();
		// 去标题但无法拖动窗口，后续自定义拖动
		//	setWindowFlags(Qt::CustomizeWindowHint);

		playTimer->start(100);
		twSliderTimer->start(100);

		tw->Ui()->twSlider->hide();
		//	tw->Ui()->twRightGroup->hide();
		this->setWindowTitle("HL-Player");

		// 加载播放列表
		auto itemList = Global::sqliteHelper->selectTableListItem();
		for (auto item : itemList)
		{
			RecordListItem::addItem(item);
		}
	}

	RootWidget::~RootWidget()
	{
		delete ui;
	}
	void RootWidget::resizeEvent(QResizeEvent *evt)
	{
		auto [width, height] = this->size();
		pw->resize(width, height);
		tw->resize(width, height);
	}
	void RootWidget::mouseDoubleClickEvent(QMouseEvent *evt)
	{
		if (isMaximized())
		{
			ShowNormal();
		}
		else
		{
			ShowFullScreen();
			if (pw->Status() != PlayStatus::STOP)
				tw->Hide();
		}
	}
	void RootWidget::BindAllEvent()
	{
		// 播放暂停事件
		connect(tw->Ui()->tbtnPauseOrPlay, &QToolButton::clicked, pw, &PlayWidget::PlayOrPause);
		// 播放进度条事件
		connect(tw->Ui()->twSlider, &Slider::sliderPressed, tw, &ToolWidget::SliderPress);
		connect(tw->Ui()->twSlider, &Slider::sliderReleased, tw, &ToolWidget::SliderRelease);
		// 音量进度条事件
		connect(tw->Ui()->twVolumeSlider, &Slider::sliderPressed, tw, &ToolWidget::SliderPress);
		connect(tw->Ui()->twVolumeSlider, &Slider::sliderReleased, tw, &ToolWidget::VolumeSliderRelease);
		// 进度条刷新事件
		connect(twSliderTimer, &QTimer::timeout, tw, &ToolWidget::VideoSliderUpdate);
		// 打开文件事件
		connect(tw->Ui()->tbtnOpenFile, &QToolButton::clicked, pw, &PlayWidget::OpenFile);
		connect(tw->Ui()->tbtnOpenFile, &QToolButton::clicked, tw, &ToolWidget::SetTotalTime);

		// 隐藏工具页
		connect(hideTimer, &QTimer::timeout, tw, &ToolWidget::Hide);
		connect(playTimer, &QTimer::timeout, tw, &ToolWidget::SetCurrentTime);
		// 音量变化事件
		connect(tw->Ui()->twVolumeSlider, &QSlider::valueChanged, &dt, &DemuxThread::setVolume);
		// 设置文件相关控件信息
		connect(pw, &PlayWidget::FileChanged, tw, &ToolWidget::FileInfoUpdate);
		// 最小化窗口
		connect(tw->Ui()->tbtnMinimize, &QToolButton::clicked, this, &RootWidget::ShowMinimized);
		// 关闭窗口
		connect(tw->Ui()->tbtnClose, &QToolButton::clicked, this, &RootWidget::Close);
		// 全屏窗口
		connect(tw->Ui()->tbtnFullOrNormal, &QToolButton::clicked, this, &RootWidget::ShowFullScreenOrNormal);
		// 播放状态改变
		connect(pw, &PlayWidget::StatusChanged, tw, &ToolWidget::PlayStatusInfoUpdate);
		// 停止播放
		connect(tw->Ui()->twStop, &QToolButton::clicked, pw, &PlayWidget::StopPlay);
		// 播放完毕事件
		connect(Global::dt, &DemuxThread::finished, this, &RootWidget::finishedEvent);
		// 播放列表显示与隐藏事件
		connect(tw->Ui()->listBtn, &QPushButton::clicked, tw, &ToolWidget::ListShowOrHide);
		// 播放上一个事件
		connect(tw->Ui()->twPrev, &QToolButton::clicked, tw, &ToolWidget::btnPrevClick);
		// 播放下一个事件
		connect(tw->Ui()->twNext, &QToolButton::clicked, tw, &ToolWidget::btnNextClick);
		// 倍速按钮点击事件
		connect(tw->Ui()->btnSpeed, &QToolButton::clicked, tw, &ToolWidget::btnSpeedClick);
		// 字幕按鈕点击事件
		connect(tw->Ui()->subBtn, &QToolButton::clicked, tw, &ToolWidget::btnSubClick);
	}
	void RootWidget::mouseMoveEvent(QMouseEvent *event)
	{
		hideTimer->stop();
		hideTimer->start(static_cast<int>(Constant::HIDDENTIME));
		if (tw->isHidden())
			tw->Show();
	}
	void RootWidget::ShowFullScreen()
	{
		normalGeo = this->geometry();
		setWindowFlags(Qt::CustomizeWindowHint);
		this->showMaximized();
		if (pw->Status() == PlayStatus::PLAY)
		{
			pw->Ui()->pwTopLabel->hide();
			pw->Ui()->pwBottomLabel->hide();
		}
		tw->Ui()->tbtnFullOrNormal->setStyleSheet("border-image: url(:/icon/normalScreen.png);");
	}
	void RootWidget::ShowNormal()
	{
		setWindowFlags(Qt::Window |
					   Qt::WindowTitleHint |
					   Qt::WindowSystemMenuHint |
					   Qt::WindowMinMaxButtonsHint |
					   Qt::WindowCloseButtonHint |
					   Qt::WindowFullscreenButtonHint);
		this->setGeometry(normalGeo);
		this->show();
		this->showNormal();
		if (pw->Ui()->pwBottomLabel->isHidden())
			pw->Ui()->pwBottomLabel->show();
		if (pw->Ui()->pwTopLabel->isHidden())
			pw->Ui()->pwTopLabel->show();
		tw->Ui()->tbtnFullOrNormal->setStyleSheet("border-image: url(:/icon/fullScreen.png);");
	}
	void RootWidget::keyPressEvent(QKeyEvent *event)
	{
		if (event->isAutoRepeat())
			return;
		switch (event->key())
		{
		case Qt::Key_Space:
			pw->PlayOrPause();
			break;
		case Qt::Key_Right:
			tw->VideoStepForward();
			break;
		case Qt::Key_Left:
			tw->VideoStepBackward();
			break;
		case Qt::Key_Up:
			tw->VolumeUp();
			break;
		case Qt::Key_Down:
			tw->VolumeDown();
			break;
		}
	}
	ToolWidget *RootWidget::GetToolWidget() const
	{
		return tw;
	}
	PlayWidget *RootWidget::GetPlayWidget() const
	{
		return pw;
	}
	void RootWidget::ShowMinimized()
	{
		this->showMinimized();
	}
	void RootWidget::Close()
	{
		this->close();
	}
	void RootWidget::ShowFullScreenOrNormal()
	{
		if (this->isMaximized())
			ShowNormal();
		else
			ShowFullScreen();
	}
	QTimer *RootWidget::getHideTimer() const
	{
		return hideTimer;
	}
	void RootWidget::finishedEvent()
	{
		pw->setPlayStatus(PlayStatus::STOP);
		update();
	}
}