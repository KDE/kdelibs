/* This file is part of the KDE libraries
   Copyright (c) Todd.Miller@cs.colorado.edu
   Copyright (c) 2002 Dirk Mueller <mueller@kde.org>
   The source code for strlcpy() and strlcat() is available free of charge and 
   under a BSD-style license as part of the OpenBSD operating system.
*/

#cmakedefine01 HAVE_STRLCPY_PROTO
#if ! HAVE_STRLCPY_PROTO
#ifdef __cplusplus
extern "C" {
#endif
unsigned long strlcpy(char*, const char*, unsigned long);
#ifdef __cplusplus
}
#endif
#endif

#cmakedefine01 HAVE_STRING_H
