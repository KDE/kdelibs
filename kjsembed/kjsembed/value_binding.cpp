#include "value_binding.h"
#include "static_binding.h"
#include <qdebug.h>

// QVariant includes
#include <QBitArray>
#include <QByteArray>

#include <stdlib.h>
#include <global.h>

#include <kjs/reference_list.h>

using namespace KJSEmbed;

ValueBinding::ValueBinding( KJS::ExecState *exec, const QVariant &value )
    : KJS::JSObject(exec->interpreter()->builtinObjectPrototype()),
      m_value(value)
{
    StaticBinding::publish( exec, this, ValueFactory::methods() );
}

QGenericArgument ValueBinding::arg(const char *type) const
{
    const void *p = m_value.constData();
    //qDebug("Ptr %0x", p );
    qDebug() << p;

    return QGenericArgument( type, p );
}

KJS::JSValue *callName( KJS::ExecState *exec, KJS::JSObject *self, const KJS::List &args )
{
    KJSEmbed::ValueBinding *imp = KJSEmbed::extractBindingImp<KJSEmbed::ValueBinding>(exec,  self );
    if( imp )
    {
        QVariant val = imp->variant();
        return KJS::String( val.typeName() );
    }
    return KJS::Null();
}

KJS::JSValue *callCast( KJS::ExecState *exec, KJS::JSObject *self, const KJS::List &args )
{
    KJSEmbed::ValueBinding *imp = KJSEmbed::extractBindingImp<KJSEmbed::ValueBinding>(exec,  self );
    if( imp )
    {
        QVariant val = imp->variant();
        QVariant::Type type = QVariant::nameToType( args[0]->toString(exec).ascii() );
        KJS::JSValue *returnValue = KJS::Boolean(val.convert(type));
        imp->setValue(val);
        return returnValue;
    }
    return KJS::Null();
}

KJS::JSValue *callToString( KJS::ExecState *exec, KJS::JSObject *self, const KJS::List &args )
{
    KJSEmbed::ValueBinding *imp = KJSEmbed::extractBindingImp<KJSEmbed::ValueBinding>(exec,  self );
    if( imp )
    {
//        qDebug("Call value to string");
        QVariant val = imp->variant();
        QString stringVal = val.toString();
        if( !stringVal.isEmpty() )
            return KJS::String( val.toString() );
        return KJS::String( val.typeName() );
    }
    return KJS::Null();
}

const Method ValueFactory::ValueMethods[] =
{
    {"cast", 1, KJS::DontDelete|KJS::ReadOnly|KJS::DontEnum, &callCast },
    {"toString", 0, KJS::DontDelete|KJS::ReadOnly|KJS::DontEnum, &callToString },
    {0, 0, 0, 0 }
};

enum JavaScriptArrayType { None, List, Map };

JavaScriptArrayType checkArray( KJS::ExecState *exec, KJS::JSValue *val )
{
    KJS::JSObject *obj = val->toObject( exec );
    if ( obj->className().qstring() == "Array" )
    {
        KJS::JSValue *len = obj->get(exec, KJS::Identifier("length"));
        char buff[4];
        if( !obj->hasProperty(exec, KJS::Identifier("length")) )
            return Map;
        else if( !obj->hasProperty(exec, KJS::Identifier( itoa((len->toNumber(exec) - 1), buff, 10 ) ) ) )
            return Map;
        else
            return List;
    }
    else
        return None;
}

QMap<QString, QVariant> KJSEmbed::convertArrayToMap( KJS::ExecState *exec, KJS::JSValue *value )
{
    QMap<QString, QVariant> returnMap;
    KJS::JSObject *obj = value->toObject(exec);
    KJS::ReferenceList lst = obj->propList(exec,false);
    KJS::ReferenceListIterator idx = lst.begin();
    for( ; idx != lst.end(); idx++ )
    {
        KJS::Identifier id = idx->getPropertyName(exec);
        KJS::JSValue *val = idx->getValue(exec);
        returnMap[id.qstring()] = convertToVariant(exec,val);
    }
    return returnMap;
}

