#include <qkeycode.h>

#include <kkeyconf.h>
#include "kkeydata.moc"
#include <ckey.h>

void initKKeyConfig( KConfig *pconf )
{
	if ( kKeys )
		return;
	new KKeyConfig(pconf);
}

KKeyConfig* KKeyConfig::pKKeyConfig = 0L;

KKeyConfig::KKeyConfig( KConfig *pconf )
    : aKeyDict(37), aWidgetDict(11)
{
	pKKeyConfig = this;
	if (!pconf)
		fatal("KKeyConfig : Null KConfig object");
	pConfig = pconf;
	aAvailableId = 1;
	aKeyDict.setAutoDelete(TRUE);
	aWidgetDict.setAutoDelete(TRUE);
}

KKeyConfig::~KKeyConfig()
{
	sync();

	/* remove all widgetEntries */
	aWidgetDict.clear();
}

uint KKeyConfig::readCurrentKey( const QString& functionName )
{
	KKeyEntry *pEntry = aKeyDict[ functionName ];
	
	if ( !pEntry )
		return 0;
	else
		return pEntry->aCurrentKeyCode;
}

uint KKeyConfig::readDefaultKey( const QString& functionName )
{
	KKeyEntry *pEntry = aKeyDict[ functionName ];
	
	if ( !pEntry )
        return 0;
    else
        return pEntry->aDefaultKeyCode;
}


bool KKeyConfig::addKey( const QString& functionName, uint keyCode,
					   bool configurable )
{
	/* search for an existing "functionName" entry */
	KKeyEntry *pEntry = aKeyDict[ functionName ];
	
	if ( pEntry ) /* already exists : remove it */
		removeKey(functionName);

	pEntry = new KKeyEntry;
	aKeyDict.insert( functionName, pEntry );
	
	pEntry->aDefaultKeyCode = keyCode;
	pEntry->aCurrentKeyCode = keyCode;
	pEntry->bConfigurable = configurable;
	pEntry->aAccelId = 0;
	pEntry->pConnectDict = NULL;

	if ( !configurable )
		return TRUE;

	/* search an entry in the KConfig object */
	pConfig->setGroup("keys");
	if ( !pConfig->hasKey(functionName) )
		return TRUE;
	
	/* read and recognize the key */
	int aKeyCode = stringToKey( pConfig->readEntry(functionName) );
	if ( aKeyCode ) /* if recognized */
		pEntry->aCurrentKeyCode = aKeyCode;
	
	return TRUE;
}

bool KKeyConfig::addKey( const QString& functionName, 
					   const QString& keyCode, bool configurable )
{
	uint iKeyCode = stringToKey(keyCode);
	if ( iKeyCode==0 ) {
		QString str;
		str.sprintf("addKey : unrecognized key string %s", 
					(const char *)keyCode);
		warning(str);
		return FALSE;
	}
	
	return addKey(functionName, iKeyCode, configurable);
}
	
void KKeyConfig::removeKey( const QString& functionName )
{
	/* search for an existing "functionName" entry */
    KKeyEntry *pEntry = aKeyDict[ functionName ];
	
    if ( !pEntry ) 
		return;
	
	/* disconnect eventuals connections */
	if ( pEntry->aAccelId ) {
		QDictIterator<KKeyConnectEntry> aConnectIt(*pEntry->pConnectDict);
		aConnectIt.toFirst();
		while ( aConnectIt.current() ) {
			disconnectFunction( aConnectIt.currentKey(), functionName );
			++aConnectIt;
		}
	}
	
	aKeyDict.remove( functionName );
}

