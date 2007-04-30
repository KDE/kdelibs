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

#include <string.h>

#include <QtDebug>
#include <QTreeWidgetItem>
#include <QTimer>
#include <QHeaderView>
#include <QRadioButton>
#include <QLabel>

#include <kaction.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kglobalaccel.h>
#include <kicon.h>
#include "kiconloader.h"
#include <kcomponentdata.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kxmlguifactory.h>
#include <kaboutdata.h>
#include <kshortcut.h>
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


enum ColumnDesignation {
	Name = 0,
	LocalPrimary,
	LocalAlternate,
	GlobalPrimary,
	GlobalAlternate,
	RockerGesture,
	ShapeGesture
};


enum myRoles {
	ShortcutRole = Qt::UserRole,
	DefaultShortcutRole,
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
	void wontStealStandardShortcut(KStandardShortcut::StandardShortcut sa, const QKeySequence &seq);
	bool stealShapeGesture(KShortcutsEditorItem *item, const KShapeGesture &gest);
	bool stealRockerGesture(KShortcutsEditorItem *item, const KRockerGesture &gest);

	//conflict resolution functions
	void changeKeyShortcut(KShortcutsEditorItem *item, uint column, const QKeySequence &capture);
	void changeShapeGesture(KShortcutsEditorItem *item, const KShapeGesture &capture);
	void changeRockerGesture(KShortcutsEditorItem *item, const KRockerGesture &capture);

// private slots
	//this invokes the appropriate conflict resolution function
	void capturedShortcut(QVariant, const QModelIndex &);

	void globalSettingsChangedSystemwide(int);

// members
	QList<KActionCollection *> actionCollections;
	KShortcutsEditor *q;

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
	setExtendIcon(SmallIcon("go-down.png"));
	setContractIcon(SmallIcon("go-up.png"));
	connect(parent, SIGNAL(clicked(QModelIndex)), this, SLOT(itemActivated(QModelIndex)));
}


//#include "kshapegestureselector.h"
//slot
void KShortcutsEditorDelegate::itemActivated(QModelIndex index)
{
	//check if we are dealing with a KShortcutsEditorItem
	if (index.model()->data(index, ItemPointerRole).isNull())
		return;
		
	//TODO: make clicking on the Name column do *exactly* the same thing
	//as clicking on the LocalPrimary column, i.e. select LocalPrimary
	int column = index.column();
	if (column == 0) {
		const QAbstractItemModel *model = index.model();
		index = model->index(index.row(), 1, index.parent());
		column = 1;
	}

	if (!isExtended(index)) {
		//in this case, we only want maximum ONE extender open at any time.
		if (m_editingIndex.isValid())
			contractItem(m_editingIndex);
		
		m_editingIndex = index;
		const QAbstractItemModel *model = index.model();
		QWidget *viewport = static_cast<QAbstractItemView*>(parent())->viewport();
		QWidget *editor;

		if (column >= LocalPrimary && column <= GlobalAlternate) {
			editor = new ShortcutEditWidget(viewport,
			        model->data(index, DefaultShortcutRole).value<QKeySequence>(),
			        model->data(index, ShortcutRole).value<QKeySequence>());

			connect(editor, SIGNAL(keySequenceChanged(const QKeySequence &)),
			        this, SLOT(keySequenceChanged(const QKeySequence &)));

		} else if (column == RockerGesture) {
			editor = new QLabel("A lame placeholder", viewport);

		} else if (column == ShapeGesture) {
			editor = new QLabel("<i>A towel</i>", viewport);

		} else
			return;

		extendItem(editor, index);

	} else {
		contractItem(index);
		m_editingIndex = QModelIndex();
	}
}


//slot
void KShortcutsEditorDelegate::keySequenceChanged(const QKeySequence &seq)
{
	QVariant ret;
	ret.setValue(seq);
	emit shortcutChanged(ret, m_editingIndex);
}


//slot
void KShortcutsEditorDelegate::shapeGestureChanged(const KShapeGesture &gest)
{
	//this is somewhat verbose because the gesture types are not "built in" to QVariant
	QVariant ret;
	ret.setValue(gest);
	emit shortcutChanged(ret, m_editingIndex);
}


