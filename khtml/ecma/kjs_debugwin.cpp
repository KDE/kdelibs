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
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "kjs_debugwin.h"
#include "kjs_proxy.h"

#ifdef KJS_DEBUGGER

#include <assert.h>
#include <stdlib.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qtextedit.h>
#include <qlistbox.h>
#include <qmultilineedit.h>
#include <qapplication.h>
#include <qsplitter.h>
#include <qcombobox.h>
#include <qbitmap.h>
#include <qwidgetlist.h>
#include <qlabel.h>

#include <klocale.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <kglobal.h>
#include <kmessagebox.h>
#include <kguiitem.h>

#include "kjs_dom.h"
#include <kjs/ustring.h>
#include <kjs/object.h>
#include <kjs/function.h>
#include <kjs/interpreter.h>

using namespace KJS;

KJSDebugWin * KJSDebugWin::kjs_html_debugger = 0;

bool FakeModal::eventFilter( QObject *o, QEvent *e )
{
  // ### does this prevent the use of alert boxes brought up
  // during a debugging session?
  switch (e->type()) {
  case QEvent::MouseButtonPress:
  case QEvent::MouseButtonRelease:
  case QEvent::MouseButtonDblClick:
  case QEvent::MouseMove:
  case QEvent::KeyPress:
  case QEvent::KeyRelease:
  case QEvent::Destroy:
  case QEvent::Close:
  case QEvent::Quit:
    while (o->parent())
      o = o->parent();
    if (o == modalWidget)
      return QWidget::eventFilter( o, e );
    else
      return TRUE;
    break;
  default:
    return QWidget::eventFilter( o, e );
  }
}


void FakeModal::enable(QWidget *modal)
{
  QWidgetList *widgets = QApplication::allWidgets();
  QWidgetListIt it(*widgets);
  for (; it.current(); ++it)
    it.current()->installEventFilter(this);
  modalWidget = modal;
}

void FakeModal::disable()
{
  QWidgetList *widgets = QApplication::allWidgets();
  QWidgetListIt it(*widgets);
  for (; it.current(); ++it)
    it.current()->removeEventFilter(this);
  modalWidget = 0;
}

//-------------------------------------------------------------------------

SourceFragment::SourceFragment(int sid, int bl, SourceFile *sf)
{
  sourceId = sid;
  baseLine = bl;
  sourceFile = sf;
  sourceFile->ref();
}

SourceFragment::~SourceFragment()
{
  sourceFile->deref();
}

//-------------------------------------------------------------------------

KJSErrorDialog::KJSErrorDialog(QWidget *parent, const QString& errorMessage)
  : KDialogBase(parent,0,true,i18n("JavaScript error"),
		KDialogBase::Ok|KDialogBase::User1,KDialogBase::Ok,false,
		KGuiItem("&Debug","gear"))
{
  QWidget *page = new QWidget(this);
  setMainWidget(page);

  QLabel *iconLabel = new QLabel("",page);
  iconLabel->setPixmap(KGlobal::iconLoader()->loadIcon("messagebox_critical",
						       KIcon::NoGroup,KIcon::SizeMedium,
						       KIcon::DefaultState,0,true));

  QWidget *contents = new QWidget(page);
  QLabel *label = new QLabel(errorMessage,contents);
  QCheckBox *cb = new QCheckBox(i18n("&Do not show this message again"),contents);

  QVBoxLayout *vl = new QVBoxLayout(contents,0,spacingHint());
  vl->addWidget(label);
  vl->addWidget(cb);

  QHBoxLayout *topLayout = new QHBoxLayout(page,0,spacingHint());
  topLayout->addWidget(iconLabel);
  topLayout->addWidget(contents);
  topLayout->addStretch(10);

  m_debugSelected = false;
}

KJSErrorDialog::~KJSErrorDialog()
{
}

void KJSErrorDialog::slotUser1()
{
  m_debugSelected = true;
  close();
}

