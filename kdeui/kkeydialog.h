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
#ifndef KKEYDIALOG_H
#define KKEYDIALOG_H

#include <qdict.h>
#include <qaccel.h>

#include <qdialog.h>
#include <qlistbox.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qbuttongroup.h>
#include <qtableview.h>
#include <qstrlist.h>
#include <qpopupmenu.h>
#include <qgroupbox.h>
#include <qobject.h>

#include <kapp.h>
#include "kaccel.h"
#include "kglobalaccel.h"

/**
* A list box item for KSplitList.It uses two columns to display
* action/key combination pairs.
* @short A list box item for KSplitList.
*/
class KSplitListItem : public QObject, public QListBoxItem
{
	Q_OBJECT
	
public:
    KSplitListItem( const QString& s , int _id = 0);
    ~KSplitListItem (){};
    int getId(){return id;}

protected:
    virtual void paint( QPainter * );
    virtual int height( const QListBox * ) const;
    virtual int width( const QListBox * ) const;

public slots:
    void setWidth( int newWidth );

protected:
    int halfWidth;
    QString keyName;
    QString actionName;
    int id;
};

/**
* A list box that can report its width to the items it
* contains. Thus it can be used for multi column lists etc.
* @short A list box capable of multi-columns
*/
class KSplitList: public QListBox
{
	Q_OBJECT

public:
	KSplitList( QWidget *parent = 0, const char *name = 0 );
	~KSplitList() { }
	int getId(int index){
	    return ((KSplitListItem*) (item(index)))->getId();
	}

signals:
	void newWidth( int newWidth );

protected:
	void resizeEvent( QResizeEvent * );
	void paletteChange ( const QPalette & oldPalette );
	void styleChange ( GUIStyle );

};

/**
* A push button that looks like a keyboard key.
* @short A push button that looks like a keyboard key.
*/
class KKeyButton: public QPushButton
{
	Q_OBJECT

public:
	KKeyButton( const char* name = 0, QWidget *parent = 0);
	~KKeyButton();
	void setText( const QString& text );
	void setEdit( bool edit );
	bool editing;

protected:
	void paint( QPainter *_painter );
	void drawButton( QPainter *p ) { paint( p ); }
};

/**
 * The KKeyDialog class is used for configuring dictionaries of key/action
 * associations for KAccel and KGlobalAccel. It uses the KKeyChooser widget and
 * offers buttons to set all keys to defaults and invoke on-line help.
 *
 * Two static methods are supplied which provide the most convienient interface
 * to the dialog. For example you could use KAccel and KKeyDialog like this
 *
 * KAccel keys;
 *
 * keys.insertItem( i18n("Zoom in" ), "Zoom in", "+" );
 * keys.connectItem( "Zoom in", myWindow, SLOT( zoomIn() ) );
 *
 * keys.connectItem( KAccel::Print, myWindow, SLOT( print() ) );
 *
 * keys.readSettings();
 *
 * if( KKeyDialog::configureKeys( &keys ) ) {
 *		...
 *	}
 *
 * This will also implicitely save the settings. If you don't want this, you can call
 *
 * if( KKeyDialog::configureKeys( &keys, false ) ) { // do not save settings
 *		...
 *	}
 *
 */
class KKeyDialog : public QDialog
{
	Q_OBJECT
	
public:
	KKeyDialog( QDict<KKeyEntry> *aKeyDict, QWidget *parent = 0,
		    bool check_against_std_keys = false);
	~KKeyDialog() {};

	static int configureKeys( KAccel *keys, bool save_settings = true  );
	static int configureKeys( KGlobalAccel *keys,  bool save_settings = true );
	
private:
	QPushButton *bDefaults;
	QPushButton *bOk;
	QPushButton *bCancel;
	QPushButton *bHelp;
};

/**
 * The KKeyChooser widget is used for configuring dictionaries of key/action
 * associations for KAccel and KGlobalAccel.
 *
 * The class takes care of all aspects of configuration, including handling key
 * conflicts internally. Connect to the allDefault slot if you want to set all
 * configurable keybindings to their default values.
 */
class KKeyChooser : public QWidget
{
	Q_OBJECT

public:
	enum { NoKey = 1, DefaultKey, CustomKey };
	
	KKeyChooser( QDict<KKeyEntry> *aKeyDict, QWidget *parent = 0,
		     bool check_against_std_keys = false);
	~KKeyChooser();
	
	QDictIterator<KKeyEntry> *aIt;
	QDictIterator<KKeyEntry> *aKeyIt;

signals:
	void keyChange();

public slots:
	void allDefault();
	void listSync();

protected slots:
	void toChange(int index);
	void changeKey();
	void updateAction(int index);
	void defaultKey();
	void noKey();
	void keyMode(int m);
	void shiftClicked();
	void ctrlClicked();
	void altClicked();
	void editKey();
	void editEnd();
	void readGlobalKeys();
	void readStdKeys();

protected:
	void keyPressEvent( QKeyEvent *e );
	void fontChange( const QFont & );

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
};



#endif
