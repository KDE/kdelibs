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

#ifndef _KACCELACTION_H
#define _KACCELACTION_H

#include <qmap.h>
#include <qptrvector.h>
#include <qstring.h>
#include <qvaluevector.h>

#include <kshortcut.h>

class KAccelBase;

class QObject;
class KConfig;
class KConfigBase;

/**
 * @internal
 * A KAccelAction prepresents an action that can be executed using 
 * an accelerator key. Each KAccelAction has a name, a label, a 
 * "What's this" string and a @ref KShortcut. The user can configure and 
 * enable/disable them using @ref KKeyDialog. 
 *
 * <pre>
 *	1) KAccelAction = "Run Command"
 *		Default3 = "Alt+F2"
 *		Default4 = "Meta+Enter;Alt+F2"
 *		1) KShortcut = "Meta+Enter"
 *			1) KKeySequence = "Meta+Enter"
 *				1) KKey = "Meta+Enter"
 *					1) Meta+Enter
 *					2) Meta+Keypad_Enter
 *		2) KShortcut = "Alt+F2"
 *			1) KKeySequence = "Alt+F2"
 *				1) Alt+F2
 *	2) KAccelAction = "Something"
 *		Default3 = ""
 *		Default4 = ""
 *		1) KShortcut = "Meta+X,Asterisk"
 *			1) KKeySequence = "Meta+X,Asterisk"
 *				1) KKey = "Meta+X"
 *					1) Meta+X
 *				2) KKey = "Asterisk"
 *					1) Shift+8 (English layout)
 *					2) Keypad_Asterisk
 * </pre>
 * @short An accelerator action
 * @see KAccel
 * @see KGlobalAccel
 * @see KKeyChooser
 * @see KKeyDialog
 */
class KAccelAction
{
 public:
        /**
	 * Creates an empty KAccelAction.
	 * @see clear()
	 */
	KAccelAction();

	/**
	 * Copy constructor.
	 */
	KAccelAction( const KAccelAction& );

	/**
	 * Creates a new KAccelAction.
	 * @param sName the name of the accelerator
	 * @param sLabel the label of the accelerator (i18n!)
	 * @param sWhatsThis the What's This text (18n!)
	 * @param cutDef3 the default shortcut for 3 modifier systems
	 * @param cutDef4 the default shortcut for 4 modifier systems
	 * @param pObjSlot the receiver of a signal when the key has been 
	 *                 pressed
	 * @param psMethodSlot the slot to connect for key presses. Receives
	 *                     an int, as set by @ref setID(), as only argument
	 * @param bConfigurable if true the user can configure the shortcut
	 * @param bEnabled true if the accelerator should be enabled
	 */
	KAccelAction( const QString& sName, const QString& sLabel, const QString& sWhatsThis,
			const KShortcut& cutDef3, const KShortcut& cutDef4,
			const QObject* pObjSlot, const char* psMethodSlot,
			bool bConfigurable, bool bEnabled );
	~KAccelAction();

	/**
	 * Clears the accelerator.
	 */
	void clear();

	/**
	 * Re-initialized the KAccelAction.
	 * @param sName the name of the accelerator
	 * @param sLabel the label of the accelerator (i18n!)
	 * @param sWhatsThis the What's This text (18n!)
	 * @param cutDef3 the default shortcut for 3 modifier systems
	 * @param cutDef4 the default shortcut for 4 modifier systems
	 * @param pObjSlot the receiver of a signal when the key has been 
	 *                 pressed
	 * @param psMethodSlot the slot to connect for key presses. Receives
	 *                     an int, as set by @ref setID(), as only argument
	 * @param bConfigurable if true the user can configure the shortcut
	 * @param bEnabled true if the accelerator should be enabled
	 * @return true if successful, false otherwise
	 */
	bool init( const QString& sName, const QString& sLabel, const QString& sWhatsThis,
			const KShortcut& cutDef3, const KShortcut& cutDef4,
			const QObject* pObjSlot, const char* psMethodSlot,
			bool bConfigurable, bool bEnabled );

	/**
	 * Copies this KAccelAction.
	 */
	KAccelAction& operator=( const KAccelAction& );

