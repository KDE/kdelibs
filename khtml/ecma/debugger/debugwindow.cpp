/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 2006 Matt Broadstone (mbroadst@gmail.com)
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "kjs_proxy.h"

#include <ktoolbar.h>
#include <kstatusbar.h>
#include <klocale.h>
#include <kdebug.h>
#include <kicon.h>
#include <kiconloader.h>
#include <kglobal.h>
#include <kmessagebox.h>
#include <kguiitem.h>
#include <kmenu.h>
#include <kmenubar.h>
#include <kaction.h>
#include <kactioncollection.h>
#include <kglobalsettings.h>
#include <kshortcut.h>
#include <kconfig.h>
#include <kconfigbase.h>
#include <kapplication.h>
#include <kstringhandler.h>
#include <kstdaction.h>
#include <kxmlguifactory.h>

#include <ktexteditor/configinterface.h>
#include <ktexteditor/sessionconfiginterface.h>
#include <ktexteditor/modificationinterface.h>
#include <ktexteditor/editorchooser.h>
#include <ktexteditor/cursor.h>

#include "kjs_dom.h"
#include "kjs_binding.h"
#include "khtml_part.h"
#include "khtmlview.h"
#include "khtml_pagecache.h"
#include "khtml_settings.h"
#include "khtml_factory.h"
#include "misc/decoder.h"
#include <kjs/ustring.h>
#include <kjs/object.h>
#include <kjs/function.h>
#include <kjs/interpreter.h>
#include <kjs/value.h>

#include <QVBoxLayout>
#include <QSplitter>
#include <QDebug>

#include "numberedtextview.h"
#include "breakpointsdock.h"
#include "consoledock.h"
#include "localvariabledock.h"
#include "watchesdock.h"
#include "callstackdock.h"
#include "scriptsdock.h"

#include "debugwindow.h"
#include "debugwindow.moc"

using namespace KJS;

DebugWindow* DebugWindow::m_debugger = 0;

DebugWindow *DebugWindow::createInstance()
{
    Q_ASSERT(!m_debugger);
    m_debugger = new DebugWindow();
    return m_debugger;
}

void DebugWindow::destroyInstance()
{
    Q_ASSERT(m_debugger);
    m_debugger->hide();
    delete m_debugger;
}

DebugWindow * DebugWindow::window()
{
    return m_debugger;
}

// ----------------------------------------------

void DebugWindow::setNextSourceInfo(QString url, int baseLine)
{
    m_nextUrl = url;
    m_nextBaseLine = baseLine;
}

// ----------------------------------------------

DebugWindow::DebugWindow(QWidget *parent)
  : KMainWindow(parent, "DebugWindow", Qt::WType_TopLevel),
    KInstance("kjs_debugger")
{
    setCaption(i18n("JavaScript Debugger"));
    kDebug() << "creating DebugWindow" << endl;

//  Testing KTextEditor stuff
    m_sourceEdit = new NumberedTextView;

    m_editor = KTextEditor::EditorChooser::editor();

    if ( !m_editor )
    {
        KMessageBox::error(this, i18n("A KDE text-editor component could not be found;\n"
                                      "please check your KDE installation."));
        kapp->exit(1);
    }

    KTextEditor::Document *document = m_editor->createDocument(0);

    // enable the modified on disk warning dialogs if any
    if (qobject_cast<KTextEditor::ModificationInterface *>(document))
        qobject_cast<KTextEditor::ModificationInterface *>(document)->setModifiedOnDiskWarning(true);

    m_documentList.append(document);
    m_view = qobject_cast<KTextEditor::View*>(document->createView(this));

    // enable the modified on disk warning dialogs if any
    if (qobject_cast<KTextEditor::ConfigInterface*>(m_view))
    {
        KTextEditor::ConfigInterface *iface =qobject_cast<KTextEditor::ConfigInterface*>(m_view);
        if (iface->configKeys().contains("line-numbers"))
            iface->setConfigValue("line-numbers", true);
        if (iface->configKeys().contains("icon-bar"))
            iface->setConfigValue("icon-bar", true);
        if (iface->configKeys().contains("dynamic-word-wrap"))
            iface->setConfigValue("dynamic-word-wrap", true);
    }
//  End Testing

    m_watches = new WatchesDock;
    m_localVariables = new LocalVariablesDock;
    m_scripts = new ScriptsDock;
    m_callStack = new CallStackDock;
    m_breakpoints = new BreakpointsDock;
    m_console = new ConsoleDock;

    addDockWidget(Qt::LeftDockWidgetArea, m_scripts);
    addDockWidget(Qt::LeftDockWidgetArea, m_localVariables);
    addDockWidget(Qt::LeftDockWidgetArea, m_callStack);
    addDockWidget(Qt::LeftDockWidgetArea, m_breakpoints);
    addDockWidget(Qt::LeftDockWidgetArea, m_watches);

    QFrame *mainFrame = new QFrame;
    QVBoxLayout *layout = new QVBoxLayout(mainFrame);
    layout->setSpacing(0);
    QSplitter *splitter = new QSplitter(Qt::Vertical);
//    splitter->addWidget(m_sourceEdit);
    splitter->addWidget(m_view);
    splitter->addWidget(m_console);
    layout->addWidget(splitter);

    setCentralWidget(mainFrame);
    resize(800, 500);

    createActions();
    createMenus();
    createToolBars();
    createStatusBar();

    connect(m_scripts, SIGNAL(displayScript(KJS::DebugDocument*)),
            this, SLOT(displayScript(KJS::DebugDocument*)));
}

