/* This file is part of the KDE libraries

   Copyright (c) 2001,2002 Carsten Pfeiffer <pfeiffer@kde.org>
                 2007 Jos van den Oever <jos@vandenoever.info>
                 2010 Sebastian Trueg <trueg@kde.org>

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

#include "kfilemetainfo.h"
#include "kfilemetainfoitem.h"
#include "kfilemetainfoitem_p.h"
#include "kfilewriteplugin.h"
#include "kfilewriteplugin_p.h"

#include "config-kde4support.h"

#if ! KIO_NO_STRIGI
#include <strigi/bufferedstream.h>
#include <strigi/analyzerconfiguration.h>
#include <strigi/indexwriter.h>
#include <strigi/analysisresult.h>
#include <strigi/fieldtypes.h>
#endif

#include <QFileInfo>
#include <QDateTime>
#include <QStringList>

class KFileMetaInfoGroupPrivate : public QSharedData {
public:
    QString name;
};

KFileMetaInfoGroup::~KFileMetaInfoGroup()
{
}

KFileMetaInfoGroup::KFileMetaInfoGroup ( KFileMetaInfoGroup const& g )
{
    d = g.d;
}

QDataStream& operator >> ( QDataStream& s, KFileMetaInfo& )
{
    return s;
}

QDataStream& operator << ( QDataStream& s, const KFileMetaInfo& )
{
    return s;
}
#if ! KIO_NO_STRIGI
/**
 * @brief Wrap a QIODevice in a Strigi stream.
 **/
class QIODeviceInputStream : public Strigi::BufferedInputStream {
private:
    QIODevice& in;
    const qint64 m_maxRead;
    qint64 m_read;
    int32_t fillBuffer ( char* start, int32_t space );
public:
    QIODeviceInputStream ( QIODevice& i, qint64 max );
};

int32_t
QIODeviceInputStream::fillBuffer ( char* start, int32_t space )
{
    if ( !in.isOpen() || !in.isReadable() )
        return -1;

    // we force a max stream read length according to the config since some Strigi
    // plugins simply ignore the value which will lead to frozen client apps
    qint64 max = m_maxRead;
    if(max < 0)
        max = space;
    else
        max = qMin(qint64(space), qMax(max-m_read,qint64(0)));

    // read into the buffer
    int32_t nwritten = in.read ( start, max );

    // check the file stream status
    if ( nwritten < 0 ) {
        m_error = "Could not read from QIODevice.";
        in.close();
        return -1;
    }
    if ( nwritten == 0 || in.atEnd() ) {
        in.close();
    }
    m_read += nwritten;
    return nwritten;
}

QIODeviceInputStream::QIODeviceInputStream ( QIODevice& i, qint64 max )
    : in ( i ),
      m_maxRead(max),
      m_read(0)
{
    // determine if we have a character device, which will likely never eof and thereby
    // potentially cause an infinite loop.
    if ( i.isSequential() ) {
        in.close(); // cause fillBuffer to return -1
    }
}

/**
 * @brief KMetaInfoWriter handles the data returned by the Strigi analyzers and
 * store it in a KFileMetaInfo.
 **/
class KMetaInfoWriter : public Strigi::IndexWriter {
public:
    // irrelevant for KFileMetaInfo
    void startAnalysis(const Strigi::AnalysisResult*) {
    }

    // irrelevant for KFileMetaInfo
    // we do not store text as metainfo
    void addText(const Strigi::AnalysisResult*, const char* /*s*/, int32_t /*n*/) {
    }
    void addValue(const Strigi::AnalysisResult* idx, const Strigi::RegisteredField* field,
            const std::string& value) {
        if (idx->writerData()) {
            QString val = QString::fromUtf8(value.c_str(), value.size());
            if( !val.startsWith(':') )
                addValue(idx, field, val);
        }
    }
    void addValue(const Strigi::AnalysisResult* idx, const Strigi::RegisteredField* field,
        const unsigned char* data, uint32_t size) {
        if (idx->writerData()) {
            QByteArray d((const char*)data, size);
            addValue(idx, field, QVariant(d));
        }
    }
    void addValue(const Strigi::AnalysisResult* idx, const Strigi::RegisteredField* field,
            uint32_t value) {
        if (idx->writerData()) {
            addValue(idx, field, QVariant((quint32)value));
        }
    }
    void addValue(const Strigi::AnalysisResult* idx, const Strigi::RegisteredField* field,
            int32_t value) {
        if (idx->writerData()) {
            addValue(idx, field, QVariant((qint32)value));
        }
    }
    void addValue(const Strigi::AnalysisResult* idx, const Strigi::RegisteredField* field,
            double value) {
        if (idx->writerData()) {
            addValue(idx, field, QVariant(value));
        }
    }
    void addValue(const Strigi::AnalysisResult* idx,
                  const Strigi::RegisteredField* field, const QVariant& value) {
        QHash<QString, KFileMetaInfoItem>* info
            = static_cast<QHash<QString, KFileMetaInfoItem>*>(
            idx->writerData());
        if (info) {
            std::string name(field->key());
            QString key = QString::fromUtf8(name.c_str(), name.size());
            QHash<QString, KFileMetaInfoItem>::iterator i = info->find(key);
            if (i == info->end()) {
                info->insert(key, KFileMetaInfoItem(key, value, 0, true));
            } else {
                i.value().addValue(value);
            }
        }
    }
    void addValue(const Strigi::AnalysisResult* ar,
                  const Strigi::RegisteredField* field, const std::string& name,
            const std::string& value) {
        if (ar->writerData()) {
            QVariantMap m;
            m.insert ( name.c_str(), value.c_str() );
            addValue ( ar, field, m );
        }
    }

