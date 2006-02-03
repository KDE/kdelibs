#include <math.h>

#include <QDebug>
#include <QFileDialog>
#include <QFile>

#include <kjs/interpreter.h>
#include <kjs/reference.h>
#include <kjs/ustring.h>
#include <kjs/object.h>

#include "console.h"
#include "jsconsole.h"
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

    connect(m_ui.mCommand, SIGNAL(activated(const QString&)), this, SLOT(on_mExecute_clicked())); 
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
        m_ui.mConsole->append(errorTemplate.arg( jsres.value()->toString(exec).qstring() ) );
    }
    else
    {
        if(value)
            m_ui.mConsole->append( jsres.value()->toString(exec).qstring() );
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
        m_ui.mConsole->append(errorTemplate.arg( jsres.value()->toString(exec).qstring() ) );
    }
    else
    {
        if(value)
            m_ui.mConsole->append( jsres.value()->toString(exec).qstring() );
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

