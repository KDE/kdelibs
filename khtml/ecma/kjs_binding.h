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

#include <stdlib.h> // for abort

#define KJS_CHECK_THIS( ClassName, theObj ) \
	if (!theObj || !theObj->inherits(&ClassName::info)) { \
		KJS::UString errMsg = "Attempt at calling a function that expects a "; \
		errMsg += ClassName::info.className; \
		errMsg += " on a "; \
		errMsg += thisObj->className(); \
		KJS::ObjectImp *err = KJS::Error::create(exec, KJS::TypeError, errMsg.ascii()); \
		exec->setException(err); \
		return err; \
	}

namespace KParts {
  class ReadOnlyPart;
  class LiveConnectExtension;
}

namespace khtml {
  class ChildFrame;
}

namespace KJS {

  /**
   * Base class for all objects in this binding. Doesn't manage exceptions any more
   */
  class DOMObject : public ObjectImp {
  protected:
    DOMObject() : ObjectImp() {}
    DOMObject(ObjectImp *proto) : ObjectImp(proto) {}
  public:
    virtual UString toString(ExecState *exec) const;
  };

  /**
   * Base class for all functions in this binding
   */
  class DOMFunction : public ObjectImp {
  protected:
    DOMFunction() : ObjectImp() {}
  public:
//     DOMFunction(ExecState* exec) : ObjectImp(
//       static_cast<FunctionPrototypeImp*>(exec->interpreter()->builtinFunctionPrototype())
//       ) {}

    virtual bool implementsCall() const { return true; }
    virtual bool toBoolean(ExecState *) const { return true; }
//From JSC,examine:
//	virtual ValueImp *toPrimitive(ExecState *exec, Type) const { return String(toString(exec)); }
//  virtual UString toString(ExecState *) const { return UString("[function]"); }
#warning "toString -- [[native code]] thing?"
  };

  /**
   * We inherit from Interpreter, to save a pointer to the HTML part
   * that the interpreter runs for.
   * The interpreter also stores the DOM object - >KJS::DOMObject cache.
   */
  class KHTML_EXPORT ScriptInterpreter : public Interpreter
  {
  public:
    ScriptInterpreter( ObjectImp *global, khtml::ChildFrame* frame );
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
   A little helper for setting stuff up given an entry
  */
  template<class FuncImp, class ThisImp>
  inline void getSlotFromEntry(const HashEntry* entry, ThisImp* thisObj, PropertySlot& slot)
  {
    if (entry->attr & Function)
      slot.setStaticEntry(thisObj, entry, staticFunctionGetter<FuncImp>);
    else 
      slot.setStaticEntry(thisObj, entry, staticValueGetter<ThisImp>);
  }


  /**
    Like getStaticPropertySlot but doesn't check the parent. Handy when there
    are both functions and values
   */
  template <class FuncImp, class ThisImp>
  inline bool getStaticOwnPropertySlot(const HashTable* table,
                                    ThisImp* thisObj, const Identifier& propertyName, PropertySlot& slot)
  {
    const HashEntry* entry = Lookup::findEntry(table, propertyName);

    if (!entry) // not found, forward to parent
      return false;

    if (entry->attr & Function)
      slot.setStaticEntry(thisObj, entry, staticFunctionGetter<FuncImp>);
    else 
      slot.setStaticEntry(thisObj, entry, staticValueGetter<ThisImp>);

    return true;
  }

  /**
    Handler for local table-looked up things.
  */
  template<class ThisImp>
  inline bool getStaticOwnValueSlot(const HashTable* table,
        ThisImp* thisObj, const Identifier& propertyName, PropertySlot& slot)
  {
    const HashEntry* entry = Lookup::findEntry(table, propertyName);
    if (!entry)
      return false;

    assert(!(entry->attr & Function));
    slot.setStaticEntry(thisObj, entry, staticValueGetter<ThisImp>);
    return true;
  }

