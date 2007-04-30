/* This file is part of the KDE libraries
    Copyright (C) 2001,2002 Ellis Whitehead <ellis@kde.org>
    Copyright (C) 2006 Hamish Rodda <rodda@kde.org>

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

#include <QtCore/QObject>

class QWidget;
class KConfigBase;
class KAction;
class QKeySequence;
class KShortcut;

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
    /// Destructor
    virtual ~KGlobalAccel();

    /**
     * Returns (and creates if necessary) the singleton instance
     */
    static KGlobalAccel *self();

    /**
     * Checks whether the accelerators are enabled.
     * @return true if the KGlobalAccel is enabled
     */
    bool isEnabled();

    /**
     * Enables or disables the KGlobalAccel
     * @param enabled true if the KGlobalAccel should be enabled, false if it
     *  should be disabled.
     */
    void setEnabled(bool enabled);

    /**
     * Read all shortcuts from @p config, or (if @p config
     * is zero) from the application's configuration file
     * KGlobal::config().
     *
     * @param config the configuration file to read from, or null for the application
     *                 configuration file
     */
    void readSettings();

    /**
     * Write the current global shortcuts to @p config,
     * or (if @p config is zero) to the application's
     * configuration file.  Alternatively, if global is true, write
     * to kdeglobals.
     *
     * @param config the configuration file to read from, or null for the application
     *                 configuration file
    * \param writeDefaults set to true to write settings which are already at defaults.
     * @param oneAction pass an action here if you only want its settings to be saved
     *                  (eg. if you know this action is the only one which has changed).
     *
     * @return true if successful, otherwise false
     */
    void writeSettings(KAction *oneAction = 0) const;

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

Q_SIGNALS:

    ///notify (via DBUS) all KGlobalAccel instances in the whole session to update their global
    ///shortcuts mapping
    void updateGlobalShortcut();

private Q_SLOTS:
    /**
     * Listens to action change() events and respond when the action is enabled
     * or disabled.
     */
    void actionChanged();

    ///to be called by DBUS via our DBUS adapter
    //void globalShortcutChanged();

private:
    friend class KAction;
    ///Grab or release shortcuts for action as appropriate, and make any shortcut changes visible
    ///to other applications via the global configuration file.
    void updateGlobalShortcut(KAction *action, const KShortcut &oldShortcut);

    ///notification that global shortcuts were allowed/disallowed for an action.
    void updateGlobalShortcutAllowed(KAction *action);

    ///as the name says, but nothing is obvious here...
    void enableAction(KAction *action, bool enable);

    /// The money slot - the global \a key was pressed
    /// Returns whether it was consumed
    bool keyPressed(int key);

    /// Returns true if the key will be consumed (i.e. like keyPressed()
    /// without actually processing the key)
    bool isHandled( int key );

    ///ungrab stale shortcuts and grab the ones from newGrab
    void changeGrab(KAction *action, const KShortcut &newGrab);

    /// Something necessitates a repeat grabKey() for each key
    void regrabKeys();

    void enableImpl(bool enable);

    //TODO: DBus interface =), via a dbus adapter class
    ///part of the DBus interface
    void DBusShortcutTheftListener(const QKeySequence &loot);

    ///part of the DBus interface
    void DBusShortcutChangeListener(const QStringList &actionIdentifier, const QList<QKeySequence> &oldCuts, const QList<QKeySequence> &newCuts);

    /// Creates a new KGlobalAccel object
    KGlobalAccel();

    void grabKey(int key, bool grab, KAction *action);

    class KGlobalAccelPrivate *const d;
};

#endif // _KGLOBALACCEL_H_
