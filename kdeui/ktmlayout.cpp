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

#include <qlist.h>

#include <ktoolbar.h>
#include <ktmlayout.h>

KTMLayout::KTMLayout(QWidget *parent, int border, int space,
					 const char *name) :
	QLayout(parent, border, space, name)
{
	topMenuBar = bottomMenuBar = statusBar = 0;
	mainItem = 0;
}

void 
KTMLayout::setGeometry(const QRect& rect)
{
	QLayout::setGeometry(rect);

	int currY = rect.y();

	/* position top menu bar */
	if (topMenuBar)
	{
		int hfw = topMenuBar->heightForWidth(rect.width());
		if (hfw == 0)
			hfw = topMenuBar->height();
		topMenuBar->setGeometry(rect.x(), currY, rect.width(), hfw);
		currY += hfw;
	}

	/* layout flattened bars */
	flatBarLayout(rect, currY, flatBars);

	/* layout top tool bars */
	hToolBarLayout(rect, currY, topToolBars);

	/* position main layout */
	int bottomHeight = toolBarHeight(rect.width(), bottomToolBars) +
		(statusBar ? statusBar->height() : 0) +
		(bottomMenuBar ? bottomMenuBar->sizeHint().height() : 0);
	int mwh = rect.height() - currY - bottomHeight;
	mainLayout(QRect(rect.x(), currY, rect.width(), mwh));
	currY += mwh;

	/* layout bottom tool bars */
	hToolBarLayout(rect, currY, bottomToolBars);

	/* position status bar */
	if (statusBar)
	{
		int hfw = statusBar->heightForWidth(rect.width());
		if (hfw == 0)
			hfw = statusBar->height();
		statusBar->setGeometry(rect.x(), currY, rect.width(), hfw);
		currY += hfw;
	}

	/* position bottom menu bar */
	if (bottomMenuBar)
	{
		int hfw = bottomMenuBar->heightForWidth(rect.width());
		if (hfw == 0)
			hfw = bottomMenuBar->height();
		bottomMenuBar->setGeometry(rect.x(), currY, rect.width(), hfw);
		currY += hfw;
	}
}

QSize 
KTMLayout::sizeHint(void) const
{
	if (!mainItem)
		return (QSize(-1, -1));

	int w = mainItem->sizeHint().width();
	int h = mainItem->sizeHint().height();

	w += toolBarWidth(h, leftToolBars) + toolBarWidth(h, rightToolBars);
	h += toolBarHeight(w, topToolBars) + toolBarHeight(w, bottomToolBars);
	h += flatBarHeight(w);

	if (topMenuBar)
		h += topMenuBar->heightForWidth(w);
	if (bottomMenuBar)
		h += bottomMenuBar->heightForWidth(w);

	if (statusBar)
		h += statusBar->sizeHint().height();

	return (QSize(w, h));
}

QSize
KTMLayout::minimumSize(void) const
{
	if (!mainItem)
		return (QSize(1, 1));

	int w = mainItem->minimumWidth();
	int h = mainItem->minimumHeight();

	int currMargin = (topMenuBar ? topMenuBar->height() : 0) +
		(bottomMenuBar ? bottomMenuBar->height() : 0) +
		flatBarHeight(geometry().width()) +
		toolBarHeight(geometry().width(), topToolBars) +
		toolBarHeight(geometry().width(), bottomToolBars) +
		(statusBar ? statusBar->height() : 0);

	QListIterator<KToolBar> qli(leftToolBars);
	for (; *qli; ++qli)
		if (!(*qli)->fullWidth() && (*qli)->maximumSizeHint().height() > h)
			h = (*qli)->maximumSizeHint().height();

	for (qli = rightToolBars; *qli; ++qli)
		if (!(*qli)->fullWidth() && (*qli)->maximumSizeHint().height() > h)
			h = (*qli)->maximumSizeHint().height();

	for (qli = topToolBars; *qli; ++qli)
		if (!(*qli)->fullWidth() && (*qli)->maximumSizeHint().width() > w)
			w = (*qli)->maximumSizeHint().width();

	for (qli = bottomToolBars; *qli; ++qli)
		if (!(*qli)->fullWidth() && (*qli)->maximumSizeHint().width() > w)
			w = (*qli)->maximumSizeHint().width();

	if (toolBarWidth(geometry().height(), leftToolBars) +
		toolBarHeight(geometry().height(), rightToolBars) > w)
	{
		w = toolBarWidth(geometry().height(), leftToolBars) +
			toolBarHeight(geometry().height(), rightToolBars);
	}

	return (QSize(w, currMargin + h));
}

void 
KTMLayout::addItem(QLayoutItem*)
{
	/* we do not support sub layouts */
}

QLayoutIterator KTMLayout::iterator()
{       
	return QLayoutIterator(new KTMLayoutIterator(&list));
}

void
KTMLayout::mainLayout(const QRect& rect)
{
	QLayout::setGeometry(rect);

	int currX = rect.x();

	/* layout left hand tool bars */
	vToolBarLayout(rect, currX, leftToolBars);

	/* position main widget */
	int mainWidth = rect.width() - currX -
		toolBarWidth(rect.height(), rightToolBars);
	if (mainItem)
	{
		mainItem->setGeometry(currX, rect.y(), mainWidth, rect.height());
		currX += mainWidth;
	}

	/* layout right hand tool bars */
	vToolBarLayout(rect, currX, rightToolBars);
}

