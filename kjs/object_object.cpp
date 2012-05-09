// -*- c-basic-offset: 2 -*-
// krazy:excludeall=doublequote_chars (UStrings aren't QStrings)
/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 1999-2000 Harri Porten (porten@kde.org)
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include "object_object.h"
#include <config.h>

#include "operations.h"
#include "function_object.h"
#include <stdio.h>

namespace KJS {


/**
 * @internal
 *
 * Class to implement all methods that are properties of the
 * Object object
 */
class ObjectObjectFuncImp : public InternalFunctionImp {
public:
    ObjectObjectFuncImp(ExecState *, FunctionPrototype *, int i, int len, const Identifier& );

    virtual JSValue *callAsFunction(ExecState *, JSObject *thisObj, const List &args);

    enum { GetPrototypeOf, GetOwnPropertyNames, Keys };

private:
    int id;
};

// ------------------------------ ObjectPrototype --------------------------------

ObjectPrototype::ObjectPrototype(ExecState* exec, FunctionPrototype* funcProto)
  : JSObject() // [[Prototype]] is null
{
    static const Identifier* hasOwnPropertyPropertyName = new Identifier("hasOwnProperty");
    static const Identifier* propertyIsEnumerablePropertyName = new Identifier("propertyIsEnumerable");
    static const Identifier* isPrototypeOfPropertyName = new Identifier("isPrototypeOf");
    static const Identifier* defineGetterPropertyName = new Identifier("__defineGetter__");
    static const Identifier* defineSetterPropertyName = new Identifier("__defineSetter__");
    static const Identifier* lookupGetterPropertyName = new Identifier("__lookupGetter__");
    static const Identifier* lookupSetterPropertyName = new Identifier("__lookupSetter__");

    putDirectFunction(new ObjectProtoFunc(exec, funcProto, ObjectProtoFunc::ToString, 0, exec->propertyNames().toString), DontEnum);
    putDirectFunction(new ObjectProtoFunc(exec, funcProto, ObjectProtoFunc::ToLocaleString, 0, exec->propertyNames().toLocaleString), DontEnum);
    putDirectFunction(new ObjectProtoFunc(exec, funcProto, ObjectProtoFunc::ValueOf, 0, exec->propertyNames().valueOf), DontEnum);
    putDirectFunction(new ObjectProtoFunc(exec, funcProto, ObjectProtoFunc::HasOwnProperty, 1, *hasOwnPropertyPropertyName), DontEnum);
    putDirectFunction(new ObjectProtoFunc(exec, funcProto, ObjectProtoFunc::PropertyIsEnumerable, 1, *propertyIsEnumerablePropertyName), DontEnum);
    putDirectFunction(new ObjectProtoFunc(exec, funcProto, ObjectProtoFunc::IsPrototypeOf, 1, *isPrototypeOfPropertyName), DontEnum);

    // Mozilla extensions
    putDirectFunction(new ObjectProtoFunc(exec, funcProto, ObjectProtoFunc::DefineGetter, 2, *defineGetterPropertyName), DontEnum);
    putDirectFunction(new ObjectProtoFunc(exec, funcProto, ObjectProtoFunc::DefineSetter, 2, *defineSetterPropertyName), DontEnum);
    putDirectFunction(new ObjectProtoFunc(exec, funcProto, ObjectProtoFunc::LookupGetter, 1, *lookupGetterPropertyName), DontEnum);
    putDirectFunction(new ObjectProtoFunc(exec, funcProto, ObjectProtoFunc::LookupSetter, 1, *lookupSetterPropertyName), DontEnum);
}

JSObject* ObjectPrototype::self(ExecState* exec)
{
    return exec->lexicalInterpreter()->builtinObjectPrototype();
}

// ------------------------------ ObjectProtoFunc --------------------------------

ObjectProtoFunc::ObjectProtoFunc(ExecState* exec, FunctionPrototype* funcProto, int i, int len, const Identifier& name)
  : InternalFunctionImp(funcProto, name)
  , id(i)
{
  putDirect(exec->propertyNames().length, len, DontDelete|ReadOnly|DontEnum);
}


// ECMA 15.2.4.2, 15.2.4.4, 15.2.4.5, 15.2.4.7

JSValue *ObjectProtoFunc::callAsFunction(ExecState *exec, JSObject *thisObj, const List &args)
{
    switch (id) {
        case ValueOf:
            return thisObj;
        case HasOwnProperty: {
            PropertySlot slot;
            return jsBoolean(thisObj->getOwnPropertySlot(exec, Identifier(args[0]->toString(exec)), slot));
        }
        case IsPrototypeOf: {
            if (!args[0]->isObject())
                return jsBoolean(false);
         
            JSValue *v = static_cast<JSObject *>(args[0])->prototype();

            while (true) {
                if (!v->isObject())
                    return jsBoolean(false);
                
                if (thisObj == static_cast<JSObject *>(v))
                    return jsBoolean(true);
                
                v = static_cast<JSObject *>(v)->prototype();
            }
        }
        case DefineGetter: 
        case DefineSetter: {
            if (!args[1]->isObject() ||
                !static_cast<JSObject *>(args[1])->implementsCall()) {
                if (id == DefineGetter)
                    return throwError(exec, SyntaxError, "invalid getter usage");
                else
                    return throwError(exec, SyntaxError, "invalid setter usage");
            }

            if (id == DefineGetter)
                thisObj->defineGetter(exec, Identifier(args[0]->toString(exec)), static_cast<JSObject *>(args[1]));
            else
                thisObj->defineSetter(exec, Identifier(args[0]->toString(exec)), static_cast<JSObject *>(args[1]));
            return jsUndefined();
        }
        case LookupGetter:
        case LookupSetter: {
            Identifier propertyName = Identifier(args[0]->toString(exec));
            
            JSObject *obj = thisObj;
            while (true) {
                JSValue *v = obj->getDirect(propertyName);
                
                if (v) {
                    if (v->type() != GetterSetterType)
                        return jsUndefined();

                    JSObject *funcObj;
                        
                    if (id == LookupGetter)
                        funcObj = static_cast<GetterSetterImp *>(v)->getGetter();
                    else
                        funcObj = static_cast<GetterSetterImp *>(v)->getSetter();
                
                    if (!funcObj)
                        return jsUndefined();
                    else
                        return funcObj;
                }
                
                if (!obj->prototype() || !obj->prototype()->isObject())
                    return jsUndefined();
                
                obj = static_cast<JSObject *>(obj->prototype());
            }
        }
        case PropertyIsEnumerable:
            return jsBoolean(thisObj->propertyIsEnumerable(exec, Identifier(args[0]->toString(exec))));
        case ToLocaleString:
            return jsString(thisObj->toString(exec));
        case ToString:
        default:
            return jsString("[object " + thisObj->className() + "]");
    }
}

// ------------------------------ ObjectObjectImp --------------------------------

ObjectObjectImp::ObjectObjectImp(ExecState* exec, ObjectPrototype* objProto, FunctionPrototype* funcProto)
  : InternalFunctionImp(funcProto)
{
  static const Identifier* getPrototypeOf = new Identifier("getPrototypeOf");
  static const Identifier* getOwnPropertyNames = new Identifier("getOwnPropertyNames");
  static const Identifier* keys = new Identifier("keys");

  // ECMA 15.2.3.1
  putDirect(exec->propertyNames().prototype, objProto, DontEnum|DontDelete|ReadOnly);

  putDirectFunction(new ObjectObjectFuncImp(exec, funcProto, ObjectObjectFuncImp::GetPrototypeOf, 1, *getPrototypeOf), DontEnum);
  putDirectFunction(new ObjectObjectFuncImp(exec, funcProto, ObjectObjectFuncImp::GetOwnPropertyNames, 1, *getOwnPropertyNames), DontEnum);
  putDirectFunction(new ObjectObjectFuncImp(exec, funcProto, ObjectObjectFuncImp::Keys, 1, *keys), DontEnum);

  // no. of arguments for constructor
  putDirect(exec->propertyNames().length, jsNumber(1), ReadOnly|DontDelete|DontEnum);
}


bool ObjectObjectImp::implementsConstruct() const
{
  return true;
}

// ECMA 15.2.2
JSObject* ObjectObjectImp::construct(ExecState* exec, const List& args)
{
  JSValue* arg = args[0];
  switch (arg->type()) {
  case StringType:
  case BooleanType:
  case NumberType:
  case ObjectType:
      return arg->toObject(exec);
  case NullType:
  case UndefinedType:
      return new JSObject(exec->lexicalInterpreter()->builtinObjectPrototype());
  default:
      //### ASSERT_NOT_REACHED();
      return 0;
  }
}

JSValue* ObjectObjectImp::callAsFunction(ExecState* exec, JSObject* /*thisObj*/, const List &args)
{
    return construct(exec, args);
}

// ------------------------------ ObjectObjectFuncImp ----------------------------

ObjectObjectFuncImp::ObjectObjectFuncImp(ExecState* exec, FunctionPrototype* funcProto, int i, int len, const Identifier& name)
    : InternalFunctionImp(funcProto, name), id(i)
{
    putDirect(exec->propertyNames().length, len, DontDelete|ReadOnly|DontEnum);
}

JSValue *ObjectObjectFuncImp::callAsFunction(ExecState* exec, JSObject*, const List& args)
{
    switch (id) {
    case GetPrototypeOf: { //ECMA Edition 5.1r6 - 15.2.3.2
        JSObject* jso = args[0]->getObject();
        if (!jso)
            return throwError(exec, TypeError, "\'" + args[0]->toString(exec) + "\' is not an Object");
        return jso->prototype();
    }
    case GetOwnPropertyNames: //ECMA Edition 5.1r6 - 15.2.3.4
    case Keys: { //ECMA Edition 5.1r6 - 15.2.3.14
        JSObject* jso = args[0]->getObject();
        if (!jso)
            return throwError(exec, TypeError, "\'" + args[0]->toString(exec) + "\' is not an Object");

        JSObject *ret = static_cast<JSObject *>(exec->lexicalInterpreter()->builtinArray()->construct(exec, List::empty()));
        PropertyNameArray propertyNames;

        if (id == Keys)
            jso->getOwnPropertyNames(exec, propertyNames, PropertyMap::ExcludeDontEnumProperties);
        else // id == GetOwnPropertyNames
            jso->getOwnPropertyNames(exec, propertyNames, PropertyMap::IncludeDontEnumProperties);
        PropertyNameArrayIterator propEnd = propertyNames.end();
        unsigned int n = 0;
        for (PropertyNameArrayIterator propIter = propertyNames.begin(); propIter != propEnd; propIter++) {
            Identifier name = *propIter;
            ret->put(exec, n, jsString(name.ustring()), None);
            ++n;
        }
        ret->put(exec, exec->propertyNames().length, jsNumber(n), DontEnum | DontDelete);
        return ret;
    }
    default:
        return jsUndefined();
    }
}

}   // namespace KJS
