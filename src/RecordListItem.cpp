// You may need to build the project (run Qt uic code generator) to get "ui_RecordListItem.h" resolved

#include "RecordListItem.h"
#include "ui_RecordListItem.h"
#include "ui_PlayWidget.h"
#include "ui_ToolWidget.h"
#include "RootWidget.h"
#include "HLabel.h"
#include <QMouseEvent>
#include <QListWidget>
#include <QMessageBox>
#include <QThread>
namespace HL
{
	RecordListItem::RecordListItem(QWidget *parent) : QWidget(parent), ui(new Ui::RecordListItem)
	{
		this->index = recordList.size();
		recordList.push_back(this);
		ui->setupUi(this);
		ui->btnRemove->setAttribute(Qt::WA_TransparentForMouseEvents, true);
		ui->btnPlay->setAttribute(Qt::WA_TransparentForMouseEvents, true);
		ui->btnPlay->hide();
		ui->btnRemove->hide();
		this->setMouseTracking(true);
	}

	RecordListItem::~RecordListItem()
	{
		int target = index;
		auto it = std::find_if(recordList.begin(), recordList.end(), [&](RecordListItem *item)
							   { return item->index == target; });
		recordList.erase(it);
		for (int i = 0; i < recordList.size(); i++)
			recordList[i]->index = i;
		delete ui;
	}

	void RecordListItem::enterEvent(QEnterEvent *event)
	{
		ui->btnPlay->show();
		ui->btnRemove->show();
		QWidget::enterEvent(event);
	}

	void RecordListItem::leaveEvent(QEvent *event)
	{
		ui->btnPlay->hide();
		ui->btnRemove->hide();
		QWidget::leaveEvent(event);
	}

	// hover事件
	void RecordListItem::mouseMoveEvent(QMouseEvent *event)
	{
		//	qDebug() << ui->btnPlay->geometry().contains(event->pos());
		if (ui->btnRemove->geometry().contains(event->pos()))
		{
			ui->btnRemove->setStyleSheet("QLabel {\t\n"
										 "border-image: url(:/icon/itemRemove_hover.png);\n"
										 "}");
		}
		else
		{
			ui->btnRemove->setStyleSheet("QLabel {\t\n"
										 "border-image: url(:/icon/itemRemove.png);\n"
										 "}");
		}

		if (ui->btnPlay->geometry().contains(event->pos()))
		{
			ui->btnPlay->setStyleSheet("QLabel {\n"
									   "border-image: url(:/icon/itemPlay_hover.png);\n"
									   "}");
		}
		else
		{
			ui->btnPlay->setStyleSheet("QLabel {\n"
									   "border-image: url(:/icon/itemPlay.png);\n"
									   "}");
		}
	}

	// 使用press事件的话，切换当前行在后，必须是release事件
	void RecordListItem::mouseReleaseEvent(QMouseEvent *event)
	{
		//	qDebug() << this->parent()->parent()->metaObject()->className();
		// 删除按钮点击
		if (ui->btnRemove->geometry().contains(event->pos()))
		{
			//	emit removeBtnClicked();
			auto lw = listWidget();
			auto item = lw->currentItem();
			auto record = qobject_cast<RecordListItem *>(lw->itemWidget(item));
			// 如果删除了当前正在播的视频，切换到下一个视频，待添加
			if (Global::tw->Ui()->twFileName->fullText != record->ui->fileName->fullText)
			{
				listWidget()->removeItemWidget(item);
				delete item;
				return;
			}
			auto iter = std::find_if(recordList.begin(), recordList.end(), [&](RecordListItem *item)
									 { return item->index == record->index; });
			iter++;
			if (iter == recordList.end())
			{
				// 已经是最后一个，根据具体情况选择设置停止或者循环播放
				// 暂定为暂停
				Global::pw->setPlayStatus(PlayStatus::STOP);
			}
			else
			{
				// 关闭当前播放媒体
				Global::pw->setPlayStatus(PlayStatus::STOP);
				// 播放iter指向的这一个
				RecordListItem *next = *iter;
				QString name = next->ui->fileName->fullText;
				if (!Global::pw->tryOpen(name.toUtf8()))
				{
					QMessageBox::information(nullptr, "警告", "文件丢失,已自动删除记录!");
					auto temp = lw->item(next->index);
					Global::tw->Ui()->twListWidget->removeItemWidget(temp);
					delete temp;
				}
			}
			lw->removeItemWidget(item);
			delete item;
		}
		// 播放按钮点击
		if (ui->btnPlay->geometry().contains(event->pos()))
		{
			//	emit playBtnClicked();
			auto curIndex = Global::tw->Ui()->twListWidget->currentRow();
			auto curItem = recordList[curIndex];
			QString name = curItem->ui->fileName->fullText;
			if (!Global::pw->tryOpen(name))
			{
				QMessageBox::information(nullptr, "警告", "文件丢失,已自动删除记录!");
				auto temp = Global::tw->Ui()->twListWidget->currentItem();
				Global::tw->Ui()->twListWidget->removeItemWidget(temp);
				delete temp;
			}
			else
			{
			}
		}
		QWidget::mouseReleaseEvent(event);
	}

	RecordListItem *RecordListItem::getItemByName(const QString &path)
	{
		for (auto ptr : recordList)
			if (ptr->ui->fileName->fullText == path)
				return ptr;
		return nullptr;
	}

	void RecordListItem::addItem(const QString &path)
	{
		RecordListItem *item = new RecordListItem();
		item->ui->fileName->setText(path);
		item->path = path;
		item->fileName = item->ui->fileName->originText;
		auto ListWidget = listWidget();
		ListWidget->addItem(new QListWidgetItem);
		ListWidget->setItemWidget(ListWidget->item(ListWidget->count() - 1), item);
	}

	QListWidget *RecordListItem::listWidget()
	{
		return Global::tw->Ui()->twListWidget;
	}

	void RecordListItem::setSubTitle(const QString &path, const QString &sPath, int sIndex)
	{
		auto item = getItemByName(path);
		if (item == nullptr)
			return;
		item->subPath = sPath;
		item->subIndex = sIndex;
		item->fileName = item->ui->fileName->originText;
	}

	void RecordListItem::setSubPts(const QString &path, int64_t _pts)
	{
		auto item = getItemByName(path);
		if (item == nullptr)
			return;
		item->pts = _pts;
	}

	void RecordListItem::addItem(const ListItem &data)
	{
		RecordListItem *item = new RecordListItem();
		item->ui->fileName->setText(data.filePath);
		item->path = data.filePath;
		item->fileName = data.fileName;
		item->pts = data.position;
		item->subPath = data.subtitlePath;
		item->subIndex = data.subtitleIndex;
		auto ListWidget = listWidget();
		ListWidget->addItem(new QListWidgetItem);
		ListWidget->setItemWidget(ListWidget->item(ListWidget->count() - 1), item);
	}
}