/* This file is part of the KDE libraries
    Copyright (C) 1997 Nicolas Hadacek <hadacek@via.ecp.fr>
    Copyright (C) 2001,2001 Ellis Whitehead <ellis@kde.org>

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

#ifndef __KKEYDIALOG_H__
#define __KKEYDIALOG_H__

#include <qdict.h>
#include <kdialogbase.h>
#include <klistview.h>

class QButtonGroup;
class QCheckBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QRadioButton;
class KAccel;
class KAccelActions;
class KActionCollection;
class KActionPtrList;
class KConfigBase;
class KGlobalAccel;
class KKeySequence;
class KShortcut;
class KShortcutList;

/**
 * Configure dictionaries of key/action associations for KAccel and
 * KGlobalAccel.
 *
 * The class takes care of all aspects of configuration, including
 * handling key conflicts internally. Connect to the @ref allDefault()
 * slot if you want to set all configurable shortcuts to their
 * default values.
 *
 * @short Widget for configuration of @ref KAccel and @ref KGlobalAccel.
 * @see KKeyDialog
 * @version $Id$
 * @author Nicolas Hadacek <hadacek@via.ecp.fr>
 */
class KKeyChooser : public QWidget
{
	Q_OBJECT
 public:
	enum ActionType { Application, ApplicationGlobal, Standard, Global };

	/**
	 * Constructor.
	 *
	 * @param aKeyDict A dictionary (@ref QMap) of key definitons.
	 * @param bAllowLetterShortcuts Set to false if unmodified alphanumeric
	 *  keys ('A', '1', etc.) are not permissible shortcuts.
	 **/
	KKeyChooser( QWidget* parent, ActionType type = Application, bool bAllowLetterShortcuts = true );
	KKeyChooser( KActionCollection* coll, QWidget* parent, bool bAllowLetterShortcuts = true );
	KKeyChooser( KAccel* actions, QWidget* parent, bool bAllowLetterShortcuts = true );
	KKeyChooser( KGlobalAccel* actions, QWidget* parent );
	KKeyChooser( KShortcutList*, QWidget* parent, ActionType type = Application, bool bAllowLetterShortcuts = true );

	virtual ~KKeyChooser();

	bool insert( KActionCollection* );

	void syncToConfig( const QString& sConfigGroup, KConfigBase* pConfig, bool bClearUnset );

	/**
	 * This function writes any shortcut changes back to the original
	 * action set(s).
	 */
	void commitChanges();

	/**
	 * This commits and then saves the actions to disk.
	 * Any KActionCollection objects with the xmlFile() value set
	 * will be written to an xml file.  All other will be written
	 * to the application's rc file.
	 */
	void save();

 signals:
	/**
	 * Emitted when an action's shortcut has been changed.
	 **/
	void keyChange();

 public slots:
	/**
	 * Set all keys to their default values (bindings).
	 **/
	void allDefault();
	// Whether to use the 3 or 4 modifier key scheme.
	//virtual void allDefault( bool useFourModifierKeys );
	// This determines which default is used when the 'Default' button is
	//  clicked.
	void setPreferFourModifierKeys( bool preferFourModifierKeys );

 protected:
	enum { NoKey = 1, DefaultKey, CustomKey };

	void initGUI( ActionType type, bool bAllowLetterShortcuts );
	bool insert( KAccelActions&, bool bGlobal );
	bool insert( KShortcutList* );
	void buildListView( uint iList );

	void readGlobalKeys();
	//void readStdKeys();

	void updateButtons();
	void fontChange( const QFont& _font );
	void setShortcut( const KShortcut& cut );
	bool isKeyPresent( const KShortcut& cut, bool warnuser = true );
	void _warning( const KKeySequence& seq, QString sAction, QString sTitle );

	void allDefault( QListViewItem* );

	void commitChanges( QListViewItem* pItem );

 protected slots:
	void slotNoKey();
	void slotDefaultKey();
	void slotCustomKey();
	void slotListItemSelected( QListViewItem *item );
	void capturedShortcut( const KShortcut& cut );

 protected:
	ActionType m_type;
	bool m_bAllowLetterShortcuts;
	bool m_bAllowWinKey;
	// When set, pressing the 'Default' button will select the aDefaultKeycode4,
	//  otherwise aDefaultKeycode.
	bool m_bPreferFourModifierKeys;

	QRadioButton* m_prbNone;
	QRadioButton* m_prbDef;
	QRadioButton* m_prbCustom;