void DebugWindow::createActions()
{
    // Standard actions
    m_exitAct = new KAction(i18n("E&xit"), actionCollection(), "exit");
    m_exitAct->setShortcut(i18n("Ctrl+Q"));
    m_exitAct->setStatusTip(i18n("Exit the application"));
    connect(m_exitAct, SIGNAL(triggered()), this, SLOT(close()));

    // Flow control actions
    m_continueAct = new KAction(KIcon(":/images/continue.png"), i18n("Continue"), actionCollection(), "continue");
    m_continueAct->setStatusTip(i18n("Continue script execution"));
    m_continueAct->setToolTip(i18n("Continue script execution"));
    connect(m_continueAct, SIGNAL(triggered(bool)), this, SLOT(continueExecution()));

    m_stopAct = new KAction(KIcon(":/images/stop.png"), i18n("Stop"), actionCollection(), "stop");
    m_stopAct->setStatusTip(i18n("Stop script execution"));
    m_stopAct->setToolTip(i18n("Stop script execution"));
    connect(m_stopAct, SIGNAL(triggered(bool)), this, SLOT(stopExecution()));

    m_stepIntoAct = new KAction(KIcon(":/images/step-into.png"), i18n("Step Into"), actionCollection(), "stepInto");
    m_stepIntoAct->setStatusTip(i18n("Step Into"));
    m_stepIntoAct->setToolTip(i18n("Step Into"));
    connect(m_stepIntoAct, SIGNAL(triggered(bool)), this, SLOT(stepInto()));

    m_stepOutAct = new KAction(KIcon(":/images/step-out.png"), i18n("Step Out"), actionCollection(), "stepOut");
    m_stepOutAct->setStatusTip(i18n("Step Out"));
    m_stepOutAct->setToolTip(i18n("Step Out"));
    connect(m_stepOutAct, SIGNAL(triggered(bool)), this, SLOT(stepOut()) );

    m_stepOverAct = new KAction(KIcon(":/images/step-over.png"), i18n("Step Over"), actionCollection(), "stepOver");
    m_stepOverAct->setStatusTip(i18n("Step Over"));
    m_stepOverAct->setToolTip(i18n("Step Over"));
    connect(m_stepOverAct, SIGNAL(triggered(bool)), this, SLOT(stepOver()) );
}

void DebugWindow::createMenus()
{
/*
    KMenu *debugMenu = new KMenu(this);
    debugMenu->addAction(m_nextAction);
    debugMenu->addAction(m_stepAction);
    debugMenu->addAction(m_continueAction);
    debugMenu->addAction(m_breakAction);

    menuBar()->insertItem("&Debug", debugMenu);
*/
    KMenu *fileMenu = new KMenu(this);
    fileMenu->addAction(m_exitAct);
    menuBar()->insertItem("F&ile", fileMenu);
}

void DebugWindow::createToolBars()
{
    toolBar()->addAction(m_stopAct);
    toolBar()->addSeparator();
    toolBar()->addAction(m_continueAct);
    toolBar()->addAction(m_stepIntoAct);
    toolBar()->addAction(m_stepOutAct);
    toolBar()->addAction(m_stepOverAct);
}

