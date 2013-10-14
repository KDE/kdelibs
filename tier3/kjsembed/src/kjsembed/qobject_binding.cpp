/* This file is part of the KDE libraries
    Copyright (C) 2005, 2006 Ian Reinhart Geiser <geiseri@kde.org>
    Copyright (C) 2005, 2006 Matt Broadstone <mbroadst@gmail.com>
    Copyright (C) 2005, 2006 Richard J. Moore <rich@kde.org>
    Copyright (C) 2005, 2006 Erik L. Bunce <kde@bunce.us>
    Copyright (C) 2007, 2008 Sebastian Sauer <mail@dipe.org>

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
#include "qobject_binding.h"

#include <QtCore/QObject>
#include <QtCore/QArgument>
#include <QtCore/QMetaEnum>
#include <QtCore/QMetaType>
#include <QtCore/QVariant>
#include <QtCore/QVector>
#include <QtCore/QUrl>
#include <QtCore/QDebug>
#include <QWidget>

#include "slotproxy.h"
#include "eventproxy.h"
#include "jseventmapper.h"
#include "pointer.h"
#include "variant_binding.h"

#include <kjs/array_instance.h>
#include <kjs/function_object.h>

//#define CREATEQOBJ_DIAG

using namespace KJSEmbed;

QByteArray createSignal( const QByteArray &sig )
{
    return '2' + sig;
}

QByteArray createSlot( const QByteArray &slt )
{
    return '1' + slt;
}

bool validSlot(const QMetaMethod& method, QObjectBinding::AccessFlags accessflags)
{
    switch( method.access() ) {
        case QMetaMethod::Private: {
            if(! (accessflags & QObjectBinding::PrivateSlots)) return false;
        } break;
        case QMetaMethod::Protected: {
            if(! (accessflags & QObjectBinding::ProtectedSlots)) return false;
        } break;
        case QMetaMethod::Public: {
            if(! (accessflags & QObjectBinding::PublicSlots)) return false;
        } break;
    }
    if(method.attributes() & QMetaMethod::Scriptable) {
        if(! (accessflags & QObjectBinding::ScriptableSlots)) return false;
    }
    else {
        if(! (accessflags & QObjectBinding::NonScriptableSlots)) return false;
    }
    return true;
}

bool validSignal(const QMetaMethod& method, QObjectBinding::AccessFlags accessflags)
{
    switch( method.access() ) {
        case QMetaMethod::Private: {
            if(! (accessflags & QObjectBinding::PrivateSignals)) return false;
        } break;
        case QMetaMethod::Protected: {
            if(! (accessflags & QObjectBinding::ProtectedSignals)) return false;
        } break;
        case QMetaMethod::Public: {
            if(! (accessflags & QObjectBinding::PublicSignals)) return false;
        } break;
    }
    if(method.attributes() & QMetaMethod::Scriptable) {
        if(! (accessflags & QObjectBinding::ScriptableSignals)) return false;
    }
    else {
        if(! (accessflags & QObjectBinding::NonScriptableSignals)) return false;
    }
    return true;
}

bool validProperty(const QMetaProperty& property, QObjectBinding::AccessFlags accessflags)
{
    if(property.isScriptable()) {
        if(! (accessflags & QObjectBinding::ScriptableProperties)) return false;
    }
    else {
        if(! (accessflags & QObjectBinding::NonScriptableProperties)) return false;
    }
    return true;
}

KJS::JSValue *callConnect( KJS::ExecState *exec, KJS::JSObject *self, const KJS::List &args )
{
    KJSEmbed::QObjectBinding *imp = KJSEmbed::extractBindingImp<KJSEmbed::QObjectBinding>(exec,  self );
    if( !imp ) // No implementation, so we need to use the first argument as we are a global static invocation.
        imp = KJSEmbed::extractBindingImp<KJSEmbed::QObjectBinding>(exec, args[0] );
    if( !imp )
        return KJS::throwError(exec, KJS::GeneralError, i18n("Wrong object type."));
        //return KJSEmbed::throwError(exec, i18n("Wrong object type."));

    if( args.size() > 2)
    {
        KJSEmbed::QObjectBinding *senderImp = KJSEmbed::extractBindingImp<KJSEmbed::QObjectBinding>(exec, args[0] );
        if( !senderImp )
        {
            return KJS::throwError(exec, KJS::GeneralError, i18n("First argument must be a QObject."));
            //return KJSEmbed::throwError(exec, i18n("First argument must be a QObject"));
        }
        QObject* receiver = 0;
        QObject* sender = senderImp->object<QObject>();
        QByteArray signal = createSignal(args[1]->toString(exec).ascii());
        QByteArray slot;
        KJSEmbed::QObjectBinding *receiverImp = 0;
        if( args.size() >= 4)
        {
            slot = createSlot(args[3]->toString(exec).ascii());
            receiverImp = KJSEmbed::extractBindingImp<KJSEmbed::QObjectBinding>(exec, args[2] );
            if( !receiverImp )
                receiver = new SlotProxy(args[2]->toObject(exec), exec->dynamicInterpreter(), sender, args[3]->toString(exec).ascii() );
            else
                receiver = receiverImp->object<QObject>();
        }
        else
        {
            receiverImp = imp;
            receiver = imp->object<QObject>();
            slot = createSlot(args[2]->toString(exec).ascii());
        }

        const QMetaObject *senderMetaObject = sender->metaObject();
        QMetaMethod senderMetaMethod = senderMetaObject->method( senderMetaObject->indexOfSignal(signal.constData()) );

        const QMetaObject *receiverMetaObject = receiver->metaObject();
        QMetaMethod receiverMetaMethod = receiverMetaObject->method( receiverMetaObject->indexOfSlot(slot.constData()) );

        if( validSignal(senderMetaMethod, senderImp->access()) && ( !receiverImp || validSlot(receiverMetaMethod, receiverImp->access()) ) )
        {
            return KJS::jsBoolean(QObject::connect(sender, signal.constData(), receiver, slot.constData()));
        }

        return KJS::jsBoolean(false);
    }
    return KJS::throwError(exec, KJS::GeneralError, i18n("Incorrect number of arguments."));
    //return KJSEmbed::throwError(exec, i18n("Incorrect number of arguments."));
}

QByteArray extractMemberName( const QMetaMethod &member )
{
    QString sig = member.methodSignature();
    return sig.left( sig.indexOf('(') ).toLatin1();
}

void QObjectBinding::publishQObject( KJS::ExecState *exec, KJS::JSObject *target, QObject *object)
{
    KJSEmbed::QObjectBinding *imp = KJSEmbed::extractBindingImp<KJSEmbed::QObjectBinding>(exec,  target);
    Q_ASSERT(imp);

    // Add the children the QObject has.
    if (imp->access() & QObjectBinding::ChildObjects) {
        //TODO uh, this one is dirty cause it may eat a lot of time to publish things that may not
        //got accessed anyway. Better solution would be to provide access to them on demand only. That
        //would also allow to manipulate the QObject-tree at runtime what is currently not possible.
        QObjectList children = object->children();
        QObjectList::Iterator child = children.begin();
        for( ; child != children.end(); ++child)
        {
            QString objectName = (*child)->objectName();
            if( !objectName.isEmpty() )
            {
                KJS::JSObject *childObject = KJSEmbed::createQObject(exec, *child);
                KJSEmbed::QObjectBinding *childImp = KJSEmbed::extractBindingImp<KJSEmbed::QObjectBinding>(exec, childObject);
                if(childImp)
                {
                    childImp->setAccess( imp->access() ); // inherit access from parent
                    target->put(exec, KJS::Identifier(toUString(objectName)), childObject);
                }
            }
        }
    }

    // Add slots of the current object.
    const QMetaObject *metaObject = object->metaObject();
    int methods = metaObject->methodCount();
    for( int idx = 0; idx < methods; ++idx )
    {
        QMetaMethod member = metaObject->method(idx);
        if(validSlot(member, imp->access()))
        {
            target->put(exec, KJS::Identifier(extractMemberName(member).constData()),
                        new SlotBinding(exec,member), KJS::DontDelete|KJS::ReadOnly|KJS::Function);
        }
    }

    // Add enums as read only uints.
    int enums = metaObject->enumeratorCount();
    for( int idx = 0; idx < enums; ++idx )
    {
        QMetaEnum enumerator = metaObject->enumerator(idx);
        int keys = enumerator.keyCount();
        for( int key = 0; key < keys; ++key)
        {
            target->put(exec, KJS::Identifier( enumerator.key(key) ),
                    KJS::jsNumber(enumerator.value(key)), KJS::DontDelete|KJS::ReadOnly);
        }
    }
}

QObjectBinding::QObjectBinding( KJS::ExecState *exec, QObject *object )
    : ObjectBinding(exec, object->metaObject()->className(), object)
    , m_evproxy(0)
    , m_access( AllSlots | AllSignals | AllProperties | AllObjects )
{
    if( object->parent() != 0 )
    {
        setOwnership( ObjectBinding::QObjOwned );
    }
    else
    {
        setOwnership( ObjectBinding::JSOwned );
    }

    m_cleanupHandler = new QObjectCleanupHandler();
    watchObject(object);

    StaticBinding::publish( exec, this, QObjectFactory::methods() );
    QObjectBinding::publishQObject(exec, this, object);

    // Make "connect" a global static method.
    exec->dynamicInterpreter()->globalObject()->put(exec, "connect", new StaticBinding(exec,  &QObjectFactory::methods()[0]) );
}

QObjectBinding::~QObjectBinding()
{
    if( m_cleanupHandler->isEmpty() )
    {
        setOwnership( ObjectBinding::QObjOwned );
    }
    else if( object<QObject>()->parent() != 0 )
    {
        setOwnership( ObjectBinding::QObjOwned );
        m_cleanupHandler->remove(object<QObject>());
    }
    else if( ownership() != ObjectBinding::JSOwned )
    {
        m_cleanupHandler->remove(object<QObject>());
    }
    else
    {
        m_cleanupHandler->remove(object<QObject>());
    }

    delete m_cleanupHandler;
}

void QObjectBinding::watchObject( QObject *object )
{
    m_cleanupHandler->add( object );
}

bool QObjectBinding::getOwnPropertySlot( KJS::ExecState *exec, const KJS::Identifier &propertyName, KJS::PropertySlot &slot )
{
    //    qDebug() << "getOwnPropertySlot called";
    QObject *obj = object<QObject>();
    const QMetaObject *meta = obj->metaObject();
    int propIndex = meta->indexOfProperty( propertyName.ascii() );
    if ( propIndex != -1 ) {
        if(! validProperty(meta->property(propIndex), m_access))
            return false;
        // qDebug() << "getOwnPropertySlot found the property " << propertyName.ascii();
        slot.setCustom( this, propertyGetter );
        return true;
    }
    return ObjectBinding::getOwnPropertySlot( exec, propertyName, slot );
}

KJS::JSValue *QObjectBinding::propertyGetter( KJS::ExecState *exec, KJS::JSObject*,
                                              const KJS::Identifier &propertyName, const KJS::PropertySlot&slot )
{
    // qDebug() << "Getter was called";
    QObjectBinding *self = static_cast<QObjectBinding *>(slot.slotBase());
    QObject *obj =  self->object<QObject>();

    QVariant val = obj->property( propertyName.ascii() );
    if ( val.isValid() ) {
        return convertToValue( exec, val );
    }
    qDebug() << QString("propertyGetter called but no property, name was '%1'").arg(propertyName.ascii());
    return 0; // ERROR
}

QObjectBinding::AccessFlags QObjectBinding::access() const
{
    return m_access;
}

void QObjectBinding::setAccess(QObjectBinding::AccessFlags access)
{
    m_access = access;
}

void QObjectBinding::put(KJS::ExecState *exec, const KJS::Identifier &propertyName, KJS::JSValue *value, int attr )
{
    QObject *obj = object<QObject>();
    if ( obj && !m_cleanupHandler->isEmpty() )
    {
        // Properties
        const QMetaObject *meta = obj->metaObject();

        if ( int propIndex = meta->indexOfProperty( propertyName.ascii() ) != -1 )
        {
            QMetaProperty prop = meta->property(propIndex);
            if(! validProperty(prop, m_access))
                return;

            bool propSet = false;
            QVariant val = convertToVariant( exec, value );
            if ( prop.isEnumType() )
            {
                    propSet = obj->setProperty( propertyName.ascii(), val.toUInt() );
            }
            else if ( val.isValid() /*&& meta->property(propIndex).isWritable() <- wtf?*/ )
            {
                    propSet = obj->setProperty( propertyName.ascii(), val );
            }
            /*
            if( !propSet )
            {
                    KJSEmbed::throwError(exec,
                            i18n("Setting property %1 failed: property invalid, read-only or does not exist").arg(propertyName.ascii()));
            }
            */

        }

        if (JSEventMapper::mapper()->isEventHandler(propertyName) )
        {
            if ( !m_evproxy )
                m_evproxy = new KJSEmbed::EventProxy( this, exec->dynamicInterpreter() );
            if( value )
                m_evproxy->addFilter( JSEventMapper::mapper()->findEventType( propertyName ) );
            else
                m_evproxy->removeFilter( JSEventMapper::mapper()->findEventType( propertyName ) );
        }
    }
    //qDebug() << "Forward put";
    // Set a property value
    ObjectBinding::put(exec, propertyName, value, attr);
}