  /* Helper for the below*/
  template<class JSTypeImp>
  ValueImp *indexGetterAdapter(ExecState* exec, const Identifier& , const PropertySlot& slot)
  {
    JSTypeImp *thisObj = static_cast<JSTypeImp*>(slot.slotBase());
    return thisObj->indexGetter(exec, slot.index());
  }

  /**
   Handler for index properties. Will call "length" method on the listObj
   to determine whether it's in range, and arrange to have indexGetter called
  */
  template<class ThisImp, class BaseObj>
  inline bool getIndexSlot(ThisImp* thisObj, const BaseObj& listObj,
      const Identifier& propertyName, PropertySlot& slot)
  {
    bool ok;
    unsigned u = propertyName.toUInt32(&ok);
    if (ok && u < listObj.length()) {
      slot.setCustomIndex(thisObj, u, indexGetterAdapter<ThisImp>);
      return true;
    }
    return false;
  }

  /**
   Version that takes an external bound
  */
  template<class ThisImp>
  inline bool getIndexSlot(ThisImp* thisObj, unsigned lengthLimit,
      const Identifier& propertyName, PropertySlot& slot)
  {
    bool ok;
    unsigned u = propertyName.toUInt32(&ok);
    if (ok && u < lengthLimit) {
      slot.setCustomIndex(thisObj, u, indexGetterAdapter<ThisImp>);
      return true;
    }
    return false;
  }

  template<class ThisImp>
  inline bool getIndexSlot(ThisImp* thisObj, int lengthLimit,
      const Identifier& propertyName, PropertySlot& slot)
  {
    return getIndexSlot(thisObj, (unsigned)lengthLimit, propertyName, slot);
  }



  /**
   Version w/o the bounds check
  */
  template<class ThisImp>
  inline bool getIndexSlot(ThisImp* thisObj, const Identifier& propertyName, PropertySlot& slot)
  {
    bool ok;
    unsigned u = propertyName.toUInt32(&ok);
    if (ok) {
      slot.setCustomIndex(thisObj, u, indexGetterAdapter<ThisImp>);
      return true;
    }
    return false;
  }


  /**
   * Retrieve from cache, or create, a KJS object around a DOM object
   */
  template<class DOMObj, class KJSDOMObj>
  inline ValueImp* cacheDOMObject(ExecState *exec, DOMObj* domObj)
  {
    DOMObject *ret;
    if (!domObj)
      return Null();
    ScriptInterpreter* interp = static_cast<ScriptInterpreter *>(exec->interpreter());
    if ((ret = interp->getDOMObject(domObj)))
      return ret;
    else {
      ret = new KJSDOMObj(exec, domObj);
      interp->putDOMObject(domObj,ret);
      return ret;
    }
  }

  /**
   * Convert an object to a Node. Returns 0 if not possible.
   */
  DOM::NodeImpl* toNode(ValueImp*);
  /**
   *  Get a String object, or Null() if s is null
   */
  ValueImp* getStringOrNull(DOM::DOMString s);

  /**
   * Convert a KJS value into a QVariant
   */
  QVariant ValueToVariant(ExecState* exec, ValueImp* val);

  // Convert a DOM implementation exception code into a JavaScript exception in the execution state.
  void setDOMException(ExecState *exec, int DOMExceptionCode);

  // Helper class to call setDOMException on exit without adding lots of separate calls to that function.
  class DOMExceptionTranslator {
  public:
    explicit DOMExceptionTranslator(ExecState *exec) : m_exec(exec), m_code(0) { }
    ~DOMExceptionTranslator() { setDOMException(m_exec, m_code); }
    operator int &() { return m_code; }
    operator int *() { return &m_code; }

    bool triggered() {
      return m_code;
    }
  private:
    ExecState *m_exec;
    int m_code;
  };

  ValueImp* getLiveConnectValue(KParts::LiveConnectExtension *lc, const QString & name, const int type, const QString & value, int id);



} // namespace

#endif
