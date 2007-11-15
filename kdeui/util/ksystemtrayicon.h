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

#include <kdeui_export.h>
#include <kglobal.h>

#include <QtGui/QSystemTrayIcon>

class KActionCollection;
class KSystemTrayIconPrivate;

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
 * @author Matthias Ettrich <ettrich@kde.org>
 **/
class KDEUI_EXPORT KSystemTrayIcon : public QSystemTrayIcon
{
    Q_OBJECT
public:

    /**
     * Construct a system tray icon.
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
    explicit KSystemTrayIcon( QWidget* parent = 0 );

    /**
     * Same as above but allows one to define the icon by name that should
     * be used for the system tray icon.
     */
    explicit KSystemTrayIcon( const QString& icon, QWidget* parent = 0 );

    /**
     * Same as above but allows one to define the icon by name that should
     * be used for the system tray icon.
     */
    explicit KSystemTrayIcon( const QIcon& icon, QWidget* parent = 0 );

    /*
      Destructor
     */
    ~KSystemTrayIcon();

    /**
       Easy access to the actions in the context menu
       Currently includes KStandardAction::Quit and minimizeRestore
    */
    KActionCollection* actionCollection();

    /** 
       Returns the QWidget set by the constructor
    */
    QWidget *parentWidget() const;

    /**
       Function to be used from function handling closing of the window associated 
       with the tray icon (i.e. QWidget::closeEvent(), KMainWindow::queryClose() or 
       similar). When false is returned, the window closing should proceed normally, 
       when true is returned, special systray-related handling should take place.
     */
    bool parentWidgetTrayClose() const;

    /**
     * Loads an icon @p icon using the icon loader class of the given componentData @p componentData.
     * The icon is applied the panel effect as it should only be used to be shown in the
     * system tray.
     * It's commonly used in the form : systray->setPixmap( systray->loadIcon( "mysystray" ) );
     */
    static QIcon loadIcon(const QString &icon, const KComponentData &componentData = KGlobal::mainComponent());

Q_SIGNALS:
    /**
     * Emitted when quit is selected in the menu. If you want to perform any other
     * action than to close the main application window please connect to this signal.
     */
    void quitSelected();

public Q_SLOTS:
    void toggleActive();

private Q_SLOTS:
    void contextMenuAboutToShow();
    void minimizeRestoreAction();
    void maybeQuit();
    void activateOrHide( QSystemTrayIcon::ActivationReason reasonCalled );

private:
    void init( QWidget* );
    void minimizeRestore( bool restore );

    KSystemTrayIconPrivate* const d;
};

#endif

