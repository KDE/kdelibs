/* This file is part of the KDE libraries
    Copyright (C) 1998 Mark Donohoe <donohoe@kde.org>
    Copyright (C) 1997 Nicolas Hadacek <hadacek@kde.org>
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

#include <string.h>

#include <qbuttongroup.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qdrawutil.h>
#include <qpainter.h>
#include <qradiobutton.h>
#include <qregexp.h>
#include <qwhatsthis.h>

#include <kaccel.h>
#include <kaction.h>
#include <kactionshortcutlist.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kglobalaccel.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kshortcut.h>
#include <kshortcutlist.h>
#include <kxmlguifactory.h>
#include <kaboutdata.h>
#include <kstaticdeleter.h>

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

//---------------------------------------------------------------------
// KKeyChooserItem
//---------------------------------------------------------------------

class KKeyChooserItem : public KListViewItem
{
 public:
	KKeyChooserItem( KListView* parent, QListViewItem* after, KShortcutList* pList, uint iAction );
	KKeyChooserItem( QListViewItem* parent, QListViewItem* after, KShortcutList* pList, uint iAction );

	QString actionName() const;
	const KShortcut& shortcut() const;
	bool isConfigurable() const
		{ return m_pList->isConfigurable( m_iAction ); }
	const KShortcut& shortcutDefault() const
		{ return m_pList->shortcutDefault( m_iAction ); }

	void setShortcut( const KShortcut& cut );
	void commitChanges();

	virtual QString text( int iCol ) const;
	virtual int compare( QListViewItem*, int iCol, bool bAscending ) const;

 protected:
	KShortcutList* m_pList;
	uint m_iAction;
	bool m_bModified;
	KShortcut m_cut;
};

//---------------------------------------------------------------------
// KKeyChooserPrivate
//---------------------------------------------------------------------

class KKeyChooserPrivate
{
 public:
	QValueList<KShortcutList*> rgpLists;
	QValueList<KShortcutList*> rgpListsAllocated;

	KListView *pList;
	QLabel *lInfo;
	KKeyButton *pbtnShortcut;
	QGroupBox *fCArea;
	QButtonGroup *kbGroup;

	QMap<QString, KShortcut> mapGlobals;

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

//---------------------------------------------------------------------
// KKeyChooser
//---------------------------------------------------------------------

KKeyChooser::KKeyChooser( QWidget* parent, ActionType type, bool bAllowLetterShortcuts )
: QWidget( parent )
{
	initGUI( type, bAllowLetterShortcuts );
}

KKeyChooser::KKeyChooser( KActionCollection* coll, QWidget* parent, bool bAllowLetterShortcuts )
: QWidget( parent )
{
	initGUI( Application, bAllowLetterShortcuts );
	insert( coll );
}

KKeyChooser::KKeyChooser( KAccel* pAccel, QWidget* parent, bool bAllowLetterShortcuts )
: QWidget( parent )
{
	initGUI( Application, bAllowLetterShortcuts );
	insert( pAccel );
}

KKeyChooser::KKeyChooser( KGlobalAccel* pAccel, QWidget* parent )
: QWidget( parent )
{
	initGUI( ApplicationGlobal, false );
	insert( pAccel );
}

KKeyChooser::KKeyChooser( KShortcutList* pList, QWidget* parent, ActionType type, bool bAllowLetterShortcuts )
: QWidget( parent )
{
	initGUI( type, bAllowLetterShortcuts );
	insert( pList );
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

	initGUI( type, bAllowLetterShortcuts );
	insert( actions );
}

KKeyChooser::KKeyChooser( KGlobalAccel* actions, QWidget* parent,
			bool bCheckAgainstStdKeys,
			bool bAllowLetterShortcuts,
			bool /*bAllowWinKey*/ )
: QWidget( parent )
{
	ActionType type = (bCheckAgainstStdKeys) ? ApplicationGlobal : Global;

	initGUI( type, bAllowLetterShortcuts );
	insert( actions );
}

