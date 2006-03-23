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
#include "test_object.h"
#include <kjsembed/value_binding.h>

#include <kjs/object.h>
#include <qdebug.h>

using namespace KJSEmbed;

namespace TestObjectNS
{
START_VARIANT_METHOD( callPrint, CustomValue )
    qDebug() << "TestObject Print:" << value.index++ << args[0]->toString(exec).qstring();
END_VARIANT_METHOD
}

START_METHOD_LUT( TestObject )
    {"print", 1, KJS::DontDelete|KJS::ReadOnly|KJS::DontEnum, &TestObjectNS::callPrint }
END_METHOD_LUT

NO_ENUMS( TestObject )

NO_STATICS( TestObject )

START_CTOR( TestObject, TestObject, 1 )
    CustomValue value;
    value.index = 1;
    if( args.size() == 1 )
        value.index = args[0]->toNumber(exec);
    KJS::JSObject *object = new KJSEmbed::ValueBinding(exec, qVariantFromValue(value) ); // For custom variants you must use qVariantFromValue
    StaticBinding::publish( exec, object, TestObject::methods() );
    return object;
END_CTOR
