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

#include "object_binding.h"

#include <QtCore/QArgument>
#include <QtCore/QDebug>

#include "static_binding.h"
#include "variant_binding.h"

using namespace KJSEmbed;

const KJS::ClassInfo ObjectBinding::info = { "ObjectBinding", 0, 0, 0 };

ObjectBinding::~ObjectBinding()
{
    if( m_owner == JSOwned )
    {
        m_value->cleanup();
    }

    delete m_value;
}

const char *ObjectBinding::typeName() const
{
    return m_name;
}

KJS::UString ObjectBinding::toString(KJS::ExecState * /*exec*/) const
{
    return KJS::UString( typeName() );
}

KJS::UString ObjectBinding::className() const
{
    return KJS::UString( typeName() );
}

KJS::JSType ObjectBinding::type() const
{
    return KJS::ObjectType;
}

ObjectBinding::Ownership ObjectBinding::ownership() const
{
    return m_owner;
}

void ObjectBinding::setOwnership( ObjectBinding::Ownership owner )
{
    m_owner = owner;
}

KJS::JSValue *callPointerName( KJS::ExecState *exec, KJS::JSObject *self, const KJS::List &/*args*/ )
{
    KJSEmbed::ObjectBinding *imp = KJSEmbed::extractBindingImp<KJSEmbed::ObjectBinding>(exec,  self );
    if( imp )
    {
        return KJS::jsString( imp->typeName() );
    }
    return KJS::jsNull();
}

KJS::JSValue *callPointerCast( KJS::ExecState *exec, KJS::JSObject *self, const KJS::List &/*args*/ )
{
    KJSEmbed::ObjectBinding *imp = KJSEmbed::extractBindingImp<KJSEmbed::ObjectBinding>(exec,  self );
    if( imp )
    {
        return KJS::jsBoolean(false);
    }
    return KJS::jsNull();
}

KJS::JSValue *callPointerToString( KJS::ExecState *exec, KJS::JSObject *self, const KJS::List &/*args*/ )
{
    KJSEmbed::ObjectBinding *imp = KJSEmbed::extractBindingImp<KJSEmbed::ObjectBinding>(exec,  self );
    if( imp )
    {
        qDebug("Object to string");
        return KJS::jsString( imp->typeName() );
    }
    return KJS::jsNull();
}

const Method ObjectFactory::ObjectMethods[] =
{
    {"type", 0, KJS::DontDelete|KJS::ReadOnly, &callPointerName },
    {"cast", 1, KJS::DontDelete|KJS::ReadOnly, &callPointerCast },
    {"toString", 0, KJS::DontDelete|KJS::ReadOnly, &callPointerToString },
    {0, 0, 0, 0 }
};

//kate: indent-spaces on; indent-width 4; replace-tabs on; indent-mode cstyle;
