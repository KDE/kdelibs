/* This file is part of the KDE libraries
    Copyright (C) 1998 Mark Donohoe <donohoe@kde.org>
    Copyright (C) 1997 Nicolas Hadacek <hadacek@via.ecp.fr>
    Copyright (C) 1998 Matthias Ettrich <ettrich@kde.org>

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

#include <qlayout.h>
#include <qpainter.h>
#include <qdrawutil.h>
#include <qradiobutton.h>


#include <qlabel.h>
#include <qbuttongroup.h>
#include <qwhatsthis.h>
#include <klistview.h>

#include <kconfig.h>
#include <kglobalaccel.h>
#include <kmessagebox.h>
#include <kxmlgui.h>
#include <kdebug.h>
#include "kkeydialog.h"

#include <kaction.h>
#include <kkey_x11.h>
#include "kkeybutton.h"

#include <kapplication.h>
#ifdef Q_WS_X11
#define XK_XKB_KEYS
#define XK_MISCELLANY
#include <X11/Xlib.h>	// For x11Event()
#include <X11/keysymdef.h> // For XK_...

#ifdef KeyPress
const int XFocusOut = FocusOut;
const int XFocusIn = FocusIn;
const int XKeyPress = KeyPress;
const int XKeyRelease = KeyRelease;
#undef KeyRelease
#undef KeyPress
#undef FocusOut
#undef FocusIn
#endif
#endif

class KKeyChooserPrivate {
public:
    QDict<int> *globalDict;
    QDict<int> *stdDict;
    KListView *wList;
    QLabel *lInfo;
    KKeyButton *bChange;
    QGroupBox *fCArea;
    QButtonGroup *kbGroup;
    KKeyEntryMap *map;

    QMap<QListViewItem*, KKeyEntryMap::Iterator> actionMap;
    bool bKeyIntercept;

    bool bAllowMetaKey;
    // If this is set, then shortcuts require a modifier:
    //  so 'A' would not be valid, whereas 'Ctrl+A' would be.
    // Note, however, that this only applies to printable characters.
    //  'F1', 'Insert', etc., could still be used.
    bool bAllowLetterShortcuts;
    // When set, pressing the 'Default' button will select the aDefaultKeycode4,
    //  otherwise aDefaultKeycode.
    bool bPreferFourModifierKeys;
};

// HACK: for getting around some of Qt's lack of Meta support
enum { QT_META_MOD = Qt::ALT << 1 };	// Supply Meta bit where Qt left it out.
//bool KKeyChooserPrivate::g_bMetaPressed = false;

/***********************************************************************/
/* KKeyButton                                                          */
/*                                                                     */
/* Added by Mark Donohoe <donohoe@kde.org>                             */
/*                                                                     */
/***********************************************************************/

KKeyButton::KKeyButton(QWidget *parent, const char *name)
  : QPushButton( parent, name )
{
  setFocusPolicy( QWidget::StrongFocus );
  editing = false;
  connect( this, SIGNAL(clicked()), this, SLOT(captureKey()) );
#ifdef Q_WS_X11
  kapp->installX11EventFilter( this );	// Allow button to capture X Key Events.
#endif
  setKey( 0 );
}

KKeyButton::~KKeyButton ()
{
	if( editing )
		captureKey( false );
}

void KKeyButton::setKey( uint _key )
{
	key = _key;
	QString keyStr = KKey::keyToString( key, true );
	setText( keyStr.isEmpty() ? i18n("None") : keyStr );
}

void KKeyButton::setText( const QString& text )
{
    QPushButton::setText( text );
    setFixedSize( sizeHint().width()+12, sizeHint().height()+8 );
}

void KKeyButton::captureKey( bool bCapture )
{
    editing = bCapture;
    if( editing == true ) {
	setFocus();
	KGlobalAccel::setKeyEventsEnabled( false );
	grabKeyboard();
	grabMouse( IbeamCursor );
    } else {
	releaseMouse();
	releaseKeyboard();
	KGlobalAccel::setKeyEventsEnabled( true );
    }
    repaint();
}

void KKeyButton::captureKey()
{
	captureKey( true );
}

#ifdef Q_WS_X11
bool KKeyButton::x11Event( XEvent *pEvent )
{
	if( editing ) {
		//kdDebug(125) << "x11Event: type: " << pEvent->type << " window: " << pEvent->xany.window << endl;
		switch( pEvent->type ) {
			case XKeyPress:
			case XKeyRelease:
				keyPressEventX( pEvent );
				return true;
			case ButtonPress:
				captureKey( false );
				setKey( key );
				return true;
		}
	}
	return QWidget::x11Event( pEvent );
}

