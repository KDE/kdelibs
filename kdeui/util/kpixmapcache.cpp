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
#include <QtCore/QTimer>
#include <QtCore/QMutex>
#include <QtCore/QMutexLocker>
#include <QtCore/QList>

#include <kglobal.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <klockfile.h>
#include <ksavefile.h>
#include <ksvgrenderer.h>
#include <kdefakes.h>

#include <config.h>

#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>

#if defined(HAVE_MADVISE)
#include <sys/mman.h>
#endif

//#define DISABLE_PIXMAPCACHE

#ifdef Q_OS_SOLARIS
#ifndef _XPG_4_2
extern "C" int madvise(caddr_t addr, size_t len, int advice);
#endif
#endif

#define KPIXMAPCACHE_VERSION 0x000208

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
            kError() << "Failed to lock file" << filename << ", last result =" << result;
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

// Contained in the header so we will know if we created this or not.  Older
// versions of kdelibs had the version on the byte right after "CACHE ".
// "DEUX" will be read as a quint32 by such versions, and will always be
// greater than the last such version (0x000207), whether a big-endian or
// little-endian system is used.  Therefore older systems will correctly
// recognize that this is from a newer kdelibs.  (This is an issue since old
// and new kdelibs do not read the version from the exact same spot.)
static const char KPC_MAGIC[] = "KDE PIXMAP CACHE DEUX";
struct KPixmapCacheDataHeader
{
    // -1 from sizeof so we don't write out the trailing null.  If you change
    // the list of members change them in the KPixmapCacheIndexHeader as well!
    char    magic[sizeof(KPC_MAGIC) - 1];
    quint32 cacheVersion;
    quint32 size;
};

struct KPixmapCacheIndexHeader
{
    // -1 from sizeof so we don't write out the trailing null.
    // The follow are also in KPixmapCacheDataHeader
    char    magic[sizeof(KPC_MAGIC) - 1];
    quint32 cacheVersion;
    quint32 size;

    // These belong only to this header type.
    quint32 cacheId;
    time_t  timestamp;
};

class KPCMemoryDevice : public QIODevice
{
public:
    KPCMemoryDevice(char* start, quint32* size, quint32 available);
    virtual ~KPCMemoryDevice();

    virtual qint64 size() const  { return *mSize; }
    void setSize(quint32 s)  { *mSize = s; }
    virtual bool seek(qint64 pos);

protected:
    virtual qint64 readData(char* data, qint64 maxSize);
    virtual qint64 writeData(const char* data, qint64 maxSize);

private:
    char* mMemory;
    KPixmapCacheIndexHeader *mHeader; // alias of mMemory
    quint32* mSize;
    quint32 mInitialSize;
    qint64 mAvailable;
    quint32 mPos;
};

KPCMemoryDevice::KPCMemoryDevice(char* start, quint32* size, quint32 available) : QIODevice()
{
    mMemory = start;
    mHeader = reinterpret_cast<KPixmapCacheIndexHeader *>(start);
    mSize = size;
    mAvailable = available;
    mPos = 0;

    this->open(QIODevice::ReadWrite);

    // Load up-to-date size from the memory
    *mSize = mHeader->size;

    mInitialSize = *mSize;
}

KPCMemoryDevice::~KPCMemoryDevice()
{
    if (*mSize != mInitialSize) {
        // Update file size
        mHeader->size = *mSize;
    }
}

bool KPCMemoryDevice::seek(qint64 pos)
{
    if (pos < 0 || pos > *mSize) {
        return false;
    }
    mPos = pos;
    return QIODevice::seek(pos);
}

qint64 KPCMemoryDevice::readData(char* data, qint64 len)
{
    len = qMin(len, qint64(*mSize) - mPos);
    if (len <= 0) {
        return 0;
    }
    memcpy(data, mMemory + mPos, len);
    mPos += len;
    return len;
}