	/**
	 * Returns the name of the accelerator action.
	 * @return the name of the accelerator action, can be null if not 
	 *         set
	 */
	const QString& name() const                { return m_sName; }

	/**
	 * Returns the label of the accelerator action.
	 * @return the label of the accelerator action, can be null if
	 *         not set
	 */
	const QString& label() const               { return m_sLabel; }

	/**
	 * Returns the What's This text of the accelerator action.
	 * @return the What's This text of the accelerator action, can be
	 *         null if not set
	 */
	const QString& whatsThis() const           { return m_sWhatsThis; }
	
	/**
	 * The shortcut that is actually used (may be used configured).
	 * @return the shortcut of the KAccelAction, can be null if not set
	 * @see shortcutDefault()
	 */
	const KShortcut& shortcut() const          { return m_cut; }

	/**
	 * The default shortcut for this system.
	 * @return the default shortcut on this system, can be null if not set
	 * @see shortcut()
	 * @see shortcutDefault3()
	 * @see shortcutDefault4()
	 */
	const KShortcut& shortcutDefault() const;

	/**
	 * The default shortcut for 3 modifier systems.
	 * @return the default shortcut for 3 modifier systems, can be null
	 *           if not set
	 * @see shortcutDefault()
	 * @see shortcutDefault4()
	 * @see useFourModifierKeys()
	 */
	const KShortcut& shortcutDefault3() const  { return m_cutDefault3; }

	/**
	 * The default shortcut for 4 modifier systems.
	 * @return the default shortcut for 4 modifier systems, can be null 
	 *         if not set
	 * @see shortcutDefault()
	 * @see shortcutDefault3()
	 * @see useFourModifierKeys()
	 */
	const KShortcut& shortcutDefault4() const  { return m_cutDefault4; }

	/**
	 * Returns the receiver of signals.
	 * @return the receiver of signals (can be 0 if not set)
	 */
	const QObject* objSlotPtr() const          { return m_pObjSlot; }

	/**
	 * Returns the slot for the signal.
	 * @return the slot for the signal
	 */
	const char* methodSlotPtr() const          { return m_psMethodSlot; }

	/**
	 * Checks whether the user can configure the action.
	 * @return true if configurable, false otherwise
	 */
	bool isConfigurable() const                { return m_bConfigurable; }

	/**
	 * Checks whether the action is enabled.
	 * @return true if enabled, false otherwise
	 */
	bool isEnabled() const                     { return m_bEnabled; }

	/**
	 * Sets the name of the accelerator action.
	 * @param name the new name
	 */
	void setName( const QString& name );

	/**
	 * Sets the user-readable label of the accelerator action.
	 * @param label the new label (i18n!)
	 */
	void setLabel( const QString& label );

	/**
	 * Sets the What's This text for the accelerator action.
	 * @param whatsThis the new What's This text (i18n!)
	 */
	void setWhatsThis( const QString& whatsThis );

	/**
	 * Sets the new shortcut of the accelerator action.
	 * @param rgCuts the shortcut to set
	 * @return true if successful, false otherwise
	 */
	bool setShortcut( const KShortcut& rgCuts );

	/**
	 * Sets the slot of the accelerator action.
	 * @param pObjSlot the receiver object of the signal
	 * @param psMethodSlot the slot for the signal
	 */
	void setSlot( const QObject* pObjSlot, const char* psMethodSlot );

	/**
	 * Enables or disabled configuring the action.
	 * @param configurable true to enable configurability, false to disable
	 */
	void setConfigurable( bool configurable );

	/**
	 * Enables or disabled the action.
	 * @param configurable true to enable the action, false to disable
	 */
	void setEnabled( bool enable );

	/**
	 * Retrieves the id set using @ref setID.
	 * @return the id of the accelerator action
	 */
	int getID() const   { return m_nIDAccel; }

	/**
	 * Allows you to set an id that will be used as the action 
	 * signal's argument.
	 *
	 * @param n the new id
	 * @see @ref getID()
	 */
	void setID( int n ) { m_nIDAccel = n; }

