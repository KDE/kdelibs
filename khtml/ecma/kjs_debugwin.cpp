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

#include "kjs_debugwin.h"
#include "kjs_proxy.h"

#ifdef KJS_DEBUGGER

#include <assert.h>
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

#include <klocale.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <kmessagebox.h>

#include "kjs_dom.h"
#include <kjs/ustring.h>
#include <kjs/object.h>
#include <kjs/function.h>
#include <kjs/interpreter.h>

using namespace KJS;

KJSDebugWin * KJSDebugWin::kjs_html_debugger = 0;

bool FakeModal::eventFilter( QObject *o, QEvent *e )
{
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

QString StackFrame::toString()
{
  QString str = "";
  QString url = sourceFragment->sourceFile->url;
  int fileLineno = sourceFragment->baseLine+lineno-1;
  if (!name.isNull())
    str = QString("%1 at %2:%3").arg(name).arg(url).arg(fileLineno);
  else
    str = QString("%1:%2").arg(url).arg(fileLineno);
  return str;
}

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
KJSDebugWin::KJSDebugWin(QWidget *parent, const char *name)
  : QWidget(parent, name),
    m_inSession(false),
    m_curSourceFile(0)
{
  setCaption(i18n("JavaScript Debugger"));
  QVBoxLayout *vl = new QVBoxLayout(this, 5);

  // frame list & code
  QSplitter *hsplitter = new QSplitter(Qt::Vertical, this);
  QSplitter *vsplitter = new QSplitter(hsplitter);
  QFont font("fixed",10);

  m_frameList = new QListBox(vsplitter);
  m_frameList->setFont(font);
  m_frameList->setMinimumSize(100,200);
  connect(m_frameList,SIGNAL(highlighted(int)),this,SLOT(showFrame(int)));

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

  m_frames.setAutoDelete(true);
  m_frames.setAutoDelete(true);


  setMinimumSize(300,200);
  resize(600,450);
  m_mode = Continue;
  m_sourceBreakpoints = 0;

  KIconLoader loader;
  m_stopIcon = loader.loadIcon("stop",KIcon::Small);
  
  m_emptyIcon = QPixmap(m_stopIcon.width(),m_stopIcon.height());
  QBitmap emptyMask(m_stopIcon.width(),m_stopIcon.height(),true);
  //  m_emptyIcon.fill(m_sourceDisplay,0,0);
  m_emptyIcon.setMask(emptyMask);

  m_nextSourceBaseLine = 1;
  m_nextSourceUrl = "";

  updateFrameList();
  m_inSession = false;
}

KJSDebugWin::~KJSDebugWin()
{
}


KJSDebugWin *KJSDebugWin::createInstance()
{
  assert(!kjs_html_debugger);
  kjs_html_debugger = new KJSDebugWin();
  //kjs_html_debugger->show();
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
  if (frameno < 0 || frameno >= (int)m_frames.count())
    return;
  StackFrame *frame = m_frames.at(frameno);
  setSourceLine(frame->sourceFragment->sourceId,frame->lineno);
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
  if (m_frameList->currentItem() >= 0) {
    StackFrame *frame = m_frames.at(m_frameList->currentItem());
    setSourceLine(frame->sourceFragment->sourceId,frame->lineno);
  }
}

void KJSDebugWin::eval()
{
  // ### Make this work when we're not in a current script execution. Need to work out
  // which interpreter to use based on the current source file - there could be multiple
  // windows open but the debug window is a singleton.
  ExecState *exec = getExecState();

  // evaluate the js code from m_evalEdit
  if (!m_inSession || !exec)
    return;
  int para, index;
  m_evalEdit->getCursorPosition(&para, &index);
  UString code(m_evalEdit->text(para-1));

  KJSCPUGuard guard;
  guard.start();

  Interpreter *interp = exec->interpreter();
  Value retval;
  if (exec->context().isNull()) {
    Completion comp = interp->evaluate(code);
    if (comp.isValid())
      retval = comp.value();
  } else {
    Object obj = Object::dynamicCast(interp->globalObject().get(exec, "eval"));
    List args;
    args.append(String(code));

    Object thisobj = exec->context().thisValue();
    Value comp = obj.call(exec, thisobj, args);
    if (comp.type() == KJS::CompletionType) {
      CompletionImp *cimp = static_cast<CompletionImp*>(comp.imp());
      retval = cimp->value();
    } else if (comp.isValid())
      retval = comp;
  }
  guard.stop();

  if (retval.isValid()) {
    m_evalEdit->insertParagraph(retval.toString(interp->globalExec()).qstring(), para);
    m_evalEdit->moveCursor(QMultiLineEdit::MoveDown, false);
  }
  if (exec->hadException())
    exec->clearException();
}

void KJSDebugWin::closeEvent(QCloseEvent *e)
{
  if (m_inSession)
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
    sourceFile = new SourceFile("(unknown)",source.qstring(),m_sourceSel->count(), exec->interpreter());
    m_sourceSelFiles[sourceFile->index] = sourceFile;
    if (m_nextSourceUrl.isNull() || m_nextSourceUrl == "")
        m_sourceSel->insertItem("???");
    else
        m_sourceSel->insertItem(m_nextSourceUrl);
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

bool KJSDebugWin::exception(KJS::ExecState *exec, int sourceId, 
        int lineno, KJS::Object &exceptionObj)
{
  // ### bring up source & hilight line
  KMessageBox::error(this, exceptionObj.toString(exec).qstring(), "JavaScript error");
  return (m_mode != Stop);
}

bool KJSDebugWin::atStatement(KJS::ExecState *exec, int sourceId, 
                              int firstLine, int lastLine)
{
  assert(m_frames.count() > 0);

  assert(m_frames.last()->exec == exec);
  assert(m_frames.last()->sourceFragment->sourceId == sourceId);
  m_frames.last()->lineno = firstLine;

  if (haveBreakpoint(sourceId,firstLine,lastLine)) {
    m_mode = Next;
    m_frames.last()->next = true;
    if (!isVisible())
      show();
  }

  if ((m_mode == Step || m_mode == Next) && m_frames.last()->next)
    enterSession();

  return (m_mode != Stop);
}

bool KJSDebugWin::enterContext(ExecState *exec, CodeType codeType, int sourceId,
			       int lineno, Object &thisVal, Object &variable,
			       Object &function, const UString &name,
			       const List &args)
{
  assert(sourceId >= 0);
  assert(m_sourceFragments[sourceId]);

  QString nameStr;
  if (codeType == GlobalCode)
    nameStr = "Global code";
  else if (codeType == EvalCode)
    nameStr = "eval()";
  else if (codeType == FunctionCode && !name.isNull())
    nameStr = name.qstring();
  else
    nameStr = "(unknown)";

  m_frames.append(new StackFrame(exec,codeType,m_sourceFragments[sourceId],lineno,function,
				 thisVal,variable,nameStr,args,m_mode == Step));

  if (haveBreakpoint(sourceId,lineno,lineno)) {
    m_mode = Next;
    m_frames.last()->next = true;
    if (!isVisible())
      show();
  }

  if ((m_mode == Step || m_mode == Next) && m_frames.last()->next)
    enterSession();

  return (m_mode != Stop);
}

bool KJSDebugWin::exitContext(const Completion &/*completion*/, int lineno)
{
  assert(m_frames.count() > 0);

  m_frames.last()->lineno = lineno;

  if (haveBreakpoint(m_frames.last()->sourceFragment->sourceId,
		     m_frames.last()->lineno,m_frames.last()->lineno)) {
    m_mode = Next;
    m_frames.last()->next = true;
    if (!isVisible())
      show();
  }

  if ((m_mode == Step || m_mode == Next) && m_frames.last()->next)
    enterSession();

  m_frames.removeLast();
  if (m_frames.count() == 0) { 
    updateFrameList();
    m_sourceDisplay->clearSelection();
  }
  else if (m_mode == Step || m_mode == Next) {
    m_frames.last()->next = true;
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
  if (lineno > 0)
    m_sourceDisplay->setSelected(source->baseLine+lineno-2,true);
  else
    m_sourceDisplay->clearSelection();
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

ExecState *KJSDebugWin::getExecState()
{
  if (m_frames.count() > 0)
    return m_frames.at(m_frames.count()-1)->exec;
  else
    return 0;
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

void KJSDebugWin::enterSession()
{
  // This "enters" a new debugging session, i.e. enables usage of the debugging window
  // It re-enters the qt event loop here, allowing execution of other parts of the
  // program to continue while the script is stopped. We have to be a bit careful here,
  // i.e. make sure the user can't quit the app, and disable other event handlers which
  // could interfere with the debugging session.
  assert(!m_inSession);
  m_mode = Continue;

  if (!isVisible())
    return;

  // ### support re-entrant debugging sessions
  m_inSession = true;
  m_fakeModal.enable(this);
  m_nextButton->setEnabled(true);
  m_stepButton->setEnabled(true);
  m_continueButton->setEnabled(true);
  m_stopButton->setEnabled(true);
  m_breakButton->setEnabled(false);
  m_breakpointButton->setEnabled(true);
  m_evalEdit->setReadOnly(false);
  updateFrameList();

  qApp->enter_loop(); // won't return until leaveSession() is called
  assert(!m_inSession);
}

void KJSDebugWin::leaveSession()
{
  // Disables debugging for this window and returns to execute the rest of the script
  // (or aborts execution, if the user pressed stop). When this returns, the program
  // will exit the qt event loop, i.e. return to whatever processing was being done
  // before the debugger was stopped.
  assert(m_inSession);
  m_nextButton->setEnabled(false);
  m_stepButton->setEnabled(false);
  m_continueButton->setEnabled(false);
  m_stopButton->setEnabled(false);
  m_breakButton->setEnabled(true);
  m_breakpointButton->setEnabled(false);
  m_sourceDisplay->clearSelection();
  m_evalEdit->setReadOnly(true);
  m_inSession = false;
  qApp->exit_loop();
  m_fakeModal.disable();
}

void KJSDebugWin::updateFrameList()
{
  uint frameno;
  disconnect(m_frameList,SIGNAL(highlighted(int)),this,SLOT(showFrame(int)));
  m_frameList->clear();
  for (frameno = 0; frameno < m_frames.count(); frameno++)
    m_frameList->insertItem(m_frames.at(frameno)->toString(),frameno);
  if (m_frames.count() > 0) {
    m_frameList->setSelected(m_frameList->count()-1, true);
    setSourceLine(m_frames.last()->sourceFragment->sourceId,m_frames.last()->lineno);
  }
  connect(m_frameList,SIGNAL(highlighted(int)),this,SLOT(showFrame(int)));
}

bool KJSDebugWin::setBreakpoint(int sourceId, int lineno)
{
  if (haveBreakpoint(sourceId,lineno,lineno))
    return false;

  SourceBreakpoints *sbp = m_sourceBreakpoints;
  while(sbp && sbp->sourceId != sourceId)
    sbp = sbp->next;
  if (!sbp) {
    sbp = new SourceBreakpoints;
    sbp->sourceId = sourceId;
    sbp->breakpoints = 0;
    sbp->next = m_sourceBreakpoints;
    m_sourceBreakpoints = sbp;
  }

  Breakpoint *newbp = new Breakpoint;
  newbp->lineno = lineno;
  newbp->next = sbp->breakpoints;
  sbp->breakpoints = newbp;

  return true;
}

bool KJSDebugWin::deleteBreakpoint(int sourceId, int lineno)
{
  for (SourceBreakpoints *sbp = m_sourceBreakpoints; sbp; sbp = sbp->next) {
    if (sbp->sourceId == sourceId) {
      // found breakpoints for this sourceId
      Breakpoint *bp = sbp->breakpoints;
      if (bp && bp->lineno == lineno) {
	// was the first breakpoint
	Breakpoint *next = bp->next;
	delete bp;
	sbp->breakpoints = next;
	return true;
      }

      while (bp->next && bp->next->lineno != lineno)
	bp = bp->next;
      if (bp->next && bp->next->lineno == lineno) {
	// found at subsequent breakpoint
	Breakpoint *next = bp->next->next;
	delete bp->next;
	bp->next = next;
	return true;
      }
      return false;
    }
  }
  // no breakpoints at all for this sourceId
  return false;
}

void KJSDebugWin::clearAllBreakpoints(int sourceId)
{
  SourceBreakpoints *nextsbp = 0;
  for (SourceBreakpoints *sbp = m_sourceBreakpoints; sbp; sbp = nextsbp) {
    nextsbp = sbp->next;
    if (sourceId == -1 || sbp->sourceId == sourceId) {
      Breakpoint *nextbp;
      for (Breakpoint *bp = sbp->breakpoints; bp; bp = nextbp) {
	nextbp = bp->next;
	delete bp;
      }
      delete sbp;
    }
  }
}

int KJSDebugWin::breakpointLine(int sourceId, int line0, int line1)
{
  for (SourceBreakpoints *sbp = m_sourceBreakpoints; sbp; sbp = sbp->next) {
    if (sbp->sourceId == sourceId) {
      // found breakpoints for this sourceId
      for (Breakpoint *bp = sbp->breakpoints; bp; bp = bp->next) {
	if (bp->lineno >= 0 && bp->lineno >= line0 && bp->lineno <= line1)
	  return bp->lineno;
      }
      return -1;
    }
  }
  // no breakpoints at all for this sourceId
  return -1;
}

bool KJSDebugWin::haveBreakpoint(int sourceId, int line0, int line1)
{
  return (breakpointLine(sourceId,line0,line1) != -1);
}

bool KJSDebugWin::haveBreakpoint(SourceFile *sourceFile, int line0, int line1)
{
  for (SourceBreakpoints *sbp = m_sourceBreakpoints; sbp; sbp = sbp->next) {
    if (m_sourceFragments[sbp->sourceId]->sourceFile == sourceFile) {
      for (Breakpoint *bp = sbp->breakpoints; bp; bp = bp->next) {
	int absLineno = m_sourceFragments[sbp->sourceId]->baseLine + bp->lineno;
	if (absLineno >= 0 && absLineno >= line0 && absLineno <= line1)
	  return true;
      }
    }
  }
  return false;
}

#include "kjs_debugwin.moc"

#endif // KJS_DEBUGGER
