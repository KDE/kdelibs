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
	/* The main item is required so it makes no sense to do a layout without
	 * it. */
	if (!mainItem)
		return;

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
	/* Return the current size if we are not fully set up yet to avoid
	 * unnecessary resize events. */
	if (!mainItem)
		return (QSize(geometry().width(), geometry().height()));

	/* For the size hint we simply accumulate all item geometries
	 * appropriately. Note that this is not identical to the current
	 * size since all calculations are based on the main items size
	 * hint. */
	int w, h;
	if (mainItem->sizeHint().isValid())
	{
		/* If the main item has a valid size hint, we use it. */
		w = mainItem->sizeHint().width();
		h = mainItem->sizeHint().height();
	}
	else if (mainItem->minimumSize().isValid())
	{
		/* If not, if it has a valid minimum size, we use it. */
		w = mainItem->minimumSize().width();
		h = mainItem->minimumSize().height();
	}
	else
	{
		/* If no size hints are available, we keep the current size. */
		w = mainItem->width();
		h = mainItem->height();
	}

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
	/* Return the current size if we are not fully set up yet to avoid
	 * unnecessary resize events. */
	if (!mainItem)
		return (QSize(geometry().width(), geometry().height()));

	/* The minimum space needed by the main widget. */
	int mainW = mainItem->minimumWidth();
	int mainH = mainItem->minimumHeight();
	int mainMaxH = mainItem->maximumHeight();

	if (mainMaxH <= 0 || mainMaxH > mainItem->height())
		mainMaxH = mainItem->height();

	/* Find the largest minimum height of vertical bars and main widget */
	QListIterator<KToolBar> qli(leftToolBars);
	for (; *qli; ++qli)
	{
		if (mainH < (*qli)->minimumSizeHint().height())
			mainH = (*qli)->minimumSizeHint().height();
	}
	
	for (qli = rightToolBars; *qli; ++qli)
		if (mainH < (*qli)->minimumSizeHint().height())
			mainH = (*qli)->minimumSizeHint().height();

	/* minimum width for main widget and vertical bars */
	mainW += toolBarWidth(mainMaxH, leftToolBars) +
		toolBarWidth(mainMaxH, rightToolBars);

	/* Accumulate the space needed by all horizontal bars including the
	 * flattened ones. The calculation is based on the current width. */
	int currMargin =
		(topMenuBar ?
		 topMenuBar->heightForWidth(geometry().width()) : 0) +
		(bottomMenuBar ?
		 bottomMenuBar->heightForWidth(geometry().width()) : 0) +
		flatBarHeight(geometry().width()) +
		toolBarHeight(geometry().width(), topToolBars) +
		toolBarHeight(geometry().width(), bottomToolBars) +
		(statusBar ? statusBar->height() : 0);

	/* Find out if there is a horizontal bar that needs more space than the
	 * have determinded so far. */
	for (qli = topToolBars; *qli; ++qli)
		if ((*qli)->minimumSizeHint().width() > mainW)
			mainW = (*qli)->minimumSizeHint().width();

	for (qli = bottomToolBars; *qli; ++qli)
		if ((*qli)->minimumSizeHint().width() > mainW)
			mainW = (*qli)->minimumSizeHint().width();

	return (QSize(mainW, currMargin + mainH));
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
	/* This function layouts the vertical bars and the main widget. */
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
	/* This function layouts the horizontal tool bars. */
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
		if (prevFullSize || (*li)->fullSize() ||
			(currX + (*li)->maximumSizeHint().width() > rect.width()))
		{
			currY += currHeight;
			currX = 0;
			currHeight = 0;
		}

		int mw = rect.width();
		if (!(*li)->fullSize())
		{
			/* If we are not in fullSize mode and the user has requested a
			 * certain width, this will be used. If no size has been requested
			 * and the parent width is larger than the maximum width, we use
			 * the maximum width. */
			if ((*li)->maxWidth() != -1)
				mw = (*li)->maxWidth();
			else if (rect.width() > (*li)->maximumSizeHint().width())
				mw = (*li)->maximumSizeHint().width();
		}	

		int hfw = (*li)->heightForWidth(mw);
		if (hfw == 0)
			hfw = (*li)->height();

		(*li)->setGeometry(currX, currY, mw, hfw);
		
		currX += mw;
		if (hfw > currHeight)
			currHeight = hfw;
		prevFullSize = (*li)->fullSize();
		++li;
	}

	currY += currHeight;
}

