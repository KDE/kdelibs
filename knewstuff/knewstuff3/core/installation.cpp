/*
    This file is part of KNewStuff2.
    Copyright (c) 2007 Josef Spillner <spillner@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "installation.h"

#include <QDir>
#include <QFile>

#include <KStandardDirs>
#include <KMimeType>
#include <KArchive>
#include <KZip>
#include <KTar>
#include <KProcess>
#include <KIO/Job>
#include <KRandom>
#include <KShell>
#include <KMessageBox>
#include <KLocalizedString>
#include <KDebug>

#include "knewstuff3/core/security.h"


struct KNS3::InstallationPrivate {
    InstallationPrivate() {
        checksumPolicy = Installation::CheckIfPossible;
        signaturePolicy = Installation::CheckIfPossible;
        scope = Installation::ScopeUser;
        customName = false;
    }

    // applications can set this if they want the installed files/directories to be piped into a shell command
    QString postInstallationCommand;
    // a custom command to run for the uninstall
    QString uninstallCommand;
    // compression policy
    QString uncompression;
    
    // only one of the four below can be set, that will be the target install path/file name
    // FIXME: check this when reading the config and make one path out of it if possible?
    QString standardResourceDirectory;
    QString targetDirectory;
    QString installPath;
    QString absoluteInstallPath;
    
    // policies wether verification needs to be done
    Installation::Policy checksumPolicy;
    Installation::Policy signaturePolicy;
    // scope: install into user or system dirs
    Installation::Scope scope;
    
    // FIXME this throws together a file name from entry name and version - why would anyone want that?
    bool customName;

    QMap<KJob*, Entry> entry_jobs;

};

using namespace KNS3;

Installation::Installation(QObject* parent)
        : QObject(parent), d(new InstallationPrivate)
{
}

Installation::~Installation()
{
    delete d;
}

bool Installation::readConfig(const KConfigGroup& group)
{
    // FIXME: add support for several categories later on
    // FIXME: read out only when actually installing as a performance improvement?
    QString uncompresssetting = group.readEntry("Uncompress", QString("never"));
    // support old value of true as equivalent of always
    if (uncompresssetting == "true") {
        uncompresssetting = "always";
    }
    if (uncompresssetting != "always" && uncompresssetting != "archive" && uncompresssetting != "never") {
        kError() << "invalid Uncompress setting chosen, must be one of: always, archive, or never" << endl;
        return false;
    }
    d->uncompression = uncompresssetting;
    d->postInstallationCommand = group.readEntry("InstallationCommand", QString());
    d->uninstallCommand = group.readEntry("UninstallCommand", QString());
    d->standardResourceDirectory = group.readEntry("StandardResource", QString());
    d->targetDirectory = group.readEntry("TargetDir", QString());
    d->installPath = group.readEntry("InstallPath", QString());
    d->absoluteInstallPath = group.readEntry("AbsoluteInstallPath", QString());
    d->customName = group.readEntry("CustomName", false);

    if (d->standardResourceDirectory.isEmpty() &&
            d->targetDirectory.isEmpty() &&
            d->installPath.isEmpty() &&
            d->absoluteInstallPath.isEmpty()) {
        kError() << "No installation target set";
        return false;
    }
    
    QString checksumpolicy = group.readEntry("ChecksumPolicy", QString());
    if (!checksumpolicy.isEmpty()) {
        if (checksumpolicy == "never")
            d->checksumPolicy = Installation::CheckNever;
        else if (checksumpolicy == "ifpossible")
            d->checksumPolicy = Installation::CheckIfPossible;
        else if (checksumpolicy == "always")
            d->checksumPolicy = Installation::CheckAlways;
        else {
            kError() << "The checksum policy '" + checksumpolicy + "' is unknown." << endl;
            return false;
        }
    }
    
    QString signaturepolicy = group.readEntry("SignaturePolicy", QString());
    if (!signaturepolicy.isEmpty()) {
        if (signaturepolicy == "never")
            d->signaturePolicy = Installation::CheckNever;
        else if (signaturepolicy == "ifpossible")
            d->signaturePolicy = Installation::CheckIfPossible;
        else if (signaturepolicy == "always")
            d->signaturePolicy = Installation::CheckAlways;
        else {
            kError() << "The signature policy '" + signaturepolicy + "' is unknown." << endl;
            return false;
        }
    }
    
    QString scope = group.readEntry("Scope", QString());
    if (!scope.isEmpty()) {
        if (scope == "user")
            d->scope = Installation::ScopeUser;
        else if (scope == "system")
            d->scope = Installation::ScopeSystem;
        else {
            kError() << "The scope '" + scope + "' is unknown." << endl;
            return false;
        }
        
        if (d->scope == Installation::ScopeSystem) {
            if (!d->installPath.isEmpty()) {
                kError() << "System installation cannot be mixed with InstallPath." << endl;
                return false;
            }
        }
    }
    return true;
}

bool Installation::isRemote() const
{
    if (!d->installPath.isEmpty()) return false;
    if (!d->targetDirectory.isEmpty()) return false;
    if (!d->absoluteInstallPath.isEmpty()) return false;
    if (!d->standardResourceDirectory.isEmpty()) return false;
    return true;
}

void Installation::install(Entry entry)
{
    downloadPayload(entry);
}

void Installation::downloadPayload(Entry entry)
{
    if(!entry.isValid()) {
        emit signalPayloadFailed(entry);
        return;
    }
    KUrl source = KUrl(entry.payload().representation());

    if (!source.isValid()) {
        kError() << "The entry doesn't have a payload." << endl;
        emit signalPayloadFailed(entry);
        return;
    }

// FIXME no clue what this is supposed to do
    if (isRemote()) {
        // Remote resource
        //kDebug() << "Relaying remote payload '" << source << "'";
        install(entry, source.pathOrUrl());
        emit signalPayloadLoaded(source);
        // FIXME: we still need registration for eventual deletion
        return;
    }

    KUrl destination = QString(KGlobal::dirs()->saveLocation("tmp") + KRandom::randomString(10));
    kDebug() << "Downloading payload '" << source << "' to '" << destination << "'";

    // FIXME: check for validity
    KIO::FileCopyJob *job = KIO::file_copy(source, destination, -1, KIO::Overwrite | KIO::HideProgressInfo);
    connect(job,
            SIGNAL(result(KJob*)),
            SLOT(slotPayloadResult(KJob*)));
    connect(job,
            SIGNAL(percent(KJob*, unsigned long)),
            SLOT(slotProgress(KJob*, unsigned long)));

    d->entry_jobs[job] = entry;
}


void Installation::slotPayloadResult(KJob *job)
{
    // for some reason this slot is getting called 3 times on one job error
    if (d->entry_jobs.contains(job)) {
        Entry entry = d->entry_jobs[job];
        d->entry_jobs.remove(job);

        if (job->error()) {
            kError() << "Cannot load payload file." << endl;
            kError() << job->errorString() << endl;

            emit signalPayloadFailed(entry);
        } else {
            KIO::FileCopyJob *fcjob = static_cast<KIO::FileCopyJob*>(job);

            install(entry, fcjob->destUrl().pathOrUrl());

            emit signalPayloadLoaded(fcjob->destUrl());
        }
    }
}


void Installation::install(Entry entry, const QString& downloadedFile)
{
    kDebug() << "Install: " << entry.name().representation() << " from " << downloadedFile;
    
    if (entry.payload().isEmpty()) {
        kDebug() << "No payload associated with: " << entry.name().representation();
        return;
        // attica needs to get the downloadlink :(
        // fix attica?
        //Provider* provider = d->provider_index.value(entry.providerId());
    }


    // FIXME: this is only so exposing the KUrl suffices for downloaded entries


    // FIXME: first of all, do the security stuff here
    // this means check sum comparison and signature verification
    // signature verification might take a long time - make async?!
    /*
    if (checksumPolicy() != Installation::CheckNever) {
        if (entry.checksum().isEmpty()) {
            if (checksumPolicy() == Installation::CheckIfPossible) {
                //kDebug() << "Skip checksum verification";
            } else {
                kError() << "Checksum verification not possible" << endl;
                return false;
            }
        } else {
            //kDebug() << "Verify checksum...";
        }
    }
    if (signaturePolicy() != Installation::CheckNever) {
        if (entry.signature().isEmpty()) {
            if (signaturePolicy() == Installation::CheckIfPossible) {
                //kDebug() << "Skip signature verification";
            } else {
                kError() << "Signature verification not possible" << endl;
                return false;
            }
        } else {
            //kDebug() << "Verify signature...";
        }
    }
    */
    

    QString targetPath = targetInstallationPath(downloadedFile);
    QStringList installedFiles = installDownloadedFileAndUncompress(entry, downloadedFile, targetPath);

    if (installedFiles.isEmpty()) {
        emit signalInstallationFailed(entry);
        return;
    }
    
    entry.setInstalledFiles(installedFiles);

    if (!d->postInstallationCommand.isEmpty()) {
        QString target;
        if (installedFiles.size() == 1) {
            runPostInstallationCommand(installedFiles.first());
        } else {
            runPostInstallationCommand(targetPath);
        }
    }

    // ==== FIXME: security code below must go above, when async handling is complete ====

    // FIXME: security object lifecycle - it is a singleton!
    Security *sec = Security::ref();

    connect(sec,
            SIGNAL(validityResult(int)),
            SLOT(slotInstallationVerification(int)));

    // FIXME: change to accept filename + signature
    sec->checkValidity(QString());

    entry.setStatus(Entry::Installed);
    emit signalInstallationFinished(entry);
}

