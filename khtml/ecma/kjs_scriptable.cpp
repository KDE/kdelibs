/* This file is part of the KDE project
   Copyright (C) 2010 Maksim Orlovich <maksim@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/


#include "kjs_scriptable.h"
#include "kjs_binding.h"

// Table of exported objects, including their refcount.
// mark from script interpreter.
// 
// Imported objects:
// weak reference table for wrappers.
// 
// importObject,
// exportObject helpers,
// 
// // ### object invalidation if the extension is removed?
// 
// ... tracking refcounts per context may help deal with emergency
// cleanups, but is heavy/icky.

namespace KJS {

static bool isException(const QVariant& v)
{
    return v.canConvert<ScriptableExtension::Exception>();
}

static bool isFuncRef(const QVariant& v)
{
    return v.canConvert<ScriptableExtension::Exception>();
}

static bool isForeignObject(const QVariant& v)
{
    return v.canConvert<ScriptableExtension::Object>();
}

const ClassInfo WrapScriptableObject::info = { " WrapScriptableObject", 0, 0, 0 };

WrapScriptableObject::WrapScriptableObject(ExecState* exec, Type t,
                                           ScriptableExtension* owner, quint64 objId,
                                           const QString& field):
    objExtension(owner), objId(objId), field(field), type(t), tableKey(owner)
{
    owner->acquire(objId);
}

WrapScriptableObject::~WrapScriptableObject()
{
    if (ScriptableExtension* o = objExtension.data()) {
        o->release(objId);
    }

    ScriptableExtension::Object obj(tableKey, objId);
    if (type == Object) {
        KHTMLScriptable::importedObjects()->remove(obj);
    } else {
        KHTMLScriptable::importedFunctions()->remove(ScriptableExtension::FunctionRef(obj, field));
    }
}

QVariant WrapScriptableObject::doGet(ExecState* exec, const ScriptableExtension::Object& o,
                                     const QString& field, bool* ok)
{
    *ok = false;

    if (!o.owner) // such as when was constructed from null .data();
        return QVariant();

    QVariant v = o.owner->get(principal(exec), o.objId, field);

    if (!isException(v))
        *ok = true;
    return v;
}

ScriptableExtension::Object WrapScriptableObject::resolveReferences(
                                                    ExecState* exec,
                                                    const ScriptableExtension::FunctionRef& f,
                                                    bool* ok)
{
    QVariant v = doGet(exec, f.base, f.field, ok);
    if (*ok) {
        // See what sort of type we got.
        if (isForeignObject(v)) {
            return v.value<ScriptableExtension::Object>();
        } else if (isFuncRef(v)) {
            return resolveReferences(exec, v.value<ScriptableExtension::FunctionRef>(), ok);
        } else {
            // We got a primitive. We don't care for those.
            *ok = false;
        }
    }
    return ScriptableExtension::Object();
}

ScriptableExtension::Object WrapScriptableObject::resolveAnyReferences(ExecState* exec, bool* ok)
{
    ScriptableExtension::Object obj(objExtension.data(), objId);

    if (type == FunctionRef)
        obj = resolveReferences(exec, ScriptableExtension::FunctionRef(obj, field), ok);

    if (!obj.owner)
        *ok = false;

    return obj;
}

bool WrapScriptableObject::getOwnPropertySlot(ExecState* exec, const Identifier& i,
                                             PropertySlot& slot)
{
    bool ok;
    ScriptableExtension::Object actualObj = resolveAnyReferences(exec, &ok);

    if (!ok)
        return false;

    QVariant v = doGet(exec, actualObj, i.qstring(), &ok);

    if (!ok)
        return false;

    return getImmediateValueSlot(this, KHTMLScriptable::importValue(exec, v), slot);
}

void WrapScriptableObject::put(ExecState* exec, const Identifier& i, JSValue* value, int)
{
    // ### Do we swallow failure, or what?
    bool ok;
    ScriptableExtension::Object actualObj = resolveAnyReferences(exec, &ok);    

    if (!ok)
        return;

    QVariant sv = KHTMLScriptable::exportValue(value);
    actualObj.owner->put(principal(exec), actualObj.objId, i.qstring(), sv);
}

bool WrapScriptableObject::deleteProperty(ExecState* exec, const Identifier& i)
{
    bool ok;
    ScriptableExtension::Object actualObj = resolveAnyReferences(exec, &ok);

    if (!ok)
        return false;

    return actualObj.owner->removeProperty(principal(exec),
                                           actualObj.objId, i.qstring());
}

ScriptableExtension::ArgList WrapScriptableObject::exportArgs(const List& l)
{
    ScriptableExtension::ArgList ol;
    for (int p = 0; p < l.size(); ++p)
        ol.append(KHTMLScriptable::exportValue(l.at(p)));
    return ol;
}

JSValue* WrapScriptableObject::callAsFunction(ExecState *exec, JSObject *thisObj, const List &args)
{
    if (type == Object) {
        // if ()
        // callAsFunction
    } else {
        // callFunctionRefence.
    }
}

/**
   SECURITY: For the conversion helpers, it is assumed that 'exec' corresponds
   to the appropriate principal.
*/   


