// -*- c-basic-offset: 2 -*-
/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 1999-2001 Harri Porten (porten@kde.org)
 *  Copyright (C) 2001,2003 Peter Kelly (pmk@post.com)
 *  Copyright (C) 2001-2003 David Faure (faure@kde.org)
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

#include "kjs_proxy.h"

#include "kjs_window.h"
#include "kjs_events.h"
#include "kjs_debugwin.h"
#include <khtml_part.h>
#include <kprotocolmanager.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>
#include <assert.h>
#include <kjs/function.h>

using namespace KJS;

extern "C" {
  KJSProxy *kjs_html_init(KHTMLPart *khtmlpart);
}

namespace KJS {

class KJSProxyImpl : public KJSProxy {
public:
  KJSProxyImpl(KHTMLPart *part);
  virtual ~KJSProxyImpl();
  virtual QVariant evaluate(QString filename, int baseLine, const QString &, const DOM::Node &n,
			    Completion *completion = 0);
  virtual void clear();
  virtual DOM::EventListener *createHTMLEventHandler(QString sourceUrl, QString name, QString code);
  virtual void finishedWithEvent(const DOM::Event &event);
  virtual KJS::Interpreter *interpreter();

  virtual void setDebugEnabled(bool enabled);
  virtual void showDebugWindow(bool show=true);
  virtual bool paused() const;
  virtual void dataReceived();

  void initScript();
  void applyUserAgent();

private:
  KJS::ScriptInterpreter* m_script;
  bool m_debugEnabled;
#ifndef NDEBUG
  static int s_count;
#endif
};

} // namespace KJS

#ifndef NDEBUG
int KJSProxyImpl::s_count = 0;
#endif

KJSProxyImpl::KJSProxyImpl(KHTMLPart *part)
{
  m_script = 0;
  m_part = part;
  m_debugEnabled = false;
#ifndef NDEBUG
  s_count++;
#endif
}

KJSProxyImpl::~KJSProxyImpl()
{
  if ( m_script ) {
    //kdDebug() << "KJSProxyImpl::~KJSProxyImpl clearing global object " << m_script->globalObject().imp() << endl;
    // This allows to delete the global-object properties, like all the protos
    static_cast<ObjectImp*>(m_script->globalObject().imp())->deleteAllProperties( m_script->globalExec() );
    //kdDebug() << "KJSProxyImpl::~KJSProxyImpl garbage collecting" << endl;
    while (KJS::Interpreter::collect())
        ;
    //kdDebug() << "KJSProxyImpl::~KJSProxyImpl deleting interpreter " << m_script << endl;
    delete m_script;
    //kdDebug() << "KJSProxyImpl::~KJSProxyImpl garbage collecting again" << endl;
    // Garbage collect - as many times as necessary
    // (we could delete an object which was holding another object, so
    // the deref() will happen too late for deleting the impl of the 2nd object).
    while (KJS::Interpreter::collect())
        ;
  }

#ifndef NDEBUG
  s_count--;
  // If it was the last interpreter, we should have nothing left
#ifdef KJS_DEBUG_MEM
  if ( s_count == 0 )
    Interpreter::finalCheck();
#endif
#endif
}

QVariant KJSProxyImpl::evaluate(QString filename, int baseLine,
                                const QString&str, const DOM::Node &n, Completion *completion) {
  // evaluate code. Returns the JS return value or an invalid QVariant
  // if there was none, an error occurred or the type couldn't be converted.

  initScript();
  // inlineCode is true for <a href="javascript:doSomething()">
  // and false for <script>doSomething()</script>. Check if it has the
  // expected value in all cases.
  // See smart window.open policy for where this is used.
  bool inlineCode = filename.isNull();
  //kdDebug(6070) << "KJSProxyImpl::evaluate inlineCode=" << inlineCode << endl;

#ifdef KJS_DEBUGGER
  if (inlineCode)
    filename = "(unknown file)";
  if (KJSDebugWin::debugWindow()) {
    KJSDebugWin::debugWindow()->attach(m_script);
    KJSDebugWin::debugWindow()->setNextSourceInfo(filename,baseLine);
  //    KJSDebugWin::debugWindow()->setMode(KJSDebugWin::Step);
  }
#else
  Q_UNUSED(baseLine);
#endif

  m_script->setInlineCode(inlineCode);
  Window* window = Window::retrieveWindow( m_part );
  KJS::Value thisNode = n.isNull() ? Window::retrieve( m_part ) : getDOMNode(m_script->globalExec(),n);

  UString code( str );

  KJSCPUGuard guard;
  guard.start();
  Completion comp = m_script->evaluate(code, thisNode);
  guard.stop();

  bool success = ( comp.complType() == Normal ) || ( comp.complType() == ReturnValue );

  if (completion)
    *completion = comp;

#ifdef KJS_DEBUGGER
    //    KJSDebugWin::debugWindow()->setCode(QString::null);
#endif

  window->afterScriptExecution();

  // let's try to convert the return value
  if (success && !comp.value().isNull())
    return ValueToVariant( m_script->globalExec(), comp.value());
  else
  {
    if ( comp.complType() == Throw )
    {
        UString msg = comp.value().toString(m_script->globalExec());
        kdDebug(6070) << "WARNING: Script threw exception: " << msg.qstring() << endl;
    }
    return QVariant();
  }
}

