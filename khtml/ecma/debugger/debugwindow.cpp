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

#include "debugwindow.h"
#include "debugwindow.moc"

#include <QSharedData>
#include <QDebug>

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
#include <kapplication.h>
#include <kstringhandler.h>
#include <kstandardaction.h>
#include <kxmlguifactory.h>

#include <ktexteditor/configinterface.h>
#include <ktexteditor/sessionconfiginterface.h>
#include <ktexteditor/modificationinterface.h>
#include <ktexteditor/editorchooser.h>
#include <ktexteditor/cursor.h>

#include "kjs_proxy.h"
#include "kjs_dom.h"
#include "kjs_binding.h"
#include "khtml_part.h"
#include "khtmlview.h"
#include "khtml_pagecache.h"
#include "khtml_settings.h"
#include "khtml_factory.h"
#include <kjs/ustring.h>
#include <kjs/object.h>
#include <kjs/function.h>
#include <kjs/interpreter.h>
#include <kjs/value.h>

#include <QVBoxLayout>
#include <QSplitter>
#include <QTabWidget>
#include <QToolButton>

#include "breakpointsdock.h"
#include "consoledock.h"
#include "localvariabledock.h"
#include "watchesdock.h"
#include "callstackdock.h"
#include "scriptsdock.h"

#include "debugdocument.h"

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
  : KXmlGuiWindow(parent, Qt::Window),
    KComponentData("kjs_debugger")
{
    setObjectName(QLatin1String("DebugWindow"));
    setCaption(i18n("JavaScript Debugger"));
    kDebug() << "creating DebugWindow";

    m_editor = KTextEditor::EditorChooser::editor();
    if ( !m_editor )
    {
        KMessageBox::error(this, i18n("A KDE text-editor component could not be found;\n"
                                      "please check your KDE installation."));
        kapp->exit(1);
    }

//     m_watches = new WatchesDock;
    m_localVariables = new LocalVariablesDock;
    m_scripts = new ScriptsDock;
    m_callStack = new CallStackDock;
    m_breakpoints = new BreakpointsDock;
    m_console = new ConsoleDock;
    m_docFrame = new QFrame;

    addDockWidget(Qt::LeftDockWidgetArea, m_scripts);
    addDockWidget(Qt::LeftDockWidgetArea, m_localVariables);
    addDockWidget(Qt::LeftDockWidgetArea, m_callStack);
    addDockWidget(Qt::LeftDockWidgetArea, m_breakpoints);
//     addDockWidget(Qt::LeftDockWidgetArea, m_watches);

    QFrame *mainFrame = new QFrame;
    QVBoxLayout *layout = new QVBoxLayout(mainFrame);
    layout->setSpacing(0);
    QSplitter *splitter = new QSplitter(Qt::Vertical);
    splitter->addWidget(m_docFrame);
    splitter->addWidget(m_console);
    splitter->setStretchFactor(0, 10);
    splitter->setStretchFactor(1, 1);
    layout->addWidget(splitter);

    setCentralWidget(mainFrame);
    resize(800, 500);

    createActions();
    createMenus();
    createToolBars();
    createStatusBar();
    createTabWidget();
    m_tabWidget->hide();

    connect(m_scripts, SIGNAL(displayScript(KJS::DebugDocument*)),
            this, SLOT(displayScript(KJS::DebugDocument*)));

    m_inSession = false;
}

