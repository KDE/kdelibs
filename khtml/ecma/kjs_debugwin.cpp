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
 *  Foundation, Inc., 51 Franklin Steet, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "kjs_debugwin.h"
#include "kjs_proxy.h"

#ifdef KJS_DEBUGGER

#include <assert.h>
#include <stdlib.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <q3textedit.h>
#include <q3listbox.h>
#include <q3multilineedit.h>
#include <qapplication.h>
#include <qsplitter.h>
#include <qcombobox.h>
#include <qbitmap.h>
#include <qwidget.h>
#include <qlabel.h>
#include <qdatastream.h>
#include <qpainter.h>
#include <qscrollbar.h>

#include <klocale.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <kglobal.h>
#include <kmessagebox.h>
#include <kguiitem.h>
#include <kpopupmenu.h>
#include <kmenubar.h>
#include <kaction.h>
#include <kactioncollection.h>
#include <kglobalsettings.h>
#include <kshortcut.h>
#include <kconfig.h>
#include <kconfigbase.h>
#include <kapplication.h>
#include <dcop/dcopclient.h>
#include <kstringhandler.h> 

#include "kjs_dom.h"
#include "kjs_binding.h"
#include "khtml_part.h"
#include "khtmlview.h"
#include "khtml_pagecache.h"
#include "khtml_settings.h"
#include "khtml_factory.h"
#include "misc/decoder.h"
#include <kjs/ustring.h>
#include <kjs/object.h>
#include <kjs/function.h>
#include <kjs/interpreter.h>

using namespace KJS;
using namespace khtml;

SourceDisplay::SourceDisplay(KJSDebugWin *debugWin, QWidget *parent, const char *name)
  : Q3ScrollView(parent,name), m_currentLine(-1), m_sourceFile(0), m_debugWin(debugWin),
    m_font(KGlobalSettings::fixedFont())
{
  verticalScrollBar()->setLineStep(QFontMetrics(m_font).height());
  viewport()->setBackgroundMode(Qt::NoBackground);
  m_breakpointIcon = KGlobal::iconLoader()->loadIcon("stop",KIcon::Small);
}

SourceDisplay::~SourceDisplay()
{
  if (m_sourceFile) {
    m_sourceFile->deref();
    m_sourceFile = 0L;
  }
}

void SourceDisplay::setSource(SourceFile *sourceFile)
{
  if ( sourceFile )
      sourceFile->ref();
  if (m_sourceFile)
      m_sourceFile->deref();
  m_sourceFile = sourceFile;
  if ( m_sourceFile )
      m_sourceFile->ref();

  if (!m_sourceFile || !m_debugWin->isVisible()) {
    return;
  }

  QString code = sourceFile->getCode();
  const QChar *chars = code.unicode();
  uint len = code.length();
  QChar newLine('\n');
  QChar cr('\r');
  QChar tab('\t');
  QString tabstr("        ");
  QString line;
  m_lines.clear();
  int width = 0;
  QFontMetrics metrics(m_font);

  for (uint pos = 0; pos < len; pos++) {
    QChar c = chars[pos];
    if (c == cr) {
      if (pos < len-1 && chars[pos+1] == newLine)
	continue;
      else
	c = newLine;
    }
    if (c == newLine) {
      m_lines.append(line);
      int lineWidth = metrics.width(line);
      if (lineWidth > width)
	width = lineWidth;
      line = "";
    }
    else if (c == tab) {
      line += tabstr;
    }
    else {
      line += c;
    }
  }
  if (line.length()) {
    m_lines.append(line);
    int lineWidth = metrics.width(line);
    if (lineWidth > width)
      width = lineWidth;
  }

  int linenoDisplayWidth = metrics.width("888888");
  resizeContents(linenoDisplayWidth+4+width,metrics.height()*m_lines.count());
  update();
  sourceFile->deref();
}

void SourceDisplay::setCurrentLine(int lineno, bool doCenter)
{
  m_currentLine = lineno;

  if (doCenter && m_currentLine >= 0) {
    QFontMetrics metrics(m_font);
    int height = metrics.height();
    center(0,height*m_currentLine+height/2);
  }

  updateContents();
}

