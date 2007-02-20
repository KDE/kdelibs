/* This file is part of the KDE libraries
    Copyright (C) 1998 Mark Donohoe <donohoe@kde.org>
    Copyright (C) 1997 Nicolas Hadacek <hadacek@kde.org>
    Copyright (C) 1998 Matthias Ettrich <ettrich@kde.org>
    Copyright (C) 2001 Ellis Whitehead <ellis@kde.org>
    Copyright (C) 2006 Hamish Rodda <rodda@kde.org>
    Copyright (C) 2007 Roberto Raggi <roberto@kdevelop.org>

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

#include "ui_kkeydialog.h"

#include <string.h>

#include <QTreeWidgetItem>
#include <QGroupBox>
#include <QTimer>
#include <QHeaderView>

#include <kaction.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kglobalaccel.h>
#include <kicon.h>
#include <kcomponentdata.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kxmlguifactory.h>
#include <kaboutdata.h>
#include <kshortcut.h>
#include <kstaticdeleter.h>
#include <kstandardshortcut.h>
#include <kconfiggroup.h>

#include "kactioncollection.h"

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
	KShortcut shortcut() const;
	bool isConfigurable() const;
	KShortcut defaultShortcut() const;

	void setLocalShortcut( const KShortcut& cut );
	void setGlobalShortcut( const KShortcut& cut );

        KShortcut globalShortcut() const;
	KShortcut defaultGlobalShortcut() const;

	void commitChanges();

	virtual QVariant data(int column, int role) const;

 private:
	void checkModified();

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
 	KKeyChooserPrivate(KKeyChooser *q): q(q)
	{
	}

	enum { NoKey = 1, DefaultKey, CustomKey };
  
	void initGUI( KKeyChooser::ActionTypes type, KKeyChooser::LetterShortcuts allowLetterShortcuts );
	void buildListView( uint iList, const QString &title = QString() );
  
	void updateButtons();
	void setLocalShortcut( const KShortcut& cut );
	void setGlobalShortcut( const KShortcut& cut );
	bool isKeyPresent( const KShortcut& cut, bool warnuser = true );
  
	bool isKeyPresentLocally( const KShortcut& cut, KKeyChooserItem* ignoreItem, bool bWarnUser );
  
	static bool promptForReassign( const QKeySequence& cut, const QString& sAction, KKeyChooser::ActionType action, QWidget* parent );
        
	// Remove the key sequences contained in cut from the standard shortcut @p name
	// which currently has @p origCut as shortcut.
	static void removeStandardShortcut( const QString& name, KKeyChooser* chooser, const KShortcut &cut );
        
	// Remove the key sequences contained in cut from the global shortcut @p name
	// which currently has @p origCut as shortcut.
	static void removeGlobalShortcut( KAction* action, KKeyChooser* chooser, const KShortcut &cut );
        
	static void readGlobalKeys( QMap< QString, KShortcut >& map );
	static bool checkGlobalShortcutsConflictInternal( const KShortcut& cut, bool bWarnUser, QWidget* parent, const QString& ignoreAction = QString() );
        
	// Remove the key sequences contained in cut from this item
	bool removeShortcut( const QString& name, const KShortcut &cut );
  
// private slots
	void slotLocalNoKey();
	void slotLocalDefaultKey();
	void slotLocalCustomKey();
	void slotLocalCapturedShortcut( const KShortcut& cut );
  
	void slotGlobalNoKey();
	void slotGlobalDefaultKey();
	void slotGlobalCustomKey();
	void slotGlobalCapturedShortcut( const KShortcut& cut );
  
	void slotListItemSelected( QTreeWidgetItem *item );
	void slotSettingsChanged( int );
  
	void captureCurrentItem();
        
// members
	KKeyChooser *q;
	QList< QList<QAction*> > rgpLists;
	QList< KActionCollection* > collections;

	Ui::KKeyDialog ui;

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
, d(new KKeyChooserPrivate(this))
{
	d->initGUI( actionType, allowLetterShortcuts );
	insert( coll );
}

KKeyChooser::KKeyChooser( QWidget* parent, ActionTypes actionType, LetterShortcuts allowLetterShortcuts )
: QWidget( parent )
, d(new KKeyChooserPrivate(this))
{
	d->initGUI( actionType, allowLetterShortcuts );
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
    if (title.isEmpty() && pColl->componentData().isValid() && pColl->componentData().aboutData()) {
        str = pColl->componentData().aboutData()->programName();
    }

	d->rgpLists.append( pColl->actions() );
	d->collections.append( pColl );
	d->buildListView(d->rgpLists.count() - 1, str);

	return true;
}

void KKeyChooser::commitChanges()
{
	kDebug(125) << "KKeyChooser::commitChanges()" << endl;

	QTreeWidgetItemIterator it( d->ui.list );
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

void KKeyChooserPrivate::initGUI( KKeyChooser::ActionTypes type, KKeyChooser::LetterShortcuts allowLetterShortcuts )
{
	this->type = type;
	this->allowLetterShortcuts = (allowLetterShortcuts == KKeyChooser::LetterShortcutsAllowed);

	ui.setupUi(q);

	ui.searchFilter->searchLine()->setTreeWidget(ui.list); // Plug into search line

	ui.list->header()->setStretchLastSection(false);
	if (!(type & KKeyChooser::GlobalAction)) {
		ui.list->header()->hideSection(3);

	} else if (!(type & ~KKeyChooser::GlobalAction)) {
		ui.list->header()->hideSection(1);
		ui.list->header()->hideSection(2);
	}

	QObject::connect( ui.list, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)),
			q, SLOT(slotListItemSelected(QTreeWidgetItem*)) );

	// handle activation of an item
	QObject::connect( ui.list, SIGNAL ( itemActivated ( QTreeWidgetItem *, int ) ),
			q, SLOT ( captureCurrentItem()) );

	QObject::connect( ui.localNone, SIGNAL(clicked()), q, SLOT(slotLocalNoKey()) );
	QObject::connect( ui.localDefault, SIGNAL(clicked()), q, SLOT(slotLocalDefaultKey()) );
	QObject::connect( ui.localCustom, SIGNAL(clicked()), q, SLOT(slotLocalCustomKey()) );
	QObject::connect( ui.localCustomSelector, SIGNAL(capturedShortcut(const KShortcut&)), q, SLOT(slotLocalCapturedShortcut(const KShortcut&)) );

	QObject::connect( ui.globalNone, SIGNAL(clicked()), q, SLOT(slotGlobalNoKey()) );
	QObject::connect( ui.globalDefault, SIGNAL(clicked()), q, SLOT(slotGlobalDefaultKey()) );
	QObject::connect( ui.globalCustom, SIGNAL(clicked()), q, SLOT(slotGlobalCustomKey()) );
	QObject::connect( ui.globalCustomSelector, SIGNAL(capturedShortcut(const KShortcut&)), q, SLOT(slotGlobalCapturedShortcut(const KShortcut&)) );

	QObject::connect( KGlobalSettings::self(), SIGNAL( settingsChanged( int )), q, SLOT( slotSettingsChanged( int )));

	if( allChoosers == 0 )
		allChoosers = allChoosersDeleter.setObject( allChoosers, new QList< KKeyChooser* > );
	allChoosers->append( q );
}

// Add all shortcuts to the list
void KKeyChooserPrivate::buildListView( uint iList, const QString &title )
{
	const QList<QAction*> pList = rgpLists[iList];

	QTreeWidgetItem *pProgramItem, *pGroupItem = 0, *pParentItem, *pItem;

	QString str = (title.isEmpty() ? i18n("Shortcuts") : title);
	pParentItem = pProgramItem = pItem = new QTreeWidgetItem( ui.list, QStringList() << str );
	ui.list->expandItem(pParentItem);
	pParentItem->setFlags(pParentItem->flags() & ~Qt::ItemIsSelectable);

	foreach (QAction* action, pList) {
		KKeyChooser::ActionTypes thisType = 0;
		switch (action->shortcutContext()) {
			case Qt::ApplicationShortcut:
				thisType = KKeyChooser::ApplicationAction;
				break;
			case Qt::WindowShortcut:
				thisType = KKeyChooser::WindowAction;
				break;
			case Qt::WidgetShortcut:
				thisType = KKeyChooser::WidgetAction;
				break;
		}

                KAction *kaction = qobject_cast<KAction*>(action);

		if (kaction!=0 && kaction->globalShortcutAllowed())
			thisType &= KKeyChooser::GlobalAction;

		if (!(thisType & type))
			continue;

		QString sName = action->text();
		kDebug(125) << "Key: " << sName << endl;
		if( sName.startsWith( QLatin1String("Program:") ) ) {
			pItem = new QTreeWidgetItem( ui.list, pProgramItem );
			pItem->setText(0, action->text());
			pItem->setFlags(pItem->flags() & ~Qt::ItemIsSelectable);
			ui.list->expandItem(pItem);
			if( !pProgramItem->childCount() )
				delete pProgramItem;
			pProgramItem = pParentItem = pItem;
		} else if( sName.startsWith( QLatin1String("Group:") ) ) {
			pItem = new QTreeWidgetItem( pProgramItem, pParentItem );
			pItem->setText(0, action->text());
			pItem->setFlags(pItem->flags() & ~Qt::ItemIsSelectable);
			ui.list->expandItem(pItem);
			if( pGroupItem && !pGroupItem->childCount() )
				delete pGroupItem;
			pGroupItem = pParentItem = pItem;
		} else if( !sName.isEmpty() && sName != QLatin1String("unnamed") && kaction!=0 && kaction->isShortcutConfigurable() ) {
			pItem = new KKeyChooserItem( pParentItem, pItem, kaction );
		}
	}
	if( !pProgramItem->childCount() )
		delete pProgramItem;
	if( pGroupItem && !pGroupItem->childCount() )
		delete pGroupItem;
}


void KKeyChooserPrivate::updateButtons()
{
	// Hack: Do this incase we still have changeKey() running.
	//  Better would be to capture the mouse pointer so that we can't click
	//   around while we're supposed to be entering a key.
	//  Better yet would be a modal dialog for changeKey()!
	q->releaseKeyboard();
	KKeyChooserItem* pItem = dynamic_cast<KKeyChooserItem*>( ui.list->currentItem() );

	if ( !pItem ) {
		// if nothing is selected -> disable radio boxes
		ui.localGroup->setEnabled( false );
		ui.localCustomSelector->setShortcut( KShortcut() );

		ui.globalGroup->setEnabled( false );
		ui.globalCustomSelector->setShortcut( KShortcut() );

	} else {
		// Set key strings
		QString localKeyStrCfg = pItem->shortcut().toString();
		QString localKeyStrDef = pItem->defaultShortcut().toString();
		QString globalKeyStrCfg = pItem->globalShortcut().toString();
		QString globalKeyStrDef = pItem->defaultGlobalShortcut().toString();

		ui.localCustomSelector->setShortcut( pItem->shortcut() );
		ui.globalCustomSelector->setShortcut( pItem->globalShortcut() );

		ui.localDefault->setText( i18nc("Default (default shortcut)", "De&fault (%1)", localKeyStrDef.isEmpty() ? i18n("none") : localKeyStrDef) );
		ui.globalDefault->setText( i18nc("Default (default shortcut)", "De&fault (%1)" , globalKeyStrDef.isEmpty() ? i18n("none") : globalKeyStrDef) );

		// Select the appropriate radio button.
		if (pItem->shortcut().isEmpty())
			ui.localNone->setChecked( true );
		else if (pItem->shortcut() == pItem->defaultShortcut())
			ui.localDefault->setChecked( true );
		else
			ui.localCustom->setChecked( true );

		if (pItem->globalShortcut().isEmpty())
			ui.globalNone->setChecked( true );
		else if (pItem->globalShortcut() == pItem->defaultGlobalShortcut())
			ui.globalDefault->setChecked( true );
		else
			ui.globalCustom->setChecked( true );

		// Enable buttons if this key is configurable.
		// The 'Default Key' button must also have a default key.
		ui.localGroup->setEnabled( pItem->isConfigurable() );
		ui.localDefault->setEnabled( pItem->isConfigurable() && !pItem->defaultShortcut().isEmpty() );

		ui.globalGroup->setEnabled( pItem->action()->globalShortcutAllowed() );
		ui.globalDefault->setEnabled( pItem->action()->globalShortcutAllowed() && !pItem->defaultGlobalShortcut().isEmpty() );
	}
}

void KKeyChooserPrivate::slotLocalNoKey()
{
	// return if no key is selected
	KKeyChooserItem* pItem = dynamic_cast<KKeyChooserItem*>( ui.list->currentItem() );
	if( pItem ) {
		//kDebug(125) << "no Key" << d->ui.list->currentItem()->text(0) << endl;
		pItem->setLocalShortcut( KShortcut() );
		updateButtons();
		emit q->keyChange();
	}
}

void KKeyChooserPrivate::slotLocalDefaultKey()
{
	// return if no key is selected
	KKeyChooserItem* pItem = dynamic_cast<KKeyChooserItem*>( ui.list->currentItem() );
	if( pItem ) // don't set it directly, check for conflicts
		setLocalShortcut( pItem->defaultShortcut() );
}

void KKeyChooserPrivate::slotLocalCustomKey()
{
	ui.localCustomSelector->captureShortcut();
}

void KKeyChooserPrivate::slotGlobalNoKey()
{
	// return if no key is selected
	KKeyChooserItem* pItem = dynamic_cast<KKeyChooserItem*>( ui.list->currentItem() );
	if( pItem ) {
		//kDebug(125) << "no Key" << d->ui.list->currentItem()->text(0) << endl;
		pItem->setGlobalShortcut( KShortcut() );
		updateButtons();
		emit q->keyChange();
	}
}

void KKeyChooserPrivate::slotGlobalDefaultKey()
{
	// return if no key is selected
	KKeyChooserItem* pItem = dynamic_cast<KKeyChooserItem*>( ui.list->currentItem() );
	if( pItem ) // don't set it directly, check for conflicts
		setGlobalShortcut( pItem->defaultShortcut() );
}

void KKeyChooserPrivate::slotGlobalCustomKey()
{
	ui.globalCustomSelector->captureShortcut();
}

void KKeyChooserPrivate::slotSettingsChanged( int category )
{
    if( category == KGlobalSettings::SETTINGS_SHORTCUTS )
        KGlobalAccel::self()->readSettings();
}

// KDE4 IMHO this shouldn't be here at all - it cannot check whether the default
// shortcut don't conflict with some already changed ones (e.g. global shortcuts).
// Also, I personally find reseting all shortcuts to default (i.e. hardcoded in the app)
// ones after pressing the 'Default' button rather a misfeature.
void KKeyChooser::allDefault()
{
	kDebug(125) << "KKeyChooser::allDefault()" << endl;

	QTreeWidgetItemIterator it( d->ui.list );
	for( ; *it; ++it ) {
		KKeyChooserItem* pItem = dynamic_cast<KKeyChooserItem*>(*it);
		if( pItem ) {
			pItem->setLocalShortcut( pItem->defaultShortcut() );
			pItem->setGlobalShortcut( pItem->defaultGlobalShortcut() );
		}
	}

	d->updateButtons();
	emit keyChange();
}

void KKeyChooserPrivate::slotListItemSelected( QTreeWidgetItem* )
{
	updateButtons();
}

void KKeyChooserPrivate::captureCurrentItem()
{
	KKeyChooserItem* pItem = dynamic_cast<KKeyChooserItem*>( ui.list->currentItem() );
	if( pItem != NULL && pItem->isConfigurable())
		ui.localCustomSelector->captureShortcut ( );
}

void KKeyChooserPrivate::slotLocalCapturedShortcut( const KShortcut& cut )
{
	if( cut.isEmpty() )
		slotLocalNoKey();
	else
		setLocalShortcut( cut );
}

void KKeyChooserPrivate::slotGlobalCapturedShortcut( const KShortcut& cut )
{
	if( cut.isEmpty() )
		slotGlobalNoKey();
	else
		setGlobalShortcut( cut );
}

void KKeyChooserPrivate::setLocalShortcut( const KShortcut& cut )
{
	kDebug(125) << k_funcinfo << cut.toString() << endl;
	KKeyChooserItem* pItem = dynamic_cast<KKeyChooserItem*>(ui.list->currentItem());
	if( !pItem )
		return;

	foreach (const QKeySequence& seq, cut) {
		int key = seq[0];

		if( !allowLetterShortcuts && (key & Qt::KeyboardModifierMask) == 0
		    && key < 0x3000 && QChar(key).isLetterOrNumber() ) {
			QString s = i18n( 	"In order to use the '%1' key as a shortcut, "
						"it must be combined with the "
						"Meta, Alt, Ctrl, and/or Shift keys.", QChar(key));
			KMessageBox::sorry( q, s, i18n("Invalid Shortcut Key") );
			return;
		}
	}

	// If key isn't already in use,
	if( !isKeyPresent( cut ) ) {
		// Set new key code
		pItem->setLocalShortcut( cut );
		// Update display
		updateButtons();
		emit q->keyChange();
	}
}

void KKeyChooserPrivate::setGlobalShortcut( const KShortcut& cut )
{
	kDebug(125) << k_funcinfo << cut.toString() << endl;
	KKeyChooserItem* pItem = dynamic_cast<KKeyChooserItem*>(ui.list->currentItem());
	if( !pItem )
		return;

	foreach (const QKeySequence& seq, cut) {
		if (seq.isEmpty())
			continue;

		int key = seq[0];

		if( !allowLetterShortcuts && (key & Qt::KeyboardModifierMask) == 0
		    && key < 0x3000 && QChar(key).isLetterOrNumber() ) {
			QString s = i18n( 	"In order to use the '%1' key as a shortcut, "
						"it must be combined with the "
						"Meta, Alt, Ctrl, and/or Shift keys." , QChar(key));
			KMessageBox::sorry( q, s, i18n("Invalid Shortcut Key") );
			return;
		}
	}

	// If key isn't already in use,
	if( !isKeyPresent( cut ) ) {
		// Set new key code
		pItem->setGlobalShortcut( cut );
		// Update display
		updateButtons();
		emit q->keyChange();
	}
}

// Returns iSeq index if cut2 has a sequence of equal or higher priority to a sequence in cut.
// else -1
static QKeySequence keyConflict( const KShortcut& cut, const KShortcut& cut2 )
{
/*	QList<QKeySequence> sl = seqList(cut);
	QList<QKeySequence> sl2 = seqList(cut2);
	for( int iSeq = 0; iSeq < sl.count(); iSeq++ ) {
		for( int iSeq2 = 0; iSeq2 < sl2.count(); iSeq2++ ) {
			if( sl[iSeq] == sl2[iSeq2] )
				return sl[iSeq];
		}
	}
	return QKeySequence();*/

