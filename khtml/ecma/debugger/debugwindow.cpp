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
#include <ktoggleaction.h>
#include <kglobalsettings.h>
#include <kshortcut.h>
#include <kconfig.h>
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
#include <ecma/kjs_window.h>

#include <QVBoxLayout>
#include <QSplitter>
#include <QTabWidget>
#include <QToolButton>
#include <QTextDocument>

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

DebugWindow * DebugWindow::window()
{
    if (!s_debugger)
        s_debugger = new DebugWindow();

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
    connect(m_callStack, SIGNAL(displayScript(KJSDebugger::DebugDocument*, int)),
            this, SLOT(updateVarView()));

    m_breakAtNext = false;
    m_modalLevel  = 0;
    m_runningSessionCtx = 0;
}

void DebugWindow::createActions()
{
    // Flow control actions
    m_stopAct = new KToggleAction(KIcon(":/images/stop.png"), i18n("&Break at Next Statement"), this );
    m_stopAct->setIconText(i18n("Break at Next"));
    actionCollection()->addAction( "stop", m_stopAct );
    m_stopAct->setEnabled(true);
    connect(m_stopAct, SIGNAL(triggered(bool)), this, SLOT(stopAtNext()));

    m_continueAct = new KAction(KIcon(":/images/continue.png"), i18n("Continue"), this );
    actionCollection()->addAction( "continue", m_continueAct );
    m_continueAct->setShortcut(Qt::Key_F9);
    m_continueAct->setEnabled(false);
    connect(m_continueAct, SIGNAL(triggered(bool)), this, SLOT(continueExecution()));    


    m_stepOverAct = new KAction(KIcon(":/images/step-over.png"), i18n("Step Over"), this );
    actionCollection()->addAction( "stepOver", m_stepOverAct );
    m_stepOverAct->setShortcut(Qt::Key_F10);
    m_stepOverAct->setEnabled(false);
    connect(m_stepOverAct, SIGNAL(triggered(bool)), this, SLOT(stepOver()) );
    

    m_stepIntoAct = new KAction(KIcon(":/images/step-into.png"), i18n("Step Into"), this );
    actionCollection()->addAction( "stepInto", m_stepIntoAct );
    m_stepIntoAct->setShortcut(Qt::Key_F11);
    m_stepIntoAct->setEnabled(false);

    connect(m_stepIntoAct, SIGNAL(triggered(bool)), this, SLOT(stepInto()));

    m_stepOutAct = new KAction(KIcon(":/images/step-out.png"), i18n("Step Out"), this );
    actionCollection()->addAction( "stepOut", m_stepOutAct );
    m_stepOutAct->setShortcut(Qt::Key_F12);
    m_stepOutAct->setEnabled(false);
    connect(m_stepOutAct, SIGNAL(triggered(bool)), this, SLOT(stepOut()) );
}

void DebugWindow::createMenus()
{
    KMenu *debugMenu = new KMenu(i18n("&Debug"), menuBar());
    debugMenu->addAction(m_stopAct);
    debugMenu->addAction(m_continueAct);
    debugMenu->addAction(m_stepOverAct);
    debugMenu->addAction(m_stepIntoAct);
    debugMenu->addAction(m_stepOutAct);
    menuBar()->addMenu(debugMenu);
}

void DebugWindow::createToolBars()
{
    toolBar()->addAction(m_stopAct);
    toolBar()->addSeparator();
    toolBar()->addAction(m_continueAct);
    toolBar()->addAction(m_stepOverAct);    
    toolBar()->addAction(m_stepIntoAct);
    toolBar()->addAction(m_stepOutAct);
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
    if (!ctx())
        return;

    DebugDocument::Ptr doc = ctx()->activeDocument();
    assert(!doc.isNull());
    KTextEditor::MarkInterface* imark = qobject_cast<KTextEditor::MarkInterface*>(doc->viewerDocument());

    if (mode == Running)
    {
        // No longer stopped there.
        if (imark)
            imark->removeMark(ctx()->activeLine() - 1, KTextEditor::MarkInterface::Execution);
    }
    else
    {
        displayScript(doc.get(), ctx()->activeLine());
        if (imark)
            imark->addMark(ctx()->activeLine() - 1, KTextEditor::MarkInterface::Execution);
    }
}

