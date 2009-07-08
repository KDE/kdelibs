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
bool g_cacheHasChanged;

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
    const int thisUseful = useCount / (g_currentDate - lastUsedDate);
    const int otherUseful = other.useCount / (g_currentDate - other.lastUsedDate);
    return thisUseful < otherUseful;
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
    qDebug() << "File" << fi.baseName << "version" << fi.version[0] << fi.version[1];
    qDebug() << " cached bytes    " << fi.bytesCached << "useCount" << fi.useCount;
    qDebug() << " servedDate      " << dateString(fi.servedDate);
    qDebug() << " lastModifiedDate" << dateString(fi.lastModifiedDate);
    qDebug() << " expireDate      " << dateString(fi.expireDate);
    qDebug() << " entity tag      " << fi.etag;
    qDebug() << " encoded URL     " << fi.url;
    qDebug() << " mimetype        " << fi.mimeType;
    qDebug() << "Response headers follow...";
    foreach (const QString &h, fi.responseHeaders) {
        qDebug() << h;
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
        qDebug() << "readBinaryHeader(): file too small?";
        return false;
    }
    QDataStream stream(d);
    stream.setVersion(QDataStream::Qt_4_5);

    stream >> fi->version[0];
    stream >> fi->version[1];
    if (fi->version[0] != version[0] || fi->version[1] != version[1]) {
        qDebug() << "readBinaryHeader(): wrong magic bytes";
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
    if (!readBinaryHeader(header, fi) && mode != FileInfo) {
        qDebug() << "readBinaryHeader() returned false, deleting file" << baseName;
        file.remove();
        return false;
    }
    readTextHeader(&file, fi, mode);
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

void dispatchCommand(const QByteArray &cmd, CacheFileInfo *fi)
{
    Q_ASSERT(cmd.size() == 80);
    CacheCleanerCommand ccc = readCommand(cmd, fi);
    QString fileName = filePath(fi->baseName);
    switch (ccc) {
    case CreateFileNotificationCommand:
        // NOTE: for now we're not keeping cache stats / a list of cache files in memory, so
        //       this command does little. When we do a complete scan of the directory we will
        //       find any new files anyway.
        qDebug() << "CreateNotificationCommand for" << fi->baseName;
        g_cacheHasChanged = true;
        break;
    case UpdateFileCommand: {
        qDebug() << "UpdateFileCommand for" << fi->baseName;
        QFile file(fileName);
        file.open(QIODevice::ReadWrite);

        CacheFileInfo fiFromDisk;
        QByteArray header = file.read(CacheFileInfo::size);
        readBinaryHeader(header, &fiFromDisk);
        Q_ASSERT(fiFromDisk.bytesCached == fi->bytesCached);

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
        g_cacheHasChanged = true;
        break;
    }
    default:
        qDebug() << "received invalid command";
        break;
    }
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

static void cleanCache(const QDir &cacheDir)
{
    qDebug() << "cleanCache() running.";
    QList<CacheFileInfo *> fiList;
    qint64 totalSizeOnDisk = 0;
    foreach (const QString &baseName, cacheDir.entryList()) {
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
            fiList.append(fi);
            totalSizeOnDisk += fi->sizeOnDisk;
        } else {
            delete fi;
        }
    }

    qDebug() << "total size of cache files is" << totalSizeOnDisk;

    // operator< implements the usefulness estimate, hence we sort by usefulness
    qSort(fiList.begin(), fiList.end());

    // TODO: delete files larger than allowed for a single file, debug output
    for (int i = 0; totalSizeOnDisk > g_maxCacheSize && i < fiList.count(); i++) {
        CacheFileInfo *fi = fiList[i];
        QString filename = filePath(fi->baseName);
        if (QFile::remove(filename)) {
            totalSizeOnDisk -= fi->sizeOnDisk;
        }
    }

    qDebug() << "total size of cache files after cleaning is" << totalSizeOnDisk;
    qDeleteAll(fiList);
    g_cacheHasChanged = false;
}

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
    g_cacheHasChanged = true;


    QString cacheDirName = KGlobal::dirs()->saveLocation("cache", "http");
    QDir cacheDir(cacheDirName);
    if (!cacheDir.exists()) {
        fprintf(stderr, "%s: '%s' does not exist.\n", appName, qPrintable(cacheDirName));
        return 0;
    }

    removeOldFiles();

    QLocalServer lServer;
    QString socketFileName = cacheDirName + '/' + ".cleaner_socket";
    // we need to create the file by opening the socket, otherwise it won't work
    QFile::remove(socketFileName);
    lServer.listen(socketFileName);
    QList<QLocalSocket *> sockets;
    int updateCounter = 1000000;    // HACK in more than one way
    while (true) {
        g_currentDate = time(0);
        // Meh, processEvents() with timeout does not work with WaitForMoreEvents, so we either have to
        // spin (hard) or poll, or just learn quite late when a http process has disconnected :/
        // (a disconnection is apparently not an 'event' that wakes up the event loop)
        QCoreApplication::processEvents(QEventLoop::WaitForMoreEvents);
        if (!lServer.isListening()) {
            return 1;
        }
        lServer.waitForNewConnection(1);

        while (QLocalSocket *sock = lServer.nextPendingConnection()) {
            qDebug() << "got new connection, openMode() is" << sock->openMode();
            sock->waitForConnected();
            sockets.append(sock);
        }

        qDebug() << sockets.size() << "sockets in waiting list.";
        for (int i = 0; i < sockets.size(); i++) {
            QLocalSocket *sock = sockets[i];
            // qDebug() << "  this socket's state is:" << sock->state();
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
                qDebug() << " received number of bytes:" << recv.size();
                //### not keeping the information...
                CacheFileInfo fi;
                dispatchCommand(recv, &fi);
                updateCounter++;
            }
        }
        if ((updateCounter % 10) == 0) {
            qDebug() << "updateCounter =" << updateCounter;
        }
        // TODO it makes more sense to keep track of cache size, which we can actually do
        if (updateCounter > 50) {
            cacheDir.refresh();
            cleanCache(cacheDir);
            updateCounter = 0;
        }
    }
    return 0;
}
