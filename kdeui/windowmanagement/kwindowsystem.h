/* This file is part of the KDE libraries
    Copyright (C) 1999 Matthias Ettrich (ettrich@kde.org)
    Copyright (C) 2007 Lubos Lunak (l.lunak@kde.org)

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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
/*
 * kwindowsystem.h. Part of the KDE project.
 */

#ifndef KWINDOWSYSTEM_H
#define KWINDOWSYSTEM_H

#include <kdeui_export.h>
#include <QtCore/QObject>
#include <QtGui/QWidgetList> //For WId
#include <netwm_def.h>
#include <kwindowinfo.h>

class KWindowSystemPrivate;

/**
 *
 * Convenience access to certain properties and features of the
 * window manager.
 *
 * The class KWindowSystem provides information about the state of the
 * window manager and allows asking the window manager to change them
 * using a more high-level interface than the NETWinInfo/NETRootInfo
 * lowlevel classes.
 *
 * Because of limitiations of the way Qt is implemented on Mac OSX, the WId's
 * returned by methods in this class are not compatible with those expected
 * by other Qt methods. So while it should be fine to pass WId's retrieved by
 * for example calling the winId method on a QWidget to methods in this class
 * the reverse is not true. You should never pass a WId obtained from this class
 * to a Qt method accepting a WId parameter.
 *
 * @short Class for interaction with the window manager.
 * @author Matthias Ettrich (ettrich@kde.org)
 */
class KDEUI_EXPORT KWindowSystem : public QObject, public NET
{
    Q_OBJECT

public:
    /**
     * Access to the singleton instance. Useful mainly for connecting to signals.
     */
    static KWindowSystem* self();

   /**
     * Returns the list of all toplevel windows currently managed by the
     * window manager in the order of creation. Please do not rely on
     * indexes of this list: Whenever you enter Qt's event loop in your
     * application, it may happen that entries are removed or added.
     * Your module should perhaps work on a copy of this list and verify a
     * window with hasWId() before any operations.
     *
     * Iteration over this list can be done easily with
     * \code
     *  QList<WId>::ConstIterator it;
     *  for ( it = KWindowSystem::windows().begin();
     *        it != KWindowSystem::windows().end(); ++it ) {
     *     ... do something here,  (*it) is the current WId.
     *       }
     * \endcode
     * @return the list of all toplevel windows
     */
    static const QList<WId>& windows();

    /**
     * Test to see if @p id still managed at present.
     * @param id the window id to test
     * @return true if the window id is still managed
     **/
    static bool hasWId(WId id);

    /**
     * Returns information about window @p win. It is recommended to check
     * whether the returned info is valid by calling the valid() method.
     * @param win the id of the window
     * @param properties all properties that should be retrieved (see NET::Property
     *    enum for details). Unlisted properties cause related information to be invalid
     *    in the returned data, but make this function faster when not all data is needed.
     * @param properties2 additional properties (see NET::Property2 enum)
     * @return the window information
     */
    static KWindowInfo windowInfo( WId win, unsigned long properties, unsigned long properties2 = 0 );

    /**
     * Returns the list of all toplevel windows currently managed by the
     * window manager in the current stacking order (from lower to
     * higher). May be useful for pagers.
     * @return the list of all toplevel windows in stacking order
     */
    static QList<WId> stackingOrder();

    /**
     * Returns the currently active window, or 0 if no window is active.
     * @return the window id of the active window, or 0 if no window is
     *  active
     **/
    static WId activeWindow();

    /**
     * Requests that window @p win is activated.
     *
     * There are two ways how to activate a window, by calling
     * activateWindow() and forceActiveWindow(). Generally,
     * applications shouldn't make attempts to explicitly activate
     * their windows, and instead let the user to activate them.
     * In the special cases where this may be needed, applications
     * should use activateWindow(). Window manager may consider whether
     * this request wouldn't result in focus stealing, which
     * would be obtrusive, and may refuse the request.
     *
     * The usage of forceActiveWindow() is meant only for pagers
     * and similar tools, which represent direct user actions
     * related to window manipulation.
     * Except for rare cases, this request will be always honored,
     * and normal applications are forbidden to use it.
     *
     * In case of problems, consult the KWin README in the kdebase
     * package (kdebase/kwin/README), or ask on the kwin@kde.org
     * mailing list.
     *
     * @param win the id of the window to make active
     * @param time X server timestamp of the user activity that
     *    caused this request
     */
    static void activateWindow( WId win, long time = 0 );

