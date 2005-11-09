// -*- c-basic-offset: 2 -*-
/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 1999-2001 Harri Porten (porten@kde.org)
 *  Copyright (C) 2003 Apple Computer, Inc.
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef _KJS_BINDING_H_
#define _KJS_BINDING_H_

#include <kjs/interpreter.h>
#include <kjs/function_object.h> /// for FunctionPrototypeImp

#include <dom/dom_node.h>
#include <qvariant.h>
#include <q3ptrdict.h>
#include <kurl.h>
#include <kjs/lookup.h>

namespace KParts {
  class ReadOnlyPart;
  class LiveConnectExtension;
}

namespace khtml {
  class ChildFrame;
}

namespace KJS {

  /**
   * Base class for all objects in this binding - get() and put() run
   * tryGet() and tryPut() respectively, and catch exceptions if they
   * occur.
   */
  class DOMObject : public ObjectImp {
  public:
    DOMObject() : ObjectImp() {}
    virtual UString toString(ExecState *exec) const;
  };

  /**
   * Base class for all functions in this binding.
   */
  class DOMFunction : public ObjectImp {
  public:
    DOMFunction() : ObjectImp() {}
    virtual bool implementsCall() const { return true; }
    virtual bool toBoolean(ExecState *) const { return true; }
    virtual ValueImp *toPrimitive(ExecState *exec, Type) const { return String(toString(exec)); }
    virtual UString toString(ExecState *) const { return UString("[function]"); }
  };

  /**
   * We inherit from Interpreter, to save a pointer to the HTML part
   * that the interpreter runs for.
   * The interpreter also stores the DOM object - >KJS::DOMObject cache.
   */
  class KDE_EXPORT ScriptInterpreter : public Interpreter
  {
  public:
    ScriptInterpreter(ObjectImp *global, khtml::ChildFrame* frame );
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
    void clear() {
      m_domObjects.clear();
    }
    /**
     * Static method. Makes all interpreters forget about the object
     */
    static void forgetDOMObject( void* objectHandle );

    /**
     * Mark objects in the DOMObject cache.
     */
    virtual void mark();
    KParts::ReadOnlyPart* part() const;

    virtual int rtti() { return 1; }

    /**
     * Set the event that is triggering the execution of a script, if any
     */
    void setCurrentEvent( DOM::Event *evt ) { m_evt = evt; }
    void setInlineCode( bool inlineCode ) { m_inlineCode = inlineCode; }
    void setProcessingTimerCallback( bool timerCallback ) { m_timerCallback = timerCallback; }
    /**
     * "Smart" window.open policy
     */
    bool isWindowOpenAllowed() const;

  private:
    khtml::ChildFrame* m_frame;
    Q3PtrDict<DOMObject> m_domObjects;
    DOM::Event *m_evt;
    bool m_inlineCode;
    bool m_timerCallback;
  };
  /**
   * Retrieve from cache, or create, a KJS object around a DOM object
   */
  template<class DOMObj, class KJSDOMObj>
  inline ValueImp *cacheDOMObject(ExecState *exec, DOMObj domObj)
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
  DOM::Node toNode(ValueImp *);
  /**
   *  Get a String object, or Null() if s is null
   */
  ValueImp *getString(DOM::DOMString s);

  /**
   * Convery a KJS value into a QVariant
   */
  QVariant ValueToVariant(ExecState* exec, ValueImp *val);

  /**
   * Simplified version of DOMObjectLookupGet in case there are no
   * functions, only "values".
   */
  template <class ThisImp, class ParentImp>
  inline ValueImp *DOMObjectLookupGetValue(ExecState *exec, const Identifier &propertyName,
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
  inline void DOMObjectLookupPut(ExecState *exec, const Identifier &propertyName,
                                 ValueImp *value, int attr,
                                 const HashTable* table, ThisImp* thisObj)
  {
    const HashEntry* entry = Lookup::findEntry(table, propertyName);

    if (!entry) // not found: forward to parent
      thisObj->ParentImp::tryPut(exec, propertyName, value, attr);
    else if (entry->attr & Function) // function: put as override property
      thisObj->ObjectImp::put(exec, propertyName, value, attr);
    else if (entry->attr & ReadOnly) // readonly! Can't put!
#ifdef KJS_VERBOSE
      fprintf(stderr,"WARNING: Attempt to change value of readonly property '%s'\n",propertyName.ascii());
#else
    ; // do nothing
#endif
    else
      thisObj->putValueProperty(exec, entry->value, value, attr);
  }

  // Modified version of IMPLEMENT_PROTOFUNC, to use DOMFunction and tryCall
#define IMPLEMENT_PROTOFUNC_DOM(ClassFunc) \
  namespace KJS { \
  class ClassFunc : public DOMFunction { \
  public: \
    ClassFunc(ExecState *exec, int i, int len) \
       : DOMFunction( exec ), id(i) { \
       /*Value protect(this);*/ \
       put(exec,lengthPropertyName,Number(len),DontDelete|ReadOnly|DontEnum); \
    } \
    /** You need to implement that one */ \
    virtual Value tryCall(ExecState *exec, Object &thisObj, const List &args); \
  private: \
    int id; \
  }; \
  }

  ValueImp *getLiveConnectValue(KParts::LiveConnectExtension *lc, const QString & name, const int type, const QString & value, int id);

} // namespace

#endif
