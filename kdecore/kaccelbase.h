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
#include <qptrvector.h>
#include <qstring.h>
#include <qvaluevector.h>
#include <qvaluelist.h>

#include "kaccelaction.h"
#include "kkeyserver_x11.h"

class QPopupMenu;
class QWidget;

//----------------------------------------------------

/**
 * @internal
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
	enum Init { QT_KEYS = 0x00, NATIVE_KEYS = 0x01 };
	enum Signal { KEYCODE_CHANGED };

	KAccelBase( int fInitCode );
	virtual ~KAccelBase();

	uint actionCount() const;
	KAccelActions& actions();
	bool isEnabled() const;

	KAccelAction* actionPtr( const QString& sAction );
	const KAccelAction* actionPtr( const QString& sAction ) const;
	KAccelAction* actionPtr( const KKey& key );
	KAccelAction* actionPtr( const KKeyServer::Key& key );

	const QString& configGroup() const { return m_sConfigGroup; }
	void setConfigGroup( const QString& group );
	void setConfigGlobal( bool global );
	virtual void setEnabled( bool bEnabled ) = 0;
	bool getAutoUpdate() { return m_bAutoUpdate; }
	// return value of AutoUpdate flag before this call.
	bool setAutoUpdate( bool bAuto );

// Procedures for manipulating Actions.
	//void clearActions();

	KAccelAction* insert( const QString& sName, const QString& sDesc );
	KAccelAction* insert(
	                 const QString& sAction, const QString& sDesc, const QString& sHelp,
	                 const KShortcut& rgCutDefaults3, const KShortcut& rgCutDefaults4,
	                 const QObject* pObjSlot, const char* psMethodSlot,
			 bool bConfigurable = true, bool bEnabled = true );
	bool remove( const QString& sAction );
	bool setActionSlot( const QString& sAction, const QObject* pObjSlot, const char* psMethodSlot );

	bool updateConnections();

	bool setShortcut( const QString& sAction, const KShortcut& cut );

// Modify individual Action sub-items
	bool setActionEnabled( const QString& sAction, bool bEnable );

	/**
	 * Read all key associations from @p config, or (if @p config
	 * is zero) from the application's configuration file
	 * @ref KGlobal::config().
	 *
	 * The group in which the configuration is stored can be
	 * set with @ref setConfigGroup().
	 */
	void readSettings( KConfigBase* pConfig = 0 );

	/**
	 * Write the current configurable associations to @p config,
         * or (if @p config is zero) to the application's
	 * configuration file.
	 */
	void writeSettings( KConfigBase* pConfig = 0 ) const;

	QPopupMenu* createPopupMenu( QWidget* pParent, const KKeySequence& );

 // Protected methods
 protected:
	void slotRemoveAction( KAccelAction* );

	void createKeyList( QValueVector<struct X>& rgKeys );
	bool insertConnection( KAccelAction* );
	bool removeConnection( KAccelAction* );

	virtual bool emitSignal( Signal ) = 0;
	virtual bool connectKey( KAccelAction&, const KKeyServer::Key& ) = 0;
	virtual bool connectKey( const KKeyServer::Key& ) = 0;
	virtual bool disconnectKey( KAccelAction&, const KKeyServer::Key& ) = 0;
	virtual bool disconnectKey( const KKeyServer::Key& ) = 0;

 protected:
	struct ActionInfo
	{
		KAccelAction* pAction;
		uint iSeq, iVariation;
		//ActionInfo* pInfoNext; // nil if only one action uses this key.

		ActionInfo() { pAction = 0; iSeq = 0xffff; iVariation = 0xffff; }
		ActionInfo( KAccelAction* _pAction, uint _iSeq, uint _iVariation )
			{ pAction = _pAction; iSeq = _iSeq; iVariation = _iVariation; }
	};
	typedef QMap<KKeyServer::Key, ActionInfo> KKeyToActionMap;

	KAccelActions m_rgActions;
	KKeyToActionMap m_mapKeyToAction;
	QValueList<KAccelAction*> m_rgActionsNonUnique;
	bool m_bNativeKeys; // Use native key codes instead of Qt codes
	bool m_bEnabled;
	bool m_bConfigIsGlobal;
	QString m_sConfigGroup;
	bool m_bAutoUpdate;
	KAccelAction* mtemp_pActionRemoving;

 private:
	KAccelBase& operator =( const KAccelBase& );

	friend class KAccelActions;
};

#endif // _KACCELBASE_H