//slot
void KShortcutsEditorDelegate::rockerGestureChanged(const KRockerGesture &gest)
{
	QVariant ret;
	ret.setValue(gest);
	emit shortcutChanged(ret, m_editingIndex);
}


ShortcutEditWidget::ShortcutEditWidget(QWidget *viewport, const QKeySequence &defaultSeq,
                                       const QKeySequence &activeSeq)
 : QWidget(viewport),
   m_defaultKeySequence(defaultSeq),
   m_ignoreKeySequenceChanged(false)
{
	QGridLayout *layout = new QGridLayout(this);

	m_defaultRadio = new QRadioButton("Default:", this);
	QString defaultText = defaultSeq.toString();
	if (defaultText.isEmpty())
		defaultText = i18n("None");
	QLabel *defaultLabel = new QLabel(defaultText, this);

	m_customRadio = new QRadioButton("Custom:", this);
	m_customEditor = new KKeySequenceWidget(this);

	if (activeSeq == defaultSeq) {
		m_defaultRadio->setChecked(true);
	} else {
		m_customRadio->setChecked(true);
		m_customEditor->setKeySequence(activeSeq);
	}

	layout->addWidget(m_defaultRadio, 0, 0);
	layout->addWidget(defaultLabel, 0, 1);
	layout->addWidget(m_customRadio, 1, 0);
	layout->addWidget(m_customEditor, 1, 1);
	layout->setColumnStretch(2, 1);
	//layout->addItem(new QSpacerItem(0, 0), 0, 2);
	
	connect(m_defaultRadio, SIGNAL(toggled(bool)),
	        this, SLOT(defaultChecked(bool)));
	connect(m_customEditor, SIGNAL(keySequenceChanged(const QKeySequence &)),
	        this, SLOT(setCustom(const QKeySequence &)));
}


void ShortcutEditWidget::defaultChecked(bool checked)
{
	if (!checked)
		return;

	//avoid a spurious signal
	m_ignoreKeySequenceChanged = true;
	m_customEditor->clearKeySequence();
	m_ignoreKeySequenceChanged = false;
	m_defaultRadio->setChecked(true);
	emit keySequenceChanged(m_defaultKeySequence);
}


void ShortcutEditWidget::setCustom(const QKeySequence &seq)
{
	if (m_ignoreKeySequenceChanged)
		return;
	m_customRadio->setChecked(true);
	emit keySequenceChanged(seq);
}


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
	//if (!KGlobalAccel::self()->actionsWithGlobalShortcut().isEmpty())
	//TODO: only do this when needed
	KGlobalAccel::self()->writeSettings();
}


void KShortcutsEditor::undoChanges()
{
	//TODO: make this crash-proof. The tree widget does not seem to live long enough.
	//Now I know what the list in the old implementation was for. Still, is there a better way?
	for (QTreeWidgetItemIterator it(d->ui.list); (*it); ++it) {
		if ((*it)->childCount())
			continue;

		static_cast<KShortcutsEditorItem *>(*it)->undoChanges();
	}
}


