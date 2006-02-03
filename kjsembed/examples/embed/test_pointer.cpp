#include <QDebug>

#include <kjs/object.h>

#include <kjsembed/object_binding.h>
#include <kjsembed/value_binding.h>

#include "test_pointer.h"

using namespace KJSEmbed;

namespace TestPointerNS
{
START_OBJECT_METHOD( callPrint, Line )
    qDebug() << object->name << " Print:" << object->offset++ << args[0]->toString(exec).qstring();
END_OBJECT_METHOD

START_STATIC_OBJECT_METHOD( callStaticPrint )
    qDebug() << "Static print:" << args[0]->toString(exec).qstring();
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