void KKeyButton::keyPressEventX( XEvent *pEvent )
{
	uint keyModX = 0, keySymX;
	KKeyX11::keyEventXToKeyX( pEvent, 0, &keySymX, 0 );

	//kdDebug(125) << QString( "keycode: 0x%1 state: 0x%2\n" )
	//			.arg( pEvent->xkey.keycode, 0, 16 ).arg( pEvent->xkey.state, 0, 16 );

	switch( keySymX ) {
		// Don't allow setting a modifier key as an accelerator.
		// Also, don't release the focus yet.  We'll wait until
		//  we get a 'normal' key.
		case XK_Shift_L:   case XK_Shift_R:	keyModX = KKeyX11::keyModXShift(); break;
		case XK_Control_L: case XK_Control_R:	keyModX = KKeyX11::keyModXCtrl(); break;
		case XK_Alt_L:     case XK_Alt_R:	keyModX = KKeyX11::keyModXAlt(); break;
		case XK_Meta_L:    case XK_Meta_R:	keyModX = KKeyX11::keyModXMeta(); break;
		case XK_Super_L:   case XK_Super_R:
		case XK_Hyper_L:   case XK_Hyper_R:
		case XK_Mode_switch:
			break;
		default:
			uint keyCombQt = KKeyX11::keyEventXToKeyQt( pEvent );
			if( keyCombQt && keyCombQt != Qt::Key_unknown ) {
				captureKey( false );
				// The parent must decide whether this is a valid
				//  key, and if so, call setKey(uint) with the new value.
				emit capturedKey( keyCombQt );
				setKey( key );
			}
			return;
	}

	if( pEvent->type == XKeyPress )
		keyModX |= pEvent->xkey.state;
	else
		keyModX = pEvent->xkey.state & ~keyModX;

	QString keyModStr;
	if( keyModX & KKeyX11::keyModXMeta() )	keyModStr += i18n("Meta") + "+";
	if( keyModX & KKeyX11::keyModXAlt() )	keyModStr += i18n("Alt") + "+";
	if( keyModX & KKeyX11::keyModXCtrl() )	keyModStr += i18n("Ctrl") + "+";
	if( keyModX & KKeyX11::keyModXShift() )	keyModStr += i18n("Shift") + "+";

	// Display currently selected modifiers, or redisplay old key.
	if( !keyModStr.isEmpty() )
		setText( keyModStr );
	else
		setKey( key );
}
#endif

void KKeyButton::drawButton( QPainter *painter )
{
  QPointArray a( 4 );
  a.setPoint( 0, 0, 0) ;
  a.setPoint( 1, width(), 0 );
  a.setPoint( 2, 0, height() );
  a.setPoint( 3, 0, 0 );

  QRegion r1( a );
  painter->setClipRegion( r1 );
  painter->setBrush( backgroundColor().light() );
  painter->drawRoundRect( 0, 0, width(), height(), 20, 20);

  a.setPoint( 0, width(), height() );
  a.setPoint( 1, width(), 0 );
  a.setPoint( 2, 0, height() );
  a.setPoint( 3, width(), height() );

  QRegion r2( a );
  painter->setClipRegion( r2 );
  painter->setBrush( backgroundColor().dark() );
  painter->drawRoundRect( 0, 0, width(), height(), 20, 20 );

  painter->setClipping( false );
  if( width() > 12 && height() > 8 )
    qDrawShadePanel( painter, 6, 4, width() - 12, height() - 8,
                     colorGroup(), true, 1, 0L );
  if ( editing )
  {
    painter->setPen( colorGroup().base() );
    painter->setBrush( colorGroup().base() );
  }
  else
  {
    painter->setPen( backgroundColor() );
    painter->setBrush( backgroundColor() );
  }
  if( width() > 14 && height() > 10 )
    painter->drawRect( 7, 5, width() - 14, height() - 10 );

  drawButtonLabel( painter );

  painter->setPen( colorGroup().text() );
  painter->setBrush( NoBrush );
  if( hasFocus() || editing )
  {
    if( width() > 16 && height() > 12 )
      painter->drawRect( 8, 6, width() - 16, height() - 12 );
  }

}

