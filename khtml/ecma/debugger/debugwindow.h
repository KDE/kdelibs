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

#include <kinstance.h>
#include <kmainwindow.h>

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
#include "misc/decoder.h"
#include "dom/dom_misc.h"

class KActionCollection;
class KAction;
class NumberedTextView;
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


struct BreakPoint
{
    int lineNumber;
    int column;

    DebugDocument *document;
};

struct SourceFragment
{
    int sourceId;
    int baseLine;
    QString source;
};

class DebugDocument : public QObject
{
    Q_OBJECT
public:
    DebugDocument(const QString &url, Interpreter *interpreter)
        : m_url(url), m_interpreter(interpreter)
    {
        QStringList splitUrl = url.split('/');
        if (!splitUrl.isEmpty())
        {
            while (m_name.isEmpty())
                m_name = splitUrl.takeLast();
        }
        else
            m_name = "undefined";

        if (m_interpreter)
        {
            ScriptInterpreter *scriptInterpreter = static_cast<ScriptInterpreter*>(m_interpreter);
            KHTMLPart *part = qobject_cast<KHTMLPart*>(scriptInterpreter->part());
            if (part &&
                m_url == part->url().url())
            {
                connect(part, SIGNAL(completed()), this, SLOT(readSource()));
            }
        }
    }
    ~DebugDocument() {}

    QString name() const { return m_name; }
    QString url() const { return m_url; }
    Interpreter *interpreter() const { return m_interpreter; }

    QList<SourceFragment> fragments() { return m_codeFragments.values(); }
    bool deleteFragment(int sourceId)
    {
        if (m_codeFragments.contains(sourceId))
        {
            m_codeFragments.remove(sourceId);
            return true;
        }
        return false;
    }
    SourceFragment fragment(int sourceId)
    {
        if (m_codeFragments.contains(sourceId))
            return m_codeFragments[sourceId];
        else
            return SourceFragment();
    }

    QString source() const { return m_source; }

    void addCodeFragment(int sourceId, int baseLine, const QString &source)
    {
        SourceFragment code;
        code.sourceId = sourceId;
        code.baseLine = baseLine;
        code.source = source;

        m_codeFragments[sourceId] = code;
    }

private slots:
    void readSource()
    {
        if (m_interpreter)
        {
            ScriptInterpreter *scriptInterpreter = static_cast<ScriptInterpreter*>(m_interpreter);
            KHTMLPart *part = qobject_cast<KHTMLPart*>(scriptInterpreter->part());
            if (part &&
                m_url == part->url().url() &&
                !part->inProgress())
            {
                m_source = part->documentSource();
            }
        }
    }

private:
    QString m_url;
    QString m_name;
    QString m_source;
    Interpreter *m_interpreter;
    QHash<int, SourceFragment> m_codeFragments;

};


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
    enum Mode
    {
        Disabled = 0, // No break on any statements
        Next     = 1, // Will break on next statement in current context
        Step     = 2, // Will break on next statement in current or deeper context
        Continue = 3, // Will continue until next breakpoint
        Stop     = 4  // The script will stop execution completely,
                      // as soon as possible
    };

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
                      const UString &source, int startingLineNumber, int errorLine, const UString &errorMsg);
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

private slots:
    void displayScript(KJS::DebugDocument *document);
    void closeTab();
    void breakpointSet(KTextEditor::Document *document, KTextEditor::Mark mark,
                       KTextEditor::MarkInterface::MarkChangeAction action);

private:
    void createActions();
    void createMenus();
    void createToolBars();
    void createStatusBar();
    void createTabWidget();

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
    QList<KTextEditor::Document*> m_documentList;

    WatchesDock *m_watches;
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

    QHash<QString, DebugDocument*> m_documents;      // map url's to internal debug documents
    QHash<int, DebugDocument*>     m_sourceIdLookup; // map sourceId's to debug documents
    QList<DebugDocument*> m_openDocuments;

    static DebugWindow *m_debugger;
};

} // namespace

#endif // DEBUGWINDOW_H