//-------------------------------------------------------------------------
KJSDebugWin::KJSDebugWin(QWidget *parent, const char *name)
  : QWidget(parent, name)
{
  m_breakpoints = 0;
  m_breakpointCount = 0;

  m_curSourceFile = 0;
  m_mode = Continue;
  m_nextSourceUrl = "";
  m_nextSourceBaseLine = 1;

  m_stopIcon = KGlobal::iconLoader()->loadIcon("stop",KIcon::Small);
  m_emptyIcon = QPixmap(m_stopIcon.width(),m_stopIcon.height());
  QBitmap emptyMask(m_stopIcon.width(),m_stopIcon.height(),true);
  m_emptyIcon.setMask(emptyMask);

  setCaption(i18n("JavaScript Debugger"));
  QVBoxLayout *vl = new QVBoxLayout(this, 5);

  // frame list & code
  QSplitter *hsplitter = new QSplitter(Qt::Vertical, this);
  QSplitter *vsplitter = new QSplitter(hsplitter);
  QFont font("fixed",10);

  m_contextList = new QListBox(vsplitter);
  m_contextList->setFont(font);
  m_contextList->setMinimumSize(100,200);
  connect(m_contextList,SIGNAL(highlighted(int)),this,SLOT(showFrame(int)));

  // source selection & display
  QWidget *sourceSelDisplay = new QWidget(vsplitter);
  QVBoxLayout *ssdvl = new QVBoxLayout(sourceSelDisplay);

  m_sourceSel = new QComboBox(sourceSelDisplay);
  connect(m_sourceSel,SIGNAL(activated(int)),this,SLOT(sourceSelected(int)));
  ssdvl->addWidget(m_sourceSel);

  m_sourceDisplay = new QListBox(sourceSelDisplay);
  m_sourceDisplay->setFont(font);
  ssdvl->addWidget(m_sourceDisplay);
  connect(m_sourceDisplay, SIGNAL(doubleClicked(QListBoxItem*)), SLOT(toggleBreakpoint()));

  QValueList<int> vsplitSizes;
  vsplitSizes.insert(vsplitSizes.end(),200);
  vsplitSizes.insert(vsplitSizes.end(),400);
  vsplitter->setSizes(vsplitSizes);

  // evaluate
  m_evalEdit = new QMultiLineEdit(hsplitter);
  m_evalEdit->setReadOnly(true);
  m_evalEdit->setWordWrap(QMultiLineEdit::NoWrap);
  m_evalEdit->setFont(font);
  connect(m_evalEdit, SIGNAL(returnPressed()), SLOT(eval()));

  QValueList<int> hsplitSizes;
  hsplitSizes.insert(hsplitSizes.end(),400);
  hsplitSizes.insert(hsplitSizes.end(),200);
  hsplitter->setSizes(hsplitSizes);

  vl->addWidget(hsplitter);

  // control buttons
  QHBoxLayout *hl2 = new QHBoxLayout(vl);
  m_nextButton = new QPushButton(i18n("&Next"), this);
  m_stepButton = new QPushButton(i18n("&Step"), this);
  m_continueButton = new QPushButton(i18n("&Continue"), this);
  m_stopButton = new QPushButton(i18n("St&op"), this);
  m_breakButton = new QPushButton(i18n("&Break at next Statement"), this);
  m_breakpointButton = new QPushButton(i18n("&Toggle Breakpoint"), this);
  hl2->addWidget(m_nextButton);
  hl2->addWidget(m_stepButton);
  hl2->addWidget(m_continueButton);
  hl2->addWidget(m_stopButton);
  hl2->addWidget(m_breakButton);
  hl2->addWidget(m_breakpointButton);
  hl2->addStretch();

  connect(m_nextButton, SIGNAL(clicked()), SLOT(next()));
  connect(m_stepButton, SIGNAL(clicked()), SLOT(step()));
  connect(m_continueButton, SIGNAL(clicked()), SLOT(cont()));
  connect(m_stopButton, SIGNAL(clicked()), SLOT(stop()));
  connect(m_breakButton, SIGNAL(clicked()), SLOT(breakNext()));
  connect(m_breakpointButton, SIGNAL(clicked()), SLOT(toggleBreakpoint()));

  m_nextButton->setEnabled(false);
  m_stepButton->setEnabled(false);
  m_continueButton->setEnabled(false);
  m_stopButton->setEnabled(false);
  m_breakButton->setEnabled(true);
  m_breakpointButton->setEnabled(false);

  setMinimumSize(300,200);
  resize(600,450);

  updateContextList();
}

