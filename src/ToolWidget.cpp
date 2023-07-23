// You may need to build the project (run Qt uic code generator) to get "ui_ToolWidget.h" resolved

#include <QTimer>
#include <QDebug>
#include <QMessageBox>
#include <QMenu>
#include <QFileDialog>
#include <QDir>
#include "ToolWidget.h"
#include "VideoThread.h"
#include "ui_ToolWidget.h"
#include "ui_RecordListItem.h"
#include "RootWidget.h"
#include "RecordListItem.h"
namespace HL
{
	ToolWidget::ToolWidget(QWidget *parent) : QWidget(parent), ui(new Ui::ToolWidget)
	{
		ui->setupUi(this);
		initSpeedMenu();
		ui->twTopGroup->setMouseTracking(true);
		ui->twBottomGroup->setMouseTracking(true);
		ui->twBlankLabel->setMouseTracking(true);
		ui->twCurrentTime->hide();
		ui->twTotalTime->hide();
		ui->twRightGroup->hide();
		ui->twVolumeSlider->setValue(100);
		ui->twListWidget->setDragEnabled(true);
		ui->btnSpeed->setText(QString::number(Global::playBackSpeed, 'g', 3) + 'x');
		ui->twVolumeSlider->setValue(Global::volume);
	}

	ToolWidget::~ToolWidget()
	{
		delete ui;
	}
	void ToolWidget::timerEvent(QTimerEvent *evt)
	{
		if (isSliderPress)
			return;
		int64_t length = Global::dt->getTotalMs();
		if (length > 0)
		{
			double pos = 1.0 * Global::dt->getPts() / length;
			pos = ui->twSlider->maximum() * pos;
			ui->twSlider->setValue(pos);
		}
	}
	void ToolWidget::SliderPress()
	{
		isSliderPress = true;
	}
	void ToolWidget::SliderRelease()
	{
		isSliderPress = false;
		double pos = pos = 1.0 * ui->twSlider->value() / ui->twSlider->maximum();
		Global::dt->Seek(pos);
	}
	void ToolWidget::VolumeSliderRelease()
	{
		isSliderPress = false;
		Global::dt->setVolume(ui->twVolumeSlider->value());
	}
	Ui::ToolWidget *ToolWidget::Ui()
	{
		return ui;
	}
	void ToolWidget::Show()
	{
		if (!isHide)
			return;
		this->show();
		isHide = false;
	}
	void ToolWidget::Hide()
	{
		if (isHide or Status() == PlayStatus::STOP)
			return;

		this->hide();
		isHide = true;
	}

