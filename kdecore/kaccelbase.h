/* This file is part of the KDE libraries
    Copyright (C) 2001 Ellis Whitehead <ellis@kde.org>

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

#ifndef _KACCELBASE_H
#define _KACCELBASE_H

#include <qmap.h>
#include <qstring.h>
#include <qvaluevector.h>

#include <kkeysequence.h>

class QObject;
class QPopupMenu;
class KConfig;
class KConfigBase;

/*
	KAccelAction holds information an a given action, such as "Execute Command"

	KAccelShortcut holds information for one given series of key
	presses which will activate the action.
	This information includes the default key assignments (one for
	keyboards with the Meta key, one without) and the current key
	series.  Values equivalent to "Alt+Tab" and "Meta+I,I" are valid.
	The key series is stored as an array of KAccelSequences.
	KAccelSequence[0] is the first key; KAccelSequence[1] is the second.

	KAccelSequence holds information on a single key sequence.
	This includes the modifier key flags (Meta, Alt, Ctrl, Shift) and
	an array of "equivalent" keys (i.e., "8" and "Keypad_8").

	1) KAccelAction = "Execute Command"
		Default3 = "Alt+F2"
		Default4 = "Meta+Enter;Alt+F2"
		1) KAccelShortcut = "Meta+Enter"
			1) KAccelSequence = "Meta+Enter"
				1) KKeySequence = Meta+Enter
				2) KKeySequence = Meta+Keypad_Enter
		2) KAccelShortcut = "Alt+F2"
			1) KAccelSequence = "Alt+F2"
				1) KKeySequence = Alt+F2
	2) KAccelAction = "Something"
		Default3 = ""
		Default4 = ""
		1) KAccelShortcut = "Meta+X,Asterisk"
			1) KAccelSequence = "Meta+X"
				1) KKeySequence = Meta+X
			2) KAccelSequence = "Asterisk"
				1) KKeySequence = Shift+8 (English layout)
				2) KKeySequence = Keypad_Asterisk
*/

//----------------------------------------------------

class KAccelSequence
{
	friend class KAccelAction;
 public:
	KAccelSequence();
	KAccelSequence( const KAccelSequence& );
	KAccelSequence( KKeySequence key );
	KAccelSequence( const QString& sKey );
	virtual ~KAccelSequence();

	uint count() const;
	QString toString( KKeySequence::I18N = KKeySequence::I18N_Yes ) const;
	KKeySequences::iterator begin();
	KKeySequences::iterator end();

	KKeySequence getKey( uint i = 0 ) const;
	void setKey( const KKeySequence& key );

	static int compare( KAccelSequence&, KAccelSequence& );
	bool operator ==( KAccelSequence& );
	bool operator ==( const KAccelSequence& );
	bool operator ==( KAccelSequence& ) const;
	bool operator ==( const KAccelSequence& ) const;

	bool operator !=( KAccelSequence& seq )
		{ return !(*this == seq); }
	bool operator !=( const KAccelSequence& seq )
		{ return !(*this == seq); }
	bool operator !=( KAccelSequence& seq ) const
		{ return !(*this == seq); }
	bool operator !=( const KAccelSequence& seq ) const
		{ return !(*this == seq); }

 protected:
	KKeySequences m_rgKeys;

 private:
	//friend class KAccel;
	//friend class KAccelAction;
	class KAccelSequencePrivate* d;
};
typedef QValueVector<KAccelSequence> KAccelSequences;

//----------------------------------------------------

class KAccelShortcut
{
	friend class KAccelAction;
 public:
	KAccelShortcut();
	KAccelShortcut( const KAccelShortcut& );
	KAccelShortcut( const QString& );
	virtual ~KAccelShortcut();

	bool init( const KAccelShortcut& );
	bool init( KKeySequence );
	bool init( const QString& );

	uint count() const;
	//KKeySequence getKeyDefault() const;
	QString toString( KKeySequence::I18N = KKeySequence::I18N_Yes ) const;

	KAccelSequence getSequence( uint i ) const;
	//void setKeySequence( const KKeySequence& key );

	KAccelSequences::iterator begin();
	KAccelSequences::iterator end();
	KAccelSequences::reference front();

	bool equals( KAccelShortcut& );
	bool equals( const KAccelShortcut& ) const;
	bool operator ==( KAccelShortcut& );
	bool operator ==( const KAccelShortcut& );
	bool operator ==( KAccelShortcut& ) const;
	bool operator ==( const KAccelShortcut& ) const;

