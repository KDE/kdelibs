#ifndef QNOW_TOOLTIP_H
#define QNOW_TOOLTIP_H

#include <qrect.h>
#include <qstring.h>
#include <qwidget.h>

class QNowToolTip
{
	public:
		static void display(QWidget *parent, const QRect& pos, const QString& text);
};

#endif