void DebugWindow::setUIMode(RunMode mode)
{
    if (mode == Running)
    {
        // Toggle buttons..
        m_continueAct->setEnabled(false);
        m_stepIntoAct->setEnabled(false);
        m_stepOutAct->setEnabled(false);
        m_stepOverAct->setEnabled(false);
        m_runningSessionCtx = ctx();
    }
    else
    {
        // Show local variables and the bt
        m_localVariables->updateDisplay(ctx()->execContexts.top());
        m_callStack->displayStack(ctx());

        // Toggle buttons..
        m_continueAct->setEnabled(true);
        m_stepIntoAct->setEnabled(true);
        m_stepOutAct->setEnabled(true);
        m_stepOverAct->setEnabled(true);
        m_runningSessionCtx = 0;
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
    m_breakAtNext = m_stopAct->isChecked();
}

bool DebugWindow::shouldContinue(InterpreterContext* ic)
{
    return !ic || ic->mode != Abort;
}

void DebugWindow::leaveDebugSession()
{
    // Update UI for running mode, unless we expect things to be quick;
    // in which case we'll only update if we have to, when running stops
    if (ctx()->mode != Step)
        setUIMode(Running);
    else  // In the other case, we still want to remove the old running marker, however
        updateStoppedMark(Running);

    m_activeSessionCtxs.pop();
    resetTimeoutsIfNeeded();
    exitLoop();
}

void DebugWindow::continueExecution()
{
    if (!ctx()) return; //In case we're in the middle of a step.. Hardly ideal, but..
    leaveDebugSession();
}

void DebugWindow::stepInto()
{
    if (!ctx()) return; //In case we're in the middle of a step.. Hardly ideal, but..
    ctx()->mode = Step;
    leaveDebugSession();
}

void DebugWindow::stepOut()
{
    if (!ctx()) return; //In case we're in the middle of a step.. Hardly ideal, but..
    ctx()->mode        = StepOut;
    ctx()->depthAtSkip = ctx()->execContexts.size();
    leaveDebugSession();
}

void DebugWindow::stepOver()
{
    if (!ctx()) return; //In case we're in the middle of a step.. Hardly ideal, but..
    ctx()->mode        = StepOver;
    ctx()->depthAtSkip = ctx()->execContexts.size();
    leaveDebugSession();
}

DebugWindow::~DebugWindow()
{
    assert(m_docsForIntrp.isEmpty());
    assert(m_docForSid.isEmpty());
    assert(m_docForIUKey.isEmpty());
    assert(m_activeSessionCtxs.isEmpty());
    s_debugger = 0;
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

void DebugWindow::cleanupDocument(DebugDocument::Ptr doc)
{
    // Remove all fragments from the sid map
    foreach (int sid, doc->fragments())
    {
        m_docForSid.remove(sid);
        // Remove from the IU-key map if needed.
        if (!doc->url().isEmpty())
            m_docForIUKey.remove(doc->iuKey());
    }
    m_scripts->documentDestroyed(doc.get());
}

static void fatalAssert(bool shouldBeTrue, const char* error)
{
    if (!shouldBeTrue)
        kFatal() << error;
}

void DebugWindow::detach(KJS::Interpreter* interp)
{
    assert(interp); //detach(0) should never get here, since only ~Debugger calls it

    // Make sure no weird recursions can still happen!
    InterpreterContext* ctx = m_contexts[interp];
    assert (!m_activeSessionCtxs.contains(ctx));

    // Go through, and kill all the fragments from here.
    QList<DebugDocument::Ptr> docs = m_docsForIntrp[interp];

    foreach (DebugDocument::Ptr doc, docs)
        cleanupDocument(doc);

    m_docsForIntrp.remove(interp);

    delete m_contexts.take(interp);
    resetTimeoutsIfNeeded();

    KJS::Debugger::detach(interp);
}

void DebugWindow::clearInterpreter(KJS::Interpreter* interp)
{
    // We may get a clear when we weren't even attached, if the
    // interpreter gets created but nothing gets run in it.
    // Be careful not to insert a bogus null into contexts map then
    InterpreterContext* ctx = m_contexts.value(interp);
    if (!ctx)
        return;

    fatalAssert(!m_activeSessionCtxs.contains(ctx), "Interpreter clear on active session");

    // Cleanup any docs that are not open, and mark others
    // as having finished their current load incarnation
    QMutableListIterator<DebugDocument::Ptr> i(m_docsForIntrp[interp]);
    while (i.hasNext())
    {
        DebugDocument::Ptr doc = i.next();
        if (m_openDocuments.contains(doc.get()))
        {
            doc->requestDeferredClear(); // Any new fragments will be from a new load..
        }
        else
        {
            cleanupDocument(doc);
            i.remove();
        }
    }
}

bool DebugWindow::sourceParsed(ExecState *exec, int sourceId, const UString& jsSourceURL,
                               const UString &source, int startingLineNumber, int errorLine, const UString &/* errorMsg */)
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
        // If there is no URL, try to figure one out from the caller ---
        // useful for function constructor and eval.
        QString uiURL = sourceURL;

        if (uiURL.isEmpty())
        {
            // Scan through all contexts, and see which one matches
            foreach (InterpreterContext* ic, m_contexts)
            {
                if (!ic->execContexts.isEmpty() && ic->execContexts.top() == exec)
                {
                    uiURL = ic->callStack.top().doc->url();
                    break;
                }
            }

        }

        document = new DebugDocument(exec->dynamicInterpreter(), uiURL, key);
        connect(document.get(), SIGNAL(documentDestroyed(KJSDebugger::DebugDocument*)),
                this, SLOT(documentDestroyed(KJSDebugger::DebugDocument*)));
        m_docsForIntrp[exec->dynamicInterpreter()].append(document);

        // Show it in the script list view
        m_scripts->addDocument(document.get());
    }

    // Memorize the document..
    m_docForSid[sourceId] = document;
    if (!sourceURL.isEmpty())
        m_docForIUKey[key] = document;

    // Tell it about this portion..
    QString qsource =  source.qstring();
    if (qsource.contains("function")) // Approximate knowledge of whether code has functions. Ewwww...
        document->setHasFunctions();
    document->addCodeFragment(sourceId, startingLineNumber, qsource);

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

    if (!exception)
        return exceptionMsg;

    // Clear exceptions temporarily so we can get/call a few things.
    // We memorize the old exception first, of course. Note that
    // This is not always the same as exceptionObj since we may be
    //  asked to translate a non-active exception
    JSValue* oldExceptionObj = exec->exception();
    exec->clearException();

    // We want to serialize the syntax errors ourselves, to provide the line number.
    // The URL is in "sourceURL" and the line is in "line"
    // ### TODO: Perhaps we want to use 'sourceId' in case of eval contexts.
    if (syntaxError)
    {
        JSValue* lineValue = valueObj->get(exec, "line");
        JSValue* urlValue  = valueObj->get(exec, "sourceURL");

        int      line = lineValue->toNumber(exec);
        QString  url  = urlValue->toString(exec).qstring();
        exceptionMsg = i18n("Parse error at %1 line %2",
                            Qt::escape(url), line - 1);
    }
    else
    {
        // ### it's still not 100% safe to call toString here, even on
        // native exception objects, since someone might have changed the toString property
        // of the exception prototype, but I'll punt on this case for now.
        exceptionMsg = exceptionObj->toString(exec).qstring();
    }
    exec->setException(oldExceptionObj);
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
    TimerPauser pause(exec); // don't let any timers fire while we're doing this!
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
        enterDebugMode = true;

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
    QString stackEntry = document->name();
    if (function && function->inherits(&InternalFunctionImp::info))
    {
        KJS::InternalFunctionImp *func = static_cast<InternalFunctionImp*>(function);
        QString functionName = func->functionName().qstring();
        if (!functionName.isEmpty())
            stackEntry = functionName;
    }

    if (exec->context()->codeType() == EvalCode)
        stackEntry = "eval";

    ctx->addCall(document, stackEntry, lineno);
    ctx->execContexts.push(exec);

    return shouldContinue(ctx);
}