bool QObjectBinding::canPut(KJS::ExecState *exec, const KJS::Identifier &propertyName) const
{
    QObject *obj = object<QObject>();
    if ( obj && !m_cleanupHandler->isEmpty() )
    {
        // Properties
        const QMetaObject *meta = obj->metaObject();
        if ( int propIndex = meta->indexOfProperty( propertyName.ascii() ) != -1 )
        {
            QMetaProperty prop = meta->property(propIndex);
            return validProperty(prop, m_access) && prop.isWritable();
        }
    }
    return ObjectBinding::canPut(exec,propertyName);
}

KJS::UString QObjectBinding::className() const
{
    return toUString( typeName() );
}

KJS::UString QObjectBinding::toString(KJS::ExecState *exec) const
{
    Q_UNUSED( exec );
    QString s( "%1 (%2)" );
    s = s.arg( object<QObject>()->objectName() );
    s = s.arg( typeName() );
    return toUString( s );
}

PointerBase *getArg( KJS::ExecState *exec, const QList<QByteArray> &types, const KJS::List &args, int idx, QString& errorText)
{
    //qDebug("Index %d, args size %d, types size %d", idx, args.size(), types.size() );

    if( types.size() == 0 && idx == 0 )
        return new NullPtr();
    if ( args.size() <= idx )
        return new NullPtr();

    if ( types.size() <= idx )
    {
        const QString firstPart = i18np("The slot asked for %1 argument", "The slot asked for %1 arguments", idx);
        const QString secondPart = i18np("but there is only %1 available", "but there are only %1 available", types.size());
        errorText = i18nc("%1 is 'the slot asked for foo arguments', %2 is 'but there are only bar available'", "%1, %2.");

        return 0;
    }

    QVariant::Type varianttype = QVariant::nameToType( types[idx].constData() );
    //qDebug( QString("type=%1 argtype=%2 variantType=%3 (%4)").arg(types[idx].constData()).arg(args[idx]->type()).arg(varianttype).arg(QVariant::typeToName(varianttype)).toLatin1() );
    switch( varianttype ) {
        case QVariant::Int:
            if( args[idx]->type() == KJS::NumberType )
                return new Value<int>( int( args[idx]->toInteger(exec) ) );
            break;
        case QVariant::UInt:
            if( args[idx]->type() == KJS::NumberType )
                return new Value<uint>( uint( args[idx]->toInteger(exec) ) );
            break;
        case QVariant::LongLong:
            if( args[idx]->type() == KJS::NumberType )
                return new Value<qlonglong>( qlonglong( args[idx]->toInteger(exec) ) );
            break;
        case QVariant::ULongLong:
            if( args[idx]->type() == KJS::NumberType )
                return new Value<qulonglong>( qulonglong( args[idx]->toInteger(exec) ) );
            break;
        case QVariant::Double:
            if( args[idx]->type() == KJS::NumberType )
                return new Value<double>( args[idx]->toNumber(exec) );
            //if ( types[idx] == "float" ) return new Value<float>( args[idx]->toNumber(exec) );
            //if ( types[idx] == "qreal" ) return new Value<qreal>( args[idx]->toNumber(exec) );
            break;
        case QVariant::Bool:
            if( args[idx]->type() == KJS::BooleanType )
                return new Value<bool>( args[idx]->toBoolean(exec) );
            break;
        case QVariant::ByteArray:
            if( args[idx]->type() == KJS::StringType )
                return new Value<QByteArray>( toQString(args[idx]->toString(exec)).toUtf8() );
            break;
        case QVariant::String:
            if( args[idx]->type() == KJS::StringType )
                return new Value<QString>( toQString(args[idx]->toString(exec)) );
            break;
        case QVariant::StringList:
            if( args[idx]->type() == KJS::ObjectType )
                return new Value<QStringList>( convertArrayToStringList(exec, args[idx]) );
            break;
        case QVariant::Size:
            if( VariantBinding *valImp = KJSEmbed::extractBindingImp<VariantBinding>(exec,args[idx]) )
                return new Value<QSize>( valImp->variant().value<QSize>() );
            break;
        case QVariant::SizeF:
            if( VariantBinding *valImp = KJSEmbed::extractBindingImp<VariantBinding>(exec,args[idx]) )
                return new Value<QSizeF>( valImp->variant().value<QSizeF>() );
            break;
        case QVariant::Point:
            if( VariantBinding *valImp = KJSEmbed::extractBindingImp<VariantBinding>(exec,args[idx]) )
                return new Value<QPoint>( valImp->variant().value<QPoint>() );
            break;
        case QVariant::PointF:
            if( VariantBinding *valImp = KJSEmbed::extractBindingImp<VariantBinding>(exec,args[idx]) )
                return new Value<QPointF>( valImp->variant().value<QPointF>() );
            break;
        case QVariant::Rect:
            if( VariantBinding *valImp = KJSEmbed::extractBindingImp<VariantBinding>(exec,args[idx]) )
                return new Value<QRect>( valImp->variant().value<QRect>() );
            break;
        case QVariant::RectF:
            if( VariantBinding *valImp = KJSEmbed::extractBindingImp<VariantBinding>(exec,args[idx]) )
                return new Value<QRectF>( valImp->variant().value<QRectF>() );
            break;
        case QVariant::Color:
            if( args[idx]->type() == KJS::StringType )
                return new Value<QColor>( QColor(toQString(args[idx]->toString(exec))) );
            if( VariantBinding *valImp = KJSEmbed::extractBindingImp<VariantBinding>(exec,args[idx]) )
                return new Value<QColor>( valImp->variant().value<QColor>() );
            break;
        case QVariant::Url:
            if( args[idx]->type() == KJS::StringType )
                return new Value<QUrl>( QUrl(toQString(args[idx]->toString(exec)) ));
            if( VariantBinding *valImp = KJSEmbed::extractBindingImp<VariantBinding>(exec,args[idx]) )
                return new Value<QUrl>( valImp->variant().value<QUrl>() );
            break;
        case QVariant::List:
            if( args[idx]->type() == KJS::ObjectType )
                return new Value<QVariantList>( convertArrayToList(exec, args[idx]) );
            break;
        case QVariant::Map:
            if( args[idx]->type() == KJS::ObjectType )
                return new Value<QVariantMap>( convertArrayToMap(exec, args[idx]) );
            break;
        case QVariant::UserType: // fall through
        default:
            if( args[idx]->type() == KJS::NullType )
                return new NullPtr();
            if( args[idx]->type() == KJS::StringType )
            {
                if( strcmp(types[idx].constData(),"KUrl") == 0 ) //downcast to QUrl
                    return new Value<QUrl>(QUrl(toQString(args[idx]->toString(exec))));
            }
            if( args[idx]->type() == KJS::ObjectType )
            {
                if(QObjectBinding *objImp = KJSEmbed::extractBindingImp<QObjectBinding>(exec, args[idx]))
                {
                    //qDebug("\tQObjectBinding");
                    if( QObject* qObj = objImp->qobject<QObject>() )
                        return new Value<void*>(qObj);
                }
                else if(ObjectBinding *objImp = KJSEmbed::extractBindingImp<ObjectBinding>(exec, args[idx]))
                {
                    //qDebug("\tObjectBinding");
                    return new Value<void*>(objImp->voidStar());
                }
                if(VariantBinding *valImp = KJSEmbed::extractBindingImp<VariantBinding>(exec,args[idx]))
                {
                    //qDebug() << "\tVariantBinding typeName="  << valImp->variant().typeName() << "type="  << valImp->variant().type() << "userType="  << valImp->variant().userType() << " variant=" << valImp->variant();
                    QVariant var = valImp->variant();

                    // if the variant is the appropriate type, return its data
                    if ((var.type() == varianttype) ||
                        ((var.type() == QVariant::UserType) &&
                         (types[idx].constData() == var.typeName())))
                        return new Value<void*>(valImp->variant().data());
                    else if ((var.type() != QVariant::UserType) &&
                           var.canConvert(varianttype))
                    {
                        // is convertable type, so convert it, and return if successful
                        if (var.convert(varianttype))
                            return new Value<void*>(valImp->variant().data());
                    }
                    else if ((var.type() == QVariant::UserType) &&
                             var.canConvert<QObject*>())
                    {
                        QObject* qObj = var.value<QObject*>();
                        if (!qObj)
                            qObj = reinterpret_cast<QObject*>(var.value<QWidget*>());
                        if (qObj) {
                            QByteArray typeName = types[idx].constData();
                            typeName.replace("*", ""); //krazy:exclude=doublequote_chars
                            if (qObj->inherits(typeName.constData()))
                                return new Value<void*>(qObj);
                        }
                    }
                }
            }

            QVariant v = KJSEmbed::extractVariant(exec, args[idx]);
            if (! v.isNull())
                return new Value<QVariant>(v);

            break;
    }

    qDebug("Cast failure %s value Type %d", types[idx].constData(), args[idx]->type() );
    // construct a meaningful exception message
    QString jsType;
    KJS::JSObject* jsObj = args[idx]->getObject();
    if (jsObj)
    {
        const KJS::ClassInfo* ci = jsObj->classInfo();
        if (ci && ci->className)
            jsType = ci->className;
        if (jsType.isEmpty())
            jsType = toQString(jsObj->className());
    }

    if (jsType.isEmpty())
    {
        switch(args[idx]->type())
        {
        case KJS::UnspecifiedType:
            jsType = "jsUnspecified";
            break;
        case KJS::NumberType:
            jsType = "jsNumber";
            break;
        case KJS::BooleanType:
            jsType = "jsBoolean";
            break;
        case KJS::UndefinedType:
            jsType = "jsUndefined";
            break;
        case KJS::NullType:
            jsType = "jsNull";
            break;
        case KJS::StringType:
            jsType = "jsString";
            break;
        case KJS::ObjectType:
            jsType = "jsObject";
            break;
        case KJS::GetterSetterType:
            jsType = "jsGetterSetter";
            break;
        default:
            jsType = QString::number(args[idx]->type());
            break;
        }
    }

    errorText = i18n("Failure to cast to %1 value from Type %2 (%3)",
                     types[idx].constData(), jsType, toQString(args[idx]->toString(exec)));

    return 0;
}