	bool operator !=( const KAccelShortcut& cut ) const
		{ return !(*this == cut); }

	KAccelShortcut& operator =( const KAccelShortcut& cut )
		{ init( cut ); return *this; }
	KAccelShortcut& operator =( const QString& s )
		{ init( s ); return *this; }

 protected:
	KAccelSequences m_rgSequences;

 private:
	class KAccelShortcutPrivate* d;
};

//----------------------------------------------------

class KAccelShortcuts : public QValueVector<KAccelShortcut>
{
 public:
	KAccelShortcuts();
	KAccelShortcuts( const QString& s );
	KAccelShortcuts( KKeySequence );
	~KAccelShortcuts();

	bool init( const KAccelShortcuts& cuts );
	bool init( const QString& s );
	bool init( KKeySequence );
	KAccelShortcuts& operator =( KAccelShortcuts& cuts )
		{ init( cuts ); return *this; }
	KAccelShortcuts& operator =( const KAccelShortcuts& cuts )
		{ init( cuts ); return *this; }

	QString toString( KKeySequence::I18N = KKeySequence::I18N_Yes, const KAccelShortcuts* prgCutDefaults = 0 ) const;

	KAccelShortcut getShortcut( uint i ) const;

	static bool equal( KAccelShortcuts&, KAccelShortcuts& );

	bool operator ==( KAccelShortcuts& cuts );
	bool operator ==( const KAccelShortcuts& );
	bool operator ==( KAccelShortcuts& cuts ) const;
	bool operator ==( const KAccelShortcuts& cuts ) const;
};

//----------------------------------------------------

class KAccelAction
{
 public:
	QString m_sName,
	        m_sDesc;
	KAccelShortcuts m_rgCutDefaults3, m_rgCutDefaults4;
	const QObject* m_pObjSlot;
	const char* m_psMethodSlot;

	bool m_bConfigurable,
	     m_bEnabled;

	int m_nIDAccel, m_nIDMenu;
	QPopupMenu* m_pMenu;

	KAccelShortcuts m_rgShortcuts;

 public:
	KAccelAction();
	KAccelAction( const QString& sName, const QString& sDesc,
			const char* rgCutDefaults3, const char* rgCutDefaults4,
			const QObject* pObjSlot, const char* psMethodSlot,
			int nIDMenu, QPopupMenu* pMenu,
			bool bConfigurable, bool bEnabled );
	KAccelAction( const QString& sName, const QString& sDesc,
			const KAccelShortcuts& rgCutDefaults3, const KAccelShortcuts& rgCutDefaults4,
			const QObject* pObjSlot, const char* psMethodSlot,
			int nIDMenu, QPopupMenu* pMenu,
			bool bConfigurable, bool bEnabled );
	~KAccelAction();

	bool init( const QString& sName, const QString& sDesc,
			const char* rgCutDefaults3, const char* rgCutDefaults4,
			const QObject* pObjSlot, const char* psMethodSlot,
			int nIDMenu, QPopupMenu* pMenu,
			bool bConfigurable, bool bEnabled );
	bool init( const QString& sName, const QString& sDesc,
			const KAccelShortcuts& rgCutDefaults3, const KAccelShortcuts& rgCutDefaults4,
			const QObject* pObjSlot, const char* psMethodSlot,
			int nIDMenu, QPopupMenu* pMenu,
			bool bConfigurable, bool bEnabled );

	void clear();
	//bool init( const QString& );
	uint shortcutCount() const;
	KAccelShortcuts::iterator begin();
	KAccelShortcuts::iterator end();
	//KAccelShortcuts::reference front();

	const KAccelShortcuts& shortcutDefaults() const;
	KAccelShortcut getShortcut( uint i ) const;
	//KKeySequence& key( uint iShortcut = 0, uint iSequence = 0, uint iKey = 0 );

	int getID() const   { return m_nIDAccel; }
	void setID( int n ) { m_nIDAccel = n; }

	QString toString( KKeySequence::I18N = KKeySequence::I18N_Yes ) const;

	//KAccelShortcut* insertShortcut( const QString& );
	//KAccelShortcut* insertShortcut( const KAccelShortcut& );
	bool setShortcuts( const KAccelShortcuts& rgCuts );
	bool setShortcut( uint i, const KAccelShortcut& );
	void clearShortcuts();
	bool contains( KAccelShortcut& );

