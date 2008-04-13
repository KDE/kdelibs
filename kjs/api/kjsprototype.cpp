/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 2008 Harri Porten (porten@kde.org)
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

#include "kjsprototype.h"
#include "kjsinterpreter.h"
#include "kjsprivate.h"

#include "kjs/object.h"
#include "kjs/context.h"
#include "kjs/interpreter.h"

#include <qmap.h>

using namespace KJS;

class KJSCustomProperty
{
public:
    KJSCustomProperty(KJSPrototype::PropertyGetter g,
                      KJSPrototype::PropertySetter s)
        : getter(g), setter(s)
    {
    }

    JSValue* read(ExecState* exec, void* object);
    void write(ExecState* exec, void* object, JSValue* value);

private:
    KJSPrototype::PropertyGetter getter;
    KJSPrototype::PropertySetter setter;
};

class CustomObject : public JSObject {
public:
    CustomObject(JSValue* proto, void* v)
        : JSObject(proto),
          iv(v)
    {
    }

    void put(ExecState* exec, const Identifier& id,
             JSValue *value, int attr = None);

    void* internalValue() { return iv; }

private:
    void* iv;
};

JSValue* KJSCustomProperty::read(ExecState* exec, void* object)
{
    assert(getter);
    
    KJSContext ctx(EXECSTATE_HANDLE(exec));
    KJSObject res = (*getter)(&ctx, object);
    return JSVALUE(&res);
}

void KJSCustomProperty::write(ExecState* exec, void* object, JSValue* value)
{
    assert(setter); // FIXME: cover read-only case

    KJSContext ctx(EXECSTATE_HANDLE(exec));

    KJSObject vo(JSVALUE_HANDLE(value));
    (*setter)(&ctx, object, vo);
}

static JSValue* getPropertyValue(ExecState* exec, JSObject *originalObject,
                                 const Identifier&, const PropertySlot& sl)
{
    CustomObject* o = static_cast<CustomObject*>(originalObject);
    KJSCustomProperty* p =
        reinterpret_cast<KJSCustomProperty*>(sl.customValue());

    return p->read(exec, o->internalValue());
}

// FIXME: or use Identifier?
// FIXME: use values
typedef QMap<UString, KJSCustomProperty*> CustomPropertyMap;

class CustomPrototype : public JSObject {
public:
    CustomPrototype(JSValue* proto)
        : JSObject(proto)
    {
    }
    ~CustomPrototype()
    {
        qDeleteAll(properties);
    }

    bool getOwnPropertySlot(ExecState *exec, const Identifier& id,
                            PropertySlot& sl)
    {
        CustomPropertyMap::iterator it = properties.find(id.ustring());
        if (it == properties.end())
            return JSObject::getOwnPropertySlot(exec, id, sl);
        
        sl.setCustomValue(0, *it, getPropertyValue);

        return true;
    }

    void registerProperty(const QString& name,
                          KJSPrototype::PropertyGetter g,
                          KJSPrototype::PropertySetter s)
    {
        properties.insert(toUString(name), new KJSCustomProperty(g, s));
    }

    bool setProperty(ExecState* exec, CustomObject* obj,
                     const Identifier& id, JSValue* value)
    {
        CustomPropertyMap::iterator it = properties.find(id.ustring());
        if (it == properties.end())
            return false;

        (*it)->write(exec, obj->internalValue(), value);
        
        return true;
    }

private:
    CustomPropertyMap properties;
};

void CustomObject::put(ExecState* exec, const Identifier& id,
                       JSValue* value, int attr)
{
    CustomPrototype* p = static_cast<CustomPrototype*>(prototype());

    if (!p->setProperty(exec, this, id, value))
        JSObject::put(exec, id, value, attr);
}

KJSPrototype::KJSPrototype(const KJSInterpreter& ip)
{
    KJS::Interpreter* i = INTERPRETER(&ip);
    JSObject* objectProto = i->builtinObjectPrototype();

    CustomPrototype* p = new CustomPrototype(objectProto);
    gcProtect(p);

    hnd = PROTOTYPE_HANDLE(p);
}

KJSPrototype::~KJSPrototype()
{
    gcUnprotect(PROTOTYPE(this));
}

void KJSPrototype::defineConstant(KJSContext* ctx,
                                  const QString& name, double value)
{
    CustomPrototype* p = PROTOTYPE(this);

    ExecState* exec = EXECSTATE(ctx);
    p->put(exec, toIdentifier(name), jsNumber(value),
           DontEnum|DontDelete|ReadOnly);
}

void KJSPrototype::defineConstant(KJSContext* ctx,
                                  const QString& name, const QString& value)
{
    CustomPrototype* p = PROTOTYPE(this);

    ExecState* exec = EXECSTATE(ctx);
    p->put(exec, toIdentifier(name), jsString(toUString(value)),
           DontEnum|DontDelete|ReadOnly);
}

KJSObject KJSPrototype::constructObject(void *internalValue)
{
    CustomPrototype* p = PROTOTYPE(this);

    CustomObject* newObj = new CustomObject(p, internalValue);
    return KJSObject(JSVALUE_HANDLE(newObj));
}

void KJSPrototype::defineProperty(KJSContext* ctx,
                                  const QString& name,
                                  PropertyGetter getter,
                                  PropertySetter setter)
{
    CustomPrototype* p = PROTOTYPE(this);

    p->registerProperty(name, getter, setter);
}

void KJSPrototype::defineFunction(KJSContext* ctx,
                                  const QString& name, FunctionCall callback)
{
    CustomPrototype* p = PROTOTYPE(this);
}