void KShortcutsEditorPrivate::initGUI( KShortcutsEditor::ActionTypes types, KShortcutsEditor::LetterShortcuts allowLetterShortcuts )
{
	actionTypes = types;
	this->allowLetterShortcuts = (allowLetterShortcuts == KShortcutsEditor::LetterShortcutsAllowed);

	ui.setupUi(q);
	ui.searchFilter->searchLine()->setTreeWidget(ui.list); // Plug into search line
	ui.list->header()->setStretchLastSection(false);
	ui.list->header()->hideSection(GlobalAlternate);  //undesirable for user friendlyness...
	if (!(actionTypes & KShortcutsEditor::GlobalAction)) {
		ui.list->header()->hideSection(GlobalPrimary);
	} else if (!(actionTypes & ~KShortcutsEditor::GlobalAction)) {
		ui.list->header()->hideSection(LocalPrimary);
		ui.list->header()->hideSection(LocalAlternate);
	}

	KShortcutsEditorDelegate *delegate = new KShortcutsEditorDelegate(ui.list);
	ui.list->setItemDelegate(delegate);
	ui.list->setSelectionBehavior(QAbstractItemView::SelectItems);
	ui.list->setSelectionMode(QAbstractItemView::SingleSelection);
	//we have our own editing mechanism
	ui.list->setEditTriggers(QAbstractItemView::NoEditTriggers);

	QObject::connect(KGlobalSettings::self(), SIGNAL(settingsChanged(int)),
	                 q, SLOT(globalSettingsChangedSystemwide(int)));
	QObject::connect(delegate, SIGNAL(shortcutChanged(QVariant, const QModelIndex &)),
	                 q, SLOT(capturedShortcut(QVariant, const QModelIndex &)));
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
	ret->setText(0, name);
	ui.list->expandItem(ret);
	ret->setFlags(ret->flags() & ~Qt::ItemIsSelectable);
	return ret;
}


//private slot
void KShortcutsEditorPrivate::capturedShortcut(QVariant newShortcut, const QModelIndex &index)
{
	//dispatch to the right handler
	//TODO: make sure that letter shortcuts only go in if allowed. modify KKeySequenceWidget.
	if (!index.isValid())
		return;
	int column = index.column();
	KShortcutsEditorItem *item = itemFromIndex(index);
	
	if (column >= LocalPrimary && column <= GlobalAlternate)
		changeKeyShortcut(item, column, newShortcut.value<QKeySequence>());
	else if (column == ShapeGesture)
		changeShapeGesture(item, newShortcut.value<KShapeGesture>());
	else if (column == RockerGesture)
		changeRockerGesture(item, newShortcut.value<KRockerGesture>());
}


void KShortcutsEditorPrivate::changeKeyShortcut(KShortcutsEditorItem *item, uint column, const QKeySequence &capture)
{
	if (capture == item->keySequence(column))
		return;

	if (!capture.isEmpty()) {
		unsigned int i = 0;

		//refuse to assign a global shortcut occupied by a standard shortcut
		if (column == GlobalPrimary || column == GlobalAlternate) {
			KStandardShortcut::StandardShortcut ssc = KStandardShortcut::find(capture);
			if (ssc != KStandardShortcut::AccelNone) {
				wontStealStandardShortcut(ssc, capture);
				return;
			}
		}

		//find conflicting shortcuts in this application
        bool conflict = false;
		KShortcutsEditorItem *otherItem = 0;
		for (QTreeWidgetItemIterator it(ui.list); (*it) && !conflict; ++it) {
			if ((*it)->childCount())
				continue;

			otherItem = static_cast<KShortcutsEditorItem *>(*it);

			for (i = LocalPrimary; i <= GlobalAlternate; i++) {
				if (capture == otherItem->keySequence(i)) {
					conflict = true;
					break;
				}
			}
		}

		if (conflict && !stealShortcut(otherItem, i, capture))
			return;

		//check for conflicts with other applications' global shortcuts
		QStringList conflicting = KGlobalAccel::findActionNameSystemwide(capture);
		if (!conflicting.isEmpty()) {
            if (KGlobalAccel::promptStealShortcutSystemwide(0/*TODO:right?*/, conflicting, capture))
				KGlobalAccel::stealShortcutSystemwide(capture);
			else
				return;
        }
	}

	item->setKeySequence(column, capture);
	//force view update
	item->setText(column, capture.toString());
	//update global configuration to reflect our changes
	//TODO:: much better to do this in setKeySequence
	if (column == GlobalPrimary || column == GlobalAlternate) {

	}
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


void KShortcutsEditorPrivate::wontStealStandardShortcut(KStandardShortcut::StandardShortcut std, const QKeySequence &seq)
{
	QString title = i18n("Conflict with Standard Application Shortcut");
	QString message = i18n("The '%1' key combination has already been allocated to the standard action "
	                       "\"%2\" that many applications use.\n"
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
	Q_UNUSED(which)
	KGlobalAccel::self()->readSettings();
	//TODO:do something about it, too?
}


//TODO: undoChanges does not ask about conflicts, is that better?
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
		return QSize(0, 20);
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
		else
			return KIcon();
		break;
	case Qt::WhatsThisRole:
		return m_action->whatsThis();
	case Qt::ToolTipRole:
		return m_action->toolTip();
	case Qt::BackgroundRole:
		//looks not that great, really
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
		}
	case KExtendableItemDelegate::ShowExtensionIndicatorRole:
		if (column == Name)
			return false;
		else
			return true;
//the following are custom roles, defined in this source file only
	case ItemPointerRole:
		return reinterpret_cast<qulonglong>(this);

	case ShortcutRole:
		switch(column) {
		case LocalPrimary:
		case LocalAlternate:
		case GlobalPrimary:
		case GlobalAlternate:
			return keySequence(column);
		case ShapeGesture: { //scoping for "ret"
			QVariant ret;
			ret.setValue(m_action->shapeGesture());
			return ret; }
		case RockerGesture: {
			QVariant ret;
			ret.setValue(m_action->rockerGesture());
			return ret; }
		}

	case DefaultShortcutRole:
		switch(column) {
		case LocalPrimary:
			return m_action->shortcut(KAction::DefaultShortcut).primary();
		case LocalAlternate:
			return m_action->shortcut(KAction::DefaultShortcut).alternate();
		case GlobalPrimary:
			return m_action->globalShortcut(KAction::DefaultShortcut).primary();
		case GlobalAlternate:
			return m_action->globalShortcut(KAction::DefaultShortcut).alternate();
		case ShapeGesture: {
			QVariant ret;
			ret.setValue(m_action->shapeGesture(KAction::DefaultShortcut));
			return ret; }
		case RockerGesture: {
			QVariant ret;
			ret.setValue(m_action->rockerGesture(KAction::DefaultShortcut));
			return ret; }
		}

	default:
		break;
	}

	return QVariant();
}


