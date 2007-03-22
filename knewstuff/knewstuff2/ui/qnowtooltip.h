#ifndef KNEWSTUFF2_UI_QNOWTOOLTIP_H
#define KNEWSTUFF2_UI_QNOWTOOLTIP_H

#include <qrect.h>
#include <qstring.h>
#include <qwidget.h>

class QNowToolTip
{
	public:
		static void display(QWidget *parent, const QRect& pos, const QString& text);
};

#endif

