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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <kdewin.h>
#include <io.h> //to avoid #includes

#define KPATH_SEPARATOR ';'
#define popen _popen
#define pclose _pclose

#ifdef MAKE_KDECORE_LIB
# define KDECORE_EXPORT _KDE_EXPORT_
#else
# define KDECORE_EXPORT _KDE_IMPORT_ //for apps and other libs
#endif

//some classes, i.e. KDock* already use EXPORT_* macro: define it too
#ifdef MAKE_KDEUI_LIB
# define KDEUI_EXPORT _KDE_EXPORT_
# define EXPORT_DOCKCLASS _KDE_EXPORT_
#elif KDE_MAKE_LIB
# define KDEUI_EXPORT _KDE_IMPORT_
# define EXPORT_DOCKCLASS _KDE_IMPORT_ //for library build export docklass by default
#else
# define KDEUI_EXPORT
# define EXPORT_DOCKCLASS
#endif

#ifdef MAKE_KDEFX_LIB
# define KDEFX_EXPORT  _KDE_EXPORT_
#else
# define KDEFX_EXPORT _KDE_IMPORT_
#endif

#ifdef MAKE_KDEPRINT_LIB
# define KDEPRINT_EXPORT  _KDE_EXPORT_
#else
# define KDEPRINT_EXPORT _KDE_IMPORT_
#endif

#ifndef KIO_EXPORT
# ifdef MAKE_KIO_LIB
#  define KIO_EXPORT _KDE_EXPORT_
# else
#  define KIO_EXPORT _KDE_IMPORT_
# endif
#endif

#ifdef MAKE_KFILE_LIB
# define KFILE_EXPORT _KDE_EXPORT_
#else
# define KFILE_EXPORT _KDE_IMPORT_
#endif

#ifdef MAKE_DCOP_LIB
# define DCOP_EXPORT _KDE_EXPORT_
#else
# define DCOP_EXPORT _KDE_IMPORT_
#endif

#ifdef MAKE_KSSL_LIB
# define KSSL_EXPORT _KDE_EXPORT_
#else
# define KSSL_EXPORT _KDE_IMPORT_
#endif

#ifdef MAKE_KPARTS_LIB
# define KPARTS_EXPORT _KDE_EXPORT_
#else
# define KPARTS_EXPORT _KDE_IMPORT_
#endif

#ifdef MAKE_KTEXTEDITOR_LIB
# define KTEXTEDITOR_EXPORT _KDE_EXPORT_
#else
# define KTEXTEDITOR_EXPORT _KDE_IMPORT_
#endif

#ifdef MAKE_KABC_LIB
# define KABC_EXPORT _KDE_EXPORT_
#else
# define KABC_EXPORT _KDE_IMPORT_
#endif

#ifdef MAKE_KDERESOURCES_LIB
# define KDERESOURCES_EXPORT _KDE_EXPORT_
#else
# define KDERESOURCES_EXPORT _KDE_IMPORT_
#endif

// all KStyle libs
#ifdef MAKE_KSTYLE_LIB
# define KSTYLE_EXPORT _KDE_EXPORT_
#else
# define KSTYLE_EXPORT _KDE_IMPORT_
#endif

#ifdef MAKE_KMDI_LIB
# define KMDI_EXPORT  _KDE_EXPORT_
#else
# define KMDI_EXPORT _KDE_IMPORT_
#endif

#ifdef MAKE_KUTILS_LIB
# define KUTILS_EXPORT  _KDE_EXPORT_
#else
# define KUTILS_EXPORT _KDE_IMPORT_
#endif

#ifdef MAKE_KATEPARTINTERFACES_LIB
# define KATEPARTINTERFACES_EXPORT  _KDE_EXPORT_
#else
# define KATEPARTINTERFACES_EXPORT _KDE_IMPORT_
#endif

#ifdef MAKE_KATEPART_LIB
# define KATEPART_EXPORT  _KDE_EXPORT_
#else
# define KATEPART_EXPORT _KDE_IMPORT_
#endif