	//KAccelSequence* addKeySequence( KKey keyDef3, KKey keyDef4 );

	//KKeySequence getPrimaryKey() const;

 private:
	class KAccelActionPrivate* d;
};

//----------------------------------------------------

class KAccelActions : public QValueVector<KAccelAction>
{
 public:
	KAccelActions();
	virtual ~KAccelActions();

	bool init( KAccelActions& );
	bool init( KConfigBase& config, QString sGroup );

	void updateShortcuts( KAccelActions& );

	KAccelActions::iterator actionIterator( const QString& sAction );
	KAccelAction* actionPtr( const QString& sAction );
	KAccelAction* actionPtr( KAccelShortcut cut );

	bool insertLabel( const QString& sName, const QString& sDesc );
	KAccelAction* insertAction( const QString& sAction, const QString& sDesc,
	                 const char* rgCutDefaults3, const char* rgCutDefaults4,
	                 const QObject* pObjSlot = 0, const char* psMethodSlot = 0,
	                 int nIDMenu = 0, QPopupMenu *pMenu = 0,
			 bool bConfigurable = true, bool bEnabled = true );
	KAccelAction* insertAction( const QString& sAction, const QString& sDesc,
	                 const KAccelShortcuts& rgCutDefaults3, const KAccelShortcuts& rgCutDefaults4,
	                 const QObject* pObjSlot = 0, const char* psMethodSlot = 0,
	                 int nIDMenu = 0, QPopupMenu *pMenu = 0,
			 bool bConfigurable = true, bool bEnabled = true );
	bool removeAction( const QString& sAction );

	void readActions( const QString& sConfigGroup, KConfigBase* pConfig = 0 );
        void writeActions( const QString& sGroup, KConfig *config = 0,
	                   bool bWriteAll = false, bool bGlobal = false ) const;

	void setKAccel( class KAccel* );
	void emitKeycodeChanged();

 private:
	class KAccelActionsPrivate* d;

	KAccelActions& operator =( KAccelActions& );
};

typedef QMap<KKeySequence, KAccelAction*> KKeyToActionMap;

//----------------------------------------------------

/**
 * Handle keyboard accelerators.
 *
 * Allow an user to configure
 * key bindings through application configuration files or through the
 * @ref KKeyChooser GUI.
 *
 * A @ref KAccel contains a list of accelerator items. Each accelerator item
 * consists of an action name and a keyboard code combined with modifiers
 * (Shift, Ctrl and Alt.)
 *
 * For example, "Ctrl+P" could be a shortcut for printing a document. The key
 * codes are listed in ckey.h. "Print" could be the action name for printing.
 * The action name identifies the key binding in configuration files and the
 * @ref KKeyChooser GUI.
 *
 * When pressed, an accelerator key calls the slot to which it has been
 * connected. Accelerator items can be connected so that a key will activate
 * two different slots.
 *
 * A KAccel object handles key events sent to its parent widget and to all
 * children of this parent widget.
 *
 * Key binding reconfiguration during run time can be prevented by specifying
 * that an accelerator item is not configurable when it is inserted. A special
 * group of non-configurable key bindings are known as the
 * standard accelerators.
 *
 * The standard accelerators appear repeatedly in applications for
 * standard document actions such as printing and saving. Convenience methods are
 * available to insert and connect these accelerators which are configurable on
 * a desktop-wide basis.
 *
 * It is possible for a user to choose to have no key associated with
 * an action.
 *
 * The translated first argument for @ref insertItem() is used only
 * in the configuration dialog.
 *<pre>
 * KAccel *a = new KAccel( myWindow );
 * // Insert an action "Scroll Up" which is associated with the "Up" key:
 * a->insertItem( i18n("Scroll up"), "Scroll Up", "Up" );
 * // Insert an action "Scroll Down" which is not associated with any key:
 * a->insertItem( i18n("Scroll down"), "Scroll Down", 0);
 * a->connectItem( "Scroll up", myWindow, SLOT( scrollUp() ) );
 * // a->insertStdItem( KStdAccel::Print ); //not necessary, since it
 *	// is done automatially with the
 *	// connect below!
 * a->connectItem(KStdAccel::Print, myWindow, SLOT( printDoc() ) );
 *
 * a->readSettings();
 *</pre>
 *
 * If a shortcut has a menu entry as well, you could insert them like
 * this. The example is again the @ref KStdAccel::Print from above.
 *
 * <pre>
 * int id;
 * id = popup->insertItem("&Print",this, SLOT(printDoc()));
 * a->changeMenuAccel(popup, id, KStdAccel::Print );
 * </pre>
 *
 * If you want a somewhat "exotic" name for your standard print action, like
 *   id = popup->insertItem(i18n("Print &Document"),this, SLOT(printDoc()));
 * it might be a good idea to insert the standard action before as
 *          a->insertStdItem( KStdAccel::Print, i18n("Print Document") )
 * as well, so that the user can easily find the corresponding function.
 *
 * This technique works for other actions as well.  Your "scroll up" function
 * in a menu could be done with
 *
 * <pre>
 *    id = popup->insertItem(i18n"Scroll &up",this, SLOT(scrollUp()));
 *    a->changeMenuAccel(popup, id, "Scroll Up" );
 * </pre>
 *
 * Please keep the order right:  First insert all functions in the
 * acceleratior, then call a -> @ref readSettings() and @em then build your
 * menu structure.
 *
 * @short Configurable key binding support.
 * @version $Id$
 */