//looks stupid but probably runs faster
	const QKeySequence &cPri = cut.primary();
	const QKeySequence &cAlt = cut.alternate();
	const QKeySequence &c2Pri = cut2.primary();
	const QKeySequence &c2Alt = cut2.alternate();
	if (!cPri.isEmpty()) {
		if (cPri == c2Pri || cPri == c2Alt)
			return cPri;
	}
	if (!cAlt.isEmpty()) {
		if (cAlt == c2Pri || cAlt == c2Alt)
			return cAlt;
	}
	return QKeySequence();
}

// Removes the sequences in cut2 from cut1
static void removeFromShortcut( KShortcut & cut1, const KShortcut &cut2 )
{
	cut1.removeAll(cut2.primary());
	cut1.removeAll(cut2.alternate());
}

bool KKeyChooserPrivate::isKeyPresent( const KShortcut& cut, bool bWarnUser )
{
	KKeyChooserItem* pItem = dynamic_cast<KKeyChooserItem*>(ui.list->currentItem());

	if (!pItem) {
		return false;
	}

	bool has_global_chooser = false;
	bool has_standard_chooser = false;
	for( QList< KKeyChooser* >::ConstIterator it = allChoosers->begin();
		 it != allChoosers->end();
		 ++it ) {
		has_global_chooser |= (((*it)->d->type & KKeyChooser::GlobalAction) == KKeyChooser::GlobalAction);
		has_standard_chooser |= (((*it)->d->type & KKeyChooser::WidgetAction) == KKeyChooser::WidgetAction);
	}

	// If editing global shortcuts, check them for conflicts with the stdaccels.
	if( type & KKeyChooser::ApplicationAction || type & KKeyChooser::GlobalAction ) {
		if( !has_standard_chooser ) {
			if( q->checkStandardShortcutsConflict( cut, bWarnUser, q ))
				return true;
		}
	}

	// only check the global keys if one of the keychoosers isn't global
	if( !has_global_chooser ) {
		if( checkGlobalShortcutsConflictInternal( cut, bWarnUser, q,
			(type & KKeyChooser::GlobalAction) ? pItem->actionName() : QString() ))
			return true;
	}

	if( isKeyPresentLocally( cut, pItem, bWarnUser ))
		return true;

	// check also other KKeyChooser's
	for( QList< KKeyChooser* >::ConstIterator it = allChoosers->begin();
		 it != allChoosers->end();
		 ++it ) {
		if( (*it) != q && (*it)->d->isKeyPresentLocally( cut, 0, bWarnUser ))
                	return true;
		}
	return false;
}

