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
#include "scalar_binding.h"
#include "static_binding.h"

using namespace KJSEmbed;

const KJS::ClassInfo ScalarBinding::info = { "ScalarBinding", 0, 0, 0 };

KJS::JSValue *callScalarType( KJS::ExecState *exec, KJS::JSObject *self, const KJS::List &args )
{
    KJSEmbed::ScalarBinding *imp = KJSEmbed::extractBindingImp<KJSEmbed::ScalarBinding>(exec,  self );
    if( imp )
    {
        return KJS::String( imp->className().ascii() );
    }
    return KJS::Null();
}

const Method ScalarFactory::ScalarMethods[] =
{
    {"type", 0, KJS::DontDelete|KJS::ReadOnly, &callScalarType },
    //{"cast", 1, KJS::DontDelete|KJS::ReadOnly, &callPointerCast },
    //{"toString", 0, KJS::DontDelete|KJS::ReadOnly, &callPointerToString },
    {0, 0, 0, 0 }
};

const Method *ScalarFactory::methods()
{
    return ScalarMethods;
}

KJS::UString ScalarBinding::toString(KJS::ExecState *exec) const
{
    return m_value->type().name();
}
