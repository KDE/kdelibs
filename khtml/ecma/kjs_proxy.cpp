/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 1999-2001 Harri Porten (porten@kde.org)
 *  Copyright (C) 2001 Peter Kelly (pmk@post.com)
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "kjs_proxy.h"

#include <kjs/kjs.h>
#include <kjs/object.h>
#include <kjs/function.h>

#include <khtml_part.h>
#include <html_element.h>
#include <html_head.h>
#include <html_inline.h>
#include <html_image.h>

#include "kjs_binding.h"
#include "kjs_dom.h"
#include "kjs_html.h"
#include "kjs_window.h"
#include "kjs_navigator.h"
#include "kjs_debugwin.h"
#include "kjs_events.h"

using namespace KJS;

extern "C" {
  KJSProxy *kjs_html_init(KHTMLPart *khtmlpart);
}

QVariant KJS::KJSOToVariant(KJSO obj) {
  QVariant res;
  switch (obj.type()) {
  case BooleanType:
    res = QVariant(obj.toBoolean().value(), 0);
    break;
  case NumberType:
    res = QVariant(obj.toNumber().value());
    break;
  case StringType:
    res = QVariant(obj.toString().value().qstring());
    break;
  default:
    // everything else will be 'invalid'
    break;
  }
  return res;
}


class KJSProxyImpl : public KJSProxy {
public:
  KJSProxyImpl(KHTMLPart *part);
  virtual ~KJSProxyImpl();
  virtual QVariant evaluate(QString filename, int baseLine, const QChar *c,
		    unsigned int len, const DOM::Node &n);
  virtual void clear();
  virtual DOM::EventListener *createHTMLEventHandler(QString sourceUrl, QString code);
  virtual KJScript *jScript();

  virtual void setDebugEnabled(bool enabled);
  virtual bool paused() const;
  virtual void setSourceFile(QString url, QString code);
  virtual void appendSourceFile(QString url, QString code);

  void initScript();

private:
  KJScript* m_script;
  bool m_debugEnabled;
};


KJSProxyImpl::KJSProxyImpl(KHTMLPart *part)
{
  m_script = 0;
  m_part = part;
  m_debugEnabled = false;
}

KJSProxyImpl::~KJSProxyImpl()
{
  delete m_script;
}

QVariant KJSProxyImpl::evaluate(QString filename, int baseLine,
			    const QChar *c, unsigned int len,
			    const DOM::Node &n) {
  // evaluate code. Returns the JS return value or an invalid QVariant
  // if there was none, an error occured or the type couldn't be converted.

  initScript();
  m_script->init(); // set a valid current interpreter

#ifdef KJS_DEBUGGER
  // ###    KJSDebugWin::instance()->attach(m_script);
  if (KJSDebugWin::instance())
    KJSDebugWin::instance()->setNextSourceInfo(filename,baseLine);
  //    KJSDebugWin::instance()->setMode(KJS::Debugger::Step);
#endif

  KJS::KJSO thisNode = n.isNull() ? KJSO( Window::retrieve( m_part ) ) : getDOMNode(n);

  KJS::Global::current().setExtra(m_part);
  bool success = m_script->evaluate(thisNode, c, len);
//  if (m_script->recursion() == 0)
//    KJS::Global::current().setExtra(0L);

#ifdef KJS_DEBUGGER
    //    KJSDebugWin::instance()->setCode(QString::null);
#endif

  // let's try to convert the return value
  if (success && m_script->returnValue())
    return KJSOToVariant(m_script->returnValue());
  else
    return QVariant();
}

void KJSProxyImpl::clear() {
  // clear resources allocated by the interpreter
  if (m_script) {
#ifdef KJS_DEBUGGER
    KJSDebugWin *debugWin = KJSDebugWin::instance();
    if (debugWin && debugWin->currentScript() == m_script) {
        debugWin->setMode(KJSDebugWin::Stop);
//        debugWin->leaveSession();
    }
#endif
    Window *win = Window::retrieveWindow(m_part);
    if (win)
        win->clear();

    m_script->clear(); // may delete window
    delete m_script;
    m_script = 0L;
  }
}

DOM::EventListener *KJSProxyImpl::createHTMLEventHandler(QString sourceUrl, QString code)
{
#ifdef KJS_DEBUGGER
  if (KJSDebugWin::instance())
    KJSDebugWin::instance()->setNextSourceInfo(sourceUrl,m_handlerLineno);
#endif

  initScript();
  m_script->init(); // set a valid current interpreter
  KJS::Global::current().setExtra(m_part);
  KJS::Constructor constr(KJS::Global::current().get("Function").imp());
  KJS::List args;
  args.append(KJS::String("event"));
  args.append(KJS::String(code));
  KJS::KJSO handlerFunc = constr.construct(args);
//  if (m_script->recursion() == 0)
//    KJS::Global::current().setExtra(0L);

  return KJS::Window::retrieveWindow(m_part)->getJSEventListener(handlerFunc,true);
}

KJScript *KJSProxyImpl::jScript()
{
  if (!m_script)
    initScript();
  return m_script;
}

void KJSProxyImpl::setDebugEnabled(bool enabled)
{
#ifdef KJS_DEBUGGER
  m_debugEnabled = enabled;
  if (m_script)
      m_script->setDebuggingEnabled(enabled);
  // NOTE: this is consistent across all KJSProxyImpl instances, as we only
  // ever have 1 debug window
  if (!enabled && KJSDebugWin::instance()) {
    KJSDebugWin::destroyInstance();
  }
  else if (enabled && !KJSDebugWin::instance()) {
    KJSDebugWin::createInstance();
    KJSDebugWin::instance()->attach(m_script);
  }
#endif
}

bool KJSProxyImpl::paused() const
{
#ifdef KJS_DEBUGGER
  if (KJSDebugWin::instance())
    return KJSDebugWin::instance()->inSession();
#endif
  return false;
}

void KJSProxyImpl::setSourceFile(QString url, QString code)
{
#ifdef KJS_DEBUGGER
  if (KJSDebugWin::instance())
    KJSDebugWin::instance()->setSourceFile(url,code);
#endif
}

void KJSProxyImpl::appendSourceFile(QString url, QString code)
{
#ifdef KJS_DEBUGGER
  if (KJSDebugWin::instance())
    KJSDebugWin::instance()->appendSourceFile(url,code);
#endif
}

void KJSProxyImpl::initScript()
{
  if (m_script)
    return;

  m_script = new KJScript();
#ifdef KJS_DEBUGGER
  m_script->setDebuggingEnabled(m_debugEnabled);
#endif
  m_script->enableDebug();
  KJS::Imp *global = m_script->globalObject();
  global->setPrototype(new Window(m_part));
}

// initialize HTML module
KJSProxy *kjs_html_init(KHTMLPart *khtmlpart)
{
  return new KJSProxyImpl(khtmlpart);
}


