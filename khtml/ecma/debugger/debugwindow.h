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

#ifndef DEBUGWINDOW_H
#define DEBUGWINDOW_H

#define KJS_DEBUGGER

#include <kcomponentdata.h>
#include <kxmlguiwindow.h>

#include <kjs/debugger.h>
#include <kjs/completion.h>
#include <kjs/interpreter.h>
#include <kjs/value.h>
#include <kjs_binding.h>

#include <ktexteditor/document.h>
#include <ktexteditor/view.h>
#include <ktexteditor/editor.h>
#include <ktexteditor/markinterface.h>

#include "khtml_pagecache.h"
#include "khtml_part.h"
#include "dom/dom_misc.h"
#include "misc/shared.h"

#include <QStack>
#include <QVector>

#include "interpreter_ctx.h"
#include "debugdocument.h"

class KAction;
class QTabWidget;
class QFrame;
class QEventLoop;


namespace KJSDebugger {

class CallStackDock;
class WatchesDock;
class LocalVariablesDock;
class ScriptsDock;
class BreakpointsDock;
class ConsoleDock;


/**
* DebuggerWindow
*
* KJSDebugWin represents the debugger window that is visible to the user. It contains
* a stack frame list, a code viewer and a source fragment selector, plus buttons
* to control execution including next, step and continue.
*
* There is only one debug window per program. This can be obtained by calling #instance
*/
class DebugWindow : public KXmlGuiWindow, public KJS::Debugger, public KComponentData,
                    public khtml::Shared<DebugWindow>
{
    Q_OBJECT

public:
    DebugWindow(QWidget *parent = 0);
    virtual ~DebugWindow();

    static DebugWindow *window();

    // Returns true if the debugger is active, and has blocked the execution
    // for some reason.
    // ### seems like some of what we (mis-)use inSession() for should use this
    static bool isBlocked();

    // Returns if we blocked execution; KHTML will attempt to use it 
    // to prevent some kinds of accidental recursion. Should go 
    // if proper modal dialog manager shows up
    bool inSession() const { return !m_activeSessionCtxs.isEmpty(); }

    // Serializes an exception for human consumption.
    static QString exceptionToString(KJS::ExecState* exec, KJS::JSValue* exception);
public:

    // All of the below are overriden from KJS::Debugger
    bool sourceParsed(KJS::ExecState *exec, int sourceId, const KJS::UString &sourceURL,
                      const KJS::UString &source, int startingLineNumber, int errorLine, const KJS::UString &errorMsg);
    bool exception(KJS::ExecState *exec, int sourceId, int lineno, KJS::JSValue *exceptionObj);
    bool atStatement(KJS::ExecState *exec, int sourceId, int firstLine, int lastLine);
    bool enterContext(KJS::ExecState *exec, int sourceId, int lineno, KJS::JSObject *function, const KJS::List &args);
    bool exitContext(KJS::ExecState *exec, int sourceId, int lineno, KJS::JSObject *function);
    void attach(KJS::Interpreter *interp);
    void detach(KJS::Interpreter *interp);

    // Called by KJSProxy when we navigate away from a page
    void clearInterpreter(KJS::Interpreter* interp);

public Q_SLOTS:
    void stopAtNext();
    void continueExecution();
    void stepInto();
    void stepOut();
    void stepOver();

    void markSet(KTextEditor::Document *document, KTextEditor::Mark mark,
                 KTextEditor::MarkInterface::MarkChangeAction action);

protected:
    virtual void closeEvent(QCloseEvent* event);
    
    bool eventFilter(QObject *object, QEvent *event);
    void disableOtherWindows();
    void enableOtherWindows();

private Q_SLOTS:
    void displayScript(KJSDebugger::DebugDocument *document);
    void displayScript(KJSDebugger::DebugDocument *document, int line); // -1 denotes not focusing on the line
    void updateVarView();
    void closeTab();
    void documentDestroyed(KJSDebugger::DebugDocument* doc);

    void doEval(const QString& code);
private:
    void createActions();
    void createMenus();
    void createToolBars();
    void createStatusBar();
    void createTabWidget();

    void enterDebugSession(KJS::ExecState *exec, DebugDocument *document, int line);
    void leaveDebugSession();

    void enterModality();
    void leaveModality();

    void enterLoop();
    void exitLoop();

    enum RunMode { Running, Stopped };

    void setUIMode        (RunMode mode);
    void updateStoppedMark(RunMode mode);
private:
    void cleanupDocument(DebugDocument::Ptr document);

    // Checks to see whether we should stop at the given location, based on the current 
    // mode and breakpoints. Returns false if we should abort
    bool checkSourceLocation(KJS::ExecState* exec, int sourceId, int firstLine, int lastLine);

    // Standard actions
    KAction *m_exitAct;

    // Flow control actions
    KAction *m_continueAct;
    KAction *m_stopAct;
    KAction *m_stepIntoAct;
    KAction *m_stepOutAct;
    KAction *m_stepOverAct;

//     WatchesDock *m_watches;
    LocalVariablesDock *m_localVariables;
    ScriptsDock *m_scripts;
    CallStackDock *m_callStack;
    BreakpointsDock *m_breakpoints;
    ConsoleDock *m_console;

    QTabWidget *m_tabWidget;

    // e.g. not aborted
    bool shouldContinue(InterpreterContext* ic);

    // This keeps track of modal dialogs we've put up
    // that may disable the CPU guard.
    int m_modalLevel;

    // This is all the nested event loops that are active
    QStack<QEventLoop*> m_activeEventLoops;

    void resetTimeoutsIfNeeded();

    // The handling of debugger modes is a bit funny.
    // essentially, we want normal step/stepOver/stepOut
    // to work per (dynamic) interpreter, but "break at next" 
    // should work globally. 
    bool m_breakAtNext;

    InterpreterContext* ctx() { return m_activeSessionCtxs.isEmpty() ? 0 : m_activeSessionCtxs.top(); }

    // We look documents up 2 ways: by sourceId, and by interpreter + url key.
    // The former is used when handling the various debugger events, and is the best
    // we can do in that case. We need the later to combine the fragments into
    // one view. Persistence of breakpoints/pending breakpoints
    // are handled by DebugDocument itself.
    QHash<int,     DebugDocument::Ptr> m_docForSid;
    QHash<QString, DebugDocument::Ptr> m_docForIUKey;

    // For each interpreter, we keep track of what documents belong to it
    // so we can discard them when needed, as well as flush for reload
    QHash<KJS::Interpreter*, QList<DebugDocument::Ptr> > m_docsForIntrp;

    // Some of the state we want to keep track of while debugging, such as backtraces,
    // is per-interpreter, and this lets us look uit up.
    QHash<KJS::Interpreter*, InterpreterContext*> m_contexts;

    // This keeps track of the contexts for the various debuggers
    // we may be in session for. It's needed because the same window is
    // used for all, so we may occassionally be a few levels of recursion in,
    // so we need to know exactly how to unwind, etc.
    QStack<InterpreterContext*> m_activeSessionCtxs;

    // This denotes the session we were in once we entered the running UI
    // mode. May be null
    InterpreterContext* m_runningSessionCtx;


    // The documents that are currently open for viewing.
    // The index matches that of the tab widget;
    QList<DebugDocument*> m_openDocuments;

    static DebugWindow *s_debugger;
};




} // namespace


#endif // DEBUGWINDOW_H
