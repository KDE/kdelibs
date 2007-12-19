/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 2006 Matt Broadstone (mbroadst@gmail.com)
 *  Copyright (C) 2007 Maks Orlovich <maksim@kde.org>
 *  Copyright (C) 2000-2001 Harri Porten (porten@kde.org)
 *  Copyright (C) 2001,2003 Peter Kelly (pmk@post.com)
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
#include <kjs/context.h>

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
#include "value2string.h"
#include "errordlg.h"

using namespace KJS;
using namespace KJSDebugger;

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
    //m_breakpoints = new BreakpointsDock;
    m_console = new ConsoleDock;
    m_docFrame = new QFrame;

    addDockWidget(Qt::LeftDockWidgetArea, m_scripts);
    addDockWidget(Qt::LeftDockWidgetArea, m_localVariables);
    addDockWidget(Qt::LeftDockWidgetArea, m_callStack);
    //addDockWidget(Qt::LeftDockWidgetArea, m_breakpoints);
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

    connect(m_scripts, SIGNAL(displayScript(KJSDebugger::DebugDocument*)),
            this, SLOT(displayScript(KJSDebugger::DebugDocument*)));

    m_breakAtNext  = false;
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
    m_stopAct->setEnabled(true);
    // ### Actually we use this for stop-at-next
    connect(m_stopAct, SIGNAL(triggered(bool)), this, SLOT(stopAtNext()));

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
    closeTabButton->setIcon(KIcon("tab-close"));
    connect(closeTabButton, SIGNAL(clicked()), this, SLOT(closeTab()));
    closeTabButton->setToolTip(i18n("Close source"));
    closeTabButton->setEnabled(true);
    layout->addWidget(m_tabWidget);
}

void DebugWindow::createStatusBar()
{
    statusBar()->showMessage(i18n("Ready"));
}

void DebugWindow::updateStoppedMark(RunMode mode)
{
    // Figure out where we're stopped or stoping.
    KTextEditor::MarkInterface* imark = 0;
    KTextEditor::View*          view  = 0;
    int                         line  = 0;

    if (ctx() && ctx()->activeDocument)
    {
        DebugDocument* document = ctx()->activeDocument;
        KTextEditor::Document* ddoc = m_debugLut[document];
        view  = viewForDocument(document);
        imark = qobject_cast<KTextEditor::MarkInterface*>(ddoc);
        line  = ctx()->activeLine() - 1; // -1 since KTextEditor 0-based, other 1-based
    }

    if (mode == Running)
    {
        // No longer stopped there.
        if (imark)
            imark->removeMark(line, KTextEditor::MarkInterface::Execution);
    }
    else
    {
        // Display the source file, and visualize the position
        displayScript(ctx()->activeDocument);
        view->setCursorPosition(KTextEditor::Cursor(line, 0));
        if (imark)
            imark->addMark(line, KTextEditor::MarkInterface::Execution);
    }
}

void DebugWindow::setUIMode(RunMode mode)
{
    if (mode == Running)
    {
        // Clear local variable and stack display..
        m_localVariables->updateDisplay(0);
        m_callStack->clearDisplay();

        // Toggle buttons..
        m_continueAct->setEnabled(false);
        m_stopAct->setEnabled(true);
        m_stepIntoAct->setEnabled(false);
        m_stepOutAct->setEnabled(false);
        m_stepOverAct->setEnabled(false);
    }
    else
    {
        // Show local variables and the bt
        m_localVariables->updateDisplay(ctx()->execContexts.top());
        m_callStack->displayStack(ctx());

        // Toggle buttons..
        m_continueAct->setEnabled(true);
        m_stopAct->setEnabled(false);
        m_stepIntoAct->setEnabled(true);
        m_stepOutAct->setEnabled(true);
        m_stepOverAct->setEnabled(true);
    }

    updateStoppedMark(mode);
}


// -------------------------------------------------------------