qint64 KPCMemoryDevice::writeData(const char* data, qint64 len)
{
    if (mPos + len > mAvailable) {
        kError() << "Overflow of" << mPos+len - mAvailable;
        return -1;
    }
    memcpy(mMemory + mPos, (uchar*)data, len);
    mPos += len;
    *mSize = qMax(*mSize, mPos);
    return len;
}


} // namespace

class KPixmapCache::Private
{
public:
    Private(KPixmapCache* q);
    ~Private();

    // Return device used to read from index or data file. The device is either
    //  QFile or KPCMemoryDevice (if mmap is used)
    QIODevice* indexDevice();
    QIODevice* dataDevice();

    // Unmmaps any currently mmapped files and then tries to (re)mmap the cache
    //  files. If mmapping is disabled then it does nothing
    bool mmapFiles();
    void unmmapFiles();
    // Marks the shared mmapped files as invalid so that all processes will
    //  reload the files
    void invalidateMmapFiles();

    // List of all KPixmapCache::Private instances in this process.
    static QList<KPixmapCache::Private *> mCaches;

    static unsigned kpcNumber; // Used to setup for qpcKey

    int findOffset(const QString& key);
    int binarySearchKey(QDataStream& stream, const QString& key, int start);
    void writeIndexEntry(QDataStream& stream, const QString& key, int dataoffset);

    bool checkLockFile();
    bool checkFileVersion(const QString& filename);
    bool loadIndexHeader();
    bool loadDataHeader();

    bool removeEntries(int newsize);
    bool scheduleRemoveEntries(int newsize);

    void init();
    bool loadData(int offset, QPixmap& pix);
    int writeData(const QString& key, const QPixmap& pix);
    void writeIndex(const QString& key, int offset);

    // Prepends key's hash to the key. This makes comparisons and key
    //  lookups faster as the beginnings of the keys are more random
    QString indexKey(const QString& key);

    // Returns a QString suitable for use in the static QPixmapCache, which
    // differentiates each KPC object in the process.
    QString qpcKey(const QString& key) const;

    KPixmapCache* q;

    QString mThisString; // Used by qpcKey
    quint32 mHeaderSize;  // full size of the index header, including custom (subclass') header data
    quint32 mIndexRootOffset;  // offset of the first entry in index file

    QString mName;
    QString mIndexFile;
    QString mDataFile;
    QString mLockFileName;
    QMutex mMutex;

    quint32 mTimestamp;
    quint32 mCacheId;  // Unique id, will change when cache is recreated
    int mCacheLimit;
    RemoveStrategy mRemoveStrategy:4;
    bool mUseQPixmapCache:4;

    bool mInited:8;  // Whether init() has been called (it's called on-demand)
    bool mEnabled:8;   // whether it's possible to use the cache
    bool mValid:8;  // whether cache has been inited and is ready to be used

    // Holds info about mmapped file
    struct MmapInfo
    {
        MmapInfo()  { file = 0; indexHeader = 0; }
        QFile* file;  // If this is not null, then the file is mmapped

        // This points to the mmap'ed file area.
        KPixmapCacheIndexHeader *indexHeader;

        quint32 size;  // Number of currently used bytes
        quint32 available;  // Number of available bytes (including those reserved for mmap)
    };
    MmapInfo mIndexMmapInfo;
    MmapInfo mDataMmapInfo;
    // Mmaps given file, growing it to newsize bytes.
    bool mmapFile(const QString& filename, MmapInfo* info, int newsize);
    void unmmapFile(MmapInfo* info);


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

