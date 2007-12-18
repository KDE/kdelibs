/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 2006 Matt Broadstone (mbroadst@gmail.com)
 *            (C) 2007 Maks Orlovich <maksim@kde.org>
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
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */
#include "value2string.h"

#include <kjs/value.h>
#include <kjs/object.h>

using namespace KJS;

namespace KJSDebugger {

QString valueToString(KJS::JSValue* value)
{
    switch(value->type())
    {
        case KJS::NumberType:
        {
            double v = 0.0;
            value->getNumber(v);
            return QString::number(v);
        }
        case KJS::BooleanType:
            return value->getBoolean() ? "true" : "false";
        case KJS::StringType:
        {
            KJS::UString s;
            value->getString(s);
            return '"' + s.qstring() + '"';
        }
        case KJS::UndefinedType:
            return "undefined";
        case KJS::NullType:
            return "null";
        case KJS::ObjectType:
            return "[object " + static_cast<KJS::JSObject*>(value)->className().qstring() +"]";
        case KJS::GetterSetterType:
        case KJS::UnspecifiedType:
        default:
            return QString();
    }
}

}
