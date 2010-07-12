/*
    This file is part of KNewStuff2.
    Copyright (c) 2007 Josef Spillner <spillner@kde.org>
    Copyright (C) 2009 Frederik Gladhorn <gladhorn@kde.org>

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

#include "kstandarddirs.h"
#include "kmimetype.h"
#include "karchive.h"
#include "kzip.h"
#include "ktar.h"
#include "kprocess.h"
#include "kio/job.h"
#include "krandom.h"
#include "kshell.h"
#include "kmessagebox.h" // TODO get rid of message box
#include "ktoolinvocation.h" // TODO remove, this was only for my playing round
#include "klocalizedstring.h"
#include "kdebug.h"

#include "core/security.h"
#ifdef Q_OS_WIN
#include <windows.h>
#include <shlobj.h>
#endif

using namespace KNS3;

Installation::Installation(QObject* parent)
    : QObject(parent)
    , checksumPolicy(Installation::CheckIfPossible)
    , signaturePolicy(Installation::CheckIfPossible)
    , scope(Installation::ScopeUser)
    , customName(false)
    , acceptHtml(false)
{
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
    uncompression = uncompresssetting;
    postInstallationCommand = group.readEntry("InstallationCommand", QString());
    uninstallCommand = group.readEntry("UninstallCommand", QString());
    standardResourceDirectory = group.readEntry("StandardResource", QString());
    targetDirectory = group.readEntry("TargetDir", QString());
    xdgTargetDirectory = group.readEntry("XdgTargetDir", QString());
    installPath = group.readEntry("InstallPath", QString());
    absoluteInstallPath = group.readEntry("AbsoluteInstallPath", QString());
    customName = group.readEntry("CustomName", false);
    acceptHtml = group.readEntry("AcceptHtmlDownloads", false);

    if (standardResourceDirectory.isEmpty() &&
            targetDirectory.isEmpty() &&
            xdgTargetDirectory.isEmpty() &&
            installPath.isEmpty() &&
            absoluteInstallPath.isEmpty()) {
        kError() << "No installation target set";
        return false;
    }

    QString checksumpolicy = group.readEntry("ChecksumPolicy", QString());
    if (!checksumpolicy.isEmpty()) {
        if (checksumpolicy == "never")
            checksumPolicy = Installation::CheckNever;
        else if (checksumpolicy == "ifpossible")
            checksumPolicy = Installation::CheckIfPossible;
        else if (checksumpolicy == "always")
            checksumPolicy = Installation::CheckAlways;
        else {
            kError() << "The checksum policy '" + checksumpolicy + "' is unknown." << endl;
            return false;
        }
    }

    QString signaturepolicy = group.readEntry("SignaturePolicy", QString());
    if (!signaturepolicy.isEmpty()) {
        if (signaturepolicy == "never")
            signaturePolicy = Installation::CheckNever;
        else if (signaturepolicy == "ifpossible")
            signaturePolicy = Installation::CheckIfPossible;
        else if (signaturepolicy == "always")
            signaturePolicy = Installation::CheckAlways;
        else {
            kError() << "The signature policy '" + signaturepolicy + "' is unknown." << endl;
            return false;
        }
    }

    QString scopeString = group.readEntry("Scope", QString());
    if (!scopeString.isEmpty()) {
        if (scopeString == "user")
            scope = ScopeUser;
        else if (scopeString == "system")
            scope = ScopeSystem;
        else {
            kError() << "The scope '" + scopeString + "' is unknown." << endl;
            return false;
        }

        if (scope == ScopeSystem) {
            if (!installPath.isEmpty()) {
                kError() << "System installation cannot be mixed with InstallPath." << endl;
                return false;
            }
        }
    }
    return true;
}

bool Installation::isRemote() const
{
    if (!installPath.isEmpty()) return false;
    if (!targetDirectory.isEmpty()) return false;
    if (!xdgTargetDirectory.isEmpty()) return false;
    if (!absoluteInstallPath.isEmpty()) return false;
    if (!standardResourceDirectory.isEmpty()) return false;
    return true;
}

void Installation::install(EntryInternal entry)
{
    downloadPayload(entry);
}

void Installation::downloadPayload(const KNS3::EntryInternal& entry)
{
    if(!entry.isValid()) {
        emit signalInstallationFailed(i18n("Invalid item."));
        return;
    }
    KUrl source = KUrl(entry.payload());

    if (!source.isValid()) {
        kError() << "The entry doesn't have a payload." << endl;
        emit signalInstallationFailed(i18n("Download of item failed: no download URL for \"%1\".", entry.name()));
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

    QString fileName(source.fileName());
    KUrl destination = QString(KGlobal::dirs()->saveLocation("tmp") + KRandom::randomString(10) + '-' + fileName);
    kDebug() << "Downloading payload '" << source << "' to '" << destination << "'";

    // FIXME: check for validity
    KIO::FileCopyJob *job = KIO::file_copy(source, destination, -1, KIO::Overwrite | KIO::HideProgressInfo);
    connect(job,
            SIGNAL(result(KJob*)),
            SLOT(slotPayloadResult(KJob*)));

    entry_jobs[job] = entry;
}


void Installation::slotPayloadResult(KJob *job)
{
    // for some reason this slot is getting called 3 times on one job error
    if (entry_jobs.contains(job)) {
        EntryInternal entry = entry_jobs[job];
        entry_jobs.remove(job);

        if (job->error()) {
            emit signalInstallationFailed(i18n("Download of \"%1\" failed, error: %2", entry.name(), job->errorString()));
        } else {
            KIO::FileCopyJob *fcjob = static_cast<KIO::FileCopyJob*>(job);

            // check if the app likes html files - disabled by default as too many bad links have been submitted to opendesktop.org
            if (!acceptHtml) {
                KMimeType::Ptr mimeType = KMimeType::findByPath(fcjob->destUrl().pathOrUrl());
                if (mimeType->is("text/html") || mimeType->is("application/x-php")) {
                    if (KMessageBox::questionYesNo(0, i18n("The downloaded file is a html file. This indicates a link to a website instead of the actual download. Would you like to open the site with a browser instead?"), i18n("Possibly bad download link"))
                        == KMessageBox::Yes) {
                        KToolInvocation::invokeBrowser(fcjob->srcUrl().url());
                        emit signalInstallationFailed(i18n("Downloaded file was a HTML file. Opened in browser."));
                        entry.setStatus(Entry::Invalid);
                        emit signalEntryChanged(entry);
                        return;
                    }
                }
            }

            install(entry, fcjob->destUrl().pathOrUrl());
            emit signalPayloadLoaded(fcjob->destUrl());
        }
    }
}


void Installation::install(KNS3::EntryInternal entry, const QString& downloadedFile)
{
    kDebug() << "Install: " << entry.name() << " from " << downloadedFile;

    if (entry.payload().isEmpty()) {
        kDebug() << "No payload associated with: " << entry.name();
        return;
    }

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
        if (entry.status() == Entry::Installing) {
            entry.setStatus(Entry::Downloadable);
        } else if (entry.status() == Entry::Updating) {
            entry.setStatus(Entry::Updateable);
        }
        emit signalEntryChanged(entry);
        emit signalInstallationFailed(i18n("Could not install \"%1\": file not found.", entry.name()));
        return;
    }

    entry.setInstalledFiles(installedFiles);

    if (!postInstallationCommand.isEmpty()) {
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

    // update version and release date to the new ones
    if (entry.status() == Entry::Updating) {
        if (!entry.updateVersion().isEmpty()) {
            entry.setVersion(entry.updateVersion());
        }
        if (!entry.updateReleaseDate().isValid()) {
            entry.setReleaseDate(entry.updateReleaseDate());
        }
    }

    entry.setStatus(Entry::Installed);
    emit signalEntryChanged(entry);
    emit signalInstallationFinished();
}

QString Installation::targetInstallationPath(const QString& payloadfile)
{
    QString installpath(payloadfile);
    QString installdir;

    if (!isRemote()) {
        // installdir is the target directory

        // installpath also contains the file name if it's a single file, otherwise equal to installdir
        int pathcounter = 0;
        if (!standardResourceDirectory.isEmpty()) {
            if (scope == ScopeUser) {
                installdir = KStandardDirs::locateLocal(standardResourceDirectory.toUtf8(), "/");
            } else { // system scope
                installdir = KStandardDirs::installPath(standardResourceDirectory.toUtf8());
            }
            pathcounter++;
        }
        if (!targetDirectory.isEmpty()) {
            if (scope == ScopeUser) {
                installdir = KStandardDirs::locateLocal("data", targetDirectory + '/');
            } else { // system scope
                installdir = KStandardDirs::installPath("data") + targetDirectory + '/';
            }
            pathcounter++;
        }
        if (!xdgTargetDirectory.isEmpty()) {
            installdir = KStandardDirs().localxdgdatadir() + '/' + xdgTargetDirectory + '/';
            pathcounter++;
        }
        if (!installPath.isEmpty()) {
#if defined(Q_WS_WIN)
#ifndef _WIN32_WCE
            WCHAR wPath[MAX_PATH+1];
            if ( SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT, wPath) == S_OK) {
                installdir = QString::fromUtf16((const ushort *) wPath) + QLatin1Char('/') + installpath + QLatin1Char('/');
            } else {
#endif
                installdir =  QDir::home().path() + QLatin1Char('/') + installPath + QLatin1Char('/');
#ifndef _WIN32_WCE
            }
#endif
#else
            installdir = QDir::home().path() + '/' + installPath + '/';
#endif
            pathcounter++;
        }
        if (!absoluteInstallPath.isEmpty()) {
            installdir = absoluteInstallPath + '/';
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

QStringList Installation::installDownloadedFileAndUncompress(const KNS3::EntryInternal&  entry, const QString& payloadfile, const QString installdir)
{
    QString installpath(payloadfile);
    // Collect all files that were installed
    QStringList installedFiles;

    if (!isRemote()) {
        bool isarchive = true;

        // respect the uncompress flag in the knsrc
        if (uncompression == "always" || uncompression == "archive") {
            // this is weird but a decompression is not a single name, so take the path instead
            installpath = installdir;
            KMimeType::Ptr mimeType = KMimeType::findByPath(payloadfile);
            //kDebug() << "Postinstallation: uncompress the file";

            // FIXME: check for overwriting, malicious archive entries (../foo) etc.
            // FIXME: KArchive should provide "safe mode" for this!
            KArchive *archive = 0;
    

            if (mimeType->is("application/zip")) {
                archive = new KZip(payloadfile);
            } else if (mimeType->is("application/tar")
                       || mimeType->is("application/x-gzip")
                       || mimeType->is("application/x-bzip")
                       || mimeType->is("application/x-lzma")
                       || mimeType->is("application/x-xz")
                       || mimeType->is("application/x-bzip-compressed-tar")
                       || mimeType->is("application/x-compressed-tar") ) {
                archive = new KTar(payloadfile);
            } else {
                delete archive;
                kError() << "Could not determine type of archive file '" << payloadfile << "'";
                if (uncompression == "always") {
                    return QStringList();
                }
                isarchive = false;
            }

            if (isarchive) {
                bool success = archive->open(QIODevice::ReadOnly);
                if (!success) {
                    kError() << "Cannot open archive file '" << payloadfile << "'";
                    if (uncompression == "always") {
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

        if (uncompression == "never" || (uncompression == "archive" && !isarchive)) {
            // no decompress but move to target

            /// @todo when using KIO::get the http header can be accessed and it contains a real file name.
            // FIXME: make naming convention configurable through *.knsrc? e.g. for kde-look.org image names
            KUrl source = KUrl(entry.payload());
            kDebug() << "installing non-archive from " << source.url();
            QString installfile;
            QString ext = source.fileName().section('.', -1);
            if (customName) {
                installfile = entry.name();
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
                kDebug() << "move: " << file.fileName() << " to " << installpath;
            }
            if (!success) {
                kError() << "Cannot move file '" << payloadfile << "' to destination '"  << installpath << "'";
                return QStringList();
            }
            installedFiles << installpath;
        }
    }
    return installedFiles;
}

void Installation::runPostInstallationCommand(const QString& installPath)
{
    KProcess process;
    QString command(postInstallationCommand);
    QString fileArg(KShell::quoteArg(installPath));
    command.replace("%f", fileArg);

    kDebug() << "Run command: " << command;

    process.setShellCommand(command);
    int exitcode = process.execute();

    if (exitcode) {
        kError() << "Command failed" << endl;
    }
}


void Installation::uninstall(EntryInternal entry)
{
    entry.setStatus(Entry::Deleted);

    if (!uninstallCommand.isEmpty()) {
        KProcess process;
        foreach (const QString& file, entry.installedFiles()) {
            QFileInfo info(file);
            if (info.isFile()) {
                QString fileArg(KShell::quoteArg(file));
                QString command(uninstallCommand);
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

    emit signalEntryChanged(entry);
}


void Installation::slotInstallationVerification(int result)
{
    //kDebug() << "SECURITY result " << result;

    //FIXME do something here ??? and get the right entry again
    EntryInternal entry;

    if (result & Security::SIGNED_OK)
        emit signalEntryChanged(entry);
    else
        emit signalEntryChanged(entry);
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
