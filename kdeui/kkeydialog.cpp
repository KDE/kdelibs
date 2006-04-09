/* This file is part of the KDE libraries
    Copyright (C) 1998 Mark Donohoe <donohoe@kde.org>
    Copyright (C) 1997 Nicolas Hadacek <hadacek@kde.org>
    Copyright (C) 1998 Matthias Ettrich <ettrich@kde.org>
    Copyright (C) 2001 Ellis Whitehead <ellis@kde.org>
    Copyright (C) 2006 Hamish Rodda <rodda@kde.org>

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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "kkeydialog.h"
#include "kkeybutton.h"

#include "ui_kkeydialogui.h"

#include <string.h>

#include <QTreeWidgetItem>
#include <QGroupBox>
#include <QTimer>
#include <QHeaderView>

#include <kaction.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kglobalaccel.h>
#include <kicon.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kxmlguifactory.h>
#include <kaboutdata.h>
#include <kstaticdeleter.h>
#include <kstdaccel.h>

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
#endif // KEYPRESS
#endif // Q_WX_X11


class KKeyChooserItem : public QTreeWidgetItem
{
 public:
	KKeyChooserItem( QTreeWidget* parent, QTreeWidgetItem* after, KAction* action );
	KKeyChooserItem( QTreeWidgetItem* parent, QTreeWidgetItem* after, KAction* action );
	virtual ~KKeyChooserItem() {}

	KAction* action() const;
	QString actionName() const;
	const KShortcut& shortcut() const;
	bool isConfigurable() const;
	const KShortcut& defaultShortcut() const;

	void setLocalShortcut( const KShortcut& cut );
	void setGlobalShortcut( const KShortcut& cut );
	
	const KShortcut& globalShortcut() const;
	const KShortcut& defaultGlobalShortcut() const;

	void commitChanges();

	virtual QVariant data(int column, int role) const;

 private:
	void checkModified();
	bool checkChanged(const KShortcut& cut1, const KShortcut& cut2, int sequence) const;
 
	KAction* m_action;
	bool m_modified;
	KShortcut m_newLocalShortcut, m_newGlobalShortcut;
};

//---------------------------------------------------------------------
// KKeyChooserPrivate
//---------------------------------------------------------------------

class KKeyChooserPrivate
{
 public:
 	KKeyChooserPrivate()
 		: ui(new Ui::KKeyDialog())
	{
	}
	
	~KKeyChooserPrivate()
	{
		delete ui;
	}

	QList< QList<KAction*> > rgpLists;
	QList< KActionCollection* > collections;

	Ui::KKeyDialog* ui;
	
	// If this is set, then shortcuts require a modifier:
	//  so 'A' would not be valid, whereas 'Ctrl+A' would be.
	// Note, however, that this only applies to printable characters.
	//  'F1', 'Insert', etc., could still be used.
	bool allowLetterShortcuts;

	KKeyChooser::ActionTypes type;
};

//---------------------------------------------------------------------
// KKeyChooser
//---------------------------------------------------------------------

KKeyChooser::KKeyChooser( KActionCollection* coll, QWidget* parent, ActionTypes actionType, LetterShortcuts allowLetterShortcuts )
: QWidget( parent )
, d(new KKeyChooserPrivate)
{
	initGUI( actionType, allowLetterShortcuts );
	insert( coll );
}

KKeyChooser::KKeyChooser( QWidget* parent, ActionTypes actionType, LetterShortcuts allowLetterShortcuts )
: QWidget( parent )
, d(new KKeyChooserPrivate)
{
	initGUI( actionType, allowLetterShortcuts );
}

// list of all existing KKeyChooser's
// Used when checking global shortcut for a possible conflict
// (just checking against kdeglobals isn't enough, the shortcuts
// might have changed in KKeyChooser and not being saved yet).
// Also used when reassigning a shortcut from one chooser to another.
static QList< KKeyChooser* >* allChoosers = NULL;
static KStaticDeleter< QList< KKeyChooser* > > allChoosersDeleter;

KKeyChooser::~KKeyChooser()
{
	allChoosers->removeAll( this );
	delete d;
}

bool KKeyChooser::insert( KActionCollection* pColl, const QString &title )
{
	QString str = title;
	if ( title.isEmpty() && pColl->instance() && pColl->instance()->aboutData() )
		str = pColl->instance()->aboutData()->programName();

	d->rgpLists.append( pColl->actions() );
	d->collections.append( pColl );
	buildListView(d->rgpLists.count() - 1, str);
	
	return true;
}

void KKeyChooser::commitChanges()
{
	kDebug(125) << "KKeyChooser::commitChanges()" << endl;

	QTreeWidgetItemIterator it( d->ui->list );
	for( ; *it; ++it ) {
		KKeyChooserItem* pItem = dynamic_cast<KKeyChooserItem*>(*it);
		if( pItem )
			pItem->commitChanges();
	}
}

void KKeyChooser::save()
{
	commitChanges();
	foreach (KActionCollection* collection, d->collections)
		collection->writeSettings();
	if (KGlobalAccel::self()->actionsWithGlobalShortcut().count())
		KGlobalAccel::self()->writeSettings();
}

void KKeyChooser::initGUI( ActionTypes type, LetterShortcuts allowLetterShortcuts )
{
	  d->type = type;
	  d->allowLetterShortcuts = allowLetterShortcuts;
	
	  d->ui->setupUi(this);
	
	  d->ui->searchFilter->searchLine()->setTreeWidget(d->ui->list); // Plug into search line
	
	  d->ui->list->header()->setStretchLastSection(false);
	
	  connect( d->ui->list, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)),
				SLOT(slotListItemSelected(QTreeWidgetItem*)) );
	
	  // handle activation of an item
	  connect( d->ui->list, SIGNAL ( itemActivated ( QTreeWidgetItem *, int ) ),
				SLOT ( captureCurrentItem()) );
	
	  connect( d->ui->localNone, SIGNAL(clicked()), SLOT(slotLocalNoKey()) );
	  connect( d->ui->localDefault, SIGNAL(clicked()), SLOT(slotLocalDefaultKey()) );
	  connect( d->ui->localCustom, SIGNAL(clicked()), SLOT(slotLocalCustomKey()) );
	  connect( d->ui->localCustomSelector, SIGNAL(capturedShortcut(const KShortcut&)), SLOT(slotLocalCapturedShortcut(const KShortcut&)) );
	
	  connect( d->ui->globalNone, SIGNAL(clicked()), SLOT(slotGlobalNoKey()) );
	  connect( d->ui->globalDefault, SIGNAL(clicked()), SLOT(slotGlobalDefaultKey()) );
	  connect( d->ui->globalCustom, SIGNAL(clicked()), SLOT(slotGlobalCustomKey()) );
	  connect( d->ui->globalCustomSelector, SIGNAL(capturedShortcut(const KShortcut&)), SLOT(slotGlobalCapturedShortcut(const KShortcut&)) );
	
	  connect( kapp, SIGNAL( settingsChanged( int )), SLOT( slotSettingsChanged( int )));
	
	  if( allChoosers == NULL )
			allChoosers = allChoosersDeleter.setObject( allChoosers, new QList< KKeyChooser* > );
	  allChoosers->append( this );
}

// Add all shortcuts to the list
void KKeyChooser::buildListView( uint iList, const QString &title )
{
	const QList<KAction*> pList = d->rgpLists[iList];

	if( d->type & GlobalAction || d->type & ApplicationAction )
	    d->ui->list->setEnabled(false);

	QTreeWidgetItem *pProgramItem, *pGroupItem = 0, *pParentItem, *pItem;

	QString str = (title.isEmpty() ? i18n("Shortcuts") : title);
	pParentItem = pProgramItem = pItem = new QTreeWidgetItem( d->ui->list, QStringList() << str );
	d->ui->list->expandItem(pParentItem);
	//pParentItem->setFlags(pParentItem->flags() & !Qt::ItemIsSelectable);
	foreach (KAction* action, pList) {
		QString sName = action->text();
		kDebug(125) << "Key: " << sName << endl;
		if( sName.startsWith( "Program:" ) ) {
			pItem = new QTreeWidgetItem( d->ui->list, pProgramItem );
			pItem->setText(0, action->text());
			//pItem->setFlags(pItem->flags() & !Qt::ItemIsSelectable);
			d->ui->list->expandItem(pItem);
			if( !pProgramItem->childCount() )
				delete pProgramItem;
			pProgramItem = pParentItem = pItem;
		} else if( sName.startsWith( "Group:" ) ) {
			pItem = new QTreeWidgetItem( pProgramItem, pParentItem );
			pItem->setText(0, action->text());
			//pItem->setFlags(pItem->flags() & !Qt::ItemIsSelectable);
			d->ui->list->expandItem(pItem);
			if( pGroupItem && !pGroupItem->childCount() )
				delete pGroupItem;
			pGroupItem = pParentItem = pItem;
		} else if( !sName.isEmpty() && sName != "unnamed"  && action->isShortcutConfigurable() ) {
			pItem = new KKeyChooserItem( pParentItem, pItem, action );
		}
	}
	if( !pProgramItem->childCount() )
		delete pProgramItem;
	if( pGroupItem && !pGroupItem->childCount() )
		delete pGroupItem;
}


void KKeyChooser::updateButtons()
{
	// Hack: Do this incase we still have changeKey() running.
	//  Better would be to capture the mouse pointer so that we can't click
	//   around while we're supposed to be entering a key.
	//  Better yet would be a modal dialog for changeKey()!
	releaseKeyboard();
	KKeyChooserItem* pItem = dynamic_cast<KKeyChooserItem*>( d->ui->list->currentItem() );

	if ( !pItem ) {
		// if nothing is selected -> disable radio boxes
		d->ui->localGroup->setEnabled( false );
		d->ui->localCustomSelector->setShortcut( KShortcut() );

		d->ui->globalGroup->setEnabled( false );
		d->ui->globalCustomSelector->setShortcut( KShortcut() );

	} else {
		// Set key strings
		QString localKeyStrCfg = pItem->shortcut().toString();
		QString localKeyStrDef = pItem->defaultShortcut().toString();
		QString globalKeyStrCfg = pItem->globalShortcut().toString();
		QString globalKeyStrDef = pItem->defaultGlobalShortcut().toString();

		d->ui->localCustomSelector->setShortcut( pItem->shortcut() );
		d->ui->globalCustomSelector->setShortcut( pItem->globalShortcut() );
		
		d->ui->localDefault->setText( i18nc("Default (default shortcut)", "De&fault (%1)", localKeyStrDef.isEmpty() ? i18n("none") : localKeyStrDef) );
		d->ui->globalDefault->setText( i18nc("Default (default shortcut)", "De&fault (%1)" , globalKeyStrDef.isEmpty() ? i18n("none") : globalKeyStrDef) );

		// Select the appropriate radio button.
		if (pItem->shortcut().isNull())
			d->ui->localNone->setChecked( true );
		else if (pItem->shortcut() == pItem->defaultShortcut())
			d->ui->localDefault->setChecked( true );
		else
			d->ui->localCustom->setChecked( true );

		if (pItem->globalShortcut().isNull())
			d->ui->globalNone->setChecked( true );
		else if (pItem->globalShortcut() == pItem->defaultGlobalShortcut())
			d->ui->globalDefault->setChecked( true );
		else
			d->ui->globalCustom->setChecked( true );

		// Enable buttons if this key is configurable.
		// The 'Default Key' button must also have a default key.
		d->ui->localGroup->setEnabled( pItem->isConfigurable() );
		d->ui->localDefault->setEnabled( pItem->isConfigurable() && pItem->defaultShortcut().count() != 0 );

		d->ui->globalGroup->setEnabled( pItem->action()->globalShortcutAllowed() );
		d->ui->globalDefault->setEnabled( pItem->action()->globalShortcutAllowed() && pItem->defaultGlobalShortcut().count() != 0 );
	}
}

void KKeyChooser::slotLocalNoKey()
{
	// return if no key is selected
	KKeyChooserItem* pItem = dynamic_cast<KKeyChooserItem*>( d->ui->list->currentItem() );
	if( pItem ) {
		//kDebug(125) << "no Key" << d->ui->list->currentItem()->text(0) << endl;
		pItem->setLocalShortcut( KShortcut() );
		updateButtons();
		emit keyChange();
	}
}

void KKeyChooser::slotLocalDefaultKey()
{
	// return if no key is selected
	KKeyChooserItem* pItem = dynamic_cast<KKeyChooserItem*>( d->ui->list->currentItem() );
	if( pItem ) // don't set it directly, check for conflicts
		setLocalShortcut( pItem->defaultShortcut() );
}

void KKeyChooser::slotLocalCustomKey()
{
	d->ui->localCustomSelector->captureShortcut();
}

void KKeyChooser::slotGlobalNoKey()
{
	// return if no key is selected
	KKeyChooserItem* pItem = dynamic_cast<KKeyChooserItem*>( d->ui->list->currentItem() );
	if( pItem ) {
		//kDebug(125) << "no Key" << d->ui->list->currentItem()->text(0) << endl;
		pItem->setGlobalShortcut( KShortcut() );
		updateButtons();
		emit keyChange();
	}
}

void KKeyChooser::slotGlobalDefaultKey()
{
	// return if no key is selected
	KKeyChooserItem* pItem = dynamic_cast<KKeyChooserItem*>( d->ui->list->currentItem() );
	if( pItem ) // don't set it directly, check for conflicts
		setGlobalShortcut( pItem->defaultShortcut() );
}

void KKeyChooser::slotGlobalCustomKey()
{
	d->ui->globalCustomSelector->captureShortcut();
}

void KKeyChooser::slotSettingsChanged( int category )
{
	if( category == KApplication::SETTINGS_SHORTCUTS )
		KGlobalAccel::self()->readSettings();
}

// KDE4 IMHO this shouldn't be here at all - it cannot check whether the default
// shortcut don't conflict with some already changed ones (e.g. global shortcuts).
// Also, I personally find reseting all shortcuts to default (i.e. hardcoded in the app)
// ones after pressing the 'Default' button rather a misfeature.
void KKeyChooser::allDefault()
{
	kDebug(125) << "KKeyChooser::allDefault()" << endl;

	QTreeWidgetItemIterator it( d->ui->list );
	for( ; *it; ++it ) {
		KKeyChooserItem* pItem = dynamic_cast<KKeyChooserItem*>(*it);
		if( pItem ) {
			pItem->setLocalShortcut( pItem->defaultShortcut() );
			pItem->setGlobalShortcut( pItem->defaultGlobalShortcut() );
		}
	}

	updateButtons();
	emit keyChange();
}

void KKeyChooser::slotListItemSelected( QTreeWidgetItem* )
{
	updateButtons();
}

void KKeyChooser::captureCurrentItem()
{
	KKeyChooserItem* pItem = dynamic_cast<KKeyChooserItem*>( d->ui->list->currentItem() );
	if( pItem != NULL && pItem->isConfigurable())
		d->ui->localCustomSelector->captureShortcut ( );
}

void KKeyChooser::slotLocalCapturedShortcut( const KShortcut& cut )
{
	if( cut.isNull() )
		slotLocalNoKey();
	else
		setLocalShortcut( cut );
}

void KKeyChooser::slotGlobalCapturedShortcut( const KShortcut& cut )
{
	if( cut.isNull() )
		slotGlobalNoKey();
	else
		setGlobalShortcut( cut );
}

void KKeyChooser::setLocalShortcut( const KShortcut& cut )
{
	kDebug(125) << k_funcinfo << cut.toString() << endl;
	KKeyChooserItem* pItem = dynamic_cast<KKeyChooserItem*>(d->ui->list->currentItem());
	if( !pItem )
		return;

	foreach (const QKeySequence& seq, cut.sequences()) {
		if (seq.isEmpty())
			continue;
		
		int key = seq[0];

		if( !d->allowLetterShortcuts && (key & Qt::KeyboardModifierMask) == 0
		    && key < 0x3000 && QChar(key).isLetterOrNumber() ) {
			QString s = i18n( 	"In order to use the '%1' key as a shortcut, "
						"it must be combined with the "
						"Meta, Alt, Ctrl, and/or Shift keys.", QChar(key));
			KMessageBox::sorry( this, s, i18n("Invalid Shortcut Key") );
			return;
		}
	}

	// If key isn't already in use,
	if( !isKeyPresent( cut ) ) {
		// Set new key code
		pItem->setLocalShortcut( cut );
		// Update display
		updateButtons();
		emit keyChange();
	}
}

void KKeyChooser::setGlobalShortcut( const KShortcut& cut )
{
	kDebug(125) << k_funcinfo << cut.toString() << endl;
	KKeyChooserItem* pItem = dynamic_cast<KKeyChooserItem*>(d->ui->list->currentItem());
	if( !pItem )
		return;

	foreach (const QKeySequence& seq, cut.sequences()) {
		if (seq.isEmpty())
			continue;
		
		int key = seq[0];

		if( !d->allowLetterShortcuts && (key & Qt::KeyboardModifierMask) == 0
		    && key < 0x3000 && QChar(key).isLetterOrNumber() ) {
			QString s = i18n( 	"In order to use the '%1' key as a shortcut, "
						"it must be combined with the "
						"Meta, Alt, Ctrl, and/or Shift keys." , QChar(key));
			KMessageBox::sorry( this, s, i18n("Invalid Shortcut Key") );
			return;
		}
	}

	// If key isn't already in use,
	if( !isKeyPresent( cut ) ) {
		// Set new key code
		pItem->setGlobalShortcut( cut );
		// Update display
		updateButtons();
		emit keyChange();
	}
}

// Returns iSeq index if cut2 has a sequence of equal or higher priority to a sequence in cut.
// else -1
static int keyConflict( const KShortcut& cut, const KShortcut& cut2 )
{
	for( int iSeq = 0; iSeq < cut.count(); iSeq++ ) {
		for( int iSeq2 = 0; iSeq2 < cut2.count(); iSeq2++ ) {
			if( cut.seq(iSeq) == cut2.seq(iSeq2) )
				return iSeq;
		}
	}
	return -1;
}

// Removes the sequences in cut2 from cut1
static void removeFromShortcut(  KShortcut & cut1, const KShortcut &cut2)
{
	for( int iSeq2 = 0; iSeq2 < cut2.count(); iSeq2++ )
		cut1.remove(cut2.seq(iSeq2));
}

bool KKeyChooser::isKeyPresent( const KShortcut& cut, bool bWarnUser )
{
	KKeyChooserItem* pItem = dynamic_cast<KKeyChooserItem*>(d->ui->list->currentItem());

	if (!pItem) {
		return false;
	}

	bool has_global_chooser = false;
	bool has_standard_chooser = false;
	for( QList< KKeyChooser* >::ConstIterator it = allChoosers->begin();
		 it != allChoosers->end();
		 ++it ) {
		has_global_chooser |= ((*it)->d->type & GlobalAction);
		has_standard_chooser |= ((*it)->d->type & StandardAction);
	}

	// If editing global shortcuts, check them for conflicts with the stdaccels.
	if( d->type & ApplicationAction || d->type & GlobalAction ) {
		if( !has_standard_chooser ) {
			if( checkStandardShortcutsConflict( cut, bWarnUser, this ))
				return true;
		}
	}

	// only check the global keys if one of the keychoosers isn't global
	if( !has_global_chooser ) {
		if( checkGlobalShortcutsConflictInternal( cut, bWarnUser, this,
			(d->type & GlobalAction) ? pItem->actionName() : QString() ))
			return true;
	}

	if( isKeyPresentLocally( cut, pItem, bWarnUser ))
		return true;

	// check also other KKeyChooser's
	for( QList< KKeyChooser* >::ConstIterator it = allChoosers->begin();
		 it != allChoosers->end();
		 ++it ) {
		if( (*it) != this && (*it)->isKeyPresentLocally( cut, NULL, bWarnUser ))
				return true;
		}
	return false;
}

bool KKeyChooser::isKeyPresentLocally( const KShortcut& cut, KKeyChooserItem* ignoreItem, bool bWarnUser )
{
    if ( cut.toString().isEmpty())
        return false;
	// Search for shortcut conflicts with other actions in the
	//  lists we're configuring.
	for( QTreeWidgetItemIterator it( d->ui->list ); *it; ++it ) {
		KKeyChooserItem* pItem2 = dynamic_cast<KKeyChooserItem*>(*it);
		if( pItem2 && pItem2 != ignoreItem ) {
			int iSeq = keyConflict( cut, pItem2->shortcut() );
			if( iSeq > -1 ) {
				if( bWarnUser ) {
					if( !promptForReassign( cut.seq(iSeq), pItem2->actionName(), WindowAction, this ))
						return true;
					// else remove the shortcut from it
					KShortcut cut2 = pItem2->shortcut();
					removeFromShortcut(cut2, cut);
					pItem2->setLocalShortcut(cut2);
					updateButtons();
					emit keyChange();
				}
			}
			iSeq = keyConflict( cut, pItem2->globalShortcut() );
			if( iSeq > -1 ) {
				if( bWarnUser ) {
					if( !promptForReassign( cut.seq(iSeq), pItem2->actionName(), WindowAction, this ))
						return true;
					// else remove the shortcut from it
					KShortcut cut2 = pItem2->globalShortcut();
					removeFromShortcut(cut2, cut);
					pItem2->setGlobalShortcut(cut2);
					updateButtons();
					emit keyChange();
				}
			}
		}
	}
        return false;
}

bool KKeyChooser::checkStandardShortcutsConflict( const KShortcut& cut, bool bWarnUser, QWidget* parent )
{
	// For each key sequence in the shortcut,
	foreach (const QKeySequence& seq, cut.sequences()) {
		KStdAccel::StdAccel id = KStdAccel::findStdAccel( seq );
		if( id != KStdAccel::AccelNone
			&& keyConflict( cut, KStdAccel::shortcut( id ) ) > -1 ) {
			if( bWarnUser ) {
				if( !promptForReassign( seq, KStdAccel::label(id), StandardAction, parent ))
					return true;
				removeStandardShortcut( KStdAccel::label(id), dynamic_cast< KKeyChooser* > ( parent ), cut);
			}
		}
	}
	return false;
}

bool KKeyChooser::checkGlobalShortcutsConflictInternal( const KShortcut& cut, bool bWarnUser, QWidget* parent, const QString& ignoreAction )
{
	foreach (KAction* action, KGlobalAccel::self()->actionsWithGlobalShortcut()) {
		int iSeq = keyConflict( cut, action->globalShortcut() );
		if( iSeq > -1 ) {
			if( ignoreAction.isEmpty() || action->objectName() != ignoreAction ) {
				if( bWarnUser ) {
					if( !promptForReassign( cut.seq(iSeq), action->objectName(), GlobalAction, parent ))
						return true;
					removeGlobalShortcut( action, dynamic_cast< KKeyChooser* >( parent ), cut);
				}
			}
		}
	}
	return false;
}

void KKeyChooser::removeStandardShortcut( const QString& name, KKeyChooser* chooser, const KShortcut &cut )
{
	bool was_in_choosers = false;
	if( allChoosers != NULL ) {
		foreach (KKeyChooser* c, *allChoosers) {
			if( c != chooser && c->d->type & StandardAction ) {
				was_in_choosers |= ( c->removeShortcut( name, cut ));
			}
		}
	}

	if( !was_in_choosers ) { // not edited, needs to be changed in config file
		KStdAccel::StdAccel id = KStdAccel::findStdAccel( name.toAscii().constData() );
		if (id != KStdAccel::AccelNone)
			KStdAccel::saveShortcut(id, cut);
		else
			kWarning(125) << k_funcinfo << "Cannot find stdaccel matching name " << name << endl;
	}
}

void KKeyChooser::removeGlobalShortcut( KAction* action, KKeyChooser* chooser, const KShortcut &cut )
{
	bool was_in_choosers = false;
	if( allChoosers != NULL ) {
		foreach (KKeyChooser* c, *allChoosers) {
			if( c != chooser && c->d->type & GlobalAction ) {
				was_in_choosers |= ( c->removeShortcut( action->objectName(), cut ));
			}
		}
	}

	if( !was_in_choosers ) { // not edited, needs to be changed in config file
		KGlobalAccel::self()->writeSettings(0L, false, action);
	}
}

bool KKeyChooser::removeShortcut( const QString& name, const KShortcut &cut )
{
	for( QTreeWidgetItemIterator it( d->ui->list ); *it; ++it ) {
		KKeyChooserItem* pItem2 = dynamic_cast<KKeyChooserItem*>(*it);
		if( pItem2 && pItem2->actionName() == name ) {
			// remove the shortcut from it
			KShortcut cut2 = pItem2->shortcut();
			removeFromShortcut(cut2, cut);
			if (cut2 != cut) {
				pItem2->setLocalShortcut(cut2);
			} else {
				cut2 = pItem2->globalShortcut();
				removeFromShortcut(cut2, cut);
				pItem2->setGlobalShortcut(cut2);
			}
			updateButtons();
			emit keyChange();
			return true;
		}
	}
	
	return false;
}

bool KKeyChooser::promptForReassign( const QKeySequence& cut, const QString& sAction, ActionType type, QWidget* parent )
{
	if(cut.isEmpty())
		return true;
	QString sTitle;
	QString s;
	if( type == StandardAction ) {
		sTitle = i18n("Conflict with Standard Application Shortcut");
		s = i18n("The '%1' key combination has already been allocated "
		"to the standard action \"%2\".\n"
		"Do you want to reassign it from that action to the current one?",
		cut.toString(), sAction.trimmed());
	}
	else if( type == GlobalAction ) {
		sTitle = i18n("Conflict with Global Shortcut");
		s = i18n("The '%1' key combination has already been allocated "
		"to the global action \"%2\".\n"
		"Do you want to reassign it from that action to the current one?",
		cut.toString(), sAction.trimmed());
	}
	else {
		sTitle = i18n("Key Conflict");
		s = i18n("The '%1' key combination has already been allocated "
		"to the \"%2\" action.\n"
		"Do you want to reassign it from that action to the current one?",
		cut.toString(), sAction.trimmed());
	}

	return KMessageBox::warningContinueCancel( parent, s, sTitle, i18n("Reassign") ) == KMessageBox::Continue;
}

//---------------------------------------------------
KKeyChooserItem::KKeyChooserItem( QTreeWidget* parent, QTreeWidgetItem* after, KAction* action )
	: QTreeWidgetItem( parent, after )
	, m_action(action)
	, m_modified(false)
	, m_newLocalShortcut(action->shortcut())
	, m_newGlobalShortcut(action->globalShortcut())
{
}

KKeyChooserItem::KKeyChooserItem( QTreeWidgetItem* parent, QTreeWidgetItem* after, KAction* action )
	: QTreeWidgetItem( parent, after )
	, m_action(action)
	, m_modified(false)
	, m_newLocalShortcut(action->shortcut())
	, m_newGlobalShortcut(action->globalShortcut())
{
}

QVariant KKeyChooserItem::data(int column, int role) const
{
	switch (column) {
		case 0:
			switch(role) {
				case Qt::DisplayRole:
					return m_action->text().remove("&");
				case Qt::DecorationRole:
					return m_action->icon();
				case Qt::WhatsThisRole:
					return m_action->whatsThis();
				case Qt::ToolTipRole:
					return m_action->toolTip();
				case Qt::StatusTipRole:
					return m_action->statusTip();
			}

		case 1:
		case 2:
			switch (role) {
				case Qt::DisplayRole:
					if (column - 1 < m_newLocalShortcut.count())
						return m_newLocalShortcut.seq(column - 1).toString();
					break;
				case Qt::FontRole:
					if (checkChanged(m_newLocalShortcut, m_action->shortcut(), column - 1)) {
						QFont parentFont = treeWidget()->font();
						parentFont.setBold(true);
						return parentFont;
					}
					break;
				case Qt::TextColorRole:
					if (checkChanged(m_newLocalShortcut, m_action->shortcut(), column - 1))
						return QColor(Qt::blue);
				default:
					break;
			}
			break;

		case 3:
			switch (role) {
				case Qt::DisplayRole:
					if (m_newGlobalShortcut.count())
						return m_newGlobalShortcut.seq(0).toString();
					break;
				case Qt::FontRole:
					if (checkChanged(m_newGlobalShortcut, m_action->globalShortcut(), 0)) {
						QFont parentFont = treeWidget()->font();
						parentFont.setBold(true);
						return parentFont;
					}
					break;
				case Qt::TextColorRole:
					if (checkChanged(m_newGlobalShortcut, m_action->globalShortcut(), 0))
						return QColor(Qt::blue);
				default:
					break;
			}
			break;
	}
	
	return QVariant();
}

QString KKeyChooserItem::actionName() const
{
	return m_action->objectName();
}

void KKeyChooserItem::setLocalShortcut( const KShortcut& cut )
{
	m_newLocalShortcut = cut;
	checkModified();
}

void KKeyChooserItem::commitChanges()
{
	if( m_modified ) {
		if ( m_newLocalShortcut != m_action->shortcut() )
			m_action->setShortcut( m_newLocalShortcut, KAction::CustomShortcut );
		if ( m_newGlobalShortcut != m_action->globalShortcut() )
			m_action->setGlobalShortcut( m_newGlobalShortcut, KAction::CustomShortcut );
	}
}

////

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
KKeyDialog::KKeyDialog( KKeyChooser::ActionTypes types, KKeyChooser::LetterShortcuts allowLetterShortcuts, QWidget *parent )
: KDialog( parent, i18n("Configure Shortcuts"), Default|Ok|Cancel )
{
	setModal(true);
	m_keyChooser = new KKeyChooser( this, types, allowLetterShortcuts );
	setMainWidget( m_keyChooser );
	connect( this, SIGNAL(defaultClicked()), m_keyChooser, SLOT(allDefault()) );

	KConfigGroup group( KGlobal::config(), "KKeyDialog Settings" );
	resize( group.readEntry( "Dialog Size", size() ) );
}

KKeyDialog::~KKeyDialog()
{
	KConfigGroup group( KGlobal::config(), "KKeyDialog Settings" );
	group.writeEntry( "Dialog Size", size(), KConfigBase::Global );
}

void KKeyDialog::insert(KActionCollection *pColl, const QString &title)
{
	m_keyChooser->insert(pColl, title);
}

bool KKeyDialog::configure( bool bSaveSettings )
{
	int retcode = exec();
	if( retcode == Accepted ) {
		if( bSaveSettings )
			m_keyChooser->save();
		else
			commitChanges();
	}
	return retcode;
}

void KKeyDialog::commitChanges()
{
	m_keyChooser->commitChanges();
}

int KKeyDialog::configure( KActionCollection* coll, KKeyChooser::LetterShortcuts allowLetterShortcuts, QWidget *parent, bool bSaveSettings )
{
	kDebug(125) << "KKeyDialog::configureKeys( KActionCollection*, " << bSaveSettings << " )" << endl;
	KKeyDialog dlg( KKeyChooser::AllActions, allowLetterShortcuts, parent );
	dlg.m_keyChooser->insert( coll );
	return dlg.configure( bSaveSettings );
}

void KKeyChooser::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

void KKeyDialog::virtual_hook( int id, void* data )
{ KDialog::virtual_hook( id, data ); }

const KShortcut& KKeyChooserItem::shortcut( ) const
{
	return m_newLocalShortcut;
}

bool KKeyChooserItem::isConfigurable( ) const
{
	return m_action->isShortcutConfigurable();
}

const KShortcut& KKeyChooserItem::defaultShortcut( ) const
{
	return m_action->defaultShortcut();
}

void KKeyChooser::resizeColumns( )
{
	for (int i = 0; i <  d->ui->list->columnCount(); ++i)
		d->ui->list->resizeColumnToContents(i);
}

void KKeyChooser::showEvent( QShowEvent * event )
{
	QWidget::showEvent(event);

	QTimer::singleShot(0, this, SLOT(resizeColumns()));
}

const KShortcut & KKeyChooserItem::globalShortcut( ) const
{
	return m_newGlobalShortcut;
}

const KShortcut & KKeyChooserItem::defaultGlobalShortcut( ) const
{
	return m_action->defaultGlobalShortcut();
}

void KKeyChooserItem::checkModified( )
{
	m_modified = (m_newLocalShortcut != m_action->shortcut()) || (m_newGlobalShortcut != m_action->globalShortcut());
	// Force redraw
	if (text(0) == "a")
		setText(0, "b");
	else
		setText(0, "a");
}

void KKeyChooserItem::setGlobalShortcut( const KShortcut & cut )
{
	m_newGlobalShortcut = cut;
	checkModified();
}

bool KKeyChooserItem::checkChanged( const KShortcut & cut1, const KShortcut & cut2, int sequence ) const
{
	if (cut1.count() == cut2.count())
		if (sequence < cut1.count())
			return cut1.seq(sequence) != cut2.seq(sequence);
		else
			return false;
	else if (cut1.count() < cut2.count())
		if (sequence < cut1.count())
			return cut1.seq(sequence) != cut2.seq(sequence);
		else
			return false;
	else
		if (sequence < cut2.count())
			return cut1.seq(sequence) != cut2.seq(sequence);
		else
			return true;
}

KAction * KKeyChooserItem::action( ) const
{
	return m_action;
}

bool KKeyChooser::checkGlobalShortcutsConflict( const KShortcut & cut, bool warnUser, QWidget * parent )
{
	return checkGlobalShortcutsConflictInternal(cut, warnUser, parent);
}

#include "kkeydialog.moc"