// list of all existing KKeyChooser's of type Global
// used when checking global shortcut for a possible conflict
// (just checking against kdeglobals isn't enough, the shortcuts
// might have changed in KKeyChooser and not being saved yet)
static QValueList< KKeyChooser* >* globalChoosers = NULL;
static KStaticDeleter< QValueList< KKeyChooser* > > globalChoosersDeleter;

KKeyChooser::~KKeyChooser()
{
        if( m_type == Global && globalChoosers != NULL )
            globalChoosers->remove( this );
	// Delete allocated KShortcutLists
	for( uint i = 0; i < d->rgpListsAllocated.count(); i++ )
		delete d->rgpListsAllocated[i];
	delete d;
}

bool KKeyChooser::insert( KActionCollection *pColl)
{
    return insert(pColl, QString::null);
}

bool KKeyChooser::insert( KActionCollection* pColl, const QString &title )
{
    QString str = title;
    if ( title.isEmpty() && pColl->instance()
         && pColl->instance()->aboutData() )
        str = pColl->instance()->aboutData()->programName();

	KShortcutList* pList = new KActionShortcutList( pColl );
	d->rgpListsAllocated.append( pList );
	d->rgpLists.append( pList );
	buildListView(d->rgpLists.count() - 1, str);
	return true;
}

bool KKeyChooser::insert( KAccel* pAccel )
{
	KShortcutList* pList = new KAccelShortcutList( pAccel );
	d->rgpListsAllocated.append( pList );
	return insert( pList );
}

bool KKeyChooser::insert( KGlobalAccel* pAccel )
{
	KShortcutList* pList = new KAccelShortcutList( pAccel );
	d->rgpListsAllocated.append( pList );
	return insert( pList );
}

bool KKeyChooser::insert( KShortcutList* pList )
{
	d->rgpLists.append( pList );
	buildListView( d->rgpLists.count() - 1, QString::null );
	return true;
}

void KKeyChooser::commitChanges()
{
	kdDebug(125) << "KKeyChooser::commitChanges()" << endl;

	QListViewItemIterator it( d->pList );
	for( ; it.current(); ++it ) {
		KKeyChooserItem* pItem = dynamic_cast<KKeyChooserItem*>(it.current());
		if( pItem )
			pItem->commitChanges();
	}
}

void KKeyChooser::save()
{
	commitChanges();
	for( uint i = 0; i < d->rgpLists.count(); i++ )
		d->rgpLists[i]->save();
}

void KKeyChooser::initGUI( ActionType type, bool bAllowLetterShortcuts )
{
  d = new KKeyChooserPrivate();

  m_type = type;
  d->bAllowLetterShortcuts = bAllowLetterShortcuts;

  d->bAllowWinKey = (m_type == Global || m_type == ApplicationGlobal);
  d->bPreferFourModifierKeys = KGlobalAccel::useFourModifierKeys();

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

  m_prbNone = new QRadioButton( i18n("no key", "&None"), d->fCArea );
  d->kbGroup->insert( m_prbNone, NoKey );
  m_prbNone->setEnabled( false );
  //grid->addMultiCellWidget( rb, 1, 1, 1, 2 );
  grid->addWidget( m_prbNone, 1, 0 );
  QWhatsThis::add( m_prbNone, i18n("The selected action will not be associated with any key.") );
  connect( m_prbNone, SIGNAL(clicked()), SLOT(slotNoKey()) );

  m_prbDef = new QRadioButton( i18n("default key", "De&fault"), d->fCArea );
  d->kbGroup->insert( m_prbDef, DefaultKey );
  m_prbDef->setEnabled( false );
  //grid->addMultiCellWidget( rb, 2, 2, 1, 2 );
  grid->addWidget( m_prbDef, 1, 1 );
  QWhatsThis::add( m_prbDef, i18n("This will bind the default key to the selected action. Usually a reasonable choice.") );
  connect( m_prbDef, SIGNAL(clicked()), SLOT(slotDefaultKey()) );

  m_prbCustom = new QRadioButton( i18n("C&ustom"), d->fCArea );
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

  d->pbtnShortcut = new KKeyButton(d->fCArea, "key");
  d->pbtnShortcut->setEnabled( false );
  connect( d->pbtnShortcut, SIGNAL(capturedShortcut(const KShortcut&)), SLOT(capturedShortcut(const KShortcut&)) );
  grid->addRowSpacing( 1, d->pbtnShortcut->sizeHint().height() + 5 );

  wtstr = i18n("Use this button to choose a new shortcut key. Once you click it, "
  		"you can press the key-combination which you would like to be assigned "
		"to the currently selected action.");
  QWhatsThis::add( d->pbtnShortcut, wtstr );

  //
  // Add widgets to the geometry manager
  //
  pushLayout->addSpacing( KDialog::spacingHint()*2 );
  pushLayout->addWidget( d->pbtnShortcut );
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
  //if (type == Application || type == ApplicationGlobal)
  //  readStdKeys();
  connect( kapp, SIGNAL( settingsChanged( int )), SLOT( slotSettingsChanged( int )));
  if( m_type == Global ) {
      if( globalChoosers == NULL )
          globalChoosers = globalChoosersDeleter.setObject( new QValueList< KKeyChooser* > );
      globalChoosers->append( this );
  }
}

