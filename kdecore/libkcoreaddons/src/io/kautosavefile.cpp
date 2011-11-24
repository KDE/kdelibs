/*  This file is part of the KDE libraries
    Copyright (c) 2006 Jacob R Rideout <kde@jacobrideout.net>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
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

#include "kautosavefile.h"

#include <stdio.h> // for FILENAME_MAX

#include <QtCore/QLatin1Char>
#include <QtCore/QCoreApplication>
#include <QtCore/QDir>
#include "klockfile.h"
#include "krandom.h"
#include "qstandardpaths.h"

class KAutoSaveFilePrivate
{
public:
    KAutoSaveFilePrivate()
            : lock(0),
              managedFileNameChanged(false)
    {}

    QString tempFileName();
    KUrl managedFile;
    KLockFile *lock;
    static const int padding;
    bool managedFileNameChanged;
};

const int KAutoSaveFilePrivate::padding = 8;

QStringList findAllStales()
{
    const QString appName = QCoreApplication::instance()->applicationName();
    const QStringList dirs = QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation);
    QStringList files;

    Q_FOREACH(const QString dir, dirs) {
        const QString appDir = dir + QChar::fromLatin1('/') + appName;
        if (QDir(appDir).exists()) {
            files = files << QDir(appDir).entryList(QStringList() << QString::fromLatin1("*"));
        }
    }
    return files;
}

QString KAutoSaveFilePrivate::tempFileName()
{
    static const int maxNameLength = FILENAME_MAX;

    // Note: we drop any query string and user/pass info
    QString protocol(managedFile.protocol());
    QString path(managedFile.directory());
    QString name(managedFile.fileName());

    // Remove any part of the path to the right if it is longer than the max file size and
    // ensure that the max filesize takes into account the other parts of the tempFileName
    // Subtract 1 for the _ char, 3 for the padding sepperator, 5 is for the .lock
    path = path.left(maxNameLength - padding - name.size() - protocol.size() - 9);

    QString junk = KRandom::randomString(padding);
    // tempName = fileName + junk.trunicated + protocol + _ + path.trunicated + junk
    // This is done so that the separation between the filename and path can be determined
    name += junk.right(3) + protocol + QLatin1Char('_');
    name += path + junk;

    return QString::fromLatin1(KUrl::toPercentEncoding(name).data());
}

KAutoSaveFile::KAutoSaveFile(const KUrl &filename, QObject *parent)
        : QFile(parent),
          d(new KAutoSaveFilePrivate)
{
    setManagedFile(filename);
}

KAutoSaveFile::KAutoSaveFile(QObject *parent)
        : QFile(parent),
          d(new KAutoSaveFilePrivate)
{

}

KAutoSaveFile::~KAutoSaveFile()
{
    releaseLock();
    delete d->lock;
    delete d;
}

KUrl KAutoSaveFile::managedFile() const
{
    return d->managedFile;
}

void KAutoSaveFile::setManagedFile(const KUrl &filename)
{
    releaseLock();

    d->managedFile = filename;
    d->managedFileNameChanged = true;
}

void KAutoSaveFile::releaseLock()
{
    if (d->lock && d->lock->isLocked()) {
        delete d->lock;
	d->lock = NULL;
        if (!fileName().isEmpty()) {
            remove();
        }
    }
}

bool KAutoSaveFile::open(OpenMode openmode)
{
    if (d->managedFile == KUrl()) {
        return false;
    }

    QString tempFile;
    if (d->managedFileNameChanged) {
        tempFile = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) +
            QChar::fromLatin1('/') +  d->tempFileName();
    } else {
        tempFile = fileName();
    }

    d->managedFileNameChanged = false;

    setFileName(tempFile);

    if (QFile::open(openmode)) {

        d->lock = new KLockFile(tempFile + QString::fromLatin1(".lock"));
        if (d->lock->isLocked()) {
            close();
            return false;
        }

        d->lock->setStaleTime(60); // HARDCODE, 1 minute

        if (d->lock->lock(KLockFile::ForceFlag|KLockFile::NoBlockFlag) == KLockFile::LockOK) {
            return true;
        } else {
            close();
        }
    }

    return false;
}

QList<KAutoSaveFile *> KAutoSaveFile::staleFiles(const KUrl &filename, const QString &applicationName)
{

    QString appName(applicationName);
    if (appName.isEmpty()) {
        appName = QCoreApplication::instance()->applicationName();
    }

    QString url = filename.fileName();

    if (url.isEmpty()) {
        return QList<KAutoSaveFile *>();
    }

    // get stale files
    const QStringList files = findAllStales();

    QList<KAutoSaveFile *> list;
    KAutoSaveFile * asFile;

    // contruct a KAutoSaveFile for each stale file
    Q_FOREACH(const QString &file, files) {
        if (file.endsWith(QLatin1String(".lock")))
            continue;
        // sets managedFile
        asFile = new KAutoSaveFile(filename);
        asFile->setFileName(file);
        // flags the name, so it isn't regenerated
        asFile->d->managedFileNameChanged = false;
        list.append(asFile);
    }

    return list;
}

QList<KAutoSaveFile *> KAutoSaveFile::allStaleFiles(const QString &applicationName)
{

    QString appName(applicationName);
    if (appName.isEmpty()) {
        appName = QCoreApplication::instance()->applicationName();
    }

    // get stale files
    const QStringList files = findAllStales();

    QList<KAutoSaveFile *> list;

    // contruct a KAutoSaveFile for each stale file
    Q_FOREACH(QString file, files) { // krazy:exclude=foreach (no const& because modified below)
        if (file.endsWith(QLatin1String(".lock")))
            continue;
        const QString sep = file.right(3);
        file.chop(KAutoSaveFilePrivate::padding);

        int sepPos = file.indexOf(sep);
        int pathPos = file.indexOf(QChar::fromLatin1('_'), sepPos);
        KUrl name;
        name.setProtocol(file.mid(sepPos + 3, pathPos - sep.size() - 3));
        name.setPath(KUrl::fromPercentEncoding(file.right(pathPos - 1).toLatin1()));
        name.addPath(KUrl::fromPercentEncoding(file.left(sepPos).toLatin1()));

        // sets managedFile
        KAutoSaveFile* asFile = new KAutoSaveFile(name);
        asFile->setFileName(file);
        // flags the name, so it isn't regenerated
        asFile->d->managedFileNameChanged = false;
        list.append(asFile);
    }

    return list;
}

#include "moc_kautosavefile.cpp"
