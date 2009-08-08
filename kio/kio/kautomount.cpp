/* This file is part of the KDE libraries
   Copyright (C) 2000 David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "kautomount.h"
#include "krun.h"
#include "kdirwatch.h"
#include "kio/job.h"
#include "kio/jobuidelegate.h"
#include <kdirnotify.h>
#include <kdebug.h>
#include <kmountpoint.h>

/***********************************************************************
 *
 * Utility classes
 *
 ***********************************************************************/

class KAutoMountPrivate
{
public:
    KAutoMountPrivate(KAutoMount *qq, const QString &device, const QString &desktopFile,
                      bool showFileManagerWindow)
        : q(qq), m_strDevice(device), m_desktopFile(desktopFile),
          m_bShowFilemanagerWindow(showFileManagerWindow)
        { }

    KAutoMount *q;
    QString m_strDevice;
    QString m_desktopFile;
    bool m_bShowFilemanagerWindow;

    void slotResult( KJob * );
};

KAutoMount::KAutoMount( bool _readonly, const QByteArray& _format, const QString& _device,
                        const QString&  _mountpoint, const QString & _desktopFile,
                        bool _show_filemanager_window )
    : d(new KAutoMountPrivate(this, _device, _desktopFile, _show_filemanager_window))
{
    KIO::Job* job = KIO::mount( _readonly, _format, _device, _mountpoint );
    connect( job, SIGNAL( result( KJob * ) ), this, SLOT( slotResult( KJob * ) ) );
}

KAutoMount::~KAutoMount()
{
    delete d;
}

void KAutoMountPrivate::slotResult( KJob * job )
{
    if ( job->error() ) {
        emit q->error();
        job->uiDelegate()->showErrorMessage();
    } else {
        KMountPoint::Ptr mp = KMountPoint::currentMountPoints().findByDevice( m_strDevice );
        if (!mp) {
            kWarning(7015) << m_strDevice << "was correctly mounted, but findByDevice() didn't find it."
                           << "This looks like a bug, please report it on http://bugs.kde.org, together with your /etc/fstab and /etc/mtab lines for this device";
        } else {
            KUrl url(mp->mountPoint());
            //kDebug(7015) << "KAutoMount: m_strDevice=" << m_strDevice << " -> mountpoint=" << mountpoint;
            if ( m_bShowFilemanagerWindow ) {
                KRun::runUrl( url, "inode/directory", 0 /*TODO - window*/ );
            }
            // Notify about the new stuff in that dir, in case of opened windows showing it
            org::kde::KDirNotify::emitFilesAdded( url.url() );
        }

        // Update the desktop file which is used for mount/unmount (icon change)
        kDebug(7015) << " mount finished : updating " << m_desktopFile;
        KUrl dfURL;
        dfURL.setPath( m_desktopFile );
        org::kde::KDirNotify::emitFilesChanged( QStringList() << dfURL.url() );
        //KDirWatch::self()->setFileDirty( m_desktopFile );

        emit q->finished();
    }
    q->deleteLater();
}

class KAutoUnmountPrivate
{
public:
    KAutoUnmountPrivate( KAutoUnmount *qq, const QString & _mountpoint, const QString & _desktopFile )
        : q(qq), m_desktopFile( _desktopFile ), m_mountpoint( _mountpoint )
    {}
    KAutoUnmount *q;
    QString m_desktopFile;
    QString m_mountpoint;

    void slotResult( KJob * job );
};

KAutoUnmount::KAutoUnmount( const QString & _mountpoint, const QString & _desktopFile )
    : d( new KAutoUnmountPrivate(this, _mountpoint, _desktopFile) )
{
    KIO::Job * job = KIO::unmount( d->m_mountpoint );
    connect( job, SIGNAL( result( KJob * ) ), this, SLOT( slotResult( KJob * ) ) );
}

void KAutoUnmountPrivate::slotResult( KJob * job )
{
    if ( job->error() ) {
        emit q->error();
        job->uiDelegate()->showErrorMessage();
    }
    else
    {
        // Update the desktop file which is used for mount/unmount (icon change)
        kDebug(7015) << "unmount finished : updating " << m_desktopFile;
        KUrl dfURL;
        dfURL.setPath( m_desktopFile );
        org::kde::KDirNotify::emitFilesChanged( QStringList() << dfURL.url() );
        //KDirWatch::self()->setFileDirty( m_desktopFile );

        // Notify about the new stuff in that dir, in case of opened windows showing it
        // You may think we removed files, but this may have also readded some
        // (if the mountpoint wasn't empty). The only possible behavior on FilesAdded
        // is to relist the directory anyway.
        KUrl mp( m_mountpoint );
        org::kde::KDirNotify::emitFilesAdded( mp.url() );

        emit q->finished();
    }

    q->deleteLater();
}

KAutoUnmount::~KAutoUnmount()
{
    delete d;
}

#include "kautomount.moc"
