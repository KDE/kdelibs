/* This file is part of the KDE project
 *
 * Copyright (C) 2000,2001 George Staikos <staikos@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */
#ifndef _INCLUDE_KSSLUTILS_H
#define _INCLUDE_KSSLUTILS_H

#include "ksslconfig.h"

#include <unistd.h>
#ifdef KSSL_HAVE_SSL
#define crypt _openssl_crypt
#include <openssl/x509.h>
#undef crypt
#endif
class QString;
class QDateTime;


#ifdef KSSL_HAVE_SSL
// This functionality is missing in OpenSSL
QString ASN1_UTCTIME_QString(ASN1_UTCTIME *tm);
QDateTime ASN1_UTCTIME_QDateTime(ASN1_UTCTIME *tm, int *isGmt);
QString ASN1_INTEGER_QString(ASN1_INTEGER *aint);
#endif




#endif
