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
#include <qptrdict.h>
#include <kurl.h>
#include <kjs/lookup.h>

class KHTMLPart;

namespace KJS {

  /**
   * Base class for all objects in this binding - get() and put() run
   * tryGet() and tryPut() respectively, and catch exceptions if they
   * occur.
   */
  class DOMObject : public ObjectImp {
  public:
    DOMObject(const Object &proto) : ObjectImp(proto) {}
    DOMObject() : ObjectImp() {}
    virtual Value get(ExecState *exec, const UString &propertyName) const;
    virtual Value tryGet(ExecState *exec, const UString &propertyName) const
      { return ObjectImp::get(exec, propertyName); }

    virtual void put(ExecState *exec, const UString &propertyName,
                     const Value &value, int attr = None);
    virtual void tryPut(ExecState *exec, const UString &propertyName,
                        const Value& value, int attr = None)
      { ObjectImp::put(exec,propertyName,value,attr); }

    virtual UString toString(ExecState *exec) const;
  };

  /**
   * Base class for all functions in this binding - get() and call() run
   * tryGet() and tryCall() respectively, and catch exceptions if they
   * occur.
   */
  class DOMFunction : public ObjectImp {
  public:
    DOMFunction() : ObjectImp( /* proto? */ ) {}
    virtual Value get(ExecState *exec, const UString &propertyName) const;
    virtual Value tryGet(ExecState *exec, const UString &propertyName) const
      { return ObjectImp::get(exec, propertyName); }

    virtual bool implementsCall() const { return true; }
    virtual Value call(ExecState *exec, Object &thisObj, const List &args);

    virtual Value tryCall(ExecState *exec, Object &thisObj, const List&args)
      { return ObjectImp::call(exec, thisObj, args); }
    virtual bool toBoolean(ExecState *) const { return true; }
    virtual Value toPrimitive(ExecState *exec, Type) const { return String(toString(exec)); }
    virtual UString toString(ExecState *) const { return UString("[function]"); }
  };

  /**
   * We inherit from Interpreter, to save a pointer to the HTML part
   * that the interpreter runs for.
   * The interpreter also stores the DOM object - >KJS::DOMObject cache.
   */
  class ScriptInterpreter : public Interpreter
  {
  public:
    ScriptInterpreter( const Object &global, KHTMLPart* part );
    virtual ~ScriptInterpreter();

    DOMObject* getDOMObject( void* objectHandle ) const {
      return m_domObjects[objectHandle];
    }
    void putDOMObject( void* objectHandle, DOMObject* obj ) {
      m_domObjects.insert( objectHandle, obj );
    }
    bool deleteDOMObject( void* objectHandle ) {
      return m_domObjects.remove( objectHandle );
    }
    /**
     * Static method. Makes all interpreters forget about the object
     */
    static void forgetDOMObject( void* objectHandle );

    /**
     * Mark objects in the DOMObject cache.
     */
    virtual void mark();
    KHTMLPart* part() const { return m_part; }

    virtual int rtti() { return 1; }

    /**
     * Set the event that is triggering the execution of a script, if any
     */
    void setCurrentEvent( DOM::Event *evt ) { m_evt = evt; }
    void setInlineCode( bool inlineCode ) { m_inlineCode = inlineCode; }
    /**
     * "Smart" window.open policy
     */
    bool isWindowOpenAllowed() const;

  private:
    KHTMLPart* m_part;
    QPtrDict<DOMObject> m_domObjects;
    DOM::Event *m_evt;
    bool m_inlineCode;
  };
  /**
   * Retrieve from cache, or create, a KJS object around a DOM object
   */
  template<class DOMObj, class KJSDOMObj>
  inline Value cacheDOMObject(ExecState *exec, DOMObj domObj)
  {
    DOMObject *ret;
    if (domObj.isNull())
      return Null();
    ScriptInterpreter* interp = static_cast<ScriptInterpreter *>(exec->interpreter());
    if ((ret = interp->getDOMObject(domObj.handle())))
      return Value(ret);
    else {
      ret = new KJSDOMObj(exec, domObj);
      interp->putDOMObject(domObj.handle(),ret);
      return Value(ret);
    }
  }

  /**
   * Convert an object to a Node. Returns a null Node if not possible.
   */
  DOM::Node toNode(const Value&);
  /**
   *  Get a String object, or Null() if s is null
   */
  Value getString(DOM::DOMString s);

  /**
   * Convery a KJS value into a QVariant
   */
  QVariant ValueToVariant(ExecState* exec, const Value& val);

  /**
   * We need a modified version of lookupGet because
   * we call tryGet instead of get, in DOMObjects.
   */
  template <class FuncImp, class ThisImp, class ParentImp>
  inline Value DOMObjectLookupGet(ExecState *exec, const UString &propertyName,
                                  const HashTable* table, const ThisImp* thisObj)
  {
    const HashEntry* entry = Lookup::findEntry(table, propertyName);

    if (!entry) // not found, forward to parent
      return thisObj->ParentImp::tryGet(exec, propertyName);

    if (entry->attr & Function)
      return lookupOrCreateFunction<FuncImp>(exec, propertyName, thisObj, entry->value, entry->params, entry->attr);
    return thisObj->getValueProperty(exec, entry->value);
  }

  /**
   * Simplified version of DOMObjectLookupGet in case there are no
   * functions, only "values".
   */
  template <class ThisImp, class ParentImp>
  inline Value DOMObjectLookupGetValue(ExecState *exec, const UString &propertyName,
                                       const HashTable* table, const ThisImp* thisObj)
  {
    const HashEntry* entry = Lookup::findEntry(table, propertyName);

    if (!entry) // not found, forward to parent
      return thisObj->ParentImp::tryGet(exec, propertyName);

    if (entry->attr & Function)
      fprintf(stderr, "Function bit set! Shouldn't happen in lookupValue!\n" );
    return thisObj->getValueProperty(exec, entry->value);
  }

  /**
   * We need a modified version of lookupPut because
   * we call tryPut instead of put, in DOMObjects.
   */
  template <class ThisImp, class ParentImp>
  inline void DOMObjectLookupPut(ExecState *exec, const UString &propertyName,
                                 const Value& value, int attr,
                                 const HashTable* table, ThisImp* thisObj)
  {
    const HashEntry* entry = Lookup::findEntry(table, propertyName);

    if (!entry) // not found: forward to parent
      thisObj->ParentImp::tryPut(exec, propertyName, value, attr);
    else if (entry->attr & Function) // function: put as override property
      thisObj->ObjectImp::put(exec, propertyName, value, attr);
    else if (entry->attr & ReadOnly) // readonly! Can't put!
#ifdef KJS_VERBOSE
      fprintf(stderr,"Attempt to change value of readonly property '%s'\n",propertyName.ascii());
#else
    ; // do nothing
#endif
    else
      thisObj->putValue(exec, entry->value, value, attr);
  }

  // Modified version of IMPLEMENT_PROTOFUNC, to use DOMFunction and tryCall
#define IMPLEMENT_PROTOFUNC_DOM(ClassFunc) \
  namespace KJS { \
  class ClassFunc : public DOMFunction { \
  public: \
    ClassFunc(ExecState *exec, int i, int len) \
       : DOMFunction( /*proto? */ ), id(i) { \
       Value protect(this); \
       put(exec,"length",Number(len),DontDelete|ReadOnly|DontEnum); \
    } \
    /** You need to implement that one */ \
    virtual Value tryCall(ExecState *exec, Object &thisObj, const List &args); \
  private: \
    int id; \
  }; \
  };

}; // namespace

#endif
