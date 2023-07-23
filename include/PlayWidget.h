#pragma once

#include <QWidget>
#include "DemuxThread.h"
#include "Util.h"
QT_BEGIN_NAMESPACE
namespace Ui
{
	class PlayWidget;
}
QT_END_NAMESPACE

namespace HL
{
	class PlayWidget : public QWidget
	{
		Q_OBJECT

	public:
		explicit PlayWidget(QWidget *parent = nullptr);
		~PlayWidget() override;
		PlayStatus Status() const;
		void setPlayStatus(PlayStatus sta);
		bool tryOpen(const QString &path);
		Ui::PlayWidget *Ui();
	public slots:
		void OpenFile();
		void PlayOrPause();
		void StopPlay();
	signals:
		// 文件更换
		void FileChanged(const QString &fileName);
		void StatusChanged(PlayStatus status);

	private:
		Ui::PlayWidget *ui;
		PlayStatus status = PlayStatus::STOP;
	};

}

