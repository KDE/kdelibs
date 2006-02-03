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

#include <kjs/function_object.h>

using namespace KJSEmbed;

QByteArray createSignal( const QByteArray &sig )
{
    return '2' + sig;
}

QByteArray createSlot( const QByteArray &slt )
{
    return '1' + slt;
}

KJS::JSValue *callConnect( KJS::ExecState *exec, KJS::JSObject *self, const KJS::List &args )
{
    KJSEmbed::QObjectBinding *imp = KJSEmbed::extractBindingImp<KJSEmbed::QObjectBinding>(exec,  self );
    if( !imp ) // No implementation, so we need to use the first argument as we are a global static invocation.
        imp = KJSEmbed::extractBindingImp<KJSEmbed::QObjectBinding>(exec, args[0] );
    if( !imp )
        return KJS::throwError(exec, KJS::GeneralError, i18n("Wrong object type."));
//        return KJSEmbed::throwError(exec, i18n("Wrong object type."));

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
        // 2#signal
        char *signal = qstrdup( createSignal(args[1]->toString(exec).ascii()).data() );
        // 1#slot
        char *slot = 0;
        if( args.size() == 4)
        {
            slot = qstrdup( createSlot(args[3]->toString(exec).ascii()).data() );
            KJSEmbed::QObjectBinding *receiverImp = KJSEmbed::extractBindingImp<KJSEmbed::QObjectBinding>(exec, args[2] );
            if( !receiverImp )
                receiver = new SlotProxy(args[2]->toObject(exec), exec->interpreter(), sender, args[3]->toString(exec).ascii() );
            else
                receiver = receiverImp->object<QObject>();
        }
        else
        {
            receiver = imp->object<QObject>();
            slot = qstrdup( createSlot(args[2]->toString(exec).ascii()).data() );
        }
        return KJS::Boolean(QObject::connect(sender, signal, receiver, slot));
    }
    return KJS::throwError(exec, KJS::GeneralError, i18n("Incorrect number of arguments."));
//    return KJSEmbed::throwError(exec, i18n("Incorrect number of arguments."));
}

QByteArray extractMemberName( const QMetaMethod &member )
{
    QString sig = member.signature();
    return sig.left( sig.indexOf('(') ).toLatin1();
}

