/* This file is part of the KDE libraries Copyright (C) 1998 Mark Donohoe <donohoe@kde.org>
    Copyright (C) 1997 Nicolas Hadacek <hadacek@kde.org>
    Copyright (C) 1998 Matthias Ettrich <ettrich@kde.org>
    Copyright (C) 2001 Ellis Whitehead <ellis@kde.org>
    Copyright (C) 2006 Hamish Rodda <rodda@kde.org>
    Copyright (C) 2007 Roberto Raggi <roberto@kdevelop.org>
    Copyright (C) 2007 Andreas Hartmetz <ahartmetz@gmail.com>

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

#include "kshortcutsdialog.h"
#include "kshortcutsdialog_p.h"
#include "kkeysequencewidget.h"

#include "ui_kshortcutsdialog.h"
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


class KShortcutsEditorItem : public QTreeWidgetItem
{
 public:
	KShortcutsEditorItem(QTreeWidgetItem *parent, KAction *action);
	virtual ~KShortcutsEditorItem();
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
// KShortcutsEditorPrivate
//---------------------------------------------------------------------

class KShortcutsEditorPrivate
{
public:
	KShortcutsEditorPrivate(KShortcutsEditor *q): q(q)
	{
	}

	void initGUI( KShortcutsEditor::ActionTypes actionTypes, KShortcutsEditor::LetterShortcuts allowLetterShortcuts );
	void appendToView( uint nList, const QString &title = QString() );

	static void readGlobalKeys( QMap< QString, KShortcut >& map );

	KShortcutsEditorItem *itemFromIndex(const QModelIndex &index);

	QTreeWidgetItem *findOrMakeItem(QTreeWidgetItem *parent, const QString &name);

	//helper functions for conflict resolution
	bool stealShortcut(KShortcutsEditorItem *item, unsigned int column, const QKeySequence &seq);
	bool stealExternalGlobalShortcut(const QString &name, const QKeySequence &seq);
	void wontStealStandardShortcut(KStandardShortcut::StandardShortcut sa, const QKeySequence &seq);
	bool stealShapeGesture(KShortcutsEditorItem *item, const KShapeGesture &gest);
	bool stealRockerGesture(KShortcutsEditorItem *item, const KRockerGesture &gest);

	//these functions do the conflict resolution
	void changeKeyShortcut(KShortcutsEditorItem *item, uint column, const QKeySequence &capture);
	void changeShapeGesture(KShortcutsEditorItem *item, const KShapeGesture &capture);
	void changeRockerGesture(KShortcutsEditorItem *item, const KRockerGesture &capture);

// private slots
	void capturedKeyShortcut(const QKeySequence &);
	void capturedShapeGesture(const KShapeGesture &);
	void capturedRockerGesture(const KRockerGesture &);

	void startEditing(QWidget *, QModelIndex);
	void doneEditingCurrent();

	void globalSettingsChangedSystemwide(int);

// members
	QList<KActionCollection *> actionCollections;
	KShortcutsEditor *q;
	QModelIndex editingIndex;

	Ui::KShortcutsDialog ui;

	bool allowLetterShortcuts;

	KShortcutsEditor::ActionTypes actionTypes;
};


//
//H4X
//
//TODO:make sure that the rest of the layout is top-aligned!
KShortcutsEditorDelegate::KShortcutsEditorDelegate(QAbstractItemView *parent)
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
void KShortcutsEditorDelegate::itemActivated(QModelIndex index)
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

/*bool KShortcutsEditorDelegate::eventFilter(QObject *object, QEvent *event)
{
	return false;
}*/

//---------------------------------------------------------------------
// KShortcutsEditor
//---------------------------------------------------------------------

KShortcutsEditor::KShortcutsEditor(KActionCollection *collection, QWidget *parent, ActionTypes actionType,
                         LetterShortcuts allowLetterShortcuts )
: QWidget( parent )
, d(new KShortcutsEditorPrivate(this))
{
	d->initGUI(actionType, allowLetterShortcuts);
	addCollection(collection);
}


