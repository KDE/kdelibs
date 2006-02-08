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
*  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
*  Boston, MA 02111-1307, USA.
*/

#ifndef OBJECT_BINDING_H
#define OBJECT_BINDING_H

#include <kdemacros.h>

#include "static_binding.h"
#include "value_binding.h"
#include "pointer.h"
#include "global.h"

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
        KJS::JSValue *result = KJS::Null(); \
        KJSEmbed::ObjectBinding *imp = KJSEmbed::extractBindingImp<KJSEmbed::ObjectBinding>(exec, self ); \
        if( imp ) \
        { \
                TYPE *object = imp->object<TYPE>();
/**
* End a variant method started by START_OBJECT_METHOD
*/
#define END_OBJECT_METHOD \
        } \
        else \
            result = KJS::throwError(exec, KJS::GeneralError, QString("Object cast failed."));\
        return result; \
}

// used to be: result = KJSEmbed::throwError(exec, KJS::GeneralError, QString("Object castfailed."));

#define START_STATIC_OBJECT_METHOD( METHODNAME ) \
KJS::JSValue *METHODNAME( KJS::ExecState *exec, KJS::JSObject *self, const KJS::List &args ) \
{\
        Q_UNUSED(exec);\
        Q_UNUSED(self);\
        Q_UNUSED(args);\
        KJS::JSValue *result = KJS::Null(); \

#define END_STATIC_OBJECT_METHOD \
        return result; \
}

namespace KJSEmbed
{
    class KDE_EXPORT ObjectFactory
    {
        public:
            static const Method ObjectMethods[];
            static const Method *methods(){ return ObjectMethods;}
    };

    class KDE_EXPORT ObjectBinding : public KJS::JSObject
    {
        public:
            enum Ownership { CPPOwned, QObjOwned, JSOwned  };

        private:
            const char *m_name;
            PointerBase *m_value;
            Ownership m_owner;

        public:
            template <typename T>
            ObjectBinding( KJS::ExecState *exec, const char *typeName, T *ptr )
                : KJS::JSObject(exec->interpreter()->builtinObjectPrototype()),
                  m_name(typeName)
            {
                StaticBinding::publish( exec, this, ValueFactory::methods() );
                StaticBinding::publish( exec, this, ObjectFactory::methods() );

                m_owner = CPPOwned;
                m_value = new Pointer<T>(ptr);
            }

            virtual ~ObjectBinding();

            bool implementsCall() const { return false; }
            bool implementsConstruct() const { return false; }

            const char *typeName() const;

            template <typename T>
            T *object() const
            {
                if( m_value )
                    return (T*)m_value->voidStar();
                else
                    return 0;
            }

            void *voidStar() const
            {
                return object<void>();
            }

            template <typename T>
            void setObject( T *ptr )
            {
                // if( m_owner == JSOwned )
                // {
                m_value->cleanup();
                // }
                delete m_value;
                m_value = new Pointer<T>(ptr);
            }

            KJS::UString toString( KJS::ExecState *exec ) const;
            KJS::UString className() const;
            KJS::Type type() const;

            Ownership ownership() const;
            void setOwnership( Ownership owner );

            QGenericArgument arg( const char *type) const;

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
                returnValue = (T*)imp->voidStar();
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
    * On failure a KJS::Null will be returned and the exception set.
    */
    template< typename T>
    KJS::JSValue *createObject(KJS::ExecState *exec, const KJS::UString &className, const T *value, KJSEmbed::ObjectBinding::Ownership owner = KJSEmbed::ObjectBinding::JSOwned )
    {
        if ( 0 == value )
            return KJS::Null();

        KJS::JSObject *parent = exec->interpreter()->globalObject();
        KJS::JSObject *returnValue = StaticConstructor::construct( exec, parent, className );
        if( returnValue )
        {
            // If its a value type setValue
            KJSEmbed::ObjectBinding *imp = extractBindingImp<KJSEmbed::ObjectBinding>(exec, returnValue );
            if( imp )
            {
                imp->setObject( value );
                imp->setOwnership( owner );
            }
            else
            {
                throwError(exec, KJS::GeneralError, i18n("%1 is not an Object type").arg(className.ascii()));
                //throwError(exec, i18n("%1 is not an Object type").arg(className.ascii()));
                return KJS::Null();
            }
        }
        else
        {
            throwError(exec, KJS::GeneralError, "Could not construct value");
            //throwError(exec, "Could not construct value" );
            return KJS::Null();
        }

        return returnValue;
    }

}
#endif