void SourceDisplay::contentsMousePressEvent(QMouseEvent *e)
{
  Q3ScrollView::mouseDoubleClickEvent(e);
  QFontMetrics metrics(m_font);
  int lineno = e->y()/metrics.height();
  emit lineDoubleClicked(lineno+1); // line numbers start from 1
}

void SourceDisplay::showEvent(QShowEvent *)
{
    setSource(m_sourceFile);
}

void SourceDisplay::drawContents(QPainter *p, int clipx, int clipy, int clipw, int cliph)
{
  if (!m_sourceFile) {
    p->fillRect(clipx,clipy,clipw,cliph,palette().active().base());
    return;
  }

  QFontMetrics metrics(m_font);
  int height = metrics.height();

  int bottom = clipy + cliph;
  int right = clipx + clipw;

  int firstLine = clipy/height-1;
  if (firstLine < 0)
    firstLine = 0;
  int lastLine = bottom/height+2;
  if (lastLine > (int)m_lines.count())
    lastLine = m_lines.count();

  p->setFont(m_font);

  int linenoWidth = metrics.width("888888");

  for (int lineno = firstLine; lineno <= lastLine; lineno++) {
    QString linenoStr = QString().sprintf("%d",lineno+1);


    p->fillRect(0,height*lineno,linenoWidth,height,palette().active().mid());

    p->setPen(palette().active().text());
    p->drawText(0,height*lineno,linenoWidth,height,Qt::AlignRight,linenoStr);

    QColor bgColor;
    QColor textColor;

    if (lineno == m_currentLine) {
      bgColor = palette().active().highlight();
      textColor = palette().active().highlightedText();
    }
    else if (m_debugWin->haveBreakpoint(m_sourceFile,lineno+1,lineno+1)) {
      bgColor = palette().active().text();
      textColor = palette().active().base();
      p->drawPixmap(2,height*lineno+height/2-m_breakpointIcon.height()/2,m_breakpointIcon);
    }
    else {
      bgColor = palette().active().base();
      textColor = palette().active().text();
    }

    p->fillRect(linenoWidth,height*lineno,right-linenoWidth,height,bgColor);
    p->setPen(textColor);
    p->drawText(linenoWidth+4,height*lineno,contentsWidth()-linenoWidth-4,height,
		Qt::AlignLeft,m_lines[lineno]);
  }

  int remainingTop = height*(lastLine+1);
  p->fillRect(0,remainingTop,linenoWidth,bottom-remainingTop,palette().active().mid());

  p->fillRect(linenoWidth,remainingTop,
	      right-linenoWidth,bottom-remainingTop,palette().active().base());
}

//-------------------------------------------------------------------------

KJSDebugWin * KJSDebugWin::kjs_html_debugger = 0;

QString SourceFile::getCode()
{
  if (interpreter) {
    KHTMLPart *part = qobject_cast<KHTMLPart*>(static_cast<ScriptInterpreter*>(interpreter)->part());
    if (part && url == part->url().url() && KHTMLPageCache::self()->isValid(part->cacheId())) {
      Decoder *decoder = part->createDecoder();
      QByteArray data;
      QDataStream stream(&data,QIODevice::WriteOnly);
      KHTMLPageCache::self()->saveData(part->cacheId(),&stream);
      QString str;
      if (data.size() == 0)
	str = "";
      else
	str = decoder->decode(data.data(),data.size()) + decoder->flush();
      delete decoder;
      return str;
    }
  }

  return code;
}

//-------------------------------------------------------------------------

SourceFragment::SourceFragment(int sid, int bl, int el, SourceFile *sf)
{
  sourceId = sid;
  baseLine = bl;
  errorLine = el;
  sourceFile = sf;
  sourceFile->ref();
}

SourceFragment::~SourceFragment()
{
  sourceFile->deref();
  sourceFile = 0L;
}

//-------------------------------------------------------------------------

