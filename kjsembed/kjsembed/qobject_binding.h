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


#ifndef QOBJECT_BINDING_H
#define QOBJECT_BINDING_H

#include <QObjectCleanupHandler>
#include <QDebug>
#include <qglobal.h>

#include <kjs/function.h>
#include <kdemacros.h>

#include "binding_support.h"
#include "object_binding.h"

/**
* A simple pointer syle method.
* This will extract the pointer, cast it to the native type and place it in "value".
* Any data that should be returned from this method should be placed into "result";
*
*/
#define START_QOBJECT_METHOD( METHODNAME, TYPE) \
KJS::JSValue *METHODNAME( KJS::ExecState *exec, KJS::JSObject *self, const KJS::List &args ) \
{ \
        Q_UNUSED( args ); \
        KJS::JSValue *result = KJS::Null(); \
        KJSEmbed::ObjectBinding *imp = KJSEmbed::extractBindingImp<KJSEmbed::QObjectBinding>(exec, self ); \
        if( imp ) \
        { \
            TYPE *object = imp->object<TYPE>(); \
            if( object ) \
            {

/**
* End a variant method started by START_QOBJECT_METHOD
*/
#define END_QOBJECT_METHOD \
            } \
            else \
                KJS::throwError(exec, KJS::ReferenceError, QString("The internal object died."));\
        } \
        else \
           KJS::throwError(exec, KJS::ReferenceError, QString("QObject died."));\
        return result; \
}

class QObject;
class QMetaMethod;

namespace KJS {
}

namespace KJSEmbed {

KJS_BINDING( QObjectFactory )

class EventProxy;
class KJSEMBED_EXPORT QObjectBinding : public ObjectBinding
{
    public:
        QObjectBinding( KJS::ExecState *exec, QObject *object );
        virtual ~QObjectBinding();

        static void publishQObject( KJS::ExecState *exec, KJS::JSObject *target, QObject *object);

        void put(KJS::ExecState *exec, const KJS::Identifier &propertyName, KJS::JSValue *value, int attr=KJS::None);
        bool canPut(KJS::ExecState *exec, const KJS::Identifier &propertyName) const;

	/**
	 * Called to ask if we have a callback for the named property.
	 * We return the callback in the property slot.
	 */
	bool getOwnPropertySlot( KJS::ExecState *exec, const KJS::Identifier &propertyName, KJS::PropertySlot &slot );

	/**
	 * Callback used to get properties.
	 */
	static KJS::JSValue *propertyGetter( KJS::ExecState *exec, KJS::JSObject*, const KJS::Identifier& name, const KJS::PropertySlot& );

        KJS::UString toString(KJS::ExecState *exec) const;
        KJS::UString className() const;

        void watchObject( QObject *object );

    private:
        EventProxy *m_evproxy;
        QObjectCleanupHandler *m_cleanupHandler;
};

class KJSEMBED_EXPORT SlotBinding : public KJS::InternalFunctionImp
{
    public:
        SlotBinding(KJS::ExecState *exec, const QMetaMethod &memberName);
        KJS::JSValue *callAsFunction( KJS::ExecState *exec, KJS::JSObject *self, const KJS::List &args );
        bool implementsCall() const { return true; }
        bool implementsConstruct() const { return false; }

    protected:
        QByteArray m_memberName;
};

/**
* Returns a binding object for the specified QObject. This method walks
* up the meta objects in order to find the most specific binding it can.
* There should always be some kind of binding possible even if it is just
* the QObject binding.
*/
template< typename T>
KJS::JSObject *createQObject(KJS::ExecState *exec, T *value, KJSEmbed::ObjectBinding::Ownership owner = KJSEmbed::ObjectBinding::JSOwned)
{
    if ( 0 == value )
        return new KJS::JSObject();
    
    const QMetaObject *meta = value->metaObject();
    KJS::JSObject *parent = exec->dynamicInterpreter()->globalObject();
    KJS::JSObject *returnValue;

    QString clazz;
    do
    {
        clazz = meta->className();
        if ( parent->hasProperty( exec, KJS::Identifier(clazz) ) )
        {
            returnValue = StaticConstructor::construct( exec, parent, clazz );
            if( returnValue )
            {
                // If its a value type setValue
                KJSEmbed::QObjectBinding *imp = extractBindingImp<KJSEmbed::QObjectBinding>(exec, returnValue );
                if( imp )
                {
                    imp->setObject( value );
                    imp->watchObject( value );
                    imp->setOwnership( owner );
                    KJSEmbed::QObjectBinding::publishQObject( exec, returnValue, value);
                }
                else
                {
	                KJS::throwError(exec, KJS::TypeError, i18n("%1 is not an Object type",  clazz ));
                    return new KJS::JSObject();
                }
            }
            else
            {
                throwError(exec, KJS::TypeError, i18n("Could not construct value"));
                return new KJS::JSObject();
            }
            return returnValue;
        }
        else
        {
            qDebug("%s not a bound type, move up the chain", meta->className() );
            meta = meta->superClass();
        }

    }
    while( meta );

    KJSEmbed::QObjectBinding *imp = new KJSEmbed::QObjectBinding(exec, value);
    imp->setOwnership( owner );

    return imp;
}


}
#endif

//kate: indent-spaces on; indent-width 4; replace-tabs on; indent-mode cstyle;
