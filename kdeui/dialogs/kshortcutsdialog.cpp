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

#include <QHeaderView>
#include <QLabel>
#include <QPainter>
#include <QRadioButton>
#include <QtDebug>
#include <QTimer>
#include <QTreeWidgetItem>


#include <kaction.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kglobalaccel.h>
#include <kicon.h>
#include "kiconloader.h"
#include <klocale.h>
#include <kmessagebox.h>
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


enum MyRoles {
	ShortcutRole = Qt::UserRole,
	DefaultShortcutRole
};


enum ItemTypes {
	NonActionItem = 0,
	ActionItem = 1
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
	bool m_isNameBold;
private:
	//recheck modified status - could have changed back to initial value
	void updateModified();
	//a null pointer means "not modified"
	KShortcut *m_oldLocalShortcut;
	KShortcut *m_oldGlobalShortcut;
	KShapeGesture *m_oldShapeGesture;
	KRockerGesture *m_oldRockerGesture;
};

Q_DECLARE_METATYPE(KShortcutsEditorItem *)


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
	//used in appendToView
	QTreeWidgetItem *findOrMakeItem(QTreeWidgetItem *parent, const QString &name);

	static KShortcutsEditorItem *itemFromIndex(QTreeWidget *const w, const QModelIndex &index);

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
	void capturedShortcut(const QVariant &, const QModelIndex &);

	void globalSettingsChangedSystemwide(int);

// members
	QList<KActionCollection *> actionCollections;
	KShortcutsEditor *q;

	Ui::KShortcutsDialog ui;

	KShortcutsEditor::ActionTypes actionTypes;
};


//a gross hack to make a protected method public
class QTreeWidgetHack : public QTreeWidget
{
public:
	QTreeWidgetItem *itemFromIndex(const QModelIndex &index) const
		{ return QTreeWidget::itemFromIndex(index); }
};


KShortcutsEditorDelegate::KShortcutsEditorDelegate(QTreeWidget *parent, bool allowLetterShortcuts)
 : KExtendableItemDelegate(parent),
   m_allowLetterShortcuts(allowLetterShortcuts),
   m_editor(0)
{
	Q_ASSERT(qobject_cast<QAbstractItemView *>(parent));

	QPixmap pixmap( 16, 16 );
	pixmap.fill( QColor( Qt::transparent ) );
	QPainter p( &pixmap );
	QStyleOption option;
	option.rect = pixmap.rect();

	bool isRtl = QApplication::isRightToLeft();
	QApplication::style()->drawPrimitive( isRtl ? QStyle::PE_IndicatorArrowLeft : QStyle::PE_IndicatorArrowRight, &option, &p );
	setExtendIcon( pixmap );

	pixmap.fill( QColor( Qt::transparent ) );
	QApplication::style()->drawPrimitive( QStyle::PE_IndicatorArrowDown, &option, &p );
	setContractIcon( pixmap );

	connect(parent, SIGNAL(activated(QModelIndex)), this, SLOT(itemActivated(QModelIndex)));
}


QSize KShortcutsEditorDelegate::sizeHint(const QStyleOptionViewItem &option,
                                         const QModelIndex &index) const
{
	QSize ret(KExtendableItemDelegate::sizeHint(option, index));
	ret.rheight() += 4;
	return ret;
}


