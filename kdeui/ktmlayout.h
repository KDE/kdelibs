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
class KHTBLayout : public QHBoxLayout
{
public:
	KHTBLayout()
	{
		mainItem = 0;
	}
	KHTBLayout(QWidget *parent, int border = 0, int space = -1,
			   const char *name = 0);

	~KHTBLayout() { }

	/**
	 * Use this function to add the main widget. All toolbars can be added
	 * with addItem(). There must be exactly one main widget!
	 */
	void addMainItem(QWidget* w, int stretch = 0, int alignment = 0);

	/**
	 * This function arranges the widgets. It determines the necessary space
	 * for the toolbars and assignes the remainder to the main widget.
	 */
	void setGeometry(const QRect& rect);

private:
	QWidget* mainItem;
} ;

/**
 * KVTBLayout is a specialized version of QVBoxLayout. It can be used when
 * you want to manage vertically aligned widgets that consist of one main
 * widget and other toolbar-like widgets. The toolbar like widgets must have
 * a fixed height that can depend on their width. The main widget takes the
 * rest of the space. The toolbars must define a heightForWidth(int) function.
 * The most prominent user of this class is KTMainWidget.
 *
 * @see KTMainWindow
 * @short Special layout managers for use with toolbar-like widgets.
 * @author Chris Schlaeger (cs@kde.org)
 */
class KVTBLayout : public QVBoxLayout
{
public:
	KVTBLayout()
	{
		mainItem = 0;
	}
	KVTBLayout(QWidget *parent, int border = 0, int space = -1,
			   const char *name = 0);

	~KVTBLayout() { }

	/**
	 * Use this function to add the main item. All toolbars can be added
	 * with addItem(). There must be exactly one main widget!
	 */
	void addMainLayout(QLayout* w, int stretch = 0);
	/**
	 * Use this function to add the main widget. All toolbars can be added
	 * with addItem(). There must be exactly one main item!
	 */
	void addMainItem(QWidget* w, int stretch = 0, int alignment = 0);

	/**
	 * This function arranges the widgets. It determines the necessary space
	 * for the toolbars and assignes the remainder to the main widget.
	 */
	void setGeometry(const QRect& rect);

private:
	QWidget* mainItem;
} ;

#endif
