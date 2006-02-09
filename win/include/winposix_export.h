/*
   This file is part of the KDE libraries
   Copyright (C) 2004 Jaroslaw Staniek <js@iidea.pl>
   Copyright (C) 2006 Ralf Habacker <ralf.habacker@freenet.de>

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

#ifndef WINPOSIX_EXPORT_H
#define WINPOSIX_EXPORT_H

#ifndef WINPOSIX_EXPORT
# if defined(MAKE_WINPOSIX_LIB) || defined(MAKE_KDEWIN32_LIB)
#  define WINPOSIX_EXPORT __declspec(dllexport)
# else
#  define WINPOSIX_EXPORT __declspec(dllimport)
# endif
#  define WINPOSIX_IMPORT __declspec(dllimport)
#endif

/* for compatibility */
#define KDEWIN32_EXPORT WINPOSIX_EXPORT
#define KDEWIN32_IMPORT WINPOSIX_IMPORT


#ifndef _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_DEPRECATE
#endif

#ifndef _CRT_NONSTDC_NO_DEPRECATE
#define _CRT_NONSTDC_NO_DEPRECATE
#endif

#endif	// WINPOSX_IEXPORT_H
