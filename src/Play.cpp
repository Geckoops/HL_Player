// You may need to build the project (run Qt uic code generator) to get "ui_Play.h" resolved

#include <QFileDialog>
#include "Play.h"
#include "ui_Play.h"
#include <QDebug>
#include "DemuxThread.h"
#include <QMessageBox>
#include <QWindow>

using namespace std::chrono_literals;
static HL::DemuxThread dt;

Play::Play(QWidget *parent) : QWidget(parent), ui(new Ui::Play)
{
	ui->setupUi(this);
	startTimer(40);
}

Play::~Play()
{
	dt.close();
	delete ui;
}

void Play::OpenFile()
{

	//	QString name = QFileDialog::getOpenFileName(nullptr,QString("选择视频文件"));
	QString name = QFileDialog::getOpenFileName(this, tr("选择视频文件"), QDir::homePath(),
		tr("视频文件 (*.mp4 *.flv *.avi *.mov *.mkv *.ts *.mpg)"), 0, QFileDialog::DontUseNativeDialog);
	if (name.isEmpty())
		return;
	this->setWindowTitle(name);
	if (!dt.open(name.toUtf8(), ui->VideoWindow))
	{
		QMessageBox::information(nullptr, "error", "open file failed");
		return;
	}
	dt.start();
	setPlayStatus(PlayStatus::PLAY);
}

void Play::timerEvent(QTimerEvent *evt)
{
	if (isSliderPress)
		return;
	int64_t length = dt.getTotalMs();
	if (length > 0)
	{
		double pos = 1.0 * dt.getPts() / length;
		pos = ui->slider->maximum() * pos;
		ui->slider->setValue(pos);
	}
}

void Play::resizeEvent(QResizeEvent *event)
{
	auto [width, height] = this->size();
	ui->slider->move(0, height - 30);
	ui->slider->resize(width, 20);
	ui->openFileBtn->move(0, height - 60);
	ui->statusBtn->move(ui->openFileBtn->x() + ui->openFileBtn->width() + 10,
						ui->openFileBtn->y());

	ui->VideoWindow->resize(this->size());
}

void Play::mouseDoubleClickEvent(QMouseEvent *evt)
{
	if (isFullScreen())
	{
		this->showNormal();
	}
	else
	{
		this->showFullScreen();
	}
}

void Play::setPlayStatus(PlayStatus sta)
{
	switch (sta)
	{
	case PlayStatus::PAUSE:
		ui->statusBtn->setText("播放");
		break;
	case PlayStatus::PLAY:
		ui->statusBtn->setText("暂停");
		break;
	}
	status = sta;
	//	dt.setStatus(status);
}

void Play::PlayOrPause()
{
	switch (status)
	{
	case PlayStatus::STOP:
	case PlayStatus::PAUSE:
		setPlayStatus(PlayStatus::PLAY);
		break;
	case PlayStatus::PLAY:
		setPlayStatus(PlayStatus::PAUSE);
		break;
	}
}

void Play::SliderPress()
{
	isSliderPress = true;
}

void Play::SliderRelease()
{
	isSliderPress = false;
	double pos = pos = 1.0 * ui->slider->value() / ui->slider->maximum();
	dt.Seek(pos);
}

bool Play::event(QEvent *e)
{
#if defined(Q_OS_WIN)
	if (e->type() == QEvent::WinIdChange)
	{
		if (windowHandle())
		{
			HWND handle = reinterpret_cast<HWND>(windowHandle()->winId());
			SetWindowLongPtr(handle, GWL_STYLE, GetWindowLongPtr(handle, GWL_STYLE) | WS_BORDER);
		}
	}
#endif
	return QWidget::event(e);
}