class KAccelBase
{
 public:
	KAccelBase();
	virtual ~KAccelBase();

	uint actionCount() const;
	KAccelActions& actions();
	bool isEnabled() const;

	KAccelActions::iterator actionIterator( const QString& sAction );
	KAccelAction* actionPtr( const QString& sAction );
	const KAccelAction* actionPtr( const QString& sAction ) const;
	KAccelAction* actionPtr( KKeySequence key );

	void setConfigGroup( const QString& group );
	void setConfigGlobal( bool global );
	virtual void setEnabled( bool bEnabled ) = 0;
	bool getAutoUpdate() { return m_bAutoUpdate; }
	// return value of AutoUpdate flag before this call.
	bool setAutoUpdate( bool bAuto );

// Procedures for manipulating Actions.
	virtual void clearActions();

	virtual bool insertLabel( const QString& sName, const QString& sDesc );
	virtual KAccelAction* insertAction( const QString& sAction, const QString& sDesc,
	                 const KAccelShortcuts& rgCutDefaults3, const KAccelShortcuts& rgCutDefaults4,
	                 const QObject* pObjSlot, const char* psMethodSlot,
	                 int nIDMenu = 0, QPopupMenu *pMenu = 0,
			 bool bConfigurable = true, bool bEnabled = true );
	virtual bool removeAction( const QString& sAction );
	bool setActionSlot( const QString& sAction, const QObject* pObjSlot, const char* psMethodSlot );

	//bool disconnectItem( const QString& sAction,
	//                     const QObject* receiver, const char *member );

	bool updateConnections();
	bool updateConnectionsIncremental( KAccelAction& action );

	bool setShortcuts( const QString& sAction, const KAccelShortcuts& rgCuts );

// Modify individual Action sub-items
	bool setActionEnabled( const QString& sAction, bool bEnable );
	void removeDeletedMenu( QPopupMenu *pMenu );

	/**
	 * Read all key associations from @p config, or (if @p config
	 * is zero) from the application's configuration file
	 * @ref KGlobal::config().
	 *
	 * The group in which the configuration is stored can be
	 * set with @ref setConfigGroup().
	 */
	void readSettings( KConfig* pConfig = 0 );

	/**
	 * Write the current configurable associations to @p config,
         * or (if @p config is zero) to the application's
	 * configuration file.
	 */
	void writeSettings( KConfig* pConfig = 0 ) const;

 protected:
	KAccelActions m_rgActions;
	KKeyToActionMap m_mapKeyToAction;
	bool m_bEnabled;
	bool m_bConfigIsGlobal;
	QString m_sConfigGroup;
	bool m_bAutoUpdate;

	virtual bool connectKey( KAccelAction&, KKeySequence ) = 0;
	virtual bool disconnectKey( KAccelAction&, KKeySequence ) = 0;

	void createKeyList( QValueVector<struct X>& rgKeys );
	bool insertConnection( KAccelAction& );
	bool removeConnection( KAccelAction& );
 private:
        class KAccelBasePrivate *d;
};

#endif // _KACCELBASE_H
