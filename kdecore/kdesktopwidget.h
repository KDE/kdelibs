/* This file is part of the KDE libraries
    Copyright (C) 2001 Balaji Ramani (balaji@yablibli.com)

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

#ifndef KDESKTOPWIDGET_H
#define KDESKTOPWIDGET_H

#include <qglobal.h>

#if QT_VERSION < 300

/**
 * This class is to support multi head/xinerama.  It is mainly a copy of 
 * QDesktopWidget in QT 3.  Note that this class can go away when KDE is
 * ported to QT 3.
 */

#include <qwidget.h>
#include <qpoint.h>
#include <qrect.h>

class KDesktopWidgetPrivate;

/**
 * Provides functions to check for multi head and xinerama enabled
 * desktops.  This class has a similar interface as the QDesktopWidget
 * class in QT 3.  Once kdelibs is ported to QT 3, this class can be 
 * a dummy class that extends QDesktopWidget.
 * 
 * @short Class to provide multi head/xinerama information
 * @author Balaji Ramani (balaji@yablibli.com)
 */
class KDesktopWidget : public QWidget
{
	Q_OBJECT
public:
	KDesktopWidget();
	~KDesktopWidget();

	/**
	 *
	 * Returns <pre>true</pre> if Xinerama is enabled for
	 * the current desktop.
	 */
	bool isVirtualDesktop() const;

	/**
	 *
	 * Returns the number of screens that make up the desktop.
	 *
	 */
	int numScreens() const;

	/**
	 *
	 * Returns the primary screen of the desktop.
	 *
	 */
	int primaryScreen() const;

	/**
	 *
	 * Returns the screen number given a widget.  If the widget
	 * overlaps two screens, returns the screen number on which
	 * the major portion of the widget lies.
	 *
	 */
	int screenNumber( QWidget *widget = 0 ) const;

	/**
	 *
	 * Returns the screen number on which the point falls on.
	 *
	 */
	int screenNumber( const QPoint & ) const;

	/**
	 *
	 * Returns a widget representing the physical screen in a 
	 * multi-headed display.  This is NOT implemented correctly
	 * in this version.  The code in QT 3 works correctly.
	 * 
	 */
	QWidget *screen( int screen = -1 );

	/**
	 *
	 * Returns the dimension of a screen.  By default, it returns
	 * the dimension of the primary screen.
	 *
	 */
	const QRect & screenGeometry( int screen = -1 ) const;

private:
	KDesktopWidgetPrivate *d;
};

#endif // QT_VERSION < 300

#endif
