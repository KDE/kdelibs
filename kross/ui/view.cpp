/***************************************************************************
 * view.cpp
 * This file is part of the KDE project
 * copyright (c) 2005-2006 Cyrille Berger <cberger@cberger.net>
 * copyright (C) 2006-2007 Sebastian Sauer <mail@dipe.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 * You should have received a copy of the GNU Library General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 ***************************************************************************/

#include "view.h"
#include "model.h"

#include <kross/core/manager.h>
#include <kross/core/action.h>
#include <kross/core/actioncollection.h>
#include <kross/core/interpreter.h>

#include <QtCore/QFileInfo>
#include <QtCore/QDir>
#include <QtGui/QBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QTreeView>
#include <QtGui/QLabel>

#include <kapplication.h>
//#include <kdeversion.h>
#include <kconfig.h>
#include <kstandarddirs.h>
#include <kmessagebox.h>
#include <kpushbutton.h>
#include <kfiledialog.h>
#include <kmenu.h>
#include <kpagedialog.h>
#include <kaction.h>
#include <kactioncollection.h>
#include <kcombobox.h>
#include <kicondialog.h>
#include <klocale.h>
#include <klineedit.h>
#include <kurlrequester.h>

//#include <ktar.h>
//#include <kio/netaccess.h>

using namespace Kross;

/*********************************************************************************
 * ActionCollectionEditor
 */

namespace Kross {

    /// \internal d-pointer class.
    class ActionCollectionEditor::Private
    {
        public:
            enum Type { ActionType, CollectionType };
            const Type type;
            union {
                Action* action;
                ActionCollection* collection;
            };

            QString name() const {
                return type == ActionType ? action->name() : collection->name();
            }
            QString text() const {
                return type == ActionType ? action->text() : collection->text();
            }
            QString description() const {
                return type == ActionType ? action->description() : collection->description();
            }
            QString iconName() const {
                return type == ActionType ? action->iconName() : collection->iconName();
            }
            bool isEnabled() const {
                return type == ActionType ? action->isEnabled() : collection->isEnabled();
            }

            KLineEdit* nameedit;
            KLineEdit* textedit;
            KLineEdit* commentedit;
            KLineEdit* iconedit;
            KComboBox* interpreteredit;
            KUrlRequester* fileedit;
            //QCheckBox* enabledcheckbox;

            explicit Private(Action* a) : type(ActionType), action(a) { Q_ASSERT(a); }
            explicit Private(ActionCollection* c) : type(CollectionType), collection(c) { Q_ASSERT(c); }
    };

}

ActionCollectionEditor::ActionCollectionEditor(Action* action, QWidget* parent)
    : QWidget(parent), d(new Private(action))
{
    initGui();
}

ActionCollectionEditor::ActionCollectionEditor(ActionCollection* collection, QWidget* parent)
    : QWidget(parent), d(new Private(collection))
{
    initGui();
}

ActionCollectionEditor::~ActionCollectionEditor()
{
    delete d;
}

Action* ActionCollectionEditor::action() const
{
    return d->type == Private::ActionType ? d->action : 0;
}

ActionCollection* ActionCollectionEditor::collection() const
{
    return d->type == Private::CollectionType ? d->collection : 0;
}

QLineEdit* ActionCollectionEditor::nameEdit() const { return d->nameedit; }
QLineEdit* ActionCollectionEditor::textEdit() const { return d->textedit; }
QLineEdit* ActionCollectionEditor::commentEdit() const { return d->commentedit; }
QLineEdit* ActionCollectionEditor::iconEdit() const { return d->iconedit; }
QComboBox* ActionCollectionEditor::interpreterEdit() const { return d->interpreteredit; }
KUrlRequester* ActionCollectionEditor::fileEdit() const { return d->fileedit; }