/************************************************************************/
/* KKeyDialog                                                           */
/*                                                                      */
/* Originally by Nicolas Hadacek <hadacek@via.ecp.fr>                   */
/*                                                                      */
/* Substantially revised by Mark Donohoe <donohoe@kde.org>              */
/*                                                                      */
/* And by Espen Sand <espen@kde.org> 1999-10-19                         */
/* (by using KDialogBase there is almost no code left ;)                */
/*                                                                      */
/************************************************************************/
KKeyDialog::KKeyDialog( KKeyEntryMap *aKeyMap, QWidget *parent,
                        bool check_against_std_keys)
  : KDialogBase( parent, 0, true, i18n("Configure Key Bindings"),
                 Help|Default|Ok|Cancel, Ok )
{
  KKeyChooser *kc =  new KKeyChooser( aKeyMap, this, check_against_std_keys );
  setMainWidget(kc);
  connect( this, SIGNAL(defaultClicked()), kc, SLOT(allDefault()) );
  enableButton ( Help, false );
}

KKeyDialog::~KKeyDialog()
{
}

int KKeyDialog::configureKeys( KAccel *keys, bool save_settings,
                               QWidget *parent )
{
    KKeyEntryMap map = keys->keyDict();
    KKeyDialog kd( &map, parent );
    int retcode = kd.exec();

    if( retcode == Accepted ) {
        keys->setKeyDict( map );
        if (save_settings)
            keys->writeSettings();
    }
    return retcode;
}

int KKeyDialog::configureKeys( KGlobalAccel *keys, bool save_settings,
                               QWidget *parent )
{
    KKeyEntryMap dict = keys->keyDict();

    KKeyDialog kd( &dict, parent );
    int retcode = kd.exec();

    if( retcode == Accepted ) {
        keys->setKeyDict( dict );
        if (save_settings)
            keys->writeSettings();
    }
    return retcode;
}

int KKeyDialog::configureKeys( KActionCollection *coll, const QString& file,
                               bool save_settings, QWidget *parent )
{
    KKeyEntryMap map = coll->keyMap();

    KKeyDialog kd( &map, parent );
    int retcode = kd.exec();

    if( retcode != Accepted )
        return retcode;

    if (!save_settings) {
        coll->setKeyMap( map );
        return retcode;
    }

    // let's start saving this info
    QString raw_xml(KXMLGUIFactory::readConfigFile(file));
    QDomDocument doc;
    doc.setContent(raw_xml);

    QString tagActionProp = QString::fromLatin1( "ActionProperties" );
    QString tagAction     = QString::fromLatin1( "Action" );
    QString attrName      = QString::fromLatin1( "name" );
    QString attrAccel     = QString::fromLatin1( "accel" );

    // first, lets see if we have existing properties
    QDomElement elem;
    QDomElement it = doc.documentElement();
    KXMLGUIFactory::removeDOMComments( it );
    it = it.firstChild().toElement();
    for ( ; !it.isNull(); it = it.nextSibling().toElement() )
        if ( it.tagName() == tagActionProp ) {
            elem = it;
            break;
        }

    // if there was none, create one
    if ( elem.isNull() )
    {
        elem = doc.createElement( tagActionProp );
        doc.firstChild().appendChild(elem);
    }

  // now, iterate through our actions
  for (unsigned int i = 0; i < coll->count(); i++)
  {
    KAction *action = coll->action(i);

    // see if we changed
    KKeyEntry key = map[action->name()];
    if (key.aCurrentKeyCode == key.aConfigKeyCode)
      continue;

    // now see if this element already exists
    QDomElement act_elem;
    for ( it = elem.firstChild().toElement(); !it.isNull(); it = it.nextSibling().toElement() )
    {
      if ( it.attribute( attrName ) == action->name() )
      {
        act_elem = it;
        break;
      }
    }

    // nope, create a new one
    if ( act_elem.isNull() )
    {
      act_elem = doc.createElement( tagAction );
      act_elem.setAttribute( attrName, action->name() );
    }
    act_elem.setAttribute( attrAccel,
                           KKey::keyToString( key.aConfigKeyCode, false) );

    elem.appendChild( act_elem );
  }

  // finally, write out the result
  KXMLGUIFactory::saveConfigFile(doc, file);

  coll->setKeyMap( map );

  return retcode;
}

//************************************************************************
// KKeyChooser                                                           *
//************************************************************************
KKeyChooser::KKeyChooser( KKeyEntryMap *aKeyMap, KKeyMapOrder *pMapOrder, QWidget* parent,
			bool check_against_std_keys,
			bool bAllowLetterShortcuts,
			bool bAllowMetaKey )
: QWidget( parent )
{
	init( aKeyMap, pMapOrder, check_against_std_keys, bAllowLetterShortcuts, bAllowMetaKey );
}