KJSDebugWin::~KJSDebugWin()
{
  free(m_breakpoints);
}

KJSDebugWin *KJSDebugWin::createInstance()
{
  assert(!kjs_html_debugger);
  kjs_html_debugger = new KJSDebugWin();
  return kjs_html_debugger;
}

void KJSDebugWin::destroyInstance()
{
  assert(kjs_html_debugger);
  kjs_html_debugger->hide();
  delete kjs_html_debugger;
}

void KJSDebugWin::next()
{
  m_mode = Next;
  leaveSession();
}

void KJSDebugWin::step()
{
  m_mode = Step;
  leaveSession();
}

void KJSDebugWin::cont()
{
  m_mode = Continue;
  leaveSession();
}

void KJSDebugWin::stop()
{
  m_mode = Stop;
  leaveSession();
}

void KJSDebugWin::breakNext()
{
  m_mode = Step;
}

void KJSDebugWin::toggleBreakpoint()
{
  if (m_sourceSel->currentItem() < 0 || m_sourceDisplay->currentItem() < 0)
    return;

  SourceFile *sourceFile = m_sourceSelFiles[m_sourceSel->currentItem()];
  int lineno = m_sourceDisplay->currentItem()+1; // line numbers start from 1

  // Find the source fragment containing the selected line (if any)
  int sourceId = -1;
  int highestBaseLine = -1;
  QMap<int,SourceFragment*>::Iterator it;

  for (it = m_sourceFragments.begin(); it != m_sourceFragments.end(); ++it) {
    SourceFragment *sourceFragment = it.data();
    if (sourceFragment &&
	sourceFragment->sourceFile == sourceFile &&
	sourceFragment->baseLine <= lineno &&
	sourceFragment->baseLine > highestBaseLine) {

	sourceId = sourceFragment->sourceId;
	// ### need to keep track of last line in sourceFragment
	highestBaseLine = sourceFragment->baseLine;
    }
  }

  if (sourceId < 0)
    return;

  QString text(m_sourceDisplay->item(lineno-1)->text());
  m_sourceDisplay->removeItem(lineno-1);
  QListBoxPixmap *item;
  int fragmentLineno = lineno-highestBaseLine+1;
  if (setBreakpoint(sourceId,fragmentLineno)) {
    item = new QListBoxPixmap(m_stopIcon,text);
  }
  else {
    deleteBreakpoint(sourceId,fragmentLineno);
    item = new QListBoxPixmap(m_emptyIcon,text);
  }
  m_sourceDisplay->insertItem(item,lineno-1);
  m_sourceDisplay->setCurrentItem(lineno-1);
}

void KJSDebugWin::showFrame(int frameno)
{
  if (frameno < 0 || frameno >= contextDepth())
    return;

  Context ctx = getContext(frameno);
  setSourceLine(ctx.sourceId(),ctx.curStmtFirstLine());
}

void KJSDebugWin::sourceSelected(int sourceSelIndex)
{
  // a souce file has been selected from the drop-down list - display the file
  // and hilight the line if it's in the current stack frame
  if (sourceSelIndex < 0 || sourceSelIndex >= (int)m_sourceSel->count())
    return;
  SourceFile *sourceFile = m_sourceSelFiles[sourceSelIndex];
  bool newsource = m_curSourceFile != sourceFile;
  m_curSourceFile = sourceFile;

  if (newsource)
    displaySourceFile(sourceFile);

  // If the currently selected stack frame is in the current source file, then hilight
  // the line it's on.
  if (m_contextList->currentItem() >= 0) {
    Context ctx = getContext(m_contextList->currentItem());
    setSourceLine(ctx.sourceId(),ctx.curStmtFirstLine());
  }
}

