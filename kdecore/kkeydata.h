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

    You should have received a copy of the GNU General Public License
    along with this library; see the file COPYING.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
#ifndef KKEYDATA_H
#define KKEYDATA_H

#include <qdict.h>
#include <qaccel.h>

#include <qdialog.h>
#include <qlistbox.h>
#include <qlabel.h>
#include <qpushbt.h>
#include <qchkbox.h>
#include <qlined.h>
#include <qtablevw.h>
#include <qstrlist.h>
#include <qpopmenu.h>
#include <qgrpbox.h>
#include <qobject.h>

#include <kapp.h>


struct KKeyConnectEntry {
	QObject* pReceiver;
	QString sMember;
};

struct KKeyEntry {
	uint aCurrentKeyCode, aDefaultKeyCode, aConfigKeyCode;
	          // configKeyCode is used when configuring via configureKeys()
	bool bConfigurable;
	int aAccelId;
	QDict<KKeyConnectEntry> *pConnectDict;
};

/**
*  A function/accelerator pair, used internally by KKeyConfig.
*
* @short A function/accelerator pair, used internally by KKeyConfig.
* @version $Id$
*/
class KKeyWidgetEntry : public QObject
{
 Q_OBJECT
	
 public:
	KKeyWidgetEntry( QWidget *widget, const QString& widgetName );
	~KKeyWidgetEntry();
	void createItem( int accelId, uint keyCode, QObject *receiver, 
					 const char *member );
	void setItemEnabled( int accelId, bool activate );
	void deleteItem( int accelId, QObject *receiver, 
					 const char *member );
	bool isItemEnabled( int accelId );
	
 private:
	QString sWidgetName;
	QWidget *pWidget;
	QAccel *pAccel;
	
 protected slots:
	void widgetDestroyed();

private:
  // Disallow assignment and copy-construction
  KKeyWidgetEntry( const KKeyWidgetEntry& ) {};
  KKeyWidgetEntry& operator= ( const KKeyWidgetEntry& ) { return *this; }
};

/**
* A list box item for SplitList.It uses two columns to display 
* action/key combination pairs.
* @short A list box item for SplitList.
*/
class SplitListItem : public QObject, public QListBoxItem
{
	Q_OBJECT
	
public:
    SplitListItem( const char *s );

protected:
    virtual void paint( QPainter * );
    virtual int height( const QListBox * ) const;
    virtual int width( const QListBox * ) const;

public slots:
	void setWidth( int newWidth );
	
private:
	int halfWidth;
	QString keyName;
	QString actionName;
};

/**
* A list box that can report its width to the items it
* contains. Thus it can be used for multi column lists etc.
* @short A list box capable of multi-columns
*/
class SplitList: public QListBox
{
	Q_OBJECT

public:
	SplitList( QWidget *parent = 0, const char *name = 0 );
	~SplitList() { }

signals:
	void newWidth( int newWidth );
	
protected:
	void resizeEvent( QResizeEvent * );
	void paletteChange ( const QPalette & oldPalette );
	void styleChange ( GUIStyle );

private:
	QColor selectColor;
	QColor selectTextColor;
};

/**
* A push button that looks like a keyboard key.
* @short A push button that looks like a keyboard key.
*/
class KeyButton: public QPushButton
{
	Q_OBJECT

public:
	KeyButton( const char* name = 0, QWidget *parent = 0);
	~KeyButton();
	void setText( QString text );
	void setEdit( bool edit );
	bool editing;
  
protected:
	void paint( QPainter *_painter );
	void drawButton( QPainter *p ) { paint( p ); }
};

/**
* A widget for configuration of function/accelerator assignments.
* @short A widget for configuration of function/accelerator assignments.
* @version $Id$
*/
class KKeyConfigure : public QDialog
{
 Q_OBJECT
	
 public:
	KKeyConfigure( QDictIterator<KKeyEntry> *aKeyIt, QWidget *parent=0 );
	~KKeyConfigure();
	
 private slots:
	void toChange(int index);
	void allDefault();
	void changeKey();
	void defaultKey();
	void shiftClicked();
	void ctrlClicked();
	void altClicked();
	void editKey();
	void editEnd();
	
 protected:
	void keyPressEvent( QKeyEvent *e );
	void fontChange( const QFont & ); 
	
 private:
	QDictIterator<KKeyEntry> *aIt;
	KKeyEntry *pEntry;
	QString sEntryKey;
	SplitList *wList;
	QLabel *lInfo, *lNotConfig;
	QLabel *actLabel, *keyLabel;
	QPushButton *bAllDefault, *bDefault, *bOk, *bCancel, *bHelp;
	KeyButton *bChange;
	QCheckBox *cShift, *cCtrl, *cAlt;
	QGroupBox *fCArea;
	//QLineEdit *eKey;
	
	bool bKeyIntercept;
	
	const QString item( uint keyCode, const QString& entryKey );
	bool isKeyPresent();
	void setKey( uint kCode );

private:
  // Disallow assignment and copy-construction
  KKeyConfigure( const KKeyConfigure& ) {};
  KKeyConfigure& operator= ( const KKeyConfigure& ) { return *this; }
};



#endif