    /**
     * Sets window @p win to be the active window. Note that this
     * should be called only in special cases, applications
     * shouldn't force themselves or other windows to be the active
     * window. Generally, this call should used only by pagers
     * and similar tools. See the explanation in description
     * of activateWindow().
     *
     * @param win the id of the window to make active
     * @param time X server timestamp of the user activity that
     *    caused this request
    */
    static void forceActiveWindow( WId win, long time = 0 );

    /**
     * When application finishes some operation and wants to notify
     * the user about it, it can call demandAttention(). Instead
     * of activating the window, which could be obtrusive, the window
     * will be marked specially as demanding user's attention.
     * See also explanation in description of activateWindow().
     *
     * Note that it's usually better to use KNotifyClient.
     */
    static void demandAttention( WId win, bool set = true );

    /**
     * Returns true if a compositing manager is running (i.e. ARGB windows
     * are supported, effects will be provided, etc.).
     */
    static bool compositingActive();

    /**
     * Returns the current virtual desktop.
     * @return the current virtual desktop
     **/
    static int currentDesktop();

    /**
     * Returns the number of virtual desktops.
     * @return the number of virtual desktops
     **/
    static int numberOfDesktops();

    /**
     * Convenience function to set the current desktop to @p desktop.
     * See NETRootInfo.
     * @param desktop the number of the new desktop
     */
    static void setCurrentDesktop( int desktop );

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
     * Sets the parent window of @p subwindow to be @p mainwindow.
     * This overrides the parent set the usual way as the QWidget parent,
     * but only for the window manager - e.g. stacking order and window grouping
     * will be affected, but features like automatic deletion of children
     * when the parent is deleted are unaffected and normally use
     * the QWidget parent.
     *
     * This function should be used before a dialog is shown for a window
     * that belongs to another application.
     */
    static void setMainWindow( QWidget* subwindow, WId mainwindow );
#ifdef Q_WS_X11
    /**
     * Returns the WM_TRANSIENT_FOR property for the given window, i.e. the mainwindow
     * for this window.
     *
     * @param window the id of the window
     */
    static WId transientFor( WId window );

    /**
     * Returns the leader window for the group the given window is in, if any.
     * @param window the id of the window
     */
    static WId groupLeader( WId window );
#endif
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
     * @param width the desired width, or -1
     * @param height the desired height, or -1
     * @param scale if true the icon will be scaled to the desired size. Otherwise the
     *        icon will not be modified.
     * @return the icon of the window
     */
    static QPixmap icon( WId win, int width = -1, int height = -1, bool scale = false );

    /**
     * Masks specifying from which sources to read an icon. They are tried from the best
     * until an icon is found.
     * @li NETWM from property from the window manager specification
     * @li WMHints from WMHints property
     * @li ClassHint load icon after getting name from the classhint
     * @li XApp load the standard X icon (last fallback)
     */
    enum IconSource { NETWM = 1, //!< read from property from the window manager specification
		      WMHints = 2, //!< read from WMHints property
		      ClassHint = 4, //!< load icon after getting name from the classhint
		      XApp = 8 //!<load the standard X icon (last fallback)
    };
    /**
     * @overload
     *
     * Overloaded variant that allows specifying from which sources the icon should be read.
     * You should usually prefer the simpler variant which tries all possibilities to get
     * an icon.
     *
     * @param win the id of the window
     * @param width the desired width, or -1
     * @param height the desired height, or -1
     * @param scale if true the icon will be scaled to the desired size. Otherwise the
     *        icon will not be modified.
     * @param flags OR-ed flags from the IconSource enum
     */
    static QPixmap icon( WId win, int width, int height, bool scale, int flags );

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
     * @param win the id of the window
     * @param windowType the type of the window (see NET::WindowType)
     */
    static void setType( WId win, NET::WindowType windowType );
    /**
     * Sets the state of window @p win to @p state.
     *
     * Possible values are or'ed combinations of NET::Modal,
     * NET::Sticky, NET::MaxVert, NET::MaxHoriz, NET::Shaded,
     * NET::SkipTaskbar, NET::SkipPager, NET::Hidden,
     * NET::FullScreen, NET::KeepAbove, NET::KeepBelow, NET::StaysOnTop
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
     * NET::FullScreen, NET::KeepAbove, NET::KeepBelow, NET::StaysOnTop
     *
     * @param win the id of the window
     * @param state the flags that will be cleared
     */
    static void clearState( WId win, unsigned long  state );

