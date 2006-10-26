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
#include <QObject>
#include <QMetaObject>
#include <QMetaProperty>
#include <QMetaMethod>
#include <QVariant>
#include <QVector>
#include <QDebug>
#include <qobjectdefs.h>

#include "slotproxy.h"
#include "eventproxy.h"
#include "jseventmapper.h"
#include "pointer.h"
#include "qobject_binding.h"
#include "variant_binding.h"

#include <kjs/function_object.h>
#include <kjs/property_slot.h>

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
            if(! accessflags & QObjectBinding::PrivateSlots) return false;
        } break;
        case QMetaMethod::Protected: {
            if(! accessflags & QObjectBinding::ProtectedSlots) return false;
        } break;
        case QMetaMethod::Public: {
            if(! accessflags & QObjectBinding::PublicSlots) return false;
        } break;
    }
    if(method.attributes() & QMetaMethod::Scriptable) {
        if(! accessflags & QObjectBinding::ScriptableSlots) return false;
    }
    else {
        if(! accessflags & QObjectBinding::NonScriptableSlots) return false;
    }
    return true;
}

bool validSignal(const QMetaMethod& method, QObjectBinding::AccessFlags accessflags)
{
    switch( method.access() ) {
        case QMetaMethod::Private: {
            if(! accessflags & QObjectBinding::PrivateSignals) return false;
        } break;
        case QMetaMethod::Protected: {
            if(! accessflags & QObjectBinding::ProtectedSignals) return false;
        } break;
        case QMetaMethod::Public: {
            if(! accessflags & QObjectBinding::PublicSignals) return false;
        } break;
    }
    if(method.attributes() & QMetaMethod::Scriptable) {
        if(! accessflags & QObjectBinding::ScriptableSignals) return false;
    }
    else {
        if(! accessflags & QObjectBinding::NonScriptableSignals) return false;
    }
    return true;
}

bool validProperty(const QMetaProperty& property, QObjectBinding::AccessFlags accessflags)
{
    if(property.isScriptable()) {
        if(! accessflags & QObjectBinding::ScriptableProperties) return false;
    }
    else {
        if(! accessflags & QObjectBinding::NonScriptableProperties) return false;
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
        char *signal = qstrdup( createSignal(args[1]->toString(exec).ascii()).data() );
        char *slot = 0;
        KJSEmbed::QObjectBinding *receiverImp = 0;
        if( args.size() >= 4)
        {
            slot = qstrdup( createSlot(args[3]->toString(exec).ascii()).data() );
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
            slot = qstrdup( createSlot(args[2]->toString(exec).ascii()).data() );
        }

        const QMetaObject *senderMetaObject = sender->metaObject();
        QMetaMethod senderMetaMethod = senderMetaObject->method( senderMetaObject->indexOfSignal(signal) );

        const QMetaObject *receiverMetaObject = receiver->metaObject();
        QMetaMethod receiverMetaMethod = receiverMetaObject->method( receiverMetaObject->indexOfSlot(slot) );

        if( validSignal(senderMetaMethod, senderImp->access()) && ( !receiverImp || validSlot(receiverMetaMethod, receiverImp->access()) ) )
        {
            return KJS::Boolean(QObject::connect(sender, signal, receiver, slot));
        }

        return KJS::Boolean(false);
    }
    return KJS::throwError(exec, KJS::GeneralError, i18n("Incorrect number of arguments."));
    //return KJSEmbed::throwError(exec, i18n("Incorrect number of arguments."));
}

QByteArray extractMemberName( const QMetaMethod &member )
{
    QString sig = member.signature();
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
                    target->put(exec, KJS::Identifier(objectName), childObject);
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
            target->put(exec, KJS::Identifier( extractMemberName( member ) ),
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
                    KJS::Number(enumerator.value(key)), KJS::DontDelete|KJS::ReadOnly);
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
    return KJS::UString( typeName() );
}

KJS::UString QObjectBinding::toString(KJS::ExecState *exec) const
{
    Q_UNUSED( exec );
    QString s( "%1 (%2)" );
    s = s.arg( object<QObject>()->objectName() );
    s = s.arg( typeName() );
    return KJS::UString( s );
}

PointerBase *getArg( KJS::ExecState *exec, const QList<QByteArray> &types, const KJS::List &args, int idx)
{
    //qDebug("Index %d, args size %d, types size %d", idx, args.size(), types.size() );
    if ( args.size() <= idx )
        return new NullPtr();

    if( types.size() == 0 && idx == 0 )
        return new NullPtr();

    if ( types.size() <= idx )
    {
        KJS::throwError(exec, KJS::GeneralError, i18n("The slot sked for %1 arguments but there are only %2 arguments available.",
                        idx,
                        types.size()));
        //KJSEmbed::throwError(exec,
        //                     i18n("The slot asked for %1 arguments but there are only are %2 arguments available.")
        //                        .arg(idx)
        //                        .arg(types.size() ) );
        return new NullPtr();
    }
    switch( args[idx]->type() )
    {
        case KJS::StringType:
            return new Value<QString>( args[idx]->toString(exec).qstring() );
            break;
        case KJS::NumberType:
            if( types[idx] == "int" )
                return new Value<int>( int( args[idx]->toInteger(exec) ) );
            else if ( types[idx] == "double" )
                return new Value<double>( args[idx]->toNumber(exec) );
            else if ( types[idx] == "float" )
                return new Value<float>( args[idx]->toNumber(exec) );
            else if ( types[idx] == "qreal" )
                return new Value<qreal>( args[idx]->toNumber(exec) );
            else if ( types[idx] == "uint" )
                return new Value<uint>( uint( args[idx]->toInteger(exec) ) );
            else
                return new NullPtr();
            break;
        case KJS::BooleanType:
            return new Value<bool>( args[idx]->toBoolean(exec) );
            break;
        case KJS::ObjectType:
            {
                if(ObjectBinding *objImp = KJSEmbed::extractBindingImp<ObjectBinding>(exec, args[idx]))
                {
                    return new Value<void*>(objImp->voidStar());
                }
                else if(VariantBinding *valImp = KJSEmbed::extractBindingImp<VariantBinding>(exec,args[idx]))
                {
                    return new Value<void*>(valImp->variant().data());
                }
            }
            break;
        case KJS::NullType:
            return new NullPtr();
            break;
        default:
            break;
    }
    qDebug("Cast failure %s value Type %d", types[idx].constData(), args[idx]->type() );
    KJS::throwError(exec, KJS::GeneralError, i18n("Cast failure %1 value Type %2",
                    types[idx].constData() ,
                    args[idx]->type()) );
    //KJSEmbed::throwError(exec,
    //        i18n("Cast failure %1 value Type %2")
    //                .arg(types[idx].constData())
    //                .arg(args[idx].type() ) );

    return new NullPtr();
}

