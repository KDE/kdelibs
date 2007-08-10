/*  This file is part of the KDE project
    Copyright (C) 2007 Matthias Kretz <kretz@kde.org>

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

#ifndef PHONON_EXPERIMENTAL_EXPORT_H
#define PHONON_EXPERIMENTAL_EXPORT_H

/* needed for KDE_EXPORT and KDE_IMPORT macros */
#include <QtCore/qglobal.h>

#ifndef PHONONEXPERIMENTAL_EXPORT
# if defined(MAKE_PHONONEXPERIMENTAL_LIB)
   /* We are building this library */ 
#  define PHONONEXPERIMENTAL_EXPORT Q_DECL_EXPORT
# else
   /* We are using this library */ 
#  define PHONONEXPERIMENTAL_EXPORT Q_DECL_IMPORT
# endif
#endif

#endif
