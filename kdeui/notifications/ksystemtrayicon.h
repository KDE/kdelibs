/* This file is part of the KDE libraries
   Copyright (C) 1999 Matthias Ettrich <ettrich@kde.org>
   Copyright (C) 2007      by Charles Connell <charles@connells.org>
   Copyright (C) 2008 Lukas Appelhans <l.appelhans@gmx.de>

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
class QAction;
class QMovie;

/**
 * \brief %KDE System Tray Window class
 *
 * This class implements system tray windows.
 *
 * A tray window is a small window (typically 22x22 pixel) that docks
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
 * Please note that this class is being phased out in favor of the KStatusNotifierItem
 * class, you should consider to use it instead if you are writing a new application
 * or consider porting the code that uses this class to the KStatusNotifierItem API.
 *
 * Also, QSystemTrayIcon::showMessage(..) should not be
 * used for KDE application because the popup message has no KDE standard
 * look & feel and cannot be controlled by KDE configurations. 
 * Use KNotification or KPassivePopup instead.
 *
 * @author Matthias Ettrich <ettrich@kde.org>
 **/
class KDEUI_EXPORT KSystemTrayIcon : public QSystemTrayIcon //krazy:exclude=qclasses
{
    Q_OBJECT
public:
    /**
     * Construct a system tray icon.
     *
     * The parent widget @p parent has a special meaning:
     * Besides owning the tray window, the parent widget will
     * disappear from taskbars when it is iconified while the tray
     * window is visible. This is the desired behavior. After all,
     * the tray window @p is the parent's taskbar icon.
     *
     * Furthermore, the parent widget is shown or raised respectively
     * when the user clicks on the tray window with the left mouse
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

    /**
     * Same as above but allows one to define the movie by QMovie that should
     * be used for the system tray icon. Memory management for the movie will
     * be handled by KSystemTrayIcon.
     */
    explicit KSystemTrayIcon(QMovie* movie, QWidget* parent);

    /**
     * Destructor
     */
    ~KSystemTrayIcon();

    /**
     * Set the movie to use. To manipulate the movie (start, stop, pause), call
     * @see movie() and make calls on the QMovie* that it returns.
     * Memory management for the movie will be handled by KSystemTrayIcon.
     * @since 4.2
     */
    void setMovie(QMovie* movie);

    /**
     * Get a pointer to the movie. Use this pointer to manipulate the movie
     * (start, stop, pause).
     * Will return null if no movie has been set
     * @since 4.2
     */
    const QMovie* movie() const;

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

    /**
     * Sets the context menu title action to @p action.
     * The following code shows how to change the current title.
     * <code>
     * QAction *titleAction = contextMenuTitle();
     * titleAction->setText("New Title");
     * setContextMenuTitle(titleAction);
     * </code>
     * @since 4.1
     */
    void setContextMenuTitle(QAction *action);

    /**
     * Returns the context menu title action.
     * @since 4.1
     */
    QAction *contextMenuTitle() const;

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

Q_PRIVATE_SLOT(d, void _k_slotNewFrame())
};

#endif

