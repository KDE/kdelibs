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


#include <qaccel.h>
#include <qdict.h>
#include <qobject.h>
#include <qpushbutton.h>

#include <kaccel.h>
#include <kapp.h>
#include <kdialogbase.h>
#include <kglobalaccel.h>
#include <klistbox.h>

class QButtonGroup;
class QCheckBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class KActionCollection;


/**
 *  A list box item for KSplitList.It uses two columns to display
 *  action/key combination pairs.
 *
 *  @short A list box item for KSplitList.
 */
class KSplitListItem : public QObject, public QListBoxItem
{
  Q_OBJECT
	
public:

  KSplitListItem( const QString& s , int _id = 0 );
  ~KSplitListItem () {};
  int getId() { return id; }

protected:

  virtual void paint( QPainter* );
  virtual int height( const QListBox* ) const;
  virtual int width( const QListBox* ) const;

public slots:

  void setWidth( int newWidth );

protected:

  int halfWidth;
  QString keyName;
  QString actionName;
  int id;

private:
  class KSplitListItemPrivate;
  KSplitListItemPrivate *d;
};

/**
 *  A list box that can report its width to the items it
 *  contains. Thus it can be used for multi column lists etc.
 *
 *  @short A list box capable of multi-columns
 */
class KSplitList: public KListBox
{
  Q_OBJECT

public:

  KSplitList( QWidget *parent = 0, const char *name = 0 );
  ~KSplitList() { }
  int getId(int index) { return ( (KSplitListItem*) ( item( index ) ) )->getId(); }
  void setVisibleItems( int numItem );
signals:

  void newWidth( int newWidth );

protected:

  void resizeEvent( QResizeEvent * );
  void paletteChange ( const QPalette & oldPalette );
  void styleChange ( GUIStyle );

private:
  class KSplitListPrivate;
  KSplitListPrivate *d;
};

/**
 *  A push button that looks like a keyboard key.
 *  @short A push button that looks like a keyboard key.
 */
class KKeyButton: public QPushButton
{
  Q_OBJECT

public:

  KKeyButton( QWidget *parent=0, const char *name=0 );
  // Obsolete
  KKeyButton( const char* name = 0, QWidget *parent = 0);
  ~KKeyButton();
  /**
   * Reimplemented for internal purposes.
   */
  void setText( const QString& text );
  // FIXME: make variable editing private and add accessor editing()
  void setEditing(bool editing);
  // Obsolete
  void setEdit( bool edit );
  bool editing;

protected:

  void paint( QPainter* _painter );
  void drawButton( QPainter* _painter ) { paint( _painter ); }

private:
  class KKeyButtonPrivate;
  KKeyButtonPrivate *d;
};

/**
 * The KKeyDialog class is used for configuring dictionaries of key/action
 * associations for KAccel and KGlobalAccel. It uses the KKeyChooser widget 
 * and offers buttons to set all keys to defaults and invoke on-line help.
 *
 * Two static methods are supplied which provide the most convienient interface
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
 * This will also implicitely save the settings. If you don't want this, 
 * you can call
 *
 * <pre>
 * if( KKeyDialog::configureKeys( &keys, false ) ) { // do not save settings
 *	      ...
 * }
 * </pre>
 * @short Dialog for configuration of @ref KAccel and @ref KGlobalAccel.
 * @version $Id$
 * @author Nicolas Hadacek <hadacek@via.ecp.fr>
 */
class KKeyDialog : public KDialogBase
{
  Q_OBJECT
	
public:

  KKeyDialog( QDict<KKeyEntry>* aKeyDict, QWidget *parent = 0, 
	      bool check_against_std_keys = false );
  ~KKeyDialog() {};

  static int configureKeys( KAccel *keys, bool save_settings = true, 
			    QWidget *parent = 0  );
  static int configureKeys( KGlobalAccel *keys,  bool save_settings = true,
			    QWidget *parent = 0 );
  static int configureKeys( KActionCollection *coll, const QString& xmlfile,
                            bool save_settings = true, QWidget *parent = 0 );

private:

  QPushButton* bDefaults;
  QPushButton* bOk;
  QPushButton* bCancel;
  QPushButton* bHelp;

  class KKeyDialogPrivate;
  KKeyDialogPrivate *d;
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
   * @param aKeyDict A dictionary (@ref QDict) of key definitons.
   **/
  KKeyChooser( QDict<KKeyEntry>* aKeyDict, QWidget* parent = 0,
	       bool check_against_std_keys = false );
  ~KKeyChooser();
	
  QDictIterator<KKeyEntry>* aIt;
  QDictIterator<KKeyEntry>* aKeyIt;

signals:
  /**
   * Emitted when a key definition has been changed.
   **/
  void keyChange();

public slots:

    /**
     * Set all keys to their default values (bindings).
     **/
  void allDefault();
  /** 
   * Synchronize the viewed split list with the currently used key codes.
   **/
  void listSync();

protected slots:

  void toChange( int _index );
  void changeKey();
  void updateAction( int _index );
  void defaultKey();
  void noKey();
  void keyMode( int _mode );
  void shiftClicked();
  void ctrlClicked();
  void altClicked();
  void editKey();
  void editEnd();
  void readGlobalKeys();
  void readStdKeys();

protected:

  void keyPressEvent( QKeyEvent* _event );
  void fontChange( const QFont& _font );

protected:

  QDict<int> *globalDict;
  QDict<int> *stdDict;
  KKeyEntry *pEntry;
  QString sEntryKey;
  KSplitList *wList;
  QLabel *lInfo;
  QLabel *lNotConfig;
  QLabel *actLabel;
  QLabel *keyLabel;
  KKeyButton *bChange;
  QCheckBox *cShift;
  QCheckBox *cCtrl;
  QCheckBox *cAlt;
  QGroupBox *fCArea;
  //QLineEdit *eKey;
  QButtonGroup *kbGroup;

  bool bKeyIntercept;
	
  int kbMode;

  const QString item( uint keyCode, const QString& entryKey );
  bool isKeyPresent();
  void setKey( uint kCode );

  class KKeyChooserPrivate;
  KKeyChooserPrivate *d;
};

#endif // __KKEYDIALOG_H__