void ActionCollectionEditor::initGui()
{
    QVBoxLayout* mainlayout = new QVBoxLayout();
    setLayout(mainlayout);

    QWidget* w = new QWidget(this);
    mainlayout->addWidget(w);
    QGridLayout* gridlayout = new QGridLayout();
    gridlayout->setMargin(0);
    //gridlayout->setSpacing(0);
    w->setLayout(gridlayout);

    QLabel* namelabel = new QLabel(i18n("Name:"), w);
    gridlayout->addWidget(namelabel, 0, 0);
    d->nameedit = new KLineEdit(w);
    namelabel->setBuddy(d->nameedit);
    d->nameedit->setText( d->name() );
    d->nameedit->setEnabled(false);
    gridlayout->addWidget(d->nameedit, 0, 1);

    QLabel* textlabel = new QLabel(i18n("Text:"), w);
    gridlayout->addWidget(textlabel, 1, 0);
    d->textedit = new KLineEdit(w);
    textlabel->setBuddy(d->textedit);
    d->textedit->setText( d->text() );
    gridlayout->addWidget(d->textedit, 1, 1);

    QLabel* commentlabel = new QLabel(i18n("Comment:"), w);
    gridlayout->addWidget(commentlabel, 2, 0);
    d->commentedit = new KLineEdit(w);
    commentlabel->setBuddy(d->commentedit);
    d->commentedit->setText( d->description() );
    gridlayout->addWidget(d->commentedit, 2, 1);

    QLabel* iconlabel = new QLabel(i18n("Icon:"), w);
    gridlayout->addWidget(iconlabel, 3, 0);
    QWidget* iconbox = new QWidget(w);
    QHBoxLayout* iconlayout = new QHBoxLayout();
    iconlayout->setMargin(0);
    iconbox->setLayout(iconlayout);
    d->iconedit = new KLineEdit(iconbox);
    iconlabel->setBuddy(d->iconedit);
    d->iconedit->setText( d->iconName() );
    iconlayout->addWidget(d->iconedit, 1);
    KIconButton* iconbutton = new KIconButton(iconbox);
    iconbutton->setIcon( d->iconName() );
    connect(iconbutton, SIGNAL(iconChanged(QString)), d->iconedit, SLOT(setText(QString)));
    iconlayout->addWidget(iconbutton);
    gridlayout->addWidget(iconbox, 3, 1);

    //QFrame* hr1 = new QFrame(w);
    //hr1->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    //gridlayout->addWidget(hr1, 4, 0, -1, -1, Qt::AlignVCenter);

    if( d->type == Private::ActionType ) {
        QLabel* interpreterlabel = new QLabel(i18n("Interpreter:"), w);
        gridlayout->addWidget(interpreterlabel, 4, 0);
        d->interpreteredit = new KComboBox(w);
        interpreterlabel->setBuddy(d->interpreteredit);
        d->interpreteredit->setMaxVisibleItems(10);
        d->interpreteredit->insertItems(0, Manager::self().interpreters());
        d->interpreteredit->setEditable(true);
        //c->lineEdit()->setText( d->action->interpreter() );
        int idx = Manager::self().interpreters().indexOf( d->action->interpreter() );
        if( idx >= 0 )
            d->interpreteredit->setCurrentIndex(idx);
        else
            d->interpreteredit->setEditText( d->action->interpreter() );
        gridlayout->addWidget(d->interpreteredit, 4, 1);

        QLabel* filelabel = new QLabel(i18n("File:"), w);
        gridlayout->addWidget(filelabel, 5, 0);
        d->fileedit = new KUrlRequester(w);
        filelabel->setBuddy(d->fileedit);
        QStringList mimetypes;
        foreach(const QString &interpretername, Manager::self().interpreters()) {
            InterpreterInfo* info = Manager::self().interpreterInfo(interpretername);
            Q_ASSERT( info );
            mimetypes.append( info->mimeTypes().join(" ").trimmed() );
        }
        //InterpreterInfo* info = Manager::self().interpreterInfo( Manager::self().interpreternameForFile( d->action->file() ) );
        //const QString defaultmime = info ? info->mimeTypes().join(" ").trimmed() : QString();
        d->fileedit->fileDialog()->setMimeFilter(mimetypes /*, defaultmime*/);
        d->fileedit->setMode( KFile::File | KFile::ExistingOnly | KFile::LocalOnly );
        d->fileedit->setUrl(KUrl(d->action->file()) );
        gridlayout->addWidget(d->fileedit, 5, 1);
    }
    else {
        d->interpreteredit = 0;
        d->fileedit = 0;
    }

    //d->enabledcheckbox = new QCheckBox(this);
    //d->enabledcheckbox->setText( i18n("Enabled") );
    //d->enabledcheckbox->setChecked( d->isEnabled() );
    //mainlayout->addWidget(d->enabledcheckbox);

    mainlayout->addStretch(1);
}

