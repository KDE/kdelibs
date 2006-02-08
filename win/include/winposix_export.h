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

#ifndef WINPOSX_IEXPORT_H
#define WINPOSX_IEXPORT_H

/* We need to be independent from kdelibs so we can't use kdelibs_export.h */
#ifdef _MSC_VER
#define KDE_EXPORT __declspec(dllexport)
#define KDE_IMPORT __declspec(dllimport)
#else
#define KDE_EXPORT
#define KDE_IMPORT
#endif

#ifndef KDEWIN32_EXPORT
# ifdef MAKE_KDEWIN32_LIB
#  define KDEWIN32_EXPORT KDE_EXPORT
# else
#  define KDEWIN32_EXPORT KDE_IMPORT
# endif
#endif

#ifndef _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_DEPRECATE
#endif

#ifndef _CRT_NONSTDC_NO_DEPRECATE
#define _CRT_NONSTDC_NO_DEPRECATE
#endif

#endif	// WINPOSX_IEXPORT_H