void KKeyConfig::connectFunction( const QString& widgetName,
							    const QString& functionName,
							    const QObject* receiver, const char* member,
							    bool activate )
{
	QString str;
	
	/* search for an existing "widgetName" entry */
	KKeyWidgetEntry *pWEntry = aWidgetDict[ widgetName ];
	if ( !pWEntry ) {
		str.sprintf( "connectFunction : \"%s\" widget has not been initialized",
					 (const char *)widgetName );
		warning(str);
		return;
	}
	
	/* search for an existing "functionName" entry */
    KKeyEntry *pEntry = aKeyDict[ functionName ];
	if ( !pEntry ) {
		str.sprintf( "connectFunction : \"%s\" function does not exist",
					 (const char *)functionName );
		warning(str);
		return;
	}
	
	KKeyConnectEntry *pCEntry;
	
	/* if the widget is already connected */
	if ( pEntry->pConnectDict ) {
		pCEntry = (*pEntry->pConnectDict)[ widgetName ];
		if ( pCEntry )
			internalDisconnectFunction( widgetName, pWEntry, pEntry, pCEntry );
	} else {
		pEntry->pConnectDict = new QDict<KKeyConnectEntry>(13);
		pEntry->pConnectDict->setAutoDelete(TRUE);
		pEntry->aAccelId = aAvailableId;
		aAvailableId++;
	}
		
	pCEntry = new KKeyConnectEntry;
	pCEntry->pReceiver = (QObject *)receiver;
	pCEntry->sMember = member;
	pEntry->pConnectDict->insert( widgetName, pCEntry );
	
	pWEntry->createItem( pEntry->aAccelId, pEntry->aCurrentKeyCode,
						 (QObject *)receiver, member );
	
	if ( !activate )
		toggleFunction( widgetName, functionName, FALSE );
}

void KKeyConfig::toggleFunction( const QString& widgetName,
							   const QString& functionName, bool activate )
{	
	QString str;
	
	/* search for an existing "widgetName" entry */
	KKeyWidgetEntry *pWEntry = aWidgetDict[ widgetName ];
	if ( !pWEntry ) {
		str.sprintf( "(dis)activateFunction : \"%s\" widget has not been initialized",
					 (const char *)widgetName );
		warning(str);
		return;
	}
	
	/* search for an existing "functionName" entry */
    KKeyEntry *pEntry = aKeyDict[ functionName ];
	if ( !pEntry ) {
		str.sprintf( "(dis)activateFunction : \"%s\" function does not exist",
					 (const char *)functionName );
		warning(str);
		return;
	}

	pWEntry->setItemEnabled( pEntry->aAccelId, activate );
}

void  KKeyConfig::disconnectFunction( const QString& widgetName,
								    const QString& functionName )
{
	/* search for an existing "widgetName" entry */
	KKeyWidgetEntry *pWEntry = aWidgetDict[ widgetName ];
	if ( !pWEntry ) {
		QString str;
		str.sprintf( "disconnectFunction : \"%s\" widget has not been initialized",
					 (const char *)widgetName );
		warning(str);
		return;
	}
	
	/* search for an existing "functionName" entry */
    KKeyEntry *pEntry = aKeyDict[ functionName ];
    if ( !pEntry ) 
		return;
	/* search for a connection */
	KKeyConnectEntry *pCEntry = (*pEntry->pConnectDict)[ widgetName ];
	if ( !pCEntry ) 
		return;

	internalDisconnectFunction( widgetName, pWEntry, pEntry, pCEntry );
	
	/* if this was the unique connection of the functionName : delete the
	   pConnectDict */
	if ( pEntry->pConnectDict->count()==0 )
		delete pEntry->pConnectDict;
}

void KKeyConfig::internalDisconnectFunction( const QString& widgetName, 
	   KKeyWidgetEntry *pWEntry, KKeyEntry *pEntry, KKeyConnectEntry *pCEntry )
{
	pWEntry->deleteItem( pEntry->aAccelId, pCEntry->pReceiver,
						 pCEntry->sMember );
	pEntry->pConnectDict->remove( widgetName );
}