bool ActionCollectionEditor::isValid()
{
    //TODO check also if such a name already exist.
    return ! d->nameedit->text().isEmpty();
}

void ActionCollectionEditor::commit()
{
    switch( d->type ) {
        case Private::ActionType: {
            d->action->setText( d->textedit->text() );
            d->action->setDescription( d->commentedit->text() );
            d->action->setIconName( d->iconedit->text() );
            d->action->setInterpreter( d->interpreteredit->currentText() );
            d->action->setFile( d->fileedit->url().path() );
            //d->action->setEnabled( d->enabledcheckbox->isChecked() );
        } break;
        case Private::CollectionType: {
            d->collection->setText( d->textedit->text() );
            d->collection->setDescription( d->commentedit->text() );
            d->collection->setIconName( d->iconedit->text() );
            //d->collection->setEnabled( d->enabledcheckbox->isChecked() );
        } break;
        default: break;
    }
}

/*********************************************************************************
 * ActionCollectionView
 */

namespace Kross {

    /// \internal d-pointer class.
    class ActionCollectionView::Private
    {
        public:
            bool modified;
            KActionCollection* collection;
            QMap< QString, KPushButton* > buttons;
            explicit Private() : modified(false) {}
    };

}

ActionCollectionView::ActionCollectionView(QWidget* parent)
    : QTreeView(parent)
    , d(new Private())
{
    header()->hide();
    setSelectionMode(QAbstractItemView::SingleSelection);
    setAlternatingRowColors(true);
    setRootIsDecorated(true);
    setSortingEnabled(false);
    setItemsExpandable(true);
    //setDragEnabled(true);
    //setAcceptDrops(true);
    setDropIndicatorShown(true);
    setDragDropMode(QAbstractItemView::InternalMove);

    d->collection = new KActionCollection(this);

    KAction* runaction = new KAction(KIcon("system-run"), i18n("Run"), this);
    runaction->setObjectName("run");
    runaction->setToolTip( i18n("Execute the selected script.") );
    runaction->setEnabled(false);
    d->collection->addAction("run", runaction);
    connect(runaction, SIGNAL(triggered()), this, SLOT(slotRun()));

    KAction* stopaction = new KAction(KIcon("process-stop"), i18n("Stop"), this);
    stopaction->setObjectName("stop");
    stopaction->setToolTip( i18n("Stop execution of the selected script.") );
    stopaction->setEnabled(false);
    d->collection->addAction("stop", stopaction);
    connect(stopaction, SIGNAL(triggered()), this, SLOT(slotStop()));

    KAction* editaction = new KAction(KIcon("document-properties"), i18n("Edit..."), this);
    editaction->setObjectName("edit");
    editaction->setToolTip( i18n("Edit selected script.") );
    editaction->setEnabled(false);
    d->collection->addAction("edit", editaction);
    connect(editaction, SIGNAL(triggered()), this, SLOT(slotEdit()));

    KAction* addaction = new KAction(KIcon("list-add"), i18n("Add..."), this);
    addaction->setObjectName("add");
    addaction->setToolTip( i18n("Add a new script.") );
    //addaction->setEnabled(false);
    d->collection->addAction("add", addaction);
    connect(addaction, SIGNAL(triggered()), this, SLOT(slotAdd()) );

    KAction* removeaction = new KAction(KIcon("list-remove"), i18n("Remove"), this);
    removeaction->setObjectName("remove");
    removeaction->setToolTip( i18n("Remove selected script.") );
    removeaction->setEnabled(false);
    d->collection->addAction("remove", removeaction);
    connect(removeaction, SIGNAL(triggered()), this, SLOT(slotRemove()) );

    connect(this, SIGNAL(enabledChanged(const QString&)), this, SLOT(slotEnabledChanged(const QString&)));
    //expandAll();
}

