#pragma once

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui
{
	class TopWidget;
}
QT_END_NAMESPACE

namespace HL
{
	class TopWidget : public QWidget
	{
		Q_OBJECT

	public:
		explicit TopWidget(QWidget *parent = nullptr);
		~TopWidget() override;

	protected:
		void resizeEvent(QResizeEvent *event) override;

	private:
		Ui::TopWidget *ui;
	};

}