KJS::JSValue *SlotBinding::callAsFunction( KJS::ExecState *exec, KJS::JSObject *self, const KJS::List &args )
{
    QObjectBinding *imp = extractBindingImp<QObjectBinding>(exec,self);
    if( imp == 0 )
        return KJS::jsNull();

    PointerBase *qtArgs[10];
    void *param[11];

    QObject *object = imp->object<QObject>();
    int count = object->metaObject()->methodCount();
    QMetaMethod metaMember;
    int offset = 0;
    bool success = false;
    for(; offset < count; ++offset)
    {
        metaMember = object->metaObject()->method(offset);
        if( extractMemberName(metaMember) == m_memberName )
        {
            if( metaMember.parameterTypes().size() == args.size() && validSlot(metaMember, imp->access()) )
            {
                success = true;
                break;
            }
        }
    }

    if( !success )
    {
        return KJS::throwError(exec, KJS::GeneralError, i18n("No such method '%1'.",  m_memberName.constData()));
        //return KJSEmbed::throwError(exec, i18n("Call to '%1' failed.").arg(m_memberName.constData()));
    }

    QList<QByteArray> types = metaMember.parameterTypes();

    QVariant::Type returnTypeId = QVariant::nameToType( metaMember.typeName() );
    int tp = QMetaType::type( metaMember.typeName() );
    PointerBase *qtRet = new Value<void*>(0);

    bool returnIsMetaType = (
        returnTypeId == QVariant::UserType ||
        returnTypeId == QVariant::Size     || returnTypeId == QVariant::SizeF  ||
        returnTypeId == QVariant::Point    || returnTypeId == QVariant::PointF ||
        returnTypeId == QVariant::Rect     || returnTypeId == QVariant::RectF  ||
        returnTypeId == QVariant::Color
    );
    QVariant returnValue = returnIsMetaType ? QVariant(tp, (void*)0) : QVariant(returnTypeId);
    QGenericReturnArgument returnArgument(metaMember.typeName(), &returnValue);
    param[0] = returnIsMetaType ? qtRet->voidStar() : returnArgument.data();

    QString errorText;
    for( int idx = 0; idx < 10; ++idx)
    {
        qtArgs[idx] = getArg(exec, types, args, idx, errorText);
        if (!qtArgs[idx]) {
            for( int i = 0; i < idx; ++i)
                delete qtArgs[i];
            delete qtRet;
            return KJS::throwError(exec, KJS::GeneralError, i18n("Call to method '%1' failed, unable to get argument %2: %3",  m_memberName.constData(), idx, errorText));
        }
        param[idx+1] = qtArgs[idx]->voidStar();
    }

    success = object->qt_metacall(QMetaObject::InvokeMetaMethod, offset, param) < 0;

    KJS::JSValue *jsReturnValue = 0;
    if( success ) {
        switch( returnTypeId ) {
            case QVariant::Invalid: // fall through
            case QVariant::UserType: {
                if( QMetaType::typeFlags(tp) & QMetaType::PointerToQObject ) {
                    const QVariant v(tp, param[0]);
                    QObject *obj = v.value< QObject* >();
                    if( obj )
                        jsReturnValue = KJSEmbed::createQObject(exec, obj, KJSEmbed::ObjectBinding::CPPOwned);
                }
            } break;
            default:
                if( returnIsMetaType )
                    returnValue = QVariant(tp, param[0]);
                break;
        }
        if(! jsReturnValue)
            jsReturnValue = KJSEmbed::convertToValue(exec, returnValue);
    }

    for( int idx = 0; idx < 10; ++idx)
        delete qtArgs[idx];
    delete qtRet;

    if( !success )
        return KJS::throwError(exec, KJS::GeneralError, i18n("Call to '%1' failed.",  m_memberName.constData()));

    return jsReturnValue;
}

