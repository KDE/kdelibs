#include "htmlhashes.h"

#include "htmltags.c"
#include "htmlattrs.c"


int khtml::getTagID(const char *tagStr, int len)
{
    const struct tags *tagPtr = findTag(tagStr, len);
    if (!tagPtr)
        return 0;

    return tagPtr->id;
}

int khtml::getAttrID(const char *tagStr, int len)
{
    const struct attrs *tagPtr = findAttr(tagStr, len);
    if (!tagPtr)
        return 0;

    return tagPtr->id;
}