QKeySequence KShortcutsEditorItem::keySequence(uint column) const
{
	switch (column) {
	case LocalPrimary:
		return m_action->shortcut().primary();
	case LocalAlternate:
		return m_action->shortcut().alternate();
	case GlobalPrimary:
		return m_action->globalShortcut().primary();
	case GlobalAlternate:
		return m_action->globalShortcut().alternate();
	default:
		return QKeySequence();
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

	//avoid also setting the default shortcut - what we are setting here is custom by definition
	if (column == GlobalPrimary || column == GlobalAlternate)
		m_action->setGlobalShortcut(ks, KAction::ActiveShortcut);
	else
		m_action->setShortcut(ks, KAction::ActiveShortcut);

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
			return m_oldLocalShortcut->primary() != m_action->shortcut().primary();
		else
			return m_oldLocalShortcut->alternate() != m_action->shortcut().alternate();
	case GlobalPrimary:
	case GlobalAlternate:
		if (!m_oldGlobalShortcut)
			return false;
		if (column == GlobalPrimary)
			return m_oldGlobalShortcut->primary() != m_action->globalShortcut().primary();
		else
			return m_oldGlobalShortcut->alternate() != m_action->globalShortcut().alternate();
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
	if (retcode != Accepted)
		d->m_keyChooser->undoChanges();
	else if (saveSettings)
		d->m_keyChooser->save();

	return retcode;
}


int KShortcutsDialog::configure(KActionCollection *collection, KShortcutsEditor::LetterShortcuts allowLetterShortcuts,
                          QWidget *parent, bool saveSettings)
{
	kDebug(125) << "KShortcutsDialog::configureKeys( KActionCollection*, " << saveSettings << " )" << endl;
	KShortcutsDialog dlg(KShortcutsEditor::AllActions, allowLetterShortcuts, parent);
	dlg.d->m_keyChooser->addCollection(collection);
	return dlg.configure(saveSettings);
}

#include "kshortcutseditor.moc"
#include "kshortcutsdialog.moc"
#include "kshortcutsdialog_p.moc"
