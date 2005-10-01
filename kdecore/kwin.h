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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
#ifndef KWIN_H
#define KWIN_H

#include <qwindowdefs.h>
#include <kdelibs_export.h>

#ifdef Q_OS_UNIX

#include <netwm_def.h>
class NETWinInfo;

class QPixmap;
class QString;

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
 * project).  The full specification can be found at
 * www.freedesktop.org/standards/wm-spec .
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
 * @see NET
 * @see NetWinInfo
 * @author Matthias Ettrich (ettrich@kde.org)
*/
class KDECORE_EXPORT KWin
{
public:

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
     * @since 3.2
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
     *
     * @since 3.2
    */
    static void forceActiveWindow( WId win, long time = 0 );
    /**
     * @deprecated Consider using activateWindow(), use forceActiveWindow()
     * only if necessary.
     */
    static void setActiveWindow( WId win ) KDE_DEPRECATED;

    /**
     * When application finishes some operation and wants to notify
     * the user about it, it can call demandAttention(). Instead
     * of activating the window, which could be obtrusive, the window
     * will be marked specially as demanding user's attention.
     * See also explanation in description of activateWindow().
     *
     * Note that it's usually better to use KNotifyClient.
     *
     * @since 3.2
     */
    static void demandAttention( WId win, bool set = true );

    /**
     * Sets user timestamp @p time on window @p win. The timestamp
     * is expressed as XServer time. If a window
     * is shown with user timestamp older than the time of the last
     * user action, it won't be activated after being shown.
     * The most common case is the special value 0 which means
     * not to activate the window after being shown.
     *
     * @since 3.2
     */
    static void setUserTime( WId win, long time );

    /**
     * Invokes interactive context help.
     */
    static void invokeContextHelp();


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
     *
     * @since 3.4
     */
    static void setMainWindow( QWidget* subwindow, WId mainwindow );

    /**
     * Makes @p trayWin a system tray window for @p forWin.
     *
     * A system tray window serves as an icon replacement. It's
     * displayed inside the panel's system tray.
     * @param trayWin the id of the system tray window
     * @param forWin the id of the window represented by the system
     *        tray window
     */
    static void setSystemTrayWindowFor( WId trayWin, WId forWin );

    class WindowInfo;
//    class WindowInfoPrivate;
    /**
     * Returns information about window @p win. It is recommended to check
     * whether the returned info is valid by calling the valid() method.
     * @param win the id of the window
     * @param properties all properties that should be retrieved (see NET::Property
     *    enum for details) - passing 0 means all properties. Unlisted properties
     *    cause related information to be invalid in the returned data, but
     *    make this function faster when not all data is needed.
     * @param properties2 additional properties (see NET::Property2 enum)
     * @return the window information
     * @since 3.2
     */
    static WindowInfo windowInfo( WId win, unsigned long properties = 0, unsigned long properties2 = 0 );

    /**
     * Returns the WM_TRANSIENT_FOR property for the given window, i.e. the mainwindow
     * for this window.
     *
     * @param window the id of the window
     * @since 3.2
     */
    static WId transientFor( WId window );

    /**
     * Returns the leader window for the group the given window is in, if any.
     * @param window the id of the window
     * @since 3.2
     */
    static WId groupLeader( WId window );

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
     * @since 3.2
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
     * Possible values are NET::Normal, NET::Desktop, NET::Dock,
     * NET::Tool, NET::Menu, NET::Dialog, NET::Override (== no
     * decoration frame), NET::TopMenu, NET::Utility or NET::Splash
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
     * Sets the opacity of window @p win to percetage @p percent.
     *
     * Convenience function that just sets an X property
     * needs a running composite manager for any visible effect
     *
     * @param win the id of the window
     * @param percent the opacity value in percent (will be justified to [ 0: transparent - 100: opaque ])
     * @since 3.4
     */
    static void setOpacity( WId win, uint percent );

    /**
     * Sets the shadowsize of window @p win to percetage @p percent.
     *
     * Convenience function that just sets an X property
     * needs the running KDE kompmgr manager for any visible effect
     *
     * @param win the id of the window
     * @param percent the opacity value in percent (0 leads to a completely unshadowed window)
     * @since 3.4
     */
    static void setShadowSize( WId win, uint percent );

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
     * @deprecated use setExtendedStrut()
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
     * Convenience function to access the current desktop.  See NETRootInfo.
     * @return the number of the current desktop
     */
    static int currentDesktop();

