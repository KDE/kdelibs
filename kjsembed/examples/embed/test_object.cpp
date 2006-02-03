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
