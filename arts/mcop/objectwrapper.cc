#include "common.h"
#include "objectwrapper.h"

Object_base* Object::_Creator() {
	return Object_base::_create();
}

// For technical reasons, this is here, too: FlowSystem isn't declared when
// the wrapper is already needed, so we can't make this function inline,
// unfortunately...

FlowSystem Object::_flowSystem()
{
	return _method_call()->_flowSystem();
}