// Add all shortcuts to the list
void KKeyChooser::buildListView( uint iList, const QString &title )
{
	KShortcutList* pList = d->rgpLists[iList];

	//d->pList->setSorting( -1 );
	KListViewItem *pProgramItem, *pGroupItem = 0, *pParentItem, *pItem;

	QString str = (title.isEmpty() ? i18n("Shortcuts") : title);
	pParentItem = pProgramItem = pItem = new KListViewItem( d->pList, str );
	pParentItem->setExpandable( true );
	pParentItem->setOpen( true );
	pParentItem->setSelectable( false );
	uint nSize = pList->count();
	for( uint iAction = 0; iAction < nSize; iAction++ ) {
		QString sName = pList->name(iAction);
		kdDebug(125) << "Key: " << sName << endl;
		if( sName.startsWith( "Program:" ) ) {
			pItem = new KListViewItem( d->pList, pProgramItem, pList->label(iAction) );
			pItem->setSelectable( false );
			pItem->setExpandable( true );
			pItem->setOpen( true );
			if( !pProgramItem->firstChild() )
				delete pProgramItem;
			pProgramItem = pParentItem = pItem;
		} else if( sName.startsWith( "Group:" ) ) {
			pItem = new KListViewItem( pProgramItem, pParentItem, pList->label(iAction) );
			pItem->setSelectable( false );
			pItem->setExpandable( true );
			pItem->setOpen( true );
			if( pGroupItem && !pGroupItem->firstChild() )
				delete pGroupItem;
			pGroupItem = pParentItem = pItem;
		} else if( !sName.isEmpty() && pList->isConfigurable(iAction) )
			pItem = new KKeyChooserItem( pParentItem, pItem, pList, iAction );
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
		d->pbtnShortcut->setEnabled( false );
		d->pbtnShortcut->setShortcut( KShortcut() );
	} else {
		bool bConfigurable = pItem->isConfigurable();
		bool bQtShortcut = (m_type == Application || m_type == Standard);
		const KShortcut& cutDef = pItem->shortcutDefault();

		// Set key strings
		QString keyStrCfg = pItem->shortcut().toString();
		QString keyStrDef = cutDef.toString();

		d->pbtnShortcut->setShortcut( pItem->shortcut(), bQtShortcut );
		//item->setText( 1, keyStrCfg );
		pItem->repaint();
		d->lInfo->setText( i18n("Default key:") + QString(" %1").arg(keyStrDef.isEmpty() ? i18n("None") : keyStrDef) );

		// Select the appropriate radio button.
		int index = (pItem->shortcut().isNull()) ? NoKey
				: (pItem->shortcut() == cutDef) ? DefaultKey
				: CustomKey;
		m_prbNone->setChecked( index == NoKey );
		m_prbDef->setChecked( index == DefaultKey );
		m_prbCustom->setChecked( index == CustomKey );

		// Enable buttons if this key is configurable.
		// The 'Default Key' button must also have a default key.
		m_prbNone->setEnabled( bConfigurable );
		m_prbDef->setEnabled( bConfigurable && cutDef.count() != 0 );
		m_prbCustom->setEnabled( bConfigurable );
		d->pbtnShortcut->setEnabled( bConfigurable );
	}
}

