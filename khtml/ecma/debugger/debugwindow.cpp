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
#include <QtAlgorithms>

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

DebugWindow* DebugWindow::s_debugger = 0;
DebugWindow *DebugWindow::createInstance()
{
    Q_ASSERT(!s_debugger);
    s_debugger = new DebugWindow();
    return s_debugger;
}

void DebugWindow::destroyInstance()
{
    Q_ASSERT(s_debugger);
    Q_ASSERT(s_debugger->m_activeSessionCtxs.isEmpty());
    s_debugger->hide();
    delete s_debugger;
}

DebugWindow * DebugWindow::window()
{
    return s_debugger;
}

// ----------------------------------------------

DebugWindow::DebugWindow(QWidget *parent)
  : KXmlGuiWindow(parent, Qt::Window),
    KComponentData("kjs_debugger")
{
    setAttribute(Qt::WA_DeleteOnClose, false);
    setObjectName(QLatin1String("DebugWindow"));
    setCaption(i18n("JavaScript Debugger"));

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
    connect(m_console, SIGNAL(requestEval(const QString&)),
            this,      SLOT  (doEval(const QString&)));


    addDockWidget(Qt::LeftDockWidgetArea, m_scripts);
    addDockWidget(Qt::LeftDockWidgetArea, m_localVariables);
    addDockWidget(Qt::LeftDockWidgetArea, m_callStack);
    //addDockWidget(Qt::LeftDockWidgetArea, m_breakpoints);
//     addDockWidget(Qt::LeftDockWidgetArea, m_watches);

    QSplitter *splitter = new QSplitter(Qt::Vertical);
    createTabWidget();
    splitter->addWidget(m_tabWidget);
    splitter->addWidget(m_console);
    splitter->setStretchFactor(0, 10);
    splitter->setStretchFactor(1, 1);

    setCentralWidget(splitter);
    resize(800, 500);

    createActions();
    createMenus();
    createToolBars();
    createStatusBar();
    m_tabWidget->hide();

    connect(m_scripts, SIGNAL(displayScript(KJSDebugger::DebugDocument*)),
            this, SLOT(displayScript(KJSDebugger::DebugDocument*)));
    connect(m_callStack, SIGNAL(displayScript(KJSDebugger::DebugDocument*, int)),
            this, SLOT(displayScript(KJSDebugger::DebugDocument*, int)));

    m_breakAtNext = false;
    m_modalLevel  = 0;
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
    m_tabWidget = new QTabWidget;

    QToolButton *closeTabButton = new QToolButton(m_tabWidget);
    m_tabWidget->setCornerWidget(closeTabButton, Qt::TopRightCorner);
    closeTabButton->setCursor(Qt::ArrowCursor);
    closeTabButton->setAutoRaise(true);
    closeTabButton->setIcon(KIcon("tab-close"));
    connect(closeTabButton, SIGNAL(clicked()), this, SLOT(closeTab()));
    closeTabButton->setToolTip(i18n("Close source"));
    closeTabButton->setEnabled(true);
}

void DebugWindow::createStatusBar()
{
    statusBar()->showMessage(i18n("Ready"));
}