//slot
void KShortcutsEditorDelegate::itemActivated(QModelIndex index)
{
	const QAbstractItemModel *model = index.model();
	if (!model)
		return;
	//As per our constructor our parent *is* a QTreeWidget
	QTreeWidget *view = static_cast<QTreeWidget *>(parent());

	KShortcutsEditorItem *item = KShortcutsEditorPrivate::itemFromIndex(view, index);
	if (!item) {
		//that probably was a non-leaf (type() !=ActionItem) item
		return;
	}

	int column = index.column();
	if (column == Name) {
		index = model->index(index.row(), LocalPrimary, index.parent());
		column = LocalPrimary;
		view->selectionModel()->select(index, QItemSelectionModel::ClearAndSelect);
	}

	if (!isExtended(index)) {
		//we only want maximum ONE extender open at any time.
		if (m_editingIndex.isValid()) {
			QModelIndex idx = model->index(m_editingIndex.row(), Name, m_editingIndex.parent());
			KShortcutsEditorItem *oldItem = KShortcutsEditorPrivate::itemFromIndex(view, idx);
			Q_ASSERT(oldItem); //here we really expect nothing but a real KShortcutsEditorItem

			oldItem->m_isNameBold = false;
			contractItem(m_editingIndex);
		}

		m_editingIndex = index;
		QWidget *viewport = static_cast<QAbstractItemView*>(parent())->viewport();

		if (column >= LocalPrimary && column <= GlobalAlternate) {
			m_editor = new ShortcutEditWidget(viewport,
			          model->data(index, DefaultShortcutRole).value<QKeySequence>(),
			          model->data(index, ShortcutRole).value<QKeySequence>(),
			          m_allowLetterShortcuts);

			connect(m_editor, SIGNAL(keySequenceChanged(const QKeySequence &)),
			        this, SLOT(keySequenceChanged(const QKeySequence &)));

		} else if (column == RockerGesture) {
			m_editor = new QLabel("A lame placeholder", viewport);

		} else if (column == ShapeGesture) {
			m_editor = new QLabel("<i>A towel</i>", viewport);

		} else
			return;

		m_editor->installEventFilter(this);
		item->m_isNameBold = true;
		extendItem(m_editor, index);

	} else {
		//the item is extended, and clicking on it again closes it
		item->m_isNameBold = false;
		contractItem(index);
		view->selectionModel()->select(index, QItemSelectionModel::Clear);
		m_editingIndex = QModelIndex();
		m_editor = 0;
	}
}


//slot
void KShortcutsEditorDelegate::hiddenBySearchLine(QTreeWidgetItem *item, bool hidden)
{
	if (!hidden || !item) {
		return;
	}
	QTreeWidget *view = static_cast<QTreeWidget *>(parent());
	QTreeWidgetItem *editingItem = KShortcutsEditorPrivate::itemFromIndex(view, m_editingIndex);
	if (editingItem == item) {
		itemActivated(m_editingIndex); //this will *close* the item's editor because it's already open
	}
}


//Prevent clicks in the empty part of the editor widget from closing the editor
//because they would propagate to the itemview and be interpreted as a click in
//an item's rect. That in turn would lead to an itemActivated() call, closing
//the current editor.
bool KShortcutsEditorDelegate::eventFilter(QObject *o, QEvent *e)
{
	if (o != m_editor)
		return false;

	switch (e->type()) {
	case QEvent::MouseButtonPress:
	case QEvent::MouseButtonRelease:
	case QEvent::MouseButtonDblClick:
		return true;
	default:
		return false;
	}
}


//slot
void KShortcutsEditorDelegate::keySequenceChanged(const QKeySequence &seq)
{
	QVariant ret = QVariant::fromValue(seq);
	emit shortcutChanged(ret, m_editingIndex);
}


//slot
void KShortcutsEditorDelegate::shapeGestureChanged(const KShapeGesture &gest)
{
	//this is somewhat verbose because the gesture types are not "built in" to QVariant
	QVariant ret = QVariant::fromValue(gest);
	emit shortcutChanged(ret, m_editingIndex);
}


//slot
void KShortcutsEditorDelegate::rockerGestureChanged(const KRockerGesture &gest)
{
	QVariant ret = QVariant::fromValue(gest);
	emit shortcutChanged(ret, m_editingIndex);
}


void TabConnectedWidget::paintEvent(QPaintEvent *e)
{
	QWidget::paintEvent(e);
	QPainter p(this);
	QPen pen(QPalette().highlight().color());
	pen.setWidth(6);
	p.setPen(pen);
	p.drawLine(0, 0, width(), 0);
	p.drawLine(0, 0, 0, height());
}


ShortcutEditWidget::ShortcutEditWidget(QWidget *viewport, const QKeySequence &defaultSeq,
                                       const QKeySequence &activeSeq, bool allowLetterShortcuts)
 : TabConnectedWidget(viewport),
   m_defaultKeySequence(defaultSeq),
   m_isUpdating(false)
{
	QGridLayout *layout = new QGridLayout(this);

	m_defaultRadio = new QRadioButton(i18n("Default:"), this);
	QString defaultText = defaultSeq.toString();
	if (defaultText.isEmpty())
		defaultText = i18n("None");
	QLabel *defaultLabel = new QLabel(defaultText, this);

	m_customRadio = new QRadioButton(i18n("Custom:"), this);
	m_customEditor = new KKeySequenceWidget(this);
	m_customEditor->setModifierlessAllowed(allowLetterShortcuts);

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

	connect(m_defaultRadio, SIGNAL(toggled(bool)),
	        this, SLOT(defaultToggled(bool)));
	connect(m_customEditor, SIGNAL(keySequenceChanged(const QKeySequence &)),
	        this, SLOT(setCustom(const QKeySequence &)));
}


