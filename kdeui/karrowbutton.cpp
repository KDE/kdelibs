/* This file is part of the KDE libraries
	 Copyright (C) 2001 Frerich Raabe <raabe@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	 Library General Public License for more details.

	 You should have received a copy of the GNU Library General Public License
	 along with this library; see the file COPYING.LIB.  If not, write to
	 the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
	 Boston, MA 02111-1307, USA.
*/

#include "karrowbutton.h"

#include <qstyle.h>

class KArrowButton::Private
{
	public:
		Private() {}

		Qt::ArrowType arrow;
};

KArrowButton::KArrowButton(QWidget *parent, Qt::ArrowType arrow,
		const char *name)
	: QPushButton(parent, name)
{
	d = new Private();
	setArrowType(arrow);
}

KArrowButton::~KArrowButton()
{
	delete d;
}

QSize KArrowButton::sizeHint() const
{
	return QSize( 12, 12 );
}

void KArrowButton::setArrowType(Qt::ArrowType a)
{
	if (arrow() != a) {
		setArrow(a);
		repaint();
	}
}

void KArrowButton::drawButton(QPainter *p)
{
	const unsigned int arrowSize = 8;
	const unsigned int margin = 2;
	
#if QT_VERSION < 300
	style().drawPanel(p, 0, 0, width(), height(), colorGroup(),
			isDown(), 2, &colorGroup().brush(QColorGroup::Background));
#else
#if defined(Q_CC_GNU)
#warning beta3's qstyle API doesn't allow to specify the background fill brush!
#endif
	style().drawPrimitive( QStyle::PE_Panel, p, QRect( 0, 0, width(), height() ),
			       colorGroup(), 
			       isDown() ? QStyle::Style_Sunken : QStyle::Style_Default,
			       QStyleOption( 2, 0 ) );
#endif

	if (static_cast<unsigned int>(width()) < arrowSize + margin ||
	    static_cast<unsigned int>(height()) < arrowSize + margin)
		return; // don't draw arrows if we are too small

	unsigned int x = 0, y = 0;
	if (arrow() == DownArrow) {
		x = (width() - arrowSize) / 2;
		y = height() - (arrowSize + margin);
	} else if (arrow() == UpArrow) {
		x = (width() - arrowSize) / 2;
		y = margin;
	} else if (arrow() == RightArrow) {
		x = width() - (arrowSize + margin);
		y = (height() - arrowSize) / 2;
	} else { // arrowType == LeftArrow
		x = margin;
		y = (height() - arrowSize) / 2;
	}

	if (isDown()) {
		x++;
		y++;
	}

#if QT_VERSION < 300
	style().drawArrow(p, arrow(), isDown(), x, y, arrowSize, arrowSize,
			colorGroup(), true);
#else
	QStyle::PrimitiveElement e = QStyle::PE_ArrowLeft;
	switch( arrow() )
	{
		case Qt::LeftArrow: e = QStyle::PE_ArrowLeft; break;
		case Qt::RightArrow: e = QStyle::PE_ArrowRight; break;
		case Qt::UpArrow: e = QStyle::PE_ArrowUp; break;
		case Qt::DownArrow: e = QStyle::PE_ArrowDown; break;
	}
	int flags = QStyle::Style_Enabled;
	if ( isDown() )
		flags |= QStyle::Style_Down;
	style().drawPrimitive( e, p, QRect( x, y, arrowSize, arrowSize ),
			       colorGroup(), flags );
#endif
}

void KArrowButton::setArrow(Qt::ArrowType a)
{
	d->arrow = a;
}

Qt::ArrowType KArrowButton::arrow() const
{
	return d->arrow;
}

#include "karrowbutton.moc"