bool DebugWindow::exitContext(ExecState *exec, int sourceId, int lineno, JSObject *function)
{
    InterpreterContext* ic  = m_contexts[exec->dynamicInterpreter()];

    if (m_localVariables->currentlyDisplaying() == exec)
    {
        // Clear local variable and stack display when exiting a function
        // it corresponds to
        m_localVariables->updateDisplay(0);
        m_callStack->clearDisplay();
    }

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

    // On the other hand, UI may be in running mode, but we've just
    // ran out of all the code for this interpreter. In this case,
    // reactive the previous session in stopped mode.
    if (!m_activeSessionCtxs.isEmpty() && m_runningSessionCtx == ic)
    {
        if (ic->execContexts.isEmpty())
            setUIMode(Stopped);
        else
            fatalAssert(exec->context()->callingContext(), "Apparent event re-entry");
            // Sanity check: the modality protection should disallow us to exit
            // from a context called by KHTML unless it's at very top level
            // (e.g. no other execs on top)
    }

    // Also, if we exit from an eval context, we probably want to
    // clear the corresponding document, unless it's open.
    // We can not do it safely if there are any functions declared,
    // however, since they can escape.
    if (exec->context()->codeType() == EvalCode)
    {
        DebugDocument::Ptr doc = m_docForSid[sourceId];
        if (!m_openDocuments.contains(doc.get()) && !doc->hasFunctions())
        {
            cleanupDocument(doc);
            m_docsForIntrp[exec->dynamicInterpreter()].removeAll(doc);
        }
    }

    return shouldContinue(ic);
}

