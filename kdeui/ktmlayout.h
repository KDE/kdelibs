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
 * KTMLayout is layout manager for KTMainWindow. It is customized
 * specifically for KTMainWindow's layout. You can use it for your own
 * widgets but there is probably little use for it. So this class can
 * be regarded as a kdeui internal class.
 *
 * It currently supports main widget with fixed size, fixed height and
 * widgets with no size requirements. heightForWidth() widgets and
 * fixed width widgets are not yet supported. It is currently unclear
 * whether the concept of widthForHeight() vertical bars can be used
 * in a widthForHeight() dominated world.
 *
 * IMPORTANT: The layout functions and the corresponding size-calculation
 * functions must always be kept 100% in sync!
 *
 * @see KTMainWindow
 * @short Special layout manager for KTMainWindow.
 * @author Chris Schlaeger (cs@kde.org) */

class KTMLayout : public QLayout
{
public:
	/**
	 * The constructor. It probably makes no use to call this version. */
	KTMLayout()
	{
		topMenuBar = bottomMenuBar = statusBar = 0;
		mainItem = 0;
	}
	/**
	 * The constructor. */
	KTMLayout(QWidget *parent, int border = 0, int space = -1,
			  const char *name = 0);

	~KTMLayout() { }

	/**
	 * Register the top menu bar. There can only be one.  */
	void addTopMenuBar(QWidget* w)
	{
		topMenuBar = w;
	}

	/**
	 * Register the bottom menu bar. There can only be one.  */
	void addBottomMenuBar(QWidget* w)
	{
		bottomMenuBar = w;
	}

	/**
	 * Call this function to register all bars that are
	 * flattened. This can tool bars as well as menu bars.  */
	void addFlatBar(QWidget* w)
	{
		flatBars.append(w);
	}

	/**
	 * Register top tool bars. There is no limit in the number of tool
	 * bars that you can register.  */
	void addTopToolBar(KToolBar* w)
	{
		topToolBars.append(w);
	}

	/**
	 * Register left hand side tool bars. There is no limit in the
	 * number of tool bars that you can register.  */
	void addLeftToolBar(KToolBar* w)
	{
		leftToolBars.append(w);
	}

	/**
	 * Register right hand side tool bars. There is no limit in the
	 * number of tool bars that you can register.  */
	void addRightToolBar(KToolBar* w)
	{
		rightToolBars.append(w);
	}

	/**
	 * Register bottom tool bars. There is no limit in the number of
	 * tool bars that you can register.  */
	void addBottomToolBar(KToolBar* w)
	{
		bottomToolBars.append(w);
	}

	/**
	 * Use this function to register the main widget. There must be
	 * exactly one main widget!  */
	void addMainItem(QWidget* w)
	{
		mainItem = w;
	}

	/**
	 * Use this function to register the status bar. There can only be
	 * one. */
	void addStatusBar(QWidget* w)
	{
		statusBar = w;
	}


	/**
	 * This function arranges the widgets. It determines the necessary
	 * space for the toolbars and assignes the remainder to the main
	 * widget.  */
	void setGeometry(const QRect& rect);

	/**
	 * Returns the preferred size of the layout. */
	QSize sizeHint(void) const;

	/**
	 * Returns the minimum required size of the layout. */
	QSize minimumSize(void) const;

	/**
	 * Never use this function. It is provided to be compliant with Qt. */
	void addItem(QLayoutItem* item);

	/**
	 * Never use this function. It is provided to be compliant with Qt. */
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

	/**
	 * This is only provided to support a Qt compliant interface. */
	QList<QLayoutItem> list;

	/**
	 * The following variables store the references to the items that are 
	 * managed by this layout manager. */
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

/**
 * Auxillary class for KTMLayout.
 */
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