KJSErrorDialog::KJSErrorDialog(QWidget *parent, const QString& errorMessage, bool showDebug)
  : KDialogBase(parent,0,true,i18n("JavaScript Error"),
		showDebug ? KDialogBase::Ok|KDialogBase::User1 : KDialogBase::Ok,
		KDialogBase::Ok,false,KGuiItem("&Debug","gear"))
{
  QWidget *page = new QWidget(this);
  setMainWidget(page);

  QLabel *iconLabel = new QLabel("",page);
  iconLabel->setPixmap(KGlobal::iconLoader()->loadIcon("messagebox_critical",
						       KIcon::NoGroup,KIcon::SizeMedium,
						       KIcon::DefaultState,0,true));

  QWidget *contents = new QWidget(page);
  QLabel *label = new QLabel(errorMessage,contents);
  m_dontShowAgainCb = new QCheckBox(i18n("&Do not show this message again"),contents);

  QVBoxLayout *vl = new QVBoxLayout(contents,0,spacingHint());
  vl->addWidget(label);
  vl->addWidget(m_dontShowAgainCb);

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
EvalMultiLineEdit::EvalMultiLineEdit(QWidget *parent)
    : Q3MultiLineEdit(parent) {
}

void EvalMultiLineEdit::keyPressEvent(QKeyEvent * e)
{
    if (e->key() == Qt::Key_Return) {
        if (hasSelectedText()) {
            m_code = selectedText();
        } else {
            int para, index;
            getCursorPosition(&para, &index);
            m_code = text(para);
        }
        end();
    }
    Q3MultiLineEdit::keyPressEvent(e);
}
//-------------------------------------------------------------------------
KJSDebugWin::KJSDebugWin(QWidget *parent, const char *name)
  : KMainWindow(parent, name, Qt::WType_TopLevel), KInstance("kjs_debugger")
{
  m_breakpoints = 0;
  m_breakpointCount = 0;

  m_curSourceFile = 0;
  m_mode = Continue;
  m_nextSourceUrl = "";
  m_nextSourceBaseLine = 1;
  m_execs = 0;
  m_execsCount = 0;
  m_execsAlloc = 0;
  m_steppingDepth = 0;

  m_stopIcon = KGlobal::iconLoader()->loadIcon("stop",KIcon::Small);
  m_emptyIcon = QPixmap(m_stopIcon.width(),m_stopIcon.height());
  QBitmap emptyMask(m_stopIcon.width(),m_stopIcon.height(),true);
  m_emptyIcon.setMask(emptyMask);

  setCaption(i18n("JavaScript Debugger"));

  QWidget *mainWidget = new QWidget(this);
  setCentralWidget(mainWidget);

  QVBoxLayout *vl = new QVBoxLayout(mainWidget,5);

  // frame list & code
  QSplitter *hsplitter = new QSplitter(Qt::Vertical,mainWidget);
  QSplitter *vsplitter = new QSplitter(hsplitter);
  QFont font(KGlobalSettings::fixedFont());

  QWidget *contextContainer = new QWidget(vsplitter);

  QLabel *contextLabel = new QLabel(i18n("Call stack"),contextContainer);
  QWidget *contextListContainer = new QWidget(contextContainer);
  m_contextList = new Q3ListBox(contextListContainer);
  m_contextList->setMinimumSize(100,200);
  connect(m_contextList,SIGNAL(highlighted(int)),this,SLOT(slotShowFrame(int)));

  QHBoxLayout *clistLayout = new QHBoxLayout(contextListContainer);
  clistLayout->addWidget(m_contextList);
  clistLayout->addSpacing(KDialog::spacingHint());

  QVBoxLayout *contextLayout = new QVBoxLayout(contextContainer);
  contextLayout->addWidget(contextLabel);
  contextLayout->addSpacing(KDialog::spacingHint());
  contextLayout->addWidget(contextListContainer);

  // source selection & display
  QWidget *sourceSelDisplay = new QWidget(vsplitter);
  QVBoxLayout *ssdvl = new QVBoxLayout(sourceSelDisplay);

  m_sourceSel = new QComboBox(toolBar());
  connect(m_sourceSel,SIGNAL(activated(int)),this,SLOT(slotSourceSelected(int)));

  m_sourceDisplay = new SourceDisplay(this,sourceSelDisplay);
  ssdvl->addWidget(m_sourceDisplay);
  connect(m_sourceDisplay,SIGNAL(lineDoubleClicked(int)),SLOT(slotToggleBreakpoint(int)));

  Q3ValueList<int> vsplitSizes;
  vsplitSizes.insert(vsplitSizes.end(),120);
  vsplitSizes.insert(vsplitSizes.end(),480);
  vsplitter->setSizes(vsplitSizes);

  // evaluate

  QWidget *evalContainer = new QWidget(hsplitter);

  QLabel *evalLabel = new QLabel(i18n("JavaScript console"),evalContainer);
  m_evalEdit = new EvalMultiLineEdit(evalContainer);
  m_evalEdit->setWordWrap(Q3MultiLineEdit::NoWrap);
  m_evalEdit->setFont(font);
  connect(m_evalEdit,SIGNAL(returnPressed()),SLOT(slotEval()));
  m_evalDepth = 0;

  QVBoxLayout *evalLayout = new QVBoxLayout(evalContainer);
  evalLayout->addSpacing(KDialog::spacingHint());
  evalLayout->addWidget(evalLabel);
  evalLayout->addSpacing(KDialog::spacingHint());
  evalLayout->addWidget(m_evalEdit);

  Q3ValueList<int> hsplitSizes;
  hsplitSizes.insert(hsplitSizes.end(),400);
  hsplitSizes.insert(hsplitSizes.end(),200);
  hsplitter->setSizes(hsplitSizes);

  vl->addWidget(hsplitter);

  // actions
  KPopupMenu *debugMenu = new KPopupMenu(this);
  menuBar()->insertItem("&Debug",debugMenu);

  m_actionCollection = new KActionCollection(this);
  m_actionCollection->setInstance(this);
  m_nextAction       = new KAction(i18n("Next breakpoint","&Next"),"dbgnext",KShortcut(),this,SLOT(slotNext()),
				   m_actionCollection,"next");
  m_stepAction       = new KAction(i18n("&Step"),"dbgstep",KShortcut(),this,SLOT(slotStep()),
				   m_actionCollection,"step");
  m_continueAction   = new KAction(i18n("&Continue"),"dbgrun",KShortcut(),this,SLOT(slotContinue()),
				   m_actionCollection,"cont");
  m_stopAction       = new KAction(i18n("St&op"),"stop",KShortcut(),this,SLOT(slotStop()),
				   m_actionCollection,"stop");
  m_breakAction      = new KAction(i18n("&Break at Next Statement"),"dbgrunto",KShortcut(),this,SLOT(slotBreakNext()),
				   m_actionCollection,"breaknext");

  m_nextAction->setToolTip(i18n("Next breakpoint","Next"));
  m_stepAction->setToolTip(i18n("Step"));
  m_continueAction->setToolTip(i18n("Continue"));
  m_stopAction->setToolTip(i18n("Stop"));
  m_breakAction->setToolTip("Break at next Statement");

  m_nextAction->setEnabled(false);
  m_stepAction->setEnabled(false);
  m_continueAction->setEnabled(false);
  m_stopAction->setEnabled(false);
  m_breakAction->setEnabled(true);

  m_nextAction->plug(debugMenu);
  m_stepAction->plug(debugMenu);
  m_continueAction->plug(debugMenu);
//   m_stopAction->plug(debugMenu); ### disabled until DebuggerImp::stop() works reliably
  m_breakAction->plug(debugMenu);

  m_nextAction->plug(toolBar());
  m_stepAction->plug(toolBar());
  m_continueAction->plug(toolBar());
//   m_stopAction->plug(toolBar()); ###
  m_breakAction->plug(toolBar());

  toolBar()->insertWidget(1,300,m_sourceSel);
  toolBar()->setItemAutoSized(1);

  updateContextList();
  setMinimumSize(300,200);
  resize(600,450);

}

KJSDebugWin::~KJSDebugWin()
{
  free(m_breakpoints);
  free(m_execs);
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

void KJSDebugWin::slotNext()
{
  m_mode = Next;
  leaveSession();
}

void KJSDebugWin::slotStep()
{
  m_mode = Step;
  leaveSession();
}

void KJSDebugWin::slotContinue()
{
  m_mode = Continue;
  leaveSession();
}

void KJSDebugWin::slotStop()
{
  m_mode = Stop;
  while (!m_execStates.isEmpty())
    leaveSession();
}

void KJSDebugWin::slotBreakNext()
{
  m_mode = Step;
}

void KJSDebugWin::slotToggleBreakpoint(int lineno)
{
  if (m_sourceSel->currentItem() < 0)
    return;

  SourceFile *sourceFile = m_sourceSelFiles.at(m_sourceSel->currentItem());

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
	highestBaseLine = sourceFragment->baseLine;
    }
  }

  if (sourceId < 0)
    return;

  // Update the source code display with the appropriate icon
  int fragmentLineno = lineno-highestBaseLine+1;
  if (!setBreakpoint(sourceId,fragmentLineno)) // was already set
    deleteBreakpoint(sourceId,fragmentLineno);

  m_sourceDisplay->updateContents();
}