KKeyChooser::KKeyChooser( KKeyEntryMap *aKeyMap, QWidget* parent,
			bool check_against_std_keys,
			bool bAllowLetterShortcuts,
			bool bAllowMetaKey )
: QWidget( parent )
{
	init( aKeyMap, 0, check_against_std_keys, bAllowLetterShortcuts, bAllowMetaKey );
}

KKeyChooser::KKeyChooser( KKeyEntryMap *aKeyMap, QWidget *parent,
                          bool check_against_std_keys)
    : QWidget( parent )
{
	init( aKeyMap, 0, check_against_std_keys, true, false );
}

void KKeyChooser::init( KKeyEntryMap *aKeyMap, KKeyMapOrder *pMapOrder,
			bool check_against_std_keys,
			bool bAllowLetterShortcuts,
			bool bAllowMetaKey )
{
  d = new KKeyChooserPrivate();

  d->bKeyIntercept = false;
  d->map = aKeyMap;
  d->bAllowMetaKey = bAllowMetaKey;
  d->bAllowLetterShortcuts = bAllowLetterShortcuts;
  d->bPreferFourModifierKeys = KKey::useFourModifierKeys();

  //
  // TOP LAYOUT MANAGER
  //
  // The following layout is used for the dialog
  //            LIST LABELS LAYOUT
  //            SPLIT LIST BOX WIDGET
  //            CHOOSE KEY GROUP BOX WIDGET
  //            BUTTONS LAYOUT
  // Items are added to topLayout as they are created.
  //

  QBoxLayout *topLayout = new QVBoxLayout( this, 0, KDialog::spacingHint() );

  QGridLayout *stackLayout = new QGridLayout(2, 2, 2);
  topLayout->addLayout( stackLayout, 10 );
  stackLayout->setRowStretch( 1, 10 ); // Only list will stretch

  //
  // CREATE SPLIT LIST BOX
  //
  // Copy all currentKeyCodes to configKeyCodes
  // and fill up the split list box with the action/key pairs.
  //
  d->wList = new KListView( this );
  d->wList->setFocus();

  stackLayout->addMultiCellWidget( d->wList, 1, 1, 0, 1 );
  QString wtstr = i18n("Here you can see a list of key bindings, \n"
                       "i.e. associations between actions (e.g. 'Copy')\n"
                       "shown in the left column and keys or combination\n"
                       "of keys (e.g. CTRL-V) shown in the right column.");

  QWhatsThis::add( d->wList, wtstr );

  d->wList->addColumn(i18n("Action"));
  d->wList->addColumn(i18n("Current Key"));

  //
  // Add all "keys" to the list
  //
  if( pMapOrder ) {
	d->wList->setSorting( -1 );
	// HACK to avoid alphabetic ording.  I'll re-write this in the
	//  next development phase where API changes are not so sensitive. -- ellis
	QListViewItem *pProgramItem, *pGroupItem = 0, *pParentItem, *pItem;

	// Initially set all of the keys to not-configurable, and then only
	//  let those which are to be displayed be configured (set below).
	//  This is so that the call to KAccel::writeKeyMap() will only write
	//  those which we are editing.
	for( KKeyEntryMap::Iterator it = aKeyMap->begin(); it != aKeyMap->end(); ++it )
		(*it).bConfigurable = false;

	pParentItem = pProgramItem = pItem = new QListViewItem( d->wList, "Shortcuts" );
	pParentItem->setExpandable( true );
	pParentItem->setOpen( true );
	pParentItem->setSelectable( false );
	for( int i = 0; i < (int)pMapOrder->count(); i++ ) {
		QString sConfigKey = (*pMapOrder)[i];
		KKeyEntryMap::Iterator it = aKeyMap->find( sConfigKey );
		kdDebug(125) << "Key: " << it.key() << endl;
		if( it.key().contains("Program:") ) {
			pItem = new QListViewItem( d->wList, pProgramItem, (*it).descr );
			pItem->setSelectable( false );
			pItem->setExpandable( true );
			pItem->setOpen( true );
			if( !pProgramItem->firstChild() )
				delete pProgramItem;
			pProgramItem = pParentItem = pItem;
		} else if( it.key().contains("Group:") ) {
			pItem = new QListViewItem( pProgramItem, pParentItem, (*it).descr );
			pItem->setSelectable( false );
			pItem->setExpandable( true );
			pItem->setOpen( true );
			if( pGroupItem && !pGroupItem->firstChild() )
				delete pGroupItem;
			pGroupItem = pParentItem = pItem;
		} else {
			(*it).aConfigKeyCode = (*it).aCurrentKeyCode;
			pItem = new QListViewItem( pParentItem, pItem, (*it).descr,
					KKey::keyToString( (*it).aConfigKeyCode, true ) );
			d->actionMap[pItem] = it;
			(*it).bConfigurable = true;
		}
	}
	if( !pProgramItem->firstChild() )
		delete pProgramItem;
	if( pGroupItem && !pGroupItem->firstChild() )
		delete pGroupItem;
  } else {

    for (KKeyEntryMap::Iterator it = aKeyMap->begin();
         it != aKeyMap->end(); ++it)
    {
      (*it).aConfigKeyCode = (*it).aCurrentKeyCode;

      QListViewItem * s = new QListViewItem(d->wList,
                                            (*it).descr,
                                            KKey::keyToString((*it).aConfigKeyCode, true));
      d->actionMap[s] = it;
    }
  }

  connect( d->wList, SIGNAL( currentChanged( QListViewItem * ) ),
           SLOT( updateAction( QListViewItem * ) ) );

  //
  // CREATE CHOOSE KEY GROUP
  //
  d->fCArea = new QGroupBox( this );
  topLayout->addWidget( d->fCArea, 1 );

  d->fCArea->setTitle( i18n("Choose a Key for the Selected Action") );
  d->fCArea->setFrameStyle( QFrame::Box | QFrame::Sunken );

  //
  // CHOOSE KEY GROUP LAYOUT MANAGER
  //
  /*QGridLayout *grid = new QGridLayout( d->fCArea, 6, 4, KDialog::spacingHint() );
  grid->setRowStretch(0,10);
  grid->setRowStretch(1,10);
  grid->setRowStretch(2,10);
  grid->setRowStretch(3,10);
  grid->setRowStretch(4,10);
  grid->setRowStretch(5,10);

  grid->setColStretch(0,0);
  grid->setColStretch(1,10);
  grid->setColStretch(2,90);
  grid->setColStretch(3,0);

  grid->addRowSpacing(0,15);
  grid->addRowSpacing(5,1);*/
  QGridLayout *grid = new QGridLayout( d->fCArea, 3, 4, KDialog::spacingHint() );
  grid->addRowSpacing( 0, 5 );

  d->kbGroup = new QButtonGroup( d->fCArea );
  d->kbGroup->hide();
  d->kbGroup->setExclusive( true );

  QRadioButton *rb = new QRadioButton( i18n("&No Key"), d->fCArea );
  d->kbGroup->insert( rb, NoKey );
  rb->setEnabled( false );
  //grid->addMultiCellWidget( rb, 1, 1, 1, 2 );
  grid->addWidget( rb, 1, 0 );
  QWhatsThis::add( rb, i18n("The selected action will not be associated with any key.") );

  rb = new QRadioButton( i18n("De&fault Key"), d->fCArea );
  d->kbGroup->insert( rb, DefaultKey );
  rb->setEnabled( false );
  //grid->addMultiCellWidget( rb, 2, 2, 1, 2 );
  grid->addWidget( rb, 1, 1 );
  QWhatsThis::add( rb, i18n("This will bind the default key to the selected action. Usually a reasonable choice.") );

  rb = new QRadioButton( i18n("Custom &Key"), d->fCArea );
  d->kbGroup->insert( rb, CustomKey );
  rb->setEnabled( false );
  //grid->addMultiCellWidget( rb, 3, 3, 1, 2 );
  grid->addWidget( rb, 1, 2 );
  QWhatsThis::add( rb, i18n("If this option is selected you can create a customized key binding for the"
    " selected action using the buttons below.") );

  connect( d->kbGroup, SIGNAL( clicked( int ) ), SLOT( keyMode( int ) ) );

  QBoxLayout *pushLayout = new QHBoxLayout( KDialog::spacingHint() );
  grid->addLayout( pushLayout, 1, 3 );

  d->bChange = new KKeyButton(d->fCArea, "key");
  d->bChange->setEnabled( false );
  connect( d->bChange, SIGNAL( capturedKey(uint) ), SLOT( capturedKey(uint) ) );
  grid->addRowSpacing( 1, d->bChange->sizeHint().height() + 5 );

  wtstr = i18n("Use this button to choose a new shortcut key. Once you click it, "
  		"you can press the key-combination which you would like to be assigned "
		"to the currently selected action.");
  QWhatsThis::add( d->bChange, wtstr );

  //
  // Add widgets to the geometry manager
  //
  pushLayout->addSpacing( KDialog::spacingHint()*2 );
  pushLayout->addWidget( d->bChange );
  pushLayout->addStretch( 10 );

  d->lInfo = new QLabel(d->fCArea);
  //resize(0,0);
  //d->lInfo->setAlignment( AlignCenter );
  //d->lInfo->setEnabled( false );
  //d->lInfo->hide();
  grid->addMultiCellWidget( d->lInfo, 2, 2, 0, 3 );

  d->globalDict = new QDict<int> ( 100, false );
  d->globalDict->setAutoDelete( true );
  readGlobalKeys();
  d->stdDict = new QDict<int> ( 100, false );
  d->stdDict->setAutoDelete( true );
  if (check_against_std_keys)
    readStdKeys();
}

