/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 1999-2001 Harri Porten (porten@kde.org)
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

using namespace KJS;

extern "C" {
  KJSProxy *kjs_html_init(KHTMLPart *khtmlpart);
}

#ifdef KJS_DEBUGGER
static KJSDebugWin *kjs_html_debugger = 0;
#endif

// initialize HTML module
KJSProxy *kjs_html_init(KHTMLPart *khtmlpart)
{
  KJScript *script = kjs_create(khtmlpart);

  // proxy class operating via callback functions
  KJSProxy *proxy = new KJSProxy(script, &kjs_create, &kjs_eval, &kjs_execFuncCall,
                                 &kjs_clear, &kjs_special, &kjs_destroy);
  proxy->khtmlpart = khtmlpart;

#ifdef KJS_DEBUGGER
  // ### share and destroy
  if (!kjs_html_debugger)
      kjs_html_debugger = new KJSDebugWin();
#endif

  return proxy;
}

namespace KJS {
    extern ScriptMap *script_map;
};

// init the interpreter
  KJScript* kjs_create(KHTMLPart *khtmlpart)
  {
    if (!script_map)
      script_map = new ScriptMap;
 
    ScriptMap::Iterator it = script_map->find(khtmlpart);
    if (it != script_map->end())
      return it.data();

    KJScript *script = new KJScript();
    script_map->insert((KHTMLPart*)khtmlpart, script);
#ifndef NDEBUG
    script->enableDebug();
#endif
    KJS::Imp *global = script->globalObject();

    global->setPrototype(new Window(khtmlpart));

    return script;
  }

  // evaluate code. Returns the JS return value or an invalid QVariant
  // if there was none, an error occured or the type couldn't be converted.
  QVariant kjs_eval(KJScript *script, const QChar *c, unsigned int len,
		    const DOM::Node &n, KHTMLPart *khtmlpart)
  {
    script->init(); // set a valid current interpreter

#ifdef KJS_DEBUGGER
    kjs_html_debugger->attach(script);
    kjs_html_debugger->setCode(QString(c, len));
    kjs_html_debugger->setMode(KJS::Debugger::Step);
#endif

    KJS::KJSO thisNode = n.isNull() ? KJSO() : getDOMNode(n);

    KJS::Global::current().setExtra(khtmlpart);
    bool ret = script->evaluate(thisNode, c, len);
    if (script->recursion() == 0)
      KJS::Global::current().setExtra(0L);

#ifdef KJS_DEBUGGER
    kjs_html_debugger->setCode(QString::null);
#endif

    // let's try to convert the return value
    QVariant res;
    if (ret && script->returnValue()) {
      KJS::KJSO retVal(script->returnValue());
      switch (retVal.type()) {
      case BooleanType:
	  res = QVariant(retVal.toBoolean().value(), 0);
	  break;
      case NumberType:
	  res = QVariant(retVal.toNumber().value());
	  break;
      case StringType:
	  res = QVariant(retVal.toString().value().qstring());
	  break;
      default:
	// everything else will be 'invalid'
	  break;
      }
    }
    return res;
  }
  // clear resources allocated by the interpreter
  void kjs_clear(KJScript *script, KHTMLPart *part)
  {
    ScriptMap::Iterator it = script_map->find(part);
    if (it != script_map->end())
      script_map->remove(it);

    script->clear();
    //    delete script;
  }
  // for later extensions.
  const char *kjs_special(KJScript *, const char *)
  {
    // return something like a version number for now
    return "1";
  }
  void kjs_destroy(KJScript *script)
  {
    delete script;
  }

  QVariant kjs_execFuncCall( KJS::KJSO &thisVal, KJS::KJSO &functionObj, KJS::List &args, bool inEvaluate, KHTMLPart *khtmlpart)
  {
    if (functionObj.implementsCall()) {
      if (!inEvaluate)
	KJS::Global::current().setExtra(khtmlpart);
      functionObj.executeCall(thisVal,&args);
      if (!inEvaluate)
	KJS::Global::current().setExtra(0L);
    }
    return QVariant(); // ### return proper value
  }