void KJSDebugWin::slotShowFrame(int frameno)
{
  if (frameno < 0 || frameno >= m_execsCount)
    return;

  Context ctx = m_execs[frameno]->context();
  setSourceLine(ctx.sourceId(),ctx.curStmtFirstLine());
}

void KJSDebugWin::slotSourceSelected(int sourceSelIndex)
{
  // A source file has been selected from the drop-down list - display the file
  if (sourceSelIndex < 0 || sourceSelIndex >= (int)m_sourceSel->count())
    return;
  SourceFile *sourceFile = m_sourceSelFiles.at(sourceSelIndex);
  displaySourceFile(sourceFile,true);

  // If the currently selected context is in the current source file, then hilight
  // the line it's on.
  if (m_contextList->currentItem() >= 0) {
    Context ctx = m_execs[m_contextList->currentItem()]->context();
    if (m_sourceFragments[ctx.sourceId()]->sourceFile == m_sourceSelFiles.at(sourceSelIndex))
      setSourceLine(ctx.sourceId(),ctx.curStmtFirstLine());
  }
}

void KJSDebugWin::slotEval()
{
  // Work out which execution state to use. If we're currently in a debugging session,
  // use the current context - otherwise, use the global execution state from the interpreter
  // corresponding to the currently displayed source file.
  ExecState *exec;
  Object thisobj;
  if (m_execStates.isEmpty()) {
    if (m_sourceSel->currentItem() < 0)
      return;
    SourceFile *sourceFile = m_sourceSelFiles.at(m_sourceSel->currentItem());
    if (!sourceFile->interpreter)
      return;
    exec = sourceFile->interpreter->globalExec();
    thisobj = exec->interpreter()->globalObject();
  }
  else {
    exec = m_execStates.top();
    thisobj = exec->context().thisValue();
  }

  // Evaluate the js code from m_evalEdit
  UString code(m_evalEdit->code());
  QString msg;

  KJSCPUGuard guard;
  guard.start();

  Interpreter *interp = exec->interpreter();

  Object obj = Object::dynamicCast(interp->globalObject().get(exec, "eval"));
  List args;
  args.append(String(code));

  m_evalDepth++;
  Value retval = obj.call(exec, thisobj, args);
  m_evalDepth--;
  guard.stop();

  // Print the return value or exception message to the console
  if (exec->hadException()) {
    Value exc = exec->exception();
    exec->clearException();
    msg = "Exception: " + exc.toString(interp->globalExec()).qstring();
  }
  else {
    msg = retval.toString(interp->globalExec()).qstring();
  }

  m_evalEdit->insert(msg+"\n");
  updateContextList();
}

