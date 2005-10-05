/* This file is part of the KDE libraries
   Copyright (C) 2004 Jaroslaw Staniek <js@iidea.pl>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <malloc.h>
#include <stdlib.h>

// .so --> .dll name mappings:
// name suffixes *so.[0-9] are removed
// "lib" prefix (if exists) is removed from the filename
void win32_mapSo2Dll( char *path )
{
    int len;
    char *p, *from;
    assert(path);
    len=strlen(path);
    if (len<=3)
        return;
    // (optionally) remove .[0-9] suffix
    p = path+len-2;
    if (p[0]=='.' && p[1]>='0' && p[1]<='9') {
        *p = 0;
        len -= 2;
    }
    // remove "lib"
    p = strrchr( path, '\\' );
    if (p && len>=3 && strncmp(p,"\\lib", 4)==0) {
        p++;
        from = p + 3;
        for (; *from; p++, from++) {
           *p = *from;
        }
        *p = 0;
        len -= 3;
    }
    //.so -> .dll
    if (len>3 && strncmp(path+len-3,".so",3)==0) {
#ifndef QT_NO_DEBUG //debug library version
        strcpy(path+len-3, "_d");
        len += 2;
#endif
        strcpy(path+len-3, ".dll");
    }
    fprintf(stderr,"win32_mapSo2Dll: '%s' )\n", path );
}

#define MAX_PATH 0x1ff
static char win32_mapDir_KDEDIR[MAX_PATH] = "";

// paths name mappings
// when mapping is performed, frees old name at *dir and allocates new path for *dir 
void win32_mapDir( char **dir )
{
    static const char* WIN32_LIBDIR_FROM = "/opt/kde3/lib/kde3";
    static const char* WIN32_LIBDIR_TO = "c:/kde/lib/kde3";
    char *e;
//TODO........
    if (!*win32_mapDir_KDEDIR) {
      e = getenv("KDEDIR");
      if (e)
        strncpy( win32_mapDir_KDEDIR, e, MAX_PATH );
    }
    assert(dir && *dir && win32_mapDir_KDEDIR && *win32_mapDir_KDEDIR);
    // /opt/kde3/lib/kde3 -> <letter>:/kde/lib/kde3

    if (strcmp(*dir, WIN32_LIBDIR_FROM)==0) {
        free(*dir);
        *dir=strdup(WIN32_LIBDIR_TO);
		if (win32_mapDir_KDEDIR && *win32_mapDir_KDEDIR) {
			(*dir)[0]=win32_mapDir_KDEDIR[0]; //copy drive letter
		}
    }
}

// converts slashes to backslashes for path
void win32_backslashify( char *path )
{
  char *p = path;
  while (*p) {
    if (*p=='/')
      *p = '\\';
    p++;
  }
}