QString Installation::targetInstallationPath(const QString& payloadfile)
{
    QString installpath(payloadfile);
    QString installdir;
    
    if (!isRemote()) {
        // installdir is the target directory

        // installpath also contains the file name if it's a single file, otherwise equal to installdir
        int pathcounter = 0;
        if (!d->standardResourceDirectory.isEmpty()) {
            if (d->scope == Installation::ScopeUser) {
                installdir = KStandardDirs::locateLocal(d->standardResourceDirectory.toUtf8(), "/");
            } else { // system scope
                installdir = KStandardDirs::installPath(d->standardResourceDirectory.toUtf8());
            }
            pathcounter++;
        }
        if (!d->targetDirectory.isEmpty()) {
            if (d->scope == Installation::ScopeUser) {
                installdir = KStandardDirs::locateLocal("data", d->targetDirectory + '/');
            } else { // system scope
                installdir = KStandardDirs::installPath("data") + d->targetDirectory + '/';
            }
            pathcounter++;
        }
        if (!d->installPath.isEmpty()) {
#if defined(Q_WS_WIN)
            WCHAR wPath[MAX_PATH+1];
            if ( SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT, wPath) == S_OK) {
                installdir = QString::fromUtf16((const ushort *) wPath) + QLatin1Char('/') + installPath() + QLatin1Char('/');
            } else {
                installdir =  QDir::home().path() + QLatin1Char('/') + d->m_installpath + QLatin1Char('/');
            }
#else
            installdir = QDir::home().path() + '/' + d->installPath + '/';
#endif
            pathcounter++;
        }
        if (!d->absoluteInstallPath.isEmpty()) {
            installdir = d->absoluteInstallPath + '/';
            pathcounter++;
        }
        if (pathcounter != 1) {
            kError() << "Wrong number of installation directories given." << endl;
            return QString();
        }

        kDebug() << "installdir: " << installdir;

    }
    
    return installdir;
}

