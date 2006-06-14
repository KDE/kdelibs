/*
 *  This file is part of the KDE libraries
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

#ifndef DEBUGWINDOW_H
#define DEBUGWINDOW_H

#define KJS_DEBUGGER

#include <kinstance.h>
#include <kmainwindow.h>

#include <kjs/debugger.h>
#include <kjs/completion.h>
#include <kjs/interpreter.h>
#include <kjs/value.h>
#include <kjs_binding.h>

class KActionCollection;
class KAction;

class NumberedTextView;
class WatchesDock;
class LocalVariablesDock;
class ScriptsDock;
class CallStackDock;
class BreakpointsDock;
class ConsoleDock;

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
class DebugWindow : public KMainWindow, public KJS::Debugger, public KInstance
{
    Q_OBJECT
public:
    DebugWindow(QWidget *parent = 0);
    virtual ~DebugWindow();

    static DebugWindow *createInstance();
    static void destroyInstance();
    static DebugWindow *window();

    void setNextSourceInfo(QString url, int baseLine);

public:
    // KJS Debugger Methods
    bool sourceParsed(ExecState *exec, int sourceId, const UString &sourceURL,
                      const UString &source, int errorLine);
    bool sourceUnused(ExecState *exec, int sourceId);
    bool exception(ExecState *exec, int sourceId, int lineno, JSObject *exceptionObj);
    bool atStatement(ExecState *exec, int sourceId, int firstLine, int lastLine);
    bool callEvent(ExecState *exec, int sourceId, int lineno, JSObject *function, const List &args);
    bool returnEvent(ExecState *exec, int sourceId, int lineno, JSObject *function);

public slots:
    void stopExecution();
    void continueExecution();
    void stepInto();
    void stepOut();
    void stepOver();

protected:
/*
    void closeEvent(QCloseEvent *e);
    bool eventFilter(QObject *obj, QEvent *evt);
*/

private:
    void createActions();
    void createMenus();
    void createToolBars();
    void createStatusBar();

    // Standard actions
    KAction *m_exitAct;

    // Flow control actions
    KAction *m_continueAct;
    KAction *m_stopAct;
    KAction *m_stepIntoAct;
    KAction *m_stepOutAct;
    KAction *m_stepOverAct;

    NumberedTextView *m_sourceEdit;
    WatchesDock *m_watches;
    LocalVariablesDock *m_localVariables;
    ScriptsDock *m_scripts;
    CallStackDock *m_callStack;
    BreakpointsDock *m_breakpoints;
    ConsoleDock *m_console;



    static DebugWindow *m_debugger;
};

} // namespace

#endif // DEBUGWINDOW_H
