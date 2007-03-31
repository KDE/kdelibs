/*  This file is part of the KDE project
    Copyright (C) 2007 David Faure <faure@kde.org>

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

#ifndef PHONON_EXPORT_H
#define PHONON_EXPORT_H

/* needed for KDE_EXPORT and KDE_IMPORT macros */
#include <kdemacros.h>

/* We use _WIN32/_WIN64 instead of Q_OS_WIN so that this header can be used from C files too */
#if defined _WIN32 || defined _WIN64

#ifndef PHONONCORE_EXPORT
# if defined(MAKE_PHONON_LIB)
   /* We are building this library */
#  define PHONONCORE_EXPORT KDE_EXPORT
# else
   /* We are using this library */
#  define PHONONCORE_EXPORT KDE_IMPORT
# endif
#endif

#ifndef PHONONUI_EXPORT
# if defined(MAKE_PHONONUI_LIB)
   /* We are building this library */
#  define PHONONUI_EXPORT KDE_EXPORT
# else
   /* We are using this library */
#  define PHONONUI_EXPORT KDE_IMPORT
# endif
#endif

#ifndef PHONON_FAKE_EXPORT
# if defined(MAKE_PHONON_FAKE_LIB)
   /* We are building this library */
#  define PHONON_FAKE_EXPORT KDE_EXPORT
# else
   /* We are using this library */
#  define PHONON_FAKE_EXPORT KDE_IMPORT
# endif
#endif

#ifndef PHONON_FAKEUI_EXPORT
# if defined(MAKE_PHONON_FAKEUI_LIB)
   /* We are building this library */
#  define PHONON_FAKEUI_EXPORT KDE_EXPORT
# else
   /* We are using this library */
#  define PHONON_FAKEUI_EXPORT KDE_IMPORT
# endif
#endif

#ifndef KAUDIODEVICELIST_EXPORT
# if defined(MAKE_KAUDIODEVICELIST_LIB)
   /* We are building this library */
#  define KAUDIODEVICELIST_EXPORT KDE_EXPORT
# else
   /* We are using this library */
#  define KAUDIODEVICELIST_EXPORT KDE_IMPORT
# endif
#endif

#else /* UNIX */

#define PHONONCORE_EXPORT KDE_EXPORT
#define PHONONUI_EXPORT KDE_EXPORT
#define PHONON_FAKE_EXPORT KDE_EXPORT
#define PHONON_FAKEUI_EXPORT KDE_EXPORT
#define KAUDIODEVICELIST_EXPORT KDE_EXPORT

#endif

# ifndef PHONONCORE_EXPORT_DEPRECATED
#  define PHONONCORE_EXPORT_DEPRECATED KDE_DEPRECATED PHONONCORE_EXPORT
# endif
# ifndef PHONONUI_EXPORT_DEPRECATED
#  define PHONONUI_EXPORT_DEPRECATED KDE_DEPRECATED PHONONUI_EXPORT
# endif
# ifndef PHONON_FAKE_EXPORT_DEPRECATED
#  define PHONON_FAKE_EXPORT_DEPRECATED KDE_DEPRECATED PHONON_FAKE_EXPORT
# endif
# ifndef PHONON_FAKEUI_EXPORT_DEPRECATED
#  define PHONON_FAKEUI_EXPORT_DEPRECATED KDE_DEPRECATED PHONON_FAKEUI_EXPORT
# endif
# ifndef KAUDIODEVICELIST_EXPORT_DEPRECATED
#  define KAUDIODEVICELIST_EXPORT_DEPRECATED KDE_DEPRECATED KAUDIODEVICELIST_EXPORT
# endif

#endif