	/**
	 * Checkes whether the action is connected (emits signals).
	 * @return true if connected, false otherwise
	 */
	bool isConnected() const;

	/**
	 * Sets a key sequence of the action's shortcut.
	 * @param i the position of the sequence
	 * @param keySeq the new new sequence
	 * @return true if successful, false otherwise
	 * @see KShortcut::setSeq()
	 */
	bool setKeySequence( uint i, const KKeySequence &keySeq );
	
	/**
	 * Clears the action's shortcut. It will not contain any sequences after
	 * calling this method.
	 * @see KShortcut::clear()
	 */
	void clearShortcut();
	
	/**
	 * Checks whether the action's shortcut contains the given key sequence.
	 * @param keySeq the key sequence to check
	 * @return true if the shortcut contains the given sequence
	 * @see KShortcut::contains()
	 */
	bool contains( const KKeySequence &keySeq );

	/**
	 * Returns the string representation of the action's shortcut.
	 * @return the string representation of the action's shortcut.
	 * @see KShortcut::toString()
	 */
	QString toString() const;

	/**
	 * @internal
	 */
	QString toStringInternal() const;

	/**
	 * Returns true if four modifier keys will be used.
	 * @return true if four modifier keys will be used.
	 */
	static bool useFourModifierKeys();

	/**
	 * Selects 3 or 4 modifier default shortcuts.
	 * @param use true to use 4 modifier shortcuts, false to use
	 *            3 modifier shortcuts
	 */
	static void useFourModifierKeys( bool use );

 protected:
	QString m_sName,
	        m_sLabel,
	        m_sWhatsThis;
	KShortcut m_cut;
	KShortcut m_cutDefault3, m_cutDefault4;
	const QObject* m_pObjSlot;
	const char* m_psMethodSlot;
	bool m_bConfigurable,
	     m_bEnabled;
	int m_nIDAccel;
	uint m_nConnections;

	void incConnections();
	void decConnections();

 private:
	static int g_bUseFourModifierKeys;
	class KAccelActionPrivate* d;

	friend class KAccelActions;
	friend class KAccelBase;
};

//---------------------------------------------------------------------
// KAccelActions
//---------------------------------------------------------------------

/**
 * @internal
 * This class represents a collection of @ref KAccelAction objects.
 *
 * @short A collection of accelerator actions
 * @see KAccelAction
 */
class KAccelActions
{
 public:
       /**
	* Creates a new, empty KAccelActions object.
	*/
	KAccelActions();

	/**
	 * Copy constructor (deep copy).
	 */
	KAccelActions( const KAccelActions& );
	virtual ~KAccelActions();

	/**
	 * Removes all items from this collection.
	 */
	void clear();

	/**
	 * Initializes this object with the given actions.
	 * It will make a deep copy of all actions.
	 * @param actions the actions to copy
	 * @return true if successful, false otherwise
	 */
	bool init( const KAccelActions &actions );

	/**
	 * Loads the actions from the given configuration file.
	 *
	 * @param config the configuration file to load from
	 * @param sGroup the group in the configuration file
	 * @return true if successful, false otherwise
	 */
	bool init( KConfigBase& config, const QString& sGroup );

	/**
	 * Updates the shortcuts of all actions in this object
	 * with the shortcuts from the given object.
	 * @param shortcuts the collection that contains the new
	 *        shortcuts
	 */
	void updateShortcuts( KAccelActions &shortcuts );

	/**
	 * Retrieves the index of the action with the given name.
	 * @param sAction the action to search
	 * @return the index of the action, or -1 if not found
	 */
	int actionIndex( const QString& sAction ) const;

	/**
	 * Returns the action with the given @p index.
	 * @param index the index of an action. You must not
	 *         use an index that is too high.
	 * @return the KAccelAction with the given index
	 * @see count()
	 */
	KAccelAction* actionPtr( uint index );

	/**
	 * Returns the action with the given @p index.
	 * @param index the index of an action. You must not
	 *         use an index that is too high.
	 * @return the KAccelAction with the given index
	 * @see count()
	 */
	const KAccelAction* actionPtr( uint index ) const;