void DebugWindow::createActions()
{
    // Flow control actions
    m_continueAct = new KAction(KIcon(":/images/continue.png"), i18n("Continue"), this );
    actionCollection()->addAction( "continue", m_continueAct );
    m_continueAct->setStatusTip(i18n("Continue script execution"));
    m_continueAct->setToolTip(i18n("Continue script execution"));
    m_continueAct->setEnabled(false);
    connect(m_continueAct, SIGNAL(triggered(bool)), this, SLOT(continueExecution()));

    m_stopAct = new KAction(KIcon(":/images/stop.png"), i18n("Stop"), this );
    actionCollection()->addAction( "stop", m_stopAct );
    m_stopAct->setStatusTip(i18n("Stop script execution"));
    m_stopAct->setToolTip(i18n("Stop script execution"));
    m_stopAct->setEnabled(false);
    connect(m_stopAct, SIGNAL(triggered(bool)), this, SLOT(stopExecution()));

    m_stepIntoAct = new KAction(KIcon(":/images/step-into.png"), i18n("Step Into"), this );
    actionCollection()->addAction( "stepInto", m_stepIntoAct );
    m_stepIntoAct->setStatusTip(i18n("Step Into"));
    m_stepIntoAct->setToolTip(i18n("Step Into"));
    m_stepIntoAct->setEnabled(false);
    connect(m_stepIntoAct, SIGNAL(triggered(bool)), this, SLOT(stepInto()));

    m_stepOutAct = new KAction(KIcon(":/images/step-out.png"), i18n("Step Out"), this );
    actionCollection()->addAction( "stepOut", m_stepOutAct );
    m_stepOutAct->setStatusTip(i18n("Step Out"));
    m_stepOutAct->setToolTip(i18n("Step Out"));
    m_stepOutAct->setEnabled(false);
    connect(m_stepOutAct, SIGNAL(triggered(bool)), this, SLOT(stepOut()) );

    m_stepOverAct = new KAction(KIcon(":/images/step-over.png"), i18n("Step Over"), this );
    actionCollection()->addAction( "stepOver", m_stepOverAct );
    m_stepOverAct->setStatusTip(i18n("Step Over"));
    m_stepOverAct->setToolTip(i18n("Step Over"));
    m_stepOverAct->setEnabled(false);
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
    // ### KDE4.1: proper debug menu. Don't want to 
    // add strings right now.
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

void DebugWindow::createTabWidget()
{
    QVBoxLayout *layout = new QVBoxLayout(m_docFrame);
    m_tabWidget = new QTabWidget;

    QToolButton *closeTabButton = new QToolButton(m_tabWidget);
    m_tabWidget->setCornerWidget(closeTabButton, Qt::TopRightCorner);
    closeTabButton->setCursor(Qt::ArrowCursor);
    closeTabButton->setAutoRaise(true);
    closeTabButton->setIcon(QIcon(":/images/removetab.png"));
    connect(closeTabButton, SIGNAL(clicked()), this, SLOT(closeTab()));
    closeTabButton->setToolTip(i18n("Close source"));
    closeTabButton->setEnabled(true);
    layout->addWidget(m_tabWidget);
}

// -------------------------------------------------------------
void DebugWindow::createStatusBar()
{
    statusBar()->showMessage(i18n("Ready"));
}

void DebugWindow::stopExecution()
{
    m_mode = Stop;
}

void DebugWindow::continueExecution()
{
    m_continueAct->setEnabled(false);
    m_stopAct->setEnabled(false);
    m_stepIntoAct->setEnabled(false);
    m_stepOutAct->setEnabled(false);
    m_stepOverAct->setEnabled(false);

    exitLoop();
    m_mode = Continue;
    m_inSession = false;
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
    while (!m_documentList.isEmpty())
        delete m_documentList.takeFirst();

    m_documentList.clear();
}

// -------------------------------------------------------------
bool DebugWindow::sourceParsed(ExecState *exec, int sourceId, const UString &sourceURL,
                               const UString &source, int startingLineNumber , int errorLine, const UString &/* errorMsg */)
{
    Q_UNUSED(exec);

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
        document = new DebugDocument(m_nextUrl, exec->dynamicInterpreter());
        m_documents[key] = document;
        m_sourceIdLookup[sourceId] = document;
    }


    if (document)
    {
        document->addCodeFragment(sourceId, m_nextBaseLine, source.qstring());
        m_scripts->addDocument(document);

        m_nextBaseLine = 0;
        m_nextUrl = "";
    }

    return (m_mode != Stop);
}

