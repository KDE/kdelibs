/* This file is part of the KDE libraries
    Copyright (C) 1998 Mark Donohoe <donohoe@kde.org>
    Copyright (C) 1997 Nicolas Hadacek <hadacek@via.ecp.fr>
    Copyright (C) 1998 Matthias Ettrich <ettrich@kde.org>
    Copyright (C) 2001 Ellis Whitehead <ellis@kde.org>

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
#include <kaccelaction.h>
#include <kaction.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kglobalaccel.h>
#include <kkey_x11.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kshortcut.h>
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

static KAccelActions *g_pactionsGlobal, *g_pactionsApplication;

class KKeyChooserPrivate {
public:
	KListView *pList;
	QLabel *lInfo;
	KKeyButton *bChange;
	QGroupBox *fCArea;
	QButtonGroup *kbGroup;
	KAccelActions *pActionsOrig;
	KAccelActions actionsNew;

	bool bAllowWinKey;
	// If this is set, then shortcuts require a modifier:
	//  so 'A' would not be valid, whereas 'Ctrl+A' would be.
	// Note, however, that this only applies to printable characters.
	//  'F1', 'Insert', etc., could still be used.
	bool bAllowLetterShortcuts;
	// When set, pressing the 'Default' button will select the aDefaultKeycode4,
	//  otherwise aDefaultKeycode.
	bool bPreferFourModifierKeys;
	};

// HACK: for getting around some of Qt's lack of Win support
enum { QT_META_MOD = Qt::ALT << 1 };	// Supply Meta bit where Qt left it out.

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
KKeyDialog::KKeyDialog( KAccelActions& actions, QWidget *parent, KKeyChooser::ActionType type )
: KDialogBase( parent, 0, true, i18n("Configure Shortcuts"), Help|Default|Ok|Cancel, Ok )
{
	m_pKeyChooser = new KKeyChooser( actions, this, type );
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

int KKeyDialog::configure( KAccelActions& actions, QWidget* parent, KKeyChooser::ActionType type )
{
	KKeyDialog kd( actions, parent, type );
	int retcode = kd.exec();
	if( retcode == Accepted )
		kd.commitChanges();
	return retcode;
}

int KKeyDialog::configure( KAccelActions& actions, const QString& sXmlFile, QWidget* parent, bool bSaveSettings )
{
	kdDebug(125) << "KKeyDialog::configureKeys( KaccelActions&, " << sXmlFile << ", " << bSaveSettings << " )" << endl;

	int retcode = configure( actions, parent, KKeyChooser::Application );
	if( retcode != Accepted || !bSaveSettings || sXmlFile.isEmpty() )
		return retcode;

	// let's start saving this info
	QString raw_xml( KXMLGUIFactory::readConfigFile( sXmlFile ) );
	QDomDocument doc;
	doc.setContent( raw_xml );

	QString tagActionProp = QString::fromLatin1("ActionProperties");
	QString tagAction     = QString::fromLatin1("Action");
	QString attrName      = QString::fromLatin1("name");
	QString attrShortcut  = QString::fromLatin1("shortcut");

	// first, lets see if we have existing properties
	QDomElement elem;
	QDomElement it = doc.documentElement();
	KXMLGUIFactory::removeDOMComments( it );
	it = it.firstChild().toElement();
	for( ; !it.isNull(); it = it.nextSibling().toElement() ) {
		if( it.tagName() == tagActionProp ) {
			elem = it;
			break;
		}
	}

	// if there was none, create one
	if( elem.isNull() )
	{
		elem = doc.createElement( tagActionProp );
		doc.firstChild().appendChild(elem);
	}

	// now, iterate through our actions
	for( uint i = 0; i < actions.count(); i++ )
	{
		KAccelAction* pAction = actions.actionPtr( i );

		// now see if this element already exists
		QDomElement act_elem;
		for( it = elem.firstChild().toElement(); !it.isNull(); it = it.nextSibling().toElement() ) {
			if( it.attribute( attrName ) == pAction->name() ) {
				act_elem = it;
				break;
			}
		}

		// nope, create a new one
		if( act_elem.isNull() ) {
			act_elem = doc.createElement( tagAction );
			act_elem.setAttribute( attrName, pAction->name() );
		}
		act_elem.setAttribute( attrShortcut, pAction->shortcut().toStringInternal() );

		elem.appendChild( act_elem );
	}

	kdDebug(125) << "calling KXMLGUIFactory::saveConfigFile()" << endl;
	// finally, write out the result
	KXMLGUIFactory::saveConfigFile( doc, sXmlFile );

	return retcode;
}

int KKeyDialog::configure( KAccel* keys, QWidget *parent, bool bSaveSettings )
{
	int retcode = configure( keys->actions(), parent, KKeyChooser::Application );
	if( retcode == Accepted ) {
		keys->updateConnections();
		if( bSaveSettings )
			keys->writeSettings();
	}
	return retcode;
}

int KKeyDialog::configure( KGlobalAccel* keys, QWidget *parent, bool bSaveSettings )
{
	int retcode = configure( keys->actions(), parent, KKeyChooser::Global );
	if( retcode == Accepted ) {
		keys->updateConnections();
		if( bSaveSettings )
			keys->writeSettings();
	}
	return retcode;
}

int KKeyDialog::configure( KActionCollection* coll, const QString& file, QWidget *parent, bool bSaveSettings )
{
	kdDebug(125) << "KKeyDialog::configureKeys( KActionCollection*, " << file << ", " << bSaveSettings << " )" << endl;
	KAccelActions actions;
	coll->createKeyMap( actions );

	int retcode = configure( actions, file, parent, bSaveSettings );
	if( retcode == Accepted )
		coll->setKeyMap( actions );

	return retcode;
}

int KKeyDialog::configure( KActionPtrList* coll, const QString& file, QWidget *parent, bool bSaveSettings )
{
	kdDebug(125) << "KKeyDialog::configureKeys( KActionCollection*, " << file << ", " << bSaveSettings << " )" << endl;
	KAccelActions actions;
	coll->createKeyMap( actions );

	int retcode = configure( actions, file, parent, bSaveSettings );
	if( retcode == Accepted )
		coll->setKeyMap( actions );

	return retcode;
}

//************************************************************************
// KKeyChooser                                                           *
//************************************************************************
KKeyChooser::KKeyChooser( KAccelActions& actions, QWidget* parent, ActionType type, bool bAllowLetterShortcuts )
: QWidget( parent )
{
	init( actions, type, bAllowLetterShortcuts );
}

KKeyChooser::KKeyChooser( KAccel* actions, QWidget* parent, bool bAllowLetterShortcuts )
: QWidget( parent )
{
	init( actions->actions(), Application, bAllowLetterShortcuts );
}

KKeyChooser::KKeyChooser( KGlobalAccel* actions, QWidget* parent )
: QWidget( parent )
{
	init( actions->actions(), Global, false );
}

KKeyChooser::KKeyChooser( KAccel* actions, QWidget* parent,
			bool bCheckAgainstStdKeys,
			bool bAllowLetterShortcuts,
			bool bAllowWinKey )
: QWidget( parent )
{
	ActionType type;
	if( bAllowWinKey )
		type = (bCheckAgainstStdKeys) ? ApplicationGlobal : Global;
	else
		type = Application;

	init( actions->actions(), type, bAllowLetterShortcuts );
}

KKeyChooser::KKeyChooser( KGlobalAccel* actions, QWidget* parent,
			bool bCheckAgainstStdKeys,
			bool bAllowLetterShortcuts,
			bool bAllowWinKey )
: QWidget( parent )
{
	ActionType type;
	if( bAllowWinKey )
		type = (bCheckAgainstStdKeys) ? ApplicationGlobal : Global;
	else
		type = Application;
	
	init( actions->actions(), type, bAllowLetterShortcuts );
}

KKeyChooser::~KKeyChooser()
{
	delete d->pList;
	delete d;
	// Make sure that we don't still have global accelerators turned off.
	//KGlobalAccel::setKeyEventsEnabled( true );
}

void KKeyChooser::commitChanges()
{
	kdDebug(125) << "KKeyChooser::commitChanges()" << endl;
	d->pActionsOrig->updateShortcuts( d->actionsNew );
}

void KKeyChooser::init( KAccelActions& actions, ActionType type, bool bAllowLetterShortcuts )
{
  d = new KKeyChooserPrivate();

  d->pActionsOrig = &actions; // Keep pointer to original for saving
  d->actionsNew.init( actions ); // Make copy to modify
  d->bAllowWinKey = (type == Global || type == ApplicationGlobal);
  d->bAllowLetterShortcuts = bAllowLetterShortcuts;
  d->bPreferFourModifierKeys = KAccelAction::useFourModifierKeys();

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
  // fill up the split list box with the action/key pairs.
  //
  d->pList = new KListView( this );
  d->pList->setFocus();

  stackLayout->addMultiCellWidget( d->pList, 1, 1, 0, 1 );
  QString wtstr = i18n("Here you can see a list of key bindings, "
                       "i.e. associations between actions (e.g. 'Copy') "
                       "shown in the left column and keys or combination "
                       "of keys (e.g. Ctrl+V) shown in the right column.");

  QWhatsThis::add( d->pList, wtstr );

  d->pList->setAllColumnsShowFocus( true );
  d->pList->addColumn(i18n("Action"));
  d->pList->addColumn(i18n("Shortcut"));
  d->pList->addColumn(i18n("Alternate"));

  buildListView();

  connect( d->pList, SIGNAL(currentChanged(QListViewItem*)),
           SLOT(slotListItemSelected(QListViewItem*)) );

  //
  // CREATE CHOOSE KEY GROUP
  //
  d->fCArea = new QGroupBox( this );
  topLayout->addWidget( d->fCArea, 1 );

  d->fCArea->setTitle( i18n("Shortcut for Selected Action") );
  d->fCArea->setFrameStyle( QFrame::Box | QFrame::Sunken );

  //
  // CHOOSE KEY GROUP LAYOUT MANAGER
  //
  QGridLayout *grid = new QGridLayout( d->fCArea, 3, 4, KDialog::spacingHint() );
  grid->addRowSpacing( 0, fontMetrics().lineSpacing() );

  d->kbGroup = new QButtonGroup( d->fCArea );
  d->kbGroup->hide();
  d->kbGroup->setExclusive( true );

  m_prbNone = new QRadioButton( i18n("&None"), d->fCArea );
  d->kbGroup->insert( m_prbNone, NoKey );
  m_prbNone->setEnabled( false );
  //grid->addMultiCellWidget( rb, 1, 1, 1, 2 );
  grid->addWidget( m_prbNone, 1, 0 );
  QWhatsThis::add( m_prbNone, i18n("The selected action will not be associated with any key.") );
  connect( m_prbNone, SIGNAL(clicked()), SLOT(slotNoKey()) );

  m_prbDef = new QRadioButton( i18n("De&fault"), d->fCArea );
  d->kbGroup->insert( m_prbDef, DefaultKey );
  m_prbDef->setEnabled( false );
  //grid->addMultiCellWidget( rb, 2, 2, 1, 2 );
  grid->addWidget( m_prbDef, 1, 1 );
  QWhatsThis::add( m_prbDef, i18n("This will bind the default key to the selected action. Usually a reasonable choice.") );
  connect( m_prbDef, SIGNAL(clicked()), SLOT(slotDefaultKey()) );

  m_prbCustom = new QRadioButton( i18n("&Custom"), d->fCArea );
  d->kbGroup->insert( m_prbCustom, CustomKey );
  m_prbCustom->setEnabled( false );
  //grid->addMultiCellWidget( rb, 3, 3, 1, 2 );
  grid->addWidget( m_prbCustom, 1, 2 );
  QWhatsThis::add( m_prbCustom, i18n("If this option is selected you can create a customized key binding for the"
    " selected action using the buttons below.") );
  connect( m_prbCustom, SIGNAL(clicked()), SLOT(slotCustomKey()) );

  //connect( d->kbGroup, SIGNAL( clicked( int ) ), SLOT( keyMode( int ) ) );

  QBoxLayout *pushLayout = new QHBoxLayout( KDialog::spacingHint() );
  grid->addLayout( pushLayout, 1, 3 );

  d->bChange = new KKeyButton(d->fCArea, "key");
  d->bChange->setEnabled( false );
  connect( d->bChange, SIGNAL(capturedShortcut(const KShortcut&)), SLOT(capturedShortcut(const KShortcut&)) );
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
  if (type == Application || type == ApplicationGlobal)
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
	for( uint i = 0; i < d->actionsNew.count(); i++ ) {
		KAccelAction& action = *d->actionsNew.actionPtr( i );
		kdDebug(125) << "Key: " << action.name() << endl;
		if( action.name().startsWith( "Program:" ) ) {
			pItem = new KListViewItem( d->pList, pProgramItem, action.desc() );
			pItem->setSelectable( false );
			pItem->setExpandable( true );
			pItem->setOpen( true );
			if( !pProgramItem->firstChild() )
				delete pProgramItem;
			pProgramItem = pParentItem = pItem;
		} else if( action.name().startsWith( "Group:" ) ) {
			pItem = new KListViewItem( pProgramItem, pParentItem, action.desc() );
			pItem->setSelectable( false );
			pItem->setExpandable( true );
			pItem->setOpen( true );
			if( pGroupItem && !pGroupItem->firstChild() )
				delete pGroupItem;
			pGroupItem = pParentItem = pItem;
		} else if( !action.name().isEmpty() )
			pItem = new KKeyChooserItem( pParentItem, pItem, action );
	}
	if( !pProgramItem->firstChild() )
		delete pProgramItem;
	if( pGroupItem && !pGroupItem->firstChild() )
		delete pGroupItem;
}

void KKeyChooser::updateButtons()
{
	// Hack: Do this incase we still have changeKey() running.
	//  Better would be to capture the mouse pointer so that we can't click
	//   around while we're supposed to be entering a key.
	//  Better yet would be a modal dialog for changeKey()!
	releaseKeyboard();
	KKeyChooserItem* pItem = dynamic_cast<KKeyChooserItem*>( d->pList->currentItem() );

	if ( !pItem ) {
		// if nothing is selected -> disable radio boxes
		m_prbNone->setEnabled( false );
		m_prbDef->setEnabled( false );
		m_prbCustom->setEnabled( false );
		d->bChange->setEnabled( false );
		d->bChange->setShortcut( KShortcut() );
	} else {
		KAccelAction& action = pItem->action();

		// Set key strings
		QString keyStrCfg = action.shortcut().toString();
		QString keyStrDef = action.shortcutDefault().toString();

		d->bChange->setShortcut( action.shortcut() );
		//item->setText( 1, keyStrCfg );
		pItem->repaint();
		d->lInfo->setText( i18n("Default Key") + QString(": %1").arg(keyStrDef.isEmpty() ? i18n("None") : keyStrDef) );

		// Select the appropriate radio button.
		int index = (action.shortcut().count() == 0) ? NoKey
				: (action.shortcut() == action.shortcutDefault()) ? DefaultKey
				: CustomKey;
		m_prbNone->setChecked( index == NoKey );
		m_prbDef->setChecked( index == DefaultKey );
		m_prbCustom->setChecked( index == CustomKey );

		// Enable buttons if this key is configurable.
		// The 'Default Key' button must also have a default key.
		m_prbNone->setEnabled( action.isConfigurable() );
		m_prbDef->setEnabled( action.isConfigurable() && action.shortcutDefault().count() != 0 );
		m_prbCustom->setEnabled( action.isConfigurable() );
		d->bChange->setEnabled( action.isConfigurable() );
	}
}

void KKeyChooser::slotNoKey()
{
	// return if no key is selected
	KKeyChooserItem* pItem = dynamic_cast<KKeyChooserItem*>( d->pList->currentItem() );
	if( pItem ) {
		//kdDebug(125) << "no Key" << d->pList->currentItem()->text(0) << endl;
		pItem->action().setShortcut( KShortcut() );
		//updateButtons( d->pList->currentItem() );
		updateButtons();
		emit keyChange();
	}
}

void KKeyChooser::slotDefaultKey()
{
	// return if no key is selected
	KKeyChooserItem* pItem = dynamic_cast<KKeyChooserItem*>( d->pList->currentItem() );
	if( pItem ) {
		pItem->action().setShortcut( pItem->action().shortcutDefault() );
		updateButtons();
		emit keyChange();
	}
}

void KKeyChooser::slotCustomKey()
{
	d->bChange->captureShortcut();
}

void KKeyChooser::allDefault()
{
	kdDebug(125) << "KKeyChooser::allDefault()" << endl;
	allDefault( d->bPreferFourModifierKeys );
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
	if( !g_pactionsGlobal )
		g_pactionsGlobal = new KAccelActions;

	// insert all global keys, even if they appear in dictionary to be configured
	//debug("KKeyChooser::readGlobalKeys()");
	//readKeysInternal( d->globalDict, QString::fromLatin1("Global Keys"));
	KConfig config;
	if( g_pactionsGlobal->count() == 0 )
		g_pactionsGlobal->init( config, QString::fromLatin1("Global Shortcuts") );
}

void KKeyChooser::readStdKeys()
{
	if( !g_pactionsApplication )
		g_pactionsApplication = new KAccelActions;

	// debug("KKeyChooser::readStdKeys()");
	//readKeysInternal( d->stdDict, QString::fromLatin1("Keys"));
	KConfig config;
	if( g_pactionsApplication->count() == 0 )
		g_pactionsApplication->init( config, QString::fromLatin1("Shortcuts") );
	// Only insert std keys which don't appear in the dictionary to be configured
//	for( KAccelActions::ConstIterator it = d->pActionsOrig->begin(); it != d->pActionsOrig->end(); ++it )
//		if ( d->stdDict->find( it.key() ) )
//			d->stdDict->remove( it.key() );
}

void KKeyChooser::fontChange( const QFont & )
{
        d->fCArea->setMinimumHeight( 4*d->bChange->sizeHint().height() );

        int widget_width = 0;

        setMinimumWidth( 20+5*(widget_width+10) );
}

void KKeyChooser::allDefault( bool useFourModifierKeys )
{
	// Change all configKeyCodes to default values
	kdDebug(125) << QString( "KKeyChooser::allDefault( %1 )\n" ).arg( useFourModifierKeys );

	for( uint i = 0; i < d->actionsNew.count(); i++ ) {
		KAccelAction& action = *d->actionsNew.actionPtr( i );
		action.setShortcut( action.shortcutDefault() );
	}

	emit keyChange();
	update();
	updateButtons();
}

void KKeyChooser::slotListItemSelected( QListViewItem* )
{
	updateButtons();
}

void KKeyChooser::setPreferFourModifierKeys( bool bPreferFourModifierKeys )
{
	d->bPreferFourModifierKeys = bPreferFourModifierKeys;
}

void KKeyChooser::capturedShortcut( const KShortcut& cut )
{
	if( cut.isNull() )
		d->lInfo->setText( i18n("Undefined key") );
	else
		setShortcut( cut );
}

void KKeyChooser::listSync()
{
	kdDebug(125) << "KKeyChooser::listSync() -- d->pActionsOrig = " << d->pActionsOrig << endl;
	if( d->pActionsOrig ) {
		d->actionsNew.updateShortcuts( *d->pActionsOrig );
		update();
		//updateButtons( d->pList->currentItem() );
		updateButtons();
	}
}

//#include <iostream.h>
void KKeyChooser::setShortcut( const KShortcut& cut )
{
	kdDebug(125) << "KKeyChooser::setShortcut( " << cut.toString() << " )" << endl;
	KKeyChooserItem* pItem = dynamic_cast<KKeyChooserItem*>(d->pList->currentItem());
	if( !pItem )
		return;

	// If key isn't already in use,
	if( !isKeyPresent( cut ) ) {
		// Set new key code
		pItem->action().setShortcut( cut );
		// Update display
		//updateButtons( pItem );
		updateButtons();
		emit keyChange();
	}
/*   KListViewItem *item = d->pList->currentItem();
   if (!item || !d->mapItemToInfo.contains( item ))
      return;

   KAccelActions::Iterator it = d->mapItemToInfo[item];

   if( !d->bAllowWinKey && (keyCode & (Qt::ALT<<1)) ) {
	QString s = i18n("The Win key is not allowed in this context.");
	KMessageBox::sorry( this, s, i18n("Invalid Shortcut Key") );
	return;
   }

   if( keyCode < 0x1000  && !d->bAllowLetterShortcuts ) {
	QString s = i18n( 	"In order to use the '%1' key as a shortcut, "
				"it must be combined with the "
				"Win, Alt, Ctrl, and/or Shift keys." ).arg(QChar(keyCode));
	KMessageBox::sorry( this, s, i18n("Invalid Shortcut Key") );
	return;
   }

   // If key isn't already in use,
   if( !isKeyPresent( keyCode ) ) {
      // Set new key code
      (*it).aConfigKeyCode = keyCode;
      // Update display
      updateButtons(item);
      emit keyChange();
   }
*/
}

