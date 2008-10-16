/* This file is part of the KDE libraries
    Copyright (C) 2001,2002 Ellis Whitehead <ellis@kde.org>
    Copyright (C) 2006 Hamish Rodda <rodda@kde.org>
    Copyright (C) 2007 Andreas Hartmetz <ahartmetz@gmail.com>

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

#ifndef _KGLOBALACCEL_H_
#define _KGLOBALACCEL_H_

#include <kdeui_export.h>
#include "kaction.h"

#include <QtCore/QObject>

class QWidget;
class KShortcut;
class KComponentData;

/**
 * @short Configurable global shortcut support
 *
 * KGlobalAccel allows you to have global accelerators that are independent of
 * the focused window.  Unlike regular shortcuts, the application's window does not need focus
 * for them to be activated.
 *
 * @see KKeyChooser
 * @see KKeyDialog
 */
class KDEUI_EXPORT KGlobalAccel : public QObject
{
    friend class KGlobalAccelImpl;
    Q_OBJECT

public:

    /**
     * Returns (and creates if necessary) the singleton instance
     */
    static KGlobalAccel *self();

    /**
     * Checks whether the accelerators are enabled.
     *
     * @Bug: enabling / disabling has currently no effect.
     *
     * @return true if the KGlobalAccel is enabled
     */
    bool isEnabled() const;

    /**
     * Enables or disables the KGlobalAccel
     * @param enabled true if the KGlobalAccel should be enabled, false if it
     *  should be disabled.
     */
    void setEnabled(bool enabled);

    /**
     * Set the KComponentData for which to manipulate shortcuts. This is for exceptional
     * situations, when you want to modify the shortcuts of another application
     * as if they were yours.
     * You cannot have your own working global shortcuts in a module/application using this
     * special functionality. All global shortcuts of KActions will essentially be proxies.
     * Be sure to set the default global shortcuts of the proxy KActions to the same as
     * those on the receiving end.
     * An example use case is the KControl Module for the window manager KWin, which has
     * no own facility for users to change its global shortcuts.
     *
     * @param componentData a KComponentData about the application for which you want to
     *                      manipulate shortcuts.
     */
    KDE_DEPRECATED void overrideMainComponentData(const KComponentData &componentData);

    /**
     * Return the unique and common names of all main components that have global shortcuts.
     * The action strings of the returned actionId stringlists will be empty.
     */
    QList<QStringList> allMainComponents();

    /**
     * Index for actionId QStringLists
     */
    enum actionIdFields
    {
        ComponentUnique = 0,        //!< Components Unique Name (ID)
        ActionUnique = 1,           //!< Actions Unique Name(ID)
        ComponentFriendly = 2,      //!< Components Friendly Translated Name
        ActionFriendly = 3          //!< Actions Friendly Translated Name
    };

    /**
     * Return the full actionIds of all actions with global shortcuts for the main component
     * specified by actionId. Only the ComponentUnique part of actionId will be used;
     * the other members of the actionId QStringList will be ignored.
     */
    QList<QStringList> allActionsForComponent(const QStringList &actionId);

    /**
     * Return the name of the action that uses the given key sequence. This applies to
     * all actions with global shortcuts in any KDE application.
     *
     * @see promptStealShortcutSystemwide(), stealShorctutSystemwide()
     */
    static QStringList findActionNameSystemwide(const QKeySequence &seq);

    /**
     * Show a messagebox to inform the user that a global shorcut is already occupied,
     * and ask to take it away from its current action. This is GUI only, so nothing will
     * be actually changed.
     *
     * @see stealShorctutSystemwide()
     */
    static bool promptStealShortcutSystemwide(QWidget *parent, const QStringList &actionIdentifier, const QKeySequence &seq);

    /**
     * Take away the given shortcut from the named action it belongs to.
     * This applies to all actions with global shortcuts in any KDE application.
     *
     * @see promptStealShortcutSystemwide()
     */
    static void stealShortcutSystemwide(const QKeySequence &seq);

    /**
     * Set global shortcut context.
     *
     * A global shortcut context allows an application to have different sets
     * of global shortcuts and to switch between them. This is used by
     * plasma to switch the active global shortcuts when switching between
     * activities.
     *
     * @param component the name of the component. KComponentData::componentName
     * @param context the name of the context.
     */
    void activateGlobalShortcutContext(const QString &component, const QString &context);

private:

    friend class KAction;

    /// Creates a new KGlobalAccel object
    KGlobalAccel();

    /// Destructor
    ~KGlobalAccel();

    class KGlobalAccelPrivate *const d;

    Q_PRIVATE_SLOT(d, void _k_invokeAction(const QStringList&,qlonglong))
    Q_PRIVATE_SLOT(d, void _k_shortcutGotChanged(const QStringList&, const QList<int>&))
    Q_PRIVATE_SLOT(d, void _k_serviceOwnerChanged(const QString&, const QString&, const QString&))
};

#endif // _KGLOBALACCEL_H_
