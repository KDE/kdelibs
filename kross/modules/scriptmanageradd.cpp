/***************************************************************************
 * scriptmanageradd.h
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

#include "scriptmanageradd.h"
#include "scriptmanagereditor.h"
#include "scriptmanager.h"
#include "formfile.h"

#include "../core/manager.h"
#include "../core/interpreter.h"
#include "../core/action.h"
#include "../core/actioncollection.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
//#include <QLineEdit>
//#include <QComboBox>
//#include <QCheckBox>
#include <QRadioButton>
//#include <kdeversion.h>
//#include <kconfig.h>

#include <klocale.h>
#include <kassistantdialog.h>

using namespace Kross;

/********************************************************************
 * ScriptManagerAddTypeWidget
 */

ScriptManagerAddTypeWidget::ScriptManagerAddTypeWidget(ScriptManagerAddWizard* wizard, QWidget* parent)
    : QWidget(parent), m_wizard(wizard)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    setLayout(layout);
    layout->addWidget( new QLabel(i18n("<qt>This wizard will guide you through the proccess of adding a new item to your scripts.</qt>"), this) );
    layout->addSpacing(10);

    m_scriptCheckbox = new QRadioButton(i18n("Add script file"), this);
    m_scriptCheckbox->setChecked(true);
    connect(m_scriptCheckbox, SIGNAL(toggled(bool)), this, SLOT(slotUpdate()));
    layout->addWidget(m_scriptCheckbox);

    m_collectionCheckbox = new QRadioButton(i18n("Add collection folder"), this);
    layout->addWidget(m_collectionCheckbox);

    m_installCheckBox = new QRadioButton(i18n("Install script package file"), this);
    m_installCheckBox->setEnabled(false);
    layout->addWidget(m_installCheckBox);

    m_onlineCheckbox = new QRadioButton(i18n("Install online script package"), this);
    m_onlineCheckbox->setEnabled(false);
    layout->addWidget(m_onlineCheckbox);

    layout->addStretch(1);
}

void ScriptManagerAddTypeWidget::slotUpdate()
{
    m_wizard->m_dialog->setAppropriate(m_wizard->m_fileItem, m_scriptCheckbox->isChecked());
    m_wizard->m_dialog->setAppropriate(m_wizard->m_scriptItem, m_scriptCheckbox->isChecked());
    m_wizard->m_dialog->setAppropriate(m_wizard->m_collectionItem, m_collectionCheckbox->isChecked());
    //m_installCheckBox->isChecked()
    //m_onlineCheckbox->isChecked()
}

/********************************************************************
 * ScriptManagerAddFileWidget
 */

ScriptManagerAddFileWidget::ScriptManagerAddFileWidget(ScriptManagerAddWizard* wizard, QWidget* parent, const QString& startDirOrVariable)
    : QWidget(parent), m_wizard(wizard)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setMargin(0);
    setLayout(layout);
    m_filewidget = new FormFileWidget(this, startDirOrVariable);

    QStringList mimetypes;
    foreach(QString interpretername, Manager::self().interpreters()) {
        InterpreterInfo* info = Manager::self().interpreterInfo(interpretername);
        Q_ASSERT( info );
        mimetypes.append( info->mimeTypes().join(" ").trimmed() );
    }
    m_filewidget->setMimeFilter(mimetypes /*, defaultmime*/);

    layout->addWidget( m_filewidget );
    connect(m_filewidget, SIGNAL(fileHighlighted(const QString&)), this, SLOT(slotUpdate()));
    connect(m_filewidget, SIGNAL(fileSelected(const QString&)), this, SLOT(slotUpdate()));
}

void ScriptManagerAddFileWidget::slotUpdate()
{
    m_wizard->m_dialog->setValid(m_wizard->m_fileItem, ! m_filewidget->selectedFile().isEmpty());
}

/********************************************************************
 * ScriptManagerAddScriptWidget
 */

ScriptManagerAddScriptWidget::ScriptManagerAddScriptWidget(ScriptManagerAddWizard* wizard, QWidget* parent)
    : QWidget(parent), m_wizard(wizard)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    setLayout(layout);
    Action* action = new Action(0, "");
    m_editor = new ScriptManagerEditor(action, this);
    layout->addWidget(m_editor);
}

void ScriptManagerAddScriptWidget::slotUpdate()
{
    m_wizard->m_dialog->setValid(m_wizard->m_scriptItem, m_editor->isValid());
}

/********************************************************************
 * ScriptManagerAddCollectionWidget
 */

ScriptManagerAddCollectionWidget::ScriptManagerAddCollectionWidget(ScriptManagerAddWizard* wizard, QWidget* parent)
    : QWidget(parent), m_wizard(wizard)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    setLayout(layout);
    ActionCollection* collection = new ActionCollection("");
    m_editor = new ScriptManagerEditor(collection, this);
    layout->addWidget(m_editor);
}

void ScriptManagerAddCollectionWidget::slotUpdate()
{
    m_wizard->m_dialog->setValid(m_wizard->m_collectionItem, m_editor->isValid());
}

/********************************************************************
 * ScriptManagerAddWizard
 */

ScriptManagerAddWizard::ScriptManagerAddWizard(QWidget* parent)
    : QObject()
{
    m_dialog = new KAssistantDialog(parent);
    m_dialog->setCaption( i18n("Add") );

    ScriptManagerAddTypeWidget* typewidget = new ScriptManagerAddTypeWidget(this, m_dialog);
    m_typeItem = m_dialog->addPage(typewidget, i18n("Add"));

    const QString startDirOrVariable = "kfiledialog:///scriptmanageraddfile";
    ScriptManagerAddFileWidget* filewidget = new ScriptManagerAddFileWidget(this, m_dialog, startDirOrVariable);
    m_fileItem = m_dialog->addPage(filewidget, i18n("Script File"));

    ScriptManagerAddScriptWidget* scriptwidget = new ScriptManagerAddScriptWidget(this, m_dialog);
    m_scriptItem = m_dialog->addPage(scriptwidget, i18n("Script"));

    ScriptManagerAddCollectionWidget* collectionwidget = new ScriptManagerAddCollectionWidget(this, m_dialog);
    m_collectionItem = m_dialog->addPage(collectionwidget, i18n("Collection"));

    m_dialog->resize( QSize(620, 460).expandedTo( m_dialog->minimumSizeHint() ) );

    //connect(m_dialog, SIGNAL(currentPageChanged(KPageWidgetItem*,KPageWidgetItem*)), this, SLOT(slotUpdate()));
    typewidget->slotUpdate();
    filewidget->slotUpdate();
    scriptwidget->slotUpdate();
    collectionwidget->slotUpdate();
}

ScriptManagerAddWizard::~ScriptManagerAddWizard()
{
    delete m_dialog;
    m_dialog = 0;
}

int ScriptManagerAddWizard::execWizard()
{
    return m_dialog->exec();
}

#include "scriptmanageradd.moc"
