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
//#include "ui_kshortcuteditor.h"

#include <string.h>

#include <QtDebug>
#include <QTreeWidgetItem>
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
#include "kgesture.h"

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


enum ColumnDesignation {
	Name = 0,
	LocalPrimary,
	LocalAlternate,
	GlobalPrimary,
	GlobalAlternate,
	RockerGesture,
	ShapeGesture
};


enum reallyEvilRole {
	ItemPointerRole = Qt::UserRole + 2342
};


class KKeyChooserItem : public QTreeWidgetItem
{
 public:
	KKeyChooserItem(QTreeWidget *parent, QTreeWidgetItem *after, KAction *action);
	KKeyChooserItem(QTreeWidgetItem *parent, QTreeWidgetItem *after, KAction *action);
	virtual ~KKeyChooserItem();
	void undoChanges();

	virtual QVariant data(int column, int role) const;

	QKeySequence keySequence(uint column) const;
	void setKeySequence(uint column, const QKeySequence &seq);
	void setShapeGesture(const KShapeGesture &gst);
	void setRockerGesture(const KRockerGesture &gst);

	bool isModified(uint column) const;

	KAction *m_action;
private:
	//recheck modified status - could have changed back to initial value
	void updateModified();
	//a null pointer means "not modified"
	KShortcut *m_oldLocalShortcut;
	KShortcut *m_oldGlobalShortcut;
	KShapeGesture *m_oldShapeGesture;
	KRockerGesture *m_oldRockerGesture;
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

	void initGUI( KKeyChooser::ActionTypes actionTypes, KKeyChooser::LetterShortcuts allowLetterShortcuts );
	void appendToView( uint nList, const QString &title = QString() );

	static void readGlobalKeys( QMap< QString, KShortcut >& map );

	KKeyChooserItem *itemFromIndex(const QModelIndex &index);

	bool stealShortcut(KKeyChooserItem *item, unsigned int column, const QKeySequence &seq);
	bool stealExternalGlobalShortcut(const QString &name, const QKeySequence &seq);
	void wontStealStandardShortcut(KStandardShortcut::StandardShortcut sa, const QKeySequence &seq);
	bool stealShapeGesture(KKeyChooserItem *item, const KShapeGesture &gest);
	bool stealRockerGesture(KKeyChooserItem *item, const KRockerGesture &gest);

// private slots
	void capturedKeyShortcut(QKeySequence);
	void capturedShapeGesture(KShapeGesture);
	void capturedRockerGesture(KRockerGesture);

	void startEditing(QWidget *, QModelIndex);
	void doneEditingCurrent();

	void globalShortcutsChangedSystemwide();

// members
	KKeyChooser *q;
	QModelIndex currentlyEditingIndex;
	//QList< QList<QAction*> > actionLists;
	QList<KActionCollection *> actionCollections;

	Ui::KKeyDialog ui;

	bool allowLetterShortcuts;

	KKeyChooser::ActionTypes actionTypes;
};


//
//H4X
//
//TODO:make sure that the rest of the layout is top-aligned!
KKeyChooserDelegate::KKeyChooserDelegate(QAbstractItemView *parent)
 : KExtendableItemDelegate(parent)
{
	Q_ASSERT(qobject_cast<QAbstractItemView *>(parent));
	//connect(this, SIGNAL(editInstead(QModelIndex)), parent, SLOT(setCurrentIndex(QModelIndex)));
	//connect(this, SIGNAL(editInstead(QModelIndex)), parent, SLOT(edit(QModelIndex)));
	//HNOTEDITOR
	connect(parent, SIGNAL(clicked(QModelIndex)), this, SLOT(itemActivated(QModelIndex)));
	//connect(this, SIGNAL(editInstead(QModelIndex)), parent, SLOT(edit(QModelIndex)));
	//HNOTEDITOR/
}