bool KKeyChooserPrivate::isKeyPresentLocally( const KShortcut& cut, KKeyChooserItem* ignoreItem, bool bWarnUser )
{
    if ( cut.toString().isEmpty())
        return false;
	// Search for shortcut conflicts with other actions in the
	//  lists we're configuring.
	for( QTreeWidgetItemIterator it( ui.list ); *it; ++it ) {
		KKeyChooserItem* pItem2 = dynamic_cast<KKeyChooserItem*>(*it);
		if( pItem2 && pItem2 != ignoreItem ) {
			QKeySequence conflict = keyConflict( cut, pItem2->shortcut() );
			if( !conflict.isEmpty() ) {
				if( bWarnUser ) {
					if( !promptForReassign( conflict, pItem2->actionName(), KKeyChooser::WindowAction, 0 ))
						return true;
					// else remove the shortcut from it
					KShortcut cut2 = pItem2->shortcut();
					removeFromShortcut(cut2, cut);
					pItem2->setLocalShortcut(cut2);
					updateButtons();
					emit q->keyChange();
				}
			}
			conflict = keyConflict( cut, pItem2->globalShortcut() );
			if( !conflict.isEmpty() ) {
				if( bWarnUser ) {
					if( !promptForReassign( conflict, pItem2->actionName(), KKeyChooser::WindowAction, q ))
						return true;
					// else remove the shortcut from it
					KShortcut cut2 = pItem2->globalShortcut();
					removeFromShortcut(cut2, cut);
					pItem2->setGlobalShortcut(cut2);
					updateButtons();
					emit q->keyChange();
				}
			}
		}
	}
        return false;
}

