#include "type.h"
#include <assert.h>

long Arts::Type::_staticTypeCount = 0;

Arts::Type::~Type()
{
	_staticTypeCount--;
}