KShortcutsEditor::KShortcutsEditor(QWidget *parent, ActionTypes actionType, LetterShortcuts allowLetterShortcuts)
: QWidget(parent)
, d(new KShortcutsEditorPrivate(this))
{
	d->initGUI(actionType, allowLetterShortcuts);
}


KShortcutsEditor::~KShortcutsEditor()
{
	delete d;
}

void KShortcutsEditor::addCollection(KActionCollection *collection, const QString &title)
{
	d->actionCollections.append(collection);
	enum hierarchyLevel {Root = 0, Program, Group, Action/*unused*/};
	KAction *kact;
	QTreeWidgetItem *hier[3];
	uint l = Program;
	hier[Root] = d->ui.list->invisibleRootItem();
	hier[Program] = d->findOrMakeItem(hier[Root], title.isEmpty() ? i18n("Shortcuts") : title);
	hier[Group] = d->findOrMakeItem(hier[Program], "if you see this, something went wrong");

	foreach (QAction *action, collection->actions()) {
		QString name = action->text();
		kDebug(125) << "Key: " << name << endl;

		if (name.startsWith(QLatin1String("Program:")))
			l = Program;
		else if (name.startsWith(QLatin1String("Group:")))
			l = Group;
		else if ((kact = qobject_cast<KAction *>(action)) && kact->isShortcutConfigurable()) {
			new KShortcutsEditorItem((hier[l]), kact);
			continue;
		}
		
		if (!hier[l]->childCount())
			delete hier[l];

		hier[l] = d->findOrMakeItem(hier[l - 1], name);
	}

	for (l = Group; l >= Program; l--) {
		if (!hier[l]->childCount())
			delete hier[l];
	}
}


void KShortcutsEditor::save()
{
	foreach (KActionCollection* collection, d->actionCollections)
		collection->writeSettings();
	if (!KGlobalAccel::self()->actionsWithGlobalShortcut().isEmpty())
		KGlobalAccel::self()->writeSettings();
}


void KShortcutsEditor::undoChanges()
{
}


void KShortcutsEditorPrivate::initGUI( KShortcutsEditor::ActionTypes types, KShortcutsEditor::LetterShortcuts allowLetterShortcuts )
{
	actionTypes = types;
	this->allowLetterShortcuts = (allowLetterShortcuts == KShortcutsEditor::LetterShortcutsAllowed);

	ui.setupUi(q);
	//ui.searchFilter->searchLine()->setTreeWidget(ui.list); // Plug into search line
	ui.list->header()->setStretchLastSection(false);
	if (!(actionTypes & KShortcutsEditor::GlobalAction)) {
		ui.list->header()->hideSection(3);
	} else if (!(actionTypes & ~KShortcutsEditor::GlobalAction)) {
		ui.list->header()->hideSection(1);
		ui.list->header()->hideSection(2);
	}

	KShortcutsEditorDelegate *delegate = new KShortcutsEditorDelegate(ui.list);
	ui.list->setItemDelegate(delegate);
	ui.list->setSelectionBehavior(QAbstractItemView::SelectItems);
	ui.list->setSelectionMode(QAbstractItemView::SingleSelection);
	//we have our own editing mechanism
	ui.list->setEditTriggers(QAbstractItemView::NoEditTriggers);

	QObject::connect(KGlobalSettings::self(), SIGNAL(settingsChanged(int)),
	                 q, SLOT(globalSettingsChangedSystemwide(int)));
	QObject::connect(delegate, SIGNAL(extenderCreated(QWidget *, QModelIndex)),
	                 q, SLOT(startEditing(QWidget *, QModelIndex)));	
	QObject::connect(delegate, SIGNAL(extenderDestroyed(QWidget *, QModelIndex)),
	                 q, SLOT(doneEditingCurrent()));
}