//#include "kshapegestureselector.h"
//slot
void KKeyChooserDelegate::itemActivated(QModelIndex index)
{
	kDebug() << "itemActivated" <<endl;
	//TODO: here, we only want maximum ONE extender open at any time.

	//testing hack
	if (index.column() == 2) {
		//KShapeGestureSelector *ksgs = new KShapeGestureSelector(0);
		//ksgs->exec();
	}
	if (index.column() == 0) {
		const QAbstractItemModel *model = index.model();
		index = model->index(index.row(), 1, index.parent());
	}
	if (!isExtended(index)) {
		// //TODO:fix the leak!!! (make a "real" KShortcutEditor widget)
		// Ui::KShortcutEditor *kseui = new Ui::KShortcutEditor;
		// QWidget *editor = new QWidget(static_cast<QAbstractItemView*>(parent())->viewport());
		// kseui->setupUi(editor);
		// extendItem(editor, index);
	} else {
		contractItem(index);
	}
}

/*bool KKeyChooserDelegate::eventFilter(QObject *object, QEvent *event)
{
	return false;
}*/

//---------------------------------------------------------------------
// KKeyChooser
//---------------------------------------------------------------------

KKeyChooser::KKeyChooser(KActionCollection *collection, QWidget *parent, ActionTypes actionType,
                         LetterShortcuts allowLetterShortcuts )
: QWidget( parent )
, d(new KKeyChooserPrivate(this))
{
	d->initGUI(actionType, allowLetterShortcuts);
	addCollection(collection);
}


KKeyChooser::KKeyChooser(QWidget *parent, ActionTypes actionType, LetterShortcuts allowLetterShortcuts)
: QWidget(parent)
, d(new KKeyChooserPrivate(this))
{
	d->initGUI(actionType, allowLetterShortcuts);
}


KKeyChooser::~KKeyChooser()
{
	delete d;
}

void KKeyChooser::addCollection(KActionCollection *collection, const QString &title)
{
	//TODO:implement
	return;
}

/*bool KKeyChooser::insert( KActionCollection* pColl, const QString &title )
{
    QString str = title;
    if (title.isEmpty() && pColl->componentData().isValid() && pColl->componentData().aboutData()) {
        str = pColl->componentData().aboutData()->programName();
    }

	d->actionLists.append( pColl->actions() );
	d->actionCollections.append( pColl );
	d->appendToView(d->actionLists.count() - 1, str);
}*/
    

void KKeyChooser::save()
{
	foreach (KActionCollection* collection, d->actionCollections)
		collection->writeSettings();
	if (!KGlobalAccel::self()->actionsWithGlobalShortcut().isEmpty())
		KGlobalAccel::self()->writeSettings();
}


void KKeyChooserPrivate::initGUI( KKeyChooser::ActionTypes types, KKeyChooser::LetterShortcuts allowLetterShortcuts )
{
	actionTypes = types;
	this->allowLetterShortcuts = (allowLetterShortcuts == KKeyChooser::LetterShortcutsAllowed);

	ui.setupUi(q);
	//ui.searchFilter->searchLine()->setTreeWidget(ui.list); // Plug into search line
	ui.list->header()->setStretchLastSection(false);
	if (!(actionTypes & KKeyChooser::GlobalAction)) {
		ui.list->header()->hideSection(3);
	} else if (!(actionTypes & ~KKeyChooser::GlobalAction)) {
		ui.list->header()->hideSection(1);
		ui.list->header()->hideSection(2);
	}

	KKeyChooserDelegate *delegate = new KKeyChooserDelegate(ui.list);
	ui.list->setItemDelegate(delegate);
	ui.list->setSelectionBehavior(QAbstractItemView::SelectItems);
	ui.list->setSelectionMode(QAbstractItemView::SingleSelection);
	//we have our own editing mechanism
	ui.list->setEditTriggers(QAbstractItemView::NoEditTriggers);

	QObject::connect(KGlobalSettings::self(), SIGNAL(settingsChanged(int)),
	                 q, SLOT(globalSettingsChanged(int)));
	QObject::connect(delegate, SIGNAL(extenderCreated(QWidget *, QModelIndex)),
	                 q, SLOT(startEditing(QWidget *, QModelIndex)));
	QObject::connect(delegate, SIGNAL(extenderDestroyed(QWidget *, QModelIndex)),
	                 q, SLOT(doneEditingCurrent()));
}

