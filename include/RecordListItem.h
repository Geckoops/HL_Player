#pragma once

#include <QWidget>

class QListWidget;

QT_BEGIN_NAMESPACE
namespace Ui
{
	class RecordListItem;
}
QT_END_NAMESPACE

namespace HL
{
	struct ListItem;
	class RecordListItem : public QWidget
	{
		Q_OBJECT
	public:
		// 用于切换下一个，只做了声明还没有实现猜测vector内的顺序与ListWidget里的顺序一致，待验证
		inline static std::vector<RecordListItem *> recordList;
		explicit RecordListItem(QWidget *parent = nullptr);
		~RecordListItem() override;
		static RecordListItem *getItemByName(const QString &path);
		static void addItem(const QString &path);
		static void setSubTitle(const QString &path, const QString &sPath, int sIndex);
		static void setSubPts(const QString &path, int64_t _pts);
		static QListWidget *listWidget();
		static void addItem(const ListItem &item);

	protected:
		void mouseMoveEvent(QMouseEvent *event) override;
		void mouseReleaseEvent(QMouseEvent *event) override;
		void enterEvent(QEnterEvent *event) override;
		void leaveEvent(QEvent *event) override;

	signals:
		void removeBtnClicked();
		void playBtnClicked();

	public:
		// private:
		Ui::RecordListItem *ui;
		int index = -1;
		QString path;
		QString subPath;
		QString fileName;
		int subIndex = -1;
		int64_t pts = 0;
	};

}