// Look, Trolltech! You made QTreeWidget::itemFromIndex protected but I'll do it anyway.
// NOTE: there is no official guarantee that this pointer will stay valid during anything.
KShortcutsEditorItem *KShortcutsEditorPrivate::itemFromIndex(const QModelIndex &index)
{
	//don't tell anyone please!
	return reinterpret_cast<KShortcutsEditorItem *>(ui.list->model()->data(index, ItemPointerRole).toULongLong());
}


QTreeWidgetItem *KShortcutsEditorPrivate::findOrMakeItem(QTreeWidgetItem *parent, const QString &name)
{
	for (int i = 0; i < parent->childCount(); i++) {
		QTreeWidgetItem *child = parent->child(i);
		if (child->text(0) == name)
			return child;
	}
	QTreeWidgetItem *ret = new QTreeWidgetItem(parent);
	ret->setText(0, QString(name).remove('&'));
	ui.list->expandItem(ret);
	ret->setFlags(ret->flags() & ~Qt::ItemIsSelectable);
	return ret;
}


//slot
void KShortcutsEditorPrivate::startEditing(QWidget *editor, QModelIndex index)
{
	editingIndex = index;
}


//slot
void KShortcutsEditorPrivate::doneEditingCurrent()
{
	editingIndex = QModelIndex();
}


//slot
void KShortcutsEditorPrivate::capturedKeyShortcut(const QKeySequence &capture)
{
	//TODO: make sure letter shortcuts only go in if allowed. modify KKeyButton.
	if (!editingIndex.isValid())
		return;

	KShortcutsEditorItem *item = itemFromIndex(editingIndex);
	int column = editingIndex.column();
	changeKeyShortcut(item, column, capture);
}