KKeyChooser::~KKeyChooser()
{
  delete d->globalDict;
  delete d->stdDict;
  delete d->wList;
  delete d;
  // Make sure that we don't still have global accelerators turned off.
  //KGlobalAccel::setKeyEventsEnabled( true );
}

void KKeyChooser::updateAction( QListViewItem *item )
{
	toChange( item );
}

void KKeyChooser::readKeysInternal( QDict< int >* dict, const QString& group )
{
  dict->clear();

  // Insert all keys into dict
  int *keyCode;
  KConfig pConfig;
  QMap<QString, QString> tmpMap = pConfig.entryMap( group );
  QMap<QString, QString>::Iterator gIt(tmpMap.begin());
  for (; gIt != tmpMap.end(); ++gIt) {
    if ( (*gIt).isEmpty() || *gIt == "default" )  // old code used to write just "default"
        continue;                                //  which is not enough
    kdDebug( 125 ) << gIt.key() << " " << *gIt << endl;
    QString tmp = *gIt;
    if( tmp.startsWith( "default(" )) {
        int pos = tmp.findRev( ')' );
        if( pos >= 0 ) // this should be really done with regexp
            tmp = tmp.mid( 8, pos - 8 );
    }
    keyCode = new int;
    *keyCode = KKey::stringToKeyQt( tmp );
    dict->insert( gIt.key(), keyCode);
  }
}

