/* This file is part of the KDE libraries
    Copyright (C) 2001,2002 Ellis Whitehead <ellis@kde.org>

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

#ifndef _KACCEL_H
#define _KACCEL_H

#include <q3accel.h>
#include <kshortcut.h>
#include <kstdaccel.h>
#include "kdelibs_export.h"

class QWidget;
class KAccelAction;
class KAccelActions;
class KConfigBase;

class KAccelPrivate;
/**
 * Handle shortcuts.
 *
 * Allow a user to configure shortcuts
 * through application configuration files or through the
 * KKeyChooser GUI.
 *
 * A KAccel contains a list of accelerator actions.
 *
 * For example, CTRL+Key_P could be a shortcut for printing a document. The key
 * codes are listed in qnamespace.h. "Print" could be the action name for printing.
 * The action name identifies the shortcut in configuration files and the
 * KKeyChooser GUI.
 *
 * A KAccel object handles key events sent to its parent widget and to all
 * children of this parent widget.  The most recently created KAccel object
 * has precedence over any KAccel objects created before it.
 * When a shortcut pressed, KAccel calls the slot to which it has been
 * connected. If you want to set global accelerators, independent of the window
 * which has the focus, use KGlobalAccel.
 *
 * Reconfiguration of a given shortcut can be prevented by specifying
 * that an accelerator item is not configurable when it is inserted. A special
 * group of non-configurable key bindings are known as the
 * standard accelerators.
 *
 * The standard accelerators appear repeatedly in applications for
 * standard document actions such as printing and saving. A convenience method is
 * available to insert and connect these accelerators which are configurable on
 * a desktop-wide basis.
 *
 * It is possible for a user to choose to have no key associated with
 * an action.
 *
 * The translated first argument for insertItem() is used only
 * in the configuration dialog.
 *\code
 * KAccel* pAccel = new KAccel( this );
 *
 * // Insert an action "Scroll Up" which is associated with the "Up" key:
 * pAccel->insert( "Scroll Up", i18n("Scroll up"),
 *                       i18n("Scroll up the current document by one line."),
 *                       Qt::Key_Up, this, SLOT(slotScrollUp()) );
 * // Insert an standard acclerator action.
 * pAccel->insert( KStdAccel::Print, this, SLOT(slotPrint()) );
 *
 * // Update the shortcuts by read any user-defined settings from the
 * //  application's config file.
 * pAccel->readSettings();
 *\endcode
 *
 * @short Configurable shortcut support for widgets.
 * @see KGlobalAccel
 * @see KAccelShortcutList
 * @see KKeyChooser
 * @see KKeyDialog
 */

class KDECORE_EXPORT KAccel : public Q3Accel
{
	Q_OBJECT
 public:
	/**
	 * Creates a new KAccel that watches @p pParent, which is also
	 * the QObject's parent.
	 *
	 * @param pParent the parent and widget to watch for key strokes
	 * @param psName the name of the QObject
	 */
	KAccel( QWidget* pParent, const char* psName = 0 );

	/**
	 * Creates a new KAccel that watches @p watch.
	 *
	 * @param watch the widget to watch for key strokes
	 * @param parent the parent of the QObject
	 * @param psName the name of the QObject
	 */
	KAccel( QWidget* watch, QObject* parent, const char* psName = 0 );
	virtual ~KAccel();

	/**
	 * @internal
	 * Returns the KAccel's @p KAccelActions, a list of @p KAccepAction.
	 * @return the KAccelActions of the KAccel
	 */
	KAccelActions& actions();

	/**
	 * @internal
	 * Returns the KAccel's @p KAccelActions, a list of @p KAccepAction.
	 * @return the KAccelActions of the KAccel
	 */
	const KAccelActions& actions() const;

	/**
	 * Checks whether the KAccel is active.
	 * @return true if the QAccel is enabled
	 */
	bool isEnabled();

	/**
	 * Enables or disables the KAccel.
	 * @param bEnabled true to enable, false to disable
	 */
	void setEnabled( bool bEnabled );

	/**
	 * Enable auto-update of connections. This means that the signals
	 * are automatically disconnected when you disable an action, and
	 * re-enabled when you enable it. By default auto update is turned
	 * on. If you disable auto-update, you need to call
	 * updateConnections() after changing actions.
	 *
	 * @param bAuto true to enable, false to disable
	 * @return the value of the flag before this call
	 */
	bool setAutoUpdate( bool bAuto );

	/**
	 * Create an accelerator action.
	 *
	 * Usage:
	 *\code
	 * insert( "Do Something", i18n("Do Something"),
	 *   i18n("This action allows you to do something really great with this program to "
	 *        "the currently open document."),
	 *   ALT+Key_D, this, SLOT(slotDoSomething()) );
	 *\endcode
	 *
	 * @param sAction The internal name of the action.
	 * @param sLabel An i18n'ized short description of the action displayed when
	 *  using KKeyChooser to reconfigure the shortcuts.
	 * @param sWhatsThis An extended description of the action.
	 * @param cutDef The default shortcut.
	 * @param pObjSlot Pointer to the slot object.
	 * @param psMethodSlot Pointer to the slot method.
	 * @param bConfigurable Allow the user to change this shortcut if set to 'true'.
	 * @param bEnabled The action will be activated by the shortcut if set to 'true'.
	 */
	KAccelAction* insert( const QString& sAction, const QString& sLabel, const QString& sWhatsThis,
	                 const KShortcut& cutDef,
	                 const QObject* pObjSlot, const char* psMethodSlot,
	                 bool bConfigurable = true, bool bEnabled = true );