QList<QVariant> KJSEmbed::convertArrayToList( KJS::ExecState *exec, KJS::JSValue *value )
{
    QList<QVariant> returnList;
    KJS::JSObject *obj = value->toObject( exec );
    if ( obj->className().qstring() == "Array" )
    {
        int length = obj->get( exec, KJS::Identifier( "length" ) )->toInteger( exec );
        for ( int index = 0; index < length; ++index )
        {
            char buff[4];
            KJS::JSValue *val = obj->get(exec, KJS::Identifier( itoa( index, buff, 10 ) ) );
            if( val )
                returnList += convertToVariant(exec, val );
            else
                returnList += QVariant();
        }
    }
    return returnList;
}

QStringList KJSEmbed::convertArrayToStringList( KJS::ExecState *exec, KJS::JSValue *value )
{
    QStringList returnList;
    KJS::JSObject *obj = value->toObject( exec );
    if ( obj->className().qstring() == "Array" )
    {
        int length = obj->get( exec, KJS::Identifier( "length" ) )->toInteger( exec );
        for ( int index = 0; index < length; ++index )
        {
            char buff[4];
            KJS::JSValue *val = obj->get(exec, KJS::Identifier( itoa( index, buff, 10 ) ) );
            if( val )
                returnList += convertToVariant(exec, val ).value<QString>();
            else
                returnList += QString::null;
        }
    }
    return returnList;
}

QDateTime convertDateToDateTime( KJS::ExecState *exec, KJS::JSValue *value )
{
    KJS::List args;
    QDateTime returnDateTime;
    KJS::JSObject *obj = value->toObject( exec );

    if ( obj->className().qstring() == "Date" )
    {
        int seconds = obj->get( exec, KJS::Identifier( "getSeconds" ) )
                ->toObject( exec )->call( exec, obj, args )->toInteger( exec );
        int minutes = obj->get( exec, KJS::Identifier( "getMinutes" ) )
                ->toObject( exec )->call( exec, obj, args )->toInteger( exec );
        int hours = obj->get( exec, KJS::Identifier( "getHours" ) )
                ->toObject( exec )->call( exec, obj, args )->toInteger( exec );
        int month = obj->get( exec, KJS::Identifier( "getMonth" ) )
                ->toObject( exec )->call( exec, obj, args )->toInteger( exec );
        int day = obj->get( exec, KJS::Identifier( "getDate" ) )
                ->toObject( exec )->call( exec, obj, args )->toInteger( exec );
        int year = obj->get( exec, KJS::Identifier( "getFullYear" ) )
                ->toObject( exec )->call( exec, obj, args )->toInteger( exec );

        returnDateTime.setDate( QDate( year, month + 1, day ) );
        returnDateTime.setTime( QTime( hours, minutes, seconds ) );
    }
    else
    {
        // Throw error
    }

    return returnDateTime;
}

QVariant KJSEmbed::convertToVariant( KJS::ExecState *exec, KJS::JSValue *value )
{
    //qDebug() << "Cast " << value.type();

    QVariant returnValue;
    switch( value->type() )
    {
        case KJS::NullType:
            break;

        case KJS::StringType:
            returnValue = value->toString(exec).qstring();
            break;

        case KJS::NumberType:
            returnValue = value->toNumber(exec);
            break;

        case KJS::BooleanType:
            returnValue = value->toBoolean(exec);
            break;

        case KJS::ObjectType:
        {
            KJS::JSObject *obj = value->toObject(exec);
            //qDebug() << "Object type: " << obj.className().qstring();
            if ( obj->className().qstring() == "Array" )
            {
                if ( checkArray( exec, value ) == List )
                    returnValue = convertArrayToList( exec, value );
                else
                    returnValue = convertArrayToMap(exec, value);
            }
            else if ( obj->className().qstring() == "Date" )
            {
                returnValue = convertDateToDateTime( exec, value );
            }
            else
            {
                returnValue = extractVariant(exec,value);
            }
            if( returnValue.isNull() )
            {
                returnValue = value->toString(exec).qstring();
            }
        }
        default:
            returnValue = extractVariant(exec,value);
            if( returnValue.isNull() )
            {
                returnValue = value->toString(exec).qstring();
            }
            break;
    }

    return returnValue;
}