void KJSDebugWin::eval()
{
  // ### Make this work when we're not in a current script execution. Need to work out
  // which interpreter to use based on the current source file - there could be multiple
  // windows open but the debug window is a singleton.

  if (m_execStates.isEmpty())
    return;

  // evaluate the js code from m_evalEdit
  ExecState *exec = m_execStates.top();
  int para, index;
  m_evalEdit->getCursorPosition(&para, &index);
  UString code(m_evalEdit->text(para-1));
  QString msg;

  KJSCPUGuard guard;
  guard.start();

  Interpreter *interp = exec->interpreter();
  Value retval;
  assert(!exec->context().isNull());

  Object obj = Object::dynamicCast(interp->globalObject().get(exec, "eval"));
  List args;
  args.append(String(code));

  Object thisobj = exec->context().thisValue();
  retval = obj.call(exec, thisobj, args);
  guard.stop();

  if (exec->hadException()) {
    msg = "Exception: " + exec->exception().toString(interp->globalExec()).qstring();
    exec->clearException();
  }
  else {
    msg = retval.toString(interp->globalExec()).qstring();
  }

  m_evalEdit->insertParagraph(msg,para);
  m_evalEdit->moveCursor(QMultiLineEdit::MoveDown,false);

  updateContextList();
}

void KJSDebugWin::closeEvent(QCloseEvent *e)
{
  while (!m_execStates.isEmpty()) // ### not sure if this will work
    leaveSession();
  return QWidget::closeEvent(e);
}

bool KJSDebugWin::sourceParsed(KJS::ExecState *exec, int sourceId,
                               const KJS::UString &source, int /*errorLine*/)
{
  // the interpreter has parsed some js code - store it in a SourceFragment object
  // ### report errors (errorLine >= 0)
  SourceFile *sourceFile = m_sourceFiles[m_nextSourceUrl];
  if (!sourceFile) {
    if (!m_nextSourceUrl.isEmpty()) {
      setSourceFile(m_nextSourceUrl,source.qstring(),exec->interpreter());
      sourceFile = m_sourceFiles[m_nextSourceUrl];
    }
    else {
      sourceFile = new SourceFile("(unknown)",source.qstring(),m_sourceSel->count(), exec->interpreter());
      m_sourceSelFiles[sourceFile->index] = sourceFile;
      m_sourceSel->insertItem("???");
    }
  }

  SourceFragment *sf = new SourceFragment(sourceId,m_nextSourceBaseLine,sourceFile);
  m_sourceFragments[sourceId] = sf;


  m_nextSourceBaseLine = 1;
  m_nextSourceUrl = "";

  return (m_mode != Stop);
}

bool KJSDebugWin::sourceUnused(KJS::ExecState * /*exec*/, int sourceId)
{
  // the source fragment is no longer in use, so we can free it

  SourceFragment *fragment = m_sourceFragments[sourceId];
  if (fragment) {
      m_sourceFragments.erase(sourceId);
      SourceFile *sourceFile = fragment->sourceFile;
      if (sourceFile->hasOneRef()) {
          m_sourceSel->removeItem(sourceFile->index);
          for (int i = sourceFile->index+1; i < m_sourceSel->count(); i++) {
              m_sourceSelFiles[i]->index--;
              m_sourceSelFiles[i-1] = m_sourceSelFiles[i];
          }
          m_sourceSelFiles.erase(m_sourceSel->count());
          m_sourceFiles.erase(sourceFile->url);
      }
      delete fragment;
  }
  return (m_mode != Stop);
}

bool KJSDebugWin::exception(ExecState *exec, const Value &value, bool /*inTryCatch*/)
{
  assert(m_currentContext.imp());

  SourceFragment *sourceFragment = m_sourceFragments[m_currentContext.sourceId()];
  QString msg = i18n("An error occurred while attempting to run a script on this page.\n\n%1 line %2:\n%3")
		.arg(sourceFragment->sourceFile->url)
		.arg(sourceFragment->baseLine+m_currentContext.curStmtFirstLine()-1)
		.arg(value.toString(exec).qstring());

  KJSErrorDialog dlg(this,msg);
  dlg.exec();
  // ### disable javascript error reporting option if don't show again is checked

  if (dlg.debugSelected()) {
    m_mode = Next;
    m_steppingContext = m_currentContext;
    enterSession(exec);
  }

  return (m_mode != Stop);
}

