#include <iostream>
#include <chrono>
#include <thread>
#include <fstream>
#include <QVideoWidget>
#include <QThread>
#include <QApplication>
#include <QDebug>
#include <QMediaPlayer>
// #include <QtMultimedia>
#include "Util.h"
#include "RecordListItem.h"
#include "Demux.h"
#include "Decode.h"
#include "Play.h"
#include "ui_Play.h"
#include "Resample.h"
#include "Util.h"
#include "QtAudioPlay.h"
#include "AudioThread.h"
#include "VideoThread.h"
#include "DemuxThread.h"
#include "RootWidget.h"
extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
}
#include <bitset>
#include "ui_ToolWidget.h"
#include "ui_RecordListItem.h"
#include "RecordListItem.h"
using namespace std::literals;

int main(int argc, char *argv[])
{
	qDebug() << "main ThreadID" << QThread::currentThread()->currentThreadId();
	QApplication a(argc, argv);

	HL::Global::sqliteHelper = new HL::SqliteHelper();

	HL::RootWidget rw;
	QScreen *scr = a.primaryScreen();
	int scr_w = scr->size().width();
	int scr_h = scr->size().height();
	rw.move((scr_w - rw.width()) / 2, (scr_h - rw.height()) / 2);
	rw.show();

	qDebug() << rw.windowFlags();
	QObject::connect(&a, &QApplication::aboutToQuit, [&]()
					 {
	  qDebug() << "程序退出";
	  // 保存设置
	  qDebug() << HL::Global::volume;
	  qDebug() << HL::Global::playBackSpeed;
	  qDebug() << HL::Global::lastFilePath;
	  auto volume = HL::Global::volume;
	  auto speed = HL::Global::playBackSpeed;
	  auto lastFilePath = HL::Global::lastFilePath;
	  HL::Global::sqliteHelper->clearTable("t_settings");
	  HL::Global::sqliteHelper->insertTableSettings({0, volume, speed, lastFilePath});
	  // 保存播放列表
	  HL::Global::sqliteHelper->clearTable("t_ListItem");
	  for(auto item : HL::RecordListItem::recordList) {
		  qDebug() << item->fileName << item->path << item->subIndex << item->subPath;
			HL::Global::sqliteHelper->insertTableListItem({
				0,
				item->fileName,
				item->path,
				item->subPath,
				item->subIndex,
				item->pts
			});
	  } });
	return QApplication::exec();
}