void DebugWindow::stopAtNext()
{
    m_breakAtNext = true;
}

bool DebugWindow::shouldContinue(InterpreterContext* ic)
{
    return !ic || ic->mode != Abort;
}

void DebugWindow::leaveDebugSession()
{
    // Update UI for running mode, unless we expect things to
    // be quick; in which case we'll only update if we have to, due to
    // finishing running
    if (ctx()->mode != Step)
        setUIMode(Running);
    else  // In the other case, we still want to remove the old running marker, however
        updateStoppedMark(Running);
    m_activeSessionCtxs.pop();

    resumeTimeoutChecks();

    // There may be a previous session in progress --- in
    // that case we need to update the UI to reflect that.
    if (!m_activeSessionCtxs.isEmpty())
        setUIMode(Running);

    exitLoop();
}

void DebugWindow::pauseTimeoutChecks()
{
    KJS::Interpreter* intp = KJS::Interpreter::firstInterpreter();
    do
    {
        intp->pauseTimeoutCheck();
        intp = intp->nextInterpreter();
    }
    while (intp != KJS::Interpreter::firstInterpreter());
}

void DebugWindow::resumeTimeoutChecks()
{
    KJS::Interpreter* intp = KJS::Interpreter::firstInterpreter();
    do
    {
        intp->resumeTimeoutCheck();
        intp = intp->nextInterpreter();
    }
    while (intp != KJS::Interpreter::firstInterpreter());
}

void DebugWindow::continueExecution()
{
    ctx()->mode   = Normal;
    m_breakAtNext = false;
    leaveDebugSession();
}

void DebugWindow::stepInto()
{
    ctx()->mode = Step;
    leaveDebugSession();
}

void DebugWindow::stepOut()
{
    ctx()->mode        = StepOut;
    ctx()->depthAtSkip = ctx()->execContexts.size();
    leaveDebugSession();
}

void DebugWindow::stepOver()
{
    ctx()->mode        = StepOver;
    ctx()->depthAtSkip = ctx()->execContexts.size();
    leaveDebugSession();
}

DebugWindow::~DebugWindow()
{
    qDeleteAll(m_documentList);
    m_documentList.clear();
}

// -------------------------------------------------------------

void DebugWindow::attach(Interpreter *interp)
{
    m_contexts[interp] = new InterpreterContext;
}

void DebugWindow::detach(Interpreter *interp)
{
    if (interp)
    {
        delete m_contexts.take(interp);
    }
    else //All
    {
        qDeleteAll(m_contexts);
        m_contexts.clear();
    }
}


bool DebugWindow::sourceParsed(ExecState *exec, int sourceId, const UString &sourceURL,
                               const UString &source, int startingLineNumber , int errorLine, const UString &/* errorMsg */)
{
    Q_UNUSED(exec);

    kDebug() << "***************************** sourceParsed **************************************************" << endl
             << "      sourceId: " << sourceId << endl
             << "     sourceURL: " << sourceURL.qstring() << endl
             << "startingLineNumber: " << startingLineNumber << endl
             << "     errorLine: " << errorLine << endl
             << "*********************************************************************************************" << endl;

    // Determine key
    QString key = QString("%1|%2").arg((long)exec->dynamicInterpreter()).arg(sourceURL.qstring());

    DebugDocument *document = 0;
    if (!sourceURL.isEmpty())
        document = m_documents[key];
    else
        key = key + "|" + sourceId; // Give separate entries for EvalCode 
                                    // and function-ctor created things;
                                    // this is quite suboptimal, unfortunately
    if (!document)
    {
        document = new DebugDocument(sourceURL.qstring(), exec->dynamicInterpreter());
        m_documents[key] = document;
    }

    m_sourceIdLookup[sourceId] = document;

    document->addCodeFragment(sourceId, startingLineNumber, source.qstring());
    m_scripts->addDocument(document);
    buildViewerDocument(document);

    return shouldContinue(m_contexts[exec->dynamicInterpreter()]);
}