    // Various comparison functions for different removal strategies
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

// List of KPixmapCache::Private instances.
QList<KPixmapCache::Private *> KPixmapCache::Private::mCaches;

unsigned KPixmapCache::Private::kpcNumber = 0;

KPixmapCache::Private::Private(KPixmapCache* _q)
{
    q = _q;
    mCaches.append(this);
    mThisString = QString("%1").arg(kpcNumber++);
}

KPixmapCache::Private::~Private()
{
    mCaches.removeAll(this);
}

bool KPixmapCache::Private::mmapFiles()
{
    unmmapFiles();  // Noop if nothing has been mmapped
    if (!q->isValid()) {
        return false;
    }

    //TODO: 100MB limit if we have no cache limit, is that sensible?
    int cacheLimit = mCacheLimit > 0 ? mCacheLimit : 100 * 1024;
    if (!mmapFile(mIndexFile, &mIndexMmapInfo, (int)(cacheLimit * 0.4 + 100) * 1024)) {
        q->setValid(false);
        return false;
    }

    if (!mmapFile(mDataFile, &mDataMmapInfo, (int)(cacheLimit * 1.5 + 500) * 1024)) {
        unmmapFile(&mIndexMmapInfo);
        q->setValid(false);
        return false;
    }

    return true;
}

void KPixmapCache::Private::unmmapFiles()
{
    unmmapFile(&mIndexMmapInfo);
    unmmapFile(&mDataMmapInfo);
}

void KPixmapCache::Private::invalidateMmapFiles()
{
    if (!q->isValid())
        return;
    // Set cache id to 0, this will force a reload the next time the files are used
    if (mIndexMmapInfo.file) {
        kDebug(264) << "Invalidating cache";
        mIndexMmapInfo.indexHeader->cacheId = 0;
    }
}

bool KPixmapCache::Private::mmapFile(const QString& filename, MmapInfo* info, int newsize)
{
    info->file = new QFile(filename);
    if (!info->file->open(QIODevice::ReadWrite)) {
        kDebug(264) << "Couldn't open" << filename;
        delete info->file;
        info->file = 0;
        return false;
    }

    if (!info->size) {
        info->size = info->file->size();
    }
    info->available = newsize;

    // Only resize if greater than current file size, otherwise we may cause SIGBUS
    // errors from mmap().
    if (info->file->size() < info->available && !info->file->resize(info->available)) {
        kError(264) << "Couldn't resize" << filename << "to" << newsize;
        delete info->file;
        info->file = 0;
        return false;
    }

    //void* indexMem = mmap(0, info->available, PROT_READ | PROT_WRITE, MAP_SHARED, info->file->handle(), 0);
    void *indexMem = info->file->map(0, info->available);
    if (indexMem == 0) {
        kError() << "mmap failed for" << filename;
        delete info->file;
        info->file = 0;
        return false;
    }
    info->indexHeader = reinterpret_cast<KPixmapCacheIndexHeader *>(indexMem);
#ifdef HAVE_MADVISE
    posix_madvise(indexMem, info->size, POSIX_MADV_WILLNEED);
#endif

    info->file->close();

    // Update our stored file size.  Other objects that have this mmaped will have to
    // invalidate their map if size is different.
    if(0 == info->indexHeader->size) {
        // This size includes index header and and custom headers tacked on
        // by subclasses.
        info->indexHeader->size = mHeaderSize;
        info->size = info->indexHeader->size;
    }

    return true;
}

void KPixmapCache::Private::unmmapFile(MmapInfo* info)
{
    if (info->file) {
        info->file->unmap(reinterpret_cast<uchar*>(info->indexHeader));
        info->indexHeader = 0;
        info->available = 0;
        info->size = 0;

        delete info->file;
        info->file = 0;
    }
}


QIODevice* KPixmapCache::Private::indexDevice()
{
    QIODevice* device = 0;

    if (mIndexMmapInfo.file) {
        // Make sure the file still exists
        QFileInfo fi(mIndexFile);

        if (!fi.exists() || fi.size() != mIndexMmapInfo.available) {
            kDebug(264) << "File size has changed, re-initializing.";
            q->recreateCacheFiles(); // Recreates memory maps as well.
        }

        fi.refresh();
        if(fi.exists() && fi.size() == mIndexMmapInfo.available) {
            // Create the device
            device = new KPCMemoryDevice(
                             reinterpret_cast<char*>(mIndexMmapInfo.indexHeader),
                             &mIndexMmapInfo.size, mIndexMmapInfo.available);
        }

        // Is it possible to have a valid cache with no file?  If not it would be easier
        // to do return 0 in the else portion of the prior test.
        if(!q->isValid()) {
            delete device;
            return 0;
        }
    }

    if (!device) {
        QFile* file = new QFile(mIndexFile);
        if (!file->exists() || (size_t) file->size() < sizeof(KPixmapCacheIndexHeader)) {
            q->recreateCacheFiles();
        }

        if (!q->isValid() || !file->open(QIODevice::ReadWrite)) {
            kDebug(264) << "Couldn't open index file" << mIndexFile;
            delete file;
            return 0;
        }

        device = file;
    }

    // Make sure the device is up-to-date
    KPixmapCacheIndexHeader indexHeader;

    int numRead = device->read(reinterpret_cast<char *>(&indexHeader), sizeof indexHeader);
    if (sizeof indexHeader != numRead) {
        kError(264) << "Unable to read header from pixmap cache index.";
        delete device;
        return 0;
    }

    if (indexHeader.cacheId != mCacheId) {
        kDebug(264) << "Cache has changed, reloading";
        delete device;

        init();
        if (!q->isValid()) {
            return 0;
        } else {
            return indexDevice(); // Careful, this is a recursive call.
        }
    }

    return device;
}

QIODevice* KPixmapCache::Private::dataDevice()
{
    if (mDataMmapInfo.file) {
        // Make sure the file still exists
        QFileInfo fi(mDataFile);

        if (!fi.exists() || fi.size() != mDataMmapInfo.available) {
            kDebug(264) << "File size has changed, re-initializing.";
            q->recreateCacheFiles(); // Recreates memory maps as well.

            // Index file has also been recreated so we cannot continue with
            //  modifying the data file because it would make things inconsistent.
            return 0;
        }

        fi.refresh();
        if (fi.exists() && fi.size() == mDataMmapInfo.available) {
            // Create the device
            return new KPCMemoryDevice(
                           reinterpret_cast<char*>(mDataMmapInfo.indexHeader),
                           &mDataMmapInfo.size, mDataMmapInfo.available);
        }
        else
            return 0;
    }

    QFile* file = new QFile(mDataFile);
    if (!file->exists() || (size_t) file->size() < sizeof(KPixmapCacheDataHeader)) {
        q->recreateCacheFiles();
        // Index file has also been recreated so we cannot continue with
        //  modifying the data file because it would make things inconsistent.
        delete file;
        return 0;
    }
    if (!file->open(QIODevice::ReadWrite)) {
        kDebug(264) << "Couldn't open data file" << mDataFile;
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

    if (fkey.isEmpty()) {
        return start;
    }

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
        // One exception is that the root node may exist but be invalid,
        // which can happen when the cache data is discarded. This is
        // represented by an empty fkey
        QString fkey;
        stream >> fkey;

        if (fkey.isEmpty()) {
            delete device;
            return -1;
        }

        int nodeoffset = binarySearchKey(stream, key, mIndexRootOffset);

        // Load the found entry and check if it's the one we're looking for.
        device->seek(nodeoffset);
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
            kError() << "Couldn't remove lockfile" << mLockFileName;
            return false;
        }
    }
    return true;
}

