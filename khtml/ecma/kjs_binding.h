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
#include <qptrdict.h>
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
    DOMObject(const Object &proto) : ObjectImp(proto) {}
    virtual Value get(ExecState *exec, const Identifier &propertyName) const;
    virtual Value tryGet(ExecState *exec, const Identifier &propertyName) const
      { return ObjectImp::get(exec, propertyName); }
    virtual bool implementsHasInstance() const { return true; }
    virtual Boolean hasInstance(ExecState *exec, const Value &value);
    virtual void put(ExecState *exec, const Identifier &propertyName,
                     const Value &value, int attr = None);
    virtual void tryPut(ExecState *exec, const Identifier &propertyName,
                        const Value& value, int attr = None);

    virtual UString toString(ExecState *exec) const;
  };

  /**
   * Base class for all functions in this binding - get() and call() run
   * tryGet() and tryCall() respectively, and catch exceptions if they
   * occur.
   */
  class DOMFunction : public InternalFunctionImp {
  public:
    DOMFunction(ExecState* exec) : InternalFunctionImp(
      static_cast<FunctionPrototypeImp*>(exec->interpreter()->builtinFunctionPrototype().imp())
      ) {}
    virtual Value get(ExecState *exec, const Identifier &propertyName) const;
    virtual Value tryGet(ExecState *exec, const Identifier &propertyName) const
      { return ObjectImp::get(exec, propertyName); }

    virtual bool implementsCall() const { return true; }
    virtual Value call(ExecState *exec, Object &thisObj, const List &args);

    virtual Value tryCall(ExecState *exec, Object &thisObj, const List&args)
      { return ObjectImp::call(exec, thisObj, args); }
    virtual bool toBoolean(ExecState *) const { return true; }
  };

  /**
   * We inherit from Interpreter, to save a pointer to the HTML part
   * that the interpreter runs for.
   * The interpreter also stores the DOM object - >KJS::DOMObject cache.
   */
  class KDE_EXPORT ScriptInterpreter : public Interpreter
  {
  public:
    ScriptInterpreter( const Object &global, khtml::ChildFrame* frame );
    virtual ~ScriptInterpreter();

    DOMObject* getDOMObject( void* objectHandle ) const {
      return m_domObjects[objectHandle];
    }
    void putDOMObject( void* objectHandle, DOMObject* obj ) {
      m_domObjects.insert( objectHandle, obj );
    }
    void customizedDOMObject( DOMObject* obj ) {
      m_customizedDomObjects.replace( obj, this );
    }
    bool deleteDOMObject( void* objectHandle ) {
      DOMObject* obj = m_domObjects.take( objectHandle );
      if (obj) {
        m_customizedDomObjects.remove( obj );
        return true;
      }
      else
        return false;
    }
    void clear() {
      m_customizedDomObjects.clear();
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
    QPtrDict<DOMObject> m_domObjects;
    QPtrDict<void> m_customizedDomObjects; //Objects which had custom properties set,
                                           //and should not be GC'd. key is DOMObject*
    DOM::Event *m_evt;
    bool m_inlineCode;
    bool m_timerCallback;
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
  inline Value DOMObjectLookupGet(ExecState *exec, const Identifier &propertyName,
                                  const HashTable* table, const ThisImp* thisObj)
  {
    const HashEntry* entry = Lookup::findEntry(table, propertyName);

    if (!entry) // not found, forward to parent
      return thisObj->ParentImp::tryGet(exec, propertyName);

    if (entry->attr & Function) {
      return lookupOrCreateFunction<FuncImp>(exec, propertyName, thisObj, entry->value, entry->params, entry->attr);
    }
    return thisObj->getValueProperty(exec, entry->value);
  }

  /**
   * Simplified version of DOMObjectLookupGet in case there are no
   * functions, only "values".
   */
  template <class ThisImp, class ParentImp>
  inline Value DOMObjectLookupGetValue(ExecState *exec, const Identifier &propertyName,
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
      fprintf(stderr,"WARNING: Attempt to change value of readonly property '%s'\n",propertyName.ascii());
#else
    ; // do nothing
#endif
    else
      thisObj->putValueProperty(exec, entry->value, value, attr);
  }
  
// Versions of prototype functions that properly support instanceof,
// and are compatible with trunk.
#define KJS_DEFINE_PROTOTYPE_IMP(ClassProto,ProtoCode) \
  class ClassProto : public ObjectImp { \
  friend Object cacheGlobalObject<ClassProto>(ExecState *exec, const Identifier &propertyName); \
  public: \
    static Object self(ExecState *exec); \
    virtual const ClassInfo *classInfo() const { return &info; } \
    static const ClassInfo info; \
    Value get(ExecState *exec, const Identifier &propertyName) const; \
  protected: \
    ClassProto( ExecState *exec ) \
      : ObjectImp( ProtoCode ) {} \
    \
    static Identifier* s_name; \
    static Identifier* name(); \
  };

#define KJS_DEFINE_PROTOTYPE(ClassProto) \
        KJS_DEFINE_PROTOTYPE_IMP(ClassProto, exec->interpreter()->builtinObjectPrototype())

#define KJS_DEFINE_PROTOTYPE_WITH_PROTOTYPE(ClassProto, ClassProtoProto) \
            KJS_DEFINE_PROTOTYPE_IMP(ClassProto, ClassProtoProto::self(exec))

#define KJS_EMPTY_PROTOTYPE_IMP(ClassName, ClassProto, ProtoCode) \
      class ClassProto : public ObjectImp { \
      friend Object cacheGlobalObject<ClassProto>(ExecState *exec, const Identifier &propertyName); \
      public: \
        static Object self(ExecState *exec) { \
          return cacheGlobalObject<ClassProto>(exec, *name()); \
        } \
        virtual const ClassInfo *classInfo() const { return &info; } \
        static const ClassInfo info; \
      protected: \
        ClassProto( ExecState *exec ) \
          : ObjectImp( ProtoCode ) {} \
        \
        static Identifier* s_name; \
        static Identifier* name() { \
            if (!s_name) s_name = new Identifier("[[" ClassName ".prototype]]"); \
            return s_name; \
        }\
      }; \
      Identifier* ClassProto::s_name = 0; \
      const ClassInfo ClassProto::info = { ClassName, 0, 0, 0 };
      
#define KJS_EMPTY_PROTOTYPE_WITH_PROTOTYPE(ClassName, ClassProto, ClassProtoProto) \
  KJS_EMPTY_PROTOTYPE_IMP(ClassName, ClassProto, ClassProtoProto::self(exec))

//### this doesn't implement hasProperty, but stuff in lookup.h didn't 
//either (just did the forward)
#define KJS_IMPLEMENT_PROTOTYPE(ClassName, ClassProto, ClassFunc) \
    const ClassInfo ClassProto::info = { ClassName, 0, &ClassProto##Table, 0 }; \
    Identifier* ClassProto::s_name = 0; \
    Object ClassProto::self(ExecState *exec) \
    { \
      return cacheGlobalObject<ClassProto>(exec, *name()); \
    } \
    Value ClassProto::get(ExecState *exec, const Identifier &propertyName) const \
    { \
      /*fprintf( stderr, "%sProto::get(%s) [in macro, no parent]\n", info.className, propertyName.ascii());*/ \
      return lookupGetFunction<ClassFunc,ObjectImp>(exec, propertyName, &ClassProto##Table, this ); \
    } \
    Identifier* ClassProto::name() \
    { \
      if (!s_name) s_name = new Identifier("[[" ClassName ".prototype]]"); \
      return s_name; \
    } 
    
  // Modified version of IMPLEMENT_PROTOFUNC, to use DOMFunction and tryCall
#define IMPLEMENT_PROTOFUNC_DOM(ClassFunc) \
  class ClassFunc : public DOMFunction { \
  public: \
    ClassFunc(ExecState *exec, int i, int len) \
       : DOMFunction( exec ), id(i) { \
       Value protect(this); \
       put(exec,lengthPropertyName,Number(len),DontDelete|ReadOnly|DontEnum); \
    } \
    /** You need to implement that one */ \
    virtual Value tryCall(ExecState *exec, Object &thisObj, const List &args); \
  private: \
    int id; \
  }; 

  Value getLiveConnectValue(KParts::LiveConnectExtension *lc, const QString & name, const int type, const QString & value, int id);


// This is used to create pseudo-constructor objects, like Mozillaish
// Element, HTMLDocument, etc., which do not act like real constructors,
// but do have the prototype property pointing to prototype of "instances"
#define DEFINE_PSEUDO_CONSTRUCTOR(ClassName) \
  class ClassName : public DOMObject { \
      public: \
          ClassName(ExecState *); \
          virtual const ClassInfo* classInfo() const { return &info; } \
          static const ClassInfo info; \
          static Object self(ExecState *exec); \
  };

#define IMPLEMENT_PSEUDO_CONSTRUCTOR_IMP(Class,ClassName,ProtoClass,ParentProto) \
    const ClassInfo Class::info = { ClassName, 0, 0, 0 }; \
    Class::Class(ExecState* exec): DOMObject(ParentProto) {\
        Object proto = ProtoClass::self(exec); \
        putDirect(prototypePropertyName, proto.imp(), DontDelete|ReadOnly); \
    }\
    Object Class::self(ExecState *exec) { \
        return Object(cacheGlobalObject<Class>(exec, "[[" ClassName ".constructor]]")); \
    }

#define IMPLEMENT_PSEUDO_CONSTRUCTOR(Class,ClassName,ProtoClass) \
    IMPLEMENT_PSEUDO_CONSTRUCTOR_IMP(Class,ClassName,ProtoClass,exec->lexicalInterpreter()->builtinObjectPrototype())

#define IMPLEMENT_PSEUDO_CONSTRUCTOR_WITH_PARENT(Class,ClassName,ProtoClass,ParentProtoClass) \
    IMPLEMENT_PSEUDO_CONSTRUCTOR_IMP(Class,ClassName,ProtoClass,ParentProtoClass::self(exec))

// This declares a constant table, which merely maps everything in its 
// table to its token value. Can be used as a prototype
#define DEFINE_CONSTANT_TABLE(Class) \
   class Class : public DOMObject { \
   public: \
     Class(ExecState *exec): DOMObject(exec->lexicalInterpreter()->builtinObjectPrototype()) {} \
     \
     virtual Value tryGet(ExecState *exec, const Identifier &propertyName) const;\
     Value  getValueProperty(ExecState * /*exec*/, int token) const; \
     virtual const ClassInfo* classInfo() const { return &info; } \
     static const ClassInfo info; \
     static Object self(ExecState *exec);\
     static Identifier* s_name; \
     static Identifier* name(); \
   };

// Emits an implementation of a constant table 
#define IMPLEMENT_CONSTANT_TABLE(Class,ClassName) \
    Value Class::tryGet(ExecState *exec, const Identifier &propertyName) const { \
        return DOMObjectLookupGetValue<Class, DOMObject>(exec, propertyName, &Class##Table, this);\
    } \
    Value Class::getValueProperty(ExecState * /*exec*/, int token) const { \
        /* We use the token as the value to return directly*/ \
        return Number((unsigned int)token); \
    }  \
    Object Class::self(ExecState *exec) { \
        return cacheGlobalObject<Class>(exec, *name()); \
    } \
    Identifier* Class::s_name = 0; \
    Identifier* Class::name() { \
        if (!s_name) s_name = new Identifier("[[" ClassName ".constant_table]]"); \
        return s_name; \
    } \
    const ClassInfo Class::info = { ClassName, 0, &Class##Table, 0 };

   
// Hide some of the stuff in lookup.h..
#undef PUBLIC_DEFINE_PROTOTYPE
#undef DEFINE_PROTOTYPE
#undef IMPLEMENT_PROTOTYPE
#undef PUBLIC_IMPLEMENT_PROTOTYPE
#undef IMPLEMENT_PROTOTYPE_WITH_PARENT

} // namespace

#endif
