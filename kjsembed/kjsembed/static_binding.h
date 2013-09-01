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

#ifndef STATIC_BINDING_H
#define STATIC_BINDING_H
#include <QtCore/QHash>

#include <kjs/function.h>

#include "binding_support.h"

#define LengthFlags KJS::DontDelete|KJS::ReadOnly|KJS::DontEnum

namespace KJSEmbed
{
/**
* A binding method that is used in VariantBinding and ObjectBinding
*/
class KJSEMBED_EXPORT StaticBinding : public KJS::InternalFunctionImp
{
public:
    /**
    * Create a new method.
    */
    StaticBinding(KJS::ExecState *exec, const Method *method );
    /**
    * Executes the callback for this method.
    */
    KJS::JSValue *callAsFunction( KJS::ExecState *exec, KJS::JSObject *self, const KJS::List &args );
    bool implementsConstruct() const { return false; }

    /**
    * Publishes an array of Methods to an object.  You should only ever need this method
    * to add methods to a binding.
    * @param object the object to add the methods to
    * @param methods an array of Method objects.
    */
    static void publish( KJS::ExecState *exec, KJS::JSObject *object, const Method *methods );

protected:
    const Method *m_method;
};

/**
* A special method that will create other objects. If you want to have your binding to be able to create
* instances of itself it must have at least one of these objects published at the global scope.
*/
class KJSEMBED_EXPORT StaticConstructor : public KJS::InternalFunctionImp
{
public:
    /**
    * Create a new constructor
    */
    StaticConstructor(KJS::ExecState *exec, const Constructor *constructor );

    /**
    * Add static methods to the object.
    * @code
    * KJS::JSObject *ctor = StaticConstructor::add( exec, parent, TestPointer::constructor() ); // Ctor
    * ctor.addStaticMethods( exec, TestPointer::staticMethods() );
    * @endcode
    */
    void addStaticMethods( KJS::ExecState *exec, const Method *methods );


    bool implementsConstruct() const { return true; }
    /**
    * Calls the callback that will in turn create a new instance of this object with
    * the arguments passed in with args.
    */
    KJS::JSObject *construct( KJS::ExecState *exec, const KJS::List &args ) Q_DECL_OVERRIDE;
    using KJS::JSObject::construct;

    KJS::JSValue *callAsFunction( KJS::ExecState *exec, KJS::JSObject * /*self*/, const KJS::List &args ) {
        return construct(exec, args);
    }

    void setDefaultValue( KJS::JSValue *value );
    KJS::JSValue *defaultValue( KJS::ExecState *exec, KJS::JSType hint ) const;

    /**
    * Add the constructor to an object.  This is usually the global scope.
    */
    static KJS::JSObject *add( KJS::ExecState *exec, KJS::JSObject *object, const Constructor *constructor );
    /**
    * This method is used to construct a KJS value from C++
    * @code
    * KJS::List args;
    * args.append("test");
    * KJS::Value myType = KJSEmbed::construct(exec, "MyType", args);
    * @endcode
    * is equivelent to the following javascript
    * @code
    * var myType = new MyType("test");
    * @endcode
    */
    static KJS::JSObject *construct( KJS::ExecState *exec, KJS::JSObject *parent,
            const KJS::UString &className, const KJS::List &args = KJS::List() );

    static KJS::JSObject* bind(KJS::ExecState* exec, const QString &className, PointerBase& objPtr);
    static const Method *methods( const KJS::UString &className );
    static const Constructor *constructor( const KJS::UString &className );

protected:
    const Constructor *m_constructor;

private:
    KJS::JSValue *m_default;

};

}

#endif

//kate: indent-spaces on; indent-width 4; replace-tabs on; indent-mode cstyle;