void KJSDebugWin::closeEvent(QCloseEvent *e)
{
  while (!m_execStates.isEmpty()) // ### not sure if this will work
    leaveSession();
  return QWidget::closeEvent(e);
}

bool KJSDebugWin::eventFilter(QObject *o, QEvent *e)
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
    if (o == this)
      return QWidget::eventFilter(o,e);
    else
      return true;
    break;
  default:
    return QWidget::eventFilter(o,e);
  }
}

void KJSDebugWin::disableOtherWindows()
{
  QWidgetList widgets = QApplication::allWidgets();
  QListIterator<QWidget*> it(widgets);
  while (it.hasNext()) {
    QWidget* widget = it.next();
    widget->installEventFilter(this);
  }
}

void KJSDebugWin::enableOtherWindows()
{
  QWidgetList widgets = QApplication::allWidgets();
  QListIterator<QWidget*> it(widgets);
  while (it.hasNext()) {
    QWidget* widget = it.next();
    widget->removeEventFilter(this);
  }
}

bool KJSDebugWin::sourceParsed(KJS::ExecState *exec, int sourceId,
                               const KJS::UString &source, int errorLine)
{
  // Work out which source file this fragment is in
  SourceFile *sourceFile = 0;
  if (!m_nextSourceUrl.isEmpty())
    sourceFile = getSourceFile(exec->interpreter(),m_nextSourceUrl);

  int index;
  if (!sourceFile) {
    index = m_sourceSel->count();
    if (!m_nextSourceUrl.isEmpty()) {

      QString code = source.qstring();
      KParts::ReadOnlyPart *part = static_cast<ScriptInterpreter*>(exec->interpreter())->part();
      if (m_nextSourceUrl == part->url().url()) {
	// Only store the code here if it's not from the part's html page... in that
	// case we can get it from KHTMLPageCache
	code = QString::null;
      }

      sourceFile = new SourceFile(m_nextSourceUrl,code,exec->interpreter());
      setSourceFile(exec->interpreter(),m_nextSourceUrl,sourceFile);
      m_sourceSelFiles.append(sourceFile);
      m_sourceSel->insertItem(m_nextSourceUrl);
    }
    else {
      // Sourced passed from somewhere else (possibly an eval call)... we don't know the url,
      // but we still know the interpreter
      sourceFile = new SourceFile("(unknown)",source.qstring(),exec->interpreter());
      m_sourceSelFiles.append(sourceFile);
      m_sourceSel->insertItem("???");
    }
  }
  else {
    for (index = 0; index < m_sourceSel->count(); index++) {
      if (m_sourceSelFiles.at(index) == sourceFile)
	break;
    }
    assert(index < m_sourceSel->count());
  }

  SourceFragment *sf = new SourceFragment(sourceId,m_nextSourceBaseLine,errorLine,sourceFile);
  m_sourceFragments[sourceId] = sf;

  if (m_sourceSel->currentItem() < 0)
    m_sourceSel->setCurrentItem(index);

  if (m_sourceSel->currentItem() == index) {
    displaySourceFile(sourceFile,true);
  }

  m_nextSourceBaseLine = 1;
  m_nextSourceUrl = "";

  return (m_mode != Stop);
}

