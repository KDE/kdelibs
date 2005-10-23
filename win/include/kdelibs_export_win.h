/*
   This file is part of the KDE libraries
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

#include <io.h> /* to avoid #includes */

#define KPATH_SEPARATOR ';'
#define popen _popen
#define pclose _pclose

#if defined(__MINGW32__)
#define KDE_IMPORT
#else 
#define KDE_IMPORT __declspec(dllimport)
#endif 

#ifdef MAKE_KDECORE_LIB
# define KDECORE_EXPORT KDE_EXPORT
#else
# ifndef KDECORE_EXPORT
#  define KDECORE_EXPORT KDE_IMPORT //for apps and other libs
# endif
#endif

#ifndef KDEWIN32_EXPORT
# ifdef MAKE_KDEWIN32_LIB
#  define KDEWIN32_EXPORT KDE_EXPORT
# else
#  define KDEWIN32_EXPORT KDE_IMPORT
# endif
#endif 

/* some classes, i.e. KDock* already use EXPORT_* macro: define it too */
#ifdef MAKE_KDEUI_LIB
# define KDEUI_EXPORT KDE_EXPORT
# define EXPORT_DOCKCLASS KDE_EXPORT
#elif defined(KDE_MAKE_LIB)
# define KDEUI_EXPORT KDE_IMPORT
# define EXPORT_DOCKCLASS KDE_IMPORT /* for library build export docklass by default */
#else
# define KDEUI_EXPORT
# define EXPORT_DOCKCLASS
#endif

#ifdef MAKE_KDEFX_LIB
# define KDEFX_EXPORT  KDE_EXPORT
#else
# define KDEFX_EXPORT KDE_IMPORT
#endif

#ifdef MAKE_KDEPRINT_LIB
# define KDEPRINT_EXPORT  KDE_EXPORT
#else
# define KDEPRINT_EXPORT KDE_IMPORT
#endif

#ifndef KIO_EXPORT
# ifdef MAKE_KIO_LIB
#  define KIO_EXPORT KDE_EXPORT
# else
#  define KIO_EXPORT KDE_IMPORT
# endif
#endif

#ifdef MAKE_DCOP_LIB
# define DCOP_EXPORT KDE_EXPORT
#else
# define DCOP_EXPORT KDE_IMPORT
#endif

#ifdef MAKE_KPARTS_LIB
# define KPARTS_EXPORT KDE_EXPORT
#else
# define KPARTS_EXPORT KDE_IMPORT
#endif

#ifdef MAKE_KTEXTEDITOR_LIB
# define KTEXTEDITOR_EXPORT KDE_EXPORT
#else
# define KTEXTEDITOR_EXPORT KDE_IMPORT
#endif

#ifdef MAKE_KABC_LIB
# define KABC_EXPORT KDE_EXPORT
#else
# define KABC_EXPORT KDE_IMPORT
#endif


#ifdef MAKE_KVCARD_LIB
# define KVCARD_EXPORT KDE_EXPORT
#else
# define KVCARD_EXPORT KDE_IMPORT
#endif

#ifdef MAKE_KRESOURCES_LIB
# define KRESOURCES_EXPORT KDE_EXPORT
#else
# define KRESOURCES_EXPORT KDE_IMPORT
#endif

#ifdef MAKE_KDESU_LIB
# define KDESU_EXPORT KDE_EXPORT
#else
# define KDESU_EXPORT KDE_IMPORT
#endif

// all KStyle libs
#ifdef MAKE_KSTYLE_LIB
# define KSTYLE_EXPORT KDE_EXPORT
#else
# define KSTYLE_EXPORT KDE_IMPORT
#endif

#ifdef MAKE_KMDI_LIB
# define KMDI_EXPORT  KDE_EXPORT
#else
# define KMDI_EXPORT KDE_IMPORT
#endif

#ifdef MAKE_KUTILS_LIB
# define KUTILS_EXPORT  KDE_EXPORT
#else
# define KUTILS_EXPORT KDE_IMPORT
#endif

#ifdef MAKE_KATEPARTINTERFACES_LIB
# define KATEPARTINTERFACES_EXPORT  KDE_EXPORT
#else
# define KATEPARTINTERFACES_EXPORT KDE_IMPORT
#endif

#ifdef MAKE_KATEPART_LIB
# define KATEPART_EXPORT  KDE_EXPORT
#else
# define KATEPART_EXPORT KDE_IMPORT
#endif

#ifdef MAKE_KDE3SUPPORT_LIB
#define KDE3SUPPORT_EXPORT KDE_EXPORT
#else
#define KDE3SUPPORT_EXPORT KDE_IMPORT
#endif

#ifdef MAKE_KHTML_LIB
# define KHTML_EXPORT KDE_EXPORT
#else
# define KHTML_EXPORT KDE_IMPORT
#endif

