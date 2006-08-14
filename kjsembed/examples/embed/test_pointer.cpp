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
#include <QDebug>

#include <kjs/object.h>

#include <kjsembed/object_binding.h>
#include <kjsembed/value_binding.h>

#include "test_pointer.h"

using namespace KJSEmbed;

namespace TestPointerNS
{
START_OBJECT_METHOD( callPrint, Line )
  qDebug() << object->name << " Print:" << object->offset++ << KJSEmbed::extractQString(exec, args, 0 );
END_OBJECT_METHOD

START_STATIC_OBJECT_METHOD( callStaticPrint )
  qDebug() << "Static print:" << KJSEmbed::extractQString(exec, args, 0 );
END_STATIC_OBJECT_METHOD

START_OBJECT_METHOD( callName, Line )
    result = KJS::String( object->name );
END_OBJECT_METHOD

START_OBJECT_METHOD( callSetName, Line )
    object->name = KJSEmbed::extractQString(exec, args, 0 );
END_OBJECT_METHOD
}

START_METHOD_LUT( TestPointer )
    {"print", 1, KJS::DontDelete|KJS::ReadOnly|KJS::DontEnum, &TestPointerNS::callPrint },
    {"name", 1, KJS::DontDelete|KJS::ReadOnly|KJS::DontEnum, &TestPointerNS::callName },
    {"setName", 0, KJS::DontDelete|KJS::ReadOnly|KJS::DontEnum, &TestPointerNS::callSetName }
END_METHOD_LUT

START_ENUM_LUT( TestPointer )
    {"START", Line::START},
    {"END", Line::END}
END_ENUM_LUT

START_STATIC_METHOD_LUT( TestPointer )
    {"print", 1, KJS::DontDelete|KJS::ReadOnly|KJS::DontEnum, &TestPointerNS::callStaticPrint }
END_METHOD_LUT

START_CTOR( TestPointer, Line, 0 )
    Line *line = new Line();
    line->name = "unnamed";
    if( args.size() == 1 )
        line->name = args[0]->toString(exec).qstring();
    KJS::JSObject * object = new KJSEmbed::ObjectBinding(exec, "Line", line );
    StaticBinding::publish( exec, object, TestPointer::methods() );
    return object;
END_CTOR

//kate: indent-spaces on; indent-width 4; replace-tabs on; indent-mode cstyle;