// Implementation of the debug() function
class TestFunctionImp : public ObjectImp {
public:
  TestFunctionImp() : ObjectImp() {}
  virtual bool implementsCall() const { return true; }
  virtual Value call(ExecState *exec, Object &thisObj, const List &args);
};

Value TestFunctionImp::call(ExecState *exec, Object &/*thisObj*/, const List &args)
{
  fprintf(stderr,"--> %s\n",args[0].toString(exec).ascii());
  return Undefined();
}

void KJSProxyImpl::clear() {
  // clear resources allocated by the interpreter, and make it ready to be used by another page
  // We have to keep it, so that the Window object for the part remains the same.
  // (we used to delete and re-create it, previously)
  if (m_script) {
#ifdef KJS_DEBUGGER
    // ###
    KJSDebugWin *debugWin = KJSDebugWin::debugWindow();
    if (debugWin) {
      if (debugWin->getExecState() &&
          debugWin->getExecState()->interpreter() == m_script)
        debugWin->slotStop();
      debugWin->clearInterpreter(m_script);
    }
#endif
    m_script->clear();

    Window *win = static_cast<Window *>(m_script->globalObject().imp());
    if (win) {
      win->clear( m_script->globalExec() );
      // re-add "debug", clear() removed it
      m_script->globalObject().put(m_script->globalExec(),
                                   "debug", Value(new TestFunctionImp()), Internal);
      if ( !win->part().isNull() )
        applyUserAgent();
    }

    // Really delete everything that can be, so that the DOM nodes get deref'ed
    //kdDebug() << k_funcinfo << "all done -> collecting" << endl;
    while (KJS::Interpreter::collect())
        ;
  }
}

DOM::EventListener *KJSProxyImpl::createHTMLEventHandler(QString sourceUrl, QString name, QString code)
{
  initScript();

#ifdef KJS_DEBUGGER
  if (KJSDebugWin::debugWindow()) {
    KJSDebugWin::debugWindow()->attach(m_script);
    KJSDebugWin::debugWindow()->setNextSourceInfo(sourceUrl,m_handlerLineno);
  }
#else
  Q_UNUSED(sourceUrl);
#endif

  //KJS::Constructor constr(KJS::Global::current().get("Function").imp());
  KJS::Object constr = m_script->builtinFunction();
  KJS::List args;
  args.append(KJS::String("event"));
  args.append(KJS::String(code));

  Object handlerFunc = constr.construct(m_script->globalExec(), args); // ### is globalExec ok ?
  if (m_script->globalExec()->hadException())
    m_script->globalExec()->clearException();

  if (!handlerFunc.inherits(&DeclaredFunctionImp::info))
    return 0; // Error creating function

  DeclaredFunctionImp *declFunc = static_cast<DeclaredFunctionImp*>(handlerFunc.imp());
  declFunc->setName(Identifier(name));
  return KJS::Window::retrieveWindow(m_part)->getJSEventListener(handlerFunc,true);
}

void KJSProxyImpl::finishedWithEvent(const DOM::Event &event)
{
  // This is called when the DOM implementation has finished with a particular event. This
  // is the case in sitations where an event has been created just for temporary usage,
  // e.g. an image load or mouse move. Once the event has been dispatched, it is forgotten
  // by the DOM implementation and so does not need to be cached still by the interpreter
  ScriptInterpreter::forgetDOMObject(event.handle());
}

KJS::Interpreter *KJSProxyImpl::interpreter()
{
  if (!m_script)
    initScript();
  return m_script;
}