	void ToolWidget::SetTotalTime()
	{
		auto [h, m, s] = Util::ms2hms(Global::dt->getTotalMs());

		QString str = QString("/ %1:%2:%3")
						  .arg(h, 2, 10, QLatin1Char('0'))
						  .arg(m, 2, 10, QLatin1Char('0'))
						  .arg(s, 2, 10, QLatin1Char('0'));
		ui->twTotalTime->setText(str);
		ui->twCurrentTime->setText("00:00:00");
		if (Global::dt->isOpen())
		{
			ui->twTotalTime->show();
			ui->twCurrentTime->show();
		}
	}
	void ToolWidget::SetCurrentTime()
	{
		auto [h, m, s] = Util::ms2hms(Global::dt->getPts());
		QString str = QString("%1:%2:%3")
						  .arg(h, 2, 10, QLatin1Char('0'))
						  .arg(m, 2, 10, QLatin1Char('0'))
						  .arg(s, 2, 10, QLatin1Char('0'));
		ui->twCurrentTime->setText(str);
	}
	void ToolWidget::VideoSliderUpdate()
	{
		if (!Global::dt->isOpen() or isSliderPress)
			return;
		ui->twSlider->setValue(ui->twSlider->maximum() * 1.0 * Global::dt->getPts() / Global::dt->getTotalMs());
	}
	void ToolWidget::VideoStepForward()
	{
		if (!Global::dt->isOpen())
			return;
		double pos = (static_cast<double>(Constant::STEPMSCOUNT) + Global::dt->getPts()) / Global::dt->getTotalMs();
		pos = std::min(pos, 1.0);
		std::cerr << pos << std::endl;
		Global::dt->Seek(pos);
	}
	void ToolWidget::VideoStepBackward()
	{
		if (!Global::dt->isOpen())
			return;
		double pos = (Global::dt->getPts() - static_cast<double>(Constant::STEPMSCOUNT)) / Global::dt->getTotalMs();
		pos = std::max(0.0, pos);
		std::cerr << pos << std::endl;
		Global::dt->Seek(pos);
	}
	void ToolWidget::VolumeUp()
	{
		int val = ui->twVolumeSlider->value() + 5;
		val = std::min(val, ui->twVolumeSlider->maximum());
		ui->twVolumeSlider->setValue(val);
		const AudioThread *at = Global::dt->getAudioThread();
		double volume = 1.0 * val / ui->twVolumeSlider->maximum();
		at->getAudioPlay()->setVolume(volume);
	}
	void ToolWidget::VolumeDown()
	{
		int val = ui->twVolumeSlider->value() - 5;
		val = std::max(val, ui->twVolumeSlider->minimum());
		ui->twVolumeSlider->setValue(val);
		const AudioThread *at = Global::dt->getAudioThread();
		double volume = 1.0 * val / ui->twVolumeSlider->maximum();
		at->getAudioPlay()->setVolume(volume);
	}
	void ToolWidget::FileInfoUpdate(const QString &fileName)
	{
		if (!Global::currentFilePath.isEmpty())
			Global::lastFilePath = Global::currentFilePath;
		Global::currentFilePath = fileName;
		Global::subManager.filePath = fileName;
		Global::subManager.enable = true;
		ui->twFileName->setText(fileName);
		SetTotalTime();
		for (auto item : RecordListItem::recordList)
		{
			if (fileName == item->ui->fileName->fullText)
				item->ui->fileName->setStyleSheet("color: rgb(51,154,240);\n"
												  "font-size: 14px;\n"
												  "font-family: \"Microsoft YaHei\"; ");
			else
				item->ui->fileName->setStyleSheet("color: rgb(177,177,177);\n"
												  "font-size: 14px;\n"
												  "font-family: \"Microsoft YaHei\"; ");
		}
		Global::rw->update();
		connect(Global::dt, &DemuxThread::finished, Global::rw, &RootWidget::finishedEvent);
	}
	void ToolWidget::PlayStatusInfoUpdate(PlayStatus status)
	{
		switch (status)
		{
		case PlayStatus::PLAY:
			ui->tbtnPauseOrPlay->setStyleSheet("\n"
											   "QToolButton:!hover {\n"
											   "\t\n"
											   "\tborder-image: url(:/icon/pause.png);\n"
											   "}\n"
											   "QToolButton:hover{\n"
											   "\t\n"
											   "\tborder-image: url(:/icon/pause_hover.png);\n"
											   "}");
			ui->twBlankLabel->setStyleSheet("");
			ui->twSlider->show();
			ui->twCurrentTime->show();
			ui->twTotalTime->show();
			break;
		case PlayStatus::PAUSE:
			ui->tbtnPauseOrPlay->setStyleSheet("\n"
											   "QToolButton:!hover {\n"
											   "\t\n"
											   "\tborder-image: url(:/icon/play.png);\n"
											   "}\n"
											   "QToolButton:hover{\n"
											   "\t\n"
											   "\tborder-image: url(:/icon/play_hover.png);\n"
											   "}");
			ui->twBlankLabel->setStyleSheet("");
			ui->twSlider->show();
			qobject_cast<RootWidget *>(this->parent())->getHideTimer()->start(static_cast<int>(Constant::HIDDENTIME));
			break;
		case PlayStatus::STOP:
			ui->twBlankLabel->setStyleSheet("border-image: url(:/icon/logo.png);");
			ui->twCurrentTime->hide();
			ui->twTotalTime->hide();
			FileInfoUpdate("");
			ui->tbtnPauseOrPlay->setStyleSheet("\n"
											   "QToolButton:!hover {\n"
											   "\t\n"
											   "\tborder-image: url(:/icon/play.png);\n"
											   "}\n"
											   "QToolButton:hover{\n"
											   "\t\n"
											   "\tborder-image: url(:/icon/play_hover.png);\n"
											   "}");
			ui->twSlider->hide();
			Global::rw->getHideTimer()->stop();
			// 停止播放线程
			if (Global::dt->isOpen())
				Global::dt->close();
			break;
		}
	}
	void ToolWidget::resizeEvent(QResizeEvent *event)
	{
		ui->twFileName->setText(ui->twFileName->originText);
		QWidget::resizeEvent(event);
	}
	PlayStatus ToolWidget::Status()
	{
		return qobject_cast<RootWidget *>(this->parent())->GetPlayWidget()->Status();
	}
	void ToolWidget::ListShowOrHide()
	{
		if (ui->twRightGroup->isHidden())
		{
			ui->twRightGroup->show();
		}
		else
		{
			ui->twRightGroup->hide();
		}
	}
	void ToolWidget::btnNextClick()
	{
		if (Global::pw->Status() == PlayStatus::STOP)
			return;
		QString &curPath = ui->twFileName->fullText;
		int index = 0;
		for (; index < RecordListItem::recordList.size(); index++)
			if (RecordListItem::recordList[index]->ui->fileName->fullText == curPath)
				break;
		if (index + 1 == RecordListItem::recordList.size())
			return;
		QString &nextPath = RecordListItem::recordList[index + 1]->ui->fileName->fullText;
		if (!Global::pw->tryOpen(nextPath))
		{
			QMessageBox::information(nullptr, "警告", "文件丢失,已自动删除记录");
			// 删除这个丢失的记录
			auto temp = ui->twListWidget->item(index + 1);
			Global::tw->Ui()->twListWidget->removeItemWidget(temp);
			delete temp;
		}
	}
	void ToolWidget::btnPrevClick()
	{
		if (Global::pw->Status() == PlayStatus::STOP)
			return;
		QString &curPath = ui->twFileName->fullText;
		int index = 0;
		for (; index < RecordListItem::recordList.size(); index++)
			if (RecordListItem::recordList[index]->ui->fileName->fullText == curPath)
				break;
		if (index == 0)
			return;
		QString &prevPath = RecordListItem::recordList[index - 1]->ui->fileName->fullText;
		if (!Global::pw->tryOpen(prevPath))
		{
			QMessageBox::information(nullptr, "警告", "文件丢失,已自动删除记录");
			// 删除这个丢失的记录
			auto temp = ui->twListWidget->item(index - 1);
			Global::tw->Ui()->twListWidget->removeItemWidget(temp);
			delete temp;
		}
	}
	void ToolWidget::btnSpeedClick()
	{
		ui->btnSpeed->menu()->exec({QCursor::pos().x(), QCursor::pos().y() - ui->btnSpeed->menu()->height()});
	}
	void ToolWidget::initSpeedMenu()
	{
		if (speedMenu == nullptr)
			speedMenu = new QMenu(this);
		for (int i = 0; i < 7; i++)
		{
			double speed = 0.5 + i * 0.25;
			QAction *act = new QAction(QString::number(speed, 'g', 3) + 'x', speedMenu);
			speedMenu->addAction(act);
			connect(act, &QAction::triggered, this, &ToolWidget::speedActionClick);
		}
		ui->btnSpeed->setMenu(speedMenu);
	}
	void ToolWidget::speedActionClick()
	{
		QString text = qobject_cast<QAction *>(sender())->text();
		ui->btnSpeed->setText(QStringLiteral("倍速"));
		double speed = 1.0;
		if (text != QStringLiteral("1x"))
		{
			ui->btnSpeed->setText(text);
			text.chop(1);
			speed = text.toDouble();
		}
		Global::dt->getAudioThread()->setSpeed(speed);
	}

