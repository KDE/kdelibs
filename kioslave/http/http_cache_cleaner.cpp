/*
This file is part of KDE

 Copyright (C) 1999-2000 Waldo Bastian (bastian@kde.org)
 Copyright (C) 2009 Andreas Hartmetz (ahartmetz@gmail.com)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
//----------------------------------------------------------------------------
//
// KDE HTTP Cache cleanup tool

#include <time.h>
#include <stdlib.h>
#include <zlib.h>

#include <QtCore/QDir>
#include <QtCore/QString>
#include <QtCore/QTime>
#include <QtDBus/QtDBus>
#include <QtNetwork/QLocalServer>
#include <QtNetwork/QLocalSocket>

#include <kcmdlineargs.h>
#include <kcomponentdata.h>
#include <kdatetime.h>
#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <kprotocolmanager.h>
#include <kstandarddirs.h>

#include <unistd.h>

time_t g_currentDate;
int g_maxCacheAge;
qint64 g_maxCacheSize;

static const char *appFullName = "org.kde.kio_http_cache_cleaner";
static const char *appName = "kio_http_cache_cleaner";

// !START OF SYNC!
// Keep the following in sync with the cache code in http.cpp

static const char *version = "A\n";

struct CacheFileInfo {
// from http.cpp
    quint8 version[2];
    quint8 compression; // for now fixed to 0
    quint8 reserved;    // for now; also alignment
    static const int useCountOffset = 4;
    qint32 useCount;
    qint64 servedDate;
    qint64 lastModifiedDate;
    qint64 expireDate;
    qint32 bytesCached;
    static const int size = 36;

    QString url;
    QString etag;
    QString mimeType;
    QStringList responseHeaders; // including status response like "HTTP 200 OK"
// end of from http.cpp
    qint64 lastUsedDate;
    QString baseName;
    qint32 sizeOnDisk;
    // we want to delete the least "useful" files and we'll have to sort a list for that...
    bool operator<(const CacheFileInfo &other) const;
};

bool CacheFileInfo::operator<(const CacheFileInfo &other) const
{
    const int thisUseful = useCount / qMax(g_currentDate - lastUsedDate, qint64(1));
    const int otherUseful = other.useCount / qMax(g_currentDate - other.lastUsedDate, qint64(1));
    return thisUseful < otherUseful;
}

bool CacheFileInfoPtrLessThan(const CacheFileInfo *cf1, const CacheFileInfo *cf2)
{
    return *cf1 < *cf2;
}

enum OperationMode {
    CleanCache = 0,
    DeleteCache,
    FileInfo
};

QString dateString(qint64 date)
{
    KDateTime dt;
    dt.setTime_t(date);
    return dt.toString(KDateTime::ISODate);
}

void printInfo(const CacheFileInfo &fi)
{
    QTextStream out(stdout, QIODevice::WriteOnly);
    out << "File " << fi.baseName << " version " << fi.version[0] << fi.version[1];
    out << "\n cached bytes     " << fi.bytesCached << " useCount " << fi.useCount;
    out << "\n servedDate       " << dateString(fi.servedDate);
    out << "\n lastModifiedDate " << dateString(fi.lastModifiedDate);
    out << "\n expireDate       " << dateString(fi.expireDate);
    out << "\n entity tag       " << fi.etag;
    out << "\n encoded URL      " << fi.url;
    out << "\n mimetype         " << fi.mimeType;
    out << "\nResponse headers follow...\n";
    foreach (const QString &h, fi.responseHeaders) {
        out << h << '\n';
    }
}

static bool timeSizeFits(qint64 intTime)
{
    time_t tTime = static_cast<time_t>(intTime);
    qint64 check = static_cast<qint64>(tTime);
    return check == intTime;
}

bool readBinaryHeader(const QByteArray &d, CacheFileInfo *fi)
{
    if (d.size() < CacheFileInfo::size) {
        kDebug(7113) << "readBinaryHeader(): file too small?";
        return false;
    }
    QDataStream stream(d);
    stream.setVersion(QDataStream::Qt_4_5);

    stream >> fi->version[0];
    stream >> fi->version[1];
    if (fi->version[0] != version[0] || fi->version[1] != version[1]) {
        kDebug(7113) << "readBinaryHeader(): wrong magic bytes";
        return false;
    }
    stream >> fi->compression;
    stream >> fi->reserved;

    stream >> fi->useCount;

    stream >> fi->servedDate;
    stream >> fi->lastModifiedDate;
    stream >> fi->expireDate;
    bool timeSizeOk = timeSizeFits(fi->servedDate) && timeSizeFits(fi->lastModifiedDate) &&
                      timeSizeFits(fi->expireDate);

    stream >> fi->bytesCached;
    return timeSizeOk;
}

static bool readLineChecked(QIODevice *dev, QByteArray *line)
{
    *line = dev->readLine(8192);
    // if nothing read or the line didn't fit into 8192 bytes(!)
    if (line->isEmpty() || !line->endsWith('\n')) {
        return false;
    }
    // we don't actually want the newline!
    line->chop(1);
    return true;
}

static QString filenameFromUrl(const QByteArray &url)
{
    QCryptographicHash hash(QCryptographicHash::Sha1);
    hash.addData(url);
    return QString::fromLatin1(hash.result().toHex());
}

static QString filePath(const QString &baseName)
{
    QString cacheDirName = KGlobal::dirs()->saveLocation("cache", "http");
    if (!cacheDirName.endsWith('/')) {
        cacheDirName.append('/');
    }
    return cacheDirName + baseName;
}

bool readTextHeader(QFile *file, CacheFileInfo *fi, OperationMode mode)
{
    bool ok = true;
    QByteArray readBuf;

    ok = ok && readLineChecked(file, &readBuf);
    fi->url = QString::fromLatin1(readBuf);
    if (filenameFromUrl(readBuf) != QFileInfo(*file).baseName()) {
        kDebug(7103) << "You have witnessed a very improbable hash collision!";
        return false;
    }

    // only read the necessary info for cache cleaning. Saves time and (more importantly) memory.
    if (mode != FileInfo) {
        return true;
    }

    ok = ok && readLineChecked(file, &readBuf);
    fi->etag = QString::fromLatin1(readBuf);

    ok = ok && readLineChecked(file, &readBuf);
    fi->mimeType = QString::fromLatin1(readBuf);

    // read as long as no error and no empty line found
    while (true) {
        ok = ok && readLineChecked(file, &readBuf);
        if (ok && !readBuf.isEmpty()) {
            fi->responseHeaders.append(QString::fromLatin1(readBuf));
        } else {
            break;
        }
    }
    return ok; // it may still be false ;)
}

// TODO common include file with http.cpp?
enum CacheCleanerCommand {
    InvalidCommand = 0,
    CreateFileNotificationCommand,
    UpdateFileCommand
};

bool readCacheFile(const QString &baseName, CacheFileInfo *fi, OperationMode mode)
{
    QFile file(filePath(baseName));
    file.open(QIODevice::ReadOnly);
    if (file.openMode() != QIODevice::ReadOnly) {
        return false;
    }
    fi->baseName = baseName;

    QByteArray header = file.read(CacheFileInfo::size);
    // do *not* modify/delete the file if we're in file info mode.
    if (!(readBinaryHeader(header, fi) && readTextHeader(&file, fi, mode)) && mode != FileInfo) {
        kDebug(7113) << "read(Text|Binary)Header() returned false, deleting file" << baseName;
        file.remove();
        return false;
    }
    // get meta-information from the filesystem
    QFileInfo fileInfo(file);
    fi->lastUsedDate = fileInfo.lastModified().toTime_t();
    fi->sizeOnDisk = fileInfo.size();
    return true;
}

CacheCleanerCommand readCommand(const QByteArray &cmd, CacheFileInfo *fi)
{
    readBinaryHeader(cmd, fi);
    QDataStream stream(cmd);
    stream.skipRawData(CacheFileInfo::size);

    quint32 ret;
    stream >> ret;

    QByteArray baseName;
    baseName.resize(40);
    stream.readRawData(baseName.data(), 40);
    Q_ASSERT(stream.atEnd());
    fi->baseName = QString::fromLatin1(baseName);

    Q_ASSERT(ret == CreateFileNotificationCommand || ret == UpdateFileCommand);
    return static_cast<CacheCleanerCommand>(ret);
}

// execute the command; return true if a new file was created, false otherwise.
bool dispatchCommand(const QByteArray &cmd, CacheFileInfo *fi)
{
    Q_ASSERT(cmd.size() == 80);
    CacheCleanerCommand ccc = readCommand(cmd, fi);
    QString fileName = filePath(fi->baseName);
    switch (ccc) {
    case CreateFileNotificationCommand:
        // NOTE: for now we're not keeping cache stats / a list of cache files in memory, so
        //       this command does little. When we do a complete scan of the directory we will
        //       find any new files anyway.
        kDebug(7113) << "CreateNotificationCommand for" << fi->baseName;
        return true;
    case UpdateFileCommand: {
        kDebug(7113) << "UpdateFileCommand for" << fi->baseName;
        QFile file(fileName);
        file.open(QIODevice::ReadWrite);

        CacheFileInfo fiFromDisk;
        QByteArray header = file.read(CacheFileInfo::size);
        if (!readBinaryHeader(header, &fiFromDisk) || fiFromDisk.bytesCached != fi->bytesCached) {
            return false;
        }

        // update the whole header according to the ioslave, except for the use count, to make sure
        // that we actually count up.
        quint32 newUseCount = fiFromDisk.useCount + 1;
        QByteArray newHeader = cmd.mid(0, CacheFileInfo::size);
        {
            QDataStream stream(&newHeader, QIODevice::WriteOnly);
            stream.skipRawData(CacheFileInfo::useCountOffset);
            stream << newUseCount;
        }

        file.seek(0);
        file.write(newHeader);
        return false;
    }
    default:
        kDebug(7113) << "received invalid command";
        break;
    }
    return false;
}

// Keep the above in sync with the cache code in http.cpp
// !END OF SYNC!

// remove files and directories used by earlier versions of the HTTP cache.
static void removeOldFiles()
{
    const char *oldDirs = "0abcdefghijklmnopqrstuvwxyz";
    const int n = strlen(oldDirs);
    QDir cacheRootDir(filePath(QString()));
    for (int i = 0; i < n; i++) {
        QString dirName = QString::fromLatin1(&oldDirs[i], 1);
        // delete files in directory...
        foreach (const QString &baseName, QDir(filePath(dirName)).entryList()) {
            QFile::remove(filePath(dirName + '/' + baseName));
        }
        // delete the (now hopefully empty!) directory itself
        cacheRootDir.rmdir(dirName);
    }
    QFile::remove(filePath(QLatin1String("cleaned")));
}

class CacheCleaner
{
public:
    CacheCleaner(const QDir &cacheDir)
     : m_totalSizeOnDisk(0)
    {
        kDebug(7113);
        m_fileNameList = cacheDir.entryList();
    }

    // Delete some of the files that need to be deleted. Return true when done, false otherwise.
    // This makes interleaved cleaning / serving ioslaves possible.
    bool processSlice()
    {
        QTime t;
        t.start();
        // phase one: gather information about cache files
        if (!m_fileNameList.isEmpty()) {
            while (t.elapsed() < 100 && !m_fileNameList.isEmpty()) {
                QString baseName = m_fileNameList.takeFirst();
                // check if the filename is of the 40 letters, 0...f type
                if (baseName.length() < 40) {
                    continue;
                }
                bool nameOk = true;
                for (int i = 0; i < 40 && nameOk; i++) {
                    QChar c = baseName[i];
                    nameOk = (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f');
                }
                if (!nameOk) {
                    continue;
                }
                if (baseName.length() > 40) {
                    if (g_currentDate - QFileInfo(filePath(baseName)).lastModified().toTime_t() > 15*60) {
                        // it looks like a temporary file that hasn't been touched in > 15 minutes...
                        QFile::remove(filePath(baseName));
                    }
                    // the temporary file might still be written to, leave it alone
                    continue;
                }

                CacheFileInfo *fi = new CacheFileInfo();
                if (readCacheFile(baseName, fi, CleanCache)) {
                    m_fiList.append(fi);
                    m_totalSizeOnDisk += fi->sizeOnDisk;
                } else {
                    delete fi;
                }
            }
            kDebug(7113) << "total size of cache files is" << m_totalSizeOnDisk;

            if (m_fileNameList.isEmpty()) {
                // final step of phase one
                qSort(m_fiList.begin(), m_fiList.end(), CacheFileInfoPtrLessThan);
            }
            return false;
        }

        // phase two: delete files until cache is under maximum allowed size

        // TODO: delete files larger than allowed for a single file
        while (t.elapsed() < 100) {
            if (m_totalSizeOnDisk <= g_maxCacheSize || m_fiList.isEmpty()) {
                qDeleteAll(m_fiList);
                kDebug(7113) << "total size of cache files after cleaning is" << m_totalSizeOnDisk;
                return true;
            }
            CacheFileInfo *fi = m_fiList.takeFirst();
            QString filename = filePath(fi->baseName);
            if (QFile::remove(filename)) {
                m_totalSizeOnDisk -= fi->sizeOnDisk;
            }
            delete fi;
        }
        return false;
    }

private:
    QStringList m_fileNameList;
    QList<CacheFileInfo *> m_fiList;
    qint64 m_totalSizeOnDisk;
};


extern "C" KDE_EXPORT int kdemain(int argc, char **argv)
{
    KCmdLineArgs::init(argc, argv, appName, "kdelibs4",
                       ki18n("KDE HTTP cache maintenance tool"), version,
                       ki18n("KDE HTTP cache maintenance tool"), KCmdLineArgs::CmdLineArgNone);

    KCmdLineOptions options;
    options.add("clear-all", ki18n("Empty the cache"));
    options.add("file-info <filename>", ki18n("Display information about cache file"));

    KCmdLineArgs::addCmdLineOptions( options );
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    KComponentData componentData(appName);

    // we need a QCoreApplication so QCoreApplication::processEvents() works as intended
    QCoreApplication app(argc, argv);

    OperationMode mode = CleanCache;
    if (args->isSet("clear-all")) {
        mode = DeleteCache;
    } else if (args->isSet("file-info")) {
        mode = FileInfo;
    }

    // file info mode: no scanning of directories, just output info and exit.
    if (mode == FileInfo) {
        CacheFileInfo fi;
        if (!readCacheFile(args->getOption("file-info"), &fi, mode)) {
            return 1;
        }
        printInfo(fi);
        return 0;
    }

    // make sure we're the only running instance of the cleaner service
    if (mode == CleanCache) {
        if (!QDBusConnection::sessionBus().isConnected()) {
            QDBusError error(QDBusConnection::sessionBus().lastError());
            fprintf(stderr, "%s: Could not connect to D-Bus! (%s: %s)\n", appName,
                    qPrintable(error.name()), qPrintable(error.message()));
            return 1;
        }

        if (!QDBusConnection::sessionBus().registerService(appFullName)) {
            fprintf(stderr, "%s: Already running!\n", appName);
            return 0;
        }
    }


    g_currentDate = time(0);
    g_maxCacheAge = KProtocolManager::maxCacheAge();
    g_maxCacheSize = mode == DeleteCache ? -1 : KProtocolManager::maxCacheSize() * 1024;

    QString cacheDirName = KGlobal::dirs()->saveLocation("cache", "http");
    QDir cacheDir(cacheDirName);
    if (!cacheDir.exists()) {
        fprintf(stderr, "%s: '%s' does not exist.\n", appName, qPrintable(cacheDirName));
        return 0;
    }

    removeOldFiles();

    if (mode == DeleteCache) {
        QTime t;
        t.start();
        cacheDir.refresh();
        qDebug() << "time to refresh the cacheDir QDir:" << t.elapsed();
        CacheCleaner cleaner(cacheDir);
        while (cleaner.processSlice()) { }
        return 0;
    }

    QLocalServer lServer;
    QString socketFileName = KStandardDirs::locateLocal("socket", "kio_http_cache_cleaner");
    // we need to create the file by opening the socket, otherwise it won't work
    QFile::remove(socketFileName);
    lServer.listen(socketFileName);
    QList<QLocalSocket *> sockets;
    int newFilesCounter = 1000000;  // force cleaner run on startup

    CacheCleaner *cleaner = 0;
    while (true) {
        g_currentDate = time(0);
        if (cleaner) {
            QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
        } else {
            // We will not immediately know when a socket was disconnected. Causes:
            // - WaitForMoreEvents does not make processEvents() return when a socket disconnects
            // - WaitForMoreEvents *and* a timeout is not possible.
            QCoreApplication::processEvents(QEventLoop::WaitForMoreEvents);
        }
        if (!lServer.isListening()) {
            return 1;
        }
        lServer.waitForNewConnection(1);

        while (QLocalSocket *sock = lServer.nextPendingConnection()) {
            sock->waitForConnected();
            sockets.append(sock);
        }

        for (int i = 0; i < sockets.size(); i++) {
            QLocalSocket *sock = sockets[i];
            if (sock->state() != QLocalSocket::ConnectedState) {
                sock->waitForDisconnected();
                delete sock;
                sockets.removeAll(sock);
                i--;
                continue;
            }
            sock->waitForReadyRead(0);
            while (true) {
                QByteArray recv = sock->read(80);
                if (recv.isEmpty()) {
                    break;
                }
                Q_ASSERT(recv.size() == 80);
                //### not keeping the information, for now...
                CacheFileInfo fi;
                if (dispatchCommand(recv, &fi)) {
                    newFilesCounter++;
                }
            }
        }
        // TODO it makes more sense to keep track of cache size, which we can actually do

        // interleave cleaning with serving ioslaves to reduce "garbage collection pauses"
        if (cleaner) {
            if (cleaner->processSlice()) {
                // that was the last slice, done
                delete cleaner;
                cleaner = 0;
            }
        } else if (newFilesCounter > 30) {
            cacheDir.refresh();
            cleaner = new CacheCleaner(cacheDir);
            newFilesCounter = 0;
        }
    }
    return 0;
}