 private:
	class KKeyChooserPrivate *d;
	friend class KKeyDialog;

#ifndef KDE_NO_COMPAT
 public:
	/** @obsolete */
	KKeyChooser( KAccel* actions, QWidget* parent,
			bool bCheckAgainstStdKeys,
			bool bAllowLetterShortcuts,
			bool bAllowWinKey = false );
	/** @obsolete */
	KKeyChooser( KGlobalAccel* actions, QWidget* parent,
			bool bCheckAgainstStdKeys,
			bool bAllowLetterShortcuts,
			bool bAllowWinKey = false );

 public slots:
	/**
	 * Rebuild list entries based on underlying map.
	 * Use this if you changed the underlying map.
	 **/
	void listSync();

#endif
};
typedef KKeyChooser KKeyChooser;

/**
 * The KKeyDialog class is used for configuring dictionaries of key/action
 * associations for KAccel and KGlobalAccel. It uses the KKeyChooser widget
 * and offers buttons to set all keys to defaults and invoke on-line help.
 *
 * Three static methods are supplied which provide the most convienient interface
 * to the dialog. For example you could use KAccel and KKeyDialog like this
 *
 * <pre>
 * KAccel keys;
 *
 * keys.insertItem( i18n( "Zoom in" ), "Zoom in", "+" );
 * keys.connectItem( "Zoom in", myWindow, SLOT( zoomIn() ) );
 *
 * keys.connectItem( KAccel::Print, myWindow, SLOT( print() ) );
 *
 * keys.readSettings();
 *
 * if( KKeyDialog::configureKeys( &keys ) ) {
 *	      ...
 * }
 * </pre>
 *
 * This will also implicitly save the settings. If you don't want this,
 * you can call
 *
 * <pre>
 * if( KKeyDialog::configureKeys( &keys, false ) ) { // do not save settings
 *	      ...
 * }
 * </pre>
 *
 * For XML-UI applications, this is much simpler. All you need to do is
 * <pre>
 * KKeyDialog::configureKeys(actionCollection(), xmlFile());
 * </pre>
 *
 * @short Dialog for configuration of @ref KAccel and @ref KGlobalAccel.
 * @version $Id$
 * @author Nicolas Hadacek <hadacek@via.ecp.fr>
 */
class KKeyDialog : public KDialogBase
{
  Q_OBJECT

public:
	KKeyDialog( bool bAllowLetterShortcuts = true, QWidget* parent = 0, const char* name = 0 );
	virtual ~KKeyDialog();

	bool insert( KActionCollection* );
	bool configure( bool bSaveSettings = true );
	void commitChanges();

	/**
	 * Pops up a modal dialog for configuring key settings. The dialog is initialized
	 * from a @ref KAccelBase object, and the modifications are written to that object
	 * when the dialog is closed.
	 * @return Accept if the dialog was closed with OK, Reject otherwise.
	 **/
	//static int configure( KAccelActions& actions, QWidget* parent = 0,
	//	KKeyChooser::ActionType = KKeyChooser::Application );
	//static int configure( KAccelActions& actions, const QString& sXmlFile = QString::null,
	//	QWidget* parent = 0, bool bSaveSettings = true );
	static int configure( KAccel* keys, QWidget* parent = 0, bool bSaveSettings = true );
	static int configure( KGlobalAccel* keys, QWidget* parent = 0, bool bSaveSettings = true );
	/**
	 * Pops up a modal dialog for configuring key settings. The dialog is initialized
	 * from an action collection (for XMLGUI based applications).
	 * @return Accept if the dialog was closed with OK, Reject otherwise.
	 **/
	static int configure( KActionCollection* coll,
		QWidget* parent = 0, bool bSaveSettings = true );
	//static int configure( KActionPtrList* coll, const QString& sXmlFile,
	//	QWidget* parent = 0, bool bSaveSettings = true );

	// obsolete.
	static int configureKeys( KAccel* keys, bool save_settings = true, QWidget* parent = 0 )
		{ return configure( keys, parent, save_settings ); }
	static int configureKeys( KGlobalAccel* keys, bool save_settings = true, QWidget* parent = 0 )
		{ return configure( keys, parent, save_settings ); }
	static int configureKeys( KActionCollection* coll, const QString& /*xmlfile*/,
		bool save_settings = true, QWidget* parent = 0 )
		{ return configure( coll, parent, save_settings ); }
	//static int configureKeys( KActionPtrList* coll, const QString& xmlfile,
	//	bool save_settings = true, QWidget* parent = 0 )
	//	{ return configure( coll, xmlfile, parent, save_settings ); }

private:
	KKeyChooser* m_pKeyChooser;
	QPushButton* bDefaults;
	QPushButton* bOk;
	QPushButton* bCancel;
	QPushButton* bHelp;

	class KKeyDialogPrivate* d;
};

#endif // __KKEYDIALOG_H__
