/* This file is part of the KDE libraries
    Copyright (C) 1997 Nicolas Hadacek <hadacek@kde.org>
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
class KConfigBase;
class KGlobalAccel;
class KKeySequence;
class KShortcut;
class KShortcutList;
class KKeyChooserItem;

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
	 * @param bAllowLetterShortcuts Set to false if unmodified alphanumeric
	 *  keys ('A', '1', etc.) are not permissible shortcuts.
	 **/
	KKeyChooser( QWidget* parent, ActionType type = Application, bool bAllowLetterShortcuts = true );
	KKeyChooser( KActionCollection* coll, QWidget* parent, bool bAllowLetterShortcuts = true );
	KKeyChooser( KAccel* actions, QWidget* parent, bool bAllowLetterShortcuts = true );
	KKeyChooser( KGlobalAccel* actions, QWidget* parent );
	KKeyChooser( KShortcutList*, QWidget* parent, ActionType type = Application, bool bAllowLetterShortcuts = true );

	virtual ~KKeyChooser();

	/**
	 * Insert an action collection, i.e. add all its actions to the ones
	 * already associated with the KKeyChooser object.
	 */
	bool insert( KActionCollection* ); // #### KDE4 : remove me
    /**
	 * Insert an action collection, i.e. add all its actions to the ones
	 * already associated with the KKeyChooser object.
	 * @param title subtree title of this collection of shortcut.
	 * @since 3.1
	 */
    bool insert( KActionCollection *, const QString &title);

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

	/**
	 * Specifies whether to use the 3 or 4 modifier key scheme.
	 * This determines which default is used when the 'Default' button is
	 * clicked.
	 */
	void setPreferFourModifierKeys( bool preferFourModifierKeys );

 // KDE4 a lot of stuff in this class should be probably private:
 protected:
	enum { NoKey = 1, DefaultKey, CustomKey };

	void initGUI( ActionType type, bool bAllowLetterShortcuts );
	bool insert( KAccel* );
	bool insert( KGlobalAccel* );
	bool insert( KShortcutList* );
	/// @since 3.1
	void buildListView( uint iList, const QString &title = QString::null );

	void readGlobalKeys();

	void updateButtons();
	void fontChange( const QFont& _font );
	void setShortcut( const KShortcut& cut );
	bool isKeyPresent( const KShortcut& cut, bool warnuser = true );
        bool isKeyPresentLocally( const KShortcut& cut, KKeyChooserItem* ignoreItem, const QString& warnText );
	void _warning( const KKeySequence& seq, QString sAction, QString sTitle );

 protected slots:
	void slotNoKey();
	void slotDefaultKey();
	void slotCustomKey();
	void slotListItemSelected( QListViewItem *item );
	void capturedShortcut( const KShortcut& cut );
        void slotSettingsChanged( int );

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


#ifndef KDE_NO_COMPAT
 public:
	/**
	 * @obsolete
	 */
	KKeyChooser( KAccel* actions, QWidget* parent,
			bool bCheckAgainstStdKeys,
			bool bAllowLetterShortcuts,
			bool bAllowWinKey = false );
	/**
	 * @obsolete
	 */
	KKeyChooser( KGlobalAccel* actions, QWidget* parent,
			bool bCheckAgainstStdKeys,
			bool bAllowLetterShortcuts,
			bool bAllowWinKey = false );

 public slots:
	/**
	 * Rebuild list entries based on underlying map.
	 * Use this if you changed the underlying map.
	 */
	void listSync();

#endif
 protected:
	virtual void virtual_hook( int id, void* data );
 private:
	class KKeyChooserPrivate *d;
	friend class KKeyDialog;
};
typedef KKeyChooser KKeyChooser;

/**
 * The KKeyDialog class is used for configuring dictionaries of key/action
 * associations for KActionCollection, KAccel, and KGlobalAccel. It uses the KKeyChooser widget
 * and offers buttons to set all keys to defaults and invoke on-line help.
 *
 * Three static methods are supplied which provide the most convienient interface
 * to the dialog. The most common and most encouraged use is with KActionCollection.
 *
 * <pre>
 * KKeyDialog::configure( actionCollection() );
 * </pre>
 *
 * @short Dialog for configuration of @ref KActionCollection, @ref KAccel, and @ref KGlobalAccel.
 * @version $Id$
 * @author Nicolas Hadacek <hadacek@via.ecp.fr>
 */