bool KKeyChooser::checkStandardShortcutsConflict( const KShortcut& cut, bool bWarnUser, QWidget* parent )
{
	// For each key sequence in the shortcut,
	foreach (const QKeySequence& seq, cut) {
		KStandardShortcut::StandardShortcut id = KStandardShortcut::findStandardShortcut( seq );
		if( id != KStandardShortcut::AccelNone
			&& !keyConflict( cut, KStandardShortcut::shortcut( id ) ).isEmpty() ) {
			if( bWarnUser ) {
				if( !KKeyChooserPrivate::promptForReassign( seq, KStandardShortcut::label(id), KKeyChooser::WidgetAction, parent ))
					return true;
				KKeyChooserPrivate::removeStandardShortcut( KStandardShortcut::label(id), dynamic_cast< KKeyChooser* > ( parent ), cut);
			}
		}
	}
	return false;
}

bool KKeyChooserPrivate::checkGlobalShortcutsConflictInternal( const KShortcut& cut, bool bWarnUser, QWidget* parent, const QString& ignoreAction )
{
	foreach (KAction* action, KGlobalAccel::self()->actionsWithGlobalShortcut()) {
		QKeySequence conflict = keyConflict( cut, action->globalShortcut() );
		if( !conflict.isEmpty() ) {
			if( ignoreAction.isEmpty() || action->objectName() != ignoreAction ) {
				if( bWarnUser ) {
					if( !promptForReassign( conflict, action->objectName(), KKeyChooser::GlobalAction, parent ))
						return true;
					removeGlobalShortcut( action, dynamic_cast< KKeyChooser* >( parent ), cut);
				}
			}
		}
	}
	return false;
}