// Add all shortcuts to the list
void KKeyChooserPrivate::appendToView( uint nList, const QString &title )
{
/*	const QList<QAction*> pList = actionLists[nList];

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

		if (!(thisType & actionTypes))
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
		//H4X
		pItem->setFlags(pItem->flags() | Qt::ItemIsEditable);
	}

	if( !pProgramItem->childCount() )
		delete pProgramItem;
	if( pGroupItem && !pGroupItem->childCount() )
		delete pGroupItem;*/
}


// Look, Trolltech! You made QTreeWidget::itemFromIndex protected but I'll do it anyway.
// NOTE: there is no official guarantee that this pointer will stay valid during anything.
KKeyChooserItem *KKeyChooserPrivate::itemFromIndex(const QModelIndex &index)
{
	//don't tell anyone please!
	return reinterpret_cast<KKeyChooserItem *>(ui.list->model()->data(index, ItemPointerRole).toULongLong());
}


//slot
void KKeyChooserPrivate::startEditing(QWidget *editor, QModelIndex index)
{
	currentlyEditingIndex = index;
}

//slot
void KKeyChooserPrivate::doneEditingCurrent()
{
	currentlyEditingIndex = QModelIndex();
}


//slot
void KKeyChooserPrivate::capturedKeyShortcut(QKeySequence capture)
{
	if (!currentlyEditingIndex.isValid())
		return;

	int editingColumn = currentlyEditingIndex.column();

	if (capture == itemFromIndex(currentlyEditingIndex)->keySequence(editingColumn))
		return;

	//TODO: make sure letter shortcuts only go in if allowed. modify KKeyButton.

	if (!capture.isEmpty()) {
		bool conflict = false;
		unsigned int i;
		KKeyChooserItem *otherItem;

		//refuse to assign a global shortcut occupied by a standard shortcut
		if (editingColumn == GlobalPrimary || editingColumn == GlobalAlternate) {
			KStandardShortcut::StandardShortcut ssc = KStandardShortcut::find(capture);
			if (ssc != KStandardShortcut::AccelNone) {
				wontStealStandardShortcut(ssc, capture);
				return;
			}
		}

		//find conflicting shortcuts in this application
		for (QTreeWidgetItemIterator it(ui.list); (*it); ++it) {
			if (!(*it)->parent())
				continue;
	
			otherItem = static_cast<KKeyChooserItem *>(*it);
	
			for (i = LocalPrimary; i <= GlobalAlternate; i++)
				if (capture == otherItem->keySequence(i)) {
					conflict = true;
					goto out;
				}
		}
		out:

		if (conflict && !stealShortcut(otherItem, i, capture))
			return;
	
		//check for conflicts with other applications' global shortcuts
		QString conflicting = KGlobalAccel::findActionNameSystemwide(capture);
		if (!conflicting.isEmpty() && !stealExternalGlobalShortcut(conflicting, capture))
			return;
	}

	itemFromIndex(currentlyEditingIndex)->setKeySequence(editingColumn - LocalPrimary, capture);
	//update global configuration to reflect our changes
	//TODO:: much better to do this in setKeySequence
	if (editingColumn == GlobalPrimary || editingColumn == GlobalAlternate) {

	}
}


//slot
void KKeyChooserPrivate::capturedShapeGesture(KShapeGesture capture)
{
	if (!currentlyEditingIndex.isValid())
		return;

	if (capture.isValid()) {
		bool conflict = false;
		KKeyChooserItem *otherItem;

		//search for conflicts
		for (QTreeWidgetItemIterator it(ui.list); (*it); ++it) {
			if (!(*it)->parent())
				continue;

			otherItem = static_cast<KKeyChooserItem *>(*it);

			//comparisons are possibly expensive
			if (!otherItem->m_action->shapeGesture().isValid())
				continue;

			if (capture == otherItem->m_action->shapeGesture()) {
				conflict = true;
				break;
			}
		}

		if (conflict && !stealShapeGesture(otherItem, capture))
			return;
	}

	itemFromIndex(currentlyEditingIndex)->setShapeGesture(capture);
}


