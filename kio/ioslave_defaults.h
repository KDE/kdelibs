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

// TIMEOUT VALUES
#define DEFAULT_RESPONSE_TIMEOUT         60     //  1 min.
#define DEFAULT_CONNECT_TIMEOUT          20     // 20 secs.
#define DEFAULT_READ_TIMEOUT             15     // 15 secs.
#define DEFAULT_PROXY_CONNECT_TIMEOUT    10     // 10 secs.
#define MIN_TIMEOUT_VALUE                 2     //  2 secs.

#define DEFAULT_MINIMUM_KEEP_SIZE         5000  //  5 Kbs

// PORT SETTINGS
#define DEFAULT_PROXY_PORT             8080
#define DEFAULT_FTP_PORT                 21
#define DEFAULT_HTTP_PORT                80
#define DEFAULT_HTTPS_PORT              443
#define MAX_PORT_VALUE                65536

#endif
