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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#ifndef _KGLOBALACCEL_H_
#define _KGLOBALACCEL_H_

#include <qobject.h>
#include <kshortcut.h>

class QPopupMenu;
class QWidget;
class KAccelAction;
class KAccelActions;
class KConfigBase;

class KGlobalAccelPrivate;

/**
* KGlobalAccel allows you to have global accelerators that are independent of
* the focused window. Unlike @ref KAccel it does not matter which window is 
* currently active.
*
* @see KAccel
* @see KAccelShortcutList
* @see KKeyChooser
* @see KKeyDialog
* @short Configurable global shortcut support
*/
class KGlobalAccel : public QObject
{
	Q_OBJECT
 public:
	/**
	 * Creates a new KGlobalAccel object with the given @ref pParent and
	 * @ref psName.
	 * @param pParent the parent of the QObject
	 * @param psName the name of the QObject
	 */
	KGlobalAccel( QObject* pParent, const char* psName = 0 );
	virtual ~KGlobalAccel();

	/**
	 * Checks whether the accelerators are enabled.
	 * @return true if the KGlobalAccel is enabled
	 */
	bool isEnabled();
	
	/**
	 * Checks whether the accelerators are enabled.
	 * @return true if the KGlobalAccel is enabled
	 */
	void setEnabled( bool bEnabled );

	/**
	 * Create an accelerator action.
	 *
	 * Usage:
	 *<pre>
	 * insert( "Do Something", i18n("Do Something"),
	 *   i18n("This action allows you to do something really great with this program to "
	 *        "the currently open document."),
	 *   ALT+CTRL+Key_Q, KKey::QtWIN+CTRL+Key_Q, this, SLOT(slotDoSomething()) );
	 *</pre>
	 *
	 * @param sAction The internal name of the action.
	 * @param sLabel An i18n'ized short description of the action displayed when
	 *  using @ref KKeyChooser to reconfigure the shortcuts.
	 * @param sWhatsThis An extended description of the action.
	 * @param cutDef3 The default 3 modifier scheme shortcut.
	 * @param cutDef4 The default 4 modifier scheme shortcut.
	 * @param pObjSlot Pointer to the slot object.
	 * @param psMethodSlot Pointer to the slot method.
	 * @param bConfigurable Allow the user to change this shortcut if set to 'true'.
	 * @param bEnabled The action will be activated by the shortcut if set to 'true'.
	 */
	KAccelAction* insert( const QString& sAction, const QString& sLabel, const QString& sWhatsThis,
	                 const KShortcut& cutDef3, const KShortcut& cutDef4,
	                 const QObject* pObjSlot, const char* psMethodSlot,
	                 bool bConfigurable = true, bool bEnabled = true );
                         
        /**
         * Removes the accelerator action identified by the name.
         * Remember to also call updateConnections().
	 * @param sAction the name of the action to remove
         */
        void remove( const QString& sAction );
        
	/**
	 * Use this to insert a label into the action list.  This will be
	 * displayed when the user configures shortcuts.
	 * @param sName of the of the action to insert
	 * @param sLabel a user-readable (i18n!) name for the action
	 * @return the KAccelAction of the action
	 */
	KAccelAction* insert( const QString& sName, const QString& sLabel );

	/**
	 * Updates the connections of the accelerations after changing them. 
	 * @return true if successful, false otherwise
	 */
	bool updateConnections();

	/**
	 * Set the shortcut to be associated with the action named by @p sAction.
	 * @param sAction the name of the action
	 * @return the shortcut. If the action does not exist a null shortcut will be returned.
	 */
	const KShortcut& shortcut( const QString& sAction ) const;
	/**
	 * Set the shortcut to be associated with the action named by @p sAction.
	 * @param sAction the name of the action
	 * @param shortcut the shortcut for the action
	 * @return true if successful, false otherwise
	 */
	bool setShortcut( const QString& sAction, const KShortcut &shortcut );
	/**
	 * Set the slot to be called when the shortcut of the action named
	 * by @p sAction is pressed.
	 * @param sAction the name of the action
	 * @param pObjSlot the receiver of the signal
	 * @param psMethodSlot the slot to receive the signal
	 * @return true if successful, false otherwise
	 */
	bool setSlot( const QString& sAction, const QObject* pObjSlot, const char* psMethodSlot );
	/** 
	 * Enable or disable the action named by @p sAction.
	 * @param sAction the name of the action
	 * @param bEnabled true to enable, false to disable
	 * @return true if successful, false otherwise
	 */
	bool setEnabled( const QString& sAction, bool bEnabled );

	/**
	 * Returns the configuration group that is used to save the accelerators.
	 * @return the configuration group
	 * @see KConfig
	 */
	const QString& configGroup() const;

	/**
	 * Sets the configuration group that is used to save the accelerators.
	 * @param cg the configuration group
	 * @see KConfig
	 */
	void setConfigGroup( const QString &cg );

	/**
	 * Read all shortcuts from @p pConfig, or (if @p pConfig
	 * is zero) from the application's configuration file
	 * @ref KGlobal::config().
	 * @param pConfig the configuration file to read from, or 0 for the application
	 *                 configuration file
	 * @return true if successful, false otherwise
	 */
	bool readSettings( KConfigBase* pConfig = 0 );

	/**
	 * Write the current shortcuts to @p pConfig,
	 * or (if @p pConfig is zero) to the application's
	 * configuration file.
	 * @param pConfig the configuration file to read from, or 0 for the application
	 *                 configuration file
	 * @return true if successful, false otherwise
	 */
	bool writeSettings( KConfigBase* pConfig = 0 ) const;
	// BCI: merge these two writeSettings methods in KDE 4.0
	/**
	 * Write the current shortcuts to @p pConfig,
	 * or (if @p pConfig is zero) to the application's
	 * configuration file.  Alternatively, if bGlobal is true, then write
	 * to kdeglobals.
	 * @param pConfig the configuration file to read from, or 0 for the application
	 *                 configuration file
	 * @param bGlobal if true write the configuration to the kde global settings
	 * @return true if successful, false otherwise
	 */
	bool writeSettings( KConfigBase* pConfig, bool bGlobal ) const;

	/**
	 * @internal -- this a wrapper function to
	 * KAccelActions::useFourModifierKeys().
	 */
	static bool useFourModifierKeys();

 private:

	KAccelActions& actions();
	const KAccelActions& actions() const;

	friend class KGlobalAccelPrivate;
	friend class KAccelShortcutList;
protected:
	/// @internal
	virtual void virtual_hook( int id, void* data );
private:
	class KGlobalAccelPrivate* d;
};

#endif // _KGLOBALACCEL_H_