KJS::JSValue *KJSEmbed::convertToValue( KJS::ExecState *exec, const QVariant &value )
{
    KJS::JSValue *returnValue;
    switch( value.type() )
    {
        case QVariant::Bool:
            returnValue = KJS::Boolean( value.value<bool>() );
            break;
        case QVariant::Int:
            returnValue = KJS::Number( value.value<int>() );
            break;
        case QVariant::Double:
            returnValue = KJS::Number( value.value<double>() );
            break;
        case QVariant::UInt:
            returnValue = KJS::Number( value.value<unsigned int>() );
            break;
        case QVariant::String:
        case QVariant::ByteArray:
        {
            returnValue = KJS::String( value.value<QString>() );
            break;
        }
        case QVariant::StringList:
        {
            KJS::List items;
            QStringList lst = value.value<QStringList>();
            QStringList::Iterator idx = lst.begin();
            for ( ; idx != lst.end(); ++idx )
            {
                items.append( KJS::String( ( *idx ) ) );
            }
            returnValue = exec->interpreter()->builtinArray()->construct( exec, items );
            break;
        }
        case QVariant::List:
        {
            KJS::List items;
            QList<QVariant> lst = value.toList();
            foreach( QVariant item, lst)
            {
                items.append( convertToValue( exec, item ) );
            }
            returnValue = exec->interpreter()->builtinArray()->construct( exec, items );
            break;
        }
        case QVariant::Map:
        {
            QMap<QString,QVariant> map = value.toMap();
            QMap<QString,QVariant>::Iterator idx = map.begin();
            KJS::JSObject *array = exec->interpreter()->builtinArray()->construct( exec, 0 );
            for ( ; idx != map.end(); ++idx )
            {
                array->put(exec, KJS::Identifier( idx.key() ), convertToValue( exec,  idx.value() ) );
            }
            returnValue =  array;
            break;
        }
        case QVariant::Date:
        case QVariant::DateTime:
        case QVariant::Time:
        {
            QDateTime dt = QDateTime::currentDateTime();
            if ( value.type() == QVariant::Date )
                dt.setDate( value.toDate() );
            else if ( value.type() == QVariant::Time )
                dt.setTime( value.toTime() );
            else
                dt = value.toDateTime();

            KJS::List items;
            items.append( KJS::Number( dt.date().year() ) );
            items.append( KJS::Number( dt.date().month() - 1 ) );
            items.append( KJS::Number( dt.date().day() ) );
            items.append( KJS::Number( dt.time().hour() ) );
            items.append( KJS::Number( dt.time().minute() ) );
            items.append( KJS::Number( dt.time().second() ) );
            items.append( KJS::Number( dt.time().msec() ) );
            returnValue = exec->interpreter()->builtinDate()->construct( exec, items );
            break;
        }
        default:
        {
            returnValue = createValue(exec, value.typeName(), value );
            if( returnValue->isNull() )
                returnValue = KJS::String( value.value<QString>() );
            break;
        }
    }
    return returnValue;
}

QVariant KJSEmbed::extractVariant( KJS::ExecState *exec, KJS::JSValue *value )
{
    KJSEmbed::ValueBinding *imp = KJSEmbed::extractBindingImp<KJSEmbed::ValueBinding>(exec,  value );
    if( imp )
        return imp->variant();
    else if( value->type() == KJS::StringType)
        return QVariant(value->toString(exec).qstring());
    else if( value->type() == KJS::NumberType)
        return QVariant(value->toNumber(exec));
    else if( value->type() == KJS::BooleanType)
        return QVariant(value->toBoolean(exec));
    return QVariant();
}