void KKeyChooser::readGlobalKeys()
{
  //debug("KKeyChooser::readGlobalKeys()");
  readKeysInternal( d->globalDict, QString::fromLatin1("Global Keys"));

// insert all global keys, even if they appear in dictionary to be configured
}

void KKeyChooser::readStdKeys()
{
  // debug("KKeyChooser::readStdKeys()");

  readKeysInternal( d->stdDict, QString::fromLatin1("Keys"));
  // Only insert std keys which don't appear in the dictionary to be configured
  for (KKeyEntryMap::ConstIterator it = d->map->begin();
       it != d->map->end(); ++it)
    if ( d->stdDict->find( it.key() ) )
      d->stdDict->remove( it.key() );
}

void KKeyChooser::toChange( QListViewItem *item )
{
    // Hack: Do this incase we still have changeKey() running.
    //  Better would be to capture the mouse pointer so that we can't click
    //   around while we're supposed to be entering a key.
    //  Better yet would be a modal dialog for changeKey()!
    d->bKeyIntercept = false;
    releaseKeyboard();

    if ( !item || !d->actionMap.contains( item ) )
    {
        // if nothing is selected -> disable radio boxes
        d->kbGroup->find(NoKey)->setEnabled( false );
        d->kbGroup->find(DefaultKey)->setEnabled( false );
        d->kbGroup->find(CustomKey)->setEnabled( false );
	d->bChange->setEnabled( false );
	d->bChange->setKey( 0 );
    } else {
        /* get the entry */
        KKeyEntryMap::Iterator it = d->actionMap[item];
	int keyDefault = /*d->bPreferFourModifierKeys ? (*it).aDefaultKeyCode4 :*/ (*it).aDefaultKeyCode;

	// Set key strings
	QString keyStrCfg = KKey::keyToString( (*it).aConfigKeyCode, true );
	QString keyStrDef = KKey::keyToString( keyDefault, true );

        d->bChange->setKey( (*it).aConfigKeyCode );
        item->setText( 1, keyStrCfg );
	d->lInfo->setText( i18n("Default Key") + QString(": %1").arg(keyStrDef.isEmpty() ? i18n("None") : keyStrDef) );

	// Select the appropriate radio button.
	int index = ((*it).aConfigKeyCode == 0) ? NoKey
			: ((*it).aConfigKeyCode == keyDefault) ? DefaultKey
			: CustomKey;
        ((QRadioButton *)d->kbGroup->find(NoKey))->setChecked( index == NoKey );
        ((QRadioButton *)d->kbGroup->find(DefaultKey))->setChecked( index == DefaultKey );
        ((QRadioButton *)d->kbGroup->find(CustomKey))->setChecked( index == CustomKey );

	// Enable buttons if this key is configurable.
	// The 'Default Key' button must also have a default key.
	((QRadioButton *)d->kbGroup->find(NoKey))->setEnabled( (*it).bConfigurable );
	((QRadioButton *)d->kbGroup->find(DefaultKey))->setEnabled( (*it).bConfigurable && keyDefault != 0 );
	((QRadioButton *)d->kbGroup->find(CustomKey))->setEnabled( (*it).bConfigurable );
	d->bChange->setEnabled( (*it).bConfigurable );
    }
}