void KKeyChooser::slotNoKey()
{
	// return if no key is selected
	KKeyChooserItem* pItem = dynamic_cast<KKeyChooserItem*>( d->pList->currentItem() );
	if( pItem ) {
		//kdDebug(125) << "no Key" << d->pList->currentItem()->text(0) << endl;
		pItem->setShortcut( KShortcut() );
		updateButtons();
		emit keyChange();
	}
}

void KKeyChooser::slotDefaultKey()
{
	// return if no key is selected
	KKeyChooserItem* pItem = dynamic_cast<KKeyChooserItem*>( d->pList->currentItem() );
	if( pItem ) {
		pItem->setShortcut( pItem->shortcutDefault() );
		updateButtons();
		emit keyChange();
	}
}

void KKeyChooser::slotCustomKey()
{
	d->pbtnShortcut->captureShortcut();
}

void KKeyChooser::readGlobalKeys()
{
        d->mapGlobals.clear();
        if( m_type == Global )
            return; // they will be checked normally, because we're configuring them
	QMap<QString, QString> mapEntry = KGlobal::config()->entryMap( "Global Shortcuts" );
	QMap<QString, QString>::Iterator it( mapEntry.begin() );
	for( uint i = 0; it != mapEntry.end(); ++it, i++ )
		d->mapGlobals[it.key()] = KShortcut(*it);
}

void KKeyChooser::slotSettingsChanged( int category )
{
    if( category == KApplication::SETTINGS_SHORTCUTS )
        readGlobalKeys(); // reread
}

void KKeyChooser::fontChange( const QFont & )
{
        d->fCArea->setMinimumHeight( 4*d->pbtnShortcut->sizeHint().height() );

        int widget_width = 0;

        setMinimumWidth( 20+5*(widget_width+10) );
}

void KKeyChooser::allDefault()
{
	kdDebug(125) << "KKeyChooser::allDefault()" << endl;

	QListViewItemIterator it( d->pList );
	for( ; it.current(); ++it ) {
		KKeyChooserItem* pItem = dynamic_cast<KKeyChooserItem*>(it.current());
		if( pItem )
			pItem->setShortcut( pItem->shortcutDefault() );
	}

	updateButtons();
	emit keyChange();
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
		slotNoKey();
	else
		setShortcut( cut );
}

// FIXME: give this functionality again -- I don't think it's ever used, though. -- ellis
// TODO: Check lxr.kde.org to see if it's used anywhere
void KKeyChooser::listSync()
{
/*	kdDebug(125) << "KKeyChooser::listSync()" << endl;

	if( d->pColl ) {
		// TODO: This is very inefficient.  Come up with something better.
		KAccelActions aa;
		d->pColl->createKeyMap( aa );
		d->actionsNew.updateShortcuts( aa );
	} else if( d->pActionsOrig ) {
		d->actionsNew.updateShortcuts( *d->pActionsOrig );
		update();
		updateButtons();
	}*/
}

void KKeyChooser::syncToConfig( const QString& sConfigGroup, KConfigBase* pConfig, bool bClearUnset )
{
	kdDebug(125) << "KKeyChooser::syncToConfig( \"" << sConfigGroup << "\", " << pConfig << " ) start" << endl;
	if( !pConfig )
		pConfig = KGlobal::config();
	KConfigGroupSaver cgs( pConfig, sConfigGroup );

	QListViewItemIterator it( d->pList );
	for( ; it.current(); ++it ) {
		KKeyChooserItem* pItem = dynamic_cast<KKeyChooserItem*>(it.current());
		if( pItem ) {
			QString sEntry = pConfig->readEntry( pItem->actionName() );
			if( !sEntry.isNull() || bClearUnset ) {
				if( sEntry == "none" )
					sEntry = QString::null;
				pItem->setShortcut( sEntry );
			}
			kdDebug(125) << pItem->actionName() << " = " << pItem->shortcut().toStringInternal() << endl;
		}
	}
	updateButtons();
	kdDebug(125) << "KKeyChooser::syncToConfig() done" << endl;
}