void KJSProxyImpl::setDebugEnabled(bool enabled)
{
#ifdef KJS_DEBUGGER
  m_debugEnabled = enabled;
  //if (m_script)
  //    m_script->setDebuggingEnabled(enabled);
  // NOTE: this is consistent across all KJSProxyImpl instances, as we only
  // ever have 1 debug window
  if (!enabled && KJSDebugWin::debugWindow()) {
    KJSDebugWin::destroyInstance();
  }
  else if (enabled && !KJSDebugWin::debugWindow()) {
    KJSDebugWin::createInstance();
    initScript();
    KJSDebugWin::debugWindow()->attach(m_script);
  }
#else
  Q_UNUSED(enabled);
#endif
}

void KJSProxyImpl::showDebugWindow(bool /*show*/)
{
#ifdef KJS_DEBUGGER
  if (KJSDebugWin::debugWindow())
    KJSDebugWin::debugWindow()->show();
#else
  //Q_UNUSED(show);
#endif
}

bool KJSProxyImpl::paused() const
{
#ifdef KJS_DEBUGGER
  if (KJSDebugWin::debugWindow())
    return KJSDebugWin::debugWindow()->inSession();
#endif
  return false;
}

void KJSProxyImpl::dataReceived()
{
#ifdef KJS_DEBUGGER
  if (KJSDebugWin::debugWindow())
    KJSDebugWin::debugWindow()->sourceChanged(m_script,m_part->url().url());
#endif
}

void KJSProxyImpl::initScript()
{
  if (m_script)
    return;

  // Build the global object - which is a Window instance
  Object globalObject( new Window(m_part) );

  // Create a KJS interpreter for this part
  m_script = new KJS::ScriptInterpreter(globalObject, m_part);
  static_cast<ObjectImp*>(globalObject.imp())->setPrototype(m_script->builtinObjectPrototype());

#ifdef KJS_DEBUGGER
  //m_script->setDebuggingEnabled(m_debugEnabled);
#endif
  //m_script->enableDebug();
  globalObject.put(m_script->globalExec(),
		   "debug", Value(new TestFunctionImp()), Internal);
  applyUserAgent();
}

void KJSProxyImpl::applyUserAgent()
{
  assert( m_script );
  QString host = m_part->url().isLocalFile() ? "localhost" : m_part->url().host();
  QString userAgent = KProtocolManager::userAgentForHost(host);
  if (userAgent.find(QString::fromLatin1("Microsoft")) >= 0 ||
      userAgent.find(QString::fromLatin1("MSIE")) >= 0)
  {
    m_script->setCompatMode(Interpreter::IECompat);
#ifdef KJS_VERBOSE
    kdDebug() << "Setting IE compat mode" << endl;
#endif
  }
  else
    // If we find "Mozilla" but not "(compatible, ...)" we are a real Netscape
    if (userAgent.find(QString::fromLatin1("Mozilla")) >= 0 &&
        userAgent.find(QString::fromLatin1("compatible")) == -1)
    {
      m_script->setCompatMode(Interpreter::NetscapeCompat);
#ifdef KJS_VERBOSE
      kdDebug() << "Setting NS compat mode" << endl;
#endif
    }
}

// initialize HTML module
KJSProxy *kjs_html_init(KHTMLPart *khtmlpart)
{
  return new KJSProxyImpl(khtmlpart);
}

void KJSCPUGuard::start(unsigned int ms, unsigned int i_ms)
{
  oldAlarmHandler = signal(SIGVTALRM, alarmHandler);
  itimerval tv = {
      { i_ms / 1000, (i_ms % 1000) * 1000 },
      { ms / 1000, (ms % 1000) * 1000 }
  };
  setitimer(ITIMER_VIRTUAL, &tv, &oldtv);
}

void KJSCPUGuard::stop()
{
  setitimer(ITIMER_VIRTUAL, &oldtv, 0L);
  signal(SIGVTALRM, oldAlarmHandler);
}

bool KJSCPUGuard::confirmTerminate() {
  kdDebug(6070) << "alarmhandler" << endl;
  return KMessageBox::warningYesNo(0L, i18n("A script on this page is causing KHTML to freeze. If it continues to run, other applications may become less responsive.\nDo you want to abort the script?"), i18n("JavaScript"), i18n("Abort"), KStdGuiItem::cont(), "kjscupguard_alarmhandler") == KMessageBox::Yes;
}

void KJSCPUGuard::alarmHandler(int) {
    ExecState::requestTerminate();
    ExecState::confirmTerminate = KJSCPUGuard::confirmTerminate;
}
