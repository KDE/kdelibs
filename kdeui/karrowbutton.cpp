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

#include <QStyle>
#include <QStyleOptionFrame>
#include <QPainter>

#include <assert.h>

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

void KArrowButton::paintEvent(QPaintEvent*)
{
	QPainter p(this);
	const unsigned int arrowSize = 8;
	const unsigned int margin = 2;

	QStyleOptionFrame opt;
	opt.init(this);
	opt.lineWidth    = 2;
	opt.midLineWidth = 0;
	
        p.fillRect( rect(), colorGroup().brush( QColorGroup::Background ) );
        
	style()->drawPrimitive( QStyle::PE_Frame, &opt, &p, this);

	if (d->arrow == Qt::NoArrow)
		return;

	if (static_cast<unsigned int>(width()) < arrowSize + margin ||
	    static_cast<unsigned int>(height()) < arrowSize + margin)
		return; // don't draw arrows if we are too small

	unsigned int x = 0, y = 0;
	if (d->arrow == Qt::DownArrow) {
		x = (width() - arrowSize) / 2;
		y = height() - (arrowSize + margin);
	} else if (d->arrow == Qt::UpArrow) {
		x = (width() - arrowSize) / 2;
		y = margin;
	} else if (d->arrow == Qt::RightArrow) {
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

	QStyle::PrimitiveElement e = QStyle::PE_IndicatorArrowLeft;
	switch (d->arrow)
	{
		case Qt::LeftArrow: e = QStyle::PE_IndicatorArrowLeft; break;
		case Qt::RightArrow: e = QStyle::PE_IndicatorArrowRight; break;
		case Qt::UpArrow: e = QStyle::PE_IndicatorArrowUp; break;
		case Qt::DownArrow: e = QStyle::PE_IndicatorArrowDown; break;
		case Qt::NoArrow: assert( 0 ); break;
	}

	opt.state |= QStyle::State_Enabled;
	opt.rect   = QRect( x, y, arrowSize, arrowSize);

	style()->drawPrimitive( e, &opt, &p, this );
}

void KArrowButton::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

#include "karrowbutton.moc"