void KKeyChooser::setShortcut( const KShortcut& cut )
{
	kdDebug(125) << "KKeyChooser::setShortcut( " << cut.toString() << " )" << endl;
	KKeyChooserItem* pItem = dynamic_cast<KKeyChooserItem*>(d->pList->currentItem());
	if( !pItem )
		return;

	for( uint i = 0; i < cut.count(); i++ ) {
		const KKeySequence& seq = cut.seq(i);
		const KKey& key = seq.key(0);

		if( !d->bAllowWinKey && (key.modFlags() & KKey::WIN) ) {
			QString s = i18n("The Win key is not allowed in this context.");
			KMessageBox::sorry( this, s, i18n("Invalid Shortcut Key") );
			return;
		}
		if( !d->bAllowLetterShortcuts && key.modFlags() == 0
		    && key.sym() < 0x3000 && QChar(key.sym()).isLetterOrNumber() ) {
			QString s = i18n( 	"In order to use the '%1' key as a shortcut, "
						"it must be combined with the "
						"Win, Alt, Ctrl, and/or Shift keys." ).arg(QChar(key.sym()));
			KMessageBox::sorry( this, s, i18n("Invalid Shortcut Key") );
			return;
		}
	}

	// If key isn't already in use,
	if( !isKeyPresent( cut ) ) {
		// Set new key code
		pItem->setShortcut( cut );
		// Update display
		updateButtons();
		emit keyChange();
	}
}

// Returns iSeq index if cut2 has a sequence of equal or higher priority to a sequence in cut.
// else -1
static int keyConflict( const KShortcut& cut, const KShortcut& cut2 )
{
	for( uint iSeq = 0; iSeq < cut.count(); iSeq++ ) {
		for( uint iSeq2 = 0; iSeq2 <= iSeq && iSeq2 < cut2.count(); iSeq2++ ) {
			if( cut.seq(iSeq) == cut2.seq(iSeq2) )
				return iSeq;
		}
	}
	return -1;
}

bool KKeyChooser::isKeyPresent( const KShortcut& cut, bool bWarnUser )
{
	KKeyChooserItem* pItem = dynamic_cast<KKeyChooserItem*>(d->pList->currentItem());

	// If editing global shortcuts, check them for conflicts with the stdaccels.
	if( m_type == ApplicationGlobal || m_type == Global ) {
		// For each key sequence in the shortcut,
		for( uint i = 0; i < cut.count(); i++ ) {
			const KKeySequence& seq = cut.seq(i);

			KStdAccel::StdAccel id = KStdAccel::findStdAccel( seq );
			if( id != KStdAccel::AccelNone
			    && keyConflict( cut, KStdAccel::shortcut( id ) ) > -1 ) {
				if( bWarnUser )
					_warning( seq, KStdAccel::label(id), i18n("Conflict with Standard Application Shortcut") );
				return true;
			}
		}
	}

	QMap<QString, KShortcut>::ConstIterator it;
	for( it = d->mapGlobals.begin(); it != d->mapGlobals.end(); ++it ) {
		int iSeq = keyConflict( cut, (*it) );
		if( iSeq > -1 ) {
			if( m_type != Global || it.key() != pItem->actionName() ) {
				if( bWarnUser )
					_warning( cut.seq(iSeq), it.key(), i18n("Conflict with Global Shortcuts") );
				return true;
			}
		}
	}
        
        if( isKeyPresentLocally( cut, pItem, bWarnUser ? i18n("Key Conflict") : QString::null ))
            return true;

        // check also other Global KKeyChooser's
        if( m_type == Global && globalChoosers != NULL ) {
            for( QValueList< KKeyChooser* >::ConstIterator it = globalChoosers->begin();
                 it != globalChoosers->end();
                 ++it ) {
                if( (*it) != this && (*it)->isKeyPresentLocally( cut, NULL,
                    bWarnUser ? i18n("Key Conflict") : QString::null))
                    return true;
            }
        }
	return false;
}

