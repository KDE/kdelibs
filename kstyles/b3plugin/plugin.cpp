#include "b3style.h"
#include <klocale.h>

extern "C" {
    KStyle* allocate();
    int minor_version();
    int major_version();
    const char *description();
}

KStyle* allocate()
{
    return(new B3Style);
}

int minor_version()
{
    return(0);
}

int major_version()
{
    return(1);
}

const char *description()
{
    return(i18n("B3/K unthemed plugin").utf8());
}