bool DebugWindow::sourceUnused(ExecState *exec, int sourceId)
{
    Q_UNUSED(exec);
    Q_UNUSED(sourceId);

    // Remove the debug document associated with this sourceId
    DebugDocument *document = m_sourceIdLookup[sourceId];
    if (document)
    {
        m_scripts->documentDestroyed(document);
        if (!document->deleteFragment(sourceId))   // this means we've removed all the source fragments
            delete document;
        m_sourceIdLookup.remove(sourceId);
    }

    return (m_mode != Stop);
}

bool DebugWindow::exception(ExecState *exec, int sourceId, int lineno, JSObject *exceptionObj)
{
    Q_UNUSED(exec);
    Q_UNUSED(sourceId);
    Q_UNUSED(lineno);
    Q_UNUSED(exceptionObj);

    kDebug() << "exception";

    return (m_mode != Stop);
}


// This is where we are going to check for a breakpoint. First check for breakpoint
// then if one is found, stop execution and display local variables in the localVariables
// dock.

bool DebugWindow::atStatement(ExecState *exec, int sourceId, int firstLine, int lastLine)
{
    Q_UNUSED(exec);
    Q_UNUSED(sourceId);
    Q_UNUSED(firstLine);
    Q_UNUSED(lastLine);

//     kDebug() << "***************************** atStatement ***************************************************" << endl
//              << "      sourceId: " << sourceId << endl
//              << "     firstLine: " << firstLine << endl
//              << "      lastLine: " << lastLine << endl;

    DebugDocument *document = m_sourceIdLookup[sourceId];
    if (document)
    {
//         kDebug() << "found document for sourceId";
//         QVector<int> bpoints = document->breakpoints();
//         foreach (int bpoint, bpoints)
//         {
//             kDebug() << " > " << bpoint;
//         }

/*
        int numLines = lastLine - firstLine;
        for (int i = 0; i < numLines; i++)
        {
            int lineNumber = firstLine + i;
            kDebug() << "breakpoint at line " << lineNumber << "?";
            if (document->hasBreakpoint(lineNumber))
            {
                kDebug() << "Hey! we actually found a breakpoint!";
                // Lets try a dump of the scope chain now..
                m_localVariables->display(exec->dynamicInterpreter());
            }
      }
*/
        if (document->hasBreakpoint(firstLine))
        {
//             kDebug() << "Hey! we actually found a breakpoint!";
            // Lets try a dump of the scope chain now..
            // m_localVariables->display(exec);
            enterDebugSession(exec, document);
        }
    }

//     kDebug() << "*********************************************************************************************";

    return (m_mode != Stop);
}

bool DebugWindow::callEvent(ExecState *exec, int sourceId, int lineno, JSObject *function, const List &args)
{
    kDebug() << "***************************** callEvent **************************************************";
    kDebug() << "  sourceId: " << sourceId << endl
             << "lineNumber: " << lineno << endl;

    DebugDocument *document = m_sourceIdLookup[sourceId];
    if (document)
    {
        if (function->inherits(&InternalFunctionImp::info))
        {
            KJS::InternalFunctionImp *func = static_cast<InternalFunctionImp*>(function);
            if (func)
            {
                QString functionName = func->functionName().qstring();
                kDebug() << "  function: " << functionName;
                func = 0;

                document->addCall(functionName, lineno);
            }
        }

        for( KJS::ListIterator item = args.begin();
            item != args.end();
            ++item)
        {
            KJS::JSValue *value = (*item);
            kDebug() << "arg: " << value->toString(exec).qstring();
        }
    }


    kDebug() << "****************************************************************************************";

    return (m_mode != Stop);
}