bool KJSDebugWin::sourceUnused(KJS::ExecState *exec, int sourceId)
{
  // Verify that there aren't any contexts on the stack using the given sourceId
  // This should never be the case because this function is only called when
  // the interpreter has deleted all Node objects for the source.
  for (int e = 0; e < m_execsCount; e++)
    assert(m_execs[e]->context().sourceId() != sourceId);

  // Now remove the fragment (and the SourceFile, if it was the last fragment in that file)
  SourceFragment *fragment = m_sourceFragments[sourceId];
  if (fragment) {
    m_sourceFragments.erase(sourceId);

    SourceFile *sourceFile = fragment->sourceFile;
    if (sourceFile->hasOneRef()) {
      for (int i = 0; i < m_sourceSel->count(); i++) {
	if (m_sourceSelFiles.at(i) == sourceFile) {
	  m_sourceSel->removeItem(i);
	  m_sourceSelFiles.remove(i);
	  break;
	}
      }
      removeSourceFile(exec->interpreter(),sourceFile->url);
    }
    delete fragment;
  }

  return (m_mode != Stop);
}

bool KJSDebugWin::exception(ExecState *exec, const Value &value, bool inTryCatch)
{
  assert(value.isValid());

  // Ignore exceptions that will be caught by the script
  if (inTryCatch)
    return true;

  KParts::ReadOnlyPart *part = static_cast<ScriptInterpreter*>(exec->interpreter())->part();
  KHTMLPart *khtmlpart = qobject_cast<KHTMLPart*>(part);
  if (khtmlpart && !khtmlpart->settings()->isJavaScriptErrorReportingEnabled())
    return true;

  QWidget *dlgParent = (m_evalDepth == 0) ? (QWidget*)part->widget() : (QWidget*)this;

  QString exceptionMsg = value.toString(exec).qstring();

  // Syntax errors are a special case. For these we want to display the url & lineno,
  // which isn't included in the exception messeage. So we work it out from the values
  // passed to sourceParsed()
  Object valueObj = Object::dynamicCast(value);
  Object syntaxError = exec->interpreter()->builtinSyntaxError();
  if (valueObj.isValid() && valueObj.get(exec,"constructor").imp() == syntaxError.imp()) {
    Value sidValue = valueObj.get(exec,"sid");
    if (sidValue.isA(NumberType)) { // sid is not set for Function() constructor
      int sourceId = (int)sidValue.toNumber(exec);
      assert(m_sourceFragments[sourceId]);
      exceptionMsg = i18n("Parse error at %1 line %2")
		     .arg(m_sourceFragments[sourceId]->sourceFile->url)
		     .arg(m_sourceFragments[sourceId]->baseLine+m_sourceFragments[sourceId]->errorLine-1);
    }
  }

  bool dontShowAgain = false;
  if (m_execsCount == 0) {
    // An exception occurred and we're not currently executing any code... this can
    // happen in some cases e.g. a parse error, or native code accessing funcitons like
    // Object::put()
    QString msg = i18n("An error occurred while attempting to run a script on this page.\n\n%1")
		  .arg(exceptionMsg);
    KJSErrorDialog dlg(dlgParent,msg,false);
    dlg.exec();
    dontShowAgain = dlg.dontShowAgain();
  }
  else {
    Context ctx = m_execs[m_execsCount-1]->context();
    SourceFragment *sourceFragment = m_sourceFragments[ctx.sourceId()];
    QString msg = i18n("An error occurred while attempting to run a script on this page.\n\n%1 line %2:\n%3")
		  .arg(KStringHandler::rsqueeze( sourceFragment->sourceFile->url,80),
		  QString::number( sourceFragment->baseLine+ctx.curStmtFirstLine()-1),
		  exceptionMsg);

    KJSErrorDialog dlg(dlgParent,msg,true);
    dlg.exec();
    dontShowAgain = dlg.dontShowAgain();

    if (dlg.debugSelected()) {
      m_mode = Next;
      m_steppingDepth = m_execsCount-1;
      enterSession(exec);
    }
  }

  if (dontShowAgain) {
    KConfig *config = kapp->config();
    KConfigGroupSaver saver(config,QLatin1String("Java/JavaScript Settings"));
    config->writeEntry("ReportJavaScriptErrors",QVariant(false,0));
    config->sync();
    QByteArray data;
    KApplication::dcopClient()->send( "konqueror*", "KonquerorIface", "reparseConfiguration()", data );
  }

  return (m_mode != Stop);
}