//slot
void KKeyChooserPrivate::capturedRockerGesture(KRockerGesture capture)
{
	if (!currentlyEditingIndex.isValid())
		return;

	if (capture.isValid()) {
		bool conflict = false;
		KKeyChooserItem *otherItem;
	
		for (QTreeWidgetItemIterator it(ui.list); (*it); ++it) {
			if (!(*it)->parent())
				continue;

			otherItem = static_cast<KKeyChooserItem *>(*it);
	
			if (capture == otherItem->m_action->rockerGesture()) {
				conflict = true;
				break;
			}
		}

		if (conflict && !stealRockerGesture(otherItem, capture))
			return;
	}

	itemFromIndex(currentlyEditingIndex)->setRockerGesture(capture);
}


bool KKeyChooserPrivate::stealShortcut(KKeyChooserItem *item, unsigned int column, const QKeySequence &seq)
{
	QString title = i18n("Key Conflict");
	QString message = i18n("The '%1' key combination has already been allocated to the \"%2\" action.\n"
	                       "Do you want to reassign it from that action to the current one?",
	                       seq.toString(), item->m_action->text());

	if (KMessageBox::warningContinueCancel(q, message, title, KGuiItem(i18n("Reassign")))
        != KMessageBox::Continue)
		return false;

	item->setKeySequence(column - LocalPrimary, QKeySequence());
	return true;
}


bool KKeyChooserPrivate::stealExternalGlobalShortcut(const QString &name, const QKeySequence &seq)
{
	if (KGlobalAccel::promptStealShortcutSystemwide(q, name, seq)) {
		KGlobalAccel::stealShortcutSystemwide(name, seq);
		return true;
	 } else
		return false;
}


void KKeyChooserPrivate::wontStealStandardShortcut(KStandardShortcut::StandardShortcut std, const QKeySequence &seq)
{
	QString title = i18n("Conflict with Standard Application Shortcut");
	QString message = i18n("The '%1' key combination has already been allocated to the standard action "
	                       "\"%2\"that many applications use.\n"
	                       "You cannot use it for global shortcuts for this reason.",
	                       seq.toString(), KStandardShortcut::name(std));
	
	KMessageBox::sorry(q, message, title);
}


bool KKeyChooserPrivate::stealShapeGesture(KKeyChooserItem *item, const KShapeGesture &gst)
{
	QString title = i18n("Key Conflict");
	QString message = i18n("The '%1' shape gesture has already been allocated to the \"%2\" action.\n"
	                       "Do you want to reassign it from that action to the current one?",
	                       gst.shapeName(), item->m_action->text());

	if (KMessageBox::warningContinueCancel(q, message, title, KGuiItem(i18n("Reassign")))
        != KMessageBox::Continue)
		return false;

	item->setShapeGesture(KShapeGesture());
	return true;
}


bool KKeyChooserPrivate::stealRockerGesture(KKeyChooserItem *item, const KRockerGesture &gst)
{
	QString title = i18n("Key Conflict");
	QString message = i18n("The '%1' rocker gesture has already been allocated to the \"%2\" action.\n"
	                       "Do you want to reassign it from that action to the current one?",
	                       gst.rockerName(), item->m_action->text());

	if (KMessageBox::warningContinueCancel(q, message, title, KGuiItem(i18n("Reassign")))
        != KMessageBox::Continue)
		return false;

	item->setRockerGesture(KRockerGesture());
	return true;
}


/*void KKeyChooserPrivate::updateButtons()
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
}*/


//TODO: get this called
//slot
void KKeyChooserPrivate::globalShortcutsChangedSystemwide()
{
	KGlobalAccel::self()->readSettings();
	//TODO:do something about it, too?
}


void KKeyChooser::resizeColumns()
{
	for (int i = 0; i < d->ui.list->columnCount(); i++)
		d->ui.list->resizeColumnToContents(i);
}


