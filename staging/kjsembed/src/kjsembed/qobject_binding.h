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


#ifndef QOBJECT_BINDING_H
#define QOBJECT_BINDING_H

#include <QtCore/QObjectCleanupHandler>
#include <QtCore/QDebug>

#include <kjs/function.h>

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
        KJS::JSValue *result = KJS::jsNull(); \
        KJSEmbed::QObjectBinding *imp = KJSEmbed::extractBindingImp<KJSEmbed::QObjectBinding>(exec, self ); \
        if( imp ) \
        { \
            TYPE *object = imp->qobject<TYPE>(); \
            if( object ) \
            {

/**
* End a variant method started by START_QOBJECT_METHOD
*/
#define END_QOBJECT_METHOD \
            } \
            else \
                KJS::throwError(exec, KJS::ReferenceError, QString("QO: The internal object died %1:%2.").arg(__FILE__).arg(__LINE__));\
        } \
        else \
           KJS::throwError(exec, KJS::ReferenceError, QString("QObject died."));\
        return result; \
}

class QObject;
class QMetaMethod;

namespace KJSEmbed {

KJS_BINDING( QObjectFactory )

class EventProxy;

class KJSEMBED_EXPORT QObjectBinding : public ObjectBinding
{
    public:

        QObjectBinding( KJS::ExecState *exec, QObject *object );
        virtual ~QObjectBinding();

        static void publishQObject( KJS::ExecState *exec, KJS::JSObject *target, QObject *object);

        /**
        * Enumeration of access-flags that could be OR-combined to define
        * what parts of the QObject should be published.
        * Default is AllSlots|AllSignals|AllProperties|AllObjects what
        * means that everything got published, even e.g. private slots.
        */
        enum Access {
            None = 0x00, ///< Don't publish anything.

            ScriptableSlots = 0x01, ///< Publish slots that have Q_SCRIPTABLE defined.
            NonScriptableSlots = 0x02, ///< Publish slots that don't have Q_SCRIPTABLE defined.
            PrivateSlots = 0x04, ///< Publish private slots.
            ProtectedSlots = 0x08, ///< Publish protected slots.
            PublicSlots = 0x10, ///< Publish public slots.
            AllSlots = ScriptableSlots|NonScriptableSlots|PrivateSlots|ProtectedSlots|PublicSlots,

            ScriptableSignals = 0x100, ///< Publish signals that have Q_SCRIPTABLE defined.
            NonScriptableSignals = 0x200, ///< Publish signals that don't have Q_SCRIPTABLE defined.
            PrivateSignals = 0x400, ///< Publish private signals.
            ProtectedSignals = 0x800, ///< Publish protected signals.
            PublicSignals = 0x1000, ///< Publish public signals.
            AllSignals = ScriptableSignals|NonScriptableSignals|PrivateSignals|ProtectedSignals|PublicSignals,

            ScriptableProperties = 0x10000, ///< Publish properties that have Q_SCRIPTABLE defined.
            NonScriptableProperties = 0x20000, ///< Publish properties that don't have Q_SCRIPTABLE defined.
            AllProperties = ScriptableProperties|NonScriptableProperties,

            GetParentObject = 0x100000, ///< Provide access to the parent QObject the QObject has.
            SetParentObject = 0x200000, ///< Be able to set the parent QObject the QObject has.
            ChildObjects = 0x400000, ///< Provide access to the child QObject's the QObject has.
            AllObjects = GetParentObject|SetParentObject|ChildObjects
        };

        Q_DECLARE_FLAGS(AccessFlags, Access)

        /**
        * \return the defined \a Access flags.
        */
        AccessFlags access() const;

        /**
        * Set the defined \a Access flags to \p access .
        */
        void setAccess(AccessFlags access);

        /**
        * Set the value \p value of the property \p propertyName .
        */
        void put(KJS::ExecState *exec, const KJS::Identifier &propertyName, KJS::JSValue *value,
                int attr=KJS::None) Q_DECL_OVERRIDE;
        using JSObject::put;

        /**
        * \return true if the property \p propertyName can be changed else false is returned.
        */
        bool canPut(KJS::ExecState *exec, const KJS::Identifier &propertyName) const;

        /**
        * Called to ask if we have a callback for the named property.
        * We return the callback in the property slot.
        */
        bool getOwnPropertySlot( KJS::ExecState *exec, const KJS::Identifier &propertyName,
                KJS::PropertySlot &slot ) Q_DECL_OVERRIDE;
        using JSObject::getOwnPropertySlot;

        /**
        * Callback used to get properties.
        */
        static KJS::JSValue *propertyGetter( KJS::ExecState *exec, KJS::JSObject*, const KJS::Identifier& name, const KJS::PropertySlot& );

        /**
        * \return a string-representation of the QObject. For example for a QWidget-instance that
        * has the QObject::objectName "mywidget" the string "mywidget (QWidget)" is returned.
        */
        KJS::UString toString(KJS::ExecState *exec) const;

        /**
        * \return the QObject's classname. For example for a QWidget-instance the string "QWidget"
        * is returned.
        */
        KJS::UString className() const;

        /**
        * Add the QObject \p object to the internal QObjectCleanupHandler to watch the
        * lifetime of the QObject to know when the QObject got deleted.
        */
        void watchObject( QObject *object );

        /**
         * \return the internal object as a pointer to type T to the
         * internal object that is derived from QObject.
         */
        template <typename T>
        T *qobject() const
        {
          QObject* object = QObjectBinding::object<QObject>();
          if (object)
            return qobject_cast<T*>(object);
          else
            return 0;
        }

    private:
        EventProxy *m_evproxy;
        QObjectCleanupHandler *m_cleanupHandler;
        AccessFlags m_access;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QObjectBinding::AccessFlags)

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
*
* \param exec Represents the current state of script execution.
* \param value The QObject or from it inherited instance we should return
* a binding object for.
* \param owner Defines who's the owner of the QObject. This could be;
*     \li CPPOwned what means, that the QObject's lifetime is handled
*         within C++ code. So, we just provide access to it and don't
*         take any future actions.
*     \li QObjOwned means that the QObject got deleted if the parent QObject
*         is destroyed. If the QObject has no parent QObject, it behaves like
*         JSOwned.
*     \li JSOwned means, that the returned KJS::JSObject takes care of
*         deleting the QObject. This means, that the QObject got deleted
*         as soon as the KJS::JSObject got destroyed what happens if the
*         KJS::JSObject is not needed / in use any longer.
* \return the binding object instance that wraps the QObject instance or
* a JSObject with a prototype of jsNull (that is, the ECMAScript "null" value,
* not a null object pointer) if we failed to provide any binding for it.
*/
KJSEMBED_EXPORT KJS::JSObject *createQObject(KJS::ExecState *exec, QObject *value, KJSEmbed::ObjectBinding::Ownership owner = KJSEmbed::ObjectBinding::JSOwned);


}
#endif

//kate: indent-spaces on; indent-width 4; replace-tabs on; indent-mode cstyle;
