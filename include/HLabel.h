#pragma once

#include <QLabel>

class HLabel : public QLabel
{
	Q_OBJECT
public:
	HLabel(QWidget *parent = nullptr);
	void setText(const QString &text);
	QString fullText;
	QString originText;
};

