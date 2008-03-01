/* This file is part of the KDE libraries Copyright (C) 1998 Mark Donohoe <donohoe@kde.org>
    Copyright (C) 1997 Nicolas Hadacek <hadacek@kde.org>
    Copyright (C) 1998 Matthias Ettrich <ettrich@kde.org>
    Copyright (C) 2001 Ellis Whitehead <ellis@kde.org>
    Copyright (C) 2006 Hamish Rodda <rodda@kde.org>
    Copyright (C) 2007 Roberto Raggi <roberto@kdevelop.org>
    Copyright (C) 2007 Andreas Hartmetz <ahartmetz@gmail.com>
    Copyright (C) 2008 Michael Jansen <kde@michael-jansen.biz>

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

#include "kshortcutseditor.h"

// The following is needed for KShortcutsEditorPrivate
#include "kshortcutsdialog_p.h"

#include <QHeaderView>
#include <QTimer>

#include "kaction.h"
#include "kactioncollection.h"
#include "kdebug.h"
#include "kglobalaccel.h"
#include "kmessagebox.h"

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


bool KShortcutsEditor::isModified() const
{
    for (QTreeWidgetItemIterator it(d->ui.list); (*it); ++it) {
        if ((*it)->childCount())
            continue;

        if (static_cast<KShortcutsEditorItem *>(*it)->isModified()) {
            return true;
        }
    }
    return false;
}

void KShortcutsEditor::clearCollections()
{
    d->delegate->clear();
    d->ui.list->clear();
    d->actionCollections.clear();
    QTimer::singleShot(0, this, SLOT(resizeColumns()));
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

        if (name.startsWith(QLatin1String("Program:")))
            l = Program;
        else if (name.startsWith(QLatin1String("Group:")))
            l = Group;
        else if (qobject_cast<QAction *>(action)) {
            // TODO  non-KAction QActions are not listed
            if ((kact = qobject_cast<KAction *>(action)) && kact->isShortcutConfigurable()) {
                // If the shortcut is not configurable skip it
                if (!kact->isShortcutConfigurable()) {
                    continue;
                }
                // Create the editor
                new KShortcutsEditorItem((hier[l]), kact);
            }
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

    d->ui.list->sortItems(0, Qt::AscendingOrder);
    QTimer::singleShot(0, this, SLOT(resizeColumns()));
}

void KShortcutsEditor::importConfiguration( KConfig *config)
{
    if (d->actionTypes & KShortcutsEditor::GlobalAction) {
        QString groupName = "Global Shortcuts";
        KConfigGroup group( config, groupName );
        foreach (KActionCollection* collection, d->actionCollections) {
            collection->readGlobalShortcuts( &group );
        }
    }
    if (d->actionTypes & !KShortcutsEditor::GlobalAction) {
        QString groupName = "Local Shortcuts";
        KConfigGroup group( config, groupName );
        foreach (KActionCollection* collection, d->actionCollections) {
            collection->readSettings( &group );
        }
    }
}

void KShortcutsEditor::exportConfiguration( KConfig *config) const
{
    if (d->actionTypes & KShortcutsEditor::GlobalAction) {
        QString groupName = "Global Shortcuts";
        KConfigGroup group( config, groupName );
        foreach (KActionCollection* collection, d->actionCollections) {
            collection->writeGlobalShortcuts( &group, true );
        }
    }
    if (d->actionTypes & !KShortcutsEditor::GlobalAction) {
        QString groupName = "Local Shortcuts";
        KConfigGroup group( config, groupName );
        foreach (KActionCollection* collection, d->actionCollections) {
            collection->writeSettings( &group, true );
        }
    }
}


void KShortcutsEditor::writeConfiguration( KConfigGroup *config) const
{
    foreach (KActionCollection* collection, d->actionCollections) {
        collection->writeSettings(config);
    }
}


//slot
void KShortcutsEditor::resizeColumns()
{
    for (int i = 0; i < d->ui.list->columnCount(); i++)
        d->ui.list->resizeColumnToContents(i);
}


void KShortcutsEditor::save()
{
    // we have to call commit on all items. If we wouldn't do that they would undo their changes
    // upon deletion! That would lead to weird problems. Changes to Global Shortcuts would vanish
    // completely. Changes to local shortcuts would vanish for this session.
    for (QTreeWidgetItemIterator it(d->ui.list); (*it); ++it) {
        if ((*it)->childCount())
            continue;

        static_cast<KShortcutsEditorItem *>(*it)->commit();
    }

    writeConfiguration();
}

// KDE5 : rename to undo()
void KShortcutsEditor::undoChanges()
{
    //This function used to crash sometimes when invoked by clicking on "cancel"
    //with Qt 4.2.something. Apparently items were deleted too early by Qt.
    //It seems to work with 4.3-ish Qt versions. Keep an eye on this.
    for (QTreeWidgetItemIterator it(d->ui.list); (*it); ++it) {
        if ((*it)->childCount())
            continue;

        static_cast<KShortcutsEditorItem *>(*it)->undo();
    }
}


//We ask the user here if there are any conflicts, as opposed to undoChanges().
//They don't do the same thing anyway, this just not to confuse any readers.
//slot
void KShortcutsEditor::allDefault()
{
    d->allDefault();
}



//---------------------------------------------------------------------
// KShortcutsEditorPrivate
//---------------------------------------------------------------------

KShortcutsEditorPrivate::KShortcutsEditorPrivate( KShortcutsEditor *q )
    :   q(q),
        delegate(0)
    {}

void KShortcutsEditorPrivate::initGUI( KShortcutsEditor::ActionTypes types, KShortcutsEditor::LetterShortcuts allowLetterShortcuts )
{

    actionTypes = types;

    ui.setupUi(q);
    ui.searchFilter->searchLine()->setTreeWidget(ui.list); // Plug into search line
    ui.list->header()->setResizeMode(QHeaderView::ResizeToContents);
    ui.list->header()->hideSection(GlobalAlternate);  //not expected to be very useful
    ui.list->header()->hideSection(ShapeGesture);  //mouse gestures didn't make it in time...
    ui.list->header()->hideSection(RockerGesture);
    if (!(actionTypes & KShortcutsEditor::GlobalAction)) {
        ui.list->header()->hideSection(GlobalPrimary);
    } else if (!(actionTypes & ~KShortcutsEditor::GlobalAction)) {
        ui.list->header()->hideSection(LocalPrimary);
        ui.list->header()->hideSection(LocalAlternate);
    }

    delegate = new KShortcutsEditorDelegate(ui.list,
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


//a gross hack to make a protected method public
class QTreeWidgetHack : public QTreeWidget
{
public:
    QTreeWidgetItem *itemFromIndex(const QModelIndex &index) const
        { return QTreeWidget::itemFromIndex(index); }
};


void KShortcutsEditorPrivate::allDefault()
{
    for (QTreeWidgetItemIterator it(ui.list); (*it); ++it) {
        if (!(*it)->parent())
            continue;

        KShortcutsEditorItem *item = static_cast<KShortcutsEditorItem *>(*it);
        KAction *act = item->m_action;

        if (act->shortcut() != act->shortcut(KAction::DefaultShortcut)) {
            changeKeyShortcut(item, LocalPrimary, act->shortcut(KAction::DefaultShortcut).primary());
            changeKeyShortcut(item, LocalAlternate, act->shortcut(KAction::DefaultShortcut).alternate());
        }

        if (act->globalShortcut() != act->globalShortcut(KAction::DefaultShortcut)) {
            changeKeyShortcut(item, GlobalPrimary, act->globalShortcut(KAction::DefaultShortcut).primary());
            changeKeyShortcut(item, GlobalAlternate, act->globalShortcut(KAction::DefaultShortcut).alternate());
        }

        if (act->shapeGesture() != act->shapeGesture(KAction::DefaultShortcut))
            changeShapeGesture(item, act->shapeGesture(KAction::DefaultShortcut));

        if (act->rockerGesture() != act->rockerGesture(KAction::DefaultShortcut))
            changeRockerGesture(item, act->rockerGesture(KAction::DefaultShortcut));
    }
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

            // We only check for conflicts with local shortcuts. If the app
            // has global shortcuts we will check against them below with the
            // global shortcuts of other applications
            for (i = LocalPrimary; i <= LocalAlternate; i++) {
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
    item->setText(column, capture.toString(QKeySequence::NativeText));
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
                           seq.toString(QKeySequence::NativeText), item->m_action->text());

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
                           seq.toString(QKeySequence::NativeText), KStandardShortcut::name(std));

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


#include "kshortcutseditor.moc"