void KKeyChooser::showEvent( QShowEvent * event )
{
	QWidget::showEvent(event);
	QTimer::singleShot(0, this, SLOT(resizeColumns()));
}


//---------------------------------------------------
KKeyChooserItem::KKeyChooserItem(QTreeWidget* parent, QTreeWidgetItem* after, KAction* action)
	: QTreeWidgetItem(parent, after)
	, m_action(action)
	, m_oldLocalShortcut(0)
	, m_oldGlobalShortcut(0)
	, m_oldShapeGesture(0)
	, m_oldRockerGesture(0)
{
}


KKeyChooserItem::KKeyChooserItem(QTreeWidgetItem* parent, QTreeWidgetItem* after, KAction* action)
	: QTreeWidgetItem(parent, after)
	, m_action(action)
	, m_oldLocalShortcut(0)
	, m_oldGlobalShortcut(0)
	, m_oldShapeGesture(0)
	, m_oldRockerGesture(0)
{
}


KKeyChooserItem::~KKeyChooserItem()
{
	delete m_oldLocalShortcut;
	delete m_oldGlobalShortcut;
	delete m_oldShapeGesture;
	delete m_oldRockerGesture;
}


QVariant KKeyChooserItem::data(int column, int role) const
{
	if (role == Qt::SizeHintRole) {
		return QSize(0, 30);
	}
			
	switch (role) {
	case Qt::DisplayRole:
		switch(column) {
		case Name:
			return m_action->text().remove('&');
		case LocalPrimary:
		case LocalAlternate:
		case GlobalPrimary:
		case GlobalAlternate:
			return keySequence(column);
		case ShapeGesture:
			return m_action->shapeGesture().shapeName();
		case RockerGesture:
			return m_action->rockerGesture().rockerName();
		default:
			break;
		}
		break;
	case Qt::DecorationRole:
		if (column == Name)
			return m_action->icon();
		break;
	case Qt::WhatsThisRole:
		return m_action->whatsThis();
		break;
	case Qt::ToolTipRole:
		return m_action->toolTip();
		break;
	case Qt::ForegroundRole:
		if (isModified(column))
			return QColor(Qt::blue);
		break;
	case Qt::FontRole:
		if (!isModified(column)) {
			break;
		} else {
			QFont modifiedFont = treeWidget()->font();
			modifiedFont.setBold(true);
			return modifiedFont;
			break;
		}
	case KExtendableItemDelegate::ShowExtensionIndicatorRole:
		if (column == Name)
			return false;
		else
			return true;
		break;
	case ItemPointerRole:
		return reinterpret_cast<qulonglong>(this);
	default:
		break;
	}

	return QVariant();
}


QKeySequence KKeyChooserItem::keySequence(uint column) const
{
	switch (column) {
	//"safe" but useless
	default:
	case LocalPrimary:
		return m_action->shortcut().primary();
	case LocalAlternate:
		return m_action->shortcut().alternate();
	case GlobalPrimary:
		return m_action->globalShortcut().primary();
	case GlobalAlternate:
		return m_action->globalShortcut().alternate();
	}
}


void KKeyChooserItem::setKeySequence(uint column, const QKeySequence &seq)
{
	KShortcut ks;
	//TODO: register changes system-wide
	if (column == GlobalPrimary || column == GlobalAlternate) {
		ks = m_action->globalShortcut();
		if (!m_oldGlobalShortcut)
			m_oldGlobalShortcut = new KShortcut(ks);
	 } else {
		ks = m_action->shortcut();
		if (!m_oldLocalShortcut)
			m_oldLocalShortcut = new KShortcut(ks);
	}

	if (column == LocalAlternate || column == GlobalAlternate)
		ks.setAlternate(seq);
	else
		ks.setPrimary(seq);

	if (column == GlobalPrimary || column == GlobalAlternate)
		m_action->setGlobalShortcut(ks);
	else
		m_action->setShortcut(ks);

	updateModified();
}


void KKeyChooserItem::setShapeGesture(const KShapeGesture &gst)
{
	if (!m_oldShapeGesture) {
		m_oldShapeGesture = new KShapeGesture(gst);
	}
	m_action->setShapeGesture(gst);
	updateModified();
}


