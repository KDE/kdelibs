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
#include <QtCore/QThread>
#include <QtCore/QTimer>
#include <QtCore/QMutex>
#include <QtCore/QMutexLocker>

#include <kglobal.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <klockfile.h>
#include <ksvgrenderer.h>
#include <kdefakes.h>

#include <config.h>

#include <time.h>
#include <unistd.h>
#ifdef HAVE_SYS_MMAN_H
#include <sys/mman.h>
#endif


//#define DISABLE_PIXMAPCACHE
#if defined HAVE_MMAP && !defined Q_WS_WIN
// Disabled due to bug 160284 while awaiting proper handling of SIGBUS.
//#define USE_MMAP
#endif

#define KPIXMAPCACHE_VERSION 0x000206

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

class KPCMemoryDevice : public QIODevice
{
public:
    KPCMemoryDevice(char* start, quint32* size, quint32 available);
    virtual ~KPCMemoryDevice();

    virtual qint64 size() const  { return *mSize; }
    void setSize(quint32 s)  { *mSize = s; }
    virtual bool seek(qint64 pos);

    static void setSizeEntryOffset(int o)  { mSizeEntryOffset = o; }

protected:
    virtual qint64 readData(char* data, qint64 maxSize);
    virtual qint64 writeData(const char* data, qint64 maxSize);

private:
    char* mMemory;
    quint32* mSize;
    quint32 mInitialSize;
    qint64 mAvailable;
    quint32 mPos;

    static int mSizeEntryOffset;  // Offset of file size entry
};
int KPCMemoryDevice::mSizeEntryOffset = 0;

KPCMemoryDevice::KPCMemoryDevice(char* start, quint32* size, quint32 available) : QIODevice()
{
    mMemory = start;
    mSize = size;
    mAvailable = available;
    mPos = 0;

    open(QIODevice::ReadWrite);

    // Load size
#if 0
    // Can anyone tell why this isn't working?
    seek(mSizeEntryOffset);
    QDataStream stream(this);
    stream >> *mSize;
#else
    // Load up-to-date size from the memory
    if (QSysInfo::ByteOrder == QSysInfo::LittleEndian) {
        char buf[4];
        uchar* p = (uchar*)mSize;
        memcpy(buf, mMemory + mSizeEntryOffset, 4);
        *p++ = buf[3];
        *p++ = buf[2];
        *p++ = buf[1];
        *p   = buf[0];
    } else {
        *mSize = *((quint32*)(mMemory + mSizeEntryOffset));
    }
#endif

    mInitialSize = *mSize;
}