    /**
     * Iconifies a window. Compatible to XIconifyWindow but has an
     * additional parameter @p animation.
     *
     * @param win the id of the window
     * @param animation true to show an animation
     * @see deIconifyWindow()
     */
    static void minimizeWindow( WId win, bool animation = true );

    /**
     * DeIconifies a window. Compatible to XMapWindow but has an
     * additional parameter @p animation.
     *
     * @param win the id of the window
     * @param animation true to show an animation
     * @see iconifyWindow()
     */
    static void unminimizeWindow( WId win, bool animation = true );

    /**
     * Raises the given window. This call is only for pagers and similar
     * tools that represent direct user actions. Applications should not
     * use it, they should keep using QWidget::raise() or XRaiseWindow()
     * if necessary.
     */
    static void raiseWindow( WId win );

    /**
     * Lowers the given window. This call is only for pagers and similar
     * tools that represent direct user actions. Applications should not
     * use it, they should keep using QWidget::lower() or XLowerWindow()
     * if necessary.
     */
    static void lowerWindow( WId win );

    /**
     * @internal
     * Returns true if the WM uses IconicState also for windows
     * on inactive virtual desktops.
     */
    static bool icccmCompliantMappingState();

    /**
     * Returns the workarea for the specified desktop, or the current
     * work area if no desktop has been specified.
     * @param desktop the number of the desktop to check, -1 for the
     *        current desktop
     * @return the size and position of the desktop
     **/
    static QRect workArea( int desktop = - 1 );


    /**
     * Returns the workarea for the specified desktop, or the current
     * work area if no desktop has been specified. Excludes struts of
     * clients in the exclude List.
     *
     * @param excludes the list of clients whose struts will be excluded
     * @param desktop the number of the desktop to check, -1 for the
     *        current desktop
     * @return the size and position of the desktop
     **/
    static QRect workArea( const QList<WId> &excludes, int desktop = -1);

    /**
     * Returns the name of the specified desktop.
     * @param desktop the number of the desktop
     * @return the name of the desktop
     **/
    static QString desktopName( int desktop );

    /**
     * Sets the name of the specified desktop.
     * @param desktop the number of the desktop
     * @param name the new name for the desktop
     **/
    static void setDesktopName( int desktop, const QString& name );

    /**
     * Returns the state of showing the desktop.
     */
    static bool showingDesktop();

    /**
     * Sets user timestamp @p time on window @p win. The timestamp
     * is expressed as XServer time. If a window
     * is shown with user timestamp older than the time of the last
     * user action, it won't be activated after being shown.
     * The most common case is the special value 0 which means
     * not to activate the window after being shown.
     */
    static void setUserTime( WId win, long time );
    /**
     * Sets the strut of window @p win to @p to @p left width
     * ranging from @p left_start to @p left_end on the left edge,
     * and simiarly for the other edges. For not reserving a strut, pass 0 as the width.
     * E.g. to reserve 10x10 square in the topleft corner, use e.g.
     * setExtendedStrut( w, 10, 0, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0 ).
     *
     * @param win the id of the window
     * @param left_width width of the strut at the left edge
     * @param left_start starting y coordinate of the strut at the left edge
     * @param left_end ending y coordinate of the strut at the left edge
     * @param right_width width of the strut at the right edge
     * @param right_start starting y coordinate of the strut at the right edge
     * @param right_end ending y coordinate of the strut at the right edge
     * @param top_width width of the strut at the top edge
     * @param top_start starting x coordinate of the strut at the top edge
     * @param top_end ending x coordinate of the strut at the top edge
     * @param bottom_width width of the strut at the bottom edge
     * @param bottom_start starting x coordinate of the strut at the bottom edge
     * @param bottom_end ending x coordinate of the strut at the bottom edge
     */
    static void setExtendedStrut( WId win, int left_width, int left_start, int left_end,
        int right_width, int right_start, int right_end, int top_width, int top_start, int top_end,
        int bottom_width, int bottom_start, int bottom_end );