bool KKeyChooser::isKeyPresent( const KShortcut& cut, bool bWarnUser )
{
	if( cut.count() == 0 )
		return false;

	// TODO: check for all sequences of a shortcut
	KKeyChooserItem* pItem = dynamic_cast<KKeyChooserItem*>(d->pList->currentItem());
	KAccelAction* pAction = d->actionsNew.actionPtr( cut.seq(0) );
	if( pAction && pAction != &pItem->action() ) {
		if( bWarnUser )
			_warning( cut, pAction->name(), i18n("Key Conflict") );
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
            QString actionName( (*d->pActionsOrig)[sIt.currentKey()].descr );
            actionName.stripWhiteSpace();

            QString keyName = KKeySequence::keyToString( *sIt.current(), true );

            QString str =
                i18n("The %1 key combination has already "
                     "been allocated "
                     "to the standard %2 action.\n"
                     "Please choose a unique key combination.").
                arg(keyName).arg(actionName);

            KMessageBox::sorry( this, str, i18n("Standard key conflict"));

            return true;
        }
        ++sIt;
    }

    // Search the aConfigKeyCodes to find if this keyCode is already used
    // elsewhere
    for (KAccelActions::ConstIterator it = d->pActionsOrig->begin();
         it != d->pActionsOrig->end(); ++it) {
        if ( it != d->mapItemToInfo[d->pList->currentItem()]
             && (*it).aConfigKeyCode == kcode ) {
            QString actionName( (*it).descr );
            actionName.stripWhiteSpace();

            QString keyName = KKeySequence::keyToString( kcode, true );

            QString str =
                i18n("The %1 key combination has already "
                     "been allocated to the %2 action.\n"
                     "Please choose a unique key combination.").
                arg(keyName).arg(actionName);

            KMessageBox::sorry( this, str, i18n("Key conflict"));

            return true;
        }
    }
*/
    return false;
}

void KKeyChooser::_warning( const KShortcut& cut, QString sAction, QString sTitle )
{
	sAction = sAction.stripWhiteSpace();

	QString s =
		i18n("The '%1' key combination has already been allocated "
		"to the \"%2\" action.\n"
		"Please choose a unique key combination.").
		arg(cut.toString()).arg(sAction);

	KMessageBox::sorry( this, s, sTitle );
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
		return action.desc();
	else if( iCol <= (int) action.sequenceCount()
		  && iCol <= (int) action.shortcut().count() )
		return action.seq(iCol-1).toString();
	else
		return QString::null;
}

#include "kkeydialog.moc"