    /**
     * Convenience function to access the number of desktops. See
     * NETRootInfo.
     * @return the number of desktops
     */
    static int numberOfDesktops();

    /**
     * Convenience function to set the current desktop to @p desktop.
     * See NETRootInfo.
     * @param desktop the number of the new desktop
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

    /**
     * Raises the given window. This call is only for pagers and similar
     * tools that represent direct user actions. Applications should not
     * use it, they should keep using QWidget::raise() or XRaiseWindow()
     * if necessary.
     * @since 3.2
     */
    static void raiseWindow( WId win );

    /**
     * Lowers the given window. This call is only for pagers and similar
     * tools that represent direct user actions. Applications should not
     * use it, they should keep using QWidget::lower() or XLowerWindow()
     * if necessary.
     * @since 3.2
     */
    static void lowerWindow( WId win );

    /**
     * @internal
     * Returns true if the WM uses IconicState also for windows
     * on inactive virtual desktops.
     */
    static bool icccmCompliantMappingState();

    /**
     * Returns true if the WM announces which actions it allows for windows.
     * @since 3.2
     */
    static bool allowedActionsSupported();

    /**
     * Function that reads and returns the contents of the given text
     * property (WM_NAME, WM_ICON_NAME,...).
     * @since 3.2
     */
    static QString readNameProperty( WId window, unsigned long atom );

};


/**
 * Information about a window.
 * @since 3.2
 */
