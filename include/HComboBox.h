#pragma once

#include <QComboBox>
#include <QLineEdit>
namespace HL
{
	class HComboBox : public QComboBox
	{
		Q_OBJECT
	public:
		explicit HComboBox(QWidget *parent = nullptr);
		void showPopup() override;

	private:
		QLineEdit *textHint = nullptr;
	};

}
