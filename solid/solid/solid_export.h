/*  This file is part of the KDE project
    Copyright (C) 2006 David Faure <faure@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef SOLID_EXPORT_H
#define SOLID_EXPORT_H

/* needed for KDE_EXPORT and KDE_IMPORT macros */
#include <kdemacros.h>

/* We use _WIN32/_WIN64 instead of Q_OS_WIN so that this header can be used from C files too */
#if defined _WIN32 || defined _WIN64

#ifndef SOLID_EXPORT
# if defined(MAKE_SOLID_LIB)
   /* We are building this library */ 
#  define SOLID_EXPORT KDE_EXPORT
# else
   /* We are using this library */ 
#  define SOLID_EXPORT KDE_IMPORT
# endif
#endif

#ifndef SOLIDIFACES_EXPORT
# if defined(MAKE_SOLIDIFACES_LIB)
   /* We are building this library */
#  define SOLIDIFACES_EXPORT KDE_EXPORT
# else
   /* We are using this library */
#  define SOLIDIFACES_EXPORT KDE_IMPORT
# endif
#endif

#else /* UNIX */

#define SOLID_EXPORT KDE_EXPORT
#define SOLIDIFACES_EXPORT KDE_EXPORT

#endif

#endif