SlotBinding::SlotBinding(KJS::ExecState *exec, const QMetaMethod &member )
  : KJS::InternalFunctionImp(static_cast<KJS::FunctionPrototype*>(exec->lexicalInterpreter()->builtinFunctionPrototype()),
                             KJS::Identifier(toUString(extractMemberName(member))))
{
    m_memberName = extractMemberName(member);
    int count = member.parameterNames().count();
    putDirect( exec->propertyNames().length, count, LengthFlags );
}


KJS::JSObject* KJSEmbed::createQObject(KJS::ExecState *exec, QObject *value, KJSEmbed::ObjectBinding::Ownership owner)
{
    if ( 0 == value )
        return new KJS::JSObject();

    const QMetaObject *meta = value->metaObject();
    KJS::JSObject *parent = exec->dynamicInterpreter()->globalObject();
    KJS::JSObject *returnValue;
    int pos;
    QString clazz;
    do
    {
        clazz = meta->className();

#ifdef CREATEQOBJ_DIAG
        qDebug() << "clazz=" << clazz;
#endif
        // strip off namespace since they aren't included
        if ((pos = clazz.lastIndexOf("::")) != -1)
            clazz.remove(0, pos + 2);
#ifdef CREATEQOBJ_DIAG
        qDebug() << "cleaned clazz=" << clazz;
#endif
        if ( parent->hasProperty( exec, KJS::Identifier(toUString(clazz)) ) )
        {
#ifdef CREATEQOBJ_DIAG
            qDebug() << "createQObject(): clazz=" << clazz << " value=" << value;
#endif
            Pointer<QObject> pov(value);
            returnValue = StaticConstructor::bind(exec, clazz, pov);
            if ( returnValue )
              return returnValue;

#ifdef CREATEQOBJ_DIAG
            qDebug("\tresort to construct() method.");
#endif
            returnValue = StaticConstructor::construct( exec, parent, toUString(clazz) );
            if( returnValue )
            {
                // If it is a value type setValue
                KJSEmbed::QObjectBinding *imp = extractBindingImp<QObjectBinding>(exec, returnValue );
                if( imp )
                {
                    imp->setObject( value );
                    imp->watchObject( value );
                    imp->setOwnership( owner );
                    KJSEmbed::QObjectBinding::publishQObject( exec, returnValue, value);
                }
                else
                {
                    KJS::throwError(exec, KJS::TypeError, i18n("%1 is not an Object type",  clazz ));
                    return new KJS::JSObject();
                }
            }
            else
            {
                KJS::throwError(exec, KJS::TypeError, i18n("Could not construct value"));
                return new KJS::JSObject();
            }
            return returnValue;
        }
        else
        {
#ifdef CREATEQOBJ_DIAG
            qDebug("%s not a bound type, move up the chain", meta->className() );
#endif
            meta = meta->superClass();
        }

    }
    while( meta );

    KJSEmbed::QObjectBinding *imp = new KJSEmbed::QObjectBinding(exec, value);
    imp->setOwnership( owner );

    return imp;
}

