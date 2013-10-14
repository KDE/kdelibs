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

#include "variant_binding.h"

#include <stdlib.h>

#include <kjs/PropertyNameArray.h>
#include <kjs/array_instance.h>

#include <QtCore/QBitRef>
#include <QtCore/QByteRef>
#include <QtCore/QDebug>
#include <QtCore/QObject>
#include <QWidget>

#include "kjseglobal.h"
#include "static_binding.h"
#include "qobject_binding.h"

//#define KJSEMBED_VARIANT_DEBUG

using namespace KJSEmbed;

const KJS::ClassInfo VariantBinding::info = { "VariantBinding", 0, 0, 0 };

VariantBinding::VariantBinding( KJS::ExecState *exec, const QVariant &value )
    : ProxyBinding(exec), m_value(value)
{
    StaticBinding::publish( exec, this, VariantFactory::methods() );
}

void *VariantBinding::pointer()
{
    return m_value.data();
}

KJS::UString VariantBinding::toString(KJS::ExecState *) const
{
    return toUString(m_value.toString());
}

KJS::UString VariantBinding::className() const
{
    return m_value.typeName();
}

QVariant VariantBinding::variant() const
{
    return m_value;
}

void VariantBinding::setValue( const QVariant &val )
{
    m_value = val;
}

QGenericArgument VariantBinding::arg(const char *type) const
{
    const void *p = m_value.constData();
    //qDebug("Ptr %0x", p );
    //qDebug() << p;
    return QGenericArgument( type, p );
}

KJS::JSValue *callName( KJS::ExecState *exec, KJS::JSObject *self, const KJS::List &args )
{
    Q_UNUSED( args );
    KJSEmbed::VariantBinding *imp = KJSEmbed::extractBindingImp<KJSEmbed::VariantBinding>(exec,  self);
    return imp ? KJS::jsString( imp->variant().typeName() ) : KJS::jsNull();
}

KJS::JSValue *callCast( KJS::ExecState *exec, KJS::JSObject *self, const KJS::List &args )
{
    KJSEmbed::VariantBinding *imp = KJSEmbed::extractBindingImp<KJSEmbed::VariantBinding>(exec,  self );
    if( imp )
    {
        QVariant val = imp->variant();
        QVariant::Type type = QVariant::nameToType( args[0]->toString(exec).ascii() );
        KJS::JSValue *returnValue = KJS::jsBoolean(val.convert(type));
        imp->setValue(val);
        return returnValue;
    }
    return KJS::jsNull();
}

KJS::JSValue *callToString( KJS::ExecState *exec, KJS::JSObject *self, const KJS::List &args )
{
    Q_UNUSED( args );
    KJSEmbed::VariantBinding *imp = KJSEmbed::extractBindingImp<KJSEmbed::VariantBinding>(exec,  self );
    if( imp )
    {
        //qDebug("Call value to string");
        QVariant val = imp->variant();
        QString stringVal = val.toString();
        if( !stringVal.isEmpty() )
            return KJS::jsString( val.toString() );
        return KJS::jsString( val.typeName() );
    }
    return KJS::jsNull();
}

const Method VariantFactory::VariantMethods[] =
{
    {"cast", 1, KJS::DontDelete|KJS::ReadOnly|KJS::DontEnum, &callCast },
    {"toString", 0, KJS::DontDelete|KJS::ReadOnly|KJS::DontEnum, &callToString },
    {0, 0, 0, 0 }
};

enum JavaScriptArrayType { None, List, Map };

JavaScriptArrayType checkArray( KJS::ExecState *exec, KJS::JSValue *val )
{
    KJS::JSObject *obj = val->toObject( exec );
    if ( toQString(obj->className()) == "Array" )
    {
        if( !obj->hasProperty(exec, KJS::Identifier("length")) )
            return Map;
        KJS::JSValue *jslen = obj->get(exec, KJS::Identifier("length"));
        const int len = jslen->toNumber(exec);
        if ( len > 0 ) {
            QByteArray buff;
            buff.setNum(len-1);
            if( !obj->hasProperty(exec, KJS::Identifier( buff.data() ) ) )
                return Map;
        }
        return List;
    }
    else
        return None;
}

QMap<QString, QVariant> KJSEmbed::convertArrayToMap( KJS::ExecState *exec, KJS::JSValue *value )
{
    QMap<QString, QVariant> returnMap;
    KJS::JSObject *obj = value->toObject(exec);
    KJS::PropertyNameArray lst;
    obj->getPropertyNames(exec, lst);
    KJS::PropertyNameArrayIterator idx = lst.begin();
    for( ; idx != lst.end(); idx++ )
    {
        KJS::Identifier id = *idx;
        KJS::JSValue *val  = obj->get(exec, id);
        returnMap[toQString(id)] = convertToVariant(exec,val);
    }
    return returnMap;
}

