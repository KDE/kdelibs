/***************************************************************************
 * scriptmanagereditor.h
 * This file is part of the KDE project
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

#include "scriptmanagereditor.h"
//#include "scriptmanager.h"

#include "../core/manager.h"
#include "../core/action.h"
#include "../core/actioncollection.h"
#include "../core/interpreter.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QHeaderView>
#include <QTableWidget>
//#include <kdeversion.h>
#include <kconfig.h>
#include <klocale.h>
#include <kpushbutton.h>
#include <kfiledialog.h>
#include <kurlrequester.h>
#include <kicondialog.h>
#include <kpagedialog.h>

using namespace Kross;

/******************************************************************************
 * ScriptManagerPropertiesEditor
 */

namespace Kross {

    /// \internal d-pointer class.
    class ScriptManagerPropertiesEditor::Private
    {
        public:
            Action* action;
            QTableWidget* table;
            KPushButton* rmbtn;

            Private(Action* a) : action(a) { Q_ASSERT(a); }

            void addItem(const QString& name, const QString& value)
            {
                const int rows = table->rowCount();
                table->insertRow(rows);
                QTableWidgetItem* nameitem = new QTableWidgetItem( name );
                table->setItem(rows, 0, nameitem);
                QTableWidgetItem* valueitem = new QTableWidgetItem( value );
                table->setItem(rows, 1, valueitem);
            }

    };

}

ScriptManagerPropertiesEditor::ScriptManagerPropertiesEditor(Action* action, QWidget* parent)
    : QWidget(parent), d(new Private(action))
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    //layout->setSpacing(0);
    //layout->setMargin(0);
    setLayout(layout);

    d->table = new QTableWidget(this);
    d->table->setColumnCount(2);
    d->table->setHorizontalHeaderLabels( QStringList() << i18n("Name") << i18n("Value") );
    d->table->verticalHeader()->hide();
    //d->table->setAlternatingRowColors(true);
    foreach(QString propname, d->action->propertyNames())
        d->addItem( propname, d->action->property(propname) );
    d->table->horizontalHeader()->resizeSections( QHeaderView::ResizeToContents );
    d->table->horizontalHeader()->setStretchLastSection(true);
    d->table->setEditTriggers( QAbstractItemView::AllEditTriggers );
    d->table->setSortingEnabled(true);
    layout->addWidget(d->table);

    QWidget* btnwidget = new QWidget(this);
    QVBoxLayout* btnlayout = new QVBoxLayout();
    btnlayout->setMargin(0);
    btnwidget->setLayout(btnlayout);
    layout->addWidget(btnwidget);

    KPushButton* addbtn = new KPushButton(KIcon("add"), i18n("Add"), btnwidget);
    addbtn->setToolTip( i18n("Add new property") );
    btnlayout->addWidget(addbtn);
    connect(addbtn, SIGNAL(clicked()), this, SLOT(slotAdd()) );

    d->rmbtn = new KPushButton(KIcon("remove"), i18n("Remove"), btnwidget);
    d->rmbtn->setToolTip( i18n("Remove selected property") );
    btnlayout->addWidget(d->rmbtn);
    connect(d->rmbtn, SIGNAL(clicked()), this, SLOT(slotRemove()) );

    btnlayout->addStretch(1);

    connect(d->table, SIGNAL(itemSelectionChanged()), this, SLOT(slotSelectionChanged()));
    slotSelectionChanged();
}

ScriptManagerPropertiesEditor::~ScriptManagerPropertiesEditor()
{
    delete d;
}

void ScriptManagerPropertiesEditor::slotSelectionChanged()
{
    d->rmbtn->setEnabled( d->table->currentRow() >= 0 );
}

void ScriptManagerPropertiesEditor::slotAdd()
{
    d->table->setSortingEnabled(false);
    const int rows = d->table->rowCount();
    d->addItem("", "");
    d->table->selectRow(rows);
    d->table->setSortingEnabled(true);
}

void ScriptManagerPropertiesEditor::slotRemove()
{
    d->table->removeRow( d->table->currentRow() );
    slotSelectionChanged();
}

void ScriptManagerPropertiesEditor::commit()
{
    QStringList oldpropnames = d->action->propertyNames();

    const int count = d->table->rowCount();
    for(int i = 0; i < count; ++i) {
        const QString name = d->table->item(i, 0)->text();
        const QString value = d->table->item(i, 1)->text();

        if( d->action->hasProperty(name) )
            oldpropnames.removeAll(name);
        d->action->setProperty(name, value, true/*persistent*/);
    }

    foreach(QString oldpropname, oldpropnames)
        d->action->removeProperty(oldpropname);
}

/******************************************************************************
 * ScriptManagerEditor
 */

