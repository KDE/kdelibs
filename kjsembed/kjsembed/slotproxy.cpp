/* This file is part of the KDE libraries
    Copyright (C) 2005, 2006 KJSEmbed Authors
    See included AUTHORS file.

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
#include <QMetaMethod>
#include <QMetaType>

#include <kjs/interpreter.h>

#include "slotproxy.h"
#include "value_binding.h"

using namespace KJSEmbed;
using namespace KJS;

SlotProxy::SlotProxy(KJS::JSObject *obj, KJS::Interpreter *interpreter, QObject *parent, const QByteArray &signature)
    : QObject(parent), m_interpreter(interpreter), m_object(obj)
{
    m_signature = QMetaObject::normalizedSignature( signature );
    uint signatureSize = m_signature.size() + 1;

    // content:
    m_data[0] = 1;  // revision
    m_data[1] = 0;  // classname
    m_data[2] = 0;  // classinfo
    m_data[3] = 0;  // classinfo
    m_data[4] = 1;  // methods
    m_data[5] = 10; // methods
    m_data[6] = 0;  // properties
    m_data[7] = 0;  // properties
    m_data[8] = 0;  // enums/sets
    m_data[9] = 0;  // enums/sets
    // slots:
    m_data[10] = 10;  //signature start
    m_data[11] = 10 + signatureSize;  //parameters start
    m_data[12] = 10 + signatureSize;  //type start
    m_data[13] = 10 + signatureSize;  //tag start
    m_data[14] = 0x0a;//flags
    m_data[15] = 0;    // eod

    m_stringData = QByteArray("SlotProxy\0", 10);
    m_stringData += m_signature;
    m_stringData += QByteArray("\0\0", 2);

    staticMetaObject.d.superdata = &QObject::staticMetaObject;
    staticMetaObject.d.stringdata = m_stringData.data();
    staticMetaObject.d.data = m_data;
    staticMetaObject.d.extradata = 0;
}

SlotProxy::~SlotProxy()
{
}

const QMetaObject *SlotProxy::metaObject() const
{
    return &staticMetaObject;
}

void *SlotProxy::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, m_stringData))
        return static_cast<void*>(const_cast<SlotProxy*>(this));
    return QObject::qt_metacast(_clname);
}

KJS::JSValue *SlotProxy::callMethod( const QByteArray & methodName, void **_a )
{
    KJS::ExecState *exec = m_interpreter->globalExec();
    // Crash
    // KJS::Interpreter::globalExec()->context().thisValue()
    KJS::List args = convertArguments(exec, _a);
    KJS::Identifier id = KJS::Identifier( KJS::UString(methodName.data()));
    KJS::JSObject *fun = m_object->get(exec, id )->toObject( exec );
    if ( !fun->implementsCall() )
    {
    // We need to create an exception here...
    }

    KJS::JSValue *retValue = fun->call(exec, m_object, args);

    if( exec->hadException() )
    {
    //TODO ext the script here with the error
      m_interpreter->deref();
      return KJS::Null();
    }
    else
    {
        if( retValue->type() == 1 || retValue->type() == 0)
        {
            return KJS::Null();
        }
    }
    return retValue;
}

KJS::List SlotProxy::convertArguments(KJS::ExecState *exec, void **_a )
{
    KJS::List args;
    int offset = metaObject()->indexOfMethod(m_signature);
    QMetaMethod method = metaObject()->method(offset);
    QList<QByteArray> params = method.parameterTypes();
    int idx = 1;
    foreach( QByteArray param, params )
    {
        // int type = QMetaType::type( param.constData() );
        //qDebug("Got a %d - %s", type, param.data());

        int tp = QVariant::nameToType(param.constData());
        //qDebug("Try to convert a %d", tp);
        if(tp != QVariant::Invalid)
        {
            QVariant variant(tp, _a[idx]);
            args.append( KJSEmbed::convertToValue(exec,variant) );
        }
        else
        {
            qDebug("not supported yet");
        }
        ++idx;
    }

    return args;
}

int SlotProxy::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
            return _id;
    if (_c == QMetaObject::InvokeMetaMethod)
    {
        switch (_id)
        {
            case 0:
                // invoke js method here
                QByteArray method = m_signature.left(m_signature.indexOf('('));
                callMethod(method, _a);
                break;
        }
        _id -= 1;
    }
    return _id;
}
