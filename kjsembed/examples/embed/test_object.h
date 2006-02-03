#ifndef TEST_OBJECT_H
#define TEST_OBJECT_H

#include <QVariant>	

#include <kjsembed/static_binding.h>

/**
* Simple value object
*/
class CustomValue
{
public:
    int index;
};

Q_DECLARE_METATYPE(CustomValue) // Make it into a QVariant.

KJS_BINDING( TestObject )

#endif