void KKeyConfig::registerWidget( const QString& widgetName,
							   QWidget *currentWidget )
{
	/* search for an existing "widgetName" entry */
	KKeyWidgetEntry *pWEntry = aWidgetDict[ widgetName ];
	if ( pWEntry ) {
		QString str;
		str.sprintf( "initKeyWidget : \"%s\" widget already initialized",
					 (const char *)widgetName );
		warning(str);
		return;
	}

	pWEntry = new KKeyWidgetEntry( currentWidget, widgetName );
	aWidgetDict.insert( widgetName, pWEntry );
}

void KKeyConfig::sync()
{
	/* write the current values in aKeyDict to KConfig object */
	pConfig->setGroup("keys");

	QDictIterator<KKeyEntry> aKeyIt(aKeyDict);
	aKeyIt.toFirst();
	while ( aKeyIt.current() ) {
		if ( aKeyIt.current()->bConfigurable )
			pConfig->writeEntry( aKeyIt.currentKey(),
				keyToString(aKeyIt.current()->aCurrentKeyCode) );
		++aKeyIt;
	}
}

bool KKeyConfig::configureKeys( QWidget *parent )
{
	QDictIterator<KKeyEntry> aKeyIt(aKeyDict);
	KKeyConfigure kDialog( &aKeyIt, parent );
	
	if ( kDialog.exec() ) {
		bool modified = FALSE;
		bool activated;
		/* copy all modified configKeyCodes to
		   currentKeyCodes & redo the connections */
		aKeyIt.toFirst();
		#define pE aKeyIt.current()
		while ( pE ) {
			if ( pE->aCurrentKeyCode != pE->aConfigKeyCode ) {
				modified = TRUE;
				pE->aCurrentKeyCode = pE->aConfigKeyCode;
				if ( pE->pConnectDict ) {
					QDictIterator<KKeyConnectEntry> aCIt( *pE->pConnectDict );
					aCIt.toFirst();
					while ( aCIt.current() ) {
						KKeyWidgetEntry *pWE = 
							kKeys->aWidgetDict[ aCIt.currentKey() ];
						activated = pWE->isItemEnabled( pE->aAccelId );
						pWE->deleteItem( pE->aAccelId, 
									aCIt.current()->pReceiver,
									(const char *)aCIt.current()->sMember );
						pWE->createItem( pE->aAccelId, pE->aCurrentKeyCode,
									aCIt.current()->pReceiver,
									(const char *)aCIt.current()->sMember );
						pWE->setItemEnabled( pE->aAccelId, activated );
						++aCIt;
					}
				}
			}
			++aKeyIt;
		}
		
		/* sync these new keys */
		if ( modified ) {
			sync();
			return TRUE;
		}
	}
	
	/* do nothing : no change */
	return FALSE; 
}

void KKeyConfig::disconnectAllFunctions( const QString& widgetName )
{
	QDictIterator<KKeyEntry> aKeyIt(aKeyDict);
	aKeyIt.toFirst();
	while ( aKeyIt.current() ) {
		disconnectFunction( widgetName, aKeyIt.currentKey() );
		++aKeyIt;
	}
}

void KKeyConfig::destroyWidgetEntry( const QString& widgetName )
{
	aWidgetDict.remove( widgetName );
}


void KKeyConfig::internalDisconnectAll( const QString& widgetName)
{
	QDictIterator<KKeyEntry> aKeyIt(aKeyDict); 
	aKeyIt.toFirst();
    while ( aKeyIt.current() ) {
		if ( aKeyIt.current()->pConnectDict )
			if ( (*aKeyIt.current()->pConnectDict)[ widgetName ] )
				aKeyIt.current()->pConnectDict->remove( widgetName );
        ++aKeyIt;
	 }
}


/*****************************************************************************/
const QString keyToString( uint keyCode )
{
	QString res;
	
	if ( keyCode & SHIFT )
		res = "SHIFT+";
	if ( keyCode & CTRL )
		res += "CTRL+";
	if ( keyCode & ALT )
		res += "ALT+";
	
	uint kCode = keyCode & ~(SHIFT | CTRL | ALT);

	for (int i=0; i<NB_KEYS; i++) {
		if ( kCode == (uint)KKeys[i].code ) {
			res += KKeys[i].name;
			return res;
		}
	}
	
	return QString((char *)NULL);
}

