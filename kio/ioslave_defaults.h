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

#ifndef _KIO_IOSLAVE_DEFAULTS_H
#define _KIO_IOSLAVE_DEFAULTS_H

// DEFAULT TIMEOUT VALUE FOR REMOTE AND PROXY CONNECTION
// AND RESPONSE WAIT PERIOD.  NOTE: CHANGING THESE VALUES
// ALSO CHANGES THE DEFAULT ESTABLISHED INITIALLY.
#define DEFAULT_RESPONSE_TIMEOUT         60		//  1 MIN
#define DEFAULT_CONNECT_TIMEOUT          20		// 20 SEC
#define DEFAULT_READ_TIMEOUT             15		// 15 SEC
#define DEFAULT_PROXY_CONNECT_TIMEOUT    10		// 10 SEC

// MINIMUM TIMEOUT VALUE ALLOWED
#define MIN_TIMEOUT_VALUE                 2		//  2 SEC

#define DEFAULT_MINIMUM_KEEP_SIZE	5000		// 5000 bytes

#endif
