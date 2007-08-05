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

#include "ksslcsessioncache.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QPair>
#include <QtCore/QString>

#include <kdebug.h>
#include <kurl.h>

#include <ksslconfig.h>

/*
 * Operation:
 *
 * Sessions will be stored per running application, not KDE
 * wide, to avoid security problems with hostile programs
 * that negotiate sessions with weak cryptographic keys and store
 * them for everybody to use - I really don't want that.
 *
 * Retrieval is organized similar to George's thoughts in the KSSLD
 * certificate cache: The cache is organised as a list, with the
 * recently fetched (or stored) session first.
 *
 * The cache has an artificial limit of 32 sessions (should really
 * be enough), and relies on the peer server for timeouts
 *
 */
#define MAX_ENTRIES 32

#ifdef KSSL_HAVE_SSL

typedef QPair<QString,QString> KSSLCSession;
typedef QList<KSSLCSession> KSSLCSessions;

static KSSLCSessions *sessions = 0L;

static QString URLtoKey(const KUrl &kurl) {
    return kurl.host() + ':' + kurl.protocol() + ':' + QString::number(kurl.port());
}


static void cleanupKSSLCSessions() {
    delete sessions;
    sessions = 0;
}

static void setup() {
    sessions = new KSSLCSessions;
    qAddPostRoutine(cleanupKSSLCSessions);
}

#endif

QString KSSLCSessionCache::getSessionForUrl(const KUrl &kurl) {
#ifdef KSSL_HAVE_SSL
    if (!sessions) return QString();
    QString key = URLtoKey(kurl);

    for (int i = 0; i < sessions->size(); ++i) {
	if (sessions->at(i).first == key) {
            QString snd = sessions->at(i).second;
            sessions->prepend(sessions->takeAt(i));
	    return snd;
	}
    }

    // Negative caching disabled: cache pollution
#if 0
    kDebug(7029) <<"Negative caching " <<key;
    if (sessions->count() >= MAX_ENTRIES) sessions->removeLast();
    sessions->prepend(new KSSLCSession(key, QString()));
#endif

#endif
    return QString();
}


void KSSLCSessionCache::putSessionForUrl(const KUrl &kurl, const QString &session) {
#ifdef KSSL_HAVE_SSL
    if (!sessions) setup();
    QString key = URLtoKey(kurl);
    KSSLCSessions::iterator it = sessions->begin();

    while ( it != sessions->end() ) {
        if ( it->first == key )
            break;
        ++it;
    }

    if (it != sessions->end()) {
	it->second = session;
    } else {
        if (sessions->size() >= MAX_ENTRIES)
            sessions->removeLast();
        sessions->prepend(KSSLCSession(key, session));
    }

#endif
}
