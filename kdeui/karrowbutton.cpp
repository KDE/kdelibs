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

KArrowButton::KArrowButton(QWidget *parent, Qt::ArrowType arrow,
		const char *name)
	: QPushButton(parent, name)
{
	setArrowType(arrow);
}

KArrowButton::~KArrowButton()
{
}

void KArrowButton::setArrowType(Qt::ArrowType arrow)
{
	if (m_arrow != arrow) {
		m_arrow = arrow;
		repaint();
	}
}

void KArrowButton::drawButton(QPainter *p)
{
	const unsigned int arrowSize = 8;
	const unsigned int margin = 2;
	
	style().drawPanel(p, 0, 0, width(), height(), colorGroup(),
			isDown(), 2, &colorGroup().brush(QColorGroup::Background));

	if (static_cast<unsigned int>(width()) < arrowSize + margin ||
	    static_cast<unsigned int>(height()) < arrowSize + margin)
		return; // don't draw arrows if we are too small

	unsigned int x = 0, y = 0;
	if (m_arrow == DownArrow) {
		x = (width() - arrowSize) / 2;
		y = height() - (arrowSize + margin);
	} else if (m_arrow == UpArrow) {
		x = (width() - arrowSize) / 2;
		y = margin;
	} else if (m_arrow == RightArrow) {
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

	style().drawArrow(p, m_arrow, isDown(), x, y, arrowSize, arrowSize,
			colorGroup(), true);
}

#include "karrowbutton.moc"
