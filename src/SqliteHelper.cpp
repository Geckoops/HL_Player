#include "SqliteHelper.h"
namespace HL
{
	SqliteHelper::SqliteHelper()
	{
		if (QSqlDatabase::contains("qt_sql_default_connection"))
			database = QSqlDatabase::database("qt_sql_default_connection");
		else
		{
			// 建立和SQlite数据库的连接
			database = QSqlDatabase::addDatabase("QSQLITE");
			// 设置数据库文件的名字
			database.setDatabaseName("../res/HL_Player.sqlite");
		}
		open();
		createTable();
	}

	void SqliteHelper::open()
	{
		if (!database.isOpen())
			database.open();
	}

	void SqliteHelper::close()
	{
		if (database.isOpen())
			database.close();
	}

	QSqlQuery SqliteHelper::executeSql(const QString &sql)
	{
		if (!isOpen())
			open();
		QSqlQuery sqlQuery;
		sqlQuery.prepare(sql);
		sqlQuery.exec();
		return sqlQuery;
	}

	bool SqliteHelper::isOpen() const
	{
		return database.isOpen();
	}

	// 判断数据库中某个数据表是否存在
	bool SqliteHelper::isTableExist(QString const &tableName)
	{
		if (database.tables().contains(tableName))
		{
			return true;
		}

		return false;
	}

	SqliteHelper::~SqliteHelper()
	{
		database.close();
	}

	void SqliteHelper::createTable() {
		QSqlQuery query;

		query.exec("CREATE TABLE IF NOT EXISTS t_ListItem ("
			"id INTEGER PRIMARY KEY AUTOINCREMENT, "
			"fileName TEXT, "
			"filePath TEXT, "
			"subtitlePath TEXT, "
			"subtitleIndex INTEGER, "
			"position INTEGER"
			")");

		query.exec("CREATE TABLE IF NOT EXISTS t_Settings ("
			"id INTEGER PRIMARY KEY AUTOINCREMENT, "
			"volume INTEGER, "
			"speed REAL, "
			"previousFile TEXT"
			")");
	}

	void SqliteHelper::insertTableListItem(ListItem data)
	{
		QString sql{"insert into t_ListItem values (null,?,?,?,?,?)"};
		QSqlQuery sql_query;
		sql_query.prepare(sql);
		sql_query.addBindValue(data.fileName);
		sql_query.addBindValue(data.filePath);
		sql_query.addBindValue(data.subtitlePath);
		sql_query.addBindValue(data.subtitleIndex);
		sql_query.addBindValue(data.position);
		sql_query.exec();
	}

	void SqliteHelper::insertTableSettings(Settings data)
	{
		QString sql{"insert into t_Settings values (null,?,?,?)"};
		QSqlQuery sql_query;
		sql_query.prepare(sql);
		sql_query.addBindValue(data.volume);
		sql_query.addBindValue(data.speed);
		sql_query.addBindValue(data.previousFile);
		sql_query.exec();
	}

	void SqliteHelper::clearTable(const QString &tableName)
	{
		QString sql = QString("delete from %1").arg(tableName);
		QSqlQuery sql_query;
		sql_query.prepare(sql);
		sql_query.exec();
		sql = "update sqlite_sequence set seq = 0 where name = ?";
		sql_query.prepare(sql);
		sql_query.addBindValue(tableName);
		sql_query.exec();
	}

	Settings SqliteHelper::selectTableSettings()
	{
		QString sql = QString("select * from t_settings");
		QSqlQuery sql_query;
		sql_query.prepare(sql);
		Settings ret{1,100,1.0,""};
		if (!sql_query.exec())
			return ret;
		if (sql_query.next())
		{
			ret.id = sql_query.value(0).toInt();
			ret.volume = sql_query.value(1).toInt();
			ret.speed = sql_query.value(2).toDouble();
			ret.previousFile = sql_query.value(3).toString();
		}
		return ret;
	}

	std::vector<ListItem> SqliteHelper::selectTableListItem()
	{
		QString sql = QString("select * from t_ListItem");
		QSqlQuery sql_query;
		sql_query.prepare(sql);
		sql_query.exec();
		std::vector<ListItem> ret;
		while (sql_query.next())
		{
			ListItem temp;
			temp.id = sql_query.value(0).toInt();
			temp.fileName = sql_query.value(1).toString();
			temp.filePath = sql_query.value(2).toString();
			temp.subtitlePath = sql_query.value(3).toString();
			temp.subtitleIndex = sql_query.value(4).toInt();
			temp.position = sql_query.value(5).toLongLong();
			ret.push_back(temp);
		}
		return ret;
	}
}
