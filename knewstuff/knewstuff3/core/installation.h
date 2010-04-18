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
#ifndef KNEWSTUFF3_INSTALLATION_H
#define KNEWSTUFF3_INSTALLATION_H

#include <QtCore/QObject>
#include <QtCore/QString>

#include <kconfiggroup.h>

#include "entryinternal.h"

class KArchiveDirectory;
class KJob;

namespace KNS3
{

/**
 * @short KNewStuff entry installation.
 *
 * The installation class stores all information related to an entry's
 * installation.
 *
 * @author Josef Spillner (spillner@kde.org)
 *
 * @internal
 */
class Installation :public QObject
{
    Q_OBJECT
public:
    /**
     * Constructor.
     */
    Installation(QObject* parent = 0);

    enum Policy {
        CheckNever,
        CheckIfPossible,
        CheckAlways
    };

    enum Scope {
        ScopeUser,
        ScopeSystem
    };

    bool readConfig(const KConfigGroup& group);
    bool isRemote() const;

public Q_SLOTS:
    /**
     * Downloads a payload file. The payload file matching most closely
     * the current user language preferences will be downloaded.
     * The file will not be installed set, for this \ref install must
     * be called.
     *
     * @param entry Entry to download payload file for
     *
     * @see signalPayloadLoaded
     * @see signalPayloadFailed
     */
    void downloadPayload(const KNS3::EntryInternal& entry);

    /**
     * Installs an entry's payload file. This includes verification, if
     * necessary, as well as decompression and other steps according to the
     * application's *.knsrc file.
     * Note that this method is asynchronous and thus the return value will
     * only report the successful start of the installation.
     *
     * @param entry Entry to be installed
     *
     * @return Whether or not installation was started successfully
     *
     * @see signalInstallationFinished
     * @see signalInstallationFailed
     */
    void install(KNS3::EntryInternal entry);

    /**
     * Uninstalls an entry. It reverses the steps which were performed
     * during the installation.
     *
     * @param entry The entry to deinstall
     *
     * @return Whether or not deinstallation was successful
     *
     * @note FIXME: I don't believe this works yet :)
     */
    void uninstall(KNS3::EntryInternal entry);

    void slotInstallationVerification(int result);
    void slotPayloadResult(KJob *job);

Q_SIGNALS:
    void signalEntryChanged(const KNS3::EntryInternal& entry);
    void signalInstallationFinished();
    void signalInstallationFailed(const QString& message);

    void signalPayloadLoaded(KUrl payload); // FIXME: return Entry

private:
    void install(KNS3::EntryInternal entry, const QString& downloadedFile);

    QString targetInstallationPath(const QString& payloadfile);
    QStringList installDownloadedFileAndUncompress(const KNS3::EntryInternal&  entry, const QString& payloadfile, const QString installdir);
    void runPostInstallationCommand(const QString& installPath);

    static QStringList archiveEntries(const QString& path, const KArchiveDirectory * dir);

    // applications can set this if they want the installed files/directories to be piped into a shell command
    QString postInstallationCommand;
    // a custom command to run for the uninstall
    QString uninstallCommand;
    // compression policy
    QString uncompression;

    // only one of the five below can be set, that will be the target install path/file name
    // FIXME: check this when reading the config and make one path out of it if possible?
    QString standardResourceDirectory;
    QString targetDirectory;
    QString xdgTargetDirectory;
    QString installPath;
    QString absoluteInstallPath;

    // policies whether verification needs to be done
    Policy checksumPolicy;
    Policy signaturePolicy;
    // scope: install into user or system dirs
    Scope scope;

    // FIXME this throws together a file name from entry name and version - why would anyone want that?
    bool customName;
    bool acceptHtml;

    QMap<KJob*, EntryInternal> entry_jobs;

    Q_DISABLE_COPY(Installation)
};

}

#endif