// -------------------------------------------------------------
void DebugWindow::createStatusBar()
{
    statusBar()->showMessage(i18n("Ready"));
}

void DebugWindow::stopExecution()
{
//    KMessageBox::information(this, "Stop!");
}

void DebugWindow::continueExecution()
{
    KMessageBox::information(this, "Continue!");
}

void DebugWindow::stepInto()
{
    KMessageBox::information(this, "Step Into!");
}

void DebugWindow::stepOut()
{
    KMessageBox::information(this, "Step Out Of!");
}

void DebugWindow::stepOver()
{
    KMessageBox::information(this, "Step Over!");
}

DebugWindow::~DebugWindow()
{
}

// -------------------------------------------------------------
bool DebugWindow::sourceParsed(ExecState *exec, int sourceId, const UString &sourceURL,
                               const UString &source, int startingLineNumber, int errorLine, const UString &errorMsg)
{
    Q_UNUSED(exec);

    kDebug() << "Testing..." << endl;
    kDebug() << "***************************** sourceParsed **************************************************" << endl
             << "      sourceId: " << sourceId << endl
             << "     sourceURL: " << sourceURL.qstring() << endl
             << "     m_nextUrl: " << m_nextUrl << endl
             << "m_nextBaseLine: " << m_nextBaseLine << endl
             << "        source: " << source.qstring() << endl
             << "     errorLine: " << errorLine << endl
             << "*********************************************************************************************" << endl;

    // Determine key
    QString key = QString("%1|%2").arg((long)exec->dynamicInterpreter()).arg(m_nextUrl);

    DebugDocument *document = 0;
    if (!m_nextUrl.isEmpty())
        document = m_documents[key];
    if (!document)
    {
//        if (!m_nextUrl.isEmpty()) // Not in our cache, but has a URL
//        {

            document = new DebugDocument(m_nextUrl, exec->dynamicInterpreter());
            m_documents[key] = document;
   //         m_documents[key]->setFullSource(

//        }
    }
    else
    {
        // interpreter should already be there, if it isn't then we should look above to the problem
    }

    document->addCodeFragment(sourceId, m_nextBaseLine, source.qstring());
    m_scripts->addDocument(document);

    m_nextBaseLine = 1;
    m_nextUrl = "";

//    return (m_mode != Stop);
    return true;
}

bool DebugWindow::sourceUnused(ExecState *exec, int sourceId)
{
    Q_UNUSED(exec);
    Q_UNUSED(sourceId);

    kDebug() << "sourceUnused" << endl;

    return true;
}

bool DebugWindow::exception(ExecState *exec, int sourceId, int lineno, JSObject *exceptionObj)
{
    Q_UNUSED(exec);
    Q_UNUSED(sourceId);
    Q_UNUSED(lineno);
    Q_UNUSED(exceptionObj);

    kDebug() << "exception" << endl;

    return true;
}

bool DebugWindow::atStatement(ExecState *exec, int sourceId, int firstLine, int lastLine)
{
    Q_UNUSED(exec);
    Q_UNUSED(sourceId);
    Q_UNUSED(firstLine);
    Q_UNUSED(lastLine);

    kDebug() << "atStatement" << endl;

    return true;
}

bool DebugWindow::callEvent(ExecState *exec, int sourceId, int lineno, JSObject *function, const List &args)
{
    Q_UNUSED(exec);
    Q_UNUSED(sourceId);
    Q_UNUSED(lineno);
    Q_UNUSED(function);
    Q_UNUSED(args);

    kDebug() << "callEvent" << endl;

    return true;
}

bool DebugWindow::returnEvent(ExecState *exec, int sourceId, int lineno, JSObject *function)
{
    Q_UNUSED(exec);
    Q_UNUSED(sourceId);
    Q_UNUSED(lineno);
    Q_UNUSED(function);

    kDebug() << "returnEvent" << endl;

    return true;
}

void DebugWindow::displayScript(KJS::DebugDocument *document)
{
    QList<SourceFragment> fragments = document->code();
    foreach (SourceFragment fragment, fragments)
    {
        int line = fragment.baseLine;
        int col = 0;

        KTextEditor::Cursor cur = m_view->cursorPosition();
        cur.setPosition(line, col);
        m_view->insertText(fragment.source);
    }

/*
    KTextEditor::Cursor cur = m_view->cursorPosition();
    cur.setPosition(0, 0);
    m_view->insertText(document->source());
*/
}