bool KKeyChooser::isKeyPresentLocally( const KShortcut& cut, KKeyChooserItem* ignoreItem, const QString& warnText )
{
	// Search for shortcut conflicts with other actions in the
	//  lists we're configuring.
	for( QListViewItemIterator it( d->pList ); it.current(); ++it ) {
		KKeyChooserItem* pItem2 = dynamic_cast<KKeyChooserItem*>(it.current());
		if( pItem2 && pItem2 != ignoreItem ) {
			int iSeq = keyConflict( cut, pItem2->shortcut() );
			if( iSeq > -1 ) {
				if( !warnText.isNull() )
					_warning( cut.seq(iSeq), pItem2->text(0), warnText );
				return true;
			}
		}
	}
        return false;            
}

void KKeyChooser::_warning( const KKeySequence& cut, QString sAction, QString sTitle )
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
KKeyChooserItem::KKeyChooserItem( KListView* parent, QListViewItem* after, KShortcutList* pList, uint iAction )
:	KListViewItem( parent, after )
{
	m_pList = pList;
	m_iAction = iAction;
	m_bModified = false;
	m_cut = m_pList->shortcut(m_iAction);
}

KKeyChooserItem::KKeyChooserItem( QListViewItem* parent, QListViewItem* after, KShortcutList* pList, uint iAction )
:	KListViewItem( parent, after )
{
	m_pList = pList;
	m_iAction = iAction;
	m_bModified = false;
	m_cut = m_pList->shortcut(m_iAction);
}

QString KKeyChooserItem::actionName() const
{
	return m_pList->name(m_iAction);
}

const KShortcut& KKeyChooserItem::shortcut() const
{
	return m_cut;
}

void KKeyChooserItem::setShortcut( const KShortcut& cut )
{
	m_cut = cut;
	m_bModified = (m_cut != m_pList->shortcut(m_iAction));
	listView()->repaintItem( this );
}

void KKeyChooserItem::commitChanges()
{
	if( m_bModified )
		m_pList->setShortcut( m_iAction, m_cut );
}

QString KKeyChooserItem::text( int iCol ) const
{
	if( iCol == 0 ) {
		// Quick HACK to get rid of '&'s.
		// TODO: convert '&&' => '&' and leave it in.
		QString s = m_pList->label(m_iAction);
		QString s2;
		for( uint i = 0; i < s.length(); i++ )
			if( s[i] != '&' )
				s2 += s[i];
		return s2;
	}
	else if( iCol <= (int) m_cut.count() )
		return m_cut.seq(iCol-1).toString();
	else
		return QString::null;
}

