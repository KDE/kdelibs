#include "qnowtooltip.h"

#include <qlabel.h>
#include <qtimer.h>

void QNowToolTip::display(QWidget *parent, const QRect& rect, const QString& text)
{
	QLabel *l = new QLabel(parent,
		Qt::Window |
		Qt::FramelessWindowHint |
		Qt::WindowStaysOnTopHint |
		Qt::Dialog);

	//l->setPaletteBackgroundColor(QColor(255, 255, 200)); // FIXME KDE4PORT
	l->setText(text);
	l->move(rect.topLeft());
	l->show();

	QTimer::singleShot(2000, l, SLOT(deleteLater()));
}