QStringList Installation::installDownloadedFileAndUncompress(Entry entry, const QString& payloadfile, const QString installdir)
{
    QString installpath(payloadfile);
    // Collect all files that were installed
    QStringList installedFiles;

    if (!isRemote()) {
        bool isarchive = true;
        
        // respect the uncompress flag in the knsrc
        if (d->uncompression == "always" || d->uncompression == "archive") {
            // this is weird but a decompression is not a single name, so take the path instead
            installpath = installdir;
            KMimeType::Ptr mimeType = KMimeType::findByPath(payloadfile);
            //kDebug() << "Postinstallation: uncompress the file";

            // FIXME: check for overwriting, malicious archive entries (../foo) etc.
            // FIXME: KArchive should provide "safe mode" for this!
            KArchive *archive = 0;

            if (mimeType->name() == "application/zip") {
                archive = new KZip(payloadfile);
            } else if (mimeType->name() == "application/tar"
                       || mimeType->name() == "application/x-gzip"
                       || mimeType->name() == "application/x-bzip"
                       || mimeType->name() == "application/x-lzma"
                       || mimeType->name() == "application/x-xz") {
                archive = new KTar(payloadfile);
            } else {
                delete archive;
                kError() << "Could not determine type of archive file '" << payloadfile << "'";
                if (d->uncompression == "always") {
                    return QStringList();
                }
                isarchive = false;
            }

            if (isarchive) {
                bool success = archive->open(QIODevice::ReadOnly);
                if (!success) {
                    kError() << "Cannot open archive file '" << payloadfile << "'";
                    if (d->uncompression == "always") {
                        return QStringList();
                    }
                    // otherwise, just copy the file
                    isarchive = false;
                }

                if (isarchive) {
                    const KArchiveDirectory *dir = archive->directory();
                    dir->copyTo(installdir);

                    installedFiles << archiveEntries(installdir, dir);
                    installedFiles << installdir + '/';

                    archive->close();
                    QFile::remove(payloadfile);
                    delete archive;
                }
            }
        }

        kDebug() << "isarchive: " << isarchive;

        if (d->uncompression == "never" || (d->uncompression == "archive" && !isarchive)) {
            // no decompress but move to target

            /// @todo when using KIO::get the http header can be accessed and it contains a real file name.
            // FIXME: make naming convention configurable through *.knsrc? e.g. for kde-look.org image names
            KUrl source = KUrl(entry.payload().representation());
            kDebug() << "installing non-archive from " << source.url();
            QString installfile;
            QString ext = source.fileName().section('.', -1);
            if (d->customName) {
                installfile = entry.name().representation();
                installfile += '-' + entry.version();
                if (!ext.isEmpty()) installfile += '.' + ext;
            } else {
                installfile = source.fileName();
            }
            installpath = installdir + '/' + installfile;

            //kDebug() << "Install to file " << installpath;
            // FIXME: copy goes here (including overwrite checking)
            // FIXME: what must be done now is to update the cache *again*
            //        in order to set the new payload filename (on root tag only)
            //        - this might or might not need to take uncompression into account
            // FIXME: for updates, we might need to force an overwrite (that is, deleting before)
            QFile file(payloadfile);
            bool success = true;
            bool update = (entry.status() == Entry::Updateable);
            
            if (QFile::exists(installpath)) {
                if (!update) {
                    if (KMessageBox::warningContinueCancel(0, i18n("Overwrite existing file?") + "\n'" + installpath + '\'', i18n("Download File:")) == KMessageBox::Cancel) {
                        return QStringList();
                    }
                }
                success = QFile::remove(installpath);
            }
            if (success) {
                success = file.rename(QUrl(installpath).toLocalFile());

                kDebug() << "move: " << file.fileName() << " to " << installpath
                    << QUrl(installpath).toLocalFile() << QUrl(installpath).toString() << QUrl(installpath).isValid();
            }
            if (!success) {
                kError() << "Cannot move file '" << payloadfile << "' to destination '"  << installpath << "'";
                return QStringList();
            }
            installedFiles << installpath;
            installedFiles << installdir + '/';
        }
    }
    return installedFiles;
}

