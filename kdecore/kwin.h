/* This file is part of the KDE libraries
    Copyright (C) 1999 Matthias Ettrich (ettrich@kde.org)

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
#ifndef KWIN_H
#define KWIN_H

#include <sys/types.h>
#include <qwindowdefs.h>
#include <qstring.h>
#include <qpixmap.h>

#include "netwm_def.h"

/**
 * Convenience access to certain properties and features of the
 * window manager.
 *
 * This class is not supposed to be instantiated.  It exists mostly as
 * a namespace for the static member functions.
 *
 * In KDE 2 and KDE 3, communication with the windowmanager is done with the
 * NET-protocol, a common window manager specification designed by
 * various authors of X11 window managers (including those of the KDE
 * project).  The full specification can be found in
 * kdebase/kwin/wm-spec/
 *
 * To access features of the NET-protocol, use the classes NETRootInfo
 * and NETWinInfo.
 *
 * The purpose of this class is to to provide easy access to the
 * most-commonly used NET-features with a simpler, KDEish interface.
 *
 * In addition, it encapsulates KDE functionality not yet provided by
 * the NET hints.  Currently that is invokeContextHelp() and
 * setSystemTrayWindowFor() only.
 *
 * @short Class for interaction with the window manager.
 * @author Matthias Ettrich (ettrich@kde.org)
*/
class KWin
{
public:

    /**
     * Sets window @p win to be the active window.
     *
     * This is a request to the window manager. It may or may not be
     * obeyed.
    */
    static void setActiveWindow( WId win);

    /**
     * Invokes interactive context help.
     */
    static void invokeContextHelp();


    /**
     * Makes @p trayWin a system tray window for @p forWin.
     *
     * A system tray window serves as an icon replacement. It's
     * displayed inside the panel's system tray.
     */
    static void setSystemTrayWindowFor( WId trayWin, WId forWin );


    struct Info
    {
	WId win;
 	long unsigned int state;
	NET::MappingState mappingState;
	NETStrut strut;
	NET::WindowType windowType;
	QString visibleName;
	QString name;
	int desktop;
	bool onAllDesktops;
	pid_t pid;
	bool isIconified() const { return mappingState == NET::Iconic; }
	QRect geometry;
	QRect frameGeometry;

	/** Returns a visible name with state.
	 *
	 * This is a simple convenience function that returns the
	 * visible name but with parentheses around iconified windows
	 */
	QString visibleNameWithState() const;
    };

    /**
     * Returns information about window @p win.
     */
    static Info info( WId win );

    /**
     * Returns an icon for window @p win.
     *
     * If  @p width and @p height are specified, the best icon for the requested
     * size is returned.
     *
     * If @p scale is true, the icon is smooth-scaled to have exactly
     * the requested size.
     *
     */
    static QPixmap icon( WId win, int width = -1, int height = -1, bool scale = false );


    /**
     * Sets an @p icon and a  @p miniIcon on window @p win
     */
    static void  setIcons( WId win, const QPixmap& icon, const QPixmap& miniIcon );

    /**
     * Sets the type of window @p win to @p windowType.
     *
     * Possible values are NET::Normal, NET::Desktop, NET::Dock,
     * NET::Tool, NET::Menu, NET::Dialog, NET::Override (== no
     * decoration frame) or NET::TopMenu
     */
    static void setType( WId win, NET::WindowType windowType );

    /**
     * Sets the state of window @p win to @p state.
     *
     * Possible values are or'ed combinations of NET::Modal,
     * NET::Sticky, NET::MaxVert, NET::MaxHoriz, NET::Shaded,
     * NET::SkipTaskbar, NET::StaysOnTop, NET::SkipPager
     */
    static void setState( WId win, unsigned long state );

    /**
     * Clears the state of window @p win from @p state.
     *
     * Possible values are or'ed combinations of NET::Modal,
     * NET::Sticky, NET::MaxVert, NET::MaxHoriz, NET::Shaded,
     * NET::SkipTaskbar, NET::StaysOnTop, NET::SkipPager
     */
    static void clearState( WId win, unsigned long  state );

    /**
     * Sets window @p win to be present on all virtual desktops if @p
     * is true. Otherwise the window lives only on one single desktop.
     */
    static void setOnAllDesktops( WId win, bool b );

    /**
     * Moves window @p win to desktop @p desktop.
     */
    static void setOnDesktop( WId win, int desktop);

    /**
     * Sets the strut of window @p win to @p left, @p right, @p top, @p bottom.
     *
     */
    static void setStrut( WId win, int left, int right, int top, int bottom );

    /**
     * Convenience function to access the current desktop.  See NETRootInfo.
     */
    static int currentDesktop();

    /**
     * Convenience function to access the number of desktops. See
     * NETRootInfo.
     */
    static int numberOfDesktops();

    /**
     * Convenience function to set the current desktop to @p desktop.
     * See NETRootInfo.
     */
    static void setCurrentDesktop( int desktop );


    /**
     * Iconifies a window. Compatible to XIconifyWindow but has an
     * additional parameter @p animation.
     *
     * @ref deIconifyWindow()
     */
    static void iconifyWindow( WId win, bool animation = true  );

    /**
     * DeIconifies a window. Compatible to XMapWindow but has an
     * additional parameter @p animation.
     *
     * @ref iconifyWindow()
     */
    static void deIconifyWindow( WId win, bool animation = true );

#ifndef KDE_NO_COMPAT
    /**
     * @deprecated
     * Use @ref KStartupInfo::appStarted
     */
    static void appStarted();
#endif
};

#endif