void KKeyChooser::fontChange( const QFont & )
{
        d->fCArea->setMinimumHeight( 4*d->bChange->sizeHint().height() );

        int widget_width = 0;

        setMinimumWidth( 20+5*(widget_width+10) );
}

void KKeyChooser::keyMode( int m )
{
        switch( m ) {
                case NoKey:
                        noKey();
                        break;
                case DefaultKey:
                        defaultKey();
                        break;
                case CustomKey:
                        d->bChange->captureKey();
                        break;
        }
}

void KKeyChooser::noKey()
{
    // return if no key is selected
    QListViewItem *item = d->wList->currentItem();
    if (!item)
        return;

    //kdDebug(125) << "no Key" << d->wList->currentItem()->text(0) << endl;
    if( d->actionMap.contains( d->wList->currentItem() ) )
        (*d->actionMap[d->wList->currentItem()]).aConfigKeyCode = 0;

    /* update the list and the change area */

    toChange(d->wList->currentItem());
    emit keyChange();
}

void KKeyChooser::defaultKey()
{
    // return if no key is selected
    QListViewItem *item = d->wList->currentItem();
    if (!item)
        return;

    /* update the list and the change area */
    if( d->actionMap.contains( item ) ) {
        (*d->actionMap[item]).aConfigKeyCode =
            /*(d->bPreferFourModifierKeys) ?
                (*d->actionMap[item]).aDefaultKeyCode4 :*/
                (*d->actionMap[item]).aDefaultKeyCode;

        item->setText( 1,
            KKey::keyToString((*d->actionMap[item]).aConfigKeyCode, true) );
    }

    toChange(d->wList->currentItem());
    emit keyChange();
}


void KKeyChooser::allDefault()
{
	allDefault( d->bPreferFourModifierKeys );
}

void KKeyChooser::allDefault( bool useFourModifierKeys )
{
    // Change all configKeyCodes to default values
    kdDebug(125) << QString( "allDefault( %1 )\n" ).arg( useFourModifierKeys );

    for( QMap<QListViewItem*, KKeyEntryMap::Iterator>::Iterator itit = d->actionMap.begin();
    	itit != d->actionMap.end(); ++itit ) {
	KKeyEntryMap::Iterator it = *itit;
        QListViewItem *at = itit.key();
	    //kdDebug(125) << QString( "allDefault: %1 3:%2 4:%3\n" ).arg(it.key()).arg((*it).aDefaultKeyCode).arg((*it).aDefaultKeyCode4);
            if ( (*it).bConfigurable ) {
                (*it).aCurrentKeyCode = (*it).aConfigKeyCode =
                    /*(useFourModifierKeys) ? (*it).aDefaultKeyCode4 :*/ (*it).aDefaultKeyCode;
            }
            at->setText(1, KKey::keyToString((*it).aConfigKeyCode, true));
    }
    emit keyChange();
}

void KKeyChooser::setPreferFourModifierKeys( bool bPreferFourModifierKeys )
{
	d->bPreferFourModifierKeys = bPreferFourModifierKeys;
}

// These should be removed during the next clean-up.
void KKeyChooser::shiftClicked()   { }
void KKeyChooser::ctrlClicked()   { }
void KKeyChooser::altClicked()   { }
void KKeyChooser::editKey()   { }
void KKeyChooser::editEnd()   { }
void KKeyChooser::changeKey() {}

void KKeyChooser::capturedKey( uint key )
{
	if( KKey::keyToString( key, true ).isEmpty() )
		d->lInfo->setText( i18n("Undefined key") );
	else
		setKey( key );
}