class KDECORE_EXPORT KWin::WindowInfo
{
public:
    /**
     * Reads all the info about the given window.
     */
    WindowInfo( WId window, unsigned long properties, unsigned long properties2 );
    WindowInfo(); // to make QValueList and others happy
    ~WindowInfo();
    /**
     * Returns false if this window info is not valid (most probably the given
     * window doesn't exist).
     * @param withdrawn_is_valid if true, windows in the withdrawn state
     *        (i.e. not managed) are also considered. This is usually not the case.
     */
    bool valid( bool withdrawn_is_valid = false ) const;
    /**
     * Returns the window identifier.
     */
    WId win() const;
    /**
     * Returns the window's state flags (see the NET::State enum for details).
     * Requires NET::WMState passed to KWin::windowInfo().
     */
    unsigned long state() const;
    /**
     * Returns true if the window has the given state flag set (see the NET::State enum for details).
     * Requires NET::WMState passed to KWin::windowInfo().
     * @since 3.2.1
     */
    bool hasState( unsigned long s ) const { return ( state() & s ) == s; }
    /**
     * Returns true if the window is minimized. Note that it is true only if
     * the window is truly minimized, not shaded or on another virtual desktops,
     * which makes it different from mappingState() == NET::Iconic
     * or QWidget::isMinimized().
     * Requires NET::WMState and NET::XAWMState passed to KWin::windowInfo().
     */
    bool isMinimized() const;
    /**
     * Returns the mapping state of the window (see NET::MappingState). Note that
     * it's very likely that you don't want to use this function, and use isOnDesktop(),
     * isMinimized() etc. instead.
     * Requires NET::XAWMState passed to KWin::windowInfo().
     */
    NET::MappingState mappingState() const;
    /**
     * Returns the window extended (partial) strut.
     * Requires NET::WM2ExtendedStrut passed to KWin::windowInfo().
     */
    NETExtendedStrut extendedStrut() const;
    /**
     * @deprecated use extendedStrut()
     * Returns the window strut.
     * Requires NET::WMStrut passed to KWin::windowInfo().
     */
    NETStrut strut() const;
    /**
     * Returns the window type of this window (see NET::WindowType). The argument
     * should be all window types your application supports (see NET::WindowTypeMask).
     * Requires NET::WMWindowType passed to KWin::windowInfo().
     */
    NET::WindowType windowType( int supported_types ) const;
    /**
     * Returns the visible name of the window (i.e. including possible <2> appended
     * when there are two or more windows with the same name).
     * Requires NET::WMVisibleName passed to KWin::windowInfo().
     */
    QString visibleName() const;
    /**
     * Returns a visible name with state.
     *
     * This is a simple convenience function that returns the
     * visible name but with parentheses around minimized windows.
     * Requires NET::WMVisibleName, NET::WMState and NET::XAWMState passed
     * to KWin::windowInfo().
     * @return the window name with state
     */
    QString visibleNameWithState() const;
    /**
     * Returns the name of the window, as specified by the application, without
     * any modifications. You should often use visibleName() instead.
     * Requires NET::WMName passed to KWin::windowInfo().
     */
    QString name() const;
    /**
     * Returns the visible name of the window that should be shown in taskbar
     * and all other "iconic" representations of the window. Note that this
     * has nothing to do with normal icons.
     * Requires NET::WMVisibleIconName passed to KWin::windowInfo().
     */
    QString visibleIconName() const;
    /**
     * Returns a visible name with state.
     *
     * This is a simple convenience function that returns the
     * visible iconic name but with parentheses around minimized windows.
     * Note that this has nothing to do with normal icons.
     * Requires NET::WMVisibleIconName, NET::WMState and NET::XAWMState passed
     * to KWin::windowInfo().
     * @return the window iconic name with state
     */
    QString visibleIconNameWithState() const;
    /**
     * Returns the name of the window that should be shown in taskbar and all other
     * "iconic" representations of the window. Note that this has nothing to do
     * with normal icons.
     * Requires NET::WMIconName passed to KWin::windowInfo().
     */
    QString iconName() const;
    /**
     * Returns true if the window is on the currently active virtual desktop.
     * Requires NET::WMDesktop passed to KWin::windowInfo().
     */
    bool isOnCurrentDesktop() const;
    /**
     * Returns true if the window is on the given virtual desktop.
     * Requires NET::WMDesktop passed to KWin::windowInfo().
     */
    bool isOnDesktop( int desktop ) const;
    /**
     * Returns true if the window is on all desktops
     * (equal to desktop()==NET::OnAllDesktops).
     * Requires NET::WMDesktop passed to KWin::windowInfo().
     */
    bool onAllDesktops() const;
    /**
     * Returns the virtual desktop this window is on (NET::OnAllDesktops if the window
     * is on all desktops). You should prefer using isOnDesktop().
     * Requires NET::WMDesktop passed to KWin::windowInfo().
     */
    int desktop() const;
    /**
     * Returns the position and size of the window contents.
     * Requires NET::WMGeometry passed to KWin::windowInfo().
     */
    QRect geometry() const;
    /**
     * Returns the frame geometry of the window, i.e. including the window decoration.
     * Requires NET::WMKDEFrameStrut passed to KWin::windowInfo().
     */
    QRect frameGeometry() const;
    /**
     * Returns the WM_TRANSIENT_FOR property for the window, i.e. the mainwindow
     * for this window.
     * Requires NET::WM2TransientFor passed to KWin::windowInfo().
     */
    WId transientFor() const;
    /**
     * Returns the leader window for the group the window is in, if any.
     * Requires NET::WM2GroupLeader passed to KWin::windowInfo().
     */
    WId groupLeader() const;

    /**
     * Returns the class component of the window class for the window
     * (i.e. WM_CLASS property).
     * Requires NET::WM2WindowClass passed to KWin::windowInfo().
     * @since 3.3
     */
    QByteArray windowClassClass() const;

    /**
     * Returns the name component of the window class for the window
     * (i.e. WM_CLASS property).
     * Requires NET::WM2WindowClass passed to KWin::windowInfo().
     * @since 3.3
     */
    QByteArray windowClassName() const;

    /**
     * Returns the window role for the window (i.e. WM_WINDOW_ROLE property).
     * Requires NET::WM2WindowRole passed to KWin::windowInfo().
     * @since 3.3
     */
    QByteArray windowRole() const;

    /**
     * Returns the client machine for the window (i.e. WM_CLIENT_MACHINE property).
     * Requires NET::WMClientMachine passed to KWin::windowInfo().
     * @since 3.3
     */
    QByteArray clientMachine() const;

    /**
     * Returns true if the given action is currently supported for the window
     * by the window manager.
     * Requires NET::WM2AllowedActions passed to KWin::windowInfo().
     */
    bool actionSupported( NET::Action action ) const;

    WindowInfo( const WindowInfo& );
    WindowInfo& operator=( const WindowInfo& );
private:
    class Private;
    Private * d;
};

#endif //Q_OS_UNIX

#endif