int KKeyChooserItem::compare( QListViewItem* item, int iCol, bool bAscending ) const
{
	KKeyChooserItem* pItem = dynamic_cast<KKeyChooserItem*>( item );
	if( iCol == 0 && pItem ) {
		QString psName1 = m_pList->name(m_iAction);
		QString psName2 = pItem->m_pList->name(pItem->m_iAction);
		QRegExp rxNumber1( " (\\d+)$" );
		QRegExp rxNumber2( " (\\d+)$" );
		int iNumber1 = rxNumber1.search( psName1 );
		int iNumber2 = rxNumber2.search( psName2 );

		// Check if the last word is one or more digits
		if( iNumber1 >= 0 && iNumber1 == iNumber2 && psName1.startsWith( psName2.left( iNumber1+1 ) ) ) {
			int n1 = rxNumber1.cap(1).toInt();
			int n2 = rxNumber2.cap(1).toInt();
			return (n1 < n2) ? -1 : (n1 > n2) ? 1 : 0;
		}
	}

	return QListViewItem::compare( item, iCol, bAscending );
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
KKeyDialog::KKeyDialog( KKeyChooser::ActionType type, bool bAllowLetterShortcuts, QWidget *parent, const char* name )
: KDialogBase( parent, name, true, i18n("Configure Shortcuts"), Help|Default|Ok|Cancel, Ok )
{
	m_pKeyChooser = new KKeyChooser( this, type, bAllowLetterShortcuts );
	setMainWidget( m_pKeyChooser );
	connect( this, SIGNAL(defaultClicked()), m_pKeyChooser, SLOT(allDefault()) );
	enableButton( Help, false );

	KConfigGroup group( KGlobal::config(), "KKeyDialog Settings" );
	QSize sz = size();
	resize( group.readSizeEntry( "Dialog Size", &sz ) );
}

KKeyDialog::KKeyDialog( bool bAllowLetterShortcuts, QWidget *parent, const char* name )
: KDialogBase( parent, name, true, i18n("Configure Shortcuts"), Help|Default|Ok|Cancel, Ok )
{
	m_pKeyChooser = new KKeyChooser( this, KKeyChooser::Application, bAllowLetterShortcuts );
	setMainWidget( m_pKeyChooser );
	connect( this, SIGNAL(defaultClicked()), m_pKeyChooser, SLOT(allDefault()) );
	enableButton( Help, false );

	KConfigGroup group( KGlobal::config(), "KKeyDialog Settings" );
	QSize sz = size();
	resize( group.readSizeEntry( "Dialog Size", &sz ) );
}

KKeyDialog::~KKeyDialog()
{
	KConfigGroup group( KGlobal::config(), "KKeyDialog Settings" );
	group.writeEntry( "Dialog Size", size(), true, true );
}

bool KKeyDialog::insert( KActionCollection* pColl )
{
	return m_pKeyChooser->insert( pColl );
}

bool KKeyDialog::insert(KActionCollection *pColl, const QString &title)
{
    return m_pKeyChooser->insert(pColl, title);
}

bool KKeyDialog::configure( bool bSaveSettings )
{
	int retcode = exec();
	if( retcode == Accepted ) {
		if( bSaveSettings )
			m_pKeyChooser->save();
		else
			commitChanges();
	}
	return retcode;
}

void KKeyDialog::commitChanges()
{
	m_pKeyChooser->commitChanges();
}

int KKeyDialog::configure( KActionCollection* coll, QWidget* parent, bool bSaveSettings )
{
	return configure( coll, true, parent, bSaveSettings);
}

int KKeyDialog::configure( KAccel* keys, QWidget* parent, bool bSaveSettings )
{
	return configure( keys, true, parent, bSaveSettings);
}

int KKeyDialog::configure( KGlobalAccel* keys, QWidget* parent, bool bSaveSettings )
{
	return configure( keys, true, parent, bSaveSettings);
}

int KKeyDialog::configure( KAccel* keys, bool bAllowLetterShortcuts, QWidget *parent, bool bSaveSettings )
{
	KKeyDialog dlg( bAllowLetterShortcuts, parent );
	dlg.m_pKeyChooser->insert( keys );
	bool b = dlg.configure( bSaveSettings );
	if( b && bSaveSettings )
		keys->updateConnections();
	return b;
}

int KKeyDialog::configure( KGlobalAccel* keys, bool bAllowLetterShortcuts, QWidget *parent, bool bSaveSettings )
{
	KKeyDialog dlg( KKeyChooser::ApplicationGlobal, bAllowLetterShortcuts, parent );
	dlg.m_pKeyChooser->insert( keys );
	bool b = dlg.configure( bSaveSettings );
	if( b && bSaveSettings )
		keys->updateConnections();
	return b;
}

int KKeyDialog::configure( KActionCollection* coll, bool bAllowLetterShortcuts, QWidget *parent, bool bSaveSettings )
{
	kdDebug(125) << "KKeyDialog::configureKeys( KActionCollection*, " << bSaveSettings << " )" << endl;
	KKeyDialog dlg( bAllowLetterShortcuts, parent );
	dlg.m_pKeyChooser->insert( coll );
	return dlg.configure( bSaveSettings );
}

/*int KKeyDialog::configure( KActionPtrList* coll, const QString& file, QWidget *parent, bool bSaveSettings )
{
	kdDebug(125) << "KKeyDialog::configureKeys( KActionCollection*, " << file << ", " << bSaveSettings << " )" << endl;
	KAccelActions actions;
	coll->createKeyMap( actions );

	int retcode = configure( actions, file, parent, bSaveSettings );
	if( retcode == Accepted )
		coll->setKeyMap( actions );

	return retcode;
}*/

void KKeyChooser::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

void KKeyDialog::virtual_hook( int id, void* data )
{ KDialogBase::virtual_hook( id, data ); }

#include "kkeydialog.moc"
