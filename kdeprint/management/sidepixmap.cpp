/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Michael Goffioul <kdeprint@swing.be>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

#include "sidepixmap.h"

#include <qpainter.h>
#include <kstandarddirs.h>

SidePixmap::SidePixmap(QWidget *parent, const char *name)
: Q3Frame(parent, name)
{
	setLineWidth(1);
	setFrameStyle(Q3Frame::WinPanel|Q3Frame::Sunken);

	m_side.load(locate("data", "kdeprint/side.png"));
	m_tileup.load(locate("data", "kdeprint/tileup.png"));
	m_tiledown.load(locate("data", "kdeprint/tiledown.png"));

	setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum));
}

bool SidePixmap::isValid()
{
	return (!m_side.isNull() && !m_tileup.isNull() && !m_tiledown.isNull() 
			&& (m_side.width() == m_tileup.width())
			&& (m_side.width() == m_tiledown.width()));
}

QSize SidePixmap::sizeHint() const
{
	return (QSize(m_side.width()+lineWidth(), 300+lineWidth()));
}

void SidePixmap::drawContents(QPainter *p)
{
	QRect	r = contentsRect();
	if (r.height() <= m_side.height())
	{
		// case where the main pixmap is enough, just center it
		int	yoffset = (m_side.height()-r.height())/2;
		p->drawPixmap(r.left(), r.top(), m_side, 0, yoffset, r.width(), r.height());
	}
	else
	{
		// we need to add tiled pixmap. Make sure we have a multiple
		// of 3 in the space above to simplifu tiling.
		int	hh = (((r.height()-m_side.height())/2)/3)*3;
		if (hh > 0)
			p->drawTiledPixmap(r.left(), r.top(), r.width(), hh, m_tileup);
		p->drawPixmap(r.left(), r.top()+hh, m_side);
		p->drawTiledPixmap(r.left(), r.top()+m_side.height()+hh, r.width(), r.height()-m_side.height()-hh, m_tiledown);
	}
}
