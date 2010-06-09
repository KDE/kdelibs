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
#include "kfilemetainfo_p.h"
#include "kglobal.h"
#include "kservicetypetrader.h"
#include <QHash>
#include <QDebug>

KFileWritePlugin::KFileWritePlugin(QObject* parent, const QStringList&)
        :QObject(parent), d(0)
{
}

KFileWritePlugin::~KFileWritePlugin() {
}

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
KFileWriterProvider::loadPlugin(const QString& key) {
    //kDebug() << "loading writer for key " << key;
    const QString constraint = QString::fromLatin1("'%1' in MetaDataKeys")
        .arg(key);
    const KService::List offers = KServiceTypeTrader::self()->query(
        "KFileWrite", constraint);
    if (offers.isEmpty()) {
        return 0;
    }
    return offers.first()->createInstance<KFileWritePlugin>();
}

#include "kfilewriteplugin.moc"

