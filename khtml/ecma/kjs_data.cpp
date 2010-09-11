/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 2010 Maksim Orlovich <maksim@kde.org>
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
 */
#include "kjs_data.h"
#include <dom/dom_exception.h>

#include <kjs/array_instance.h>

#include <QSet>

namespace KJS {

// HTML5 deep copy algorithm, as described in "2.7.5 Safe passing of structured data"
static JSValue* cloneInternal(ExecState* exec, Interpreter* ctx, JSValue* in, QSet<JSObject*>& visited)
{
    switch (in->type()) {
        case NumberType:
        case BooleanType:
        case UndefinedType:
        case NullType:
        case StringType:
            // Values -> can pass straight through.
            return in;

        case ObjectType: {
            JSObject* obj = in->getObject();

            // Some things are handled by creating a new wrapper for their value;
            // this includes both JS-builtin types like autoboxing wrappers, and
            // those of HTML5 types we support that have deep cloning specified.
            // This goes through valueClone.
            if (JSObject* copy = obj->valueClone(ctx))
                return copy;

            // Otherwise, we can only clone if it it's an array or plain object
            // we haven't yet visited
            if (visited.contains(obj)) {
                setDOMException(exec, DOM::DOMException::DATA_CLONE_ERR);
                break;
            }

            visited.insert(obj);

            JSObject* clone = 0;
            if (obj->inherits(&ArrayInstance::info)) {
                clone = new ArrayInstance(ctx->builtinArrayPrototype(), 0);
            } else if (!obj->classInfo()) { // plain object
                clone = new JSObject(ctx->builtinObjectPrototype());
            } else {            
                // Something complicated and native -> error out
                setDOMException(exec, DOM::DOMException::DATA_CLONE_ERR);
                break;
            }

            // Copy over clones of properties
            PropertyNameArray props;
            obj->getOwnPropertyNames(exec, props);
            for (PropertyNameArrayIterator i = props.begin(); i != props.end(); ++i) {
                JSValue* propVal = obj->get(exec, *i);
                clone->put(exec, *i, cloneInternal(exec, ctx, propVal, visited)); // ### flags?
            }
            
            break;
        }

        default: // shouldn't happen!
            setDOMException(exec, DOM::DOMException::DATA_CLONE_ERR);
    }

    return jsUndefined();
}

class JSMessageData : public DOM::MessageEventImpl::Data {
public:
    virtual DOM::MessageEventImpl::DataType messageDataType() const {
        return DOM::MessageEventImpl::JS_VALUE;
    }

    JSMessageData(JSValue* val): m_value(val) {}

    ProtectedPtr<JSValue> m_value;
};

DOM::MessageEventImpl::Data* encapsulateMessageEventData(ExecState* exec, Interpreter* ctx,
                                                         JSValue* data)
{
    QSet<JSObject*> visited;
    JSValue* copy = cloneInternal(exec, ctx, data, visited);
    if (exec->hadException())
        return 0;
    else
        return new JSMessageData(copy);
}

JSValue* getMessageEventData(ExecState* exec, DOM::MessageEventImpl::Data* data)
{
    if (data && data->messageDataType() == DOM::MessageEventImpl::JS_VALUE)
        return static_cast<JSMessageData*>(data)->m_value.get();
    else
        return jsUndefined();
}

} // namespace KJS

// kate: indent-width 4; replace-tabs on; tab-width 4; space-indent on;