bool KPixmapCache::Private::checkFileVersion(const QString& filename)
{
    if (!mEnabled) {
        return false;
    }

    if (QFile::exists(filename)) {
        // File already exists, make sure it can be opened
        QFile f(filename);
        if (!f.open(QIODevice::ReadOnly)) {
            kError() << "Couldn't open file" << filename;
            return false;
        }

        // The index header is the same as the beginning of the data header (on purpose),
        // so use index header for either one.
        KPixmapCacheIndexHeader indexHeader;

        // Ensure we have a valid cache.
        if(sizeof indexHeader != f.read(reinterpret_cast<char*>(&indexHeader), sizeof indexHeader) ||
           qstrncmp(indexHeader.magic, KPC_MAGIC, sizeof(indexHeader.magic)) != 0)
        {
            kDebug(264) << "File" << filename << "is not KPixmapCache file, or is";
            kDebug(264) << "version <= 0x000207, will recreate...";
            return q->recreateCacheFiles();
        }

        if(indexHeader.cacheVersion == KPIXMAPCACHE_VERSION)
            return true;

        // Don't recreate the cache if it has newer version to avoid
        //  problems when upgrading kdelibs.
        if(indexHeader.cacheVersion > KPIXMAPCACHE_VERSION) {
            kDebug(264) << "File" << filename << "has newer version, disabling cache";
            return false;
        }

        kDebug(264) << "File" << filename << "is outdated, will recreate...";
    }

    return q->recreateCacheFiles();
}