void KKeyChooser::listSync()
{
    QListViewItem *item = d->wList->firstChild();

    while (item) {
        if( d->actionMap.contains( item ) ) { // ellis -- for labels
            KKeyEntryMap::Iterator it = d->actionMap[item];

            if ( (*it).bConfigurable )
                item->setText(1, KKey::keyToString((*it).aConfigKeyCode, true));
        }

        item = item->itemBelow();
    }
    updateAction( d->wList->currentItem() );
    toChange( d->wList->currentItem() );
}

void KKeyChooser::setKey( int keyCode )
{
   QListViewItem *item = d->wList->currentItem();
   if (!item || !d->actionMap.contains( item ))
      return;

   KKeyEntryMap::Iterator it = d->actionMap[item];

   if( !d->bAllowMetaKey && (keyCode & (Qt::ALT<<1)) ) {
	QString s = i18n("The Meta key is not allowed in this context.");
	KMessageBox::sorry( this, s, i18n("Invalid Shortcut Key") );
	return;
   }

   if( keyCode < 0x1000  && !d->bAllowLetterShortcuts ) {
	QString s = i18n( 	"In order to use the '%1' key as a shortcut,\n"
				"it must be combined with the\n"
				"Meta, Alt, Ctrl, and/or Shift keys." ).arg(QChar(keyCode));
	KMessageBox::sorry( this, s, i18n("Invalid Shortcut Key") );
	return;
   }

   // If key isn't already in use,
   if( !isKeyPresent( keyCode ) ) {
      // Set new key code
      (*it).aConfigKeyCode = keyCode;
      // Update display
      toChange(item);
      emit keyChange();
   }
}

bool KKeyChooser::isKeyPresent( int kcode, bool warnuser )
{
    if (!kcode || !d->actionMap.contains( d->wList->currentItem() ))
        return false;

    // Search the global key codes to find if this keyCode is already used
    //  elsewhere

    QDictIterator<int> gIt( *d->globalDict );

    gIt.toFirst();
    while ( gIt.current() ) {
        if ( (*gIt.current()) == kcode && *gIt.current() != 0 ) {

            if (!warnuser)
                return true;

            QString actionName = gIt.currentKey();
            actionName.stripWhiteSpace();

            QString keyName = KKey::keyToString( *gIt.current(), true );

            QString str =
                i18n("The %1 key combination has already been "
                     "allocated\n"
                     "to the global %2 action.\n\n"
                     "Please choose a unique key combination.").
                arg(keyName).arg(actionName);

            KMessageBox::sorry( this, str, i18n("Global key conflict"));

            return true;
        }

        ++gIt;
    }

    // Search the std key codes to find if this keyCode is already used
    //  elsewhere

    QDictIterator<int> sIt( *d->stdDict );

    sIt.toFirst();
    while ( sIt.current() ) {
        kdDebug(125) << "current " << sIt.currentKey() << ":" << *sIt.current() << " code " << kcode << endl;
        if ( *sIt.current() == kcode && *sIt.current() != 0 ) {
            QString actionName( (*d->map)[sIt.currentKey()].descr );
            actionName.stripWhiteSpace();

            QString keyName = KKey::keyToString( *sIt.current(), true );

            QString str =
                i18n("The %1 key combination has already "
                     "been allocated\n"
                     "to the standard %2 action.\n"
                     "\n"
                     "Please choose a unique key combination.").
                arg(keyName).arg(actionName);

            KMessageBox::sorry( this, str, i18n("Standard key conflict"));

            return true;
        }
        ++sIt;
    }

    // Search the aConfigKeyCodes to find if this keyCode is already used
    // elsewhere
    for (KKeyEntryMap::ConstIterator it = d->map->begin();
         it != d->map->end(); ++it) {
        if ( it != d->actionMap[d->wList->currentItem()]
             && (*it).aConfigKeyCode == kcode ) {
            QString actionName( (*it).descr );
            actionName.stripWhiteSpace();

            QString keyName = KKey::keyToString( kcode, true );

            QString str =
                i18n("The %1 key combination has already "
                     "been allocated\n"
                     "to the %2 action.\n"
                     "\n"
                     "Please choose a unique key combination.").
                arg(keyName).arg(actionName);

            KMessageBox::sorry( this, str, i18n("Key conflict"));

            return true;
        }
    }

    return false;
}

QDict<int>* KKeyChooser::globalDict()
    {
    return d->globalDict;
    }

QDict<int>* KKeyChooser::stdDict()
    {
    return d->stdDict;
    }

#include "kkeydialog.moc"
#include "kkeybutton.moc"