    /* irrelevant for KFileMetaInfo: These triples does not convey information
     * about this file, so we ignore it
     */
    void addTriplet ( const std::string& /*subject*/,
                      const std::string& /*predicate*/, const std::string& /*object*/ ) {
    }

    // irrelevant for KFileMetaInfo
    void finishAnalysis(const Strigi::AnalysisResult*) {}
    // irrelevant for KFileMetaInfo
    void deleteEntries(const std::vector<std::string>&) {}
    // irrelevant for KFileMetaInfo
    void deleteAllEntries() {}
};


class KFileMetaInfoPrivate : public QSharedData
{
public:
    QHash<QString, KFileMetaInfoItem> items;
    QUrl m_url;

    void init ( QIODevice& stream, const QUrl& url, const QDateTime &mtime, KFileMetaInfo::WhatFlags w = KFileMetaInfo::Everything );
    void initWriters ( const QUrl& /*file*/ );
    void operator= ( const KFileMetaInfoPrivate& k ) {
        items = k.items;
    }
};
static const KFileMetaInfoItem nullitem;

class KFileMetaInfoAnalysisConfiguration : public Strigi::AnalyzerConfiguration
{
public:
    KFileMetaInfoAnalysisConfiguration( KFileMetaInfo::WhatFlags indexDetail )
    : m_indexDetail(indexDetail) {
    }

    int64_t maximalStreamReadLength ( const Strigi::AnalysisResult& ar ) {
        if(ar.depth() > 0)
            return 0; // ignore all data that has a depth > 0, i.e. files in archives
        else if(m_indexDetail == KFileMetaInfo::Everything)
            return -1;
        else
            return 65536; // do not read the whole file - this is used for on-the-fly analysis
    }

private:
    KFileMetaInfo::WhatFlags m_indexDetail;
};

void KFileMetaInfoPrivate::init ( QIODevice& stream, const QUrl& url, const QDateTime &mtime, KFileMetaInfo::WhatFlags w )
{
    m_url = url;

    // get data from Strigi
    KFileMetaInfoAnalysisConfiguration c( w );
    Strigi::StreamAnalyzer indexer ( c );
    KMetaInfoWriter writer;
    //qDebug() << url;
    Strigi::AnalysisResult idx ( url.toLocalFile().toUtf8().constData(), mtime.toTime_t(), writer, indexer );
    idx.setWriterData ( &items );

    QIODeviceInputStream strigiStream ( stream, c.maximalStreamReadLength(idx) );
    indexer.analyze ( idx, &strigiStream );

    // TODO: get data from Nepomuk
}

void KFileMetaInfoPrivate::initWriters ( const QUrl& file )
{
    QStringList mimetypes;
    QHash<QString, KFileMetaInfoItem>::iterator i;
    for ( i = items.begin(); i != items.end(); ++i ) {
        KFileWritePlugin *w =
            KFileWriterProvider::self()->loadPlugin ( i.key() );
        if ( w && w->canWrite ( file, i.key() ) ) {
            i.value().d->writer = w;
        }
    }
}

KFileMetaInfo::KFileMetaInfo ( const QString& path, const QString& /*mimetype*/,
                               KFileMetaInfo::WhatFlags w )
        : d ( new KFileMetaInfoPrivate() )
{
    QFileInfo fileinfo ( path );
    QFile file ( path );
    // only open the file if it is a filetype Qt understands
    // if e.g. the path points to a pipe, it is not opened
    if ( ( fileinfo.isFile() || fileinfo.isDir() || fileinfo.isSymLink() )
            && file.open ( QIODevice::ReadOnly ) ) {
        const QUrl u = QUrl::fromLocalFile(path);
        d->init ( file, u, fileinfo.lastModified(), w );
        if ( fileinfo.isWritable() ) {
            d->initWriters ( u );
        }
    }
}

