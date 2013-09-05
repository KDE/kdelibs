/* This file is part of the KDE libraries
   Copyright (c) Todd C. Miller <Todd.Miller@courtesan.com>
   Copyright (c) 2002 Dirk Mueller <mueller@kde.org>

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

   1. Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
   2. Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.

   THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
   IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
   OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
   IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
   INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
   NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
   THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <config-strlcpy.h>
#include <kcrash_export.h>

#ifndef HAVE_STRLCPY

#if HAVE_STRING_H
#include <string.h>
#endif

KCRASH_EXPORT unsigned long strlcpy(char* d, const char* s, unsigned long bufsize)
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

