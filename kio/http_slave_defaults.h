/* This file is part of the KDE libraries
   Copyright (C) 2001 Waldo Bastian <bastian@kde.org>

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

#ifndef _KIO_HTTP_SLAVE_DEFAULTS_H
#define _KIO_HTTP_SLAVE_DEFAULTS_H

// CACHE SETTINGS
#define DEFAULT_MAX_CACHE_SIZE          5120          //  5 MB
#define DEFAULT_MAX_CACHE_AGE           60*60*24*14   // 14 DAYS
#define DEFAULT_EXPIRE_TIME             1*60          //  1 MIN
#define DEFAULT_CACHE_CONTROL		KIO::CC_Verify     // Verify with remote

#endif
