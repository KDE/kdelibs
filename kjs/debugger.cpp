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

#include "debugger.h"
#include "kjs.h"
#include "internal.h"
#include "ustring.h"

using namespace KJS;

KJSO ExecutionContext::resolveVar(const UString &/*varName*/) const
{
  // ###
  return KJSO();
}

KJSO ExecutionContext::executeCall(KJScript *script, KJSO &func, const KJSO &thisV, const List *args) const
{
  KJScriptImp *oldCurrent = KJScriptImp::current(); // ### hack
  KJScriptImp::setCurrent(script->rep);
  Context *oldContext = script->rep->context(); // ### hack
  script->rep->setContext(rep);

  KJSO ret = func.executeCall(thisV,args);

  script->rep->setContext(oldContext);
  KJScriptImp::setCurrent(oldCurrent);
  return ret;
}

KJSO ExecutionContext::thisValue() const
{
  return rep->thisValue();
}

ExecutionContext::ExecutionContext(Context *c)
{
  rep = c;
}

Debugger::Debugger()
{
  rep = new DebuggerImp(this);
}

Debugger::~Debugger()
{
  delete rep;
}

void Debugger::attach(KJScript *e)
{
  e->rep->setDebugger(this);
}

void Debugger::detach(KJScript *script)
{
  // ### take care of script == 0 situation
  if (script->rep->debugger() == this)
    script->rep->setDebugger(0);
}

bool Debugger::sourceParsed(KJScript */*script*/, int /*sourceId*/,
			    const UString &/*source*/, int /*errorLine*/)
{
  return true;
}

bool Debugger::sourceUnused(KJScript */*script*/, int /*sourceId*/)
{
  return true;
}

bool Debugger::error(KJScript */*script*/, int /*sourceId*/, int /*lineno*/,
		     int /*errorType*/, const UString &/*errorMessage*/)
{
  return true;
}

bool Debugger::atLine(KJScript */*script*/, int /*sourceId*/, int /*lineno*/,
		      const ExecutionContext */*execContext*/)
{
  return true;
}

bool Debugger::callEvent(KJScript */*script*/, int /*sourceId*/, int /*lineno*/,
			 const ExecutionContext */*execContext*/,
			 FunctionImp */*function*/, const List */*args*/)
{
  return true;
}

bool Debugger::returnEvent(KJScript */*script*/, int /*sourceId*/, int /*lineno*/,
			   const ExecutionContext */*execContext*/,
			   FunctionImp */*function*/)
{
  return true;
}

/*
UString Debugger::varInfo(const UString &ident)
{
  // ###
  //  if (!eng)
  //    return UString();

  int dot = ident.find('.');
  if (dot < 0)
      dot = ident.size();
  UString sub = ident.substr(0, dot);
  KJSO obj;
  // resolve base
  if (sub == "this") {
      obj = KJScriptImp::current()->context()->thisValue();
  } else {
      const List *chain = KJScriptImp::current()->context()->pScopeChain();
      ListIterator scope = chain->begin();
      while (scope != chain->end()) {
	  if (scope->hasProperty(ident)) {
	      obj = scope->get(ident);
	      break;
	  }
	  scope++;
      }
      if (scope == chain->end())
	return UString();
  }
  // look up each part of a.b.c.
  while (dot < ident.size()) {
    int olddot = dot;
    dot = ident.find('.', olddot+1);
    if (dot < 0)
      dot = ident.size();
    sub = ident.substr(olddot+1, dot-olddot-1);
    obj = obj.get(sub);
    if (!obj.isDefined())
      break;
  }

  return sub + "=" + objInfo(obj) + ":" + UString(obj.imp()->typeInfo()->name);
}

// called by varInfo() and recursively by itself on each properties
UString Debugger::objInfo(const KJSO &obj) const
{
  // ###
  const char *cnames[] = { "Undefined", "Array", "String", "Boolean",
			   "Number", "Object", "Date", "RegExp",
			   "Error", "Function" };
  PropList *plist = obj.imp()->propList(0, 0, false);
  if (!plist)
    return obj.toString().value();
  else {
    UString result = "{";
    while (1) {
      result += plist->name + "=";
      KJSO p = obj.get(plist->name);
      result += objInfo(p) + ":";
      Object obj = Object::dynamicCast(p);
      if (obj.isNull())
	result += p.imp()->typeInfo()->name;
      else
	result += cnames[int(obj.getClass())];
      plist = plist->next;
      if (!plist)
	break;
      result += ",";
    }
    result += "}";
    return result;
  }
}

bool Debugger::setVar(const UString &ident, const KJSO &value)
{
  // ###
  //  if (!eng)
  //    return false;
  const List *chain = KJScriptImp::current()->context()->pScopeChain();
  ListIterator scope = chain->begin();
  while (scope != chain->end()) {
    if (scope->hasProperty(ident)) {
      if (!scope->canPut(ident))
	return false;
      scope->put(ident, value);
      return true;
    }
    scope++;
  }
  // didn't find variable
  return false;
}
*/