bool DebugWindow::sourceUnused(ExecState *exec, int sourceId)
{
    Q_UNUSED(exec);

    // Remove the debug document associated with this sourceId
    DebugDocument *document = m_sourceIdLookup[sourceId];
    if (document)
    {
        m_scripts->documentDestroyed(document);
        if (!document->deleteFragment(sourceId))   // this means we've removed all the source fragments
            delete document;
        m_sourceIdLookup.remove(sourceId);
    }

    return shouldContinue(m_contexts[exec->dynamicInterpreter()]);
}

bool DebugWindow::exception(ExecState *exec, int sourceId, int lineNo, JSValue *exceptionObj)
{
    // Fixup the line..
    lineNo = lineNo - 1;

    // Don't report it if error reporting is not on
    KParts::ReadOnlyPart *part = static_cast<ScriptInterpreter*>(exec->dynamicInterpreter())->part();
    KHTMLPart *khtmlpart = qobject_cast<KHTMLPart*>(part);
    if (khtmlpart && !khtmlpart->settings()->isJavaScriptErrorReportingEnabled())
        return shouldContinue(m_contexts[exec->dynamicInterpreter()]);

    // ### adjust to have m_evalDepth, for console, like in KDE3 version
    QString exceptionMsg = valueToString(exceptionObj);

    // Since we purposefully bypass toString, we need to figure out
    // string serialization ourselves.
    //### might be easier to export class info for ErrorInstance --- 

    JSObject* valueObj = exceptionObj->getObject();
    JSValue*  protoObj = valueObj ? valueObj->prototype() : 0;

    bool exception   = false;
    bool syntaxError = false;
    if (protoObj == exec->lexicalInterpreter()->builtinSyntaxErrorPrototype())
    {
        exception   = true;
        syntaxError = true;
    }

    if (protoObj == exec->lexicalInterpreter()->builtinErrorPrototype()          ||
        protoObj == exec->lexicalInterpreter()->builtinEvalErrorPrototype()      ||
        protoObj == exec->lexicalInterpreter()->builtinReferenceErrorPrototype() ||
        protoObj == exec->lexicalInterpreter()->builtinRangeErrorPrototype()     ||
        protoObj == exec->lexicalInterpreter()->builtinTypeErrorPrototype()      ||
        protoObj == exec->lexicalInterpreter()->builtinURIErrorPrototype())
    {
        exception = true;
    }

    // Extract messages for exceptions --- add syntax error properly
    if (exception)
    {
        // ### it's still not 100% safe to call toString here, 
        // since someone might have changed the toString property of the 
        // exception prototype, but I'll punt on this case for now.
        // We also need to clear exception temporarily so that JSObject::toString
        // does not do a "oy, and exception" routine for us
        exec->clearException();
        exceptionMsg = exceptionObj->toString(exec).qstring();
        exec->setException(exceptionObj);
    }

    // Look up fragment info from sourceId
    DebugDocument* doc = m_sourceIdLookup[sourceId];

    // Figure out filename.
    QString url = "????";
    if (exec->context()->codeType() == EvalCode)
        url = "eval";
    if (!doc->url().isEmpty())
        url = doc->url();

    QString msg = i18n("An error occurred while attempting to run a script on this page.\n\n%1 line %2:\n%3",
                KStringHandler::rsqueeze(url, 80), lineNo, exceptionMsg);

    KJSErrorDialog dlg(this /*dlgParent*/, msg, true);
    dlg.exec();
    if (dlg.debugSelected())
    {
        // We want to stop at the current line, to see what's going on.
        enterDebugSession(exec, doc, lineNo);
    }

    return shouldContinue(m_contexts[exec->dynamicInterpreter()]);
}


bool DebugWindow::atStatement(ExecState *exec, int sourceId, int firstLine, int lastLine)
{
    // ### line number seems to be off-by-one here
    return checkSourceLocation(exec, sourceId, firstLine - 1, lastLine);
}

