// -*- c-basic-offset: 2 -*-
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

#ifndef _KJS_BINDING_H_
#define _KJS_BINDING_H_

#include <kjs/interpreter.h>
#include <dom/dom_node.h>
#include <qvariant.h>
#include <kurl.h>

class KHTMLPart;

namespace KJS {

  /** Base class for all objects in this binding - get() and put() run
      tryGet() and tryPut() respectively, and catch exceptions if they
      occur. */
  class DOMObject : public ObjectImp {
  public:
    virtual Value get(ExecState *exec, const UString &propertyName) const;
    virtual Value tryGet(ExecState *exec, const UString &propertyName) const
      { return ObjectImp::get(exec, propertyName); }

    virtual void put(ExecState *exec, const UString &propertyName,
                     const Value &value, int attr = None);
    virtual void tryPut(ExecState *exec, const UString &propertyName,
                        const Value& value, int attr = None)
      { ObjectImp::put(exec,propertyName,value,attr); }

    virtual String toString(ExecState *exec) const;
  };

  /** Base class for all functions in this binding - get() and call() run
      tryGet() and tryCall() respectively, and catch exceptions if they
      occur. */
  class DOMFunction : public ObjectImp {
  public:
    DOMFunction() : ObjectImp() {}
    virtual Value get(ExecState *exec, const UString &propertyName) const;
    virtual Value tryGet(ExecState *exec, const UString &propertyName) const
      { return ObjectImp::get(exec, propertyName); }

    virtual bool implementsCall() const { return true; }
    virtual Value call(ExecState *exec, Object &thisObj, const List &args);

    virtual Value tryCall(ExecState *exec, Object &thisObj, const List&args)
      { return ObjectImp::call(exec, thisObj, args); }
    virtual Boolean toBoolean(ExecState *) const { return Boolean(true); }
    virtual String toString(ExecState *) const { return UString("[function]"); }
  };

  /**
   * We inherit from Interpreter, to save a pointer to the HTML part
   * that the interpreter runs for.
   */
  class ScriptInterpreter : public Interpreter
  {
  public:
    ScriptInterpreter( const Object &global, KHTMLPart* part )
      : Interpreter( global ), m_part( part )
      {}
    KHTMLPart* part() const { return m_part; }
  private:
    KHTMLPart* m_part;
  };

  /**
   * Convert an object to a Node. Returns a null Node if not possible.
   */
  DOM::Node toNode(const Value&);
  /**
   *  Get a String object, or Null() if s is null
   */
  Value getString(DOM::DOMString s);

  bool originCheck(const KURL &kurl1, const KURL &url2);

  /**
   * Convery a KJS value into a QVariant
   */
  QVariant ValueToVariant(ExecState* exec, const Value& val);

}; // namespace

#endif