void KKeyChooserPrivate::removeStandardShortcut( const QString& name, KKeyChooser* chooser, const KShortcut &cut )
{
	bool was_in_choosers = false;
	if( allChoosers != NULL ) {
		foreach (KKeyChooser* c, *allChoosers) {
			if( c != chooser && c->d->type & KKeyChooser::WidgetAction ) {
				was_in_choosers |= ( c->d->removeShortcut( name, cut ));
			}
		}
	}

	if( !was_in_choosers ) { // not edited, needs to be changed in config file
		KStandardShortcut::StandardShortcut id = KStandardShortcut::findStandardShortcut( name.toAscii().constData() );
		if (id != KStandardShortcut::AccelNone)
			KStandardShortcut::saveShortcut(id, cut);
		else
			kWarning(125) << k_funcinfo << "Cannot find stdaccel matching name " << name << endl;
	}
}

void KKeyChooserPrivate::removeGlobalShortcut( KAction* action, KKeyChooser* chooser, const KShortcut &cut )
{
	bool was_in_choosers = false;
	if( allChoosers != NULL ) {
		foreach (KKeyChooser* c, *allChoosers) {
			if( c != chooser && c->d->type & KKeyChooser::GlobalAction ) {
				was_in_choosers |= ( c->d->removeShortcut( action->objectName(), cut ));
			}
		}
	}

	if( !was_in_choosers ) { // not edited, needs to be changed in config file
		KGlobalAccel::self()->writeSettings(0L, false, action);
	}
}

