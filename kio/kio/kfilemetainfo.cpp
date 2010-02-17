/* This file is part of the KDE libraries

   Copyright (c) 2001,2002 Carsten Pfeiffer <pfeiffer@kde.org>
                 2007 Jos van den Oever <jos@vandenoever.info>

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
#include <strigi/bufferedstream.h>
#include <strigi/analyzerconfiguration.h>
#include <strigi/indexwriter.h>
#include <strigi/analysisresult.h>
#include <strigi/fieldtypes.h>
#include <kurl.h>
#include <kdebug.h>
#include <QFileInfo>
#include <QDateTime>
#include <QStringList>

using namespace std;
using namespace Strigi;

class KFileMetaInfoGroupPrivate : public QSharedData {
public:
    QString name;
};
KFileMetaInfoGroup::~KFileMetaInfoGroup() {
}
KFileMetaInfoGroup::KFileMetaInfoGroup(KFileMetaInfoGroup const& g) {
    p = g.p;
}

QDataStream& operator >>(QDataStream& s, KFileMetaInfo& ) {
    return s;
}
QDataStream& operator <<(QDataStream& s, const KFileMetaInfo&) {
    return s;
}

/**
 * @brief Wrap a QIODevice in a Strigi stream.
 **/
class QIODeviceInputStream : public BufferedInputStream {
private:
    QIODevice& in;
    int32_t fillBuffer(char* start, int32_t space);
public:
    QIODeviceInputStream(QIODevice& i);
};

int32_t
QIODeviceInputStream::fillBuffer(char* start, int32_t space) {
    if (!in.isOpen() || !in.isReadable()) return -1;
    // read into the buffer
    int32_t nwritten = in.read(start, space);
    // check the file stream status
    if (nwritten < 0) {
        m_error = "Could not read from QIODevice.";
        in.close();
        return -1;
    }
    if (nwritten == 0 || in.atEnd()) {
        in.close();
    }
    return nwritten;
}

QIODeviceInputStream::QIODeviceInputStream(QIODevice &i) :in(i)
{
    // determine if we have a character device, which will likely never eof and thereby
    // potentially cause an infinite loop.
    if(i.isSequential()) {
        in.close(); // cause fillBuffer to return -1
    }
}

/**
 * @brief KMetaInfoWriter handles the data returned by the Strigi analyzers and
 * store it in a KFileMetaInfo.
 **/
class KMetaInfoWriter : public IndexWriter {
public:
    // irrelevant for KFileMetaInfo
    void startAnalysis(const AnalysisResult*) {
    }
    // irrelevant for KFileMetaInfo
    // we do not store text as metainfo
    void addText(const AnalysisResult*, const char* /*s*/, int32_t /*n*/) {
    }
    void addValue(const AnalysisResult* idx, const RegisteredField* field,
            const string& value) {
        if (idx->writerData()) {
            QString val = QString::fromUtf8(value.c_str(), value.size());
            addValue(idx, field, val);
        }
    }
    void addValue(const AnalysisResult* idx, const RegisteredField* field,
        const unsigned char* data, uint32_t size) {
        if (idx->writerData()) {
            QByteArray d((const char*)data, size);
            addValue(idx, field, QVariant(d));
        }
    }
    void addValue(const AnalysisResult* idx, const RegisteredField* field,
            uint32_t value) {
        if (idx->writerData()) {
            addValue(idx, field, QVariant((quint32)value));
        }
    }
    void addValue(const AnalysisResult* idx, const RegisteredField* field,
            int32_t value) {
        if (idx->writerData()) {
            addValue(idx, field, QVariant((qint32)value));
        }
    }
    void addValue(const AnalysisResult* idx, const RegisteredField* field,
            double value) {
        if (idx->writerData()) {
            addValue(idx, field, QVariant(value));
        }
    }
    void addValue(const AnalysisResult* idx,
            const RegisteredField* field, const QVariant& value) {
        QHash<QString, KFileMetaInfoItem>* info
            = static_cast<QHash<QString, KFileMetaInfoItem>*>(
            idx->writerData());
        if (info) {
            string name(field->key());
            QString key = QString::fromUtf8(name.c_str(), name.size());
            QHash<QString, KFileMetaInfoItem>::iterator i = info->find(key);
            if (i == info->end()) {
                // retrieve the info describing this field
                PredicateProperties pp(key);
                info->insert(key, KFileMetaInfoItem(pp, value, 0, true));
            } else {
                i.value().addValue(value);
            }
        }
    }
    void addValue(const Strigi::AnalysisResult* ar,
            const RegisteredField* field, const std::string& name,
            const std::string& value) {
        if (ar->writerData()) {
            QVariantMap m;
            m.insert(name.c_str(), value.c_str());
            addValue(ar, field, m);
        }
    }
    /* irrelevant for KFileMetaInfo: These triples does not convey information
     * about this file, so we ignore it
     */
    void addTriplet(const std::string& /*subject*/,
        const std::string& /*predicate*/, const std::string& /*object*/) {
    }
    // irrelevant for KFileMetaInfo
    void finishAnalysis(const AnalysisResult*) {}
    // irrelevant for KFileMetaInfo
    void deleteEntries(const vector<string>&) {}
    // irrelevant for KFileMetaInfo
    void deleteAllEntries() {}
};
class KFileMetaInfoPrivate : public QSharedData {
public:
    QHash<QString, KFileMetaInfoItem> items;
    KUrl kurl;

