/*
    This file is part of the KDE libraries

    Copyright (c) 1999 Chris Schlaeger (cs@kde.org)

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
#ifndef _KHTBLayout_h_
#define _KHTBLayout_h_

#include <qlayout.h>
#include <qlist.h>

#include <ktoolbar.h>

/**
 * KHTBLayout is a specialized version of QHBoxLayout. It can be used when
 * you want to manage horizontally aligned widgets that consist of one main
 * widget and other toolbar-like widgets. The toolbar like widgets must have
 * a fixed width that can depend on their height. The main widget takes the
 * rest of the space. The toolbars must define a widthForHeight(int) function.
 * The most prominent user of this class is KTMainWidget.
 *
 * @see KTMainWindow
 * @short Special layout managers for use with toolbar-like widgets.
 * @author Chris Schlaeger (cs@kde.org)
 */
class KTMLayout : public QLayout
{
public:
	KTMLayout()
	{
		mainItem = 0;
	}
	KTMLayout(QWidget *parent, int border = 0, int space = -1,
			   const char *name = 0);

	~KTMLayout() { }

	void addTopMenuBar(QWidget* w)
	{
		topMenuBar = w;
	}

	void addBottomMenuBar(QWidget* w)
	{
		bottomMenuBar = w;
	}

	void addFlatBar(QWidget* w)
	{
		flatBars.append(w);
	}

	void addTopToolBar(KToolBar* w)
	{
		topToolBars.append(w);
	}

	void addLeftToolBar(KToolBar* w)
	{
		leftToolBars.append(w);
	}

	void addRightToolBar(KToolBar* w)
	{
		rightToolBars.append(w);
	}

	void addBottomToolBar(KToolBar* w)
	{
		bottomToolBars.append(w);
	}

	/**
	 * Use this function to add the main widget. All toolbars can be added
	 * with addItem(). There must be exactly one main widget!
	 */
	void addMainItem(QWidget* w)
	{
		mainItem = w;
	}

	void addStatusBar(QWidget* w)
	{
		statusBar = w;
	}


	/**
	 * This function arranges the widgets. It determines the necessary space
	 * for the toolbars and assignes the remainder to the main widget.
	 */
	void setGeometry(const QRect& rect);

	QSize sizeHint(void) const;
	QSize minimumSize(void) const;
	void addItem(QLayoutItem* item);
	QLayoutIterator iterator();
	
private:
	void mainLayout(const QRect& rect);

	int toolBarWidth(int h, const QList<KToolBar>& tbl) const;
	int toolBarHeight(int w, const QList<KToolBar>& tbl) const;
	int flatBarHeight(int w) const;

	void vToolBarLayout(const QRect& rect, int& currX,
					   const QList<KToolBar>& tbl);
	void hToolBarLayout(const QRect& rect, int& currY,
					   const QList<KToolBar>& tbl);
	void flatBarLayout(const QRect& rect, int& currY,
					   const QList<QWidget>& fbl);

	QList<QLayoutItem> list;

	QWidget* topMenuBar;
	QWidget* bottomMenuBar;

	QList<QWidget> flatBars;

	QList<KToolBar> topToolBars;
	QList<KToolBar> leftToolBars;
	QList<KToolBar> rightToolBars;
	QList<KToolBar> bottomToolBars;

	QWidget* mainItem;
	QWidget* statusBar;
} ;

class KTMLayoutIterator :public QGLayoutIterator
{
public:
	KTMLayoutIterator(QList<QLayoutItem> *l)
		: idx(0), list(l) { }

	QLayoutItem *current()
	{
		return idx < int(list->count()) ? list->at(idx) : 0;
	}

	QLayoutItem *next()
	{
		idx++;
		return current();
	}

	void removeCurrent()
	{
		list->remove( idx );
	}

private:
	int idx;
	QList<QLayoutItem> *list;
};

#endif