	/**
	 * Returns the action with the given name.
	 * @param aAction the name of the action to search
	 * @return the KAccelAction with the given name, or 0
	 *          if not found
	 */
	KAccelAction* actionPtr( const QString& sAction );

	/**
	 * Returns the action with the given name.
	 * @param aAction the name of the action to search
	 * @return the KAccelAction with the given name, or 0
	 *          if not found
	 */
	const KAccelAction* actionPtr( const QString& sAction ) const;

	/**
	 * Returns the action with the given key sequence.
	 * @param cut the sequence to search for
	 * @return the KAccelAction with the given sequence, or 0
	 *          if not found
	 */
	KAccelAction* actionPtr( KKeySequence cut );

	/**
	 * Returns the action with the given @p index.
	 * @param index the index of an action. You must not
	 *         use an index that is too high.
	 * @return the KAccelAction with the given index
	 * @see actionPtr()
	 * @see count()
	 */
	KAccelAction& operator []( uint index );

	/**
	 * Returns the action with the given @p index.
	 * @param index the index of an action. You must not
	 *         use an index that is too high.
	 * @return the KAccelAction with the given index
	 * @see actionPtr()
	 * @see count()
	 */
	const KAccelAction& operator []( uint index ) const;

	/**
	 * Inserts an action into the collection.
	 * @param sName the name of the accelerator
	 * @param sLabel the label of the accelerator (i18n!)
	 * @param sWhatsThis the What's This text (18n!)
	 * @param cutDef3 the default shortcut for 3 modifier systems
	 * @param cutDef4 the default shortcut for 4 modifier systems
	 * @param pObjSlot the receiver of a signal when the key has been 
	 *                 pressed
	 * @param psMethodSlot the slot to connect for key presses. Receives
	 *                     an int, as set by @ref setID(), as only argument
	 * @param bConfigurable if true the user can configure the shortcut
	 * @param bEnabled true if the accelerator should be enabled
	 * @return the new action
	 */
	KAccelAction* insert( const QString& sAction, const QString& sLabel, const QString& sWhatsThis,
			const KShortcut& rgCutDefaults3, const KShortcut& rgCutDefaults4,
			const QObject* pObjSlot = 0, const char* psMethodSlot = 0,
			bool bConfigurable = true, bool bEnabled = true );

	/**
	 * Inserts an action into the collection.
	 * @param sName the name of the accelerator
	 * @param sLabel the label of the accelerator (i18n!)
	 * @return the new action
	 */
	KAccelAction* insert( const QString& sName, const QString& sLabel );

	/**
	 * Removes the given action.
	 * @param sAction the name of the action.
	 * @return true if successful, false otherwise
	 */
	bool remove( const QString& sAction );

	/**
	 * Loads the actions from the given configuration file.
	 *
	 * @param sConfigGroup the group in the configuration file
	 * @param pConfig the configuration file to load from
	 * @return true if successful, false otherwise
	 */
	bool readActions( const QString& sConfigGroup = "Shortcuts", KConfigBase* pConfig = 0 );

	/**
	 * Writes the actions to the given configuration file.
	 *
	 * @param sConfigGroup the group in the configuration file
	 * @param pConfig the configuration file to save to
	 * @param bWriteAll true to write all actions
	 * @param bGlobal true to write to the global configuration file
	 * @return true if successful, false otherwise
	 */
	bool writeActions( const QString& sConfigGroup = "Shortcuts", KConfigBase* pConfig = 0,
			bool bWriteAll = false, bool bGlobal = false ) const;

	/**
	 * Emit a keycodeChanged signal.
	 */
	void emitKeycodeChanged();

	/**
	 * Returns the number of actions in the collection.
	 * @return the number of actions
	 */
	uint count() const;

 protected:
	KAccelBase* m_pKAccelBase;
	KAccelAction** m_prgActions;
	uint m_nSizeAllocated, m_nSize;

	void resize( uint );
	void insertPtr( KAccelAction* );

 private:
	class KAccelActionsPrivate* d;

	KAccelActions( KAccelBase* );
	void initPrivate( KAccelBase* );
	KAccelActions& operator =( KAccelActions& );

	friend class KAccelBase;
};

#endif // _KACCELACTION_H
