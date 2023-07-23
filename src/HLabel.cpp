#include "HLabel.h"
#include <QGroupBox>
HLabel::HLabel(QWidget *parent) : QLabel(parent)
{
}
void HLabel::setText(const QString &text)
{
	fullText = text;
	originText = text.sliced(text.lastIndexOf('/') + 1);
	QFontMetrics fontWidth(this->font()); // 得到每个字符的宽度
	QWidget *fa = qobject_cast<QWidget *>(this->parent());
	int width = fa->width() - 200;
	QString elideNote = fontWidth.elidedText(originText, Qt::ElideRight, std::min(width, 600)); // 最大宽度150像素
	QLabel::setText(elideNote);																	// 显示省略好的字符串
	this->setToolTip(text);																		// 设置tooltips
}