uint stringToKey(const QString& key )
{
	char *toks[4], *next_tok;
	uint keyCode = 0;
	int j, nb_toks = 0;
	char sKey[200];
	
	strncpy(sKey, (const char *)key, 200);
	next_tok = strtok(sKey,"+");
	
	if ( next_tok==NULL ) return 0;
	
	do {
		toks[nb_toks] = next_tok;
		nb_toks++;
		if ( nb_toks==5 ) return 0;
		next_tok = strtok(NULL, "+");
	} while ( next_tok!=NULL );
	
	/* we test if there is one and only one key (the others tokens
	   are accelerators) ; we also fill the keycode with infos */
	bool  keyFound = FALSE;
	for (int i=0; i<nb_toks; i++) {
		if ( strcmp(toks[i], "SHIFT")==0 )
			keyCode |= SHIFT;
		else if ( strcmp(toks[i], "CTRL")==0 )
		    keyCode |= CTRL;
	    else if ( strcmp(toks[i], "ALT")==0 )
		    keyCode |= ALT;
	    else {
			/* key already found ? */
			if ( keyFound ) return 0;
			keyFound = TRUE;
			
			/* search key */
			for(j=0; j<NB_KEYS; j++) {
				if ( strcmp(toks[i], KKeys[j].name)==0 ) {
				    keyCode |= KKeys[j].code;
					break;
				}
			}
			
			/* key name ot found ? */
			if ( j==NB_KEYS ) return 0;
		}
	}
	
	return keyCode;
}


/*****************************************************************************/
/* KKeyWidgetEntry */
KKeyWidgetEntry::KKeyWidgetEntry( QWidget *widget, const QString& widgetName )
    : QObject()
{
	pWidget = widget;
	sWidgetName = widgetName;
	pAccel = new QAccel( widget );
	
	connect( widget->topLevelWidget(), SIGNAL(destroyed()),
			 SLOT(widgetDestroyed()) );
}

KKeyWidgetEntry::~KKeyWidgetEntry()
{
	kKeys->internalDisconnectAll( sWidgetName );
//	delete pAccel;
//	disconnect( pWidget, 0, this, 0);
}

void KKeyWidgetEntry::widgetDestroyed()
{
	kKeys->destroyWidgetEntry( sWidgetName );
}

void KKeyWidgetEntry::createItem( int accelId, uint keyCode,
								  QObject *receiver, const char *member )
{
	pAccel->insertItem( keyCode, accelId );
	pAccel->connectItem( accelId, receiver, member );
}

void KKeyWidgetEntry::setItemEnabled( int accelId, bool activate )
{
	pAccel->setItemEnabled( accelId, activate );
}

bool KKeyWidgetEntry::isItemEnabled( int accelId )
{
	return pAccel->isItemEnabled( accelId );
}

void KKeyWidgetEntry::deleteItem( int accelId,
								  QObject *receiver, const char *member )
{
	pAccel->disconnectItem( accelId, receiver, member );
	pAccel->removeItem( accelId ); 
}

/*****************************************************************************/
/* KKeyConfigure */

#define V_SPACING          12
#define H_SPACING          20
#define LIST_WIDTH         100
#define LIST_HEIGHT        100
#define H_DEC              5
#define TEXT_HEIGHT        25
#define CHANGE_AREA_WIDTH  270
#define CHANGE_AREA_HEIGHT 70
#define FRAME_WIDTH        15
#define FRAME_HEIGHT       5
#define ALL_DEFAULT_WIDTH  90
#define BIG_BUTTON_WIDTH   60
#define BIG_BUTTON_HEIGHT  40
#define BIG_BUTTON_DEC     20
#define CHECK_BOX_WIDTH    50
#define CHANGE_BUTTON_WIDTH 130
#define EDIT_BUTTON_WIDTH  70
 
