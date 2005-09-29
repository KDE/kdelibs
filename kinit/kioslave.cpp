/*
 * This file is part of the KDE libraries
 * Copyright (c) 1999-2000 Waldo Bastian <bastian@kde.org>
 *           (c) 1999 Mario Weilguni <mweilguni@sime.com>
 *           (c) 2001 Lubos Lunak <l.lunak@kde.org>
 *
 * $Id$
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License version 2 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include <qstring.h>

#include "ltdl.h"

#ifdef HAVE_DLFCN_H
# include <dlfcn.h>
#endif

#ifdef RTLD_GLOBAL
# define LTDL_GLOBAL	RTLD_GLOBAL
#else
# ifdef DL_GLOBAL
#  define LTDL_GLOBAL	DL_GLOBAL
# else
#  define LTDL_GLOBAL	0
# endif
#endif

/* These are to link libkio even if 'smart' linker is used */
#include <kio/authinfo.h>
extern "C" KIO::AuthInfo* _kioslave_init_kio() { return new KIO::AuthInfo(); }

int main(int argc, char **argv)
{
     if (argc < 5)
     {
        fprintf(stderr, "Usage: kioslave <slave-lib> <protocol> <klauncher-socket> <app-socket>\n\nThis program is part of KDE.\n");
        exit(1);
     }
     QCString libpath = argv[1];     

     if (libpath.isEmpty())
     {
        fprintf(stderr, "library path is empty.\n");
        exit(1); 
     }
     lt_dlinit();

     lt_dlhandle handle = lt_dlopen( libpath.data() );
     if (!handle )
     {
        const char * ltdlError = lt_dlerror();
        fprintf(stderr, "could not open %s: %s", libpath.data(), ltdlError != 0 ? ltdlError : "(null)" );
        exit(1);
     }  

     lt_ptr sym = lt_dlsym( handle, "kdemain");
     if (!sym )
     {
        sym = lt_dlsym( handle, "main");
        if (!sym )
        {
           const char * ltdlError = lt_dlerror();
           fprintf(stderr, "Could not find main: %s\n", ltdlError != 0 ? ltdlError : "(null)" );
           exit(1);
        }
     }

     int (*func)(int, char *[]) = (int (*)(int, char *[])) sym;

     exit( func(argc-1, argv+1)); /* Launch! */
}
