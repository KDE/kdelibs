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
#include "value_binding.h"
#include "static_binding.h"

using namespace KJSEmbed;

const KJS::ClassInfo ValueBinding::info = { "ValueBinding", 0, 0, 0 };

KJS::JSValue *callValueType( KJS::ExecState *exec, KJS::JSObject *self, const KJS::List &args )
{
    Q_UNUSED(args);
    KJSEmbed::ValueBinding *imp = KJSEmbed::extractBindingImp<KJSEmbed::ValueBinding>(exec,  self );
    if( imp )
    {
        return KJS::jsString( imp->className().ascii() );
    }
    return KJS::jsNull();
}

const Method ValueFactory::ValueMethods[] =
{
    {"type", 0, KJS::DontDelete|KJS::ReadOnly, &callValueType },
    //{"cast", 1, KJS::DontDelete|KJS::ReadOnly, &callPointerCast },
    //{"toString", 0, KJS::DontDelete|KJS::ReadOnly, &callPointerToString },
    {0, 0, 0, 0 }
};

const Method *ValueFactory::methods()
{
    return ValueMethods;
}

ValueBinding::ValueBinding( KJS::ExecState *exec, const char *typeName)
    : ProxyBinding( exec ),
      m_value(0),
      m_name(typeName)
{
    StaticBinding::publish( exec, this, ValueFactory::methods() );
}

ValueBinding::~ValueBinding() 
{
    delete m_value;
}

KJS::UString ValueBinding::toString(KJS::ExecState *exec) const
{
    Q_UNUSED(exec);
    return m_value->type().name();
}

//kate: indent-spaces on; indent-width 4; replace-tabs on; indent-mode cstyle;
