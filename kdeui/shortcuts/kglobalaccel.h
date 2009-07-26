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

#include "kdeui_export.h"
#include "kaction.h"
#include "kglobalshortcutinfo.h"

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
    Q_OBJECT

public:

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
     * Returns (and creates if necessary) the singleton instance
     */
    static KGlobalAccel *self();

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
     *
     * @since 4.2
     */
    static void activateGlobalShortcutContext(
            const QString &contextUnique,
            const QString &contextFriendly,
            const KComponentData &component = KGlobal::mainComponent());

    /**
     * Clean the shortcuts for component @a componentUnique.
     *
     * If the component is not active all global shortcut registrations are
     * purged and the component is removed completely.
     *
     * If the component is active all global shortcut registrations not in use
     * will be purged. If there is no shortcut registration left the component
     * is purged too.
     *
     * If a purged component or shortcut is activated the next time it will
     * reregister itself. All you probably will lose on wrong usage are the
     * user's set shortcuts.
     *
     * If you make sure your component is running and all global shortcuts it
     * has are active this function can be used to clean up the registry.
     *
     * Handle with care!
     *
     * If the method return @c true at least one shortcut was purged so handle
     * all previously acquired information with care.
     */
    static bool cleanComponent(const QString &componentUnique);


    /**
     * Check if @a component is active.
     *
     * @param componentUnique the components unique identifier
     * @return @c true if active, @false if not
     */
    static bool isComponentActive(const QString &componentName);

    /**
     * Returns a list of global shortcuts registered for the shortcut @seq.
     *
     * If the list contains more that one entry it means the component
     * that registered the shortcuts uses global shortcut contexts. All
     * returned shortcuts belong to the same component.
     *
     * @since 4.2
     */
    static QList<KGlobalShortcutInfo> getGlobalShortcutsByKey(const QKeySequence &seq);

    /**
     * Check if the shortcut @seq is available for the @p component. The
     * component is only of interest if the current application uses global shortcut
     * contexts. In that case a global shortcut by @p component in an inactive
     * global shortcut contexts does not block the @p seq for us.
     *
     * @since 4.2
     */
    static bool isGlobalShortcutAvailable(
            const QKeySequence &seq,
            const QString &component = QString());

    /**
     * Show a messagebox to inform the user that a global shorcut is already occupied,
     * and ask to take it away from its current action(s). This is GUI only, so nothing will
     * be actually changed.
     *
     * @see stealShortcutSystemwide()
     *
     * @since 4.2
     */
    static bool promptStealShortcutSystemwide(
            QWidget *parent,
            const QList<KGlobalShortcutInfo> &shortcuts,
            const QKeySequence &seq);

    /**
     * No effect.
     *
     * @deprecated
     */
    KDE_DEPRECATED bool isEnabled() const;

    /**
     * No effect.
     *
     * @deprecated
     */
    KDE_DEPRECATED void setEnabled(bool enabled);

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
     *
     * @deprecated
     */
    KDE_DEPRECATED void overrideMainComponentData(const KComponentData &componentData);

    /**
     * Return the unique and common names of all main components that have global shortcuts.
     * The action strings of the returned actionId stringlists will be empty.
     *
     * @deprecated
     */
    KDE_DEPRECATED QList<QStringList> allMainComponents();

    /**
     * @see getGlobalShortcutsByComponent
     *
     * @deprecated
     */
    KDE_DEPRECATED QList<QStringList> allActionsForComponent(const QStringList &actionId);

    /**
     * @see getGlobalShortcutsByKey
     *
     * @deprecated
     */
    KDE_DEPRECATED static QStringList findActionNameSystemwide(const QKeySequence &seq);

    /**
     * @see promptStealShortcutSystemwide below
     *
     * @deprecated
     */
    KDE_DEPRECATED static bool promptStealShortcutSystemwide(QWidget *parent, const QStringList &actionIdentifier, const QKeySequence &seq);

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
