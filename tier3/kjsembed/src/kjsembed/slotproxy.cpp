/* This file is part of the KDE libraries
    Copyright (C) 2005, 2006 Ian Reinhart Geiser <geiseri@kde.org>
    Copyright (C) 2005, 2006 Matt Broadstone <mbroadst@gmail.com>
    Copyright (C) 2005, 2006 Richard J. Moore <rich@kde.org>
    Copyright (C) 2005, 2006 Erik L. Bunce <kde@bunce.us>

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
#include "slotproxy.h"

#include <QtCore/QMetaEnum>
#include <QtCore/QMetaType>
#include <QtCore/QDebug>
#include <QWidget>

#include <kjs/interpreter.h>

#include "variant_binding.h"
#include "qobject_binding.h"

//#define DEBUG_SLOTPROXY 1

using namespace KJSEmbed;

SlotProxy::SlotProxy(KJS::JSObject *obj, KJS::Interpreter *interpreter, QObject *parent, const QByteArray &signature)
    : QObject(parent), m_interpreter(interpreter), m_object(obj)
{
    m_signature = QMetaObject::normalizedSignature(signature.constData());
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
    // Q_SLOTS:
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
    staticMetaObject.d.stringdata = m_stringData.data_ptr();
    staticMetaObject.d.data = m_data;
    staticMetaObject.d.extradata = 0;
#ifdef DEBUG_SLOTPROXY
		qDebug() << "SlotProxy() obj=" << this <<  " m_signature=" << m_signature;
#endif
}

SlotProxy::~SlotProxy()
{
#ifdef DEBUG_SLOTPROXY
	qDebug() << "??????SlotProxy::~SlotProxy() obj=" << this <<" m_signature=" << m_signature;
#endif
}

const QMetaObject *SlotProxy::metaObject() const
{
    return &staticMetaObject;
}

void *SlotProxy::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, m_stringData.constData()))
        return static_cast<void*>(const_cast<SlotProxy*>(this));
    return QObject::qt_metacast(_clname);
}

KJS::JSValue *SlotProxy::callMethod( const QByteArray & methodName, void **_a )
{
#ifdef DEBUG_SLOTPROXY
	qDebug() << "SlotProxy::callMethod(" << methodName << ",_a) obj=" << this;
#endif
    KJS::ExecState *exec = m_interpreter->globalExec();
    exec->clearException();

    // Crash
    // KJS::Interpreter::globalExec()->context().thisValue()
    KJS::List args = convertArguments(exec, _a);
    KJS::Identifier id = KJS::Identifier( KJS::UString(methodName.data()));
    KJS::JSObject *fun = m_object->get(exec, id )->toObject( exec );
    KJS::JSValue *retValue;
    if ( !fun->implementsCall() )
    {
#ifdef DEBUG_SLOTPROXY
        qDebug() << "SlotProxy::callMethod got bad handler";
#endif
        QString msg = i18n( "Bad slot handler: Object %1 Identifier %2 Method %3 Signature: %4.",
                            m_object->className().ascii(),
                            id.ascii(),
                            methodName.data(),
                            QString(m_signature));

        retValue = throwError(exec, KJS::TypeError, msg);
    }
    else
        retValue = fun->call(exec, m_object, args);

    if( exec->hadException() )
    {
#ifdef DEBUG_SLOTPROXY
        qDebug() << "SlotProxy::callMethod had exception";
#endif
        if (m_interpreter->shouldPrintExceptions())
        {
            KJS::JSLock lock;
            KJS::JSObject* exceptObj = exec->exception()->toObject(exec);//retValue->toObject(exec);
            QString message = toQString(exceptObj->toString(exec));
            QString sourceURL = toQString(exceptObj->get(exec, "sourceURL")->toString(exec));
            int sourceId = exceptObj->get(exec, "sourceId")->toUInt32(exec);
            // would include the line number, but it's always last line of file
            int line = exceptObj->get(exec, "line")->toUInt32(exec);
            (*KJSEmbed::conerr()) << i18n("Exception calling '%1' slot from %2:%3:%4", QString(methodName), !sourceURL.isEmpty() ? sourceURL : QString::number(sourceId), line, message) << endl;
        }

        // clear it so it doesn't stop other things
        exec->clearException();

        return KJS::jsNull();
    }
    else
    {
        if( retValue->type() == 1 || retValue->type() == 0)
        {
            return KJS::jsNull();
        }
    }
    return retValue;
}

KJS::List SlotProxy::convertArguments(KJS::ExecState *exec, void **_a )
{
    KJS::List args;
    int offset = metaObject()->indexOfMethod(m_signature.constData());
    QMetaMethod method = metaObject()->method(offset);
    QList<QByteArray> params = method.parameterTypes();
    int idx = 1;
#ifdef DEBUG_SLOTPROXY
    qDebug() << "SlotProxy::convertArguments(): obj=" << this << " m_signature=" << m_signature << " offset=" << offset << " params=" << params ;
#endif
    foreach( const QByteArray &param, params )
    {
#ifdef DEBUG_SLOTPROXY
        int type = QMetaType::type( param.constData() );
        qDebug("\tGot a %d - %s - _a[%d] = %p", type, param.data(), idx, _a[idx]);
        qDebug("\t QMetaType::type()=%d", QMetaType::type(QByteArray("Pinya::") + param.constData()));
#endif
        int tp = QVariant::nameToType(param.constData());
        switch(tp)
        {
        case QVariant::Int:
            args.append(KJS::jsNumber(*(int*)_a[idx]));
            break;
        case QVariant::UInt:
            args.append(KJS::jsNumber(*(uint*)_a[idx]));
            break;
        case QVariant::LongLong:
            args.append(KJS::jsNumber(*(qlonglong*)_a[idx]));
            break;
        case QVariant::ULongLong:
            args.append(KJS::jsNumber(*(qulonglong*)_a[idx]));
            break;
        case QVariant::Double:
            args.append(KJS::jsNumber(*(double*)_a[idx]));
            break;
        case QVariant::Bool:
            args.append(KJS::jsBoolean(*(bool*)_a[idx]));
            break;
        case QVariant::String:
            args.append(KJS::jsString((*reinterpret_cast<QString(*)>(_a[idx]))));
            break;
        case QVariant::UserType:
        {
            KJS::JSObject* returnValue;
            KJS::JSObject* parent = exec->dynamicInterpreter()->globalObject();
            QByteArray typeName = param.constData();
            bool isPtr = typeName.contains("*");
            if (isPtr)
                typeName.replace("*", ""); //krazy:exclude=doublequote_chars
#ifdef DEBUG_SLOTPROXY
            qDebug() << "\tQVariant::UserType: typeName=" << typeName << " param=" << param.constData() << " isPtr" << isPtr;
#endif
            if ( parent->hasProperty( exec, KJS::Identifier(toUString(typeName))) )
            {
                QObject* qObj;
                if (isPtr &&
                    ((qObj = *reinterpret_cast<QObject**>(_a[idx])) != 0))
                {
#ifdef DEBUG_SLOTPROXY
                    qDebug() << "qObj=" << qObj;
#endif
                    Pointer<QObject> pov(*reinterpret_cast<QObject*(*)>(_a[idx]));
                    returnValue = StaticConstructor::bind(exec, typeName, pov);
                    if ( returnValue )
                    {
                        args.append(returnValue);
                        break;
                    }
                    else
                    {
#ifdef DEBUG_SLOTPROXY
                        qDebug("\t\tNo binding retrieved");
#endif
                        returnValue = StaticConstructor::construct( exec, parent, toUString(typeName) );
                        if( returnValue )
                        {
                            if(QObjectBinding *objImp = KJSEmbed::extractBindingImp<QObjectBinding>(exec, returnValue))
                            {
#ifdef DEBUG_SLOTPROXY
                                qDebug() << "\t\t\tFound QObjectBinding";
#endif
                                objImp->setOwnership( KJSEmbed::ObjectBinding::JSOwned );
                                objImp->setObject(qObj);
                                if (qObj->parent() != 0)
                                    objImp->setOwnership(KJSEmbed::ObjectBinding::QObjOwned);
                                else
                                    objImp->setOwnership(KJSEmbed::ObjectBinding::CPPOwned);
                                args.append(returnValue);
                                break;
                            }
                        }
                    }
                }
            }
            else
            {
#ifdef DEBUG_SLOTPROXY
                qDebug("\t\tNo binding registered");
#endif
                KJS::JSObject* returnValue = 0;
                const int metaTypeId = QMetaType::type( param.constData() );
                if( QMetaType::typeFlags(metaTypeId) & QMetaType::PointerToQObject ) {
                    QObject *obj = (*reinterpret_cast< QObject*(*)>( _a[idx] ));
                    returnValue = KJSEmbed::createQObject(exec, obj, KJSEmbed::ObjectBinding::QObjOwned);
                }
                if( returnValue ) {
                    args.append(returnValue);
                    break;
                }
            }
        }
        case QVariant::StringList:
        case QVariant::List:
        case QVariant::Map:
        default:
            //qDebug("\t\tconverting to variant");
            QVariant variant(tp, _a[idx]);
            args.append( KJSEmbed::convertToValue(exec,variant) );
            break;
        }
        ++idx;
    }

    return args;
}

int SlotProxy::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
#ifdef DEBUG_SLOTPROXY
	qDebug("SlotProxy::qt_metacall(_c=%d, _id=%d, _a=%p _a[0]=%p _a[1]=%p) obj=", _c, _id, _a, _a[0], _a[1], this);
#endif
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
            return _id;
    if (_c == QMetaObject::InvokeMetaMethod)
    {
        switch (_id)
        {
            case 0: {
                // invoke js method here
                QByteArray method = m_signature.left(m_signature.indexOf('('));
                KJS::JSValue *result = callMethod(method, _a);
                m_tmpResult = convertToVariant(m_interpreter->globalExec(), result);
#ifdef DEBUG_SLOTPROXY
                qDebug()<<"SlotProxy::qt_metacall result="<<m_tmpResult.toString();
#endif
                _a[0] = &(m_tmpResult);
            } break;
        }
        _id -= 1;
    }
    return _id;
}

//kate: indent-spaces on; indent-width 4; replace-tabs on; indent-mode cstyle;