bool KJSDebugWin::atStatement(KJS::ExecState *exec)
{
  checkBreak(exec);
  return (m_mode != Stop);
}

bool KJSDebugWin::enterContext(ExecState *exec)
{
  assert(exec->context().callingContext() == m_currentContext);
  m_currentContext = exec->context();
  if (m_mode == Step)
    m_steppingContext = m_currentContext;

  checkBreak(exec);
  return (m_mode != Stop);
}

bool KJSDebugWin::exitContext(ExecState *exec, const Completion &/*completion*/)
{
  assert(m_currentContext.imp());

  checkBreak(exec);

  if (m_steppingContext == m_currentContext)
    m_steppingContext = m_currentContext.callingContext();
  m_currentContext = m_currentContext.callingContext();

  if (!m_currentContext.imp()) { 
    updateContextList();
    m_sourceDisplay->clearSelection();
  }
  else if (m_mode == Step || m_mode == Next) {
    m_currentContext = m_currentContext; // ### needed?
  }

  return (m_mode != Stop);
}

void KJSDebugWin::displaySourceFile(SourceFile *sourceFile)
{
  const QChar *chars = sourceFile->code.unicode();
  uint len = sourceFile->code.length();
  QChar newLine('\n');
  QChar cr('\r');
  QChar tab('\t');
  QString tabstr("        ");
  QString line;
  m_sourceDisplay->clear();
  int lineno = 1;
  for (uint i = 0; i < len; i++) {
    QChar c = chars[i];
    if (c == cr) {
      if (i < len-1 && chars[i+1] == newLine)
	continue;
      else
	c = newLine;
    }
    if (c == newLine) {
      QPixmap icon = haveBreakpoint(sourceFile,lineno+1,lineno+1) ? m_stopIcon : m_emptyIcon;
      QString str = QString().sprintf("%4d",m_sourceDisplay->count()+1) + " " + line;
      m_sourceDisplay->insertItem(new QListBoxPixmap(icon,str));
      lineno++;
      line = "";
    } else if (c == tab) {
      line += tabstr;
    } else
      line += c;
  }
  if (line.length()) {
    QPixmap icon = haveBreakpoint(sourceFile,lineno+1,lineno+1) ? m_stopIcon : m_emptyIcon;
    QString str = QString().sprintf("%4d",m_sourceDisplay->count()+1) + " " + line;
    m_sourceDisplay->insertItem(new QListBoxPixmap(icon,str));
  }
}

void KJSDebugWin::setSourceLine(int sourceId, int lineno)
{
  SourceFragment *source = m_sourceFragments[sourceId];
  if (!source)
    return;

  SourceFile *sourceFile = source->sourceFile;
  if (m_curSourceFile != source->sourceFile) {
      m_sourceSel->setCurrentItem(source->sourceFile->index);
      displaySourceFile(sourceFile);
  }
  m_curSourceFile = sourceFile;
  if (lineno > 0) {
    m_sourceDisplay->setSelected(source->baseLine+lineno-2,true);
    m_sourceDisplay->ensureCurrentVisible();
  }
  else {
    m_sourceDisplay->clearSelection();
  }
}

void KJSDebugWin::setNextSourceInfo(QString url, int baseLine)
{
  m_nextSourceUrl = url;
  m_nextSourceBaseLine = baseLine;
}

void KJSDebugWin::setSourceFile(QString url, QString code, Interpreter* interp)
{
  SourceFile *srcfile = m_sourceFiles[url];
  if (srcfile) {
    srcfile->code = code;
    srcfile->interpreter = interp;
  } else {
    int index = m_sourceSel->count();
    srcfile = new SourceFile(url, code, index, interp);
    m_sourceSel->insertItem(url, index);
    m_sourceFiles[url] = srcfile;
    m_sourceSelFiles[index] = srcfile;
  }
  if (m_sourceSel->currentItem() >= 0 && m_sourceSel->currentText() == url) {
    m_curSourceFile = 0; // force refresh
    sourceSelected(m_sourceSel->currentItem());
  }
}

