/* This file is part of the KDE libraries
   Copyright (C) 1999 Matthias Ettrich <ettrich@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/
#ifndef KDOCKWINDOW_H
#define KDOCKWINDOW_H

#include <qlabel.h>

class KPopupMenu;
class KDockWindowPrivate;

/**
 *  KDE Dock Window class
 *
 * This class implements dock windows.
 *
 * A dock window is a small window (typically 24x24 pixel) that docks
 * into the desktop panel. It usually displays an icon or an animated
 * icon there. The icon serves as representative for the application,
 * similar to a taskbar button, but consumes less screen space.
 *
 * When the user clicks with the left mouse button on the icon, the
 * main application window is shown/raised and activated. With the
 * right mouse button, she gets a popupmenu with application specific
 * commands, including "Minimize/Restore" and "Quit".
 * 
 * Docking happens magically when calling show(). The window undocks
 * with either hide() or when it is destroyed.
 *
 * KDockWindow inherits methods such as @ref setPixmap() and @ref setMovie() to
 * specify an icon or movie (animated icon) respectively. It is
 * designed to be usable "as is", without the need to subclass it. In
 * case you need to provide something special (such as an additional
 * popupmenu on a click with the left mouse button), you can subclass
 * anyway, of course.
 *
 * Docking is a useful technique for daemon-like applications that may
 * run for some time without user interaction but have to be there
 * immediately when the user needs them. Examples are kppp, kisdn, kscd,
 * kmix or knotes. With kppp and kisdn, the docked icon even provides
 * real-time information about the network status.
 *
 * @author Matthias Ettrich <ettrich@kde.org>
 * @short KDE Dock Window class
 **/
class KDockWindow : public QLabel
{
    Q_OBJECT
public:

    /**
     * Construct a KDockWindow widget just like any other widget.
     *
     * The parent widget @p parent has a special meaning:
     * Besides owning the dock window, the parent widget will
     * dissappear from taskbars when it is iconified while the dock
     * window is visible. This is the desired behaviour. After all,
     * the dock window @p is the parent's taskbar icon.
     *
     * Furthermore, the parent widget is shown or raised respectively
     * when the user clicks on the dock window with the left mouse
     * button.
     **/
    KDockWindow( QWidget* parent = 0, const char* name  = 0 );

    /*
      Destructor
     */
    ~KDockWindow();


protected:

    /**
       Reimplemented to provide the standard show/raise behaviour
       for the parentWidget() and the context menu.

       Feel free to reimplement this if you need something special.
     */
    void mousePressEvent( QMouseEvent * );

    /**
       Reimplemented to provide the standard show/raise behaviour
       for the parentWidget() and the context menu.

       Feel free to reimplement this if you need something special.
     */
    void mouseReleaseEvent( QMouseEvent * );

    /**
       Access to the context menu. This makes it easy to add new items
       to it.
     */
    KPopupMenu* contextMenu();


    /**
       Makes it easy to adjust some menu items right before the
       context menu becomes visible.
     */
    virtual void contextMenuAboutToShow( KPopupMenu* menu );
    
    /**
       Reimplemented for internal reasons.
     */
    void showEvent( QShowEvent * );
    
private slots:
    void toggleMinimizeRestore();

private:
    KPopupMenu* menu;
    KDockWindowPrivate* d;
    int minimizeRestoreId;
    uint hasQuit :1;
};


#endif
