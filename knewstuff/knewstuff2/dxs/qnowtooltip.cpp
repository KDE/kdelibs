#include "qnowtooltip.h"

#include <qlabel.h>
#include <qtimer.h>

void QNowToolTip::display(QWidget *parent, const QRect& rect, const QString& text)
{
	QLabel *l = new QLabel(parent, NULL,
		Qt::WType_TopLevel |
		Qt::WStyle_Customize |
		Qt::WStyle_NoBorder |
		Qt::WStyle_StaysOnTop |
		Qt::WStyle_Dialog);

	l->setPaletteBackgroundColor(QColor(255, 255, 200));
	l->setText(text);
	l->move(rect.topLeft());
	l->show();

	QTimer::singleShot(2000, l, SLOT(deleteLater()));
}

