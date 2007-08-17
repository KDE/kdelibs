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
#include <QtCore/QQueue>

#include <kglobal.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <klockfile.h>
#include <ksvgrenderer.h>
#include <kdefakes.h>

#include <time.h>
#include <unistd.h>


//#define DISABLE_PIXMAPCACHE

#define KPIXMAPCACHE_VERSION 0x000204

namespace {

class KPCLockFile
{
public:
    KPCLockFile(const QString& filename)
    {
        mValid = false;
        mLockFile = new KLockFile(filename);
        // Try to lock the file up to 5 times, waiting 5 ms between retries
        KLockFile::LockResult result;
        for (int i = 0; i < 5; i++) {
            result = mLockFile->lock(KLockFile::NoBlockFlag);
            if (result == KLockFile::LockOK) {
                mValid = true;
                break;
            }
            usleep(5*1000);
        }
        // Output error msg if locking failed
        if (!mValid) {
            kError() << k_funcinfo << "Failed to lock file '" << filename << "', last result = " << result << endl;
        }
    }
    ~KPCLockFile()
    {
        unlock();
        delete mLockFile;
    }

    void unlock()
    {
        if (mValid) {
            mLockFile->unlock();
            mValid = false;
        }
    }

    bool isValid() const  { return mValid; }

private:
    bool mValid;
    KLockFile* mLockFile;
};

} // namespace


class KPixmapCache::Private
{
public:
    Private(KPixmapCache* q);

    QIODevice* indexDevice();
    QIODevice* dataDevice();

    int findOffset(const QString& key);
    int binarySearchKey(QDataStream& stream, const QString& key, int start);
    void writeIndexEntry(QDataStream& stream, const QString& key, int dataoffset);

    bool checkLockFile();
    bool checkFileVersion(const QString& filename);
    bool loadIndexHeader();

    bool removeEntries(int newsize);

    QString indexKey(const QString& key);


    KPixmapCache* q;

    static const char* kpc_magic;
    static const int kpc_magic_len;
    static const int kpc_header_len;
    quint32 mHeaderSize;  // full size of the index header, including custom (subclass') header data
    quint32 mIndexRootOffset;  // offset of the first entry in index file

    QString mName;
    QString mIndexFile;
    QString mDataFile;
    QString mLockFileName;

    quint32 mTimestamp;
    quint32 mCacheId;  // Unique id, will change when cache is recreated
    int mCacheLimit;
    RemoveStrategy mRemoveStrategy:4;
    bool mUseQPixmapCache:4;

    bool mInited:8;  // Whether init() has been called (it's called on-demand)
    bool mEnabled:8;   // whether it's possible to use the cache
    bool mValid:8;  // whether cache has been inited and is ready to be used


    // Used by removeEntries()
    class KPixmapCacheEntry
    {
    public:
        KPixmapCacheEntry(int indexoffset_, const QString& key_, int dataoffset_,
                          int pos_, quint32 timesused_, quint32 lastused_)
        {
            indexoffset = indexoffset_;
            key = key_;
            dataoffset = dataoffset_;
            pos = pos_;
            timesused = timesused_;
            lastused = lastused_;
        }

        int indexoffset;
        QString key;
        int dataoffset;

        int pos;
        quint32 timesused;
        quint32 lastused;
    };