QList<QVariant> KJSEmbed::convertArrayToList( KJS::ExecState *exec, KJS::JSValue *value )
{
    QList<QVariant> returnList;
    /*
    KJS::JSObject *obj = value->toObject( exec );
    if ( toQString(obj->className() == "Array" )
    {
        int length = int(obj->get( exec, KJS::Identifier( "length" ) )->toInteger( exec ) );
        for ( int index = 0; index < length; ++index )
        {
            QByteArray buff;
            buff.setNum(index);
            KJS::JSValue *val = obj->get(exec, KJS::Identifier( buff.data() ) );
            if( val )
                returnList += convertToVariant(exec, val );
            else
                returnList += QVariant();
        }
    }
    */
    KJS::ArrayInstance* arrayImp = extractBindingImp<KJS::ArrayInstance>(exec, value);
    if(arrayImp)
    {
        const unsigned numItems = arrayImp->getLength();
        for ( unsigned i = 0; i < numItems; ++i )
            returnList.append( convertToVariant(exec, arrayImp->getItem(i)) );
    }
    return returnList;
}

QStringList KJSEmbed::convertArrayToStringList( KJS::ExecState *exec, KJS::JSValue *value )
{
    QStringList returnList;
    /*
    KJS::JSObject *obj = value->toObject( exec );
    if ( toQString(obj->className()) == "Array" )
    {
        int length = int( obj->get( exec, KJS::Identifier( "length" ) )->toInteger( exec ) );
        for ( int index = 0; index < length; ++index )
        {
            QByteArray buff;
            buff.setNum(index);
            KJS::JSValue *val = obj->get(exec, KJS::Identifier( buff.data() ) );
            if( val )
                returnList += convertToVariant(exec, val ).value<QString>();
            else
                returnList += QString();
        }
    }
    */
    KJS::ArrayInstance* arrayImp = extractBindingImp<KJS::ArrayInstance>(exec, value);
    if(arrayImp)
    {
        const unsigned numItems = arrayImp->getLength();
        for ( unsigned i = 0; i < numItems; ++i )
            returnList.append( convertToVariant(exec, arrayImp->getItem(i)).toString() );
    }
    return returnList;
}

QDateTime convertDateToDateTime( KJS::ExecState *exec, KJS::JSValue *value )
{
    KJS::List args;
    QDateTime returnDateTime;
    KJS::JSObject *obj = value->toObject( exec );
    if ( toQString(obj->className()) == "Date" )
    {
        int seconds = int( obj->get( exec, KJS::Identifier( "getSeconds" ) )->toObject( exec )->call( exec, obj, args )->toInteger( exec ) );
        int minutes = int( obj->get( exec, KJS::Identifier( "getMinutes" ) )->toObject( exec )->call( exec, obj, args )->toInteger( exec ) );
        int hours = int( obj->get( exec, KJS::Identifier( "getHours" ) )->toObject( exec )->call( exec, obj, args )->toInteger( exec ) );
        int month = int( obj->get( exec, KJS::Identifier( "getMonth" ) )->toObject( exec )->call( exec, obj, args )->toInteger( exec ) );
        int day = int( obj->get( exec, KJS::Identifier( "getDate" ) )->toObject( exec )->call( exec, obj, args )->toInteger( exec ) );
        int year = int( obj->get( exec, KJS::Identifier( "getFullYear" ) )->toObject( exec )->call( exec, obj, args )->toInteger( exec ) );
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
#ifdef KJSEMBED_VARIANT_DEBUG
    qDebug()<<"KJSEmbed::convertToVariant";
#endif

    QVariant returnValue;
    switch( value->type() )
    {
        case KJS::UndefinedType:
        case KJS::NullType:
            break;
        case KJS::StringType:
            returnValue = toQString(value->toString(exec));
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
            //qDebug() << "Object type: " << toQString(obj->className());
            if ( toQString(obj->className()) == "Array" ) {
                if (checkArray(exec, value) == List)
                    returnValue = convertArrayToList(exec, value);
                else
                    returnValue = convertArrayToMap(exec, value);
            }
            else if ( toQString(obj->className()) == "Date" )
                returnValue = convertDateToDateTime( exec, value );
            else
                returnValue = extractVariant(exec,value);
            //if( returnValue.isNull() ) returnValue = toQString(value->toString(exec));
        } break;
        default:
            returnValue = extractVariant(exec,value);
            //if( returnValue.isNull() ) returnValue = toQString(value->toString(exec));
            break;
    }
    return returnValue;
}

