/* This file is part of the KDE libraries
    Copyright (C) 1997 Nicolas Hadacek <hadacek@via.ecp.fr>

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
#include <kaccelbase.h> // for KAccelActions
#include <kdialogbase.h>
#include <klistview.h>

class QButtonGroup;
class QCheckBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class KAccel;
class KAccelBase;
class KActionCollection;
class KGlobalAccel;

class KKeyDialogPrivate;
class KKeyChooserPrivate;
class KKeyChooserItemPrivate;

class KKeyChooserItem : public KListViewItem
{
 public:
	//KKeyChooserItem( KListView* parent, KListViewItem* after, KKeyChooserItemPrivate* pInfo );
	//KKeyChooserItem( KListViewItem* parent, KListViewItem* after, KKeyChooserItemPrivate* pInfo );
	KKeyChooserItem( KListView* parent, QListViewItem* after, KAccelAction& action );
	KKeyChooserItem( QListViewItem* parent, QListViewItem* after, KAccelAction& action );

	virtual QString text( int iCol ) const;
	KAccelAction& action() const { return *m_pAction; }

 protected:
	KAccelAction* m_pAction;
};

/**
 * Configure dictionaries of key/action associations for KAccel and
 * KGlobalAccel.
 *
 * The class takes care of all aspects of configuration, including
 * handling key conflicts internally. Connect to the @ref allDefault()
 * slot if you want to set all configurable keybindings to their
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

    enum { NoKey = 1, DefaultKey, CustomKey };

    /**
     * Constructor.
     *
     * @param aKeyDict A dictionary (@ref QMap) of key definitons.
     **/
    // bAllowLetterShortcuts should be true if i.e. 'A' should be
    //  usable as a shortcut.
    // bAllowMetaKey should be true only if this is not a shortcut
    //  for a specific application.
    KKeyChooser( KAccelActions& actions, QWidget* parent,
		bool check_against_std_keys = false,
		bool bAllowLetterShortcuts = true,
		bool bAllowMetaKey = false );
    ~KKeyChooser();

    void commitChanges();

protected:
    void init( KAccelActions& actions,
		bool check_against_std_keys,
		bool bAllowLetterShortcuts,
		bool bAllowMetaKey );

    void buildListView();

signals:
    /**
     * Emitted when a key definition has been changed.
     **/
    //void keyChange();

public slots:
    /**
     * Set all keys to their default values (bindings).
     **/
    void allDefault();
    // Whether to use the 3 or 4 modifier key scheme.
    void allDefault( bool useFourModifierKeys );
    // This determines which default is used when the 'Default' button is
    //  clicked.
    void setPreferFourModifierKeys( bool preferFourModifierKeys );

    /**
     * Rebuild list entries based on underlying map.
     * Use this if you changed the underlying map.
     **/
    void listSync();

protected slots:
    void toChange( QListViewItem *item );
    //void changeKey();
    void capturedKey( KAccelShortcut cut );
    void updateAction( QListViewItem *item );
    void defaultKey();
    void noKey();
    void keyMode( int _mode );


protected:
    void readGlobalKeys();
    void readStdKeys();

    void fontChange( const QFont& _font );
    //void readKeysInternal( QDict< int >* dict, const QString& group );

protected:

    bool isKeyPresent( KAccelShortcut cut, bool warnuser = true );
    void _warning( KAccelShortcut cut, QString sAction, QString sTitle );
    void setKey( KAccelShortcut cut );
    //QDict<int>* globalDict(); // for accessing d->globalDict ( kdebase/kcontrol/keys )
    //QDict<int>* stdDict(); // for accessing d->stdDict

    KKeyChooserPrivate *d; // this has to be private, not protected
    bool m_bAllowMetaKey;
    // If this is set, then shortcuts require a modifier:
    //  so 'A' would not be valid, whereas 'Ctrl+A' would be.
    // Note, however, that this only applies to printable characters.
    //  'F1', 'Insert', etc., could still be used.
    bool m_bAllowLetterShortcuts;
    // When set, pressing the 'Default' button will select the aDefaultKeycode4,
    //  otherwise aDefaultKeycode.
    bool m_bPreferFourModifierKeys;
};

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

  KKeyDialog( KAccelActions& actions, QWidget *parent = 0,
	      bool check_against_std_keys = false );
  virtual ~KKeyDialog();

  void commitChanges();

  /**
   * Pops up a modal dialog for configuring key settings. The dialog is initialized
   * from a @ref KAccelBase object, and the modifications are written to that object
   * when the dialog is closed.
   * @return Accept if the dialog was closed with OK, Reject otherwise.
   **/
  static int configureKeys( KAccelBase *keys, bool save_settings = true,
                            QWidget *parent = 0  );
  static int configureKeys( KAccel *keys, bool save_settings = true,
                            QWidget *parent = 0  );
  static int configureKeys( KGlobalAccel *keys, bool save_settings = true,
                            QWidget *parent = 0  );
  /**
   * Pops up a modal dialog for configuring key settings. The dialog is initialized
   * from an action collection (for XMLGUI based applications).
   * @return Accept if the dialog was closed with OK, Reject otherwise.
   **/
  static int configureKeys( KActionCollection *coll, const QString& xmlfile,
                            bool save_settings = true, QWidget *parent = 0 );

private:

  KKeyChooser* m_pKeyChooser;
  QPushButton* bDefaults;
  QPushButton* bOk;
  QPushButton* bCancel;
  QPushButton* bHelp;

  KKeyDialogPrivate *d;
};

#endif // __KKEYDIALOG_H__