void KShortcutsEditorPrivate::changeKeyShortcut(KShortcutsEditorItem *item, uint column, const QKeySequence &capture)
{
	if (capture == item->keySequence(column))
		return;

	if (!capture.isEmpty()) {
		bool conflict = false;
		unsigned int i;
		KShortcutsEditorItem *otherItem;

		//refuse to assign a global shortcut occupied by a standard shortcut
		if (column == GlobalPrimary || column == GlobalAlternate) {
			KStandardShortcut::StandardShortcut ssc = KStandardShortcut::find(capture);
			if (ssc != KStandardShortcut::AccelNone) {
				wontStealStandardShortcut(ssc, capture);
				return;
			}
		}

		//find conflicting shortcuts in this application
		for (QTreeWidgetItemIterator it(ui.list); (*it); ++it) {
			if ((*it)->childCount())
				continue;

			otherItem = static_cast<KShortcutsEditorItem *>(*it);

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

	item->setKeySequence(column, capture);
	//update global configuration to reflect our changes
	//TODO:: much better to do this in setKeySequence
	if (column == GlobalPrimary || column == GlobalAlternate) {

	}
}


//slot
void KShortcutsEditorPrivate::capturedShapeGesture(const KShapeGesture &capture)
{
	if (!editingIndex.isValid())
		return;

	changeShapeGesture(itemFromIndex(editingIndex), capture);
}


void KShortcutsEditorPrivate::changeShapeGesture(KShortcutsEditorItem *item, const KShapeGesture &capture)
{
	if (capture == item->m_action->shapeGesture())
		return;

	if (capture.isValid()) {
		bool conflict = false;
		KShortcutsEditorItem *otherItem;

		//search for conflicts
		for (QTreeWidgetItemIterator it(ui.list); (*it); ++it) {
			if (!(*it)->parent())
				continue;

			otherItem = static_cast<KShortcutsEditorItem *>(*it);

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

	item->setShapeGesture(capture);
}


//slot
void KShortcutsEditorPrivate::capturedRockerGesture(const KRockerGesture &capture)
{
	if (!editingIndex.isValid())
		return;

	changeRockerGesture(itemFromIndex(editingIndex), capture);
}


void KShortcutsEditorPrivate::changeRockerGesture(KShortcutsEditorItem *item, const KRockerGesture &capture)
{
	if (capture == item->m_action->rockerGesture())
		return;

	if (capture.isValid()) {
		bool conflict = false;
		KShortcutsEditorItem *otherItem;

		for (QTreeWidgetItemIterator it(ui.list); (*it); ++it) {
			if (!(*it)->parent())
				continue;

			otherItem = static_cast<KShortcutsEditorItem *>(*it);

			if (capture == otherItem->m_action->rockerGesture()) {
				conflict = true;
				break;
			}
		}

		if (conflict && !stealRockerGesture(otherItem, capture))
			return;
	}

	item->setRockerGesture(capture);
}


bool KShortcutsEditorPrivate::stealShortcut(KShortcutsEditorItem *item, unsigned int column, const QKeySequence &seq)
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


bool KShortcutsEditorPrivate::stealExternalGlobalShortcut(const QString &name, const QKeySequence &seq)
{
	if (KGlobalAccel::promptStealShortcutSystemwide(q, name, seq)) {
		KGlobalAccel::stealShortcutSystemwide(name, seq);
		return true;
	} else
		return false;
}


void KShortcutsEditorPrivate::wontStealStandardShortcut(KStandardShortcut::StandardShortcut std, const QKeySequence &seq)
{
	QString title = i18n("Conflict with Standard Application Shortcut");
	QString message = i18n("The '%1' key combination has already been allocated to the standard action "
	                       "\"%2\"that many applications use.\n"
	                       "You cannot use it for global shortcuts for this reason.",
	                       seq.toString(), KStandardShortcut::name(std));

	KMessageBox::sorry(q, message, title);
}


bool KShortcutsEditorPrivate::stealShapeGesture(KShortcutsEditorItem *item, const KShapeGesture &gst)
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


bool KShortcutsEditorPrivate::stealRockerGesture(KShortcutsEditorItem *item, const KRockerGesture &gst)
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


//slot
void KShortcutsEditorPrivate::globalSettingsChangedSystemwide(int which)
{
	KGlobalAccel::self()->readSettings();
	//TODO:do something about it, too?
}


//slot
void KShortcutsEditor::allDefault()
{
	for (QTreeWidgetItemIterator it(d->ui.list); (*it); ++it) {
		if (!(*it)->parent())
			continue;

		KShortcutsEditorItem *item = static_cast<KShortcutsEditorItem *>(*it);
		KAction *act = item->m_action;
		
		if (act->shortcut() != act->shortcut(KAction::DefaultShortcut)) {
			d->changeKeyShortcut(item, LocalPrimary, act->shortcut(KAction::DefaultShortcut).primary());
			d->changeKeyShortcut(item, LocalAlternate, act->shortcut(KAction::DefaultShortcut).alternate());
		}
		
		if (act->globalShortcut() != act->globalShortcut(KAction::DefaultShortcut)) {
			d->changeKeyShortcut(item, GlobalPrimary, act->globalShortcut(KAction::DefaultShortcut).primary());
			d->changeKeyShortcut(item, GlobalAlternate, act->globalShortcut(KAction::DefaultShortcut).alternate());
		}
		
		if (act->shapeGesture() != act->shapeGesture(KAction::DefaultShortcut))
			d->changeShapeGesture(item, act->shapeGesture(KAction::DefaultShortcut));
		
		if (act->rockerGesture() != act->rockerGesture(KAction::DefaultShortcut))
			d->changeRockerGesture(item, act->rockerGesture(KAction::DefaultShortcut));
	}
}


//slot
void KShortcutsEditor::resizeColumns()
{
	for (int i = 0; i < d->ui.list->columnCount(); i++)
		d->ui.list->resizeColumnToContents(i);
}


void KShortcutsEditor::showEvent( QShowEvent * event )
{
	QWidget::showEvent(event);
	QTimer::singleShot(0, this, SLOT(resizeColumns()));
}


//---------------------------------------------------
KShortcutsEditorItem::KShortcutsEditorItem(QTreeWidgetItem *parent, KAction *action)
	: QTreeWidgetItem(parent)
	, m_action(action)
	, m_oldLocalShortcut(0)
	, m_oldGlobalShortcut(0)
	, m_oldShapeGesture(0)
	, m_oldRockerGesture(0)
{
}


KShortcutsEditorItem::~KShortcutsEditorItem()
{
	delete m_oldLocalShortcut;
	delete m_oldGlobalShortcut;
	delete m_oldShapeGesture;
	delete m_oldRockerGesture;
}


QVariant KShortcutsEditorItem::data(int column, int role) const
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


QKeySequence KShortcutsEditorItem::keySequence(uint column) const
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


void KShortcutsEditorItem::setKeySequence(uint column, const QKeySequence &seq)
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


void KShortcutsEditorItem::setShapeGesture(const KShapeGesture &gst)
{
	if (!m_oldShapeGesture) {
		m_oldShapeGesture = new KShapeGesture(gst);
	}
	m_action->setShapeGesture(gst);
	updateModified();
}


void KShortcutsEditorItem::setRockerGesture(const KRockerGesture &gst)
{
	if (!m_oldRockerGesture) {
		m_oldRockerGesture = new KRockerGesture(gst);
	}
	m_action->setRockerGesture(gst);
	updateModified();
}


//our definition of modified is "modified since the chooser was shown".
void KShortcutsEditorItem::updateModified()
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


bool KShortcutsEditorItem::isModified(uint column) const
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


void KShortcutsEditorItem::undoChanges()
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
/* KShortcutsDialog                                                           */
/*                                                                      */
/* Originally by Nicolas Hadacek <hadacek@via.ecp.fr>                   */
/*                                                                      */
/* Substantially revised by Mark Donohoe <donohoe@kde.org>              */
/*                                                                      */
/* And by Espen Sand <espen@kde.org> 1999-10-19                         */
/* (by using KDialog there is almost no code left ;)                    */
/*                                                                      */
/************************************************************************/

class KShortcutsDialog::KShortcutsDialogPrivate
{
public:
  KShortcutsDialogPrivate(KShortcutsDialog *q): q(q), m_keyChooser(0) {}

  KShortcutsDialog *q;
  KShortcutsEditor* m_keyChooser; // ### move
};


KShortcutsDialog::KShortcutsDialog( KShortcutsEditor::ActionTypes types, KShortcutsEditor::LetterShortcuts allowLetterShortcuts, QWidget *parent )
: KDialog( parent ), d(new KShortcutsDialogPrivate(this))
{
	setCaption(i18n("Configure Shortcuts"));
	setButtons(Default|Ok|Cancel);
	setModal(true);
	d->m_keyChooser = new KShortcutsEditor( this, types, allowLetterShortcuts );
	setMainWidget( d->m_keyChooser );
	connect( this, SIGNAL(defaultClicked()), d->m_keyChooser, SLOT(allDefault()) );

	KConfigGroup group( KGlobal::config(), "KShortcutsDialog Settings" );
	resize( group.readEntry( "Dialog Size", sizeHint() ) );
}


KShortcutsDialog::~KShortcutsDialog()
{
	KConfigGroup group( KGlobal::config(), "KShortcutsDialog Settings" );
	group.writeEntry( "Dialog Size", size(), KConfigBase::Global );
	delete d;
}


void KShortcutsDialog::addCollection(KActionCollection *collection, const QString &title)
{
	d->m_keyChooser->addCollection(collection, title);
}


bool KShortcutsDialog::configure(bool saveSettings)
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


/*void KShortcutsDialog::commitChanges()
{
	d->m_keyChooser->commitChanges();
}*/


int KShortcutsDialog::configure(KActionCollection *collection, KShortcutsEditor::LetterShortcuts allowLetterShortcuts,
                          QWidget *parent, bool saveSettings)
{
	kDebug(125) << "KShortcutsDialog::configureKeys( KActionCollection*, " << saveSettings << " )" << endl;
	KShortcutsDialog dlg(KShortcutsEditor::AllActions, allowLetterShortcuts, parent);
	dlg.d->m_keyChooser->addCollection(collection);
	return dlg.configure(saveSettings);
}

#include "kshortcutsdialog.moc"
#include "kshortcutsdialog_p.moc"