START_QOBJECT_METHOD( callParent, QObject )
    //TODO it would be better, if each QObjectBinding remembers it's parent rather then
    //creating a new instance each time. That wouldn't only be more logical, but also
    //does prevent losing of additional information like e.g. the access-level.
    if( imp->access() & QObjectBinding::GetParentObject )
    {
        QObject *parent = imp->object<QObject>()->parent();
        KJS::JSObject *parentObject = KJSEmbed::createQObject(exec, parent);
        KJSEmbed::QObjectBinding *parentImp = KJSEmbed::extractBindingImp<KJSEmbed::QObjectBinding>(exec, parentObject);
        if( parentImp ) {
            parentImp->setAccess( imp->access() ); // inherit access from child since we don't know the access-level of the parent here :-(
        }
        result = parentObject;
    }
END_QOBJECT_METHOD
START_QOBJECT_METHOD( callIsWidgetType, QObject )
    result = KJS::jsBoolean(object->isWidgetType());
END_QOBJECT_METHOD
START_QOBJECT_METHOD( callInherits, QObject)
    QByteArray className = KJSEmbed::extractQString(exec, args, 0).toLatin1();
    result = KJS::jsBoolean(object->inherits(className.constData()));
END_QOBJECT_METHOD
START_QOBJECT_METHOD( callSetParent, QObject )
    if( imp->access() & QObjectBinding::SetParentObject )
    {
        QObject *parent = KJSEmbed::extractObject<QObject>(exec, args, 0, 0);
        object->setParent(parent);
    }
