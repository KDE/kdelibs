/*
*  Copyright (C) 2005, Ian Reinhart Geiser <geiseri@kde.org>
*  Copyright (C) 2006, Matt Broadstone <mbroadst@gmail.com>
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
*/

#ifndef VALUE_BINDING_H
#define VALUE_BINDING_H

#include <QVariant>

#include <kdemacros.h>
#include <kjs/object.h>
#include <kjs/interpreter.h>

#include "static_binding.h"

/**
* A simple variant syle method.
* This will extract the QVariant, cast it to the native type and place it in "value".
* Any data that should be returned from this method should be placed into "result";
*
*/
#define START_VARIANT_METHOD( METHODNAME, TYPE) \
KJS::JSValue *METHODNAME( KJS::ExecState *exec, KJS::JSObject *self, const KJS::List &args ) \
{ \
        Q_UNUSED(exec);\
        Q_UNUSED(self);\
        Q_UNUSED(args);\
        KJS::JSValue *result = KJS::Null(); \
        KJSEmbed::ValueBinding *imp = KJSEmbed::extractBindingImp<KJSEmbed::ValueBinding>(exec, self ); \
        if( imp ) \
        { \
                TYPE value = imp->value<TYPE>();
/**
* End a variant method started by START_VARIANT_METHOD
*/
#define END_VARIANT_METHOD \
                imp->setValue(qVariantFromValue(value)); \
        } \
        else \
        {\
            KJS::throwError(exec, KJS::GeneralError, "We have a problem baby");\
        }\
        return result; \
}

namespace KJSEmbed
{
    /**
    * QVariant based binding.  This class wraps the implementation.
    * You should never have to use this class directly instead use KJSEmbed::createValue.
    */

    /**
    * QVariant bindinging implementation.
    */
    class KDE_EXPORT ValueBinding : public KJS::JSObject
    {
        public:
            /**
            * Create a new binding implementation with a QVariant to wrap
            */
            ValueBinding( KJS::ExecState *exec, const QVariant &value );
            virtual ~ValueBinding() {}

            bool implementsCall() const { return true; }
            bool implementsConstruct() const { return true; }

            void *pointer() { return m_value.data(); }

            KJS::UString toString(KJS::ExecState *) const { return m_value.toString(); }
            KJS::UString className() const { return m_value.typeName(); }

            /**
            * Return the wrapped QVariant
            */
            QVariant variant() const { return m_value; }

            /**
            * Extract the actual value from the wrapper.  This method wraps QVariant::value, so it will support
            * some aspects of casting.  If the cast fails a default value of T will be returned.
            */
            template< typename T>
            T value() const { return qVariantValue<T>(m_value); }
            /**
            * Set the internal value of the QVariant.
            */
            void setValue( const QVariant &val ) { m_value = val; }

            /**
            * Constructs a QGenericArgument that is used with QMetaObject::invokeMember
            */
            QGenericArgument arg( const char *type) const;

        protected:
            QVariant m_value;
    };

    /**
    * Extracts a QVariant from a KJS::JSValue if the conversion fails a QVariant::Null is returned.
    */
    QVariant KDE_EXPORT extractVariant( KJS::ExecState *exec, KJS::JSValue *value );

    /**
    * Extracts a value based type from a ValueBinding object.  This method calls @ref extractVariant so if
    * the conversions fails then the default value will be returned.  Care should be taken that this method
    * is not used with KJSEmbed::ObjectBinding objects because the cast will fail.
    */
    template< typename T>
    T KDE_EXPORT extractValue( KJS::ExecState *exec, KJS::JSValue *arg, const T &defaultValue )
    {
        if( !arg )
            return defaultValue;
        else
        {
            QVariant variant = extractVariant( exec, arg );
            if( !variant.isNull() )
            {
                if( qVariantCanConvert<T>(variant) )
                    return qVariantValue<T>(variant);
                else
                {
                    throwError(exec, KJS::GeneralError, "Cast failed" );
                    return defaultValue;
                }
            }
            else
                return defaultValue;
        }
    }