void KJSDebugWin::clear(Interpreter* interp) {
  // ###
  QMap<QString,SourceFile*>::iterator iter = m_sourceFiles.begin();
  QMap<QString,SourceFile*>::iterator remove = m_sourceFiles.end();
  for (; iter != m_sourceFiles.end(); iter++) {
    if (remove != m_sourceFiles.end()) {
      delete *remove;
      m_sourceFiles.erase(remove);
      remove = m_sourceFiles.end();
    }
    SourceFile *srcfile = *iter;
    if (!srcfile)
      remove = iter;
    else if (srcfile->interpreter == interp && srcfile->refCount() <= 0) {
      remove = iter;
      m_sourceSel->removeItem(srcfile->index);
      for (int i = srcfile->index; i < int(m_sourceSelFiles.count())-1; i++) {
        m_sourceSelFiles[i] = m_sourceSelFiles[i+1];
        m_sourceSelFiles[i]->index--;
      }
      m_sourceSelFiles.erase(m_sourceSelFiles.count()-1);
      if (m_curSourceFile == srcfile)
        m_curSourceFile = 0;
    }
  }
  if (remove != m_sourceFiles.end()) {
    delete *remove;
    m_sourceFiles.erase(remove);
  }
}

void KJSDebugWin::appendSourceFile(QString url, QString code, Interpreter* interp)
{
  SourceFile *existing = m_sourceFiles[url];
  if (!existing) {
    setSourceFile(url,code, interp);
    return;
  }
  existing->code.append(code);
  if (m_sourceSel->currentItem() >= 0 && m_sourceSel->currentText() == url) {
    m_curSourceFile = 0; // force refresh
    sourceSelected(m_sourceSel->currentItem());
  }
}

void KJSDebugWin::checkBreak(ExecState *exec)
{
  if (haveBreakpoint(m_currentContext.sourceId(),
		     m_currentContext.curStmtFirstLine(),
		     m_currentContext.curStmtLastLine())) {
    m_mode = Next;
    m_steppingContext = m_currentContext;
  }

  if ((m_mode == Step || m_mode == Next) && m_steppingContext == m_currentContext)
    enterSession(exec);
}

void KJSDebugWin::enterSession(ExecState *exec)
{
  // This "enters" a new debugging session, i.e. enables usage of the debugging window
  // It re-enters the qt event loop here, allowing execution of other parts of the
  // program to continue while the script is stopped. We have to be a bit careful here,
  // i.e. make sure the user can't quit the app, and disable other event handlers which
  // could interfere with the debugging session.
  if (!isVisible())
    show();

  m_mode = Continue;

  if (m_execStates.isEmpty()) {
    m_fakeModal.enable(this);
    m_nextButton->setEnabled(true);
    m_stepButton->setEnabled(true);
    m_continueButton->setEnabled(true);
    m_stopButton->setEnabled(true);
    m_breakButton->setEnabled(false);
    m_breakpointButton->setEnabled(true);
    m_evalEdit->setReadOnly(false);
  }
  m_execStates.push(exec);

  updateContextList();

  qApp->enter_loop(); // won't return until leaveSession() is called
}

void KJSDebugWin::leaveSession()
{
  // Disables debugging for this window and returns to execute the rest of the script
  // (or aborts execution, if the user pressed stop). When this returns, the program
  // will exit the qt event loop, i.e. return to whatever processing was being done
  // before the debugger was stopped.
  assert(!m_execStates.isEmpty());

  m_execStates.pop();

  if (m_execStates.isEmpty()) {
    m_nextButton->setEnabled(false);
    m_stepButton->setEnabled(false);
    m_continueButton->setEnabled(false);
    m_stopButton->setEnabled(false);
    m_breakButton->setEnabled(true);
    m_breakpointButton->setEnabled(false);
    m_sourceDisplay->clearSelection();
    m_evalEdit->setReadOnly(true);
    m_fakeModal.disable();
  }

  qApp->exit_loop();
}

