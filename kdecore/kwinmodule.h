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
/*
 * kwinmodule.h. Part of the KDE project.
 */

#ifndef KWINMODULE_H
#define KWINMODULE_H

#include <qobject.h>
#include <qvaluelist.h>

class KWinModulePrivate;

/**
 *
 * The class KWinModule provides information about the state of the
 * window manager as required by windowmanager modules. It informs a
 * module about all currently managed windows and changes to them (via
 * Qt signals).
 *
 * KWinModule uses NETRootInfo internally. Modules written with this
 * class will work fine under any window manager that implements the
 * NET_WM protocol.
 *
 * There are no methods to manipulate windows. Those are defined in
 * the classes KWin, NETWinInfo and NETRootInfo.
 *
 *
 * @short Base class for KDE Window Manager modules.
 * @author Matthias Ettrich (ettrich@kde.org)
 * @version $Id$
 */
class KWinModule : public QObject
{
    Q_OBJECT

public:

    /**
     * Creates a KWinModule object and connects to the window
     * manager.
     * @param parent the parent for the @ref QObject
     **/
    KWinModule( QObject* parent = 0 );

    /**
     * Destructor. Internal cleanup, nothing fancy.
     **/
    ~KWinModule();

    /**
     * Returns the list of all toplevel windows currently managed by the
     * window manager in the order of creation. Please do not rely on
     * indexes of this list: Whenever you enter Qt's event loop in your
     * application, it may happen that entries are removed or added.
     * Your module should perhaps work on a copy of this list and verify a
     * window with @ref hasWindow() before any operations.
     *
     * Iteration over this list can be done easily with
     * <pre>
     *  QValueList<WId>::ConstIterator it;
     *  for ( it = module->windows().begin();
     *        it != modules->windows().end(); ++it ) {
     *     ... do something here,  (*it) is the current WId.
     *       }
     * </pre>
     * @return the list of all toplevel windows
     */
    const QValueList<WId>& windows() const;

    /**
     * Returns the list of all toplevel windows currently managed by the
     * window manager in the current stacking order (from lower to
     * higher). May be useful for pagers.
     * @return the list of all toplevel windows in stacking order
     */
    const QValueList<WId>& stackingOrder() const;

    /**
     * Test to see if @p id still managed at present.
     * @param id the window id to test
     * @return true if the window id is still managed
     **/
    bool hasWId(WId id) const;

    /**
     * Returns a list of the system tray windows.
     * @return a list of all system tray windows
     **/
    const QValueList<WId>& systemTrayWindows() const;

    /**
     * Returns the current virtual desktop.
     * @return the current virtual desktop
     **/
    int currentDesktop() const;

    /**
     * Returns the number of virtual desktops.
     * @return the number of virtual desktops
     **/
    int numberOfDesktops() const;

    /**
     * Returns the currently active window, or 0 if no window is active.
     * @return the window id of the active window, or 0 if no window is 
     *  active
     **/
    WId activeWindow() const;

    /**
     * Returns the workarea for the specified desktop, or the current
     * work area if no desktop has been specified.
     * @param desktop the number of the desktop to check, -1 for the
     *        current desktop
     * @return the size and position of the desktop
     **/
    QRect workArea( int desktop = - 1 ) const;


    /**
     * Returns the workarea for the specified desktop, or the current
     * work area if no desktop has been specified. Excludes struts of
     * clients in the exclude List.
     * 
     * @param clients the list of clients whose struts will be excluded
     * @param desktop the number of the desktop to check, -1 for the
     *        current desktop
     * @return the size and position of the desktop
     **/
    QRect workArea( const QValueList<WId> &excludes, int desktop = -1) const;

    /**
     * Returns the name of the specified desktop.
     * @param desktop the number of the desktop
     * @return the name of the desktop
     **/
    QString desktopName( int desktop ) const;

    /**
     * Sets the name of the specified desktop.
     * @param desktop the number of the desktop
     * @param name the new name for the desktop
     **/
    void setDesktopName( int desktop, const QString& name );

    /**
     * Informs kwin via dcop to not manage a window with the
     * specified @p title.
     *
     * Useful for swallowing legacy applications, for example java
     * applets.
     *
     * @param title the title of the window
     */
    void doNotManage( const QString& title );


signals:

    /**
     * Switched to another virtual desktop.
     * @param desktop the number of the new desktop
     */
    void currentDesktopChanged( int desktop);

    /**
     * A window has been added.
     * @param id the id of the the window 
     */
    void windowAdded(WId id);

    /**
     * A window has been removed.
     * @param id the id of the window that has been removed
     */
    void windowRemoved(WId id);

    /**
     * Hint that <Window> is active (= has focus) now.
     * @param id the id of the window that is active
     */
    void activeWindowChanged(WId);

    /**
     * Desktops have been renamed.
     */
    void desktopNamesChanged();

    /**
     * The number of desktops changed.
     * @param num the new number of desktops
     */
    void numberOfDesktopsChanged(int num);

    /**
     * Emitted when a dock window has been added.
     * @param id the id of the new system tray window
     */
    void systemTrayWindowAdded(WId id);

    /**
     * Emitted when a dock window has been removed.
     * @param id the id of the former system tray window
     */
    void systemTrayWindowRemoved(WId id);

    /**
     * The workarea has changed.
     */
    void workAreaChanged();

    /** 
     * Something changed with the struts, may or may not have changed
     * the work area.
     */
    void strutChanged();
    
    /**
     * Emitted when the stacking order of the window changed. The new order
     * can be obtained with @ref stackingOrder().
     */
    void stackingOrderChanged();


    /**
     * The window changed.
     *
     * The unsigned int parameter contains the NET properties that
     * were modified (see netem_def.h).
     * @param id the id of the window
     * @param properties the properties that were modified
     */
    void windowChanged(WId id, unsigned int properties);

    /**
     * The window changed somehow.
     * @param id the id of the window
     */
    void windowChanged(WId id);
    
private:
    KWinModulePrivate* d;

    friend class KWinModulePrivate;
};

#endif