    /**
    * Extracts a value from a KJS::List of KJS::JSValues.  If the argument is out of range the default value
    * is returned.
    */
    template< typename T>
    T KDE_EXPORT extractValue( KJS::ExecState *exec, const KJS::List &args, int idx, const T &defaultValue = T())
    {
        if( args.size() >= idx )
        {
            return extractValue<T>( exec, args[idx], defaultValue );
        }
        else
            return defaultValue;
    }

    /**
    * Can create any known KJSEmbed::ValueBinding object and set the value.
    * On failure a KJS::Null will be returned and the exception set. Only values
    * that are supported by QVariant will work.
    */
    template< typename T>
    KJS::JSValue* createValue(KJS::ExecState *exec, const KJS::UString &className, const T &value)
    {
        KJS::JSObject *parent;
        //if( exec->context().imp() != 0 )
        //	parent = exec->context().thisValue();
        //else
        parent = exec->interpreter()->globalObject();
        KJS::JSObject *returnValue = StaticConstructor::construct( exec, parent, className );
        //qDebug( "Created classname %s" , returnValue.className().ascii() );
        if( returnValue )
        {
            // If its a value type setValue
            KJSEmbed::ValueBinding *imp = extractBindingImp<KJSEmbed::ValueBinding>(exec, returnValue );
            if( imp )
                imp->setValue( qVariantFromValue( value ) );
            else
            {
                throwError(exec, KJS::GeneralError, QString("Created failed to cast to %1 failed").arg(className.qstring()) );
                //throwError(exec, QString("Created failed to cast to %1 failed").arg(className.qstring()) );
                return KJS::Null();
            }
        }
        else
        {
            throwError(exec, KJS::GeneralError, QString("Could not construct a %1").arg(className.qstring() ));
            //throwError(exec, QString("Could not construct a %1").arg(className.qstring() ));
            return KJS::Null();
        }
        return returnValue;
    }

    /**
    * Convert a KJS::JSValue that contains an associative array into a QMap.  If you call
    * this on a normal Javascript object you will get each property as the key and its
    * data as the value.  A normal array will give you a QMap with each key being the
    * index.
    */
    QMap<QString, QVariant> KDE_EXPORT convertArrayToMap( KJS::ExecState *exec, KJS::JSValue *value );

    /**
    * Convert a KJS::JSValue into a QList.  If the list contains only strings, or objects that can be
    * converted to strings you can call to @ref convertArrayToStringList.  If you call this on an
    * associative array or a javascript object the list will return empty.  Unless there happens to be
    * a property with a numeric index present, then all bets are off.
    */
    QList<QVariant> KDE_EXPORT convertArrayToList( KJS::ExecState *exec, KJS::JSValue *value );

    /**
    * Convert a KJS::JSValue inot a QStringList. @see convertArrayToList.
    */
    QStringList KDE_EXPORT convertArrayToStringList( KJS::ExecState *exec, KJS::JSValue *value );

    /**
    * Convert a KJS::JSValue into a QVariant object
    */
    QVariant KDE_EXPORT convertToVariant( KJS::ExecState *exec, KJS::JSValue *value );

    /**
    * Convert a QVariant to a KJS::JSValue.  If the type is a known type the bindings will be added.
    * If the type is supported by QVariant, but is not supported by KJSEmbed then it will just be wrapped.
    * This wrapped value can be used just like normal value bindings, save for the lack of methods available to the object.
    */
    KJS::JSValue* convertToValue( KJS::ExecState *exec, const QVariant &value );

    /**
    * The Bindings for the KJSEmbed::ValueBinding
    */
    struct Method;
    class KDE_EXPORT ValueFactory
    {
        public:
        static const Method ValueMethods[];
        static const Method *methods(){ return ValueMethods;}
    };

}
#endif