    /**
     * Convenience function for setExtendedStrut() that automatically makes struts
     * as wide/high as the screen width/height.
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
     * Returns true if the WM announces which actions it allows for windows.
     */
    static bool allowedActionsSupported();
    /**
     * Function that reads and returns the contents of the given text
     * property (WM_NAME, WM_ICON_NAME,...).
     */
    static QString readNameProperty( WId window, unsigned long atom );

    /**
     * Informs kwin via dbus to not manage a window with the
     * specified @p title.
     *
     * Useful for swallowing legacy applications, for example java
     * applets.
     *
     * @param title the title of the window
     */
    static void doNotManage( const QString& title );

    /**
     * Allows a window from another process to raise and activate itself.
     * Depending on the window manager, the grant may only be temporary,
     * or for a single activation, and it may require the current process
     * to be the "foreground" one" (ie. the process with the input focus).
     *
     * You should call this function before executing actions that may trigger
     * the showing of a window or dialog in another process, e.g. a dbus signal
     * or function call, or any other inter-process notification mechanism.
     *
     * This is mostly used on Windows, where windows are not allowed to be raised
     * and activated if their process is not the foreground one, but it may also
     * apply to other window managers.
     *
     * @param pid if specified, the grant only applies to windows belonging to the
     *            specific process. By default, a value of -1 means all processes.
     */
    static void allowExternalProcessWindowActivation( int pid = -1 );

#ifdef Q_WS_X11
    /**
     * @internal
     * Returns true if viewports are mapped to virtual desktops.
     */
    static bool mapViewport();
    /**
     * @internal
     * Returns mapped virtual desktop for the given position in the viewport.
     */
    static int viewportToDesktop( const QPoint& pos );
    /**
     * @internal
     * Returns mapped virtual desktop for the given window geometry.
     */
    static int viewportWindowToDesktop( const QRect& r );
    /**
     * @internal
     * Returns topleft corner of the viewport area for the given mapped virtual desktop.
     */
    static QPoint desktopToViewport( int desktop, bool absolute );
    /**
     * @internal
     * @since 4.0.1
     * Checks the relative difference used to move a window will still be inside
     * valid desktop area.
     */
    static QPoint constrainViewportRelativePosition( const QPoint& pos );
#endif

Q_SIGNALS:

    /**
     * Switched to another virtual desktop.
     * @param desktop the number of the new desktop
     */
    void currentDesktopChanged( int desktop);

    /**
     * A window has been added.
     * @param id the id of the window
     */
    void windowAdded(WId id);

    /**
     * A window has been removed.
     * @param id the id of the window that has been removed
     */
    void windowRemoved(WId id);

    /**
     * Hint that \<Window> is active (= has focus) now.
     * @param id the id of the window that is active
     */
    void activeWindowChanged(WId id);

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
     * The workarea has changed.
     */
    void workAreaChanged();

    /**
     * Something changed with the struts, may or may not have changed
     * the work area. Usually just using the workAreaChanged() signal
     * is sufficient.
     */
    void strutChanged();

    /**
     * Emitted when the stacking order of the window changed. The new order
     * can be obtained with stackingOrder().
     */
    void stackingOrderChanged();

    /**
     * The window changed.
     *
     * The properties parameter contains the NET properties that
     * were modified (see netwm_def.h). First element are NET::Property
     * values, second element are NET::Property2 values (i.e. the format
     * is the same like for the NETWinInfo class constructor).
     * @param id the id of the window
     * @param properties the properties that were modified
     */
    void windowChanged(WId id, const unsigned long* properties );

    /**
     * @deprecated
     * The window changed.
     *
     * The unsigned int parameter contains the NET properties that
     * were modified (see netwm_def.h).
     * @param id the id of the window
     * @param properties the properties that were modified
     */
    void windowChanged(WId id, unsigned int properties);

    /**
     * The window changed somehow.
     * @param id the id of the window
     */
    void windowChanged(WId id);

    /**
     * The state of showing the desktop has changed.
     */
    void showingDesktopChanged( bool showing );

protected:
    virtual void connectNotify( const char* signal );

private:
    friend class KWindowSystemStaticContainer;

    KWindowSystem() {}

    enum { INFO_BASIC=1, // desktop info, not per-window
           INFO_WINDOWS=2 }; // also per-window info

    static void init(int);

    friend class KWindowSystemPrivate;
    static KWindowSystemPrivate* s_d_func();
};

#endif