ActionCollectionView::~ActionCollectionView()
{
    delete d;
}

void ActionCollectionView::setModel(QAbstractItemModel* m)
{
    QTreeView::setModel(m);
    d->modified = false;

    QItemSelectionModel* selectionmodel = new QItemSelectionModel(m, this);
    setSelectionModel(selectionmodel);

    connect(selectionModel(), SIGNAL(selectionChanged(const QItemSelection&,const QItemSelection&)),
            this, SLOT(slotSelectionChanged()));
    connect(m, SIGNAL(dataChanged(const QModelIndex&,const QModelIndex&)),
            this, SLOT(slotDataChanged(const QModelIndex&,const QModelIndex&)));
}

bool ActionCollectionView::isModified() const
{
    return d->modified;
}

void ActionCollectionView::setModified(bool modified)
{
    d->modified = modified;
}

KActionCollection* ActionCollectionView::actionCollection() const
{
    return d->collection;
}

KPushButton* ActionCollectionView::button(const QString& actionname) const
{
    return d->buttons.contains(actionname) ? d->buttons[actionname] : 0;
}

QItemSelection ActionCollectionView::itemSelection() const
{
    QAbstractProxyModel* proxymodel = dynamic_cast< QAbstractProxyModel* >( model() );
    QItemSelection selection = selectionModel()->selection();
    return proxymodel ? proxymodel->mapSelectionToSource(selection) : selection;
}

KPushButton* ActionCollectionView::createButton(QWidget* parentWidget, const QString& actionname)
{
    QAction* action = d->collection->action(actionname);
    if( ! action ) return 0;
    //if( d->buttons.contains(actionname) ) delete d->buttons[];
    KPushButton* btn = new KPushButton(parentWidget);
    btn->setText( action->text() );
    btn->setToolTip( action->toolTip() );
    btn->setIcon( KIcon(action->icon()) );
    btn->setEnabled( action->isEnabled() );
    if( parentWidget && parentWidget->layout() )
        parentWidget->layout()->addWidget(btn);
    QObject::connect(btn, SIGNAL(clicked()), action, SLOT(trigger()));
    d->buttons.insert( actionname, btn );
    return btn;
}

void ActionCollectionView::slotEnabledChanged(const QString& actionname)
{
    if( d->buttons.contains( actionname ) ) {
        QAction* action = d->collection->action( actionname );
        d->buttons[ actionname ]->setEnabled( action ? action->isEnabled() : false );
    }
}

void ActionCollectionView::slotSelectionChanged()
{
    bool startenabled = selectionModel()->hasSelection();
    bool stopenabled = false;
    bool hasselection = selectionModel()->selectedIndexes().count() > 0;
    foreach(const QModelIndex &index, itemSelection().indexes()) {
        Action* action = ActionCollectionModel::action(index);
        if( startenabled && ! action )
            startenabled = false;
        if( ! stopenabled )
            stopenabled = (action && ! action->isFinalized());
    }
    QAction* runaction = d->collection->action("run");
    if( runaction ) {
        runaction->setEnabled(startenabled);
        emit enabledChanged("run");
    }
    QAction* stopaction = d->collection->action("stop");
    if( stopaction ) {
        stopaction->setEnabled(stopenabled);
        emit enabledChanged("stop");
    }
    QAction* editaction = d->collection->action("edit");
    if( editaction ) {
        editaction->setEnabled(hasselection);
        emit enabledChanged("edit");
    }
    QAction* removeaction = d->collection->action("remove");
    if( removeaction ) {
        removeaction->setEnabled(hasselection);
        emit enabledChanged("remove");
    }
}

