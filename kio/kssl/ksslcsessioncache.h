/* This file is part of the KDE project
 *
 * Copyright (C) 2003 Stefan Rompf <sux@loplof.de>
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
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef __KSSLCSESSIONCACHE_H
#define __KSSLCSESSIONCACHE_H

#include <kdelibs_export.h>

class KUrl;
class QString;

class KIO_EXPORT KSSLCSessionCache {
 public:

    /**
     * Store a SSL session (client side only)
     * @param url URL the key belongs to. Method, host and port are used
     * @param session QString representing session to store
     */
    static void putSessionForURL(const KUrl &kurl, const QString &session);

    /**
     * Retrieve a SSL session (client side only)
     * @param url URL the key belongs to
     * @return if a key can be found, QString() otherwise
     */
    static QString getSessionForURL(const KUrl &kurl);
};

#endif
