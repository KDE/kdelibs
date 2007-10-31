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

class KAction;
class WatchesDock;
class LocalVariablesDock;
class ScriptsDock;
class CallStackDock;
class BreakpointsDock;
class ConsoleDock;
class QTabWidget;
class QFrame;

namespace KJS
{

/**
* DebuggerWindow
*
* KJSDebugWin represents the debugger window that is visible to the user. It contains
* a stack frame list, a code viewer and a source fragment selector, plus buttons
* to control execution including next, step and continue.
*
* There is only one debug window per program. This can be obtained by calling #instance
*/
class DebugWindow : public KXmlGuiWindow, public KJS::Debugger, public KComponentData
{
    Q_OBJECT
public:
    enum Mode
    {
        Disabled = 0, // No break on any statements
        Next     = 1, // Will break on next statement in current context
        Step     = 2, // Will break on next statement in current or deeper context
        Continue = 3, // Will continue until next breakpoint
        Stop     = 4  // The script will stop execution completely,
                      // as soon as possible
    };

Q_SIGNALS:
    void quitLoop();

public:
    DebugWindow(QWidget *parent = 0);
    virtual ~DebugWindow();

    static DebugWindow *createInstance();
    static void destroyInstance();
    static DebugWindow *window();

    void setNextSourceInfo(QString url, int baseLine);

public:
    bool sourceParsed(ExecState *exec, int sourceId, const UString &sourceURL,
                      const UString &source, int startingLineNumber, int errorLine, const UString &errorMsg);
    bool sourceUnused(ExecState *exec, int sourceId);
    bool exception(ExecState *exec, int sourceId, int lineno, JSObject *exceptionObj);
    bool atStatement(ExecState *exec, int sourceId, int firstLine, int lastLine);
    bool callEvent(ExecState *exec, int sourceId, int lineno, JSObject *function, const List &args);
    bool returnEvent(ExecState *exec, int sourceId, int lineno, JSObject *function);

public Q_SLOTS:
    void stopExecution();
    void continueExecution();
    void stepInto();
    void stepOut();
    void stepOver();

protected:
    bool eventFilter(QObject *object, QEvent *event);
    void disableOtherWindows();
    void enableOtherWindows();

private Q_SLOTS:
    void displayScript(KJS::DebugDocument *document);
    void closeTab();
    void markSet(KTextEditor::Document *document, KTextEditor::Mark mark,
                 KTextEditor::MarkInterface::MarkChangeAction action);

private:
    void createActions();
    void createMenus();
    void createToolBars();
    void createStatusBar();
    void createTabWidget();

    void enterDebugSession(KJS::ExecState *exec, DebugDocument *document);
    void enableKateHighlighting(KTextEditor::Document *document);

    void enterModality();
    void leaveModality();

    void enterLoop();
    void exitLoop();

private:
    // Standard actions
    KAction *m_exitAct;

    // Flow control actions
    KAction *m_continueAct;
    KAction *m_stopAct;
    KAction *m_stepIntoAct;
    KAction *m_stepOutAct;
    KAction *m_stepOverAct;

    // Text editing stuff
    KTextEditor::Editor *m_editor;

//     WatchesDock *m_watches;
    LocalVariablesDock *m_localVariables;
    ScriptsDock *m_scripts;
    CallStackDock *m_callStack;
    BreakpointsDock *m_breakpoints;
    ConsoleDock *m_console;

    QTabWidget *m_tabWidget;
    QFrame *m_docFrame;

    // Internal temp variables to overcome some issues with KJS::Debugger...
    int m_nextBaseLine;
    QString m_nextUrl;

    Mode m_mode;

    QList<KTextEditor::Document*>  m_documentList;
    QHash<QString, DebugDocument*> m_documents;      // map url's to internal debug documents
    QHash<int, DebugDocument*>     m_sourceIdLookup; // map sourceId's to debug documents
    QHash<KTextEditor::Document*, DebugDocument*> m_documentLut; // map KTextEditor::Document's to DebugDocuments
    QHash<DebugDocument*, KTextEditor::Document*> m_debugLut; // map DebugDocument's to KTextEditor::Document's
    QList<DebugDocument*> m_openDocuments;

    static DebugWindow *m_debugger;
};




} // namespace




#endif // DEBUGWINDOW_H