void DebugWindow::updateStoppedMark(RunMode mode)
{
    if (!ctx() || !ctx()->activeDocument)
        return;

    DebugDocument* doc = ctx()->activeDocument;
    KTextEditor::MarkInterface* imark = qobject_cast<KTextEditor::MarkInterface*>(doc->viewerDocument());

    if (mode == Running)
    {
        // No longer stopped there.
        if (imark)
            imark->removeMark(ctx()->activeLine() - 1, KTextEditor::MarkInterface::Execution);
    }
    else
    {
        displayScript(doc, ctx()->activeLine());
        if (imark)
            imark->addMark(ctx()->activeLine() - 1, KTextEditor::MarkInterface::Execution);
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

bool DebugWindow::isBlocked()
{
    DebugWindow* self = window();
    if (!self)
        return false;
    return self->inSession() || self->m_modalLevel;
}

void DebugWindow::resetTimeoutsIfNeeded()
{
    if (!isBlocked())
    {
        KJS::Interpreter* intp = KJS::Interpreter::firstInterpreter();
        do
        {
            intp->restartTimeoutCheck();
            intp = intp->nextInterpreter();
        }
        while (intp != KJS::Interpreter::firstInterpreter());
    }
}

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
    resetTimeoutsIfNeeded();

    // There may be a previous session in progress --- in
    // that case we need to update the UI to reflect that.
    if (!m_activeSessionCtxs.isEmpty())
        setUIMode(Running);

    exitLoop();
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
    assert(m_sidsForIntrp.isEmpty());
    assert(m_docForSid.isEmpty());
    assert(m_docForIUKey.isEmpty());
}

void DebugWindow::closeEvent(QCloseEvent* event)
{
    if (inSession())
        event->setAccepted(false);
    else
        KXmlGuiWindow::closeEvent(event);
}

// -------------------------------------------------------------

void DebugWindow::attach(Interpreter *interp)
{
    // ::attach can be called many times, so handle that
    if (!m_contexts[interp])
        m_contexts[interp] = new InterpreterContext;
    KJS::Debugger::attach(interp);
}

void DebugWindow::detach(KJS::Interpreter* interp)
{
    assert(interp); //detach(0) should never get here, since only ~Debugger calls it

    // Go through, and kill all the fragments from here.
    QList<int>& outSids = m_sidsForIntrp[interp];

    foreach (int sid, outSids)
    {
        // Cleanup all the fragments we have loaded.
        DebugDocument::Ptr doc = m_docForSid[sid];

        if (doc->deleteFragment(sid) == DebugDocument::LastFragment)
        {
            // Remove from the IU-key map if needed.
            if (!doc->url().isEmpty())
                m_docForIUKey.remove(doc->iuKey());
            m_scripts->documentDestroyed(doc.get());
        }

        m_docForSid.remove(sid);
    }

    m_sidsForIntrp.remove(interp);

    // See if we're the active session...
    InterpreterContext* ctx = m_contexts[interp];
    if (m_activeSessionCtxs.contains(ctx))
    {
        kDebug() << "### detach while session active!";
        // Hopefully, we're the last one. In this case,
        // we merely behave as if someone clicked continue,
        // and leave the session.
        if (m_activeSessionCtxs.top() == ctx)
        {
            leaveDebugSession();
        }
        else
        {
            // Here, we want to remove the old context, drop out
            // one event loop level, but not actually
            // leave the session, since it's unrelated.
            // ### This is all wrong!
            m_activeSessionCtxs.remove(m_activeSessionCtxs.indexOf(ctx));
            exitLoop();
        }
    }

    delete m_contexts.take(interp);
    resetTimeoutsIfNeeded();

    KJS::Debugger::detach(interp);
}

void DebugWindow::clearInterpreter(KJS::Interpreter* interp)
{
    InterpreterContext* ctx = m_contexts[interp];
    assert (!m_activeSessionCtxs.contains(ctx));
}

bool DebugWindow::sourceParsed(ExecState *exec, int sourceId, const UString& jsSourceURL,
                               const UString &source, int startingLineNumber , int errorLine, const UString &/* errorMsg */)
{
    Q_UNUSED(exec);

    kDebug() << "***************************** sourceParsed **************************************************" << endl
             << "      sourceId: " << sourceId << endl
             << "     sourceURL: " << jsSourceURL.qstring() << endl
             << "startingLineNumber: " << startingLineNumber << endl
             << "     errorLine: " << errorLine << endl
             << "*********************************************************************************************" << endl;

    QString sourceURL = jsSourceURL.qstring();
    QString key       = QString("%1|%2").arg((long)exec->dynamicInterpreter()).arg(sourceURL);

    DebugDocument::Ptr document;
    if (!sourceURL.isEmpty()) // Perhaps we already have a document for this url...
        document = m_docForIUKey[key];

    // If we don't have a document, make a new one.
    if (!document)
    {
        document = new DebugDocument(sourceURL, key);
        connect(document.get(), SIGNAL(documentDestroyed(KJSDebugger::DebugDocument*)),
                this, SLOT(documentDestroyed(KJSDebugger::DebugDocument*)));
    }

    // Memorize the document..
    m_docForSid[sourceId] = document;
    if (!sourceURL.isEmpty())
        m_docForIUKey[key] = document;
    m_sidsForIntrp[exec->dynamicInterpreter()].append(sourceId);

    // Tell it about this portion..
    document->addCodeFragment(sourceId, startingLineNumber, source.qstring());

    // Show it in the script list view
    m_scripts->addDocument(document.get());

    return shouldContinue(m_contexts[exec->dynamicInterpreter()]);
}

QString DebugWindow::exceptionToString(ExecState* exec, JSValue* exceptionObj)
{
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

    return exceptionMsg;
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

    QString exceptionMsg = exceptionToString(exec, exceptionObj);

    // Look up fragment info from sourceId
    DebugDocument::Ptr doc = m_docForSid[sourceId];

    // Figure out filename.
    QString url = "????";
    if (exec->context()->codeType() == EvalCode)
        url = "eval";
    if (!doc->url().isEmpty())
        url = doc->url();

    QString msg = i18n("An error occurred while attempting to run a script on this page.\n\n%1 line %2:\n%3",
                KStringHandler::rsqueeze(url, 80), lineNo, exceptionMsg);

    KJSErrorDialog dlg(this /*dlgParent*/, msg, true);
    ++m_modalLevel;
    dlg.exec();
    --m_modalLevel;
    resetTimeoutsIfNeeded();
    if (dlg.debugSelected())
        // We want to stop at the current line, to see what's going on.
        enterDebugSession(exec, doc.get(), lineNo);

    return shouldContinue(m_contexts[exec->dynamicInterpreter()]);
}


bool DebugWindow::atStatement(ExecState *exec, int sourceId, int firstLine, int lastLine)
{
    // ### line number seems to be off-by-one here
    InterpreterContext* ctx = m_contexts[exec->dynamicInterpreter()];
    ctx->updateCall(firstLine - 1);
    return checkSourceLocation(exec, sourceId, firstLine - 1, lastLine);
}

bool DebugWindow::checkSourceLocation(KJS::ExecState *exec, int sourceId, int firstLine, int lastLine)
{
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
    else if (candidateCtx->mode == StepOver)
    {
        if (candidateCtx->execContexts.size() <= candidateCtx->depthAtSkip)
            enterDebugMode = true;
    }

    DebugDocument::Ptr document = m_docForSid[sourceId];
    assert(!document.isNull());

    // Now check for breakpoints if needed
    if (document->hasBreakpoint(firstLine))
        enterDebugMode = true;

    // Block the UI, and enable all the debugging buttons, etc.
    if (enterDebugMode)
        enterDebugSession(exec, document.get(), firstLine);

    // re-checking the abort mode here, in case it got change when recursing
    return shouldContinue(candidateCtx);
}

bool DebugWindow::enterContext(ExecState *exec, int sourceId, int lineno, JSObject *function, const List &args)
{
    InterpreterContext* ctx = m_contexts[exec->dynamicInterpreter()];

    // First update call stack.
    DebugDocument::Ptr document = m_docForSid[sourceId];
    QString functionName;
    if (function && function->inherits(&InternalFunctionImp::info))
    {
        KJS::InternalFunctionImp *func = static_cast<InternalFunctionImp*>(function);
        functionName = func->functionName().qstring();
    }

    ctx->addCall(document, functionName.isEmpty() ? document->name() : functionName, lineno);
    ctx->execContexts.push(exec);

    return shouldContinue(ctx);
}

bool DebugWindow::exitContext(ExecState *exec, int sourceId, int lineno, JSObject *function)
{
    InterpreterContext* ic  = m_contexts[exec->dynamicInterpreter()];

    ic->removeCall();
    assert(ic->execContexts.top() == exec);
    ic->execContexts.pop();

    // See if we should stop on the next instruction.
    // Note that we should not test StepOver here, as
    // we may have a return event at same level
    // in case of a f(g(), h()) type setup
    // Note that in the above case a StepOut from
    // g() would step into h() from below, which is reasonable
    if (ic->mode == StepOut)
    {
        if (ic->execContexts.size() < ic->depthAtSkip)
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
    }

    return shouldContinue(ic);
}

// End KJS::Debugger overloads

void DebugWindow::doEval(const QString& qcode)
{
    // Work out which execution state to use. Currently, limit ourselves to
    // working inside an active session. It's not clear what behavior
    // would be appropriate otherwise.
    if (!inSession())
        m_console->reportResult(qcode, "????"); //### KDE4.1: proper error message

    InterpreterContext* ctx = m_activeSessionCtxs.top();
    ExecState* exec    = ctx->execContexts.top();
    JSObject*  thisObj = exec->context()->thisValue();
    JSValue*   oldException = 0;

    UString code(qcode);

    Interpreter *interp = exec->dynamicInterpreter();

    // If there was a previous exception, clear it for now and save it.
    if (exec->hadException())
    {
        oldException = exec->exception();
        exec->clearException();
    }

    JSObject *obj = interp->globalObject()->get(exec, "eval")->getObject();
    List args;
    args.append(jsString(code));

    // ### we want the CPU guard here.. But only for this stuff,
    // not others things. oooh boy. punt for now

    JSValue *retVal = obj->call(exec, thisObj, args);

    // Print the return value or exception message to the console
    QString msg;
    if (exec->hadException())
    {
        JSValue *exc = exec->exception();
        exec->clearException();
        msg = "Exception: " + exceptionToString(exec, exc);
    }
    else
    {
        msg = valueToString(retVal);
    }

    // Restore old exception if need be
    if (oldException)
        exec->setException(oldException);

    m_console->reportResult(qcode, msg);
}

void DebugWindow::displayScript(DebugDocument* document)
{
    displayScript(document, -1);
}

void DebugWindow::displayScript(DebugDocument* document, int line)
{
    if (m_tabWidget->isHidden())
        m_tabWidget->show();

    KTextEditor::View* view = document->viewerView();

    if (!m_openDocuments.contains(document))
    {
        m_openDocuments.append(document);
        m_tabWidget->addTab(view, document->name());
    }

    // Focus the tab
    int idx = m_openDocuments.indexOf(document);
    m_tabWidget->setCurrentIndex(idx);

    // Go to line..
    if (line != -1)
        view->setCursorPosition(KTextEditor::Cursor(line - 1, 0));
}

void DebugWindow::documentDestroyed(KJSDebugger::DebugDocument* doc)
{
    //### this is likely to be very ugly UI-wise
    // Close this document..
    int idx = m_openDocuments.indexOf(doc);
    if (idx == -1)
        return;

    m_tabWidget->removeTab(idx);
    m_openDocuments.removeAt(idx);
    if (m_openDocuments.isEmpty())
        m_tabWidget->hide();
}

void DebugWindow::closeTab()
{
    int idx = m_tabWidget->currentIndex();
    m_tabWidget->removeTab(idx);
    m_openDocuments.removeAt(idx);
    if (m_openDocuments.isEmpty())
        m_tabWidget->hide();
}

void DebugWindow::markSet(KTextEditor::Document* document, KTextEditor::Mark mark,
                          KTextEditor::MarkInterface::MarkChangeAction action)
{
    if (mark.type != KTextEditor::MarkInterface::BreakpointActive)
        return;

    // ### ugleeee -- get our docu from viewer docu's parent, to avoid book keeping
    DebugDocument* debugDocument = qobject_cast<DebugDocument*>(document->parent());
    assert(debugDocument);

    int lineNumber = mark.line + 1;
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
             << "line: " << lineNumber;
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

    m_activeSessionCtxs.push(m_contexts[exec->dynamicInterpreter()]);

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
        case QEvent::Shortcut:
        case QEvent::ShortcutOverride:
            {
                while (object->parent())
                    object = object->parent();
                if (object == this)
                    return QWidget::eventFilter(object, event);
                else
                {
                    if (event->type() == QEvent::Close)
                        event->setAccepted(false);
                    return true;
                }
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
