/*  This file is part of the KDE project
    Copyright (C) 2007 David Faure <faure@kde.org>
    Copyright (C) 2007 Kevin Ottens <ervin@kde.org>

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

#ifndef __KDE_HAVE_GCC_VISIBILITY
#cmakedefine __KDE_HAVE_GCC_VISIBILITY
#endif

#ifdef SOLID_EXPORT
  /* We are building a test case */
# define SOLID_NO_EXPORT

#else
  /* We are not building a test case */

# ifdef __KDE_HAVE_GCC_VISIBILITY

#  define SOLID_NO_EXPORT __attribute__ ((visibility("hidden")))
#  define SOLID_EXPORT __attribute__ ((visibility("default")))

# elif defined(_WIN32) || defined(_WIN64)

#  define SOLID_NO_EXPORT
#  if defined(MAKE_SOLID_LIB)
    /* We are building this library */
#   define SOLID_EXPORT __declspec(dllexport)
#  else
    /* We are using this library */
#   define SOLID_EXPORT __declspec(dllimport)
#  endif

# else

#  define SOLID_NO_EXPORT
#  define SOLID_EXPORT

# endif
#endif

#ifndef SOLID_EXPORT_DEPRECATED
# define SOLID_EXPORT_DEPRECATED KDE_DEPRECATED SOLID_EXPORT
#endif

#endif
