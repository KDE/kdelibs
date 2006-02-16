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

#ifndef SCALAR_BINDING_H
#define SCALAR_BINDING_H

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
#define START_SCALAR_METHOD( METHODNAME, TYPE) \
KJS::JSValue *METHODNAME( KJS::ExecState *exec, KJS::JSObject *self, const KJS::List &args ) \
{ \
        Q_UNUSED(exec);\
        Q_UNUSED(self);\
        Q_UNUSED(args);\
        KJS::JSValue *result = KJS::Null(); \
        KJSEmbed::ScalarBinding *imp = KJSEmbed::extractBindingImp<KJSEmbed::ScalarBinding>(exec, self ); \
        if( imp ) \
        { \
                TYPE value = imp->value<TYPE>();
/**
* End a variant method started by START_SCALAR_METHOD
*/
#define END_SCALAR_METHOD \
                imp->setValue(value); \
        } \
        else { \
            KJS::throwError(exec, KJS::GeneralError, "Problem in ScalarBinding here");\
        }\
        return result; \
}

namespace KJSEmbed
{
   /**
    * The Bindings for the KJSEmbed::ScalarBinding
    */
    class ScalarFactory
    {
        public:
            static const Method ScalarMethods[];
            static const Method *methods();
    };

   /**
    * QVariant bindinging implementation.
    */
    class ScalarBinding : public KJS::JSObject
    {
        public:
            template <typename T>
            ScalarBinding( KJS::ExecState *exec, const char *typeName, T val )
                : KJS::JSObject(exec->interpreter()->builtinObjectPrototype()),
                  m_name(typeName)
            {
                m_value = new Scalar<T>(val);
                StaticBinding::publish( exec, this, ScalarFactory::methods() );
            }
            virtual ~ScalarBinding() {delete m_value;}

            bool implementsCall() const { return false; }
            bool implementsConstruct() const { return false; }
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
            * Set the internal value..
            */
            template< typename T>
            void setValue( const T &val )
            {
                delete m_value;
                m_value = new Scalar<T>(val);
            }

            template< typename T>
            static T castValue( ScalarBinding *imp)
            {
                const T *ptr = reinterpret_cast<const T*>( imp->m_value->voidStar() );
                if( ptr )
                    return *ptr;
                else
                    return T();
            }

        private:
            PointerBase *m_value;
            const char *m_name;

    };

     /**
    * Extracts a pointer based type from an ObjectBinding object.  Care should be taken that this method
    * is not used with KJSEmbed::ObjectBinding objects because the cast will fail.
    */
    template< typename T>
    T extractScalar( KJS::ExecState *exec, KJS::JSValue *arg, const T &defaultValue )
    {
        if( arg )
        {
            KJSEmbed::ScalarBinding *imp =
                    KJSEmbed::extractBindingImp<KJSEmbed::ScalarBinding>(exec, arg );
            if( imp )
                return ScalarBinding::castValue<T>( imp );
        }
        return defaultValue;
    }

    /**
    * Extracts a pointer from a KJS::List of KJS::Values.  If the argument is out of range the default value
    * is returned.
    */
    template< typename T>
    T extractScalar( KJS::ExecState *exec, const KJS::List &args, int idx, const T &defaultValue = T())
    {
        if( args.size() > idx )
        {
            return extractScalar<T>( exec, args[idx], defaultValue );
        }
        else
            return defaultValue;
    }

    template< typename T>
    KJS::JSValue *createScalar(KJS::ExecState *exec, const KJS::UString &className, const T &value)
    {
        KJS::JSObject *parent = exec->interpreter()->globalObject();
        KJS::JSObject *returnValue = StaticConstructor::construct( exec, parent, className );
        if( returnValue )
        {
            // If its a value type setValue
            KJSEmbed::ScalarBinding *imp =
                    extractBindingImp<KJSEmbed::ScalarBinding>(exec, returnValue );
            if( imp )
                imp->setValue( value );
            else
            {
                KJS::throwError(exec, KJS::GeneralError, QString("Created failed to cast to %1 failed").arg(className.qstring()) );
                //throwError(exec, QString("Created failed to cast to %1 failed").arg(className.qstring()) );
                return KJS::Null();
            }
        }
        else
        {
            KJS::throwError(exec, KJS::GeneralError, QString("Could not construct a %1").arg(className.qstring() ));
            // throwError(exec, QString("Could not construct a %1").arg(className.qstring() ));
            return KJS::Null();
        }
        return returnValue;
    }
}

#endif
