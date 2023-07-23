#include <QApplication>
#include "HComboBox.h"

namespace HL
{
	void HComboBox::showPopup()
	{
		QComboBox::showPopup();
		QWidget *popup = this->findChild<QFrame *>();
		popup->move(popup->x(), popup->y() - this->height() - popup->height());
	}

	HComboBox::HComboBox(QWidget *parent) : QComboBox(parent)
	{
		QApplication::setEffectEnabled(Qt::UI_AnimateCombo, false);
		//	textHint = new QLineEdit(this);
		//	textHint->setStyleSheet("qproperty-alignment:AlignHCenter;border:none;font-size:26px;font-weight:bold;color:#ffffff;background:transparent;");//
		//	textHint->setPlaceholderText("选择字幕");
		//	textHint->setReadOnly(true);
		//	textHint->setFocusPolicy(Qt::NoFocus);
		//	textHint->setEnabled(false);
		//	this->setLineEdit(textHint);
		qDebug() << this->lineEdit() << "111";
		this->setPlaceholderText("字幕");
	}
} // HL