bool DebugWindow::checkSourceLocation(KJS::ExecState *exec, int sourceId, int firstLine, int lastLine)
{
    kDebug() << firstLine << m_breakAtNext;

    InterpreterContext* candidateCtx = m_contexts[exec->dynamicInterpreter()];

    if (!shouldContinue(candidateCtx))
        return false;

    bool enterDebugMode = false;

    // We stop when breakAtNext is set regardless of the context.
    if (m_breakAtNext)
    {
        enterDebugMode = true;
        m_breakAtNext  = false;
    }

    if (candidateCtx->mode == Step)
        enterDebugMode = true;

    //### test StepOver on top level, etc.

    DebugDocument *document = m_sourceIdLookup[sourceId];
    assert(document);

    // Now check for breakpoints if needed
    if (document->hasBreakpoint(firstLine))
        enterDebugMode = true;

    // Block the UI, and enable all the debugging buttons, etc.
    if (enterDebugMode)
        enterDebugSession(exec, document, firstLine);

    // re-checking the abort mode here, in case it got change when recursing
    return shouldContinue(candidateCtx);
}

bool DebugWindow::callEvent(ExecState *exec, int sourceId, int lineno, JSObject *function, const List &args)
{
    InterpreterContext* ctx = m_contexts[exec->dynamicInterpreter()];

    // First update call stack.
    DebugDocument *document = m_sourceIdLookup[sourceId];
    QString functionName = "?????";
    if (function->inherits(&InternalFunctionImp::info))
    {
        KJS::InternalFunctionImp *func = static_cast<InternalFunctionImp*>(function);
        functionName = func->functionName().qstring();
    }

    ctx->addCall(functionName.isEmpty() ? document->name() : functionName, lineno);
    ctx->execContexts.push(exec);

    return shouldContinue(ctx);
}

bool DebugWindow::returnEvent(ExecState *exec, int sourceId, int lineno, JSObject *function)
{
    InterpreterContext* ic  = m_contexts[exec->dynamicInterpreter()];
    DebugDocument *document = m_sourceIdLookup[sourceId];
    ic->removeCall();

    assert(ic->execContexts.top() == exec);
    ic->execContexts.pop();

    // See if we should stop on the next instruction
    if (ic->mode == StepOut)
    {
        if (ic->execContexts.size() < ic->depthAtSkip)
            ic->mode = Step;
    }
    else if (ic->mode == StepOver)
    {
        if (ic->execContexts.size() <= ic->depthAtSkip)
            ic->mode = Step;
    }

    // There is a special case here: we may have clicked step, and
    // ran out of code, and now UI is in stopped mode (since we avoid
    // switching Stopped->Running->Stopped on plain single-step)
    // This happens when:
    // 1) No session is active
    // 2) The context steck for this session is empty
    // 3) This session is thought to be waiting for a step.
    if (m_activeSessionCtxs.isEmpty() &&
        ic->execContexts.isEmpty() && ic->mode == Step)
    {
        setUIMode(Running);
        ic->mode = Normal;
    }

    return shouldContinue(ic);
}

// End KJS::Debugger overloads


void DebugWindow::enableKateHighlighting(KTextEditor::Document *document)
{
    if (!document)
        return;

    document->setMode("JavaScript");
}


void DebugWindow::displayScript(DebugDocument *document)
{
    if (m_tabWidget->isHidden())
        m_tabWidget->show();

    if (m_openDocuments.contains(document))
    {
        int idx = m_openDocuments.indexOf(document);
        m_tabWidget->setCurrentIndex(idx);
        return;
    }

    KTextEditor::Document *doc = m_debugLut[document];

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
        // ### KDE4.1: fix this hack used to avoid new string
        markInterface->setMarkDescription(KTextEditor::MarkInterface::BreakpointActive, 
                                          i18n("Breakpoints"));
        markInterface->setMarkPixmap(KTextEditor::MarkInterface::BreakpointActive, 
                                     SmallIcon("flag-red"));
        markInterface->setMarkPixmap(KTextEditor::MarkInterface::Execution, 
                                     SmallIcon("arrow-right"));
    }

    doc->setReadWrite(false);
    m_openDocuments.append(document);
    int idx = m_tabWidget->addTab(view, document->name());
    m_tabWidget->setCurrentIndex(idx);
}