int
KTMLayout::toolBarWidth(int recth, const QList<KToolBar>& tbl) const
{
	QListIterator<KToolBar> li(tbl);
	int currX = 0;
	int currY = 0;
	bool prevFullSize = false;
	int currWidth = 0;

	while (*li)
	{
		/* If the previous toolbar was not full height and the current one
		 * is also not full height, we put them in the same column. If this is
		 * not the case or the current one does not fit in the column anymore
		 * we start a new column. */
		if (prevFullSize || (*li)->fullWidth() ||
			(currY + (*li)->maximumSizeHint().height() > recth))
		{
			currX += currWidth;
			currY = 0;
			currWidth = 0;
		}
		int wfh = (*li)->widthForHeight(recth);
		if (wfh == 0)
			wfh = (*li)->width();
		int h = !(*li)->fullWidth() ?
			(*li)->maximumSizeHint().height() : recth;
		currY += h;
		if (wfh > currWidth)
			currWidth = wfh;
		prevFullSize = (*li)->fullWidth();
		++li;
	}
	currX += currWidth;

	return (currX);
}

int
KTMLayout::toolBarHeight(int rectw, const QList<KToolBar>& tbl) const
{
	QListIterator<KToolBar> li(tbl);
	int currX = 0;
	int currY = 0;
	bool prevFullSize = false;
	int currHeight = 0;
	while (*li)
	{
		/* If the previous toolbar was not full width and the current one
		 * is also not full width, we put them in the same line. If this is
		 * not the case or the current one does not fit in the line anymore
		 * we start a new line. */
		if (prevFullSize || (*li)->fullWidth() ||
			(currX + (*li)->maximumSizeHint().width() > rectw))
		{
			currY += currHeight;
			currX = 0;
			currHeight = 0;
		}
		int hfw = (*li)->heightForWidth(rectw);
		if (hfw == 0)
			hfw = (*li)->height();
		int w = !(*li)->fullWidth() ? (*li)->maximumSizeHint().width() : rectw;
		currX += w;
		if (hfw > currHeight)
			currHeight = hfw;
		prevFullSize = (*li)->fullWidth();
		++li;
	}
	currY += currHeight;

	return (currY);
}

void
KTMLayout::flatBarLayout(const QRect& rect, int& currY,
						  const QList<QWidget>& fbl)
{
	/* All flattened bars show up together on one or more lines. This way
	 * flattened bars consume minimum space but being still accessible. */
	QListIterator<QWidget> li(fbl);

	int currX = 0;
	int currHeight = 0;

	while (*li)
	{
		int w = (*li)->sizeHint().width();
		int h = (*li)->sizeHint().height();

		if (currX + w > rect.width())
		{
			currY += currHeight;
			currX = 0;
			currHeight = 0;
		}
		
		(*li)->setGeometry(currX, currY, w, h);

		currX += w;
		/* I think all flattened bars have the same height but I can deal
		 * with alternating heights as well. */
		if (h > currHeight)
			currHeight = h;
		++li;
	}
	currY += currHeight;
}

int
KTMLayout::flatBarHeight(int rectW) const
{
	/* All flattened bars show up together on one or more lines. This way
	 * flattened bars consume minimum space but being still accessible. */
	QListIterator<QWidget> li(flatBars);

	int currX = 0;
	int currY = 0;
	int currHeight = 0;

	while (*li)
	{
		int w = (*li)->sizeHint().width();
		int h = (*li)->sizeHint().height();

		if (currX + w > rectW)
		{
			currY += currHeight;
			currX = 0;
			currHeight = 0;
		}
		
		currX += w;
		/* I think all flattened bars have the same height but I can deal
		 * with alternating heights as well. */
		if (h > currHeight)
			currHeight = h;
		++li;
	}
	return (currY + currHeight);
}

void
KTMLayout::hToolBarLayout(const QRect& rect, int& currY,
						  const QList<KToolBar>& tbl)
{
	QListIterator<KToolBar> li(tbl);
	int currX = 0;
	bool prevFullSize = false;
	int currHeight = 0;
	while (*li)
	{
		/* If the previous toolbar was not full width and the current one
		 * is also not full width, we put them in the same line. If this is
		 * not the case or the current one does not fit in the line anymore
		 * we start a new line. */
		if (prevFullSize || (*li)->fullWidth() ||
			(currX + (*li)->maximumSizeHint().width() > rect.width()))
		{
			currY += currHeight;
			currX = 0;
			currHeight = 0;
		}
		int hfw = (*li)->heightForWidth(rect.width());
		if (hfw == 0)
			hfw = (*li)->height();
		int w = !(*li)->fullWidth() ? (*li)->maximumSizeHint().width() :
			rect.width();

		(*li)->setGeometry(currX, currY, w, hfw);
		
		currX += w;
		if (hfw > currHeight)
			currHeight = hfw;
		prevFullSize = (*li)->fullWidth();
		++li;
	}

	currY += currHeight;
}

void
KTMLayout::vToolBarLayout(const QRect& rect, int& currX,
						  const QList<KToolBar>& tbl)
{
	QListIterator<KToolBar> li(tbl);
	int currY = 0;
	bool prevFullSize = false;
	int currWidth = 0;

	while (*li)
	{
		/* If the previous toolbar was not full height and the current
		 * one is also not full height, we put them in the same
		 * column. If this is not the case or the current one does not
		 * fit in the column anymore we start a new line. */
		if (prevFullSize || (*li)->fullWidth() ||
			(currY + (*li)->maximumSizeHint().height() > rect.height()))
		{
			currX += currWidth;
			currY = 0;
			currWidth = 0;
		}
		int wfh = (*li)->widthForHeight(rect.height());
		if (wfh == 0)
			wfh = (*li)->width();
		int h = !(*li)->fullWidth() ? (*li)->maximumSizeHint().height() :
			rect.height();

		(*li)->setGeometry(currX, rect.y() + currY, wfh, h);
		
		currY += h;
		if (wfh > currWidth)
			currWidth = wfh;
		prevFullSize = (*li)->fullWidth();
		++li;
	}

	currX += currWidth;
}