KPCMemoryDevice::~KPCMemoryDevice()
{
    if (*mSize != mInitialSize) {
        // Update file size
        seek(mSizeEntryOffset);
        QDataStream stream(this);
        stream << *mSize;
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
        MmapInfo()  { file = 0; memory = 0; }
        QFile* file;  // If this is not null, then the file is mmapped
        char* memory;
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

    // Helper class to run the possibly expensive removeEntries() operation in
    //  the background thread
    class RemovalThread : public QThread
    {
    public:
        RemovalThread(KPixmapCache::Private* _d) : QThread()
        {
            d = _d;
            mRemovalScheduled = false;
        }
        ~RemovalThread()
        {
        }

        void scheduleRemoval(int newsize)
        {
            mNewSize = newsize;
            if (!mRemovalScheduled) {
                QTimer::singleShot(5000, this, SLOT(start()));
                mRemovalScheduled = true;
            }
        }

    protected:
        virtual void run()
        {
            mRemovalScheduled = false;
            kDebug(264) << "starting";
            d->removeEntries(mNewSize);
            kDebug(264) << "done";
        }

    private:
        bool mRemovalScheduled;
        int mNewSize;
        KPixmapCache::Private* d;
    };
    RemovalThread* mRemovalThread;
};

// Magic in the cache files
const char* KPixmapCache::Private::kpc_magic = "KDE PIXMAP CACHE ";
const int KPixmapCache::Private::kpc_magic_len = qstrlen(KPixmapCache::Private::kpc_magic);
// Whole header is magic + version (4 bytes)
const int KPixmapCache::Private::kpc_header_len = KPixmapCache::Private::kpc_magic_len + 4;


KPixmapCache::Private::Private(KPixmapCache* _q)
{
    q = _q;
    mRemovalThread = 0;

    KPCMemoryDevice::setSizeEntryOffset(kpc_header_len);  // ugly
}

bool KPixmapCache::Private::mmapFiles()
{
#ifdef USE_MMAP
    unmmapFiles();  // Noop if nothing has been mmapped
    if (!q->isValid()) {
        return false;
    }

    if (!mmapFile(mIndexFile, &mIndexMmapInfo, (int)(q->cacheLimit() * 0.4 + 100) * 1024)) {
        q->setValid(false);
        return false;
    }
    if (!mmapFile(mDataFile, &mDataMmapInfo, (int)(q->cacheLimit() * 1.5 + 500) * 1024)) {
        unmmapFile(&mIndexMmapInfo);
        q->setValid(false);
        return false;
    }

    return true;
#else
    return false;
#endif
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
#ifdef USE_MMAP
    // Set cache id to 0, this will force a reload the next time the files are used
    if (mIndexMmapInfo.file) {
        KPCMemoryDevice dev(mIndexMmapInfo.memory, &mIndexMmapInfo.size, mIndexMmapInfo.available);
        QDataStream stream(&dev);
        kDebug(264) << "Invalidating cache";
        dev.seek(kpc_header_len + sizeof(quint32));
        stream << (quint32)0;
    }
#endif
}

bool KPixmapCache::Private::mmapFile(const QString& filename, MmapInfo* info, int newsize)
{
#ifdef USE_MMAP
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
    if (!info->file->resize(info->available)) {
        kError(264) << "Couldn't resize" << filename << "to" << newsize;
        delete info->file;
        info->file = 0;
        return false;
    }

    void* indexMem = mmap(0, info->available, PROT_READ | PROT_WRITE, MAP_SHARED, info->file->handle(), 0);
    if (indexMem == MAP_FAILED) {
        kError() << "mmap failed for" << filename;
        delete info->file;
        info->file = 0;
        return false;
    }
    info->memory = reinterpret_cast<char*>(indexMem);
#ifdef HAVE_MADVISE
    madvise(info->memory, info->size, MADV_WILLNEED);
#endif

    // Update our stored file size
    int size = info->size;
    KPCMemoryDevice dev(info->memory, &info->size, info->available);
    if (!info->size) {
        // Null size was read from the file. This means that we're the first to
        //  mmap it and so we have to write the correct size to the file.
        dev.setSize(size);
        // New size will be written to file in KPCMemoryDevice dtor
    }

    return true;
#else
    return false;
#endif
}

void KPixmapCache::Private::unmmapFile(MmapInfo* info)
{
#ifdef USE_MMAP
    if (info->file) {
        munmap(info->memory, info->available);
        delete info->file;
        info->file = 0;
    }
#endif
}

QIODevice* KPixmapCache::Private::indexDevice()
{
    QIODevice* device = 0;
#ifdef USE_MMAP
    if (mIndexMmapInfo.file) {
        // Make sure the file still exists
        QFileInfo fi(mIndexFile);
        if (!fi.exists() || fi.size() != mIndexMmapInfo.available) {
            kDebug(264) << "File size has changed, re-initing";
            q->recreateCacheFiles();  // Also tries to re-init mmap
            if (!q->isValid()) {
                return 0;
            } else {
                return indexDevice();
            }
        }

        // Create the device
        device = new KPCMemoryDevice(mIndexMmapInfo.memory, &mIndexMmapInfo.size, mIndexMmapInfo.available);
    }
#endif
    if (!device) {
        QFile* file = new QFile(mIndexFile);
        if (!file->exists() || file->size() < kpc_header_len) {
            q->recreateCacheFiles();
        }
        if (!file->open(QIODevice::ReadWrite)) {
            kDebug(264) << "Couldn't open index file" << mIndexFile;
            delete file;
            return 0;
        }
        device = file;
    }
    // Make sure the device is up-to-date
    QDataStream stream(device);
    device->seek(kpc_header_len + sizeof(quint32));
    quint32 cacheid;
    stream >> cacheid;
    if (cacheid != mCacheId) {
        kDebug(264) << "Cache has changed, reloading";
        delete device;

        init();
        if (!q->isValid()) {
            return 0;
        } else {
            return indexDevice();
        }
    }

    return device;
}

QIODevice* KPixmapCache::Private::dataDevice()
{
#ifdef USE_MMAP
    if (mDataMmapInfo.file) {
        // TODO: make sure the mmapped memory is up-to-date
        KPCMemoryDevice* device = new KPCMemoryDevice(mDataMmapInfo.memory, &mDataMmapInfo.size, mDataMmapInfo.available);
        return device;
    }
#endif
    QFile* file = new QFile(mDataFile);
    if (!file->exists() || file->size() < kpc_header_len) {
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

        QDataStream stream(&f);

        // Check header and version
        char buf[100];
        quint32 version;
        stream.readRawData(buf, kpc_magic_len);
        stream >> version;
        if (qstrncmp(buf, kpc_magic, kpc_magic_len) == 0) {
            if (version == KPIXMAPCACHE_VERSION) {
                return true;
            } else if (version < KPIXMAPCACHE_VERSION) {
                kDebug(264) << "File" << filename << "is outdated, will recreate...";
            } else {
                // Don't recreate the cache if it has newer version to avoid
                //  problems when upgrading kdelibs.
                kDebug(264) << "File" << filename << "has newer version, disabling cache";
                return false;
            }
        } else {
            kDebug(264) << "File" << filename << "is not KPixmapCache file, will recreate...";
        }
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
    file.seek(kpc_header_len);

    QDataStream stream(&file);
    stream >> mDataMmapInfo.size;

    return true;
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
    // Load file size
    stream >> mIndexMmapInfo.size;
    // Load cache id
    stream >> mCacheId;
    // Load timestamp
    stream >> mTimestamp;

    if (stream.status() != QDataStream::Ok) {
        kWarning() << "Failed to read index file's header";
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
    // Find parent index node for this node.
    int parentoffset = binarySearchKey(stream, key, mIndexRootOffset);
    if (parentoffset != stream.device()->size()) {
        // Check if this entry has the same key
        QString fkey;
        stream.device()->seek(parentoffset);
        stream >> fkey;
        if (key == fkey) {
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

bool KPixmapCache::Private::scheduleRemoveEntries(int newsize)
{
    if (!mRemovalThread) {
        mRemovalThread = new RemovalThread(this);
    }
    mRemovalThread->scheduleRemoval(newsize);
    return true;
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
    newistream.writeRawData(header, mHeaderSize);
    // Write root index node offset
    newistream << (quint32)(mHeaderSize + sizeof(quint32));
    // Copy data file header
    int dataheaderlen = kpc_header_len + sizeof(quint32);
    // mHeaderSize is always bigger than dataheaderlen, so we needn't create
    //  new buffer
    if (dstream.readRawData(header, dataheaderlen) != dataheaderlen) {
        kDebug(264) << "Couldn't read data header";
        delete [] header;
        return false;
    }
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

    // Set file size to 0 for mmap stuff
    newindexfile.seek(kpc_header_len);
    newistream << (quint32)0;
    newdatafile.seek(kpc_header_len);
    newdstream << (quint32)0;

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
    if (d->mRemovalThread) {
        d->mRemovalThread->wait();
        delete d->mRemovalThread;
    }
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
    device->seek(d->kpc_header_len + 2*sizeof(quint32));
    QDataStream stream(device);
    stream << d->mTimestamp;

    delete device;
}

int KPixmapCache::size() const
{
    ensureInited();
#ifdef USE_MMAP
    if (d->mDataMmapInfo.file) {
        return d->mDataMmapInfo.size / 1024;
    }
#endif
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
    if (!isEnabled()) {
        return false;
    }

    d->invalidateMmapFiles();

    d->mEnabled = false;
    // Create index file
    QFile indexfile(d->mIndexFile);
    if (!indexfile.open(QIODevice::WriteOnly)) {
        kError() << "Couldn't create index file" << d->mIndexFile;
        return false;
    }
    QDataStream istream(&indexfile);
    istream.writeRawData(d->kpc_magic, d->kpc_magic_len);
    istream << (quint32)KPIXMAPCACHE_VERSION;
    // Write invalid file size, valid one will be written later by mmap code
    d->mIndexMmapInfo.size = 0;
    istream << d->mIndexMmapInfo.size;
    // Write cache id
    d->mCacheId = ::time(0);
    istream << d->mCacheId;
    // Write default timestamp
    d->mTimestamp = ::time(0);
    istream << d->mTimestamp;

    // Create data file
    QFile datafile(d->mDataFile);
    if (!datafile.open(QIODevice::WriteOnly)) {
        kError() << "Couldn't create data file" << d->mDataFile;
        return false;
    }
    QDataStream dstream(&datafile);
    dstream.writeRawData(d->kpc_magic, d->kpc_magic_len);
    dstream << (quint32)KPIXMAPCACHE_VERSION;
    d->mDataMmapInfo.size = 0;
    dstream << d->mDataMmapInfo.size;

    setValid(true);
    writeCustomIndexHeader(istream);
    d->mHeaderSize = indexfile.pos();

    d->mIndexRootOffset = d->mHeaderSize + sizeof(quint32);
    istream << d->mIndexRootOffset;

    // Close the files and mmap them (if mmapping is used)
    indexfile.close();
    datafile.close();
    d->mEnabled = true;
    d->mmapFiles();
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
    d->invalidateMmapFiles();

    d->mInited = false;
    d->init();
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

    //kDebug(264) << "key:" << key << ", use QPC:" << d->mUseQPixmapCache;
    // First try the QPixmapCache
    if (d->mUseQPixmapCache && QPixmapCache::find(key, pix)) {
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
        QPixmapCache::insert(key, pix);
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
        QPixmapCache::insert(key, pix);
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
    if (size() > cacheLimit()) {
        lock.unlock();
        if (size() > (int)(cacheLimit() * 1.2)) {
            // Can't wait any longer, do it immediately
            d->removeEntries(int(cacheLimit() * 0.75));
        } else {
            d->scheduleRemoveEntries(int(cacheLimit() * 0.75));
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

