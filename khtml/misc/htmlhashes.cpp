#include "htmlhashes.h"

#include "khtmltags.c"
#include "khtmlattrs.c"


int getTagID(const char *tagStr, int len)
{
    const struct tags *tagPtr = findTag(tagStr, len);
    if (!tagPtr)
        return 0;

    return tagPtr->id;
}

int getAttrID(const char *tagStr, int len)
{
    const struct attrs *tagPtr = findAttr(tagStr, len);
    if (!tagPtr)
        return 0;

    return tagPtr->id;
}

