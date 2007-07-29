/*
 *
 * This file is part of the KDE project.
 * Copyright (C) 2007 Rivo Laks <rivolaks@hot.ee>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License version 2 as published by the Free Software Foundation.
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

#include "kpixmapcache.h"

#include <QtCore/QString>
#include <QtGui/QPixmap>
#include <QtCore/QFile>
#include <QtCore/QDataStream>
#include <QtCore/QFileInfo>
#include <QtCore/QDateTime>
#include <QtGui/QPixmapCache>
#include <QtCore/QtGlobal>
#include <QtGui/QPainter>

#include <kglobal.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <klockfile.h>
#include <ksvgrenderer.h>


//#define DISABLE_PIXMAPCACHE

#define KPIXMAPCACHE_VERSION 0x000201


class LockFile
{
public:
    LockFile(const QString& filename, bool exclusive = false)
    {
        mValid = false;
        mLockFile = new KLockFile(filename);
        KLockFile::LockResult result = mLockFile->lock(KLockFile::NoBlockFlag);
        // TODO: If locking blocks then sleep for a small amount of time (e.g.
        //  20ms) and try again for a few times
        if (result != KLockFile::LockOK) {
            kError() << k_funcinfo << "Failed to lock file '" << filename << "', result = " << result << endl;
        } else {
            mValid = true;
        }
    }
    ~LockFile()
    {
        if (mValid) {
            mLockFile->unlock();
        }
        delete mLockFile;
    }

    bool isValid() const  { return mValid; }

private:
    bool mValid;
    KLockFile* mLockFile;
};


class KPixmapCache::Private
{
public:
    Private(KPixmapCache* q);

    int findOffset(const QString& key);
    int binarySearchKey(QDataStream& stream, const QString& key, int start);

    bool checkLockFile();
    bool checkFileVersion(const QString& filename);
    bool loadIndexHeader();

    QString indexKey(const QString& key);


    KPixmapCache* q;

    static const char* kpc_magic;
    static const int kpc_magic_len;
    static const int kpc_header_len;
    quint32 mIndexRootOffset;  // offset of the first entry in index file

    bool mInited;  // Whether init() has been called (it's called on-demand)
    bool mEnabled;   // whether it's possible to use the cache
    bool mValid;  // whether cache has been inited and is ready to be used

    QString mName;
    QString mIndexFile;
    QString mDataFile;
    QString mLockFileName;

    quint32 mTimestamp;
    bool mUseQPixmapCache;
    int mCacheLimit;
    RemoveStrategy mRemoveStrategy;
};

// Magic in the cache files
const char* KPixmapCache::Private::kpc_magic = "KDE PIXMAP CACHE ";
const int KPixmapCache::Private::kpc_magic_len = qstrlen(KPixmapCache::Private::kpc_magic);
// Whole header is magic + version (4 bytes)
const int KPixmapCache::Private::kpc_header_len = KPixmapCache::Private::kpc_magic_len + 4;


KPixmapCache::Private::Private(KPixmapCache* _q)
{
    q = _q;
}

int KPixmapCache::Private::binarySearchKey(QDataStream& stream, const QString& key, int start)
{
    stream.device()->seek(start);

    QString fkey;
    qint32 foffset;
    qint32 leftchild, rightchild;
    stream >> fkey >> foffset >> leftchild >> rightchild;

    if (key < fkey) {
        if (leftchild) {
            return binarySearchKey(stream, key, leftchild);
        }
    } else if (key == fkey) {
        return start;
    } else if (rightchild) {
        return binarySearchKey(stream, key, rightchild);
    }

    return start;
}

int KPixmapCache::Private::findOffset(const QString& key)
{
    // Open file and datastream on it
    QFile file(mIndexFile);
    if (!file.open(QIODevice::ReadOnly)) {
        return -1;
    }
    file.seek(mIndexRootOffset);
    QDataStream stream(&file);

    // If we're already at the end of the stream then the root node doesn't
    //  exist yet and there are no entries. Otherwise, do a binary search
    //  starting from the root node.
    if (!stream.atEnd()) {
        int nodeoffset = binarySearchKey(stream, key, mIndexRootOffset);

        // Load the found entry and check if it's the one we're looking for.
        file.seek(nodeoffset);
        QString fkey;
        qint32 foffset;
        stream >> fkey >> foffset;
        if(fkey == key)
            return foffset;
    }

    // Nothing was found
    return -1;
}

bool KPixmapCache::Private::checkLockFile()
{
    // For KLockFile we need to ensure the lock file doesn't exist.
    if (QFile::exists(mLockFileName)) {
        if (!QFile::remove(mLockFileName)) {
            kError() << k_funcinfo << "Couldn't remove lockfile '" << mLockFileName << "'" << endl;
            return false;
        }
    }
    return true;
}

bool KPixmapCache::Private::checkFileVersion(const QString& filename)
{
    if (QFile::exists(filename)) {
        // File already exists, make sure it can be opened
        QFile f(filename);
        if (!f.open(QIODevice::ReadOnly)) {
            kError() << k_funcinfo << "Couldn't open file '" << filename << "'" << endl;
            return false;
        }

        QDataStream stream(&f);

        // Check header and version
        char buf[100];
        quint32 version;
        stream.readRawData(buf, kpc_magic_len);
        stream >> version;
        if (qstrncmp(buf, kpc_magic, kpc_magic_len) == 0 && version == KPIXMAPCACHE_VERSION) {
            return true;
        } else {
            kWarning() << k_funcinfo << "File '" << filename << "' is outdated, will recreate..." << endl;
        }
    }

    return q->recreateCacheFiles();
}

bool KPixmapCache::Private::loadIndexHeader()
{
    // Open file and datastream on it
    QFile file(mIndexFile);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }
    file.seek(kpc_header_len);

    QDataStream stream(&file);
    if (file.atEnd()) {
        // Write default timestamp
        mTimestamp = QDateTime::currentDateTime().toTime_t();
        stream << (quint32)mTimestamp;
    } else {
        // Load timestamp
        stream >> mTimestamp;
    }

    if (stream.status() != QDataStream::Ok) {
        kWarning() << k_funcinfo << "Failed to read index file's header" << endl;
        q->recreateCacheFiles();
        return false;
    }

    // Give custom implementations chance to load their headers
    if (!q->loadCustomIndexHeader(stream)) {
        return false;
    }

    // Load root node pos.
    stream >> mIndexRootOffset;

    return true;
}

QString KPixmapCache::Private::indexKey(const QString& key)
{
    QByteArray latin1 = key.toLatin1();
    return QString("%1%2").arg((ushort)qChecksum(latin1.data(), latin1.size()), 4, 16, QLatin1Char('0')).arg(key);
}




KPixmapCache::KPixmapCache(const QString& name)
    :d(new Private(this))
{
    d->mName = name;
    d->mUseQPixmapCache = true;
    d->mCacheLimit = 3 * 1024;
    d->mRemoveStrategy = RemoveLeastRecentlyUsed;

    // We cannot call init() here because the subclasses haven't been
    //  constructed yet and so their virtual methods cannot be used.
    d->mInited = false;
}

KPixmapCache::~KPixmapCache()
{
    delete d;
}

void KPixmapCache::init()
{
    d->mInited = true;

#ifdef DISABLE_PIXMAPCACHE
    d->mValid = d->mEnabled = false;
#else
    d->mValid = false;

    // Find locations of the files
    d->mIndexFile = KGlobal::dirs()->locateLocal("cache", "kpc/" + d->mName + ".index");
    d->mDataFile  = KGlobal::dirs()->locateLocal("cache", "kpc/" + d->mName + ".data");
    d->mLockFileName = KGlobal::dirs()->locateLocal("cache", "kpc/" + d->mName + ".lock");

    d->mEnabled = true;
    d->mEnabled &= d->checkLockFile();
    d->mEnabled &= d->checkFileVersion(d->mDataFile);
    d->mEnabled &= d->checkFileVersion(d->mIndexFile);
    if (!d->mEnabled) {
        kDebug() << k_funcinfo << "Pixmap cache '" + d->mName + "' is disabled" << endl;
    } else {
        // Cache is enabled, but check if it's ready for use
        setValid(d->loadIndexHeader());
    }
#endif
}

void KPixmapCache::ensureInited() const
{
    if (!d->mInited) {
        const_cast<KPixmapCache*>(this)->init();
    }
}

bool KPixmapCache::loadCustomIndexHeader(QDataStream&)
{
    return true;
}

void KPixmapCache::writeCustomIndexHeader(QDataStream&)
{
}

bool KPixmapCache::isEnabled() const
{
    ensureInited();
    return d->mEnabled;
}

bool KPixmapCache::isValid() const
{
    ensureInited();
    return d->mValid;
}

void KPixmapCache::setValid(bool valid)
{
    ensureInited();
    d->mValid = valid;
}

unsigned int KPixmapCache::timestamp() const
{
    ensureInited();
    return d->mTimestamp;
}

void KPixmapCache::setTimestamp(unsigned int ts)
{
    ensureInited();
    d->mTimestamp = ts;

    // Write to file
    QFile file(d->mIndexFile);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Append)) {
        return;
    }
    file.seek(d->kpc_header_len);
    QDataStream stream(&file);
    stream << d->mTimestamp;
}

int KPixmapCache::size() const
{
    ensureInited();
    return QFileInfo(d->mDataFile).size() / 1024;
}

void KPixmapCache::setUseQPixmapCache(bool use)
{
    d->mUseQPixmapCache = use;
}

bool KPixmapCache::useQPixmapCache() const
{
    return d->mUseQPixmapCache;
}

int KPixmapCache::cacheLimit() const
{
    return d->mCacheLimit;
}

void KPixmapCache::setCacheLimit(int kbytes)
{
    d->mCacheLimit = kbytes;
    // TODO: cleanup if size() > cacheLimit()
}

KPixmapCache::RemoveStrategy KPixmapCache::removeEntryStrategy() const
{
    return d->mRemoveStrategy;
}

void KPixmapCache::setRemoveEntryStrategy(KPixmapCache::RemoveStrategy strategy)
{
    d->mRemoveStrategy = strategy;
}

bool KPixmapCache::recreateCacheFiles()
{
    d->mEnabled = false;
    // Create index file
    QFile indexfile(d->mIndexFile);
    if (!indexfile.open(QIODevice::WriteOnly)) {
        kError() << k_funcinfo << "Couldn't create index file '" << d->mIndexFile << "'" << endl;
        return false;
    }
    QDataStream istream(&indexfile);
    istream.writeRawData(d->kpc_magic, d->kpc_magic_len);
    istream << (quint32)KPIXMAPCACHE_VERSION;
    // Write default timestamp
    d->mTimestamp = QDateTime::currentDateTime().toTime_t();
    istream << (quint32)d->mTimestamp;

    // Create data file
    QFile datafile(d->mDataFile);
    if (!datafile.open(QIODevice::WriteOnly)) {
        kError() << k_funcinfo << "Couldn't create data file '" << d->mDataFile << "'" << endl;
        return false;
    }
    QDataStream dstream(&datafile);
    dstream.writeRawData(d->kpc_magic, d->kpc_magic_len);
    dstream << (quint32)KPIXMAPCACHE_VERSION;

    setValid(true);
    writeCustomIndexHeader(istream);

    d->mIndexRootOffset = indexfile.pos() + sizeof(quint32);
    istream << d->mIndexRootOffset;
    return true;
}

void KPixmapCache::deleteCache(const QString& name)
{
    QString indexFile = KGlobal::dirs()->locateLocal("cache", "kpc/" + name + ".index");
    QString dataFile  = KGlobal::dirs()->locateLocal("cache", "kpc/" + name + ".data");
    if (QFile::exists(indexFile)) {
        QFile::remove(indexFile);
    }
    if (QFile::exists(dataFile)) {
        QFile::remove(dataFile);
    }
    // No need to remove the lockfile
}

void KPixmapCache::discard()
{
    deleteCache(d->mName);
    if (d->mUseQPixmapCache) {
        QPixmapCache::clear();
    }

    d->mInited = false;
    init();
}

void KPixmapCache::removeEntries(int newsize)
{
    if (!newsize) {
        newsize = cacheLimit();
    }
    //TODO!!!
}

bool KPixmapCache::find(const QString& key, QPixmap& pix)
{
    ensureInited();
    if (!isValid()) {
        return false;
    }

    //kDebug() << "KPC::find(" << key << "), use QPC = " << d->mUseQPixmapCache << endl;
    // First try the QPixmapCache
    if (d->mUseQPixmapCache && QPixmapCache::find(key, pix)) {
        //kDebug() << k_funcinfo << "Found from QPC" << endl;
        return true;
    }

    LockFile lock(d->mLockFileName);
    if (!lock.isValid()) {
        return false;
    }

    // Try to find the offset
    QString indexkey = d->indexKey(key);
    int offset = d->findOffset(indexkey);
    //kDebug() << "KPC: " << "found offset " << offset << endl;
    if (offset == -1) {
        return false;
    }

    // Load the data
    bool ret = loadData(offset, pix);
    if (ret && d->mUseQPixmapCache) {
        // This pixmap wasn't in QPC, put it there
        QPixmapCache::insert(key, pix);
    }
    return ret;
}

bool KPixmapCache::loadData(int offset, QPixmap& pix)
{
    // Open file and datastream on it
    QFile file(d->mDataFile);
    if (!file.open(QIODevice::ReadOnly)) {
        return -1;
    }
    //kDebug() << "KPC: " << k_funcinfo << "Seeking to pos " << offset << "/" << file.size() << endl;
    if (!file.seek(offset)) {
        kError() << k_funcinfo << "Couldn't seek to pos " << offset << endl;
        return false;
    }
    QDataStream stream(&file);

    // Load
    QString fkey;
    stream >> fkey;

    // Load image info and compressed data
    qint32 format, w, h, bpl;
    stream >> format >> w >> h >> bpl;
    QByteArray imgdatacompressed;
    stream >> imgdatacompressed;

    // Uncompress the data and create the image
    // TODO: make sure this actually works. QImage ctor we use here seems to
    //  want 32-bit aligned data. QByteArray uses malloc() to allocate it's
    //  data, which _probably_ returns 32-bit aligned data.
    QByteArray imgdata = qUncompress(imgdatacompressed);
    QImage img((const uchar*)imgdata.constData(), w, h, bpl, (QImage::Format)format);
    img.bits();  // make deep copy since we don't want to keep imgdata around
    pix = QPixmap::fromImage(img);

    if (!loadCustomData(stream)) {
        return false;
    }

    if (stream.status() != QDataStream::Ok) {
        kError() << "KPC: " << k_funcinfo << "stream is bad :-(  status=" <<
                stream.status() << ", pos=" << file.pos() << endl;
        return false;
    }

    //kDebug() << "KPC: " << k_funcinfo << "pixmap successfully loaded" << endl;
    return true;
}

bool KPixmapCache::loadCustomData(QDataStream&)
{
    return true;
}

void KPixmapCache::insert(const QString& key, const QPixmap& pix)
{
    ensureInited();
    if (!isValid()) {
        return;
    }

    //kDebug() << "KPC::insert(" << key << ", " << pix.width() << "x" << pix.height() << ")" << endl;
    // Insert to QPixmapCache as well
    if (d->mUseQPixmapCache) {
        QPixmapCache::insert(key, pix);
    }

    LockFile lock(d->mLockFileName, true);
    if (!lock.isValid()) {
        return;
    }

    // Make sure this key isn't already in the cache
    QString indexkey = d->indexKey(key);
    int offset = d->findOffset(indexkey);
    if (offset >= 0) {
        // This pixmap is already in cache
        kDebug() << "KPC::insert() " << "pixmap already present in cache" << endl;
        return;
    }

    // Insert to cache
    offset = writeData(key, pix);
    //kDebug() << "KPC::insert(): " << "data is at offset " << offset << endl;
    if (offset == -1) {
        return;
    }

    writeIndex(indexkey, offset);
}

int KPixmapCache::writeData(const QString& key, const QPixmap& pix)
{
    // Open file and datastream on it
    QFile file(d->mDataFile);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Append)) {
        return -1;
    }
    int offset = file.pos();
    QDataStream stream(&file);

    // Write the data
    stream << key;
    // Write image info and compressed data
    QImage img = pix.toImage();
    QByteArray imgdatacompressed = qCompress(img.bits(), img.numBytes());
    stream << (qint32)img.format() << (qint32)img.width() << (qint32)img.height() << (qint32)img.bytesPerLine();
    stream << imgdatacompressed;

    writeCustomData(stream);

    return offset;
}

bool KPixmapCache::writeCustomData(QDataStream&)
{
    return true;
}

void KPixmapCache::writeIndex(const QString& key, int dataoffset)
{
    // Open file and datastream on it
    QFile file(d->mIndexFile);
    if (!file.open(QIODevice::ReadWrite)) {
        return;
    }
    QDataStream stream(&file);

    // New entry will be written to the end of the file
    qint32 offset = file.size();
    file.seek(offset);
    // Write the data
    stream << key << (qint32)dataoffset;
    // Write (empty) children offsets
    stream << (qint32)0 << (qint32)0;

    // Find parent index node for this node.
    int parentoffset = d->binarySearchKey(stream, key, d->mIndexRootOffset);
    // If we created the root node then the two offsets are equal and we're
    //  done. Otherwise set parent's child offset to correct value.
    if (parentoffset != offset) {
        file.seek(parentoffset);
        QString fkey;
        qint32 foffset, tmp;
        stream >> fkey >> foffset;
        if (key < fkey) {
            // New entry will be parent's left child
            stream << offset;
        } else {
            // New entry will be parent's right child
            stream >> tmp;
            stream << offset;
        }
    }
}

QPixmap KPixmapCache::loadFromFile(const QString& filename)
{
    QFileInfo fi(filename);
    if (!fi.exists()) {
        return QPixmap();
    } else if (fi.lastModified().toTime_t() > timestamp()) {
        // Cache is obsolete, will be regenerated
        discard();
    }

    QPixmap pix;
    QString key("file:" + filename);
    if (!find(key, pix)) {
        // It wasn't in the cache, so load it...
        pix = QPixmap(filename);
        if (pix.isNull()) {
            return pix;
        }
        // ... and put it there
        insert(key, pix);
    }

    return pix;
}

QPixmap KPixmapCache::loadFromSVG(const QString& filename, const QSize& size)
{
    QFileInfo fi(filename);
    if (!fi.exists()) {
        return QPixmap();
    } else if (fi.lastModified().toTime_t() > timestamp()) {
        // Cache is obsolete, will be regenerated
        discard();
    }

    QPixmap pix;
    QString key = QString("file:%1_%2_%3").arg(filename).arg(size.width()).arg(size.height());
    if (!find(key, pix)) {
        // It wasn't in the cache, so load it...
        KSvgRenderer svg;
        if (!svg.load(filename)) {
            return pix;  // null pixmap
        } else {
            QSize pixSize = size.isValid() ? size : svg.defaultSize();
            pix = QPixmap(pixSize);
            pix.fill(Qt::transparent);

            QPainter p(&pix);
            svg.render(&p, QRectF(QPointF(), pixSize));
        }

        // ... and put it there
        insert(key, pix);
    }

    return pix;
}

