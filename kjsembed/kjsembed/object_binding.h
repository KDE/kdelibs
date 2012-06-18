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


#ifndef OBJECT_BINDING_H
#define OBJECT_BINDING_H

#include "static_binding.h"
#include "variant_binding.h"
#include "pointer.h"
#include "kjseglobal.h"

/**
* A simple pointer syle method.
* This will extract the pointer, cast it to the native type and place it in "value".
* Any data that should be returned from this method should be placed into "result";
*
*/
#define START_OBJECT_METHOD( METHODNAME, TYPE) \
KJS::JSValue *METHODNAME( KJS::ExecState *exec, KJS::JSObject *self, const KJS::List &args ) \
{ \
        Q_UNUSED(exec);\
        Q_UNUSED(self);\
        Q_UNUSED(args);\
        KJS::JSValue *result = KJS::jsNull(); \
        KJSEmbed::ObjectBinding *imp = KJSEmbed::extractBindingImp<KJSEmbed::ObjectBinding>(exec, self ); \
        if( imp ) \
        { \
            TYPE *object = imp->object<TYPE>(); \
            if( object ) \
            {

/**
* End a variant method started by START_OBJECT_METHOD
*/
#define END_OBJECT_METHOD \
            } \
            else \
                KJS::throwError(exec, KJS::ReferenceError, QString("O: The internal object died."));\
        } \
        else \
            KJS::throwError(exec, KJS::GeneralError, QString("Object cast failed."));\
        return result; \
}

#define START_STATIC_OBJECT_METHOD( METHODNAME ) \
KJS::JSValue *METHODNAME( KJS::ExecState *exec, KJS::JSObject *self, const KJS::List &args ) \
{\
        Q_UNUSED(exec);\
        Q_UNUSED(self);\
        Q_UNUSED(args);\
        KJS::JSValue *result = KJS::jsNull(); \

#define END_STATIC_OBJECT_METHOD \
        return result; \
}

namespace KJSEmbed
{
    class KJSEMBED_EXPORT ObjectFactory
    {
        public:
            static const Method ObjectMethods[];
            static const Method *methods(){ return ObjectMethods;}
    };

    class KJSEMBED_EXPORT ObjectBinding : public ProxyBinding
    {
        public:
            enum Ownership { CPPOwned, QObjOwned, JSOwned  };
            static const KJS::ClassInfo info;

        private:
            const char *m_name;
            mutable PointerBase *m_value;
            Ownership m_owner;

        public:
            template <typename T>
            ObjectBinding( KJS::ExecState *exec, const char *typeName, T *ptr )
                : ProxyBinding(exec),
                  m_name(typeName)
            {
                StaticBinding::publish( exec, this, ObjectFactory::methods() );

                m_owner = CPPOwned;
                m_value = new Pointer<T>(ptr);
            }

            virtual ~ObjectBinding();

            const char *typeName() const;

            /**
             * \return the internal object as a pointer of type T
             */
            template <typename T>
            T *object() const
            {
                if( m_value )
                    return pointer_cast<T>(m_value);
                else
                    return 0;
            }

            void *voidStar() const
            {
                return m_value->voidStar();
            }

            template <typename T>
            void setObject( T *ptr )
            {
                if( m_owner == JSOwned )
                {
                  //qDebug("object cleans up");
                    m_value->cleanup();
                }
                delete m_value;
                m_value = new Pointer<T>(ptr);
            }

            KJS::UString toString( KJS::ExecState *exec ) const;
            KJS::UString className() const;
            KJS::JSType type() const;

            Ownership ownership() const;
            void setOwnership( Ownership owner );

    };

    /**
    * Extracts a pointer based type from an ObjectBinding object.  Care should be taken that this method
    * is not used with KJSEmbed::ObjectBinding objects because the cast will fail.
    */
    template< typename T>
    T * extractObject( KJS::ExecState *exec, KJS::JSValue *arg, T *defaultValue )
    {
        if( !arg )
            return defaultValue;
        else
        {
            T *returnValue = 0;
            KJSEmbed::ObjectBinding *imp = KJSEmbed::extractBindingImp<KJSEmbed::ObjectBinding>(exec, arg );
            if( imp )
            {
                // GCC 3.3 has problems calling template functions in another class from a template class.
                // returnValue = imp->object<T>();

                returnValue = (T *)imp->voidStar();
            }
            if( returnValue )
                return returnValue;
            else
                return defaultValue;
        }
    }

    /**
    * Extracts a pointer from a KJS::List of KJS::JSValues.  If the argument is out of range the default value
    * is returned.
    */
    template< typename T>
    T * extractObject( KJS::ExecState *exec, const KJS::List &args, int idx, T *defaultValue = 0L)
    {
        if( args.size() > idx )
        {
            return extractObject<T>( exec, args[idx], defaultValue );
        }
        else
            return defaultValue;
    }

    /**
    * Can create any known KJSEmbed::ObjectBinding object and set the value.
    * On failure a KJS::jsNull will be returned and the exception set.
    */
    template< typename T>
    KJS::JSValue *createObject(KJS::ExecState *exec, const KJS::UString &className, const T *value, KJSEmbed::ObjectBinding::Ownership owner = KJSEmbed::ObjectBinding::JSOwned )
    {
        if ( 0 == value )
            return KJS::jsNull();

        KJS::JSObject *parent = exec->dynamicInterpreter()->globalObject();
        KJS::JSObject *returnValue = StaticConstructor::construct( exec, parent, className );
        if( returnValue )
        {
            // If it is a value type setValue
            KJSEmbed::ObjectBinding *imp = extractBindingImp<KJSEmbed::ObjectBinding>(exec, returnValue );
            if( imp )
            {
                imp->setOwnership( KJSEmbed::ObjectBinding::JSOwned );
                imp->setObject( value );
                imp->setOwnership( owner );
            }
            else
            {
	            throwError(exec, KJS::TypeError, i18n("%1 is not an Object type", className.ascii()));
                return KJS::jsNull();
            }
        }
        else
        {
            throwError(exec, KJS::GeneralError, "Could not construct value");
            //throwError(exec, "Could not construct value" );
            return KJS::jsNull();
        }

        return returnValue;
    }

    template< typename T >
    T extractParameter( KJS::ExecState *exec, KJS::JSValue *arg, const T &defaultValue )
    {
	if( !arg )
	    return defaultValue;
	else
	{
            switch (arg->type())
            {
                case KJS::NumberType:
                    return extractInt(exec, arg, defaultValue);
                    break;
                case KJS::BooleanType:
                    return extractBool(exec, arg, 0);
                    break;
                case KJS::UnspecifiedType:
                case KJS::UndefinedType:
                case KJS::NullType:
                case KJS::GetterSetterType:
                case KJS::StringType:
                    return defaultValue;
                    break;
            }

            KJS::JSObject* object = arg->toObject(exec);
            if(object->inherits(&VariantBinding::info))
            {
                return extractVariant<T>(exec, arg, defaultValue);
            }
            else if(object->inherits(&ObjectBinding::info))
            {
                return extractObject<T>(exec, arg, defaultValue);
            }
            else
                return defaultValue;
        }
    }
}
#endif

//kate: indent-spaces on; indent-width 4; replace-tabs on; indent-mode cstyle;