KTextEditor::View* DebugWindow::viewForDocument(DebugDocument* document)
{
    assert (m_openDocuments.contains(document));

    int idx = m_openDocuments.indexOf(document);
    return qobject_cast<KTextEditor::View*>(m_tabWidget->widget(idx));
}

KTextEditor::Document* DebugWindow::buildViewerDocument(DebugDocument *document)
{
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

    doc->setReadWrite(true);
    doc->clear();
    
    // Note: in case there are fragments on the same line, and some 
    // are inline code, the order will not match the document.
    // This needs column information to work right; with corresponding 
    // adjustments for breakpoints
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
        QString source = fragment.source;

        // ### can we guarantee this as a separator? probably not
        QStringList sourceLines = source.split("\n");

        if (line <= doc->lines() - 1) {
            // There is actually something there, so we want to combine,
            // taking care that there may be fragments already there are beginning and end.
            sourceLines[0] = doc->line(line) + "  " + sourceLines[0];

            int lastLine = line + sourceLines.size() - 1;
            if (lastLine < doc->lines())
                sourceLines[sourceLines.size() - 1] += "  " + doc->line(lastLine);

            // Now get rid of all the lines in this range..
            int linesToRemove = sourceLines.size();
            if (lastLine >= doc->lines())
                linesToRemove = doc->lines() - line;

            while (linesToRemove > 0) {
                doc->removeLine(line);
                --linesToRemove;
            }
        }
        
        // Insert enough blank lines to get us to the end. Kind of sucks.
        while (doc->lines() - 1 < line) {
            doc->insertLine(doc->lines(), "");
        }
        

        // Now put in our code
        doc->insertLines(line, sourceLines);
    }
    
    return doc;
}

void DebugWindow::markSet(KTextEditor::Document *document, KTextEditor::Mark mark,
                          KTextEditor::MarkInterface::MarkChangeAction action)
{
    if (mark.type != KTextEditor::MarkInterface::BreakpointActive)
        return;

    DebugDocument *debugDocument = m_documentLut[document];
    if (!debugDocument)
        return;

    int lineNumber = mark.line + 1;         // we do this because bookmarks are technically set
                                            // the line before what looks like the line you chose..

    switch(action)
    {
        case KTextEditor::MarkInterface::MarkAdded:
            kDebug() << lineNumber;
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


void DebugWindow::enterDebugSession(KJS::ExecState *exec, DebugDocument *document, int line)
{
    // This "enters" a new debugging session, i.e. enables usage of the debugging window
    // It re-enters the qt event loop here, allowing execution of other parts of the
    // program to continue while the script is stopped. We have to be a bit careful here,
    // i.e. make sure the user can't quit the app, and disable other event handlers which
    // could interfere with the debugging session.

    if (!isVisible())
        show();

    pauseTimeoutChecks();
    m_activeSessionCtxs.push(m_contexts[exec->dynamicInterpreter()]);

    // In global code, we may have to swizzle the lowest
    // frame as appropriate; also will have to fix up
    // the context stack. ### fix up KJS::Debugger instead
    if (exec->context()->codeType() == GlobalCode) 
    {
        ctx()->setGlobalFrame(document->url());
        ctx()->execContexts.push(exec);
    }
    ctx()->updateCall(line);
    ctx()->activeDocument = document;

    setUIMode(Stopped);
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
        case QEvent::HideToParent:
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