//slot
void ShortcutEditWidget::defaultToggled(bool checked)
{
	if (m_isUpdating)
		return;

	m_isUpdating = true;
	m_customEditor->clearKeySequence();
	if  (checked) {
		emit keySequenceChanged(m_defaultKeySequence);
	} else {
		//custom was checked
		emit keySequenceChanged(QKeySequence());
	}
	m_isUpdating = false;
}


//slot
void ShortcutEditWidget::setCustom(const QKeySequence &seq)
{
	if (m_isUpdating)
		return;

	m_isUpdating = true;
	if (seq != m_defaultKeySequence)
		m_customRadio->setChecked(true);
	else
		m_defaultRadio->setChecked(true);
	emit keySequenceChanged(seq);
	m_isUpdating = false;
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
		QString name = action->text().remove('&');
		kDebug(125) << "Key: " << name;

		if (name.startsWith(QLatin1String("Program:")))
			l = Program;
		else if (name.startsWith(QLatin1String("Group:")))
			l = Group;
		else if (qobject_cast<QAction *>(action)) {
			// TODO  non-KAction QActions are not listed
			if ((kact = qobject_cast<KAction *>(action)) && kact->isShortcutConfigurable())
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

	QTimer::singleShot(0, this, SLOT(resizeColumns()));
}


//slot
void KShortcutsEditor::resizeColumns()
{
	for (int i = 0; i < d->ui.list->columnCount(); i++)
		d->ui.list->resizeColumnToContents(i);
}


void KShortcutsEditor::save()
{
	foreach (KActionCollection* collection, d->actionCollections)
		collection->writeSettings();
}


void KShortcutsEditor::undoChanges()
{
	//This function used to crash sometimes when invoked by clicking on "cancel"
	//with Qt 4.2.something. Apparently items were deleted too early by Qt.
	//It seems to work with 4.3-ish Qt versions. Keep an eye on this.
	for (QTreeWidgetItemIterator it(d->ui.list); (*it); ++it) {
		if ((*it)->childCount())
			continue;

		static_cast<KShortcutsEditorItem *>(*it)->undoChanges();
	}
}


void KShortcutsEditorPrivate::initGUI( KShortcutsEditor::ActionTypes types, KShortcutsEditor::LetterShortcuts allowLetterShortcuts )
{
	actionTypes = types;

	ui.setupUi(q);
	ui.searchFilter->searchLine()->setTreeWidget(ui.list); // Plug into search line
	ui.list->header()->setStretchLastSection(false);
	ui.list->header()->setResizeMode(QHeaderView::ResizeToContents);
	ui.list->header()->setResizeMode(0, QHeaderView::Stretch);
	ui.list->header()->hideSection(GlobalAlternate);  //not expected to be very useful
	ui.list->header()->hideSection(ShapeGesture);  //mouse gestures didn't make it in time...
	ui.list->header()->hideSection(RockerGesture);
	if (!(actionTypes & KShortcutsEditor::GlobalAction)) {
		ui.list->header()->hideSection(GlobalPrimary);
	} else if (!(actionTypes & ~KShortcutsEditor::GlobalAction)) {
		ui.list->header()->hideSection(LocalPrimary);
		ui.list->header()->hideSection(LocalAlternate);
	}

	KShortcutsEditorDelegate *delegate = new KShortcutsEditorDelegate(ui.list,
	                                       allowLetterShortcuts == KShortcutsEditor::LetterShortcutsAllowed);
	ui.list->setItemDelegate(delegate);
	ui.list->setSelectionBehavior(QAbstractItemView::SelectItems);
	ui.list->setSelectionMode(QAbstractItemView::SingleSelection);
	//we have our own editing mechanism
	ui.list->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.list->setAlternatingRowColors(true);

	//TODO listen to changes to global shortcuts
	QObject::connect(delegate, SIGNAL(shortcutChanged(QVariant, const QModelIndex &)),
	                 q, SLOT(capturedShortcut(QVariant, const QModelIndex &)));
	//hide the editor widget chen its item becomes hidden
	QObject::connect(ui.searchFilter->searchLine(), SIGNAL(hiddenChanged(QTreeWidgetItem *, bool)),
	                 delegate, SLOT(hiddenBySearchLine(QTreeWidgetItem *, bool)));
	ui.searchFilter->setFocus();
}


//static
KShortcutsEditorItem *KShortcutsEditorPrivate::itemFromIndex(QTreeWidget *const w,
                                                             const QModelIndex &index)
{
	QTreeWidgetItem *item = static_cast<QTreeWidgetHack *>(w)->itemFromIndex(index);
	if (item && item->type() == ActionItem) {
		return static_cast<KShortcutsEditorItem *>(item);
	}
	return 0;
}


QTreeWidgetItem *KShortcutsEditorPrivate::findOrMakeItem(QTreeWidgetItem *parent, const QString &name)
{
	for (int i = 0; i < parent->childCount(); i++) {
		QTreeWidgetItem *child = parent->child(i);
		if (child->text(0) == name)
			return child;
	}
	QTreeWidgetItem *ret = new QTreeWidgetItem(parent, NonActionItem);
	ret->setText(0, name);
	ui.list->expandItem(ret);
	ret->setFlags(ret->flags() & ~Qt::ItemIsSelectable);
	return ret;
}


//private slot
void KShortcutsEditorPrivate::capturedShortcut(const QVariant &newShortcut, const QModelIndex &index)
{
	//dispatch to the right handler
	if (!index.isValid())
		return;
	int column = index.column();
	KShortcutsEditorItem *item = itemFromIndex(ui.list, index);
	Q_ASSERT(item);

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
			//when moving around a shortcut in the same action item the intent is clear
			//and we don't need to ask about conflicts
			if ((*it)->childCount() || (*it == item))
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
	q->keyChange();
	//force view update
	item->setText(column, capture.toString());
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
			if (!(*it)->parent() || (*it == item))
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
			if (!(*it)->parent() || (*it == item))
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
	q->keyChange();
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


//TODO have KGlobalAccel emit a signal on changes, connect it to this, do what needs to be done
//slot
void KShortcutsEditorPrivate::globalSettingsChangedSystemwide(int which)
{
	Q_UNUSED(which)
	//update display or similar
}


//We ask the user here if there are any conflicts, as opposed to undoChanges().
//They don't do the same thing anyway, this just not to confuse any readers.
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


//---------------------------------------------------
KShortcutsEditorItem::KShortcutsEditorItem(QTreeWidgetItem *parent, KAction *action)
	: QTreeWidgetItem(parent, ActionItem)
	, m_action(action)
	, m_isNameBold(false)
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
	switch (role) {
	case Qt::DisplayRole:
		switch(column) {
		case Name:
			return i18nc("@item:intable Action name in shortcuts configuration", "%1", m_action->text().remove('&'));
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
		return i18nc("@info:tooltip Action tooltip in shortcuts configuration", "%1", m_action->toolTip());
	case Qt::FontRole:
		if (column == Name && m_isNameBold) {
			QFont modifiedFont = treeWidget()->font();
			modifiedFont.setBold(true);
			return modifiedFont;
		}
		break;
	case KExtendableItemDelegate::ShowExtensionIndicatorRole:
		if (column == Name)
			return false;
		else
			return true;
//the following are custom roles, defined in this source file only
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
		m_action->setGlobalShortcut(ks, KAction::ActiveShortcut, KAction::NoAutoloading);
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
		m_action->setGlobalShortcut(*m_oldGlobalShortcut, KAction::ActiveShortcut,
		                            KAction::NoAutoloading);

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
	group.writeEntry( "Dialog Size", size(), KConfigGroup::Global );
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

QSize KShortcutsDialog::sizeHint() const
{
    return QSize(500, 400);
}

int KShortcutsDialog::configure(KActionCollection *collection, KShortcutsEditor::LetterShortcuts allowLetterShortcuts,
                          QWidget *parent, bool saveSettings)
{
	kDebug(125) << "KShortcutsDialog::configureKeys( KActionCollection*, " << saveSettings << " )";
	KShortcutsDialog dlg(KShortcutsEditor::AllActions, allowLetterShortcuts, parent);
	dlg.d->m_keyChooser->addCollection(collection);
	return dlg.configure(saveSettings);
}

#include "kshortcutseditor.moc"
#include "kshortcutsdialog.moc"
#include "kshortcutsdialog_p.moc"
