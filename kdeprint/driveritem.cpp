/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Michael Goffioul <goffioul@imec.be>
 *
 *  $Id$
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
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

#include "driveritem.h"
#include "driver.h"

#include <qpainter.h>
#include <kiconloader.h>
#include <kdebug.h>

DriverItem::DriverItem(QListView *parent, DrBase *item)
: QListViewItem(parent), m_item(item), m_conflict(false)
{
	setOpen(depth() < 3);
	setPixmap(0,SmallIcon("fileprint"));
	updateText();
}

DriverItem::DriverItem(QListViewItem *parent, QListViewItem *after, DrBase *item)
: QListViewItem(parent, after), m_item(item), m_conflict(false)
{
	setOpen(depth() < 3);
	if (item) setPixmap(0,SmallIcon((item->isOption() ? "document" : "folder")));
	updateText();
}

void DriverItem::updateText()
{
	if (m_item)
	{
		QString	s(m_item->get("text"));
		if (m_item->isOption())
			s.append(QString::fromLatin1(": <%1>").arg(m_item->prettyText()));
		if (m_item->type() == DrBase::List)
		{
			// remove all children: something has changed (otherwise this
			// function would not be called), so it make sense to remove
			// those children in all cases.
			while (firstChild())
				delete firstChild();
			DrBase	*ch = static_cast<DrListOption*>(m_item)->currentChoice();
			if (ch && ch->type() == DrBase::ChoiceGroup)
			{
				// add new children
				static_cast<DrChoiceGroup*>(ch)->createItem(this);
				setOpen(true);
			}
		}
		setText(0,s);
	}
	else
		setText(0,"ERROR");
	widthChanged();
}

void DriverItem::paintCell(QPainter *p, const QColorGroup& cg, int, int width, int)
{
	// background
	p->fillRect(0, 0, width, height(), cg.base());

	// highlight rectangle
	if (isSelected())
		p->fillRect(0, 0, /*2+p->fontMetrics().width(text(0))+(pixmap(0) ? pixmap(0)->width()+2 : 0)*/ width, height(), (m_conflict ? red : cg.highlight()));

	// draw pixmap
	int	w(0);
	if (pixmap(0) && !pixmap(0)->isNull())
	{
		int	h((height()-pixmap(0)->height())/2);
		p->drawPixmap(w,h,*pixmap(0));
		w += (pixmap(0)->width()+2);
	}

	// draw Text
	if (!m_item || !m_item->isOption() || isSelected())
	{
		p->setPen((isSelected() ? cg.highlightedText() : (m_conflict ? red : cg.text())));
		p->drawText(w,0,width-w,height(),Qt::AlignLeft|Qt::AlignVCenter,text(0));
	}
	else
	{
		int	w1(0);
		QString	s(m_item->get("text") + ": <");
		w1 = p->fontMetrics().width(s);
		p->setPen(cg.text());
		p->drawText(w,0,w1,height(),Qt::AlignLeft|Qt::AlignVCenter,s);
		w += w1;
		p->setPen((m_conflict ? red : darkGreen));
		s = m_item->prettyText();
		w1 = p->fontMetrics().width(s);
		p->drawText(w,0,w1,height(),Qt::AlignLeft|Qt::AlignVCenter,s);
		w += w1;
		p->setPen(cg.text());
		s = QString::fromLatin1(">");
		w1 = p->fontMetrics().width(s);
		p->drawText(w,0,w1,height(),Qt::AlignLeft|Qt::AlignVCenter,s);
	}
}

bool DriverItem::updateConflict()
{
	m_conflict = false;
	if (m_item)
	{
		if (!m_item->isOption())
		{
			DriverItem	*item = (DriverItem*)firstChild();
			while (item)
			{
				if (item->updateConflict())
					m_conflict = true;
				item = (DriverItem*)item->nextSibling();
			}
		}
		else
		{
			m_conflict = (m_item->conflict());
		}
	}
	repaint();
	return m_conflict;
}

void DriverItem::updateTextRecursive()
{
	if ( m_item->isOption() )
		updateText();

	DriverItem *item = static_cast<DriverItem*>( firstChild() );
	while ( item )
	{
		item->updateTextRecursive();
		item = static_cast<DriverItem*>( item->nextSibling() );
	}
}