END_QOBJECT_METHOD
START_QOBJECT_METHOD( callFindChild, QObject )
    if( imp->access() & QObjectBinding::ChildObjects )
    {
        QString childName = KJSEmbed::extractQString(exec, args, 0);
        QObject *child = object->findChild<QObject*>(childName);
        KJS::JSObject *childObject = KJSEmbed::createQObject(exec, child);
        KJSEmbed::QObjectBinding *childImp = KJSEmbed::extractBindingImp<KJSEmbed::QObjectBinding>(exec, childObject);
        if( childImp ) {
            childImp->setAccess( imp->access() ); // inherit access from parent
        }
        result = childObject;
    }
END_QOBJECT_METHOD

START_METHOD_LUT(QObjectFactory)
    {"connect", 4, KJS::DontDelete|KJS::ReadOnly, &callConnect },
    {"parent", 0, KJS::DontDelete|KJS::ReadOnly, &callParent },
    {"inherits", 1, KJS::DontDelete|KJS::ReadOnly, &callInherits },
    {"isWidgetType", 0, KJS::DontDelete|KJS::ReadOnly, &callIsWidgetType },
    {"setParent", 1, KJS::DontDelete|KJS::ReadOnly, &callSetParent },
    {"findChild", 1, KJS::DontDelete|KJS::ReadOnly, &callFindChild }
END_METHOD_LUT

NO_ENUMS( QObjectFactory )
NO_STATICS( QObjectFactory )

//kate: indent-spaces on; indent-width 4; replace-tabs on; indent-mode cstyle;
