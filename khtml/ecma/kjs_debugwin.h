/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 2000-2001 Harri Porten (porten@kde.org)
 *  Copyright (C) 2001 Peter Kelly (pmk@post.com)
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
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef _KJS_DEBUGGER_H_
#define _KJS_DEBUGGER_H_

#include <qglobal.h>

//#define KJS_DEBUGGER

#ifdef KJS_DEBUGGER

#include <qwidget.h>
#include <qpixmap.h>
#include <qptrlist.h>

#include <kjs/debugger.h>

#include "dom/dom_misc.h"

class QListBox;
class QComboBox;
class QMultiLineEdit;
class QPushButton;

namespace KJS {
  class FunctionImp;
  class List;
  class Interpreter;
};

/**
 * @internal
 *
 * A hack to allow KJSDebugWin to act as a modal window only some of the time
 * (i.e. when it is paused, to prevent the user quitting the app during a
 * debugging session)
 */
class FakeModal : public QWidget
{
    Q_OBJECT
public:
    FakeModal() {}
    void enable(QWidget *modal);
    void disable();

protected:
    bool eventFilter( QObject *obj, QEvent *evt );
    QWidget *modalWidget;
};


/**
 * @internal
 *
 * Represents a frame on the execution stack. The top frame is the global code for
 * the script, and each frame below it represents a function call.
 */
class StackFrame {
 public:
  StackFrame(int s, int l, QString n, bool stepped)
    : sourceId(s), lineno(l), name(n), step(stepped),
    next(stepped) {}
  QString toString();

  int sourceId;
  int lineno;
  QString name;
  bool step;
  bool next;
};

class SourceFile : public DOM::DomShared {
 public:
  SourceFile(QString u, QString c, int i, KJS::Interpreter *interp) 
      : url(u), code(c), index(i), interpreter(interp) {}
  QString url;
  QString code;
  int index;
  KJS::Interpreter *interpreter;
};

/**
 * @internal
 *
 * When kjs parses some code, it generates a source code fragment (or just "source").
 * This is referenced by its source id in future calls to functions such as atLine()
 * and callEvent(). We keep a record of all source fragments parsed in order to display
 * then to the user.
 *
 * For .js files, the source fragment will be the entire file. For js code included
 * in html files, however, there may be multiple source fragments within the one file
 * (e.g. multiple SCRIPT tags or onclick="..." attributes)
 *
 * In the case where a single file has multiple source fragments, the source objects
 * for these fragments will all point to the same SourceFile for their code.
 */
class SourceFragment {
 public:
  SourceFragment(int sid, int bl, SourceFile *sf);
  ~SourceFragment();

  int sourceId;
  int baseLine;
  SourceFile *sourceFile;
};

/**
 * @internal
 *
 * KJSDebugWin represents the debugger window that is visible to the user. It contains
 * a stack frame list, a code viewer and a source fragment selector, plus buttons
 * to control execution including next, step and continue.
 *
 * There is only one debug window per program. This can be obtained by calling #instance
 */
class KJSDebugWin : public QWidget, public KJS::Debugger {
  Q_OBJECT
public:
  KJSDebugWin(QWidget *parent=0, const char *name=0);
  virtual ~KJSDebugWin();

  static KJSDebugWin *createInstance();
  static void destroyInstance();
  static KJSDebugWin *instance() { return kjs_html_debugger; }

  enum Mode { Disabled = 0, // No break on any statements
	      Next     = 1, // Will break on next statement in current context
	      Step     = 2, // Will break on next statement in current or deeper context
	      Continue = 3, // Will continue until next breakpoint
	      Stop     = 4  // The script will stop execution completely,
	                    // as soon as possible
  };

  void highLight(int sourceId, int line);
  void setNextSourceInfo(QString url, int baseLine);
  void setSourceFile(QString url, QString code, KJS::Interpreter* interp);
  void appendSourceFile(QString url, QString code, KJS::Interpreter* interp);
  bool inSession() const { return m_inSession; }
  void setMode(Mode m) { m_mode = m; }
  void clear(KJS::Interpreter *interp);
  KJS::ExecState * getExecState() const { return m_curExecState; }

  // functions overridden from KJS:Debugger
  bool sourceParsed(KJS::ExecState *exec, int sourceId,
                    const KJS::UString &source, int errorLine);
  bool sourceUnused(KJS::ExecState * exec, int sourceId);
  bool exception(KJS::ExecState *exec, int sourceId, 
                 int lineno, KJS::Object &exceptionObj);
  bool atStatement(KJS::ExecState *exec, int sourceId, 
                   int firstLine, int lastLine);
  bool callEvent(KJS::ExecState *exec, int sourceId, int lineno,
          KJS::Object &function, const KJS::List &args);
  bool returnEvent(KJS::ExecState *exec, int sourceId, 
                   int lineno, KJS::Object &function);
public slots:
  void next();
  void step();
  void cont();
  void stop();
  void breakNext();
  void toggleBreakpoint();
  void showFrame(int frameno);
  void sourceSelected(int sourceSelIndex);
  void eval();

protected:

  virtual void closeEvent(QCloseEvent *e);

private:
  void enterSession();
  void leaveSession();
  void setCode(const QString &code,int SourceId);
  void updateFrameList();

  struct Breakpoint {
    int lineno;
    Breakpoint *next;
  };
  struct SourceBreakpoints {
    int sourceId;
    Breakpoint *breakpoints;
    SourceBreakpoints *next;
  };
  SourceBreakpoints *m_sourceBreakpoints;

  bool setBreakpoint(int sourceId, int line);
  bool deleteBreakpoint(int sourceId, int line);
  void clearAllBreakpoints(int sourceId = -1);
  int breakpointLine(int sourceId, int line0, int line1);
  bool haveBreakpoint(int sourceId, int line0, int line1);
  void setExecState(KJS::ExecState *exec);

  bool m_inSession;

  QListBox *m_sourceDisplay;
  QListBox *m_frameList;
  QPushButton *m_stepButton;
  QPushButton *m_nextButton;
  QPushButton *m_continueButton;
  QPushButton *m_stopButton;
  QPushButton *m_breakButton;
  QPushButton *m_breakpointButton;
  QComboBox *m_sourceSel;
  QPixmap m_stopIcon;
  QPixmap m_emptyIcon;
  QMultiLineEdit *m_evalEdit;

  SourceFile *m_curSourceFile;
  QPtrList<StackFrame> m_frames;
  Mode m_mode;
  /* url->SourceFile mapping*/
  QMap<QString,SourceFile*> m_sourceFiles;
  /* SourceId->SourceFragment mapping*/
  QMap<int,SourceFragment*> m_sourceFragments;
  /* combobox index->SourceFile mapping*/
  QMap<int,SourceFile*> m_sourceSelFiles;

  QString m_nextSourceUrl;
  int m_nextSourceBaseLine;
  FakeModal m_fakeModal;
  KJS::ExecState *m_curExecState;
  static KJSDebugWin *kjs_html_debugger;
};

#endif // KJS_DEBUGGER

#endif // _KJS_DEBUGGER_H_