void Installation::runPostInstallationCommand(const QString& installPath)
{
    KProcess process;
    QString command(d->postInstallationCommand);
    QString fileArg(KShell::quoteArg(installPath));
    command.replace("%f", fileArg);

    //kDebug() << "Postinstallation: execute command";
    //kDebug() << "Command is: " << command;

    process.setShellCommand(command);
    int exitcode = process.execute();

    if (exitcode) {
        kError() << "Command failed" << endl;
    }
}


void Installation::uninstall(Entry entry)
{
    entry.setStatus(Entry::Deleted);

    if (!d->uninstallCommand.isEmpty()) {
        KProcess process;
        foreach (const QString& file, entry.installedFiles()) {
            QFileInfo info(file);
            if (info.isFile()) {
                QString fileArg(KShell::quoteArg(file));
                QString command(d->uninstallCommand);
                command.replace("%f", fileArg);

                process.setShellCommand(command);
                int exitcode = process.execute();

                if (exitcode) {
                    kError() << "Command failed" << endl;
                } else {
                    //kDebug() << "Command executed successfully";
                }
            }
        }
    }

    foreach(const QString &file, entry.installedFiles()) {
        if (file.endsWith('/')) {
            QDir dir;
            bool worked = dir.rmdir(file);
            if (!worked) {
                // Maybe directory contains user created files, ignore it
                continue;
            }
        } else {
            if (QFile::exists(file)) {
                bool worked = QFile::remove(file);
                if (!worked) {
                    kWarning() << "unable to delete file " << file;
                    return;
                }
            } else {
                kWarning() << "unable to delete file " << file << ". file does not exist.";
            }
        }
    }
    entry.setUnInstalledFiles(entry.installedFiles());
    entry.setInstalledFiles(QStringList());

    emit signalUninstallFinished(entry);
}


void Installation::slotInstallationVerification(int result)
{
    //kDebug() << "SECURITY result " << result;

    //FIXME do something here ??? and get the right entry again
    Entry entry;
    
    if (result & Security::SIGNED_OK)
        emit signalInstallationFinished(entry);
    else
        emit signalInstallationFailed(entry);
}


QStringList Installation::archiveEntries(const QString& path, const KArchiveDirectory * dir)
{
    QStringList files;
    foreach(const QString &entry, dir->entries()) {
        QString childPath = path + '/' + entry;
        if (dir->entry(entry)->isFile()) {
            files << childPath;
        }

        if (dir->entry(entry)->isDirectory()) {
            const KArchiveDirectory* childDir = static_cast<const KArchiveDirectory*>(dir->entry(entry));
            files << archiveEntries(childPath, childDir);
            files << childPath + '/';
        }
    }
    return files;
}


#include "installation.moc"
