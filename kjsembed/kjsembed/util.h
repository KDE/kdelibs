#ifndef UTIL_H
#define UTIL_H

namespace KJSEmbed
{
    bool isBasic(const KJS::Value &val)
    {
        switch (val.type())
        {


        }

    }


    bool isVariant(KJS::JSObject *object)
    {
        if (object->inherits(ValueBinding::info))
            return true;
        return false;
    }

    bool isScalar(KJS::JSObject *object)
    {
        if (object->inherits(ScalarBinding::info))
            return true;
        return false;
    }

    bool isObject(KJS::JSObject *object)
    {
        if (object->inherits(ObjectBinding::info))
            return true;
        return false;
    }

}


#endif

