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

#include "kkeydialog.h"
#include "kkeybutton.h"

#include <qlayout.h>
#include <qpainter.h>
#include <qdrawutil.h>
#include <qradiobutton.h>
#include <qlabel.h>
#include <qbuttongroup.h>
#include <qwhatsthis.h>

#include <kaccel.h>
#include <kaction.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kglobalaccel.h>
#include <kkey_x11.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kxmlguifactory.h>

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
    KAccelActions globalDict, stdDict;
    //QDict<int> *globalDict;
    //QDict<int> *stdDict;
    KListView *pList;
    QLabel *lInfo;
    KKeyButton *bChange;
    QGroupBox *fCArea;
    QButtonGroup *kbGroup;
    KAccelActions *map;
    KAccelActions actionsNew;

    //QMap<KListViewItem*, KAccelActions::Iterator> actionMap;

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
KKeyDialog::KKeyDialog( KAccelActions& actions, QWidget *parent,
                        bool check_against_std_keys)
  : KDialogBase( parent, 0, true, i18n("Configure Key Bindings"),
                 Help|Default|Ok|Cancel, Ok )
{
  m_pKeyChooser = new KKeyChooser( actions, this, check_against_std_keys );
  setMainWidget( m_pKeyChooser );
  connect( this, SIGNAL(defaultClicked()), m_pKeyChooser, SLOT(allDefault()) );
  enableButton( Help, false );
}

KKeyDialog::~KKeyDialog()
{
}

void KKeyDialog::commitChanges()
{
	m_pKeyChooser->commitChanges();
}

int KKeyDialog::configureKeys( KAccelBase *keys, bool save_settings, QWidget *parent )
{
    KAccelActions& actions = keys->actions();
    KKeyDialog kd( actions, parent );
    int retcode = kd.exec();

    if( retcode == Accepted ) {
    	kd.commitChanges();
        keys->updateConnections();
        if (save_settings)
            keys->writeSettings();
    }
    return retcode;
}

int KKeyDialog::configureKeys( KAccel *keys, bool save_settings, QWidget *parent )
{
	return configureKeys( keys->basePtr(), save_settings, parent );
}

int KKeyDialog::configureKeys( KGlobalAccel *keys, bool save_settings, QWidget *parent )
{
	return configureKeys( keys->basePtr(), save_settings, parent );
}

int KKeyDialog::configureKeys( KActionCollection *coll, const QString& file,
                               bool save_settings, QWidget *parent )
{
    KAccelActions& map = coll->keyMap();

    KKeyDialog kd( map, parent );
    int retcode = kd.exec();

    if( retcode != Accepted )
        return retcode;

    if (!save_settings) {
    	kd.commitChanges();
	// FIXME: How do we get the new bindings to be connected?
	//  We need an equivalent to KAccel::updateConnections() for
	//  KActionCollection -- ellis
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
    //if (key.aCurrentKeyCode == key.aConfigKeyCode)
    //  continue;

    KAccelAction *pAccelAction = map.actionPtr(action->name());
    if( !pAccelAction )
      continue;
    KAccelShortcuts &cuts = pAccelAction->m_rgShortcuts;

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
    act_elem.setAttribute( attrAccel, cuts.toString( KKeySequence::I18N_No ) );

    elem.appendChild( act_elem );
  }

  // finally, write out the result
  KXMLGUIFactory::saveConfigFile(doc, file);

  //coll->setKeyMap( map );

  return retcode;
}

//************************************************************************
// KKeyChooser                                                           *
//************************************************************************
KKeyChooser::KKeyChooser( KAccelActions& actions, QWidget* parent,
			bool check_against_std_keys,
			bool bAllowLetterShortcuts,
			bool bAllowMetaKey )
: QWidget( parent )
{
	init( actions, check_against_std_keys, bAllowLetterShortcuts, bAllowMetaKey );
}

KKeyChooser::~KKeyChooser()
{
  delete d->pList;
  delete d;
  // Make sure that we don't still have global accelerators turned off.
  //KGlobalAccel::setKeyEventsEnabled( true );
}