bool KPixmapCache::Private::loadDataHeader()
{
    // Open file and datastream on it
    QFile file(mDataFile);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    KPixmapCacheDataHeader dataHeader;
    if(sizeof dataHeader != file.read(reinterpret_cast<char*>(&dataHeader), sizeof dataHeader)) {
        kDebug(264) << "Unable to read from data file" << mDataFile;
        return false;
    }

    mDataMmapInfo.size = dataHeader.size;
    return true;
}

bool KPixmapCache::Private::loadIndexHeader()
{
    // Open file and datastream on it
    QFile file(mIndexFile);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    KPixmapCacheIndexHeader indexHeader;
    if(sizeof indexHeader != file.read(reinterpret_cast<char*>(&indexHeader), sizeof indexHeader)) {
        kWarning(264) << "Failed to read index file's header";
        q->recreateCacheFiles();
        return false;
    }

    mCacheId = indexHeader.cacheId;
    mTimestamp = indexHeader.timestamp;
    mIndexMmapInfo.size = indexHeader.size;

    QDataStream stream(&file);

    // Give custom implementations chance to load their headers
    if (!q->loadCustomIndexHeader(stream)) {
        return false;
    }

    mHeaderSize = file.pos();
    mIndexRootOffset = file.pos();

    return true;
}

QString KPixmapCache::Private::indexKey(const QString& key)
{
    const QByteArray latin1 = key.toLatin1();
    return QString("%1%2").arg((ushort)qChecksum(latin1.data(), latin1.size()), 4, 16, QLatin1Char('0')).arg(key);
}


QString KPixmapCache::Private::qpcKey(const QString& key) const
{
    return mThisString + key;
}