KJS::JSValue *KJSEmbed::convertToValue( KJS::ExecState *exec, const QVariant &value )
{
#ifdef KJSEMBED_VARIANT_DEBUG
    qDebug()<<"KJSEmbed::convertToValue typeid="<<value.type()<<"typename="<<value.typeName()<<"toString="<<value.toString();
#endif

    KJS::JSValue *returnValue;
    switch( value.type() )
    {
        case QVariant::Invalid:
            returnValue = KJS::jsNull();
            break;
        case QVariant::Int:
            returnValue = KJS::jsNumber( value.value<int>() );
            break;
        case QVariant::UInt:
            returnValue = KJS::jsNumber( value.value<unsigned int>() );
            break;
        case QVariant::LongLong:
            returnValue = KJS::jsNumber( value.value<qlonglong>() );
            break;
        case QVariant::ULongLong:
            returnValue = KJS::jsNumber( value.value<qulonglong>() );
            break;
        case QVariant::Double:
            returnValue = KJS::jsNumber( value.value<double>() );
            break;
        case QVariant::Bool:
            returnValue = KJS::jsBoolean( value.value<bool>() );
            break;
        case QVariant::ByteArray:
            returnValue = KJS::jsString( QString(value.value<QByteArray>()) );
            break;
        case QVariant::String:
            returnValue = KJS::jsString( value.value<QString>() );
            break;
        case QVariant::StringList:
        {
            KJS::List items;
            QStringList lst = value.value<QStringList>();
            QStringList::Iterator idx = lst.begin();
            for ( ; idx != lst.end(); ++idx )
                items.append( KJS::jsString( ( *idx ) ) );
            returnValue = exec->lexicalInterpreter()->builtinArray()->construct( exec, items );
            break;
        }
        case QVariant::Date: // fall through
        case QVariant::DateTime: // fall through
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
            items.append( KJS::jsNumber( dt.date().year() ) );
            items.append( KJS::jsNumber( dt.date().month() - 1 ) );
            items.append( KJS::jsNumber( dt.date().day() ) );
            items.append( KJS::jsNumber( dt.time().hour() ) );
            items.append( KJS::jsNumber( dt.time().minute() ) );
            items.append( KJS::jsNumber( dt.time().second() ) );
            items.append( KJS::jsNumber( dt.time().msec() ) );
            returnValue = exec->lexicalInterpreter()->builtinDate()->construct( exec, items );
            break;
        }
        case QVariant::List:
        {
            KJS::List items;
            QList<QVariant> lst = value.toList();
            foreach( const QVariant &item, lst)
                items.append( convertToValue( exec, item ) );
            returnValue = exec->lexicalInterpreter()->builtinArray()->construct( exec, items );
            break;
        }
        case QVariant::Map:
        {
            QMap<QString,QVariant> map = value.toMap();
            QMap<QString,QVariant>::Iterator idx = map.begin();
            KJS::JSObject *obj = exec->lexicalInterpreter()->builtinObject()->construct( exec, KJS::List() );
            for ( ; idx != map.end(); ++idx )
                obj->put(exec, KJS::Identifier( toUString(idx.key()) ), convertToValue( exec,  idx.value() ) );
            returnValue =  obj;
            break;
        }
        default:
        {
            if( value.canConvert< QWidget* >() ) {
                QWidget* widget = qvariant_cast< QWidget* >(value);
                returnValue = widget ? createQObject(exec, widget, KJSEmbed::ObjectBinding::CPPOwned) : KJS::jsNull();
            }
            else if( value.canConvert< QObject* >() ) {
                QObject* object = qvariant_cast< QObject* >(value);
                returnValue = object ? createQObject(exec, object, KJSEmbed::ObjectBinding::CPPOwned) : KJS::jsNull();
            }
            else {
                returnValue = createVariant(exec, value.typeName(), value );
                if( returnValue->isNull() )
                    returnValue = KJS::jsString( value.value<QString>() );
            }
            break;
        }
    }
    return returnValue;
}

QVariant KJSEmbed::extractVariant( KJS::ExecState *exec, KJS::JSValue *value )
{
#ifdef KJSEMBED_VARIANT_DEBUG
    qDebug()<<"KJSEmbed::extractVariant";
#endif

    KJSEmbed::VariantBinding *imp = KJSEmbed::extractBindingImp<KJSEmbed::VariantBinding>(exec,  value );
    if( imp )
        return imp->variant();
    if( value->type() == KJS::StringType)
        return QVariant(toQString(value->toString(exec)));
    if( value->type() == KJS::NumberType)
        return QVariant(value->toNumber(exec));
    if( value->type() == KJS::BooleanType)
        return QVariant(value->toBoolean(exec));

    KJS::JSObject *obj = value->toObject( exec );
    if ( obj ) {
        if(QObjectBinding *objImp = KJSEmbed::extractBindingImp<QObjectBinding>(exec, value)) {
            QVariant v;
            if( QObject* qobj = objImp->qobject<QObject>() )
                v.setValue(qobj);
            return v;
        }
        if( toQString(obj->className()) == "Array" )
            return convertArrayToList( exec, value );
    }
    return QVariant();
}

//kate: indent-spaces on; indent-width 4; replace-tabs on; indent-mode cstyle;