KFileMetaInfo::KFileMetaInfo(const QUrl& url)
        : d ( new KFileMetaInfoPrivate() )
{
    QFile file(url.toLocalFile());
    if ( file.open ( QIODevice::ReadOnly ) ) {
        QFileInfo fileinfo(url.toLocalFile());
        d->init(file, url, fileinfo.lastModified());
        if ( fileinfo.isWritable() ) {
            d->initWriters ( url );
        }
    }
}

KFileMetaInfo::KFileMetaInfo() : d ( new KFileMetaInfoPrivate() )
{
}

KFileMetaInfo::KFileMetaInfo ( const KFileMetaInfo& k ) : d ( k.d )
{
}

KFileMetaInfo& KFileMetaInfo::operator= ( KFileMetaInfo const & kfmi )
{
    d = kfmi.d;
    return *this;
}

KFileMetaInfo::~KFileMetaInfo()
{
}

bool KFileMetaInfo::applyChanges()
{
    // go through all editable fields and group them by writer
    QHash<KFileWritePlugin*, QVariantMap> data;
    QHash<QString, KFileMetaInfoItem>::const_iterator i;
    for ( i = d->items.constBegin(); i != d->items.constEnd(); ++i ) {
        if ( i.value().isModified() && i.value().d->writer ) {
            data[i.value().d->writer][i.key() ] = i.value().value();
        }
    }

    // call the writers on the data they can write
    bool ok = true;
    QHash<KFileWritePlugin*, QVariantMap>::const_iterator j;
    for ( j = data.constBegin(); j != data.constEnd(); ++j ) {
        ok &= j.key()->write ( d->m_url, j.value() );
    }
    return ok;
}

QUrl KFileMetaInfo::url() const
{
    return d->m_url;
}

const QHash<QString, KFileMetaInfoItem>& KFileMetaInfo::items() const
{
    return d->items;
}

const KFileMetaInfoItem& KFileMetaInfo::item ( const QString& key ) const
{
    QHash<QString, KFileMetaInfoItem>::const_iterator i = d->items.constFind ( key );
    return ( i == d->items.constEnd() ) ? nullitem : i.value();
}

QStringList KFileMetaInfo::keys() const
{
    return d->items.keys();
}

KFileMetaInfoItem& KFileMetaInfo::item ( const QString& key )
{
    return d->items[key];
}

bool KFileMetaInfo::isValid() const
{
    return !d->m_url.isEmpty();
}

QStringList KFileMetaInfo::preferredKeys() const
{
    return QStringList();
}

QStringList KFileMetaInfo::supportedKeys() const
{
    return QStringList();
}

#ifndef KDE_NO_DEPRECATED
KFileMetaInfoGroupList KFileMetaInfo::preferredGroups() const
{
    return KFileMetaInfoGroupList();
}
#endif

#ifndef KDE_NO_DEPRECATED
KFileMetaInfoGroupList KFileMetaInfo::supportedGroups() const
{
    return KFileMetaInfoGroupList();
}
#endif
#else //KIO_NO_STRIGI

class KFileMetaInfoPrivate : public QSharedData
{
public:
};

KFileMetaInfo::KFileMetaInfo ( const QString& path, const QString& /*mimetype*/,
                               KFileMetaInfo::WhatFlags w )
{
}

KFileMetaInfo::KFileMetaInfo(const QUrl& url)
{
}

KFileMetaInfo::KFileMetaInfo()
{
}

KFileMetaInfo::KFileMetaInfo ( const KFileMetaInfo& k )
{
}

KFileMetaInfo& KFileMetaInfo::operator= ( KFileMetaInfo const & kfmi )
{
    d = kfmi.d;
    return *this;
}

KFileMetaInfo::~KFileMetaInfo()
{
}

bool KFileMetaInfo::applyChanges()
{
    return false;
}

QUrl KFileMetaInfo::url() const
{
    return QUrl();
}

const QHash<QString, KFileMetaInfoItem>& KFileMetaInfo::items() const
{
    static const QHash<QString, KFileMetaInfoItem> items;
    return items;
}

const KFileMetaInfoItem& KFileMetaInfo::item ( const QString& key ) const
{
    static const KFileMetaInfoItem item;
    return item;
}

QStringList KFileMetaInfo::keys() const
{
    return QStringList();
}

KFileMetaInfoItem& KFileMetaInfo::item ( const QString& key )
{
    static KFileMetaInfoItem item;
    return item;
}

bool KFileMetaInfo::isValid() const
{
    return false;
}

QStringList KFileMetaInfo::preferredKeys() const
{
    return QStringList();
}

QStringList KFileMetaInfo::supportedKeys() const
{
    return QStringList();
}
#endif //KIO_NO_STRIGI

KFileMetaInfoItemList KFileMetaInfoGroup::items() const
{
    return KFileMetaInfoItemList();
}

const QString& KFileMetaInfoGroup::name() const
{
    return d->name;
}