KKeyConfigure::KKeyConfigure( QDictIterator<KKeyEntry> *aKeyIt,
							  QWidget *parent )
    : QDialog( parent, 0, TRUE )
{
	setCaption("Keys Configuration");
	
	bKeyIntercept = FALSE;
	setFocusPolicy( QWidget::StrongFocus );
	
	/* create the listbox + copy all currentKeyCodes to configKeyCodes */
	wList = new QListBox(this);
	wList->setAutoBottomScrollBar(FALSE);
	
	aIt = aKeyIt;
	aIt->toFirst();
	while ( aIt->current() ) {
		aIt->current()->aConfigKeyCode = aIt->current()->aCurrentKeyCode;
		wList->insertItem(item(aIt->current()->aConfigKeyCode,
							   aIt->currentKey()));
		++(*aIt);
	}

	/* if no keys defined */
	if ( wList->count() == 0 ) wList->hide();
	
	/* adjust the listbox geometry */
	//  wList->setFont( QFont("Courier", 12, QFont::Normal) );
	
	/* there is a problem with the maxItemWidth() when using a non default
	 * 	      font */
	
	int lWidth = wList->maxItemWidth() + H_DEC;
	if ( lWidth < LIST_WIDTH ) 
		lWidth = LIST_WIDTH;
	wList->setGeometry( H_SPACING, V_SPACING, lWidth, LIST_HEIGHT );
	
	/* connect the list's signals */
	connect( wList, SIGNAL(highlighted(int)), SLOT(toChange(int)) );
	
	/* create the buttons */
	bAllDefault = new QPushButton(this);
	bAllDefault->setGeometry( H_SPACING + (lWidth-ALL_DEFAULT_WIDTH)/2,
							  2*V_SPACING + LIST_HEIGHT, ALL_DEFAULT_WIDTH,
							  TEXT_HEIGHT );
	bAllDefault->setText("All Default");
	connect(bAllDefault, SIGNAL(clicked()), SLOT(allDefault()));
	if ( wList->count()==0 ) bAllDefault->hide();
	
	int totalWidth = 3*H_SPACING + lWidth + CHANGE_AREA_WIDTH + 2*FRAME_WIDTH;
	int partialHeight = 3*V_SPACING + LIST_HEIGHT + TEXT_HEIGHT
		+ BIG_BUTTON_DEC;
	int hSpacing = (totalWidth-2*BIG_BUTTON_WIDTH)/3;
	
	bOk = new QPushButton(this);
	bOk->setGeometry( hSpacing, partialHeight,
					  BIG_BUTTON_WIDTH, BIG_BUTTON_HEIGHT );
	bOk->setText("OK");
	connect(bOk, SIGNAL(clicked()), SLOT(accept()));
	bCancel = new QPushButton(this);
	bCancel->setGeometry( 2*hSpacing+BIG_BUTTON_WIDTH, partialHeight,
						  BIG_BUTTON_WIDTH, BIG_BUTTON_HEIGHT );
	bCancel->setText("Cancel");
	connect(bCancel, SIGNAL(clicked()), SLOT(reject()));
	
	/* create the change area */
	fCArea = new QFrame(this);
	fCArea->setGeometry( 2*H_SPACING + lWidth, V_SPACING,
						 CHANGE_AREA_WIDTH + 2*FRAME_WIDTH,
						 CHANGE_AREA_HEIGHT + 3*FRAME_HEIGHT + TEXT_HEIGHT );
	fCArea->setFrameStyle( QFrame::Box | QFrame::Sunken );
	
	int vSpacing = (CHANGE_AREA_HEIGHT-3*TEXT_HEIGHT)/2 + TEXT_HEIGHT;
	cShift = new QCheckBox(fCArea);
	cShift->setGeometry( FRAME_WIDTH, FRAME_HEIGHT,
						 CHECK_BOX_WIDTH, TEXT_HEIGHT );
	cShift->setText("SHIFT");
	cShift->hide();
	connect(cShift, SIGNAL(clicked()), SLOT(shiftClicked()));
	cCtrl = new QCheckBox(fCArea);
	cCtrl->setGeometry( FRAME_WIDTH, FRAME_HEIGHT + vSpacing,
					    CHECK_BOX_WIDTH, TEXT_HEIGHT );
	cCtrl->setText("CTRL");
	cCtrl->hide();
	connect(cCtrl, SIGNAL(clicked()), SLOT(ctrlClicked()));
	cAlt = new QCheckBox(fCArea);
	cAlt->setGeometry( FRAME_WIDTH, FRAME_HEIGHT + 2*vSpacing,
					   CHECK_BOX_WIDTH, TEXT_HEIGHT );
	cAlt->setText("ALT");
	cAlt->hide();
	connect(cAlt, SIGNAL(clicked()), SLOT(altClicked()));
	
	hSpacing = (CHANGE_AREA_WIDTH-CHECK_BOX_WIDTH
				-CHANGE_BUTTON_WIDTH-EDIT_BUTTON_WIDTH)/2;
	vSpacing = (CHANGE_AREA_HEIGHT-TEXT_HEIGHT)/2;
	
	bChange = new QPushButton(fCArea);
	bChange->setGeometry( CHECK_BOX_WIDTH + hSpacing + FRAME_WIDTH,
						  FRAME_HEIGHT + vSpacing,
						  CHANGE_BUTTON_WIDTH, TEXT_HEIGHT );
	bChange->setFont( QFont("Helvetica", 14, QFont::Bold) );
	bChange->hide();
	connect(bChange, SIGNAL(clicked()), SLOT(changeKey()));
	
	eKey = new QLineEdit(fCArea);
	eKey->setGeometry( CHECK_BOX_WIDTH + hSpacing + FRAME_WIDTH,
					   FRAME_HEIGHT + vSpacing,
					   CHANGE_BUTTON_WIDTH, TEXT_HEIGHT );
	eKey->setFont( QFont("Helvetica", 14, QFont::Bold) );
	eKey->setMaxLength(MAX_KEY_LENGTH);
	eKey->hide();
	connect(eKey, SIGNAL(returnPressed()), SLOT(editEnd()));

	int hSpacing2 = (CHANGE_AREA_WIDTH-CHANGE_BUTTON_WIDTH)/2;
	vSpacing = (CHANGE_AREA_HEIGHT+FRAME_HEIGHT+TEXT_HEIGHT-TEXT_HEIGHT)/2;
	
	lNotConfig = new QLabel(fCArea);
	lNotConfig->setGeometry( FRAME_WIDTH + hSpacing2, FRAME_HEIGHT + vSpacing,
							 CHANGE_BUTTON_WIDTH, TEXT_HEIGHT );
	lNotConfig->setFont( QFont("Helvetica", 14, QFont::Bold) );
	lNotConfig->setAlignment( AlignCenter );
	lNotConfig->setFrameStyle( QFrame::Panel | QFrame::Sunken );
	if ( wList->count()==0 )
		lNotConfig->setText("No keys defined");
	else {
		lNotConfig->setText("Not configurable");
		lNotConfig->hide();
	}
	
	hSpacing = FRAME_WIDTH + CHECK_BOX_WIDTH + CHANGE_BUTTON_WIDTH
		       + 2*hSpacing;
	vSpacing = CHANGE_AREA_HEIGHT - 2*TEXT_HEIGHT;
	
	bEdit = new QPushButton(fCArea);
	bEdit->setGeometry( hSpacing, FRAME_HEIGHT,
					    EDIT_BUTTON_WIDTH, TEXT_HEIGHT );
	bEdit->setText("Edit");
	bEdit->hide();
	connect(bEdit, SIGNAL(clicked()), SLOT(editKey()));
	
	bDefault = new QPushButton(fCArea);
	bDefault->setGeometry( hSpacing, FRAME_HEIGHT + TEXT_HEIGHT + vSpacing,
						   EDIT_BUTTON_WIDTH, TEXT_HEIGHT );
	bDefault->setText("Default");
	bDefault->hide();
	connect(bDefault, SIGNAL(clicked()), SLOT(defaultKey()));
	
	lInfo = new QLabel(fCArea);
	lInfo->setGeometry( FRAME_WIDTH, 2*FRAME_HEIGHT + CHANGE_AREA_HEIGHT,
					    CHANGE_AREA_WIDTH, TEXT_HEIGHT );
	lInfo->setAlignment( AlignCenter );
	lInfo->hide();
}

