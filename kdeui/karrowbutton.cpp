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
	 the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
	 Boston, MA 02110-1301, USA.
*/

#include "karrowbutton.h"

#include <qstyle.h>
#include <qpainter.h>

class KArrowButtonPrivate
{
	public:
		Qt::ArrowType arrow;
};

KArrowButton::KArrowButton(QWidget *parent, Qt::ArrowType arrow,
		const char *name)
	: QPushButton(parent, name)
{
	d = new KArrowButtonPrivate();
	d->arrow = arrow;
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
	if (d->arrow != a) {
		d->arrow = a;
		repaint();
	}
}
Qt::ArrowType KArrowButton::arrowType() const
{
	return d->arrow;
}

void KArrowButton::drawButton(QPainter *p)
{
	const unsigned int arrowSize = 8;
	const unsigned int margin = 2;
	
        p->fillRect( rect(), colorGroup().brush( QColorGroup::Background ) );
	style().drawPrimitive( QStyle::PE_Panel, p, QRect( 0, 0, width(), height() ),
			       colorGroup(), 
			       isDown() ? QStyle::Style_Sunken : QStyle::Style_Default,
			       QStyleOption( 2, 0 ) );

	if (static_cast<unsigned int>(width()) < arrowSize + margin ||
	    static_cast<unsigned int>(height()) < arrowSize + margin)
		return; // don't draw arrows if we are too small

	unsigned int x = 0, y = 0;
	if (d->arrow == DownArrow) {
		x = (width() - arrowSize) / 2;
		y = height() - (arrowSize + margin);
	} else if (d->arrow == UpArrow) {
		x = (width() - arrowSize) / 2;
		y = margin;
	} else if (d->arrow == RightArrow) {
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

	QStyle::PrimitiveElement e = QStyle::PE_ArrowLeft;
	switch (d->arrow)
	{
		case Qt::LeftArrow: e = QStyle::PE_ArrowLeft; break;
		case Qt::RightArrow: e = QStyle::PE_ArrowRight; break;
		case Qt::UpArrow: e = QStyle::PE_ArrowUp; break;
		case Qt::DownArrow: e = QStyle::PE_ArrowDown; break;
	}
	int flags = QStyle::Style_Enabled;
	if ( isDown() )
		flags |= QStyle::Style_Down;
	style().drawPrimitive( e, p, QRect( QPoint( x, y ), QSize( arrowSize, arrowSize ) ),
			       colorGroup(), flags );
}

void KArrowButton::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

#include "karrowbutton.moc"
