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
 * kwindowinfo.h. Part of the KDE project.
 */

#ifndef KWINDOWINFO_H
#define KWINDOWINFO_H

#include <kdeui_export.h>
#include <QtGui/QWidgetList> //For WId

#include <netwm_def.h>

/**
 * Information about a window.
 */
class KDEUI_EXPORT KWindowInfo
{
public:
    /**
     * Reads all the info about the given window.
     */
    KWindowInfo( WId window, unsigned long properties, unsigned long properties2 = 0 );
    KWindowInfo(); // to make QList and others happy
    ~KWindowInfo();
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
     */
    bool hasState( unsigned long s ) const;
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
     * Requires NET::WMFrameExtents passed to KWin::windowInfo().
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
     */
    QByteArray windowClassClass() const;

    /**
     * Returns the name component of the window class for the window
     * (i.e. WM_CLASS property).
     * Requires NET::WM2WindowClass passed to KWin::windowInfo().
     */
    QByteArray windowClassName() const;

    /**
     * Returns the window role for the window (i.e. WM_WINDOW_ROLE property).
     * Requires NET::WM2WindowRole passed to KWin::windowInfo().
     */
    QByteArray windowRole() const;

    /**
     * Returns the client machine for the window (i.e. WM_CLIENT_MACHINE property).
     * Requires NET::WMClientMachine passed to KWin::windowInfo().
     */
    QByteArray clientMachine() const;

    /**
     * Returns true if the given action is currently supported for the window
     * by the window manager.
     * Requires NET::WM2AllowedActions passed to KWin::windowInfo().
     */
    bool actionSupported( NET::Action action ) const;

    KWindowInfo( const KWindowInfo& );
    KWindowInfo& operator=( const KWindowInfo& );
private:
    class Private;
    Private * d; //krazy:exclude=dpointer (implicitly shared)
#ifdef Q_WS_MAC
    // KWindowSystem needs access to the d-pointer
    friend class KWindowSystem;
    friend class KWindowSystemPrivate;
#endif
};

#endif // multiple inclusion guard

