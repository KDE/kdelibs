/* This file is part of the KDE libraries
   Copyright (c) Todd.Miller@cs.colorado.edu
   Copyright (c) 2002 Dirk Mueller <mueller@kde.org>
   The source code for strlcpy() and strlcat() is available free of charge and 
   under a BSD-style license as part of the OpenBSD operating system.
*/

#include <config-strlcpy.h>
#include <kdecore_export.h>

#ifndef HAVE_STRLCPY

#if HAVE_STRING_H
#include <string.h>
#endif

KDECORE_EXPORT unsigned long strlcpy(char* d, const char* s, unsigned long bufsize)
{
    unsigned long len, ret = strlen(s);

    if (ret >= bufsize) {
        if (bufsize) {
            len = bufsize - 1;
            memcpy(d, s, len);
            d[len] = '\0';
        }
    } else
        memcpy(d, s, ret + 1);

    return ret;
}
#endif /* !HAVE_STRLCPY */