KKeyConfigure::~KKeyConfigure()
{
	delete wList;
}

void KKeyConfigure::toChange(int index)
{
	bKeyIntercept = FALSE;
	
	/* get the entry */
	aIt->toFirst();
	(*aIt) += index;
	sEntryKey = aIt->currentKey();
	pEntry = aIt->current();
	
	eKey->hide();
	
	/* is the key configurable ? */
	if ( !pEntry->bConfigurable) {
		lInfo->hide();
		cShift->hide(); cCtrl->hide(); cAlt->hide();
		bChange->hide(); bEdit->hide(); bDefault->hide();
		lNotConfig->show();
	} else {
		lNotConfig->hide();
		lInfo->setText(""); lInfo->show();
		
		uint kCode = pEntry->aConfigKeyCode;
		uint kSCode = kCode & ~(SHIFT | CTRL | ALT);
		
		cShift->show(); cCtrl->show(); cAlt->show();
		if ( kSCode == Key_Shift ) cShift->setChecked(FALSE);
		else cShift->setChecked( kCode & SHIFT );
		if ( kSCode == Key_Control ) cCtrl->setChecked(FALSE);
		else cCtrl->setChecked( kCode & CTRL );
		if ( kSCode == Key_Alt ) cAlt->setChecked(FALSE);
		else cAlt->setChecked( kCode & ALT );
		
		QString str = keyToString( kSCode );
		bChange->setText(str); eKey->setText(str);
		bChange->show(); bEdit->show(); bDefault->show();
		
		if ( isKeyPresent() )
			lInfo->setText("Attention : key already used");
	}
}

