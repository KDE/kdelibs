/* This file is part of the KDE libraries
    Copyright (c) 2002 KDE Team

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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#ifndef _KDE_MACROS_H_
#define _KDE_MACROS_H_

#if __GNUC__ - 0 > 3 || (__GNUC__ - 0 == 3 && __GNUC_MINOR__ - 0 > 2)
#define KDE_NO_EXPORT __attribute__ ((visibility("hidden")))
#define KDE_EXPORT __attribute__ ((visibility("visible")))
#else
#define KDE_NO_EXPORT
#define KDE_EXPORT
#endif

#if __GNUC__ - 0 > 3 || (__GNUC__ - 0 == 3 && __GNUC_MINOR__ - 0 >= 2)
# define KDE_DEPRECATED __attribute__ ((deprecated))
#else
# define KDE_DEPRECATED
#endif

#endif // _KDE_MACROS_H_