    static bool compareEntriesByAge(const KPixmapCacheEntry& a, const KPixmapCacheEntry& b)
    {
        return a.pos > b.pos;
    }
    static bool compareEntriesByTimesUsed(const KPixmapCacheEntry& a, const KPixmapCacheEntry& b)
    {
        return a.timesused > b.timesused;
    }
    static bool compareEntriesByLastUsed(const KPixmapCacheEntry& a, const KPixmapCacheEntry& b)
    {
        return a.lastused > b.lastused;
    }
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

QIODevice* KPixmapCache::Private::indexDevice()
{
    QFile* file = new QFile(mIndexFile);
    if (!file->exists() || file->size() < kpc_header_len) {
        q->recreateCacheFiles();
    }
    if (!file->open(QIODevice::ReadWrite)) {
        kDebug() << k_funcinfo << "Couldn't open index file" << mIndexFile << endl;
        delete file;
        return 0;
    }
    // Make sure the file is up-to-date
    QDataStream stream(file);
    file->seek(kpc_header_len + sizeof(quint32));
    quint32 cacheid;
    stream >> cacheid;
    if (cacheid != mCacheId) {
        kDebug() << k_funcinfo << "Cache has changed, reloading" << endl;
        delete file;

        q->init();
        if (!q->isValid()) {
            return 0;
        } else {
            return indexDevice();
        }
    }

    return file;
}

QIODevice* KPixmapCache::Private::dataDevice()
{
    QFile* file = new QFile(mDataFile);
    if (!file->exists() || file->size() < kpc_header_len) {
        q->recreateCacheFiles();
        // Index file has also been recreated so we cannot continue with
        //  modifying the data file because it would make things inconsistent.
        delete file;
        return 0;
    }
    if (!file->open(QIODevice::ReadWrite)) {
        kDebug() << k_funcinfo << "Couldn't open data file" << mDataFile << endl;
        delete file;
        return 0;
    }
    return file;
}

int KPixmapCache::Private::binarySearchKey(QDataStream& stream, const QString& key, int start)
{
    stream.device()->seek(start);

    QString fkey;
    qint32 foffset;
    quint32 timesused, lastused;
    qint32 leftchild, rightchild;
    stream >> fkey >> foffset >> timesused >> lastused >> leftchild >> rightchild;

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
    // Open device and datastream on it
    QIODevice* device = indexDevice();
    if (!device) {
        return -1;
    }
    device->seek(mIndexRootOffset);
    QDataStream stream(device);

    // If we're already at the end of the stream then the root node doesn't
    //  exist yet and there are no entries. Otherwise, do a binary search
    //  starting from the root node.
    if (!stream.atEnd()) {
        int nodeoffset = binarySearchKey(stream, key, mIndexRootOffset);

        // Load the found entry and check if it's the one we're looking for.
        device->seek(nodeoffset);
        QString fkey;
        stream >> fkey;
        if (fkey == key) {
            // Read offset and statistics
            qint32 foffset;
            quint32 timesused, lastused;
            stream >> foffset >> timesused;
            // Update statistics
            timesused++;
            lastused = ::time(0);
            stream.device()->seek(stream.device()->pos() - sizeof(quint32));
            stream << timesused << lastused;
            delete device;
            return foffset;
        }
    }

    // Nothing was found
    delete device;
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
            kWarning() << k_funcinfo << "File '" << filename << "' is outdated, will recreate...";
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
    // Load timestamp
    stream >> mTimestamp;
    // Load cache id
    stream >> mCacheId;

    if (stream.status() != QDataStream::Ok) {
        kWarning() << k_funcinfo << "Failed to read index file's header";
        q->recreateCacheFiles();
        return false;
    }

    // Give custom implementations chance to load their headers
    if (!q->loadCustomIndexHeader(stream)) {
        return false;
    }
    mHeaderSize = stream.device()->pos();

    // Load root node pos.
    stream >> mIndexRootOffset;

    return true;
}

QString KPixmapCache::Private::indexKey(const QString& key)
{
    QByteArray latin1 = key.toLatin1();
    return QString("%1%2").arg((ushort)qChecksum(latin1.data(), latin1.size()), 4, 16, QLatin1Char('0')).arg(key);
}

void KPixmapCache::Private::writeIndexEntry(QDataStream& stream, const QString& key, int dataoffset)
{
    // New entry will be written to the end of the file
    qint32 offset = stream.device()->size();
    stream.device()->seek(offset);
    // Write the data
    stream << key << (qint32)dataoffset;
    // Statistics (# of uses and last used timestamp)
    stream << (quint32)1 << (quint32)::time(0);
    // Write (empty) children offsets
    stream << (qint32)0 << (qint32)0;

    // Find parent index node for this node.
    int parentoffset = binarySearchKey(stream, key, mIndexRootOffset);
    // If we created the root node then the two offsets are equal and we're
    //  done. Otherwise set parent's child offset to correct value.
    if (parentoffset != offset) {
        stream.device()->seek(parentoffset);
        QString fkey;
        qint32 foffset, tmp;
        quint32 timesused, lastused;
        stream >> fkey >> foffset >> timesused >> lastused;
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

bool KPixmapCache::Private::removeEntries(int newsize)
{
    KPCLockFile lock(mLockFileName);
    if (!lock.isValid()) {
        kDebug() << k_funcinfo << "Couldn't lock cache " << mName << endl;
        return false;
    }

    // Open old (current) files
    QFile indexfile(mIndexFile);
    if (!indexfile.open(QIODevice::ReadOnly)) {
        kDebug() << k_funcinfo << "Couldn't open old index file" << endl;
        return false;
    }
    QDataStream istream(&indexfile);
    QFile datafile(mDataFile);
    if (!datafile.open(QIODevice::ReadOnly)) {
        kDebug() << k_funcinfo << "Couldn't open old data file" << endl;
        return false;
    }
    if (datafile.size() <= newsize*1024) {
        kDebug() << k_funcinfo << "Cache size is already within limit (" << datafile.size() << " <= " << newsize*1024 << ")" << endl;
        return true;
    }
    QDataStream dstream(&datafile);
    // Open new files
    QFile newindexfile(mIndexFile + ".new");
    if (!newindexfile.open(QIODevice::ReadWrite)) {
        kDebug() << k_funcinfo << "Couldn't open new index file" << endl;
        return false;
    }
    QDataStream newistream(&newindexfile);
    QFile newdatafile(mDataFile + ".new");
    if (!newdatafile.open(QIODevice::WriteOnly)) {
        kDebug() << k_funcinfo << "Couldn't open new inddataex file" << endl;
        return false;
    }
    QDataStream newdstream(&newdatafile);

    // Copy index file header
    char* header = new char[mHeaderSize];
    if (istream.readRawData(header, mHeaderSize) != (int)mHeaderSize) {
        kDebug() << k_funcinfo << "Couldn't read index header" << endl;
        return false;
    }
    newistream.writeRawData(header, mHeaderSize);
    // Write root index node offset
    newistream << (quint32)(mHeaderSize + sizeof(quint32));
    // Copy data file header
    // mHeaderSize is always bigger than kpc_header_len, so we needn't create
    //  new buffer
    if (dstream.readRawData(header, kpc_header_len) != kpc_header_len) {
        kDebug() << k_funcinfo << "Couldn't read data header" << endl;
        return false;
    }
    newdstream.writeRawData(header, kpc_header_len);


    // Load all entries
    QList<KPixmapCacheEntry> entries;
    // Do BFS to find all entries
    QQueue<int> open;
    open.enqueue(mIndexRootOffset);
    while (!open.isEmpty()) {
        int indexoffset = open.dequeue();
        indexfile.seek(indexoffset);
        QString fkey;
        qint32 foffset;
        quint32 timesused, lastused;
        qint32 leftchild, rightchild;
        istream >> fkey >> foffset >> timesused >> lastused >> leftchild >> rightchild;
        entries.append(KPixmapCacheEntry(indexoffset, fkey, foffset, entries.count(), timesused, lastused));
        if (leftchild) {
            open.enqueue(leftchild);
        }
        if (rightchild) {
            open.enqueue(rightchild);
        }
    }


    // Sort the entries according to RemoveStrategy. This moves the best
    //  entries to the beginning of the list
    if (q->removeEntryStrategy() == RemoveOldest) {
        qSort(entries.begin(), entries.end(), compareEntriesByAge);
    } else if (q->removeEntryStrategy() == RemoveSeldomUsed) {
        qSort(entries.begin(), entries.end(), compareEntriesByTimesUsed);
    } else {
        qSort(entries.begin(), entries.end(), compareEntriesByLastUsed);
    }


    // Write some entries to the new files
    int entrieswritten = 0;
    for (entrieswritten = 0; entrieswritten < entries.count(); entrieswritten++) {
        const KPixmapCacheEntry& entry = entries[entrieswritten];
        // Load data
        datafile.seek(entry.dataoffset);
        int entrysize = -datafile.pos();
        // We have some duplication here but this way we avoid uncompressing
        //  the data and constructing QPixmap which we don't really need.
        QString fkey;
        dstream >> fkey;
        qint32 format, w, h, bpl;
        dstream >> format >> w >> h >> bpl;
        QByteArray imgdatacompressed;
        dstream >> imgdatacompressed;
        // Load custom data as well. This will be stored by the subclass itself.
        if (!q->loadCustomData(dstream)) {
            return false;
        }
        // Find out size of this entry
        entrysize += datafile.pos();

        // Make sure we'll stay within size limit
        if (newdatafile.size() + entrysize > newsize*1024) {
            break;
        }

        // Now write the same data to the new file
        int newdataoffset = newdatafile.pos();
        newdstream << fkey;
        newdstream << format << w << h << bpl;
        newdstream << imgdatacompressed;
        q->writeCustomData(newdstream);

        // Finally, add the index entry
        writeIndexEntry(newistream, entry.key, newdataoffset);
    }

    // Remove old files and rename the new ones
    indexfile.remove();
    datafile.remove();
    newindexfile.rename(mIndexFile);
    newdatafile.rename(mDataFile);

    kDebug() << k_funcinfo << "Wrote back" << entrieswritten << "of" << entries.count() << "entries" << endl;

    return true;
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
        kDebug() << k_funcinfo << "Pixmap cache '" + d->mName + "' is disabled";
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
    KPCLockFile lock(d->mLockFileName);
    if (!lock.isValid()) {
        // FIXME: now what?
        return;
    }
    QIODevice* device = d->indexDevice();
    if (!device) {
        return;
    }
    device->seek(d->kpc_header_len);
    QDataStream stream(device);
    stream << d->mTimestamp;

    delete device;
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
    d->mTimestamp = ::time(0);
    istream << d->mTimestamp;
    // Write cache id
    d->mCacheId = ::time(0);
    istream << d->mCacheId;

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
    d->mHeaderSize = indexfile.pos();

    d->mIndexRootOffset = d->mHeaderSize + sizeof(quint32);
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

    d->removeEntries(newsize);
}

bool KPixmapCache::find(const QString& key, QPixmap& pix)
{
    ensureInited();
    if (!isValid()) {
        return false;
    }

    //kDebug() << "KPC::find(" << key << "), use QPC = " << d->mUseQPixmapCache;
    // First try the QPixmapCache
    if (d->mUseQPixmapCache && QPixmapCache::find(key, pix)) {
        //kDebug() << k_funcinfo << "Found from QPC";
        return true;
    }

    KPCLockFile lock(d->mLockFileName);
    if (!lock.isValid()) {
        return false;
    }

    // Try to find the offset
    QString indexkey = d->indexKey(key);
    int offset = d->findOffset(indexkey);
    //kDebug() << "KPC: " << "found offset " << offset;
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
    // Open device and datastream on it
    QIODevice* device = d->dataDevice();
    if (!device) {
        return -1;
    }
    //kDebug() << "KPC: " << k_funcinfo << "Seeking to pos " << offset << "/" << file.size();
    if (!device->seek(offset)) {
        kError() << k_funcinfo << "Couldn't seek to pos " << offset << endl;
        delete device;
        return false;
    }
    QDataStream stream(device);

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
        delete device;
        return false;
    }

    delete device;
    if (stream.status() != QDataStream::Ok) {
        kError() << "KPC: " << k_funcinfo << "stream is bad :-(  status=" <<
                stream.status() << endl;
        return false;
    }

    //kDebug() << "KPC: " << k_funcinfo << "pixmap successfully loaded";
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

    //kDebug() << "KPC::insert(" << key << ", " << pix.width() << "x" << pix.height() << ")";
    // Insert to QPixmapCache as well
    if (d->mUseQPixmapCache) {
        QPixmapCache::insert(key, pix);
    }

    KPCLockFile lock(d->mLockFileName);
    if (!lock.isValid()) {
        return;
    }

    // Make sure this key isn't already in the cache
    QString indexkey = d->indexKey(key);
    int offset = d->findOffset(indexkey);
    if (offset >= 0) {
        // This pixmap is already in cache
        kDebug() << "KPC::insert() " << "pixmap already present in cache";
        return;
    }

    // Insert to cache
    offset = writeData(key, pix);
    //kDebug() << "KPC::insert(): " << "data is at offset " << offset;
    if (offset == -1) {
        return;
    }

    writeIndex(indexkey, offset);

    // Make sure the cache size stays within limits
    if (size() > cacheLimit()) {
        lock.unlock();
        d->removeEntries(int(cacheLimit() * 0.75));
    }
}

int KPixmapCache::writeData(const QString& key, const QPixmap& pix)
{
    // Open device and datastream on it
    QIODevice* device = d->dataDevice();
    if (!device) {
        return -1;
    }
    int offset = device->size();
    device->seek(offset);
    QDataStream stream(device);

    // Write the data
    stream << key;
    // Write image info and compressed data
    QImage img = pix.toImage();
    QByteArray imgdatacompressed = qCompress(img.bits(), img.numBytes());
    stream << (qint32)img.format() << (qint32)img.width() << (qint32)img.height() << (qint32)img.bytesPerLine();
    stream << imgdatacompressed;

    writeCustomData(stream);

    delete device;
    return offset;
}

bool KPixmapCache::writeCustomData(QDataStream&)
{
    return true;
}

void KPixmapCache::writeIndex(const QString& key, int dataoffset)
{
    // Open device and datastream on it
    QIODevice* device = d->indexDevice();
    if (!device) {
        return;
    }
    QDataStream stream(device);

    d->writeIndexEntry(stream, key, dataoffset);
    delete device;
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

