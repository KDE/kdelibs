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
#include "kfilewriteplugin.h"
#include "predicateproperties.h"
#include "kfilemetainfoitem.h"
#include "kfilemetainfo.h"
#include <QHash>

namespace jstreams {
    class StreamIndexer;
}

class KFileWriterProvider {
private:
    KFileWritePlugin* loadPlugin(const QString& mimetype,
        const QString& key);
    QHash<QString, QHash<QString, KFileWritePlugin*> > loadedWriters;
public:
    KFileWritePlugin* plugin(const QString& key, const KUrl& url,
        const QStringList& mimetypes);
    QHash<QString, KFileWritePlugin*> plugins;
    ~KFileWriterProvider();
    static KFileWriterProvider* self();
};

class PredicatePropertyProvider {
private:
    class Private;
    Private* p;
public:
    PredicatePropertyProvider();
    ~PredicatePropertyProvider();
    static PredicatePropertyProvider* self();
    jstreams::StreamIndexer& indexer();
    static PredicateProperties getPredicateProperties(const QString& key);
};

class KFileMetaInfoItemPrivate : public QSharedData {
public:
    static const QVariant null;
    PredicateProperties pp;
    QVariant value;
    KFileWritePlugin* writer;
    bool embedded;
    bool modified;
};

