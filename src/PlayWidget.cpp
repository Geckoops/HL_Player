// You may need to build the project (run Qt uic code generator) to get "ui_PlayWidget.h" resolved

#include <QFileDialog>
#include <QDir>
#include <QMessageBox>
#include "RecordListItem.h"
#include "PlayWidget.h"
#include "ui_PlayWidget.h"
#include "ui_ToolWidget.h"
#include "ui_RecordListItem.h"
#include "RootWidget.h"
#include "Util.h"
namespace HL
{
	PlayWidget::PlayWidget(QWidget *parent) : QWidget(parent), ui(new Ui::PlayWidget)
	{
		ui->setupUi(this);
		ui->pwBottomLabel->setMouseTracking(true);
		ui->pwTopLabel->setMouseTracking(true);
		ui->pwVideoWidget->setMouseTracking(true);
	}

	PlayWidget::~PlayWidget()
	{
		delete ui;
	}
	void PlayWidget::OpenFile()
	{
		QString name = QFileDialog::getOpenFileName(this, tr("选择视频文件"), QDir::homePath(),
			tr("视频文件 (*.mp4 *.flv *.avi *.mov *.mkv *.ts *.mpg)"), 0, QFileDialog::DontUseNativeDialog);
		if (name.isEmpty())
			return;
		if (!tryOpen(name))
		{
			QMessageBox::information(nullptr, "error", "open file failed");
			return;
		}
		if (RecordListItem::getItemByName(name) != nullptr)
			return;
		RecordListItem::addItem(name);
		emit FileChanged(name);
		Global::dt->start();
		setPlayStatus(PlayStatus::PLAY);
	}
	void PlayWidget::setPlayStatus(PlayStatus sta)
	{
		status = sta;
		Global::dt->setStatus(status);
		emit StatusChanged(sta);
	}

	void PlayWidget::PlayOrPause()
	{
		//	if(!Global::dt->isOpen())
		//		return;
		switch (status)
		{
		case PlayStatus::STOP:
			// 停止时点击播放按钮，从播放历史中选择视频播放
			if (!Global::lastFilePath.isEmpty())
			{
				if (!tryOpen(Global::lastFilePath))
				{
					QMessageBox::information(nullptr, "警告", "文件丢失,已自动删除记录!");
					int index = 0;
					for (; index < RecordListItem::recordList.size(); index++)
						if (RecordListItem::recordList[index]->ui->fileName->fullText == Global::lastFilePath)
							break;
					if (!RecordListItem::recordList.empty() and index < RecordListItem::recordList.size())
					{
						auto temp = Global::tw->Ui()->twListWidget->item(index);
						Global::tw->Ui()->twListWidget->removeItemWidget(temp);
						delete temp;
					}
				}
				else
					emit Global::pw->FileChanged(Global::lastFilePath);
				//				DemuxThread *dt = new DemuxThread();
				//				if(dt->open(Global::lastFilePath.toUtf8(),Global::pw->Ui()->pwVideoWidget)) {
				//					Global::dt->close();
				//					emit Global::pw->FileChanged(Global::lastFilePath);
				//					dt->start();
				//					Global::dt = dt;
				//					Global::pw->setPlayStatus(PlayStatus::PLAY);
				//				}
				//				else
				//					delete dt;
			}
			break;
		case PlayStatus::PAUSE:
			setPlayStatus(PlayStatus::PLAY);
			break;
		case PlayStatus::PLAY:
			setPlayStatus(PlayStatus::PAUSE);
			break;
		}
	}
	Ui::PlayWidget *PlayWidget::Ui()
	{
		return ui;
	}
	void PlayWidget::StopPlay()
	{
		setPlayStatus(PlayStatus::STOP);
	}
	PlayStatus PlayWidget::Status() const
	{
		return status;
	}
	bool PlayWidget::tryOpen(const QString &path)
	{
		DemuxThread *dt = new DemuxThread();
		if (!dt->open(path.toUtf8(), ui->pwVideoWidget))
		{
			dt->close();
			return false;
		}
		else
		{
			Global::pw->setPlayStatus(PlayStatus::STOP);
			Global::dt = dt;
			emit FileChanged(path);
			dt->start();
			setPlayStatus(PlayStatus::PLAY);
			return true;
		}
	}

}