bool DebugWindow::returnEvent(ExecState *exec, int sourceId, int lineno, JSObject *function)
{
    kDebug() << "***************************** returnEvent **************************************************";
    kDebug() << "  sourceId: " << sourceId << endl
             << "lineNumber: " << lineno << endl;

    Q_UNUSED( exec );
    DebugDocument *document = m_sourceIdLookup[sourceId];
    if (document)
    {
        if (function->inherits(&InternalFunctionImp::info))
        {
            KJS::InternalFunctionImp *func = static_cast<InternalFunctionImp*>(function);
            if (func)
            {
                QString functionName = func->functionName().qstring();
                kDebug() << "  function: " << functionName;
                func = 0;

                document->removeCall(functionName, lineno);
            }
        }
    }

    kDebug() << "****************************************************************************************";

    return (m_mode != Stop);
}

// End KJS::Debugger overloads


void DebugWindow::enableKateHighlighting(KTextEditor::Document *document)
{
    if (!document)
        return;

    document->setMode("JavaScript");
}

void DebugWindow::displayScript(KJS::DebugDocument *document)
{
    if (m_tabWidget->isHidden())
        m_tabWidget->show();
        
    if (m_openDocuments.contains(document))
    {
        int idx = m_openDocuments.indexOf(document);
        m_tabWidget->setCurrentIndex(idx);
        return;
    }

    KTextEditor::Document *doc = 0;
    doc = m_debugLut[document];     // Check to see if we've already worked on this document
    if (!doc)
    {
        doc = m_editor->createDocument(0);
        m_documentList.append(doc);

        m_debugLut[document] = doc;
        m_documentLut[doc] = document;
    }

    enableKateHighlighting(doc);
    QList<SourceFragment> fragments = document->fragments();

    // Below makes the assumption that we parse top->bottom....
    // ...which is wrong w/lazily parsed event handles. urgh.
    // So the list should actually be sorted, w/some sort of left-to
    // right ordering, too.
    doc->setReadWrite(true);
    doc->clear();
    
    foreach (SourceFragment fragment, fragments)
    {
        // Note: the KTextEditor interface counts the lines/columns from 0,
        // but the UI shows them from 1,1.
        // KHTML appears to report lines from 1 up.
        int line = fragment.baseLine - 1;
        if (line < 0)
            line = 0;

        // We have to be a bit careful here, since
        // in an ultra-stupid HTML documents, there may be more than
        // one script tag on a line. So we try to append things. 
        QString source = fragment.source + "  ";

        if (line == doc->lines() - 1) {
            // We want to append to the end, so join up with the line, 
            // remove it, and then re-append the whole chunk
            source = doc->line(line) + source;
            doc->removeLine(line);
        }
        
        // Insert enough blank lines to get us to the end. Kind of sucks.
        while (doc->lines() - 1 < line) {
            doc->insertLine(doc->lines(), "");
        }
        
        // ### can we guarantee this as a separator? probably not
        doc->insertLines(line, source.split("\n"));
    }

    KTextEditor::View *view = qobject_cast<KTextEditor::View*>(doc->createView(this));
    KTextEditor::ConfigInterface *configInterface = qobject_cast<KTextEditor::ConfigInterface*>(view);
    if (configInterface)
    {
        if (configInterface->configKeys().contains("line-numbers"))
            configInterface->setConfigValue("line-numbers", true);
        if (configInterface->configKeys().contains("icon-bar"))
            configInterface->setConfigValue("icon-bar", true);
        if (configInterface->configKeys().contains("dynamic-word-wrap"))
            configInterface->setConfigValue("dynamic-word-wrap", true);
    }

    KTextEditor::MarkInterface *markInterface = qobject_cast<KTextEditor::MarkInterface*>(doc);
    if (markInterface)
    {
        markInterface->setEditableMarks(KTextEditor::MarkInterface::BreakpointActive);
        connect(doc, SIGNAL(markChanged(KTextEditor::Document*, KTextEditor::Mark, KTextEditor::MarkInterface::MarkChangeAction)),
                this, SLOT(markSet(KTextEditor::Document*, KTextEditor::Mark, KTextEditor::MarkInterface::MarkChangeAction)));
    }

    doc->setReadWrite(false);
    m_openDocuments.append(document);
    int idx = m_tabWidget->addTab(view, document->name());
    m_tabWidget->setCurrentIndex(idx);
}