bool KJSDebugWin::atStatement(KJS::ExecState *exec)
{
  assert(m_execsCount > 0);
  assert(m_execs[m_execsCount-1] == exec);
  checkBreak(exec);
  return (m_mode != Stop);
}

bool KJSDebugWin::enterContext(ExecState *exec)
{
  if (m_execsCount >= m_execsAlloc) {
    m_execsAlloc += 10;
    m_execs = (ExecState**)realloc(m_execs,m_execsAlloc*sizeof(ExecState*));
  }
  m_execs[m_execsCount++] = exec;

  if (m_mode == Step)
    m_steppingDepth = m_execsCount-1;

  checkBreak(exec);
  return (m_mode != Stop);
}

bool KJSDebugWin::exitContext(ExecState *exec, const Completion &/*completion*/)
{
  assert(m_execsCount > 0);
  assert(m_execs[m_execsCount-1] == exec);

  checkBreak(exec);

  m_execsCount--;
  if (m_steppingDepth > m_execsCount-1)
    m_steppingDepth = m_execsCount-1;
  if (m_execsCount == 0)
    updateContextList();

  return (m_mode != Stop);
}

void KJSDebugWin::displaySourceFile(SourceFile *sourceFile, bool forceRefresh)
{
  if (m_curSourceFile == sourceFile && !forceRefresh)
    return;
  sourceFile->ref();
  m_sourceDisplay->setSource(sourceFile);
  if (m_curSourceFile)
     m_curSourceFile->deref();
  m_curSourceFile = sourceFile;
}