    //Private() :QSharedData() {qDebug() <<"ok: " << this;}
    void init(QIODevice& stream, const KUrl& url, time_t mtime, KFileMetaInfo::WhatFlags w=KFileMetaInfo::Everything);
    void initWriters(const KUrl& /*file*/);
    void operator=(const KFileMetaInfoPrivate& k) {
        items = k.items;
        kurl = k.kurl;
    }
};
static const KFileMetaInfoItem nullitem;

//used to override 64k limit of PredicatePropertyProvider's indexer.
//this is important for getting gettext po files statistics, for example
class ComprehensiveAnalysisConfiguration : public Strigi::AnalyzerConfiguration {
     int64_t maximalStreamReadLength(const Strigi::AnalysisResult& ar) {return (ar.depth() == 0) ?-1:0;} 
};

//const KFileMetaInfoItem KFileMetaInfoPrivate::null;
void
KFileMetaInfoPrivate::init(QIODevice& stream, const KUrl& url, time_t mtime, KFileMetaInfo::WhatFlags w) {
    // get data from Strigi
    kurl = url;
    ComprehensiveAnalysisConfiguration c;
    StreamAnalyzer comprehensiveIndexer(c);
    StreamAnalyzer& indexer = (w==KFileMetaInfo::Everything)?comprehensiveIndexer:PredicatePropertyProvider::self()->indexer();
    KMetaInfoWriter writer;
    QIODeviceInputStream strigiStream(stream);
    kDebug(7033) << url;
    AnalysisResult idx(url.toLocalFile().toUtf8().constData(), mtime, writer, indexer);

    idx.setWriterData(&items);
    indexer.analyze(idx, &strigiStream);

    // TODO: get data from Nepomuk
}
void
KFileMetaInfoPrivate::initWriters(const KUrl& file) {
    QStringList mimetypes;
    QHash<QString, KFileMetaInfoItem>::iterator i;
    for (i = items.begin(); i != items.end(); ++i) {
        KFileWritePlugin *w =
            KFileWriterProvider::self()->loadPlugin(i.key());
        if (w && w->canWrite(file, i.key())) {
            i.value().p->writer = w;
        }
    }
}
KFileMetaInfo::KFileMetaInfo(const QString& path, const QString& /*mimetype*/,
                             KFileMetaInfo::WhatFlags w)
    : p(new KFileMetaInfoPrivate())
{
    QFileInfo fileinfo(path);
    QFile file(path);
    // only open the file if it is a filetype Qt understands
    // if e.g. the path points to a pipe, it is not opened
    if ((fileinfo.isFile() || fileinfo.isDir() || fileinfo.isSymLink())
            && file.open(QIODevice::ReadOnly)) {
        KUrl u(path);
        p->init(file, u, fileinfo.lastModified().toTime_t(), w);
        if (fileinfo.isWritable()) {
            p->initWriters(u);
        }
    }
}
KFileMetaInfo::KFileMetaInfo(const KUrl& url)
    : p(new KFileMetaInfoPrivate())
{
    QFileInfo fileinfo(url.toLocalFile());
    QFile file(url.toLocalFile());
    if (file.open(QIODevice::ReadOnly)) {
        p->init(file, url, fileinfo.lastModified().toTime_t());
        if (fileinfo.isWritable()) {
            p->initWriters(url);
        }
    }
}
KFileMetaInfo::KFileMetaInfo() :p(new KFileMetaInfoPrivate()) {
}
KFileMetaInfo::KFileMetaInfo(const KFileMetaInfo& k) :p(k.p) {
}
const KFileMetaInfo&
KFileMetaInfo::operator=(KFileMetaInfo const& kfmi) {
    p = kfmi.p;
    return kfmi;
}
KFileMetaInfo::~KFileMetaInfo() {
}
bool
KFileMetaInfo::applyChanges() {

    // go through all editable fields and group them by writer
    QHash<KFileWritePlugin*, QVariantMap> data;
    QHash<QString, KFileMetaInfoItem>::const_iterator i;
    for (i = p->items.constBegin(); i != p->items.constEnd(); ++i) {
        if (i.value().isModified() && i.value().p->writer) {
            data[i.value().p->writer][i.key()] = i.value().value();
        }
    }

    // call the writers on the data they can write
    bool ok = true;
    QHash<KFileWritePlugin*, QVariantMap>::const_iterator j;
    for (j = data.constBegin(); j != data.constEnd(); ++j) {
        ok &= j.key()->write(p->kurl, j.value());
    }
    return ok;
}
const KUrl&
KFileMetaInfo::url() const {
    return p->kurl;
}
const QHash<QString, KFileMetaInfoItem>&
KFileMetaInfo::items() const {
    return p->items;
}
const KFileMetaInfoItem&
KFileMetaInfo::item(const QString& key) const {
    QHash<QString, KFileMetaInfoItem>::const_iterator i = p->items.constFind(key);
    return (i == p->items.constEnd()) ?nullitem :i.value();
}
QStringList
KFileMetaInfo::keys() const {
    return p->items.keys();
}
KFileMetaInfoItem&
KFileMetaInfo::item(const QString& key) {
    return p->items[key];
}
bool
KFileMetaInfo::isValid() const {
    return !p->kurl.isEmpty();
}
QStringList KFileMetaInfo::preferredKeys() const { return QStringList(); }
QStringList KFileMetaInfo::supportedKeys() const { return QStringList(); }

KFileMetaInfoGroupList
KFileMetaInfo::preferredGroups() const {
    return KFileMetaInfoGroupList();
}
KFileMetaInfoGroupList
KFileMetaInfo::supportedGroups() const {
    return KFileMetaInfoGroupList();
}
KFileMetaInfoItemList
KFileMetaInfoGroup::items() const {
    return KFileMetaInfoItemList();
}
const QString&
KFileMetaInfoGroup::name() const {
    return p->name;
}