void DebugWindow::markSet(KTextEditor::Document *document, KTextEditor::Mark mark,
                          KTextEditor::MarkInterface::MarkChangeAction action)
{
    DebugDocument *debugDocument = m_documentLut[document];
    if (!debugDocument)
        return;

    int lineNumber = mark.line + 1;         // we do this because bookmarks are technically set
                                            // the line before what looks like the line you chose..

    switch(action)
    {
        case KTextEditor::MarkInterface::MarkAdded:
            debugDocument->setBreakpoint(lineNumber);
            break;
        case KTextEditor::MarkInterface::MarkRemoved:
            debugDocument->removeBreakpoint(lineNumber);
            break;
    }

    kDebug() << "breakpoint set for: " << endl
             << "document: " << document->documentName() << endl
             << "line: " << lineNumber << " type: " << mark.type << endl;

    kDebug() << "breakpoints at lines:";
    QVector<int> bpoints = debugDocument->breakpoints();
    foreach (int bpoint, bpoints)
        kDebug() << " > " << bpoint;
}

void DebugWindow::closeTab()
{
    int idx = m_tabWidget->currentIndex();
    m_tabWidget->removeTab(idx);
    m_openDocuments.removeAt(idx);
    if (m_openDocuments.isEmpty())
        m_tabWidget->hide();
}


void DebugWindow::enterDebugSession(KJS::ExecState *exec, DebugDocument *document)
{
    // This "enters" a new debugging session, i.e. enables usage of the debugging window
    // It re-enters the qt event loop here, allowing execution of other parts of the
    // program to continue while the script is stopped. We have to be a bit careful here,
    // i.e. make sure the user can't quit the app, and disable other event handlers which
    // could interfere with the debugging session.

    if (!isVisible())
        show();

    m_mode = Stop;
//    if (m_execStates.isEmpty())
    {
        m_continueAct->setEnabled(true);
        m_stopAct->setEnabled(true);
        m_stepIntoAct->setEnabled(true);
        m_stepOutAct->setEnabled(true);
        m_stepOverAct->setEnabled(true);
    }

    m_callStack->displayStack(document);
    m_localVariables->display(exec);

    m_inSession = true;
    enterLoop();
}




//// Event handling - ripped from old kjsdebugger

bool DebugWindow::eventFilter(QObject *object, QEvent *event)
{
    switch (event->type())
    {
        case QEvent::MouseButtonPress:
        case QEvent::MouseButtonRelease:
        case QEvent::MouseButtonDblClick:
        case QEvent::MouseMove:
        case QEvent::KeyPress:
        case QEvent::KeyRelease:
        case QEvent::Destroy:
        case QEvent::Close:
        case QEvent::Quit:
            {
                while (object->parent())
                    object = object->parent();
                if (object == this)
                    return QWidget::eventFilter(object, event);
                else
                    return true;
            }
            break;
        default:
            return QWidget::eventFilter(object, event);
    }

}

void DebugWindow::enterLoop()
{
    QEventLoop eventLoop;
    connect(this, SIGNAL(quitLoop()), &eventLoop, SLOT(quit()));
    enterModality();

//    eventLoop.exec(QEventLoop::X11ExcludeTimers | QEventLoop::ExcludeSocketNotifiers);
    eventLoop.exec();
}

void DebugWindow::exitLoop()
{
    leaveModality();
    emit quitLoop();
}

void DebugWindow::enterModality()
{
    QWidgetList widgets = QApplication::allWidgets();
    foreach (QWidget *widget, widgets)
        widget->installEventFilter(this);
}

void DebugWindow::leaveModality()
{
    QWidgetList widgets = QApplication::allWidgets();
    foreach (QWidget *widget, widgets)
        widget->removeEventFilter(this);
}

// kate: indent-width 4; replace-tabs on; tab-width 4; space-indent on;