void KPixmapCache::Private::writeIndexEntry(QDataStream& stream, const QString& key, int dataoffset)
{
    // New entry will be written to the end of the file
    qint32 offset = stream.device()->size();
    // Find parent index node for this node.
    int parentoffset = binarySearchKey(stream, key, mIndexRootOffset);
    if (parentoffset != stream.device()->size()) {
        // Check if this entry has the same key
        QString fkey;
        stream.device()->seek(parentoffset);
        stream >> fkey;

        // The key would be empty if the cache had been discarded.
        if (key == fkey || fkey.isEmpty()) {
            // We're overwriting an existing entry
            offset = parentoffset;
        }
    }

    stream.device()->seek(offset);
    // Write the data
    stream << key << (qint32)dataoffset;
    // Statistics (# of uses and last used timestamp)
    stream << (quint32)1 << (quint32)::time(0);
    // Write (empty) children offsets
    stream << (qint32)0 << (qint32)0;

    // If we created the root node or overwrote existing entry then the two
    //  offsets are equal and we're done. Otherwise set parent's child offset
    //  to correct value.
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
        kDebug(264) << "Couldn't lock cache" << mName;
        return false;
    }
    QMutexLocker mutexlocker(&mMutex);

    // Open old (current) files
    QFile indexfile(mIndexFile);
    if (!indexfile.open(QIODevice::ReadOnly)) {
        kDebug(264) << "Couldn't open old index file";
        return false;
    }
    QDataStream istream(&indexfile);
    QFile datafile(mDataFile);
    if (!datafile.open(QIODevice::ReadOnly)) {
        kDebug(264) << "Couldn't open old data file";
        return false;
    }
    if (datafile.size() <= newsize*1024) {
        kDebug(264) << "Cache size is already within limit (" << datafile.size() << " <= " << newsize*1024 << ")";
        return true;
    }
    QDataStream dstream(&datafile);
    // Open new files
    QFile newindexfile(mIndexFile + ".new");
    if (!newindexfile.open(QIODevice::ReadWrite)) {
        kDebug(264) << "Couldn't open new index file";
        return false;
    }
    QDataStream newistream(&newindexfile);
    QFile newdatafile(mDataFile + ".new");
    if (!newdatafile.open(QIODevice::WriteOnly)) {
        kDebug(264) << "Couldn't open new data file";
        return false;
    }
    QDataStream newdstream(&newdatafile);

    // Copy index file header
    char* header = new char[mHeaderSize];
    if (istream.readRawData(header, mHeaderSize) != (int)mHeaderSize) {
        kDebug(264) << "Couldn't read index header";
        delete [] header;
        return false;
    }

    // Set file size to 0 for mmap stuff
    reinterpret_cast<KPixmapCacheIndexHeader *>(header)->size = 0;
    newistream.writeRawData(header, mHeaderSize);

    // Copy data file header
    int dataheaderlen = sizeof(KPixmapCacheDataHeader);

    // mHeaderSize is always bigger than dataheaderlen, so we needn't create
    //  new buffer
    if (dstream.readRawData(header, dataheaderlen) != dataheaderlen) {
        kDebug(264) << "Couldn't read data header";
        delete [] header;
        return false;
    }

    // Set file size to 0 for mmap stuff
    reinterpret_cast<KPixmapCacheDataHeader *>(header)->size = 0;
    newdstream.writeRawData(header, dataheaderlen);
    delete [] header;

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
    invalidateMmapFiles();

    kDebug(264) << "Wrote back" << entrieswritten << "of" << entries.count() << "entries";

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
    d->unmmapFiles();
    delete d;
}

void KPixmapCache::Private::init()
{
    mInited = true;

#ifdef DISABLE_PIXMAPCACHE
    mValid = mEnabled = false;
#else
    mValid = false;

    // Find locations of the files
    mIndexFile = KGlobal::dirs()->locateLocal("cache", "kpc/" + mName + ".index");
    mDataFile  = KGlobal::dirs()->locateLocal("cache", "kpc/" + mName + ".data");
    mLockFileName = KGlobal::dirs()->locateLocal("cache", "kpc/" + mName + ".lock");

    mEnabled = true;
    mEnabled &= checkLockFile();
    mEnabled &= checkFileVersion(mDataFile);
    mEnabled &= checkFileVersion(mIndexFile);
    if (!mEnabled) {
        kDebug(264) << "Pixmap cache" << mName << "is disabled";
    } else {
        // Cache is enabled, but check if it's ready for use
        loadDataHeader();
        q->setValid(loadIndexHeader());
        // Init mmap stuff if mmap is used
        mmapFiles();
    }
#endif
}