void ActionCollectionView::slotDataChanged(const QModelIndex&, const QModelIndex&)
{
    d->modified = true;
}

void ActionCollectionView::slotRun()
{
    if( ! selectionModel() ) return;
    QAction* stopaction = d->collection->action("stop");

    foreach(const QModelIndex &index, itemSelection().indexes()) {
        if( ! index.isValid() )
            continue;
        if( stopaction ) {
            stopaction->setEnabled(true);
            emit enabledChanged("stop");
        }
        Action* action = ActionCollectionModel::action(index);
        if( ! action )
            continue;
        connect(action, SIGNAL(finished(Kross::Action*)), SLOT(slotSelectionChanged()));
        action->trigger();
    }
    slotSelectionChanged();
}

void ActionCollectionView::slotStop()
{
    if( ! selectionModel() ) return;
    foreach(const QModelIndex &index, itemSelection().indexes()) {
        if( ! index.isValid() )
            continue;
        Action* action = ActionCollectionModel::action(index);
        if( ! action )
            continue;
        //connect(action, SIGNAL(started(Kross::Action*)), SLOT(slotSelectionChanged()));
        //connect(action, SIGNAL(finished(Kross::Action*)), SLOT(slotSelectionChanged()));
        action->finalize();
    }
    slotSelectionChanged();
}

void ActionCollectionView::slotEdit()
{
    if( ! selectionModel() ) return;
    Action* action = 0;
    ActionCollection* collection = 0;
    foreach(const QModelIndex &index, itemSelection().indexes()) {
        if( ! index.isValid() ) continue;
        if( Action* a = ActionCollectionModel::action(index) )
            action = a;
        else if( ActionCollection* c = ActionCollectionModel::collection(index) )
            collection = c;
        else
            continue;
        break;
    }
    if( (! action) && (! collection) ) return;
    KPageDialog* dialog = new KPageDialog( this );
    dialog->setCaption( i18n("Edit") );
    dialog->setButtons( KDialog::Ok | KDialog::Cancel );
    //dialog->enableButtonOk( false );
    dialog->setFaceType( KPageDialog::Plain ); //Auto Plain List Tree Tabbed
    ActionCollectionEditor* editor =
        action ? new ActionCollectionEditor(action, dialog->mainWidget())
               : new ActionCollectionEditor(collection, dialog->mainWidget());
    dialog->addPage(editor, i18nc("@title:group Script properties", "General"));
    //dialog->addPage(new QWidget(this), i18n("Security"));
    dialog->resize( QSize(580, 200).expandedTo( dialog->minimumSizeHint() ) );
    int result = dialog->exec();
    if( result == QDialog::Accepted /*&& dialog->result() == KDialog::Ok*/ ) {
        editor->commit();
    }
    dialog->delayedDestruct();
}

void ActionCollectionView::slotAdd()
{

//TODO
KMessageBox::sorry(0, "TODO");

//ScriptManagerAddWizard wizard(this, collection);
//int result = wizard.exec();

#if 0
    if( ! selectionModel() ) return;
    ActionCollection* collection = 0;
    foreach(QModelIndex index, itemSelection().indexes()) {
        if( ! index.isValid() ) continue;
        if( ActionCollectionModel::action(index) ) {
            //TODO propably add the item right after the current selected one?
            QModelIndex parent = index;
            while( parent.isValid() && ! collection ) {
                parent = d->view->model()->parent(parent);
                collection = ActionCollectionModel::collection(parent);
            }
            if( collection ) break; // job done
        }
        else if( ActionCollection* c = ActionCollectionModel::collection(index) ) {
            collection = c;
            break; // job done
        }
    }
    ScriptManagerAddWizard wizard(this, collection);
    int result = wizard.exec();
    Q_UNUSED(result);
#endif
}

void ActionCollectionView::slotRemove()
{
    if( ! selectionModel() ) return;
    KMessageBox::sorry(0, "TODO");
}

#include "view.moc"
