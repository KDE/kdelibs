/*  This file is part of the KDE project
    Copyright (C) 2007 David Faure <faure@kde.org>
    Copyright (C) 2009 Jeremy Whiting <jpwhiting@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef KNEWSTUFF3_EXPORT_H
#define KNEWSTUFF3_EXPORT_H

/* needed for KDE_EXPORT and KDE_IMPORT macros */
#include <kdemacros.h>

#ifndef KNEWSTUFF_EXPORT
# if defined(MAKE_KNEWSTUFF3_LIB)
/* We are building this library */
#  define KNEWSTUFF_EXPORT KDE_EXPORT
# else
/* We are using this library */
#  define KNEWSTUFF_EXPORT KDE_IMPORT
# endif
#endif

# ifndef KNEWSTUFF_EXPORT_DEPRECATED
#  define KNEWSTUFF_EXPORT_DEPRECATED KDE_DEPRECATED KNEWSTUFF_EXPORT
# endif

#endif