void KPixmapCache::ensureInited() const
{
    if (!d->mInited) {
        const_cast<KPixmapCache*>(this)->d->init();
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
    return d->mEnabled && d->mValid;
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

    KPixmapCacheIndexHeader header;
    device->seek(0);
    if(sizeof header != device->read(reinterpret_cast<char*>(&header), sizeof header)) {
        delete device;
        return;
    }

    header.timestamp = ts;
    device->seek(0);
    device->write(reinterpret_cast<char *>(&header), sizeof header);

    delete device;
}

int KPixmapCache::size() const
{
    ensureInited();
    if (d->mDataMmapInfo.file) {
        return d->mDataMmapInfo.size / 1024;
    }
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
    //FIXME: KDE5: this should be uint!
    if (kbytes < 0) {
        return;
    }

    d->mCacheLimit = kbytes;

    // if we are initialized, let's make sure that we are actually within
    // our limits.
    if (d->mInited && d->mCacheLimit && size() > d->mCacheLimit) {
        if (size() > (int)(d->mCacheLimit)) {
            // Can't wait any longer, do it immediately
            d->removeEntries(d->mCacheLimit * 0.65);
        }
    }
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
    if (!isEnabled()) {
        return false;
    }

    KPCLockFile lock(d->mLockFileName);
    // Hope we got the lock...

    d->invalidateMmapFiles();
    d->mEnabled = false;

    // Create index file
    KSaveFile indexfile(d->mIndexFile);
    if (!indexfile.open(QIODevice::WriteOnly)) {
        kError() << "Couldn't create index file" << d->mIndexFile;
        return false;
    }

    d->mCacheId = ::time(0);
    d->mTimestamp = ::time(0);

    // We can't know the full size until custom headers written.
    // mmapFiles() will take care of correcting the size.
    KPixmapCacheIndexHeader indexHeader = { {0}, KPIXMAPCACHE_VERSION, 0, d->mCacheId, d->mTimestamp };
    memcpy(indexHeader.magic, KPC_MAGIC, sizeof(indexHeader.magic));

    indexfile.write(reinterpret_cast<char*>(&indexHeader), sizeof indexHeader);

    // Create data file
    KSaveFile datafile(d->mDataFile);
    if (!datafile.open(QIODevice::WriteOnly)) {
        kError() << "Couldn't create data file" << d->mDataFile;
        return false;
    }

    KPixmapCacheDataHeader dataHeader = { {0}, KPIXMAPCACHE_VERSION, sizeof dataHeader };
    memcpy(dataHeader.magic, KPC_MAGIC, sizeof(dataHeader.magic));

    datafile.write(reinterpret_cast<char*>(&dataHeader), sizeof dataHeader);

    setValid(true);

    QDataStream istream(&indexfile);
    writeCustomIndexHeader(istream);
    d->mHeaderSize = indexfile.pos();

    d->mIndexRootOffset = d->mHeaderSize;

    // Close the files and mmap them (if mmapping is used)
    indexfile.close();
    datafile.close();
    indexfile.finalize();
    datafile.finalize();

    d->mEnabled = true;
    d->mmapFiles();

    return true;
}

void KPixmapCache::deleteCache(const QString& name)
{
    QString indexFile = KGlobal::dirs()->locateLocal("cache", "kpc/" + name + ".index");
    QString dataFile  = KGlobal::dirs()->locateLocal("cache", "kpc/" + name + ".data");

    QFile::remove(indexFile);
    QFile::remove(dataFile);
}

void KPixmapCache::discard()
{
    // To "discard" the cache we simply have to make sure that every that
    // was in there before is no longer present when we search for them.
    // Easiest way to do *that* is to simply delete the index.

    KPCLockFile lock(d->mLockFileName);
    if(!lock.isValid()) {
        kError(264) << "Unable to lock pixmap cache when trying to discard it";
        return;
    }

    QIODevice *device = d->indexDevice();
    if (!device) {
        kError(264) << "Unable to access index when trying to discard cache";
        return;
    }

    device->seek(d->mIndexRootOffset);
    QDataStream stream(device);

    // Stream an empty QString as the hash key to signify that the cache
    // has been discarded.
    stream << QString();

    if (d->mUseQPixmapCache) {
        // TODO: This is broken, it removes every cached QPixmap in the whole
        // process, not just this KPixmapCache.
        QPixmapCache::clear();
    }
}

void KPixmapCache::removeEntries(int newsize)
{
    if (!newsize) {
        newsize = d->mCacheLimit;

        if (!newsize) {
            // nothing to do!
            return;
        }
    }

    d->removeEntries(newsize);
}

bool KPixmapCache::find(const QString& key, QPixmap& pix)
{
    ensureInited();
    if (!isValid()) {
        return false;
    }

    //kDebug(264) << "key:" << key << ", use QPC:" << d->mUseQPixmapCache;
    // First try the QPixmapCache
    if (d->mUseQPixmapCache && QPixmapCache::find(d->qpcKey(key), &pix)) {
        //kDebug(264) << "Found from QPC";
        return true;
    }

    KPCLockFile lock(d->mLockFileName);
    if (!lock.isValid()) {
        return false;
    }

    // Try to find the offset
    QString indexkey = d->indexKey(key);
    int offset = d->findOffset(indexkey);
    //kDebug(264) << "found offset" << offset;
    if (offset == -1) {
        return false;
    }

    // Load the data
    bool ret = d->loadData(offset, pix);
    if (ret && d->mUseQPixmapCache) {
        // This pixmap wasn't in QPC, put it there
        QPixmapCache::insert(d->qpcKey(key), pix);
    }
    return ret;
}

bool KPixmapCache::Private::loadData(int offset, QPixmap& pix)
{
    // Open device and datastream on it
    QIODevice* device = dataDevice();
    if (!device) {
        return false;
    }
    //kDebug(264) << "Seeking to pos" << offset << "/" << file.size();
    if (!device->seek(offset)) {
        kError() << "Couldn't seek to pos" << offset;
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
    if (!imgdata.isEmpty()) {
        QImage img((const uchar*)imgdata.constData(), w, h, bpl, (QImage::Format)format);
        img.bits();  // make deep copy since we don't want to keep imgdata around
        pix = QPixmap::fromImage(img);
    } else {
        pix = QPixmap(w, h);
    }

    if (!q->loadCustomData(stream)) {
        delete device;
        return false;
    }

    delete device;
    if (stream.status() != QDataStream::Ok) {
        kError() << "stream is bad :-(  status=" << stream.status();
        return false;
    }

    //kDebug(264) << "pixmap successfully loaded";
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

    //kDebug(264) << "key:" << key << ", size:" << pix.width() << "x" << pix.height();
    // Insert to QPixmapCache as well
    if (d->mUseQPixmapCache) {
        QPixmapCache::insert(d->qpcKey(key), pix);
    }

    KPCLockFile lock(d->mLockFileName);
    if (!lock.isValid()) {
        return;
    }

    // Insert to cache
    QString indexkey = d->indexKey(key);
    int offset = d->writeData(key, pix);
    //kDebug(264) << "data is at offset" << offset;
    if (offset == -1) {
        return;
    }

    d->writeIndex(indexkey, offset);

    // Make sure the cache size stays within limits
    if (d->mCacheLimit && size() > d->mCacheLimit) {
        lock.unlock();
        if (size() > (int)(d->mCacheLimit)) {
            // Can't wait any longer, do it immediately
            d->removeEntries(d->mCacheLimit * 0.65);
        }
    }
}

int KPixmapCache::Private::writeData(const QString& key, const QPixmap& pix)
{
    // Open device and datastream on it
    QIODevice* device = dataDevice();
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

    q->writeCustomData(stream);

    delete device;
    return offset;
}

bool KPixmapCache::writeCustomData(QDataStream&)
{
    return true;
}

void KPixmapCache::Private::writeIndex(const QString& key, int dataoffset)
{
    // Open device and datastream on it
    QIODevice* device = indexDevice();
    if (!device) {
        return;
    }
    QDataStream stream(device);

    writeIndexEntry(stream, key, dataoffset);
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

QPixmap KPixmapCache::loadFromSvg(const QString& filename, const QSize& size)
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