void QObjectBinding::publishQObject( KJS::ExecState *exec, KJS::JSObject *target, QObject *object)
{
    QObjectList children = object->children();
    QObjectList::Iterator child = children.begin();
    for( ; child != children.end(); ++child)
    {
        QString objectName = (*child)->objectName();
        if( !objectName.isEmpty() )
        {
            target->put(exec, KJS::Identifier(objectName), KJSEmbed::createQObject(exec,(*child) ));
        }
    }
            // Add slots of the current object
    const QMetaObject *metaObject = object->metaObject();
    int methods = metaObject->methodCount();
    for( int idx = 0; idx < methods; ++idx )
    {
        QMetaMethod member = metaObject->method(idx);
        target->put(exec, KJS::Identifier( extractMemberName( member ) ),
            new SlotBinding(exec,member), KJS::DontDelete|KJS::ReadOnly);
    }

    // Add enums as read only uints.
    // int enums = metaObject->enumeratorCount();
    for( int idx = 0; idx < methods; ++idx )
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
    : ObjectBinding(exec, object->metaObject()->className(), object),
      m_evproxy(0)
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

    // Add QObjectBindings for each child object.
    // Publish the object_bindings
    StaticBinding::publish( exec, this, ValueFactory::methods() );
    StaticBinding::publish( exec, this, ObjectFactory::methods() );
    StaticBinding::publish( exec, this, QObjectFactory::methods() );
    QObjectBinding::publishQObject(exec, this, object);

    // Make "connect" a global static method.
    exec->interpreter()->globalObject()->put(exec, "connect", new StaticBinding(exec,  &QObjectFactory::methods()[0]) );
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

KJS::JSValue *QObjectBinding::get(KJS::ExecState *exec, const KJS::Identifier &propertyName) const
{
    QObject *obj = object<QObject>();
    if ( obj && !m_cleanupHandler->isEmpty() )
    {
        // Properties
        //QString prop = propertyName.qstring();
        const QMetaObject *meta = obj->metaObject();

        if ( meta->indexOfProperty( propertyName.ascii() ) != -1 )
        {
            QVariant val = obj->property( propertyName.ascii() );
            return convertToValue( exec, val );
        }
    }
    // Get a property value
    return ObjectBinding::get(exec,propertyName);
}

void QObjectBinding::put(KJS::ExecState *exec, const KJS::Identifier &propertyName, KJS::JSValue *value, int attr )
{
    QObject *obj = object<QObject>();
    if ( obj && !m_cleanupHandler->isEmpty() )
    {
        // Properties
        //QString prop = propertyName.qstring();
        const QMetaObject *meta = obj->metaObject();

        if ( int propIndex = meta->indexOfProperty( propertyName.ascii() ) != -1 )
        {
            bool propSet = false;
            QVariant val = convertToVariant( exec, value );
            if ( meta->property(propIndex).isEnumType() )
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
                m_evproxy = new KJSEmbed::EventProxy( this, exec->interpreter() );
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
        //QString prop = propertyName.qstring();
        const QMetaObject *meta = obj->metaObject();

        if ( int propIndex = meta->indexOfProperty( propertyName.ascii() ) != -1 )
        {
            if ( meta->property(propIndex).isWritable()  )
            {
                return true;
            }
        }
    }
    return ObjectBinding::canPut(exec,propertyName);
}

bool QObjectBinding::hasProperty(KJS::ExecState *exec, const KJS::Identifier &propertyName) const
{
    Q_UNUSED( exec );
    return ( object<QObject>()->metaObject()->indexOfProperty( propertyName.ascii() ) != -1 );
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
        KJS::throwError(exec, KJS::GeneralError, i18n("The slot sked for %1 arguments but there are only %2 arguments available.").arg(idx).arg(types.size()));
//        KJSEmbed::throwError(exec,
//                i18n("The slot asked for %1 arguments but there are only are %2 arguments available.")
//                        .arg(idx)
//                        .arg(types.size() ) );
        return new NullPtr();
    }
    switch( args[idx]->type() )
    {
        case KJS::StringType:
            return new Scalar<QString>( args[idx]->toString(exec).qstring() );
            break;
        case KJS::NumberType:
            if( types[idx] == "int" )
                return new Scalar<int>( args[idx]->toNumber(exec) );
            else if ( types[idx] == "double" )
                return new Scalar<double>( args[idx]->toNumber(exec) );
            else if ( types[idx] == "float" )
                return new Scalar<float>( args[idx]->toNumber(exec) );
            else if ( types[idx] == "qreal" )
                return new Scalar<qreal>( args[idx]->toNumber(exec) );
            else
                return new NullPtr();
            break;
        case KJS::BooleanType:
            return new Scalar<bool>( args[idx]->toBoolean(exec) );
            break;
        case KJS::ObjectType:
            {
                if(ObjectBinding *objImp = KJSEmbed::extractBindingImp<ObjectBinding>(exec, args[idx]))
                {
                    return new Scalar<void*>(objImp->voidStar());
                }
                else if(ValueBinding *valImp = KJSEmbed::extractBindingImp<ValueBinding>(exec,args[idx]))
                {
                    return new Scalar<void*>(valImp->variant().data());
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
    KJS::throwError(exec, KJS::GeneralError, i18n("Cast failure %1 value Type %2").arg(types[idx].constData()).arg(args[idx]->type()));
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
        return KJS::Boolean(false);

    PointerBase *qtArgs[10];
    void *param[11];

    QGenericArgument returnArgument;
    QObject *object = imp->object<QObject>();
    int count = object->metaObject()->methodCount();
    QMetaMethod metaMember;
    int offset = 0;
    for(; offset < count; ++offset)
    {
        metaMember = object->metaObject()->method(offset);
        if( extractMemberName(metaMember) == m_memberName )
        {
            if( metaMember.parameterTypes().size() == args.size() )
                break;
        }
    }
    // int offset = object->metaObject()->indexOfMethod( m_memberName.constData() );
    // QMetaMethod metaMember = object->metaObject()->method(offset);
    bool success = false;
    QList<QByteArray> types = metaMember.parameterTypes();

//    int kjsArgCount = args.size();
//    int qArgCount = types.size();

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
        KJS::throwError(exec, KJS::GeneralError, i18n("Call to '%1' failed.").arg(m_memberName.constData()));
        // KJSEmbed::throwError(exec, i18n("Call to '%1' failed.").arg(m_memberName.constData()));
    }
    return KJS::Boolean(success);
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
    QObject *parent = imp->object<QObject>()->parent();
    result = KJSEmbed::createQObject(exec, parent);
END_QOBJECT_METHOD
START_QOBJECT_METHOD( callIsWidgetType, QObject )
    result = KJS::Boolean(object->isWidgetType());
END_QOBJECT_METHOD
START_QOBJECT_METHOD( callInherits, QObject)
    QByteArray className = KJSEmbed::extractQString(exec, args, 0).toLatin1();
    result = KJS::Boolean(object->inherits(className.constData()));
END_QOBJECT_METHOD
START_QOBJECT_METHOD( callSetParent, QObject )
    QObject *parent = KJSEmbed::extractObject<QObject>(exec, args, 0, 0);
    object->setParent(parent);
END_QOBJECT_METHOD
START_QOBJECT_METHOD( callFindChild, QObject )
    QString childName = KJSEmbed::extractQString(exec, args, 0);
    QObject *child = object->findChild<QObject*>(childName);
    result = KJSEmbed::createQObject(exec, child);
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
