#ifndef UTIL_H
#define UTIL_H

namespace KJSEmbed
{
    bool isBasic(KJS::Value *value)
    {
        switch (value->type())
        {
            case KJS::NumberType:
            case KJS::BooleanType:
            case KJS::StringType:
                return true;
                break;
            default:
                return false;

        }
    }


    bool isVariant(KJS::JSObject *object)
    {
        return object->inherits(ValueBinding::info);
    }

    bool isScalar(KJS::JSObject *object)
    {
        return object->inherits(ScalarBinding::info);
    }

    bool isObject(KJS::JSObject *object)
    {
        object->inherits(ObjectBinding::info);
    }

}


#endif

