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


#ifndef VALUE_BINDING_H
#define VALUE_BINDING_H

#include <kjs/object.h>
#include <kjs/interpreter.h>

#include "static_binding.h"
#include "pointer.h"

/**
* A simple variant syle method.
* This will extract the value, cast it to the native type and place it in "value".
* Any data that should be returned from this method should be placed into "result";
*
*/
#define START_VALUE_METHOD( METHODNAME, TYPE) \
KJS::JSValue *METHODNAME( KJS::ExecState *exec, KJS::JSObject *self, const KJS::List &args ) \
{ \
        Q_UNUSED(exec);\
        Q_UNUSED(self);\
        Q_UNUSED(args);\
        KJS::JSValue *result = KJS::jsNull(); \
        KJSEmbed::ValueBinding *imp = KJSEmbed::extractBindingImp<KJSEmbed::ValueBinding>(exec, self ); \
        if( imp ) \
        { \
                TYPE value = imp->value<TYPE>();
/**
* End a variant method started by START_VALUE_METHOD
*/
#define END_VALUE_METHOD \
                imp->setValue(value); \
        } \
        else { \
            KJS::throwError(exec, KJS::GeneralError, "Problem in ValueBinding here");\
        }\
        return result; \
}

#define KJSO_VALUE_SIMPLE_BINDING_CTOR( NAME, JSNAME, TYPE, BASENAME )        \
    NAME::NAME(KJS::ExecState *exec, const char* typeName ) \
      : BASENAME( exec, typeName )                    \
    { \
      StaticBinding::publish( exec, this, NAME::methods() ); \
    } \
    NAME::NAME(KJS::ExecState *exec, const TYPE & value) \
      : BASENAME( exec, #JSNAME , value )                    \
    { \
      StaticBinding::publish( exec, this, NAME::methods() ); \
    }

#define KJSO_VALUE_DERIVED_BINDING_CTOR( NAME, JSNAME, TYPE, BASENAME )  \
    NAME::NAME(KJS::ExecState *exec, const char* typeName ) \
      : BASENAME( exec, typeName )                    \
    { \
      StaticBinding::publish( exec, this, NAME::methods() ); \
    } \
    NAME::NAME(KJS::ExecState *exec, const TYPE & value)                \
    : BASENAME( exec, #JSNAME )                                         \
    {                                                                   \
        setValue(value);                                                \
        StaticBinding::publish( exec, this, NAME::methods() );          \
    }


namespace KJSEmbed
{
   /**
    * The Bindings for the KJSEmbed::ValueBinding
    */
    class ValueFactory
    {
        public:
            static const Method ValueMethods[];
            static const Method *methods();
    };

   /**
    * Value binding implementation.
    */
    class ValueBinding : public ProxyBinding
    {
        public:
            template <typename T>
            ValueBinding( KJS::ExecState *exec, const char *typeName, T val )
                : ProxyBinding( exec ),
                  m_name(typeName)
            {
                m_value = new Value<T>(val);
                StaticBinding::publish( exec, this, ValueFactory::methods() );
            }
            ValueBinding( KJS::ExecState *exec, const char *typeName);
            virtual ~ValueBinding();

            KJS::UString toString(KJS::ExecState *exec) const;
            KJS::UString className() const { return m_name; }

            /**
            * Returns the stored value.
            */
            template< typename T>
            T value() const
            {
                const T *ptr = reinterpret_cast<const T*>(m_value->voidStar());
                if( ptr )
                    return *ptr;
                else
                    return T();
            }

            /**
            * Set the internal value.
            */
            template< typename T>
            void setValue( const T &val )
            {
                delete m_value;
                m_value = new Value<T>(val);
            }

            template< typename T>
            static T castValue( ValueBinding *imp)
            {
                const T *ptr = reinterpret_cast<const T*>( imp->m_value->voidStar() );
                if( ptr )
                    return *ptr;
                else
                    return T();
            }
            static const KJS::ClassInfo info;

        private:
            virtual const KJS::ClassInfo* classInfo() const { return &info; }

            PointerBase *m_value;
            const char *m_name;

    };

     /**
    * Extracts a pointer based type from an ObjectBinding object.  Care should be taken that this method
    * is not used with KJSEmbed::ObjectBinding objects because the cast will fail.
    */
    template< typename T>
    T extractValue( KJS::ExecState *exec, KJS::JSValue *arg, const T &defaultValue )
    {
        if( arg )
        {
            KJSEmbed::ValueBinding *imp =
                    KJSEmbed::extractBindingImp<KJSEmbed::ValueBinding>(exec, arg );
            if( imp )
                return ValueBinding::castValue<T>( imp );
        }
        return defaultValue;
    }

    /**
    * Extracts a pointer from a KJS::List of KJS::Values.  If the argument is out of range the default value
    * is returned.
    */
    template< typename T>
    T extractValue( KJS::ExecState *exec, const KJS::List &args, int idx, const T &defaultValue = T())
    {
        if( args.size() > idx )
        {
            return extractValue<T>( exec, args[idx], defaultValue );
        }
        else
            return defaultValue;
    }

    template< typename T>
    KJS::JSValue *createValue(KJS::ExecState *exec, const KJS::UString &className, const T &value)
    {
        KJS::JSObject *parent = exec->dynamicInterpreter()->globalObject();
        KJS::JSObject *returnValue = StaticConstructor::construct( exec, parent, className );
        if( returnValue )
        {
            // If it is a value type setValue
            KJSEmbed::ValueBinding *imp =
                    extractBindingImp<KJSEmbed::ValueBinding>(exec, returnValue );
            if( imp )
                imp->setValue( value );
            else
            {
                KJS::throwError(exec, KJS::TypeError, toUString(QString("Created failed to cast to %1 failed").arg(toQString(className))));
                return KJS::jsNull();
            }
        }
        else
        {
            KJS::throwError(exec, KJS::TypeError, toUString(QString("Could not construct a %1").arg(toQString(className) )));
            return KJS::jsNull();
        }
        return returnValue;
    }
}

#endif

//kate: indent-spaces on; indent-width 4; replace-tabs on; indent-mode cstyle;