class KKeyDialog : public KDialogBase
{
	Q_OBJECT

public:
	/**
	 * Constructs a KKeyDialog called @p name as a child of @p parent.
	 * Set @p bAllowLetterShortcuts to false if unmodified alphanumeric
	 * keys ('A', '1', etc.) are not permissible shortcuts.
	 */
	KKeyDialog( bool bAllowLetterShortcuts = true, QWidget* parent = 0, const char* name = 0 );

	/**
	 * Destructor. Deletes all resources used by a KKeyDialog object.
	 */
	virtual ~KKeyDialog();

	/**
	 * Insert an action collection, i.e. add all its actions to the ones
	 * displayed by the dialog.
	 * This method can be useful in applications following the document/view
	 * design, with actions in both the document and the view.
	 * Simply call insert with the action collections of each one in turn.
	 * @return true :)
	 */
	bool insert( KActionCollection* ); // #### KDE4: remove me

        /**
         * Insert an action collection, i.e. add all its actions to the ones
	 * displayed by the dialog.
	 * This method can be useful in applications following the document/view
	 * design, with actions in both the document and the view.
	 * Simply call insert with the action collections of each one in turn.
         *
         * @param title the title associated with the collection (if null, the
         *        @ref KAboutData::progName() of the collection's instance is used)
         * @return true :)
	 * @since 3.1
	 */
        bool insert(KActionCollection *, const QString &title);

	bool configure( bool bSaveSettings = true );

	/**
	 * Commit key setting changes so that changed settings actually become active.
	 * This method is implicitly called from #configure if
	 * @p bSaveSettings is true.
	 */
	void commitChanges();

	/**
	 * Pops up a modal dialog for configuring key settings. The new
	 * shortcut settings will be active if the user presses OK.  If
	 * @p bSaveSettings is true, the settings will also be saved back to
	 * the *uirc file which they were intially read from.
	 * @return Accept if the dialog was closed with OK, Reject otherwise.
	 */
	static int configure( KActionCollection* coll, QWidget* parent = 0, bool bSaveSettings = true );

	/**
	 * This is an overloaded member function, provided for convenience.
	 * It behaves essentially like the above function, except that settings
	 * are saved to a *.rc file using KConfig.
	 */
	static int configure( KAccel* keys, QWidget* parent = 0, bool bSaveSettings = true );

	/**
	 * This is an overloaded member function, provided for convenience.
	 * It behaves essentially like the above function.
	 */
	static int configure( KGlobalAccel* keys, QWidget* parent = 0, bool bSaveSettings = true );


	/**
	 * This is an overloaded member function, provided for convenience.
	 * It behaves essentially like the above function.
	 * @param bAllowLetterShortcuts Set to false if unmodified alphanumeric
	 *  keys ('A', '1', etc.) are not permissible shortcuts.
	 */
	static int configure( KActionCollection* coll, bool bAllowLetterShortcuts, QWidget* parent = 0, bool bSaveSettings = true ); // BCI: merge with bAllowLetterShortcuts = true

	/**
	 * This is an overloaded member function, provided for convenience.
	 * It behaves essentially like the above function.
	 **/
	static int configure( KAccel* keys, bool bAllowLetterShortcuts, QWidget* parent = 0, bool bSaveSettings = true ); // BCI: merge with bAllowLetterShortcuts = true

	/**
	 * This is an overloaded member function, provided for convenience.
	 * It behaves essentially like the above function.
	 **/
	static int configure( KGlobalAccel* keys, bool bAllowLetterShortcuts, QWidget* parent = 0, bool bSaveSettings = true ); // BCI: merge with bAllowLetterShortcuts = true

	/**
	 * @deprecated Obsolete.
	 * Please use @ref KKeyDialog::configure instead
	 */
	static int configureKeys( KAccel* keys, bool save_settings = true, QWidget* parent = 0 )
		{ return configure( keys, parent, save_settings ); }
	/**
	 * @deprecated Obsolete.
	 * Please use @ref KKeyDialog::configure instead
	 */
	static int configureKeys( KGlobalAccel* keys, bool save_settings = true, QWidget* parent = 0 )
		{ return configure( keys, parent, save_settings ); }
	/**
	 * @deprecated Obsolete.
	 * Please use @ref KKeyDialog::configure instead
	 */
	static int configureKeys( KActionCollection* coll, const QString& /*xmlfile*/,
		bool save_settings = true, QWidget* parent = 0 )
		{ return configure( coll, parent, save_settings ); }

 protected:
	virtual void virtual_hook( int id, void* data );

 private:
	class KKeyDialogPrivate* d;
	KKeyChooser* m_pKeyChooser;
};

#endif // __KKEYDIALOG_H__
