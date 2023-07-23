#pragma once

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

namespace HL
{
	struct ListItem
	{
		int id;
		QString fileName;
		QString filePath;
		QString subtitlePath;
		int subtitleIndex;
		int64_t position;
		ListItem() = default;
		ListItem(int _id, QString const &name, QString const &fpath, QString const &spath, int sIdx, int64_t pos)
			: id(_id), fileName(name), filePath(fpath), subtitlePath(spath), subtitleIndex(sIdx), position(pos) {}
	};

	struct Settings
	{
		int id;
		int volume;
		double speed;
		QString previousFile;
		Settings() = default;
		Settings(int _id, int vol, double sp, QString const &pf)
			: id(_id), volume(vol), speed(sp), previousFile(pf) {}
	};

	class SqliteHelper
	{
		QSqlDatabase database;

	public:
		SqliteHelper();
		~SqliteHelper();
		void open();
		void close();
		bool isOpen() const;
		QSqlQuery executeSql(QString const &sql);
		bool isTableExist(QString const &tableName);
		void insertTableListItem(ListItem data);
		void insertTableSettings(Settings data);
		void createTable();
		void clearTable(QString const &tableName);
		Settings selectTableSettings();
		std::vector<ListItem> selectTableListItem();
	};
}