	/**
	 * This is an overloaded function provided for convenience.
	 * The advantage of this is when you want to use the same text for the name
	 * of the action as for the user-visible label.
	 *
	 * Usage:
	 * \code
	 * insert( i18n("Do Something"), ALT+Key_D, this, SLOT(slotDoSomething()) );
	 * \endcode
	 *
	 * @param psAction The name AND label of the action.
	 * @param cutDef The default shortcut for this action.
	 * @param pObjSlot Pointer to the slot object.
	 * @param psMethodSlot Pointer to the slot method.
	 * @param bConfigurable Allow the user to change this shortcut if set to 'true'.
	 * @param bEnabled The action will be activated by the shortcut if set to 'true'.
	 */
	KAccelAction* insert( const char* psAction, const KShortcut& cutDef,
	                 const QObject* pObjSlot, const char* psMethodSlot,
	                 bool bConfigurable = true, bool bEnabled = true );
	/**
	 * Similar to the first insert() method, but with the action
	 * name, short description, help text, and default shortcuts all
	 * set according to one of the standard accelerators.
	 * @see KStdAccel.
	 */
	KAccelAction* insert( KStdAccel::StdAccel id,
	                 const QObject* pObjSlot, const char* psMethodSlot,
	                 bool bConfigurable = true, bool bEnabled = true );

        /**
         * Removes the accelerator action identified by the name.
         * Remember to also call updateConnections().
	 * @param sAction the name of the action to remove
	 * @return true if successful, false otherwise
         */
	bool remove( const QString& sAction );

	/**
	 * Updates the connections of the accelerations after changing them.
	 * This is only necessary if you have disabled auto updates which are
	 * on by default.
	 * @return true if successful, false otherwise
	 * @see setAutoUpdate()
	 * @see getAutoUpdate()
	 */
	bool updateConnections();

	/**
	 * Return the shortcut associated with the action named by @p sAction.
	 * @param sAction the name of the action
	 * @return the action's shortcut, or a null shortcut if not found
	 */
	const KShortcut& shortcut( const QString& sAction ) const;

	/**
	 * Set the shortcut to be associated with the action named by @p sAction.
	 * @param sAction the name of the action
	 * @param shortcut the shortcut to set
	 * @return true if successful, false otherwise
	 */
	bool setShortcut( const QString& sAction, const KShortcut &shortcut );

	/**
	 * Set the slot to be called when the shortcut of the action named
	 * by @p sAction is pressed.
	 * @param sAction the name of the action
	 * @param pObjSlot the owner of the slot
	 * @param psMethodSlot the name of the slot
	 * @return true if successful, false otherwise
	 */
	bool setSlot( const QString& sAction, const QObject* pObjSlot, const char* psMethodSlot );
	/**
	 * Enable or disable the action named by @p sAction.
	 * @param sAction the action to en-/disable
	 * @param bEnabled true to enable, false to disable
	 * @return true if successful, false otherwise
	 */
	bool setEnabled( const QString& sAction, bool bEnabled );

	/**
	 * Returns the configuration group of the settings.
	 * @return the configuration group
	 * @see KConfig
	 */
	const QString& configGroup() const;

	/**
	 * Returns the configuration group of the settings.
	 * @param name the new configuration group
	 * @see KConfig
	 */
	void setConfigGroup( const QString &name );

	/**
	 * Read all shortcuts from @p pConfig, or (if @p pConfig
	 * is zero) from the application's configuration file
	 * KGlobal::config().
	 *
	 * The group in which the configuration is stored can be
	 * set with setConfigGroup().
	 * @param pConfig the configuration file, or 0 for the application
	 *         configuration file
	 * @return true if successful, false otherwise
	 */
	bool readSettings( KConfigBase* pConfig = 0 );
	/**
	 * Write the current shortcuts to @p pConfig,
	 * or (if @p pConfig is zero) to the application's
	 * configuration file.
	 * @param pConfig the configuration file, or 0 for the application
	 *         configuration file
	 * @return true if successful, false otherwise
	 */
	bool writeSettings( KConfigBase* pConfig = 0 ) const;

	/**
	 * Emits the keycodeChanged() signal.
	 */
	void emitKeycodeChanged();

 Q_SIGNALS:
	/**
	 * Emitted when one of the key codes has changed.
	 */
	void keycodeChanged();

 protected:

 private:
	class KAccelPrivate* const d;
	friend class KAccelPrivate;
};

#endif // _KACCEL_H
