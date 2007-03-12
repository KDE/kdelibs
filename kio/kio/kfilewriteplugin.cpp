/* This file is part of the KDE libraries

   Copyright (c) 2007 Jos van den Oever <jos@vandenoever.info>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License (LGPL) as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#include "kfilemetainfo_private.h"
#include "kglobal.h"
#include "kmimetypetrader.h"
#include <QHash>

K_GLOBAL_STATIC(KFileWriterProvider, staticKFileWriterProvider)

KFileWriterProvider*
KFileWriterProvider::self() {
    return staticKFileWriterProvider;
}

KFileWriterProvider::~KFileWriterProvider() {
    qDeleteAll(plugins);
    plugins.clear();
}

KFileWritePlugin*
KFileWriterProvider::loadPlugin(const QString& /*mimetype*/, const QString& key) {
    const QString constraint = QString::fromLatin1(
         "'%1' in MetaDataKeys" ).arg(key);
    KService::List offers;// = KMimeTypeTrader::self()->query(mimetype,
        //"KFileWritePlugin", constraint);
    if (offers.isEmpty()) {
        return 0;
    }
    KService::Ptr service = *(offers.begin());
    if (!service || !service->isValid()) {
        return 0;
    }
    KFileWritePlugin* plugin
        = KService::createInstance<KFileWritePlugin>(service);
    return plugin;
}

KFileWritePlugin*
KFileWriterProvider::plugin(const QString& key, const KUrl& /*url*/,
        const QStringList& mimetypes) {
    KFileWritePlugin* w = 0;
    foreach (const QString& mt, mimetypes) {
        w = loadPlugin(mt, key);
        if (w) break;
    }
    return w;
}

#include "kfilewriteplugin.moc"