void KKeyChooserItem::setRockerGesture(const KRockerGesture &gst)
{
	if (!m_oldRockerGesture) {
		m_oldRockerGesture = new KRockerGesture(gst);
	}
	m_action->setRockerGesture(gst);
	updateModified();
}


//our definition of modified is "modified since the chooser was shown".
void KKeyChooserItem::updateModified()
{
	if (m_oldLocalShortcut && *m_oldLocalShortcut == m_action->shortcut()) {
		delete m_oldLocalShortcut;
		m_oldLocalShortcut = 0;
	}
	if (m_oldGlobalShortcut && *m_oldGlobalShortcut == m_action->globalShortcut()) {
		delete m_oldGlobalShortcut;
		m_oldGlobalShortcut = 0;
	}
	if (m_oldShapeGesture && *m_oldShapeGesture == m_action->shapeGesture()) {
		delete m_oldShapeGesture;
		m_oldShapeGesture = 0;
	}
	if (m_oldRockerGesture && *m_oldRockerGesture == m_action->rockerGesture()) {
		delete m_oldShapeGesture;
		m_oldRockerGesture = 0;
	}
}


bool KKeyChooserItem::isModified(uint column) const
{
	switch (column) {
	case Name:
		return false;
	case LocalPrimary:
	case LocalAlternate:
		if (!m_oldLocalShortcut)
			return false;
		if (column == LocalPrimary)
			return m_oldLocalShortcut->primary() == m_action->shortcut().primary();
		else
			return m_oldLocalShortcut->alternate() == m_action->shortcut().alternate();
	case GlobalPrimary:
	case GlobalAlternate:
		if (!m_oldGlobalShortcut)
			return false;
		if (column == GlobalPrimary)
			return m_oldGlobalShortcut->primary() == m_action->globalShortcut().primary();
		else
			return m_oldGlobalShortcut->alternate() == m_action->globalShortcut().alternate();
	case ShapeGesture:
		return static_cast<bool>(m_oldShapeGesture);
	case RockerGesture:
		return static_cast<bool>(m_oldRockerGesture);
	default:
		return false;
	}
}


void KKeyChooserItem::undoChanges()
{
	if (m_oldLocalShortcut)
		m_action->setShortcut(*m_oldLocalShortcut);

	if (m_oldGlobalShortcut)
		m_action->setGlobalShortcut(*m_oldGlobalShortcut);

	if (m_oldShapeGesture)
		m_action->setShapeGesture(*m_oldShapeGesture);

	if (m_oldRockerGesture)
		m_action->setRockerGesture(*m_oldRockerGesture);

	updateModified();
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
	resize( group.readEntry( "Dialog Size", sizeHint() ) );
}


KKeyDialog::~KKeyDialog()
{
	KConfigGroup group( KGlobal::config(), "KKeyDialog Settings" );
	group.writeEntry( "Dialog Size", size(), KConfigBase::Global );
	delete d;
}


void KKeyDialog::addCollection(KActionCollection *collection, const QString &title)
{
	d->m_keyChooser->addCollection(collection, title);
}


bool KKeyDialog::configure(bool saveSettings)
{
	int retcode = exec();
	if( retcode == Accepted ) {
		if(saveSettings)
			d->m_keyChooser->save();
		else
			//commitChanges(); //TODO: needed? review semantics.
			return retcode;
	}
	return retcode;
}


/*void KKeyDialog::commitChanges()
{
	d->m_keyChooser->commitChanges();
}*/


int KKeyDialog::configure(KActionCollection *collection, KKeyChooser::LetterShortcuts allowLetterShortcuts,
                          QWidget *parent, bool saveSettings)
{
	kDebug(125) << "KKeyDialog::configureKeys( KActionCollection*, " << saveSettings << " )" << endl;
	KKeyDialog dlg(KKeyChooser::AllActions, allowLetterShortcuts, parent);
	dlg.d->m_keyChooser->addCollection(collection);
	return dlg.configure(saveSettings);
}

#include "kkeydialog.moc"
