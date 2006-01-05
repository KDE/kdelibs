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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#ifndef KSYSTEMTRAY_H
#define KSYSTEMTRAY_H

#include <kglobal.h>
#include <qlabel.h>

class KActionCollection;
class KMenu;
class KSystemTrayPrivate;

/**
 * \brief %KDE System Tray Window class
 *
 * This class implements system tray windows.
 *
 * A tray window is a small window (typically 24x24 pixel) that docks
 * into the system tray in the desktop panel. It usually displays an
 * icon or an animated icon there. The icon represents
 * the application, similar to a taskbar button, but consumes less
 * screen space.
 *
 * When the user clicks with the left mouse button on the icon, the
 * main application window is shown/raised and activated. With the
 * right mouse button, she gets a popupmenu with application specific
 * commands, including "Minimize/Restore" and "Quit".
 *
 * Docking happens magically when calling show(). The window undocks
 * with either hide() or when it is destroyed.
 *
 * KSystemTray inherits methods such as setPixmap() and setMovie() to
 * specify an icon or movie (animated icon) respectively. It is
 * designed to be usable "as is", without the need to subclass it. In
 * case you need to provide something special (such as an additional
 * popupmenu on a click with the left mouse button), you can subclass
 * anyway, of course.
 *
 * Having an icon on the system tray is a useful technique for
 * daemon-like applications that may run for some time without user
 * interaction but have to be there immediately when the user needs
 * them. Examples are kppp, kisdn, kscd, kmix or knotes. With kppp and
 * kisdn, the docked icon even provides real-time information about
 * the network status.
 *
 * @author Matthias Ettrich <ettrich@kde.org>
 **/
class KDEUI_EXPORT KSystemTray : public QLabel
{
    Q_OBJECT
public:

    /**
     * Construct a KSystemTray widget just like any other widget.
     *
     * The parent widget @p parent has a special meaning:
     * Besides owning the tray window, the parent widget will
     * dissappear from taskbars when it is iconified while the tray
     * window is visible. This is the desired behavior. After all,
     * the tray window @p is the parent's taskbar icon.
     *
     * Furthermore, the parent widget is shown or raised respectively
     * when the user clicks on the trray window with the left mouse
     * button.
     **/
    KSystemTray( QWidget* parent = 0 );

    /*
      Destructor
     */
    ~KSystemTray();

    /**
       Access to the context menu. This makes it easy to add new items
       to it.
     */
    KMenu* contextMenu() const;

    /**
       Easy access to the actions in the context menu
       Currently includes KStdAction::Quit and minimizeRestore
       @since 3.1
    */
    KActionCollection* actionCollection();

    /**
     * Changes the tray's icon.
     */
    virtual void setPixmap( const QPixmap& icon );

    /**
     * Changes the tray's text description (which can be seen e.g. in the systray
     * configuration dialog). The default value is KAboutData::programName().
     */
    virtual void setCaption( const QString& title );
    
    /**
     * Loads an icon @p icon using the icon loader class of the given instance @p instance.
     * The icon is applied the panel effect as it should only be used to be shown in the
     * system tray.
     * It's commonly used in the form : systray->setPixmap( systray->loadIcon( "mysystray" ) );
     *
     * @since 3.2
     */
    static QPixmap loadIcon( const QString &icon, KInstance *instance=KGlobal::instance() );

signals:
    /**
     * Emitted when quit is selected in the menu. If you want to perform any other
     * action than to close the main application window please connect to this signal.
     * @since 3.1
     */
    void quitSelected();

public slots:

    /**
     * Toggles the state of the window associated with this system tray icon (hides it,
     * shows it or activates it depending on the window state). The default implementation
     * of mousePressEvent() calls toggleActive() when the tray icon is left-clicked, use
     * it when reimplementing mousePressEvent().
     * @since 3.3
     */
    void toggleActive();
    /**
     * Activates the window associated with this system tray icon, regardless of its current state.
     * @since 3.3
     */
    void setActive();
    /**
     * Hides the window associated with this system tray icon, regardless of its current state.
     * @since 3.3
     */
    void setInactive();

protected:

    /**
       Reimplemented to provide the standard show/raise behavior
       for the parentWidget() and the context menu.

       Feel free to reimplement this if you need something special.
     */
    void mousePressEvent( QMouseEvent * );

    /**
       Reimplemented to provide the standard show/raise behavior
       for the parentWidget() and the context menu.

       Feel free to reimplement this if you need something special.
     */
    void mouseReleaseEvent( QMouseEvent * );



    /**
       Makes it easy to adjust some menu items right before the
       context menu becomes visible.
     */
    virtual void contextMenuAboutToShow( KMenu* menu );

    /**
       Reimplemented for internal reasons.
     */
    void showEvent( QShowEvent * );


private slots:
    void minimizeRestoreAction();
    void maybeQuit();

private:
    void activateOrHide();
    void minimizeRestore( bool restore );
    KMenu* menu;
    // minimizeRestoreId is no longer needed. remove in KDE 4.0
    int minimizeRestoreId;
    uint hasQuit :1;
protected:
    virtual void virtual_hook( int id, void* data );
private:
    KSystemTrayPrivate* d;
};


#endif