int
KTMLayout::toolBarHeight(int rectw, const QList<KToolBar>& tbl) const
{
	/* Calculates the height needed for given group of toolbars with the
	 * width rectw. */
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
		if (prevFullSize || (*li)->fullSize() ||
			(currX + (*li)->maximumSizeHint().width() > rectw))
		{
			currY += currHeight;
			currX = 0;
			currHeight = 0;
		}

		int mw = rectw;
		if (!(*li)->fullSize())
		{
			/* If we are not in fullSize mode and the user has requested a
			 * certain width, this will be used. If no size has been requested
			 * and the parent width is larger than the maximum width, we use
			 * the maximum width. */
			if ((*li)->maxWidth() != -1)
				mw = (*li)->maxWidth();
			else if (rectw > (*li)->maximumSizeHint().width())
				mw = (*li)->maximumSizeHint().width();
		}	

		int hfw = (*li)->heightForWidth(mw);
		if (hfw == 0)
			hfw = (*li)->height();

		currX += mw;
		if (hfw > currHeight)
			currHeight = hfw;
		prevFullSize = (*li)->fullSize();
		++li;
	}
	currY += currHeight;

	return (currY);
}

void
KTMLayout::vToolBarLayout(const QRect& rect, int& currX,
						  const QList<KToolBar>& tbl)
{
	/* This function layouts the vertical tool bars. */
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
		if (prevFullSize || (*li)->fullSize() ||
			(currY + (*li)->maximumSizeHint().height() > rect.height()))
		{
			currX += currWidth;
			currY = 0;
			currWidth = 0;
		}

		int mh = rect.height();
		if (!(*li)->fullSize())
		{
			/* If we are not in fullSize mode and the user has requested a
			 * certain height, this will be used. If no size has been requested
			 * and the parent height is larger than the maximum height, we use
			 * the maximum height. */
			if ((*li)->maxHeight() != -1)
				mh = (*li)->maxHeight();
			else if (rect.height() > (*li)->maximumSizeHint().height())
				mh = (*li)->maximumSizeHint().height();
		}
		
		int wfh = (*li)->widthForHeight(mh);
		if (wfh == 0)
			wfh = (*li)->width();

		(*li)->setGeometry(currX, rect.y() + currY, wfh, mh);
		
		currY += mh;
		if (wfh > currWidth)
			currWidth = wfh;
		prevFullSize = (*li)->fullSize();
		++li;
	}

	currX += currWidth;
}

int
KTMLayout::toolBarWidth(int recth, const QList<KToolBar>& tbl) const
{
	/* Calculates the width needed for given group of toolbars with the
	 * height recth. */
	QListIterator<KToolBar> li(tbl);
	int currX = 0;
	int currY = 0;
	bool prevFullSize = false;
	int currWidth = 0;

	while (*li)
	{
		/* If the previous toolbar was not full height and the current
		 * one is also not full height, we put them in the same
		 * column. If this is not the case or the current one does not
		 * fit in the column anymore we start a new line. */
		if (prevFullSize || (*li)->fullSize() ||
			(currY + (*li)->maximumSizeHint().height() > recth))
		{
			currX += currWidth;
			currY = 0;
			currWidth = 0;
		}

		int mh = recth;
		if (!(*li)->fullSize())
		{
			/* If we are not in fullSize mode and the user has requested a
			 * certain height, this will be used. If no size has been requested
			 * and the parent height is larger than the maximum height, we use
			 * the maximum height. */
			if ((*li)->maxHeight() != -1)
				mh = (*li)->maxHeight();
			else if (recth > (*li)->maximumSizeHint().height())
				mh = (*li)->maximumSizeHint().height();
		}
		
		int wfh = (*li)->widthForHeight(mh);
		if (wfh == 0)
			wfh = (*li)->width();

		currY += mh;
		if (wfh > currWidth)
			currWidth = wfh;
		prevFullSize = (*li)->fullSize();
		++li;
	}
	currX += currWidth;

	return (currX);
}
