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

#include <qstring.h>
#include <qapplication.h>
#include <qpixmap.h>

#undef WithdrawnState
#undef NormalState
#undef IconicState

/**
 * Get or modify window properties and to interact with the
 * windowmanager.
 *
 * This class is not supposed to be instantiated.  It exists mostly as
 * a namespace for the static member functions.
 *
 * It implements the new NET-protocol, a common window manager
 * specification designed by various authors of X11 window managers.
 * TODO: Add more information here, links etc. once the spec is done
 * and fully implemented.
 *
 * @short Class for interaction with the windowmanager.
 * @authors Matthias Ettrich (ettrich@kde.org), Rik Hemsley <rik@kde.org>
*/
class KWin
{
public:

  enum AnchorEdge { Top, Bottom, Left, Right };

    /**
     * Rerieve the number of virtual desktops.
     **/
  static int numberOfDesktops();

    /**
     * Set the number of virtual desktops.
     *
     * This is a request to the window manager. It may or may not be
     * obeyed.
     */
    static void setNumberOfDesktops(int num);

    /**
     * The current virtual desktop.
     *
     * @return On success a number > 0 indicating the desktop.
     *         0 if the desktop is unknown.   
     */
    static int currentDesktop();


    /**
     * Set the current virtual desktop
     *
     * This is a request to the window manager. It may or may not be
     * obeyed.
     **/
    static void setCurrentDesktop( int desktop );

    /**
     * Retrieve the window which has the focus, or 0 if no window has the focus.
    */
    static WId activeWindow();


    /**
     * Set the active window.
     *
     * This is a request to the window manager. It may or may not be
     * obeyed.
    */
    static void setActiveWindow( WId win);


    /**
     * Register the window @p dockWin as docking window for window
     * @p forWin.
     *
     * This is a low-level API. See @ref KDockWindow in libkdeui for
     * easier usage.
     */
    static void setDockWindow(WId dockWin, WId forWin  = 0 );

    /**
     * Test whether the window @p dockWin is a docking window.
     *
     *  If
     * @p forWin is specified, @p forWin returns the window @p dockWin is
     * docking for.
     */
    static bool isDockWindow( WId dockWin, WId *forWin = 0 );


    enum WindowState { WithdrawnState = 0, NormalState = 1, IconicState = 3 };
    /**
     * Retrieve the state of the window @p win.
     *
     * Possible states are
     * @p NormalState, @p IconicState and @p WithdrawnState
     */
    static WindowState windowState( WId win );
    
    
    /**
     * Invoke interactive context help.
     */
    static void invokeContextHelp();

    /**
     * Add the window @p win to the list of those that should be avoided
     * when maximising, mapping new windows etc.
     *
     * Use @p edge to specify which edge your window is stuck to.
     *
     * You can call this function as many times as you like. kwin
     * will just forget about the old edge you were anchored to, so
     * you can move to another one if you want.
     */
    static void avoid(WId win, AnchorEdge edge);

    /**
     * Query whether a window with WId @p win should be avoided.
     * Ugh, better naming required.
     */
    static bool avoid(WId win);

    /**
     * Remove the window @p win from the list of those that should be avoided.
     *
     * This is useful if your window can be hidden in some way, e.g.
     * kicker can be scrolled off-screen.
     */
    static void stopAvoiding(WId win);

    /**
     * Force a re-check of the area available to clients.
     */
    static void updateClientArea();

    /**
     * @return area available to clients. This is adjusted from the
     * desktop size to allow for edge-anchored windows.
     */
    static QRect clientArea();
    
    /**
     * @return area available to edge-anchored windows. This is adjusted
     * from the desktop size to allow for other edge-anchored windows which
     * have priority.
     */
    static QRect edgeClientArea();

    /**
     * @return pid of process that owns this window, or -1 if unavailable.
     */
    static long pid(WId win);
};

#endif