void KKeyChooser::init( KAccelActions& actions,
			bool check_against_std_keys,
			bool bAllowLetterShortcuts,
			bool bAllowMetaKey )
{
  d = new KKeyChooserPrivate();

  d->map = &actions; // Keep pointer to original for saving
  d->actionsNew.init( actions ); // Make copy to modify
  d->bAllowMetaKey = bAllowMetaKey;
  d->bAllowLetterShortcuts = bAllowLetterShortcuts;
  d->bPreferFourModifierKeys = KKeySequence::useFourModifierKeys();

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
  d->pList = new KListView( this );
  d->pList->setAlternateBackground( QColor(0xb0, 0xb0, 0xff) );
  d->pList->setFocus();

  stackLayout->addMultiCellWidget( d->pList, 1, 1, 0, 1 );
  QString wtstr = i18n("Here you can see a list of key bindings, \n"
                       "i.e. associations between actions (e.g. 'Copy')\n"
                       "shown in the left column and keys or combination\n"
                       "of keys (e.g. CTRL-V) shown in the right column.");

  QWhatsThis::add( d->pList, wtstr );

  d->pList->addColumn(i18n("Action"));
  d->pList->addColumn(i18n("Current Key"));

  buildListView();

  connect( d->pList, SIGNAL( currentChanged( QListViewItem * ) ),
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
  connect( d->bChange, SIGNAL(capturedKey(KAccelShortcut)), SLOT(capturedKey(KAccelShortcut)) );
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

  //d->globalDict = new QDict<int> ( 100, false );
  //d->globalDict->setAutoDelete( true );
  readGlobalKeys();
  //d->stdDict = new QDict<int> ( 100, false );
  //d->stdDict->setAutoDelete( true );
  if (check_against_std_keys)
    readStdKeys();
}

//
// Add all "keys" to the list
//
void KKeyChooser::buildListView()
{
	d->pList->clear();
	d->pList->setSorting( -1 );
	// HACK to avoid alphabetic ording.  I'll re-write this in the
	//  next development phase where API changes are not so sensitive. -- ellis
	KListViewItem *pProgramItem, *pGroupItem = 0, *pParentItem, *pItem;

	pParentItem = pProgramItem = pItem = new KListViewItem( d->pList, "Shortcuts" );
	pParentItem->setExpandable( true );
	pParentItem->setOpen( true );
	pParentItem->setSelectable( false );
	for( KAccelActions::iterator it = d->actionsNew.begin(); it != d->actionsNew.end(); ++it ) {
		KAccelAction& action = *it;
		kdDebug(125) << "Key: " << action.m_sName << endl;
		if( action.m_sName.startsWith( "Program:" ) ) {
			pItem = new KListViewItem( d->pList, pProgramItem, action.m_sDesc );
			pItem->setSelectable( false );
			pItem->setExpandable( true );
			pItem->setOpen( true );
			if( !pProgramItem->firstChild() )
				delete pProgramItem;
			pProgramItem = pParentItem = pItem;
		} else if( action.m_sName.startsWith( "Group:" ) ) {
			pItem = new KListViewItem( pProgramItem, pParentItem, action.m_sDesc );
			pItem->setSelectable( false );
			pItem->setExpandable( true );
			pItem->setOpen( true );
			if( pGroupItem && !pGroupItem->firstChild() )
				delete pGroupItem;
			pGroupItem = pParentItem = pItem;
		} else {
			pItem = new KKeyChooserItem( pParentItem, pItem, action );
		}
	}
	if( !pProgramItem->firstChild() )
		delete pProgramItem;
	if( pGroupItem && !pGroupItem->firstChild() )
		delete pGroupItem;
}

void KKeyChooser::commitChanges()
{
	kdDebug(125) << "KKeyChooser::commitChanges()" << endl;
	d->map->updateShortcuts( d->actionsNew );
}

void KKeyChooser::updateAction( QListViewItem *item )
{
	toChange( item );
}

/*
void KKeyChooser::readKeysInternal( QMap<KKeySequence, QString>& map, const QString& group )
{
	map.clear();

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
	*keyCode = KKeySequence::stringToKeyQt( tmp );
	dict->insert( gIt.key(), keyCode);
	}
}*/

void KKeyChooser::readGlobalKeys()
{
	// insert all global keys, even if they appear in dictionary to be configured
	//debug("KKeyChooser::readGlobalKeys()");
	//readKeysInternal( d->globalDict, QString::fromLatin1("Global Keys"));
	KConfig config;
	d->globalDict.init( config, QString::fromLatin1("Global Keys") );
}

void KKeyChooser::readStdKeys()
{
	// debug("KKeyChooser::readStdKeys()");
	//readKeysInternal( d->stdDict, QString::fromLatin1("Keys"));
	KConfig config;
	d->stdDict.init( config, QString::fromLatin1("Keys") );
	// Only insert std keys which don't appear in the dictionary to be configured
//	for( KAccelActions::ConstIterator it = d->map->begin(); it != d->map->end(); ++it )
//		if ( d->stdDict->find( it.key() ) )
//			d->stdDict->remove( it.key() );
}

void KKeyChooser::toChange( QListViewItem *item )
{
    // Hack: Do this incase we still have changeKey() running.
    //  Better would be to capture the mouse pointer so that we can't click
    //   around while we're supposed to be entering a key.
    //  Better yet would be a modal dialog for changeKey()!
    releaseKeyboard();
    KKeyChooserItem* pInfo = dynamic_cast<KKeyChooserItem*>( item );

    if ( !pInfo ) {
        // if nothing is selected -> disable radio boxes
        d->kbGroup->find(NoKey)->setEnabled( false );
        d->kbGroup->find(DefaultKey)->setEnabled( false );
        d->kbGroup->find(CustomKey)->setEnabled( false );
	d->bChange->setEnabled( false );
	d->bChange->setKey( KAccelShortcut() );
    } else {
        /* get the entry */
	KAccelAction& action = pInfo->action();
	KAccelShortcut cutCur = action.getShortcut( 0 );
	KAccelShortcut cutDef;
	if( action.shortcutDefaults().size() > 0 )
		cutDef = action.shortcutDefaults()[0];

	// Set key strings
	QString keyStrCfg = cutCur.toString();
	QString keyStrDef = cutDef.toString();

        d->bChange->setKey( cutCur );
        //item->setText( 1, keyStrCfg );
	item->repaint();
	d->lInfo->setText( i18n("Default Key") + QString(": %1").arg(keyStrDef.isEmpty() ? i18n("None") : keyStrDef) );

	// Select the appropriate radio button.
	int index = (cutCur.count() == 0) ? NoKey
			: (cutCur == cutDef) ? DefaultKey
			: CustomKey;
        ((QRadioButton *)d->kbGroup->find(NoKey))->setChecked( index == NoKey );
        ((QRadioButton *)d->kbGroup->find(DefaultKey))->setChecked( index == DefaultKey );
        ((QRadioButton *)d->kbGroup->find(CustomKey))->setChecked( index == CustomKey );

	// Enable buttons if this key is configurable.
	// The 'Default Key' button must also have a default key.
	((QRadioButton *)d->kbGroup->find(NoKey))->setEnabled( action.m_bConfigurable );
	((QRadioButton *)d->kbGroup->find(DefaultKey))->setEnabled( action.m_bConfigurable && cutDef.count() != 0 );
	((QRadioButton *)d->kbGroup->find(CustomKey))->setEnabled( action.m_bConfigurable );
	d->bChange->setEnabled( action.m_bConfigurable );
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
	 case NoKey:      noKey(); break;
	 case DefaultKey: defaultKey(); break;
	 case CustomKey:  d->bChange->captureKey( true ); break;
	}
}

void KKeyChooser::noKey()
{
	// return if no key is selected
	KKeyChooserItem* pItem = dynamic_cast<KKeyChooserItem*>( d->pList->currentItem() );
	if( pItem ) {
		//kdDebug(125) << "no Key" << d->pList->currentItem()->text(0) << endl;
		pItem->action().setShortcut( 0, KAccelShortcut() );
		toChange( d->pList->currentItem() );
		//emit keyChange();
	}
}

void KKeyChooser::defaultKey()
{
	// return if no key is selected
	KKeyChooserItem* pItem = dynamic_cast<KKeyChooserItem*>( d->pList->currentItem() );
	if( pItem ) {
		//kdDebug(125) << "no Key" << d->pList->currentItem()->text(0) << endl;
		KAccelShortcuts cuts = pItem->action().shortcutDefaults();
		if( cuts.size() > 0 )
			pItem->action().setShortcut( 0, cuts[0] );
		else
			pItem->action().setShortcut( 0, KAccelShortcut() );
		toChange( d->pList->currentItem() );
		//emit keyChange();
	}
}

void KKeyChooser::allDefault()
{
	allDefault( d->bPreferFourModifierKeys );
}

void KKeyChooser::allDefault( bool useFourModifierKeys )
{
	// Change all configKeyCodes to default values
	kdDebug(125) << QString( "allDefault( %1 )\n" ).arg( useFourModifierKeys );

	for( KAccelActions::iterator it = d->actionsNew.begin(); it != d->actionsNew.end(); ++it ) {
		KAccelAction& action = *it;
		action.m_rgShortcuts = action.shortcutDefaults();
	}

	/*
	for( QMap<KListViewItem*, KAccelActions::Iterator>::Iterator itit = d->mapItemToInfo.begin();
		itit != d->mapItemToInfo.end(); ++itit ) {
		KAccelActions::Iterator it = *itit;
		KListViewItem *at = itit.key();
		//kdDebug(125) << QString( "allDefault: %1 3:%2 4:%3\n" ).arg(it.key()).arg((*it).aDefaultKeyCode).arg((*it).aDefaultKeyCode4);
		if ( (*it).bConfigurable ) {
			(*it).aCurrentKeyCode = (*it).aConfigKeyCode =
			//(useFourModifierKeys) ? (*it).aDefaultKeyCode4 :
			 (*it).aDefaultKeyCode;
		}
		at->setText(1, KKeySequence::keyToString((*it).aConfigKeyCode, true));
	}
	//emit keyChange();
	*/

	update();
}

void KKeyChooser::setPreferFourModifierKeys( bool bPreferFourModifierKeys )
{
	d->bPreferFourModifierKeys = bPreferFourModifierKeys;
}

void KKeyChooser::capturedKey( KAccelShortcut cut )
{
	if( cut.count() == 0 )
		d->lInfo->setText( i18n("Undefined key") );
	else
		setKey( cut );
}

void KKeyChooser::listSync()
{
	kdDebug(125) << "KKeyChooser::listSync() -- d->map = " << d->map << endl;
	if( d->map ) {
		d->actionsNew.updateShortcuts( *d->map );
		update();
		toChange( d->pList->currentItem() );
	}
}

//#include <iostream.h>
void KKeyChooser::setKey( KAccelShortcut cut )
{
	kdDebug(125) << "KKeyChooser::setKey( " << cut.toString() << " )" << endl;
	KKeyChooserItem* pItem = dynamic_cast<KKeyChooserItem*>(d->pList->currentItem());
	if( !pItem )
		return;

	// If key isn't already in use,
	if( !isKeyPresent( cut ) ) {
		// Set new key code
		pItem->action().setShortcut( 0, cut );
		// Update display
		toChange( pItem );
		//emit keyChange();
	}
/*   KListViewItem *item = d->pList->currentItem();
   if (!item || !d->mapItemToInfo.contains( item ))
      return;

   KAccelActions::Iterator it = d->mapItemToInfo[item];

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
      //emit keyChange();
   }
*/
}

void KKeyChooser::_warning( KAccelShortcut cut, QString sAction, QString sTitle )
{
	sAction = sAction.stripWhiteSpace();

	QString s =
		i18n("The '%1' key combination has already been allocated\n"
		"to the global \"%2\" action.\n\n"
		"Please choose a unique key combination.").
		arg(cut.toString()).arg(sAction);

	KMessageBox::sorry( this, s, sTitle );
}

bool KKeyChooser::isKeyPresent( KAccelShortcut cut, bool bWarnUser )
{
	if( cut.count() == 0 )
		return false;

	KAccelAction* pAction = d->actionsNew.actionPtr( cut );
	if( pAction ) {
		if( bWarnUser ) {
			QString s =
				i18n("The %1 key combination has already "
				"been allocated\n"
				"to the action: %2.\n"
				"\n"
				"Please choose a unique key combination.").
				arg(cut.toString()).arg(pAction->m_sDesc);
			KMessageBox::sorry( this, s, i18n("Key Conflict") );
		}
		return true;
	}
	return false;
/*
	// Search the global key codes to find if this keyCode is already used
	//  elsewhere

	QDictIterator<int> gIt( *d->globalDict );
	for( gIt.toFirst(); gIt.current(); ++gIt ) {
		if( (*gIt.current()) == kcode && *gIt.current() != 0 ) {
			if( warnuser )
				_warning( *gIt.current(), gIt.currentKey(), i18n("Global key conflict") );
			return true;
		}
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

            QString keyName = KKeySequence::keyToString( *sIt.current(), true );

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
    for (KAccelActions::ConstIterator it = d->map->begin();
         it != d->map->end(); ++it) {
        if ( it != d->mapItemToInfo[d->pList->currentItem()]
             && (*it).aConfigKeyCode == kcode ) {
            QString actionName( (*it).descr );
            actionName.stripWhiteSpace();

            QString keyName = KKeySequence::keyToString( kcode, true );

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
*/
    return false;
}

//---------------------------------------------------
KKeyChooserItem::KKeyChooserItem( KListView* parent, QListViewItem* after, KAccelAction& action )
:	KListViewItem( parent, after )
{
	m_pAction = &action;
}

KKeyChooserItem::KKeyChooserItem( QListViewItem* parent, QListViewItem* after, KAccelAction& action )
:	KListViewItem( parent, after )
{
	m_pAction = &action;
}

QString KKeyChooserItem::text( int iCol ) const
{
	KAccelAction& action = *m_pAction;

	if( iCol == 0 )
		return action.m_sDesc;
	else if( iCol <= (int) action.shortcutCount() )
		return action.m_rgShortcuts[iCol-1].toString();
	else
		return QString::null;
}

#include "kkeydialog.moc"
