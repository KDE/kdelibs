// -*- c-basic-offset: 2 -*-
/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 1999-2000 Harri Porten (porten@kde.org)
 *  Copyright (C) 2003, 2006 Apple Computer, Inc.
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
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 *
 */

#ifndef KJS_FUNCTION_H
#define KJS_FUNCTION_H

#include "object.h"
#include <wtf/OwnPtr.h>

namespace KJS {

  class ActivationImp;
  class FunctionPrototype;

  enum CodeType { GlobalCode,
                  EvalCode,
                  FunctionCode };

  class KJS_EXPORT InternalFunctionImp : public JSObject {
  public:
    InternalFunctionImp();
    InternalFunctionImp(FunctionPrototype*);
    InternalFunctionImp(FunctionPrototype*, const Identifier&);

    virtual bool implementsCall() const;
    virtual JSValue* callAsFunction(ExecState*, JSObject* thisObjec, const List& args) = 0;
    virtual bool implementsHasInstance() const;

    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
    const Identifier& functionName() const { return m_name; }
    void setFunctionName(const Identifier& name) { m_name = name; }

  private:
    Identifier m_name;
#ifdef WIN32
    InternalFunctionImp(const InternalFunctionImp&);
    InternalFunctionImp& operator=(const InternalFunctionImp&);
#endif
  };

  /**
   * @internal
   *
   * The initial value of Function.prototype (and thus all objects created
   * with the Function constructor)
   */
  class FunctionPrototype : public InternalFunctionImp {
  public:
    FunctionPrototype(ExecState *exec);
    virtual ~FunctionPrototype();

    virtual JSValue *callAsFunction(ExecState *exec, JSObject *thisObj, const List &args);
  };

  class IndexToNameMap {
  public:
    IndexToNameMap(FunctionImp *func, const List &args);
    ~IndexToNameMap();

    Identifier& operator[](int index);
    Identifier& operator[](const Identifier &indexIdentifier);
    bool isMapped(const Identifier &index) const;
    void unMap(const Identifier &index);

  private:
    IndexToNameMap(); // prevent construction w/o parameters
    int size;
    Identifier * _map;
  };

  class Arguments : public JSObject {
  public:
    Arguments(ExecState *exec, FunctionImp *func, const List &args, ActivationImp *act);
    virtual void mark();
    virtual bool getOwnPropertySlot(ExecState *, const Identifier &, PropertySlot&);
    virtual void put(ExecState *exec, const Identifier &propertyName, JSValue *value, int attr = None);
    virtual bool deleteProperty(ExecState *exec, const Identifier &propertyName);
    virtual const ClassInfo *classInfo() const { return &info; }
    static const ClassInfo info;
  private:
    static JSValue *mappedIndexGetter(ExecState *exec, JSObject *, const Identifier &, const PropertySlot& slot);

    ActivationImp *_activationObject;
    mutable IndexToNameMap indexToNameMap;
  };

  class ActivationImp : public JSObject {
  public:
    ActivationImp(FunctionImp *function, const List &arguments);
    ~ActivationImp();

    virtual bool getOwnPropertySlot(ExecState *exec, const Identifier &, PropertySlot&);
    virtual void put(ExecState *exec, const Identifier &propertyName, JSValue *value, int attr = None);
    virtual bool deleteProperty(ExecState *exec, const Identifier &propertyName);
    
    //This is only used by declaration code, so it never check r/o attr
    void putLocal(int propertyID, JSValue *value) {
      assert(validLocal(propertyID));
      _locals[propertyID].value = value;
        //We do not have to touch the flags here -- they're pre-computed..
    }

    void putLocalChecked(int propertyID, JSValue *value) {
      if (isLocalReadOnly(propertyID)) return;
      _locals[propertyID].value = value;
    }

    JSValue** getLocalDirect(int propertyID) {
      assert(validLocal(propertyID));
      return &_locals[propertyID].value;
    }

    bool isLocalReadOnly(int propertyID) {
      return (_locals[propertyID].attr & ReadOnly) == ReadOnly;
    }

    virtual const ClassInfo *classInfo() const { return &info; }
    static const ClassInfo info;

    virtual void mark();
    virtual bool isActivation() const { return true; }
    void releaseArguments() { _arguments.reset(); }

    void setupLocals();
    void setupFunctionLocals(ExecState *exec);
  private:
    static PropertySlot::GetValueFunc getArgumentsGetter();
    static JSValue *argumentsGetter(ExecState *exec, JSObject *, const Identifier &, const PropertySlot& slot);
    void createArgumentsObject(ExecState *exec);

    struct Local {
      JSValue* value;
      int      attr;
    };

    FunctionImp *_function;
    List _arguments;


    //List of locals, but the entry 0 is special:
    //the value here is the arguments object,
    //the attr is the length of the array (e.g. numLocals + 1)
    Local *_locals;

    int  numLocals() const        { return _locals[0].attr; }
    bool validLocal(int id) const { return 0 < id && id < numLocals(); }
  };

  class GlobalFuncImp : public InternalFunctionImp {
  public:
    GlobalFuncImp(ExecState*, FunctionPrototype*, int i, int len, const Identifier&);
    virtual JSValue *callAsFunction(ExecState *exec, JSObject *thisObj, const List &args);
    virtual CodeType codeType() const;
    enum { Eval, ParseInt, ParseFloat, IsNaN, IsFinite, Escape, UnEscape,
           DecodeURI, DecodeURIComponent, EncodeURI, EncodeURIComponent
#ifndef NDEBUG
           , KJSPrint
#endif
};
  private:
    int id;
  };

} // namespace

#endif