void KKeyConfigure::defaultKey()
{
	/* change the configKeyCode */
	pEntry->aConfigKeyCode = pEntry->aDefaultKeyCode;
	
	/* update the list and the change area */
	wList->changeItem(item(pEntry->aConfigKeyCode,
						   sEntryKey), wList->currentItem());
	toChange(wList->currentItem());
}

void KKeyConfigure::allDefault()
{
	/* change all configKeyCodes to default values */
	aIt->toFirst();
	wList->clear();
	while ( aIt->current() ) {
		aIt->current()->aConfigKeyCode = aIt->current()->aDefaultKeyCode;
		wList->insertItem(item(aIt->current()->aConfigKeyCode,
							   aIt->currentKey()));
		++(*aIt);
	}
	/* clear the change area */
	lInfo->hide(); lNotConfig->hide(); eKey->hide();
	cShift->hide(); cCtrl->hide(); cAlt->hide();
	bChange->hide(); bEdit->hide(); bDefault->hide();
}

#define MAX_FCTN_LENGTH 15

const QString KKeyConfigure::item( uint keyCode, const QString& entryKey )
{
	QString str = entryKey;
	str = str.leftJustify(MAX_FCTN_LENGTH, ' ', TRUE);
	str += " : ";
	str += keyToString(keyCode);
	str = str.leftJustify( MAX_FCTN_LENGTH + 3 + 
						   MAX_KEY_LENGTH+MAX_KEY_MODIFIER_LENGTH, ' ', TRUE );
	return str;
}

