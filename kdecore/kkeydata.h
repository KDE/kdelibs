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
	
 private:
	QDictIterator<KKeyEntry> *aIt;
	KKeyEntry *pEntry;
	QString sEntryKey;
	QListBox *wList;
	QLabel *lInfo, *lNotConfig;
	QPushButton *bAllDefault, *bChange, *bEdit, *bDefault, *bOk, *bCancel;
	QCheckBox *cShift, *cCtrl, *cAlt;
	QFrame *fCArea;
	QLineEdit *eKey;
	
	bool bKeyIntercept;
	
	const QString item( uint keyCode, const QString& entryKey );
	bool isKeyPresent();
	void setKey( uint kCode );
};



#endif