	void ToolWidget::btnSubClick()
	{
		auto setFilter = [](QString path = "", int index = -1)
		{
			auto oldStatus = Global::pw->Status();
			Global::pw->setPlayStatus(PlayStatus::PAUSE);
			Global::dt->getVideoThread()->init_filter(path, index);
			Global::pw->setPlayStatus(oldStatus);
		};
		QMenu *org = ui->subBtn->menu();
		if (org != nullptr)
			delete org;
		QMenu *menu = new QMenu();
		// 打开/关闭字幕
		QString title;
		if (Global::subManager.enable)
			title = "关闭字幕";
		else
			title = "打开字幕";
		QAction *act = new QAction(title, menu);
		connect(act, &QAction::triggered, this, [this, setFilter]()
				{
					auto act = qobject_cast<QAction *>(sender());
					if (Global::subManager.enable)
						act->setText("打开字幕");
					else
						act->setText("关闭字幕");
					Global::subManager.enable ^= 1;
					if (Global::subManager.enable)
					{
						int idx = Global::subManager.curIndex;
						setFilter(Global::subManager.subList[idx].path, Global::subManager.subList[idx].index);
					}
					else
						setFilter(); });
		menu->addAction(act);
		// 加载外挂字幕
		act = new QAction("其他字幕", menu);
		connect(act, &QAction::triggered, this, [this, setFilter]()
				{
	  QString path = QFileDialog::getOpenFileName(this,tr("选择文件"),QDir::homePath(),
												  tr("*.ass"),nullptr,
												  QFileDialog::DontUseNativeDialog);
	  // 检查字幕文件
	  if(path == nullptr or path.isEmpty() or Global::pw->Status() == PlayStatus::STOP)
		  return;
	  auto iter = std::find_if(Global::subManager.subList.begin(), Global::subManager.subList.end(),[&path](SubInfo item) {
		  return item.path == path and item.index == 0 and item.title == "";
	  });
	  if(iter == Global::subManager.subList.end()) {
		  Global::subManager.subList.emplace_back("",path, 0);
		  Global::subManager.curIndex = Global::subManager.subList.size() - 1;
	  }
	  else {
		  Global::subManager.curIndex = iter - Global::subManager.subList.begin();
	  }
	  int idx = Global::subManager.curIndex;
	  setFilter(Global::subManager.subList[idx].path, Global::subManager.subList[idx].index);
	  RecordListItem::setSubTitle(Global::tw->ui->twFileName->fullText, path, 0); });
		menu->addAction(act);
		// 内封字幕点击
		for (auto [title, _, index] : Global::subManager.subList)
		{
			if (title == "" and index == 0)
				continue;
			act = new QAction(title, menu);
			menu->addAction(act);
			connect(act, &QAction::triggered, this, [this, setFilter]()
					{
		  QString text = qobject_cast<QAction*>(sender())->text();
		  int idx = -1;
		  for(int i=0;i<Global::subManager.subList.size();i++) {
			  if(Global::subManager.subList[i].title == text) {
				  idx = Global::subManager.curIndex = i;
				  break;
			  }
		  }
		  if(idx != -1) {
			  setFilter(Global::subManager.subList[idx].path, Global::subManager.subList[idx].index);
			  RecordListItem::setSubTitle(Global::tw->ui->twFileName->fullText,
										  Global::subManager.subList[idx].path,
										  Global::subManager.subList[idx].index);
		  }
		  else
			  setFilter(); });
		}

		ui->subBtn->setMenu(menu);
		//	ui->subBtn->menu()->exec({QCursor::pos().x(), QCursor::pos().y() - ui->subBtn->menu()->height()});
		ui->subBtn->menu()->exec({QCursor::pos().x(),
								  QCursor::pos().y()});
	}

}