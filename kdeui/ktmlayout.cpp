/* This file is part of the KDE libraries

    Copyright  (c) 1999 Chris Schlaeger (cs@kde.org)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <stdio.h>
#include <ktoolbar.h>
#include <ktmlayout.h>

KHTBLayout::KHTBLayout(QWidget *parent, int border, int space,
					   const char *name) :
	QHBoxLayout(parent, border, space, name)
{
	mainItem = 0;
}

void 
KHTBLayout::addMainItem(QWidget* w, int stretch, int alignment)
{
	mainItem = w;
	addWidget(w, stretch, alignment);
}

void 
KHTBLayout::setGeometry(const QRect& rect)
{
	QLayout::setGeometry(rect);

	QLayoutItem* li;
	QLayoutIterator qlit = iterator();

	/* the total width of all wrapping widgets */
	int tbWidth = 0;
	for (; (li = qlit.current()) != 0; ++qlit)
	{
		QWidget* w = li->widget();
		if (w != mainItem)
			tbWidth += ((KToolBar*) w)->widthForHeight(rect.height());
	}
	/* Now we iterate over all widgets and adjust their geometry. */
	int currX = rect.x();
	for (qlit = iterator(); (li = qlit.current()) != 0; ++qlit)
	{
		QWidget* w = li->widget();
		if (w != mainItem)
		{
			/* adjust tool bar */
			int wfh = ((KToolBar*) w)->widthForHeight(rect.height());
			w->setGeometry(currX, rect.y(), wfh, rect.height());
			currX += wfh;
		}
		else
		{
			/* adjust main view */
			w->setGeometry(currX, rect.y(), rect.width() - tbWidth,
						   rect.height());
			currX += rect.width() - tbWidth;
		}
	}
}

/*
 * class KVTBLayout
 */

KVTBLayout::KVTBLayout(QWidget *parent, int border, int space,
					   const char *name) :
	QVBoxLayout(parent, border, space, name)
{
	mainItem = 0;
}

void 
KVTBLayout::addMainItem(QWidget* w, int stretch, int alignment)
{
	mainItem = w;
	addWidget(w, stretch, alignment);
}

void
KVTBLayout::addMainLayout(QLayout* w, int stretch)
{
	mainItem = w->mainWidget();
	addLayout(w, stretch);
}

void 
KVTBLayout::setGeometry(const QRect& rect)
{
	QLayout::setGeometry(rect);

	QLayoutItem* li;
	QLayoutIterator qlit = iterator();

	/* the total height of all wrapping widgets */
	int tbHeight = 0;
	for (; (li = qlit.current()) != 0; ++qlit)
	{
		QWidget* w = li->widget();
		if (w != mainItem)
		{
			int hfw = w->heightForWidth(rect.width());
			if (hfw == 0)
				hfw = w->height();
			tbHeight += hfw;
		}
	}
	/* Now we iterate over all widgets and adjust their geometry. */
	int currY = rect.y();
	for (qlit = iterator(); (li = qlit.current()) != 0; ++qlit)
	{
		QWidget* w = li->widget();
		if (w != mainItem)
		{
			/* adjust wrapping widget */
			int hfw = w->heightForWidth(rect.width());
			if (hfw == 0)
				hfw = w->height();
			w->setGeometry(rect.x(), currY, rect.width(), hfw);
			currY += hfw;
		}
		else
		{
			/* adjust main widget */
			li->setGeometry(QRect(rect.x(), currY, rect.width(),
								  rect.height() - tbHeight));
			currY += rect.height() - tbHeight;
		}
	}
}