void KJSDebugWin::updateContextList()
{
  disconnect(m_contextList,SIGNAL(highlighted(int)),this,SLOT(showFrame(int)));
  m_contextList->clear();

  Context ctx = m_currentContext;
  while (ctx.imp()) {
    m_contextList->insertItem(contextStr(ctx),0);
    ctx = ctx.callingContext();
  }

  if (m_currentContext.imp()) {
    m_contextList->setSelected(m_contextList->count()-1, true);
    setSourceLine(m_currentContext.sourceId(),m_currentContext.curStmtFirstLine());
  }

  connect(m_contextList,SIGNAL(highlighted(int)),this,SLOT(showFrame(int)));
}

Context KJSDebugWin::getContext(int depth)
{
  Context ctx = m_currentContext;
  for (int f = contextDepth()-1; f > depth; f--)
    ctx = ctx.callingContext();
  assert(ctx.imp());
  return ctx;
}

QString KJSDebugWin::contextStr(const Context &ctx)
{
  QString str = "";
  SourceFragment *sourceFragment = m_sourceFragments[ctx.sourceId()];
  QString url = sourceFragment->sourceFile->url;
  int fileLineno = sourceFragment->baseLine+ctx.curStmtFirstLine()-1;

  switch (ctx.codeType()) {
  case GlobalCode:
    str = QString("Global code at %1:%2").arg(url).arg(fileLineno);
    break;
  case EvalCode:
    str = QString("Eval code at %1:%2").arg(url).arg(fileLineno);
    break;
  case FunctionCode:
    if (!ctx.functionName().isNull())
      str = QString("%1() at %2:%3").arg(ctx.functionName().qstring()).arg(url).arg(fileLineno);
    else
      str = QString("Anonymous function at %1:%2").arg(url).arg(fileLineno);
    break;
  }

  return str;
}

int KJSDebugWin::contextDepth()
{
  int depth = 0;
  Context ctx = m_currentContext;
  while (ctx.imp()) {
    depth++;
    ctx = ctx.callingContext();
  }
  return depth;
}

bool KJSDebugWin::setBreakpoint(int sourceId, int lineno)
{
  if (haveBreakpoint(sourceId,lineno,lineno))
    return false;

  m_breakpointCount++;
  m_breakpoints = static_cast<Breakpoint*>(realloc(m_breakpoints,
						   m_breakpointCount*sizeof(Breakpoint)));
  m_breakpoints[m_breakpointCount-1].sourceId = sourceId;
  m_breakpoints[m_breakpointCount-1].lineno = lineno;

  return true;
}

bool KJSDebugWin::deleteBreakpoint(int sourceId, int lineno)
{
  for (int i = 0; i < m_breakpointCount; i++) {
    if (m_breakpoints[i].sourceId == sourceId && m_breakpoints[i].lineno == lineno) {

      memmove(m_breakpoints+i,m_breakpoints+i+1,(m_breakpointCount-i-1)*sizeof(Breakpoint));
      m_breakpointCount--;
      m_breakpoints = static_cast<Breakpoint*>(realloc(m_breakpoints,
						       m_breakpointCount*sizeof(Breakpoint)));
      return true;
    }
  }

  return false;
}

bool KJSDebugWin::haveBreakpoint(SourceFile *sourceFile, int line0, int line1)
{
  for (int i = 0; i < m_breakpointCount; i++) {
    int sourceId = m_breakpoints[i].sourceId;
    int lineno = m_breakpoints[i].lineno;
    if (m_sourceFragments[sourceId]->sourceFile == sourceFile) {
      int absLineno = m_sourceFragments[sourceId]->baseLine+lineno-1;
      if (absLineno >= line0 && absLineno <= line1)
	return true;
    }
  }

  return false;
}

#include "kjs_debugwin.moc"

#endif // KJS_DEBUGGER
