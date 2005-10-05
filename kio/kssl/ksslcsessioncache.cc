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

#include <qpair.h>
#include <qstring.h>
#include <q3ptrlist.h>

#include <kdebug.h>
#include <kstaticdeleter.h>
#include <kurl.h>

#ifdef Q_WS_WIN
#include "ksslconfig_win.h"
#else
#include "ksslconfig.h"
#endif

#include "ksslcsessioncache.h"

/* 
 * Operation:
 *
 * Sessions will be stored per running application, not KDE
 * wide, to avoid security problems with hostile programs
 * that negotiate sessions with weak cryptographic keys and store
 * them for everybody to use - I really don't want that.
 *
 * Retrieval is organised similiar to George's thoughts in the KSSLD
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
typedef Q3PtrList<KSSLCSession> KSSLCSessions;

static KSSLCSessions *sessions = 0L;
static KStaticDeleter<KSSLCSessions> med;


static QString URLtoKey(const KURL &kurl) {
    return kurl.host() + ":" + kurl.protocol() + ":" + QString::number(kurl.port());
}


static void setup() {
    KSSLCSessions *ses = new KSSLCSessions;
    ses->setAutoDelete(true);
    med.setObject(sessions, ses);
}

#endif

QString KSSLCSessionCache::getSessionForURL(const KURL &kurl) {
#ifdef KSSL_HAVE_SSL
    if (!sessions) return QString::null;
    QString key = URLtoKey(kurl);

    for(KSSLCSession *it = sessions->first(); it; it=sessions->next()) {
	if (it->first == key) {
	    sessions->take();
	    sessions->prepend(it);
	    return it->second;
	}
    }

    // Negative caching disabled: cache pollution
#if 0 
    kdDebug(7029) <<"Negative caching " <<key <<endl;
    if (sessions->count() >= MAX_ENTRIES) sessions->removeLast();
    sessions->prepend(new KSSLCSession(key, QString::null));
#endif

#endif
    return QString::null;
}


void KSSLCSessionCache::putSessionForURL(const KURL &kurl, const QString &session) {
#ifdef KSSL_HAVE_SSL
    if (!sessions) setup();
    QString key = URLtoKey(kurl);
    KSSLCSession *it;

    for(it = sessions->first(); it && it->first != key; it=sessions->next());

    if (it) {
	sessions->take();
	it->second = session;
    } else {
	it = new KSSLCSession(key, session);
	if (sessions->count() >= MAX_ENTRIES) sessions->removeLast();
    }

    sessions->prepend(it);
#endif
}
