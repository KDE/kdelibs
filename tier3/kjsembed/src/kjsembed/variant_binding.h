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


#ifndef VARIANT_BINDING_H
#define VARIANT_BINDING_H

#include <QtCore/QVariant>

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
        KJS::JSValue *result = KJS::jsNull(); \
        KJSEmbed::VariantBinding *imp = KJSEmbed::extractBindingImp<KJSEmbed::VariantBinding>(exec, self ); \
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

#define KJSO_VARIANT_SIMPLE_BINDING_CTOR( NAME, JSNAME, TYPE, BASENAME ) \
    NAME::NAME(KJS::ExecState *exec, const char* typeName ) \
      : BASENAME( exec, typeName )                    \
    { \
      StaticBinding::publish( exec, this, NAME::methods() ); \
    } \
    NAME::NAME(KJS::ExecState *exec, const TYPE & value) \
      : BASENAME( exec, QVariant::fromValue(value))							 \
    { \
      StaticBinding::publish( exec, this, NAME::methods() ); \
    }

namespace KJSEmbed
{
    /**
    * QVariant based binding.  This class wraps the implementation.
    * You should never have to use this class directly instead use KJSEmbed::createVariant.
    */

    /**
    * QVariant bindinging implementation.
    */
    class KJSEMBED_EXPORT VariantBinding : public ProxyBinding
    {
        public:
            /**
            * Create a new binding implementation with a QVariant to wrap
            */
            VariantBinding( KJS::ExecState *exec, const QVariant &value );
            virtual ~VariantBinding() {}

            void *pointer();

            KJS::UString toString(KJS::ExecState *) const;
            KJS::UString className() const;

            /**
            * Return the wrapped QVariant
            */
            QVariant variant() const;

            /**
            * Extract the actual value from the wrapper.  This method wraps QVariant::value, so it will support
            * some aspects of casting.  If the cast fails a default value of T will be returned.
            */
            template<typename T>
            T value() const { return m_value.value<T>(); }
            /**
            * Set the internal value of the QVariant.
            */
            void setValue( const QVariant &val );

            /**
            * Constructs a QGenericArgument that is used with QMetaObject::invokeMember
            */
            QGenericArgument arg(const char *type) const;

            static const KJS::ClassInfo info;

        private:
            virtual const KJS::ClassInfo* classInfo() const { return &info; }
            QVariant m_value;

    };

    /**
    * Extracts a QVariant from a KJS::JSValue if the conversion fails a QVariant::Null is returned.
    */
    QVariant KJSEMBED_EXPORT extractVariant( KJS::ExecState *exec, KJS::JSValue *value );

    /**
    * Extracts a value based type from a VariantBinding object.  This method calls @ref extractVariant so if
    * the conversions fails then the default value will be returned.  Care should be taken that this method
    * is not used with KJSEmbed::ObjectBinding objects because the cast will fail.
    */
    template< typename T>
    T extractVariant( KJS::ExecState *exec, KJS::JSValue *arg, const T &defaultValue )
    {
        if( !arg )
            return defaultValue;
        else
        {
            QVariant variant = extractVariant( exec, arg );
            if( !variant.isNull() )
            {
                if( variant.canConvert<T>() )
                    return variant.value<T>();
                else
                {
                    throwError(exec, KJS::TypeError, "Cast failed" );
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
    T extractVariant( KJS::ExecState *exec, const KJS::List &args, int idx, const T &defaultValue = T())
    {
        if( args.size() >= idx )
        {
            return extractVariant<T>( exec, args[idx], defaultValue );
        }
        else
            return defaultValue;
    }

    /**
    * Can create any known KJSEmbed::VariantBinding object and set the value.
    * On failure a KJS::jsNull will be returned and the exception set. Only values
    * that are supported by QVariant will work.
    */
    template< typename T>
    KJS::JSValue* createVariant(KJS::ExecState *exec, const KJS::UString &className, const T &value)
    {
        KJS::JSObject *parent;
        parent = exec->dynamicInterpreter()->globalObject();
        KJS::JSObject *returnValue = StaticConstructor::construct( exec, parent, className );
        if( returnValue )
        {
            // If it is a value type setValue
            KJSEmbed::VariantBinding *imp = extractBindingImp<KJSEmbed::VariantBinding>(exec, returnValue );
            if( imp )
                imp->setValue( qVariantFromValue( value ) );
            else
            {
                throwError(exec, KJS::TypeError, toUString(QString("Created failed to cast to %1 failed").arg(toQString(className)) ));
                return KJS::jsNull();
            }
        }
        else
        {
            throwError(exec, KJS::TypeError, toUString(QString("Could not construct a %1").arg(toQString(className) )));
            return KJS::jsNull();
        }
        return returnValue;
    }

    /**
    * Convert a KJS::JSValue that contains an associative array into a QMap.  If you call
    * this on a normal Javascript object you will get each property as the key and its
    * data as the value.  A normal array will give you a QMap with each key being the
    * index.
    */
    QMap<QString, QVariant> KJSEMBED_EXPORT convertArrayToMap( KJS::ExecState *exec, KJS::JSValue *value );

    /**
    * Convert a KJS::JSValue into a QList.  If the list contains only strings, or objects that can be
    * converted to strings you can call to @ref convertArrayToStringList.  If you call this on an
    * associative array or a javascript object the list will return empty.  Unless there happens to be
    * a property with a numeric index present, then all bets are off.
    */
    QList<QVariant> KJSEMBED_EXPORT convertArrayToList( KJS::ExecState *exec, KJS::JSValue *value );

    /**
    * Convert a KJS::JSValue inot a QStringList. @see convertArrayToList.
    */
    QStringList KJSEMBED_EXPORT convertArrayToStringList( KJS::ExecState *exec, KJS::JSValue *value );

    /**
    * Convert a KJS::JSValue into a QVariant object
    */
    QVariant KJSEMBED_EXPORT convertToVariant( KJS::ExecState *exec, KJS::JSValue *value );

    /**
    * Convert a QVariant to a KJS::JSValue.  If the type is a known type the bindings will be added.
    * If the type is supported by QVariant, but is not supported by KJSEmbed then it will just be wrapped.
    * This wrapped value can be used just like normal value bindings, save for the lack of methods available to the object.
    */
    KJSEMBED_EXPORT KJS::JSValue* convertToValue( KJS::ExecState *exec, const QVariant &value );

    /**
    * The Bindings for the KJSEmbed::VariantBinding
    */
    struct Method;
    class KJSEMBED_EXPORT VariantFactory
    {
        public:
        static const Method VariantMethods[];
        static const Method *methods(){ return VariantMethods;}
    };

}
#endif

//kate: indent-spaces on; indent-width 4; replace-tabs on; indent-mode cstyle;