// End KJS::Debugger overloads

void DebugWindow::doEval(const QString& qcode)
{
    // Work out which execution state to use. If we're currently in a debugging session,
    // use the context of the presently selected frame, if any --- otherwise, use the global execution
    // state from the interpreter corresponding to the currently displayed source file.
    ExecState* exec;
    JSObject*  thisObj;

    if (inSession())
    {
        exec = m_callStack->selectedFrameContext();
        if (!exec)
            exec = m_activeSessionCtxs.top()->execContexts.top();
        thisObj = exec->context()->thisValue();
    }
    else
    {
        int idx = m_tabWidget->currentIndex();
        if (idx < 0)
        {
            m_console->reportResult(qcode,
                i18n("Don't know where to evaluate the expression. Please pause a script or open a source file."));
            return;
        }
        DebugDocument* document = m_openDocuments[idx];
        exec    = document->interpreter()->globalExec();
        thisObj = document->interpreter()->globalObject();
    }

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
        msg = i18n("Evaluation threw an exception %1", exceptionToString(exec, exc));
    }
    else
    {
        msg = valueToString(retVal);
    }

    // Restore old exception if need be, and always clear ours
    exec->clearException();
    if (oldException)
        exec->setException(oldException);

    m_console->reportResult(qcode, msg);

    // Make sure to re-activate the line we were stopped in,
    // in case a nested session was active
    if (inSession())
        setUIMode(Stopped);
}

void DebugWindow::updateVarView()
{
    m_localVariables->updateDisplay(m_callStack->selectedFrameContext());
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
    ctx()->mode   = Normal;
    m_breakAtNext = false;
    m_stopAct->setChecked(false);

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
    m_activeEventLoops.push(&eventLoop);

    if (m_activeSessionCtxs.size() == 1)
        enterModality();

//    eventLoop.exec(QEventLoop::X11ExcludeTimers | QEventLoop::ExcludeSocketNotifiers);
    eventLoop.exec();
    m_activeEventLoops.pop();
}

void DebugWindow::exitLoop()
{
    if (m_activeSessionCtxs.isEmpty())
        leaveModality();
    m_activeEventLoops.top()->quit();
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