void KJSDebugWin::setSourceLine(int sourceId, int lineno)
{
  SourceFragment *source = m_sourceFragments[sourceId];
  if (!source)
    return;

  SourceFile *sourceFile = source->sourceFile;
  if (m_curSourceFile != source->sourceFile) {
      for (int i = 0; i < m_sourceSel->count(); i++)
	if (m_sourceSelFiles.at(i) == sourceFile)
	  m_sourceSel->setCurrentItem(i);
      displaySourceFile(sourceFile,false);
  }
  m_sourceDisplay->setCurrentLine(source->baseLine+lineno-2);
}

void KJSDebugWin::setNextSourceInfo(QString url, int baseLine)
{
  m_nextSourceUrl = url;
  m_nextSourceBaseLine = baseLine;
}

void KJSDebugWin::sourceChanged(Interpreter *interpreter, QString url)
{
  SourceFile *sourceFile = getSourceFile(interpreter,url);
  if (sourceFile && m_curSourceFile == sourceFile)
    displaySourceFile(sourceFile,true);
}

void KJSDebugWin::clearInterpreter(Interpreter *interpreter)
{
  QMap<int,SourceFragment*>::Iterator it;

  for (it = m_sourceFragments.begin(); it != m_sourceFragments.end(); ++it)
    if (it.data() && it.data()->sourceFile->interpreter == interpreter)
      it.data()->sourceFile->interpreter = 0;
}

SourceFile *KJSDebugWin::getSourceFile(Interpreter *interpreter, QString url)
{
  QString key = QString("%1|%2").arg((long)interpreter).arg(url);
  return m_sourceFiles[key];
}

void KJSDebugWin::setSourceFile(Interpreter *interpreter, QString url, SourceFile *sourceFile)
{
  QString key = QString("%1|%2").arg((long)interpreter).arg(url);
  m_sourceFiles[key] = sourceFile;
}

void KJSDebugWin::removeSourceFile(Interpreter *interpreter, QString url)
{
  QString key = QString("%1|%2").arg((long)interpreter).arg(url);
  m_sourceFiles.remove(key);
}

void KJSDebugWin::checkBreak(ExecState *exec)
{
  if (m_breakpointCount > 0) {
    Context ctx = m_execs[m_execsCount-1]->context();
    if (haveBreakpoint(ctx.sourceId(),ctx.curStmtFirstLine(),ctx.curStmtLastLine())) {
      m_mode = Next;
      m_steppingDepth = m_execsCount-1;
    }
  }

  if ((m_mode == Step || m_mode == Next) && m_steppingDepth == m_execsCount-1)
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
    disableOtherWindows();
    m_nextAction->setEnabled(true);
    m_stepAction->setEnabled(true);
    m_continueAction->setEnabled(true);
    m_stopAction->setEnabled(true);
    m_breakAction->setEnabled(false);
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
    m_nextAction->setEnabled(false);
    m_stepAction->setEnabled(false);
    m_continueAction->setEnabled(false);
    m_stopAction->setEnabled(false);
    m_breakAction->setEnabled(true);
    m_sourceDisplay->setCurrentLine(-1);
    enableOtherWindows();
  }

  qApp->exit_loop();
}

void KJSDebugWin::updateContextList()
{
  disconnect(m_contextList,SIGNAL(highlighted(int)),this,SLOT(slotShowFrame(int)));

  m_contextList->clear();
  for (int i = 0; i < m_execsCount; i++)
    m_contextList->insertItem(contextStr(m_execs[i]->context()));

  if (m_execsCount > 0) {
    m_contextList->setSelected(m_execsCount-1, true);
    Context ctx = m_execs[m_execsCount-1]->context();
    setSourceLine(ctx.sourceId(),ctx.curStmtFirstLine());
  }

  connect(m_contextList,SIGNAL(highlighted(int)),this,SLOT(slotShowFrame(int)));
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
    if (m_sourceFragments.contains(sourceId) &&
        m_sourceFragments[sourceId]->sourceFile == sourceFile) {
      int absLineno = m_sourceFragments[sourceId]->baseLine+lineno-1;
      if (absLineno >= line0 && absLineno <= line1)
	return true;
    }
  }

  return false;
}

#include "kjs_debugwin.moc"

#endif // KJS_DEBUGGER