KJS::JSValue *SlotBinding::callAsFunction( KJS::ExecState *exec, KJS::JSObject *self, const KJS::List &args )
{
    QObjectBinding *imp = extractBindingImp<QObjectBinding>(exec,self);
    if( imp == 0 )
        return KJS::Null();

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
    QVariant returnValue( returnTypeId );
    QGenericReturnArgument returnArgument(metaMember.typeName(), &returnValue);
    param[0] = returnArgument.data();
    for( int idx = 0; idx < 10; ++idx)
    {
        qtArgs[idx] = getArg(exec, types, args, idx);
        param[idx+1] = qtArgs[idx]->voidStar();
    }

    //qDebug("param ptr %0x", *(void**)param[1]);
    success = object->qt_metacall(QMetaObject::InvokeMetaMethod, offset, param) < 0;
    //qDebug("after param ptr %0x", *(void**)param[1]);

    for( int idx = 0; idx < 10; ++idx)
    {
        delete qtArgs[idx];
    }

    if( !success )
    {
        return KJS::throwError(exec, KJS::GeneralError, i18n("Call to '%1' failed.",  m_memberName.constData()));
        //return KJSEmbed::throwError(exec, i18n("Call to '%1' failed.").arg(m_memberName.constData()));
    }

    //TODO use the QMetaType-stuff ( defined as QVariant::UserType ) to handle also other cases
    return KJSEmbed::convertToValue(exec, returnValue);
}

SlotBinding::SlotBinding(KJS::ExecState *exec, const QMetaMethod &member )
    : KJS::InternalFunctionImp(static_cast<KJS::FunctionPrototype*>(exec->lexicalInterpreter()->builtinFunctionPrototype()))
{
    m_memberName = extractMemberName(member);
    int count = member.parameterNames().count();
    putDirect( KJS::lengthPropertyName, count, LengthFlags );
    setFunctionName( KJS::Identifier( m_memberName ) );
}

START_QOBJECT_METHOD( callParent, QObject )
    KJSEmbed::QObjectBinding *objImp = KJSEmbed::extractBindingImp<KJSEmbed::QObjectBinding>(exec, imp);
    QObject *parent = (!objImp || objImp->access() & QObjectBinding::GetParentObject)
        ? imp->object<QObject>()->parent()
        : 0;
    KJS::JSObject *parentObject = KJSEmbed::createQObject(exec, parent);
    KJSEmbed::QObjectBinding *parentImp = KJSEmbed::extractBindingImp<KJSEmbed::QObjectBinding>(exec, parentObject);
    if(objImp && parentImp) {
        parentImp->setAccess( objImp->access() ); // inherit access from child since we don't know the access-level of the parent here :-(
    }
    result = parentObject;
END_QOBJECT_METHOD
START_QOBJECT_METHOD( callIsWidgetType, QObject )
    result = KJS::Boolean(object->isWidgetType());
END_QOBJECT_METHOD
START_QOBJECT_METHOD( callInherits, QObject)
    QByteArray className = KJSEmbed::extractQString(exec, args, 0).toLatin1();
    result = KJS::Boolean(object->inherits(className.constData()));
END_QOBJECT_METHOD
START_QOBJECT_METHOD( callSetParent, QObject )
    KJSEmbed::QObjectBinding *objImp = KJSEmbed::extractBindingImp<KJSEmbed::QObjectBinding>(exec, imp);
    if(!objImp || objImp->access() & QObjectBinding::SetParentObject) {
        QObject *parent = KJSEmbed::extractObject<QObject>(exec, args, 0, 0);
        object->setParent(parent);
    }
END_QOBJECT_METHOD
START_QOBJECT_METHOD( callFindChild, QObject )
    KJSEmbed::QObjectBinding *objImp = KJSEmbed::extractBindingImp<KJSEmbed::QObjectBinding>(exec, imp);
    QString childName = KJSEmbed::extractQString(exec, args, 0);
    QObject *child = (!objImp || objImp->access() & QObjectBinding::ChildObjects)
        ? object->findChild<QObject*>(childName)
        : 0;
    KJS::JSObject *childObject = KJSEmbed::createQObject(exec, child);
    KJSEmbed::QObjectBinding *childImp = KJSEmbed::extractBindingImp<KJSEmbed::QObjectBinding>(exec, childObject);
    if(objImp && childImp) {
        childImp->setAccess( objImp->access() ); // inherit access from parent
    }
    result = childObject;
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
