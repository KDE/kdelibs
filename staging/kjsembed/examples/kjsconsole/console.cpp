/* This file is part of the KDE libraries
    Copyright (C) 2005, 2006 Ian Reinhart Geiser <geiseri@kde.org>
    Copyright (C) 2005, 2006 Matt Broadstone <mbroadst@gmail.com>
    Copyright (C) 2005, 2006 Richard J. Moore <rich@kde.org>
    Copyright (C) 2005, 2006 Erik L. Bunce <kde@bunce.us>

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

#include "console.h"

#include <math.h>

#include <QtCore/QDebug>
#include <QFileDialog>
#include <QtCore/QFile>

#include <kjsembed/kjseglobal.h>
#include <kjs/interpreter.h>
#include <kjs/ustring.h>
#include <kjs/object.h>
#include <kjs/JSVariableObject.h>

#include "ui_jsconsole.h"
#include "kjs_object_model.h"

Ui::KJSConsole m_ui;

Console::Console( QWidget *parent ) :
    QMainWindow( parent )
{
    KJS::Interpreter *js = mKernel.interpreter();
    KJS::JSObject *obj = js->globalObject();
    m_model = new KJSObjectModel(js, this);
    m_ui.setupUi(this);
    m_ui.mObjectModel->setModel(m_model);
    m_model->updateModel(obj);

    connect(m_ui.mCommand, SIGNAL(activated(QString)), this, SLOT(on_mExecute_clicked()));
}

Console::~Console()
{
}

QString errorTemplate = "<font color='#FF0000'>%1</font>";

void Console::on_mExecute_clicked()
{
    KJS::Interpreter *js = mKernel.interpreter();
    KJS::ExecState *exec = js->globalExec();

    KJSEmbed::Engine::ExitStatus result = mKernel.execute(m_ui.mCommand->currentText());
    KJS::Completion jsres = mKernel.completion();
    m_ui.mConsole->append(m_ui.mCommand->currentText());
    KJS::JSValue *value = jsres.value();
    if ( result != KJSEmbed::Engine::Success )
    {
        m_ui.mConsole->append(errorTemplate.arg( KJSEmbed::toQString(jsres.value()->toString(exec)) ));
    }
    else
    {
        if(value)
            m_ui.mConsole->append( KJSEmbed::toQString(jsres.value()->toString(exec) ));
    }
    KJS::JSObject *obj = js->globalObject();
    m_model->updateModel(obj);
    m_ui.mCommand->clearEditText();
}

void Console::on_actionOpenScript_activated()
{
    QString m_lastDir;
    QString openFile = QFileDialog::getOpenFileName(this, tr("Select script to open..."),
            m_lastDir, tr("Scripts (*.js *.kjs *.qjs)"));

    if( openFile.isEmpty() )
        return;

    QString code;
    QFile fIn(openFile);

    if (!fIn.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    while (!fIn.atEnd())
    {
        QByteArray line = fIn.readLine();
        code += line;
    }

    m_ui.mInput->setText(code);
}

void Console::on_actionCloseScript_activated()
{
}

void Console::on_actionQuit_activated()
{
    close();
}

void Console::on_actionRun_activated()
{
    KJS::Interpreter *js = mKernel.interpreter();
    KJS::ExecState *exec = js->globalExec();

    KJSEmbed::Engine::ExitStatus result = mKernel.execute(m_ui.mInput->text());
    KJS::Completion jsres = mKernel.completion();
    KJS::JSValue *value = jsres.value();
    if ( result != KJSEmbed::Engine::Success )
    {
        m_ui.mConsole->append(errorTemplate.arg( KJSEmbed::toQString(jsres.value()->toString(exec)) ) );
    }
    else
    {
        if(value)
            m_ui.mConsole->append( KJSEmbed::toQString(jsres.value()->toString(exec)));
    }
    KJS::JSObject *obj = js->globalObject();
    m_model->updateModel(obj);
}

void Console::on_actionRunTo_activated()
{
}
void Console::on_actionStep_activated()
{
}

void Console::on_actionStop_activated()
{
}


