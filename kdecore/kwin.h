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
 * To access features of the NET-protocol, use the classes @ref NETRootInfo
 * and @ref NETWinInfo.
 *
 * The purpose of this class is to to provide easy access to the
 * most-commonly used NET-features with a simpler, KDEish interface.
 *
 * In addition, it encapsulates KDE functionality not yet provided by
 * the NET hints.  Currently that is invokeContextHelp() and
 * setSystemTrayWindowFor() only.
 *
 * @short Class for interaction with the window manager.
 * @see NET
 * @see NetWinInfo
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
     * @param win the if of the window to make active
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
     * @param trayWin the id of the system tray window
     * @param trayWin the id of the window represented by the system
     *        tray window
     */
    static void setSystemTrayWindowFor( WId trayWin, WId forWin );


    /**
     * Information about a window.
     */
    struct Info
    {
      /// The window's id.
	WId win;
      /// The window's state.
 	long unsigned int state;
      /// The mapping state.
	NET::MappingState mappingState;
      /// The strut.
	NETStrut strut;
      /// The window type.
	NET::WindowType windowType;
      /// The visible name of the window.
	QString visibleName;
      /// The name of the window.
	QString name;
      /// The number of the window's desktop.
	int desktop;
      /// true if the window is on all desktops.
	bool onAllDesktops;
      /// The process id of the window's owner
	pid_t pid;
      /**
       * Checks whether the window is iconified (minimized).
       * @return true if iconified
       */
	bool isIconified() const { return mappingState == NET::Iconic; }
      /// Position and size of the window contents.
	QRect geometry;
      /// Position and size of the window's frame.
	QRect frameGeometry;

	/** Returns a visible name with state.
	 *
	 * This is a simple convenience function that returns the
	 * visible name but with parentheses around iconified windows
	 * @return the window name with state
	 */
	QString visibleNameWithState() const;
    };

    /**
     * Returns information about window @p win.
     * @param win the id of the windoe
     * @return the window information
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
     * @param win the id of the window
     * @param the desired width, or -1
     * @param the desired height, or -1
     * @param if true the icon will be scaled to the desired size. Otherwise the
     *        icon will not be modified.
     * @return the icon of the window
     */
    static QPixmap icon( WId win, int width = -1, int height = -1, bool scale = false );


    /**
     * Sets an @p icon and a  @p miniIcon on window @p win
     * @param win the id of the window
     * @param icon the new icon
     * @param miniIcon the new mini icon
     */
    static void  setIcons( WId win, const QPixmap& icon, const QPixmap& miniIcon );

    /**
     * Sets the type of window @p win to @p windowType.
     *
     * Possible values are NET::Normal, NET::Desktop, NET::Dock,
     * NET::Tool, NET::Menu, NET::Dialog, NET::Override (== no
     * decoration frame) or NET::TopMenu
     *
     * @param win the id of the window
     * @param windowType the type of the window
     */
    static void setType( WId win, NET::WindowType windowType );

    /**
     * Sets the state of window @p win to @p state.
     *
     * Possible values are or'ed combinations of NET::Modal,
     * NET::Sticky, NET::MaxVert, NET::MaxHoriz, NET::Shaded,
     * NET::SkipTaskbar, NET::SkipPager, NET::Hidden,
     * NET::FullScreen, NET::Above, NET::Below, NET::StaysOnTop
     *
     * @param win the id of the window
     * @param state the new flags that will be set
     */
    static void setState( WId win, unsigned long state );

    /**
     * Clears the state of window @p win from @p state.
     *
     * Possible values are or'ed combinations of NET::Modal,
     * NET::Sticky, NET::MaxVert, NET::MaxHoriz, NET::Shaded,
     * NET::SkipTaskbar, NET::SkipPager, NET::Hidden,
     * NET::FullScreen, NET::Above, NET::Below, NET::StaysOnTop
     *
     * @param win the id of the window
     * @param state the flags that will be cleared
     */
    static void clearState( WId win, unsigned long  state );

    /**
     * Sets window @p win to be present on all virtual desktops if @p
     * is true. Otherwise the window lives only on one single desktop.
     *
     * @param win the id of the window
     * @param b true to show the window on all desktops, false
     *          otherwise
     */
    static void setOnAllDesktops( WId win, bool b );

    /**
     * Moves window @p win to desktop @p desktop.
     *
     * @param win the id of the window
     * @param desktop the number of the new desktop
     */
    static void setOnDesktop( WId win, int desktop);

    /**
     * Sets the strut of window @p win to @p left, @p right, @p top, @p bottom.
     *
     * @param win the id of the window
     * @param left the left strut
     * @param right the right strut
     * @param top the top strut
     * @param bottom the bottom strut
     */
    static void setStrut( WId win, int left, int right, int top, int bottom );

    /**
     * Convenience function to access the current desktop.  See @ref NETRootInfo.
     * @return the number of the current desktop
     */
    static int currentDesktop();

    /**
     * Convenience function to access the number of desktops. See @ref
     * NETRootInfo.
     * @return the number of desktops
     */
    static int numberOfDesktops();

    /**
     * Convenience function to set the current desktop to @p desktop.
     * See @ref NETRootInfo.
     * @param the number of the new desktop
     */
    static void setCurrentDesktop( int desktop );


    /**
     * Iconifies a window. Compatible to XIconifyWindow but has an
     * additional parameter @p animation.
     *
     * @param win the id of the window
     * @param animation true to show an animation
     * @see deIconifyWindow()
     */
    static void iconifyWindow( WId win, bool animation = true  );

    /**
     * DeIconifies a window. Compatible to XMapWindow but has an
     * additional parameter @p animation.
     *
     * @param win the id of the window
     * @param animation true to show an animation
     * @see iconifyWindow()
     */
    static void deIconifyWindow( WId win, bool animation = true );

#ifdef KDE_NO_COMPAT
private:
#endif
    /**
     * @deprecated
     * Use @ref KStartupInfo::appStarted
     */
    static void appStarted();
};

#endif