void KKeyConfigure::shiftClicked()
{
	uint kSCode = pEntry->aConfigKeyCode & ~(SHIFT | CTRL | ALT);
	if ( kSCode != Key_Shift ) {
		if ( cShift->isOn() )
			pEntry->aConfigKeyCode |= SHIFT;
		else
			pEntry->aConfigKeyCode &= ~SHIFT;
		wList->changeItem(item(pEntry->aConfigKeyCode,
							   sEntryKey), wList->currentItem());
	}
	toChange(wList->currentItem());
}

void KKeyConfigure::ctrlClicked()
{
	uint kSCode = pEntry->aConfigKeyCode & ~(SHIFT | CTRL | ALT);
	if ( kSCode != Key_Control ) {
		if ( cCtrl->isOn() )
			pEntry->aConfigKeyCode |= CTRL;
		else
			pEntry->aConfigKeyCode &= ~CTRL;
		wList->changeItem(item(pEntry->aConfigKeyCode,
							   sEntryKey), wList->currentItem());
	}
	toChange(wList->currentItem());
}

void KKeyConfigure::altClicked()
{
	uint kSCode = pEntry->aConfigKeyCode & ~(SHIFT | CTRL | ALT);
	if ( kSCode != Key_Alt ) {
		if ( cAlt->isOn() )
			pEntry->aConfigKeyCode |= ALT;
		else
			pEntry->aConfigKeyCode &= ~ALT;
		wList->changeItem(item(pEntry->aConfigKeyCode,
							   sEntryKey), wList->currentItem());
	}
	toChange(wList->currentItem());
}

void KKeyConfigure::changeKey()
{
	bChange->hide();
	lInfo->setText("Press the wanted key");
	lInfo->show();
	/* give the focus to the widget */
	setFocus();
	bKeyIntercept = TRUE;
}

void KKeyConfigure::keyPressEvent( QKeyEvent *e )
{
	/* the keys are processed if the change button was pressed */
	if ( !bKeyIntercept )
		return;
	
	uint kCode = e->key() & ~(SHIFT | CTRL | ALT);
	/* check the given key :
	   if it is a non existent key (=0) : keep the old value and say
	   what happened. */
	if ( keyToString(kCode).isNull() ) {
		lInfo->setText("Undefined key");
		return;
	}
	
	bKeyIntercept = FALSE;
	setKey(kCode);
}

void KKeyConfigure::setKey( uint kCode)
{
	/* add the current modifier to the key */
	if ( kCode!=Key_Shift ) kCode |= (pEntry->aConfigKeyCode & SHIFT);
	if ( kCode!=Key_Control ) kCode |= (pEntry->aConfigKeyCode & CTRL);
	if ( kCode!=Key_Alt ) kCode |= (pEntry->aConfigKeyCode & ALT);
	
	/* set the list and the chande button */
	pEntry->aConfigKeyCode = kCode;
	wList->changeItem(item(pEntry->aConfigKeyCode,
						   sEntryKey), wList->currentItem());
	toChange(wList->currentItem());
}

void KKeyConfigure::editKey()
{
	bChange->hide(); eKey->show(); bEdit->hide();
	lInfo->setText("Return to end edition");
}

void KKeyConfigure::editEnd()
{
	uint kCode = stringToKey(eKey->text());
	if ( kCode==0 || (kCode & (SHIFT | CTRL | ALT)) ) {
		lInfo->setText("Incorrect key");
		return;
	}
	setKey(kCode);
}

bool KKeyConfigure::isKeyPresent()
{
	/* search the aConfigKeyCodes to find if this keyCode is already used
	   elsewhere */
	aIt->toFirst();
	while ( aIt->current() ) {
		if ( aIt->current()!=pEntry
			&& aIt->current()->aConfigKeyCode==pEntry->aConfigKeyCode )
			return TRUE;
		++(*aIt);
	}
	return FALSE;
}   