JSObject* KHTMLScriptable::importObject(ExecState* exec, const QVariant& v)
{
    ScriptableExtension::Object obj = v.value<ScriptableExtension::Object>();
    if (JSObject* our = tryGetNativeObject(obj)) {
        return our;
    } else {
        // Create a wrapper, and register the import, this is just for
        // hashconsing.
        if (WrapScriptableObject* old = importedObjects()->value(obj)) {
            return old;
        } else {
            WrapScriptableObject* wrap =
                new WrapScriptableObject(exec, WrapScriptableObject::Object,
                                         obj.owner, obj.objId);
            importedObjects()->insert(obj, wrap);
            return wrap;            
        }
    }
}

// Note: this is used to convert a full function ref to a value,
// which loses the this-binding in the native case.  We do keep the pair in the
// external case, inside the wrapper,  since the method might not have an own
// name, and we'd like to be able to refer to it.
JSValue* KHTMLScriptable::importFunctionRef(ExecState* exec, const QVariant& v)
{
    ScriptableExtension::FunctionRef fr = v.value<ScriptableExtension::FunctionRef>();

    if (JSObject* base = tryGetNativeObject(fr.base)) {
        return base->get(exec, fr.field);
    } else {
        if (WrapScriptableObject* old = importedFunctions()->value(fr)) {
            return old;
        } else {
            WrapScriptableObject* wrap =
                new WrapScriptableObject(exec, WrapScriptableObject::FunctionRef,
                                         fr.base.owner, fr.base.objId, fr.field);
            importedFunctions()->insert(fr, wrap);
            return wrap;
        }
    }
}

JSValue* KHTMLScriptable::importValue(ExecState* exec, const QVariant& v)
{
    if (v.canConvert<ScriptableExtension::FunctionRef>())
        return importFunctionRef(exec, v);
    if (v.canConvert<ScriptableExtension::Object>())
        return importObject(exec, v);
    if (v.canConvert<ScriptableExtension::Null>())
        return jsNull();
    if (v.canConvert<ScriptableExtension::Undefined>())
        return jsUndefined();
    if (v.type() == QVariant::Bool)
        return jsBoolean(v.toBool());
    if (v.type() == QVariant::String)
        return jsString(v.toString());
    if (v.canConvert<double>())
        return jsNumber(v.toDouble());
    kWarning() << "conversion from " << v << "failed";
    return jsNull();
}

QVariant exportObject(JSObject* o)
{
    // XSS checks are done at get time, so if we have a value here, we can
    // export it.

    if (o->inherits(&WrapScriptableObject::info)) {
        // Re-exporting external one
    } else {
        // Add to export table if needed. Use the pointer
        // as an ID.
        //TODO
    }
}

QVariant KHTMLScriptable::exportValue(JSValue* v)
{
    switch (v->type()) {
    case NumberType:
        return QVariant::fromValue(v->getNumber());
    case BooleanType:
        return QVariant::fromValue(v->getBoolean());
    case NullType:
        return QVariant::fromValue(ScriptableExtension::Null());
    case StringType:
        return QVariant::fromValue(v->getString().qstring());
    case ObjectType:
        return exportObject(v->getObject());
    case UndefinedType:
    default:
        return QVariant::fromValue(ScriptableExtension::Undefined());
    }
}

#if 0
exportValue ones.






get, etc --- check for part for extension, do XSS check on that?
#endif

KHTMLScriptable::KHTMLScriptable(KHTMLPart* part):
    ScriptableExtension(part), m_part(part)
{
}


QVariant KHTMLScriptable::enclosingObject(KParts::ReadOnlyPart* childPart)
{
    khtml::ChildFrame* f = m_part->frame(childPart);

    if (!f) {
        kWarning(6031) << "unable to find frame. Huh?";
        return scriptableNull();
    }

    // ### Note: this should never actually get an iframe once iframes are fixed
    // TODO    
}

QVariant KHTMLScriptable::callAsFunction(ScriptableExtension* caller,
                                                  quint64 objId, const ArgList& args)
{
    KHTMLPart* ctx = partForPrincipal(caller);
    KJSProxy*  kjs = ctx ? ctx->jScript() : 0;
    if (!kjs) {
        kWarning(6031) << "Trying to script the unscriptable";
        return exception("No scripting context or frame");
    }

    JSObject* fn = objectForId(objId);
    if (!fn || !fn->implementsCall()) {
        kWarning(6031) << "Call on a function or non-calleable" << fn << objId;
        return exception("Not a function");
    }

    //fn->callAsFunction(
}

KHTMLPart* KHTMLScriptable::partForPrincipal(ScriptableExtension* caller)
{
    // We implement our security checks by delegating to the KHTMLPart corresponding
    // to the given plugin's principal (which is the KHTMLPart owning it), and letting
    // the underlying implementation perform them (which it has to anyway)

    if (KHTMLScriptable* o = qobject_cast<KHTMLScriptable*>(caller)) {
        return o->m_part;
    } else {
        //TODO
    }
}


List KHTMLScriptable::decodeArgs(const ArgList& args)
{
    List out;
    for (int i = 0; i < args.size(); ++i)
        out.append(importValue(args[i]));
    return out;
}

// ::get() {
// 
//     if result is a function... return function reference instead..
// }


JSObject* KHTMLScriptable::tryGetNativeObject(const Object& sObj)
{
    if (KHTMLScriptable* o = qobject_cast<KHTMLScriptable*>(sObj.owner)) {
        return o->objectForId(sObj.objId);
    } else {
        return 0;
    }
}

} // namespace KJS

// kate: space-indent on; indent-width 4; replace-tabs on;