namespace Kross {

    /// \internal d-pointer class.
    class ScriptManagerEditor::Private
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

            QLineEdit* nameedit;
            QLineEdit* textedit;
            QLineEdit* commentedit;
            QLineEdit* iconedit;
            QComboBox* interpreteredit;
            KUrlRequester* fileedit;
            QCheckBox* enabledcheckbox;

            explicit Private(Action* a) : type(ActionType), action(a) { Q_ASSERT(a); }
            explicit Private(ActionCollection* c) : type(CollectionType), collection(c) { Q_ASSERT(c); }
    };

}

ScriptManagerEditor::ScriptManagerEditor(Action* action, QWidget* parent)
    : QWidget(parent), d(new Private(action))
{
    initGui();
}

ScriptManagerEditor::ScriptManagerEditor(ActionCollection* collection, QWidget* parent)
    : QWidget(parent), d(new Private(collection))
{
    initGui();
}

ScriptManagerEditor::~ScriptManagerEditor()
{
    delete d;
}

void ScriptManagerEditor::initGui()
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
    d->nameedit = new QLineEdit(w);
    namelabel->setBuddy(d->nameedit);
    d->nameedit->setText( d->name() );
    d->nameedit->setEnabled(false);
    gridlayout->addWidget(d->nameedit, 0, 1);

    QLabel* textlabel = new QLabel(i18n("Text:"), w);
    gridlayout->addWidget(textlabel, 1, 0);
    d->textedit = new QLineEdit(w);
    textlabel->setBuddy(d->textedit);
    d->textedit->setText( d->text() );
    gridlayout->addWidget(d->textedit, 1, 1);

    QLabel* commentlabel = new QLabel(i18n("Comment:"), w);
    gridlayout->addWidget(commentlabel, 2, 0);
    d->commentedit = new QLineEdit(w);
    commentlabel->setBuddy(d->commentedit);
    d->commentedit->setText( d->description() );
    gridlayout->addWidget(d->commentedit, 2, 1);

    QLabel* iconlabel = new QLabel(i18n("Icon:"), w);
    gridlayout->addWidget(iconlabel, 3, 0);
    QWidget* iconbox = new QWidget(w);
    QHBoxLayout* iconlayout = new QHBoxLayout();
    iconlayout->setMargin(0);
    iconbox->setLayout(iconlayout);
    d->iconedit = new QLineEdit(iconbox);
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
        d->interpreteredit = new QComboBox(w);
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
        foreach(QString interpretername, Manager::self().interpreters()) {
            InterpreterInfo* info = Manager::self().interpreterInfo(interpretername);
            Q_ASSERT( info );
            mimetypes.append( info->mimeTypes().join(" ").trimmed() );
        }
        //InterpreterInfo* info = Manager::self().interpreterInfo( Manager::self().interpreternameForFile( d->action->file() ) );
        //const QString defaultmime = info ? info->mimeTypes().join(" ").trimmed() : QString();
        d->fileedit->fileDialog()->setMimeFilter(mimetypes /*, defaultmime*/);
        d->fileedit->setMode( KFile::File | KFile::ExistingOnly | KFile::LocalOnly );
        //d->fileedit->setUrl( d->action->file() );
        d->fileedit->setPath( d->action->file() );
        gridlayout->addWidget(d->fileedit, 5, 1);
    }
    else {
        d->interpreteredit = 0;
        d->fileedit = 0;
    }

    d->enabledcheckbox = new QCheckBox(this);
    d->enabledcheckbox->setText( i18n("Enabled") );
    d->enabledcheckbox->setChecked( d->isEnabled() );
    mainlayout->addWidget(d->enabledcheckbox);

    mainlayout->addStretch(1);
}

bool ScriptManagerEditor::isValid()
{
    //TODO check also if such a name already exist.
    return ! d->nameedit->text().isEmpty();
}

void ScriptManagerEditor::commit()
{
    switch( d->type ) {
        case Private::ActionType: {
            d->action->setText( d->textedit->text() );
            d->action->setDescription( d->commentedit->text() );
            d->action->setIconName( d->iconedit->text() );
            d->action->setInterpreter( d->interpreteredit->currentText() );
            d->action->setFile( d->fileedit->url().path() );
            d->action->setEnabled( d->enabledcheckbox->isChecked() );
        } break;
        case Private::CollectionType: {
            d->collection->setText( d->textedit->text() );
            d->collection->setDescription( d->commentedit->text() );
            d->collection->setIconName( d->iconedit->text() );
            d->collection->setEnabled( d->enabledcheckbox->isChecked() );
        } break;
        default: break;
    }
}

#include "scriptmanagereditor.moc"
