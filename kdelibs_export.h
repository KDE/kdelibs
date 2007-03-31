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

#ifndef _KDELIBS_EXPORT_H
#define _KDELIBS_EXPORT_H

/* needed for KDE_EXPORT macros */
#include <kdemacros.h>

/* needed, because e.g. Q_OS_UNIX is so frequently used */
#ifdef __cplusplus
# include <QtCore/qglobal.h>
#endif

/* Qt macros aren't known in a non C++ project */
#if defined _WIN32 || defined _WIN64
# include <kdelibs_export_win.h>
#else /* UNIX */

/* export statements for unix */
#define KDEPRINT_EXPORT KDE_EXPORT
#define KDEPRINT_MANAGEMENT_EXPORT KDE_EXPORT
#define KDEPRINT_MANAGEMENT_MODULE_EXPORT KDE_EXPORT
#define KNTLM_EXPORT KDE_EXPORT
#define KHTML_EXPORT KDE_EXPORT
#define KHTML_JAVA_EXPORT KDE_EXPORT
#define KDE3SUPPORT_EXPORT KDE_EXPORT
#define KATEPARTINTERFACES_EXPORT KDE_EXPORT
#define KATEPART_EXPORT KDE_EXPORT
#define KUNITTEST_EXPORT KDE_EXPORT
#define KROSSCORE_EXPORT KDE_EXPORT

#ifndef KJS_EXPORT
#define KJS_EXPORT KDE_EXPORT
#endif

#define SONNETCORE_EXPORT KDE_EXPORT
#define SONNETUI_EXPORT KDE_EXPORT

#define PHONONCORE_EXPORT     KDE_EXPORT
#define PHONONUI_EXPORT       KDE_EXPORT
#define PHONON_FAKE_EXPORT    KDE_EXPORT
#define PHONON_FAKEUI_EXPORT  KDE_EXPORT
#define KAUDIODEVICELIST_EXPORT KDE_EXPORT

#define KDED_EXPORT KDE_EXPORT

#define KIOSLAVE_FILE_EXPORT KDE_EXPORT

#ifndef O_BINARY
#define O_BINARY 0 /* for open() */
#endif

#endif

#endif /*_KDELIBS_EXPORT_H*/

/* workaround for kdecore: stupid moc's grammar doesn't accept two macros
   between 'class' keyword and <classname>: */
#ifdef KDE_DEPRECATED
# ifndef KDE3SUPPORT_EXPORT_DEPRECATED
#  define KDE3SUPPORT_EXPORT_DEPRECATED KDE_DEPRECATED KDE3SUPPORT_EXPORT
# endif
#endif
/* (let's add KDE****_EXPORT_DEPRECATED for other libraries if it's needed) */