bool KKeyChooserPrivate::removeShortcut( const QString& name, const KShortcut &cut )
{
	for( QTreeWidgetItemIterator it( ui.list ); *it; ++it ) {
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
			emit q->keyChange();
			return true;
		}
	}

	return false;
}

bool KKeyChooserPrivate::promptForReassign( const QKeySequence& cut, const QString& sAction, KKeyChooser::ActionType type, QWidget* parent )
{
	if(cut.isEmpty())
		return true;
	QString sTitle;
	QString s;
	if( type == KKeyChooser::WidgetAction ) {
		sTitle = i18n("Conflict with Standard Application Shortcut");
		s = i18n("The '%1' key combination has already been allocated "
		"to the standard action \"%2\".\n"
		"Do you want to reassign it from that action to the current one?",
		cut.toString(), sAction.trimmed());
	}
	else if( type == KKeyChooser::GlobalAction ) {
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

	return KMessageBox::warningContinueCancel( parent, s, sTitle, KGuiItem(i18n("Reassign")) ) == KMessageBox::Continue;
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
					return m_action->text().remove('&');
				case Qt::DecorationRole:
					return m_action->icon();
				case Qt::WhatsThisRole:
					return m_action->whatsThis();
				case Qt::ToolTipRole:
					return m_action->toolTip();
				case Qt::StatusTipRole:
					return m_action->statusTip();
			}
			break;

		case 1:
                    if (role==Qt::DisplayRole)
                        return shortcut().primary();
                    break;
		case 2:
                    if (role==Qt::DisplayRole)
                        return shortcut().alternate();
                    break;

		case 3:
			switch (role) {
				case Qt::DisplayRole:
					if (!m_newGlobalShortcut.isEmpty())
						return m_newGlobalShortcut.toString();
					//H4X
					/*else
						return "glob:none";*/
					/*else
						return globalShortcut().seq(column - 1).toString();*/
					break;
/*				case Qt::FontRole:
					if (checkChanged(m_newGlobalShortcut, m_action->globalShortcut(), 0)) {
						QFont parentFont = treeWidget()->font();
						parentFont.setBold(true);
						return parentFont;
					}
					break;
				case Qt::ForegroundRole:
					if (checkChanged(m_newGlobalShortcut, m_action->globalShortcut(), 0))
						return QColor(Qt::blue);
					break;
				case Qt::BackgroundRole:
					if (!m_action->globalShortcutAllowed())
					return QColor(Qt::gray);*/
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
			m_action->setShortcut( m_newLocalShortcut, KAction::ActiveShortcut );
		if ( m_newGlobalShortcut != m_action->globalShortcut() )
			m_action->setGlobalShortcut( m_newGlobalShortcut, KAction::ActiveShortcut );
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
/* (by using KDialog there is almost no code left ;)                    */
/*                                                                      */
/************************************************************************/

class KKeyDialog::KKeyDialogPrivate
{
public:
  KKeyDialogPrivate(KKeyDialog *q): q(q), m_keyChooser(0) {}
  
  KKeyDialog *q;
  KKeyChooser* m_keyChooser; // ### move
};

KKeyDialog::KKeyDialog( KKeyChooser::ActionTypes types, KKeyChooser::LetterShortcuts allowLetterShortcuts, QWidget *parent )
: KDialog( parent ), d(new KKeyDialogPrivate(this))
{
	setCaption(i18n("Configure Shortcuts"));
	setButtons(Default|Ok|Cancel);
	setModal(true);
	d->m_keyChooser = new KKeyChooser( this, types, allowLetterShortcuts );
	setMainWidget( d->m_keyChooser );
	connect( this, SIGNAL(defaultClicked()), d->m_keyChooser, SLOT(allDefault()) );

	KConfigGroup group( KGlobal::config(), "KKeyDialog Settings" );
	resize( group.readEntry( "Dialog Size", size() ) );
}

KKeyDialog::~KKeyDialog()
{
	KConfigGroup group( KGlobal::config(), "KKeyDialog Settings" );
	group.writeEntry( "Dialog Size", size(), KConfigBase::Global );
	delete d;
}

void KKeyDialog::insert(KActionCollection *pColl, const QString &title)
{
	d->m_keyChooser->insert(pColl, title);
}

bool KKeyDialog::configure( bool bSaveSettings )
{
	int retcode = exec();
	if( retcode == Accepted ) {
		if( bSaveSettings )
			d->m_keyChooser->save();
		else
			commitChanges();
	}
	return retcode;
}

void KKeyDialog::commitChanges()
{
	d->m_keyChooser->commitChanges();
}

int KKeyDialog::configure( KActionCollection* coll, KKeyChooser::LetterShortcuts allowLetterShortcuts, QWidget *parent, bool bSaveSettings )
{
	kDebug(125) << "KKeyDialog::configureKeys( KActionCollection*, " << bSaveSettings << " )" << endl;
	KKeyDialog dlg( KKeyChooser::AllActions, allowLetterShortcuts, parent );
	dlg.d->m_keyChooser->insert( coll );
	return dlg.configure( bSaveSettings );
}


KShortcut KKeyChooserItem::shortcut( ) const
{
	return m_newLocalShortcut;
}

bool KKeyChooserItem::isConfigurable( ) const
{
	return m_action->isShortcutConfigurable();
}

KShortcut KKeyChooserItem::defaultShortcut( ) const
{
	return m_action->shortcut( KAction::DefaultShortcut );
}

void KKeyChooser::resizeColumns( )
{
	for (int i = 0; i <  d->ui.list->columnCount(); ++i)
		d->ui.list->resizeColumnToContents(i);
}

void KKeyChooser::showEvent( QShowEvent * event )
{
	QWidget::showEvent(event);

	QTimer::singleShot(0, this, SLOT(resizeColumns()));
}

KShortcut KKeyChooserItem::globalShortcut( ) const
{
	return m_newGlobalShortcut;
}

KShortcut KKeyChooserItem::defaultGlobalShortcut( ) const
{
	return m_action->globalShortcut( KAction::DefaultShortcut );
}

void KKeyChooserItem::checkModified( )
{
	m_modified = (m_newLocalShortcut != m_action->shortcut()) || (m_newGlobalShortcut != m_action->globalShortcut());

	// why do I still have to use viewport() (with Qt 4.2.0)?
	treeWidget()->viewport()->update();
}

void KKeyChooserItem::setGlobalShortcut( const KShortcut & cut )
{
	m_newGlobalShortcut = cut;
	checkModified();
}

KAction * KKeyChooserItem::action( ) const
{
	return m_action;
}

bool KKeyChooser::checkGlobalShortcutsConflict( const KShortcut & cut, bool warnUser, QWidget * parent )
{
	return KKeyChooserPrivate::checkGlobalShortcutsConflictInternal(cut, warnUser, parent);
}

#include "kkeydialog.moc"
