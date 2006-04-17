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

/*
So this might be how the val checking would go:
    JSValue *arg = args[0];
    if (isBasic(arg))
    {
        switch (arg->type())
        {
            case KJS::Number:
                extractInt...;
                break;
            case KJS::String:
                extractString...;
                break;
            case KJS::Boolean:
                extractBool...;
                break;
            default: hmmm
        }
    }
    else
    {
        JSObject *object = arg->toObject(exec);
        if (isScalar(object))
        {
            extractScalar...
        }
        else if (isObject(object))
        {
            extractObject...
        }
        else
        {
            hmmm
        }
    }




*/

}


#endif

