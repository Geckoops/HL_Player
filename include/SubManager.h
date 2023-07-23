#pragma once

#include <QString>
#include <QFile>
#include <QDebug>
namespace HL
{
	struct SubInfo
	{
		QString title;
		QString path;
		int index;
		SubInfo() = default;
		SubInfo(QString const &t, QString const &p, int idx) : title(t), path(p), index(idx) {}
	};
	struct SubManager
	{
		bool enable = true;
		QString filePath = "";
		int currentSubStreamIndex = -1;
		std::vector<int> subStreamIndexList;
		std::vector<SubInfo> subList;
		QFile fs;
		int curIndex = -1;
		SubManager()
		{
			fs.setFileName("Log.txt");
		}
		void loadMetadata(QString const &path)
		{
			subList.clear();
			curIndex = -1;
			if (!fs.isOpen())
				fs.open(QFile::ReadOnly | QFile::Text);
			char buf[256];
			int idx = 0;
			while (fs.readLine(buf, 256) > 0)
			{
				QString str{buf};
				if (str.contains(": Subtitle:"))
				{
					fs.readLine(buf, 256);
					fs.readLine(buf, 256);
					str = QString{buf};
					subList.emplace_back(
						str.right(str.length() - (str.indexOf(": ") + 2)),
						path,
						idx++);
				}
			}
		}
		void clearFile()
		{
			return;
			// 从launcher启动时无法清除文件内容，文件也无法删除，改为不清空接着往下读
			if (fs.isOpen())
				fs.close();
			QFile::remove("Log.txt");
			fs.open(QFile::WriteOnly | QFile::Text);
			fs.close();
		}
	};

}