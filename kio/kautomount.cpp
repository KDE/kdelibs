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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "kautomount.h"
#include "krun.h"
#include "kdirwatch.h"
#include "kio/job.h"
#include <kdebug.h>

/***********************************************************************
 *
 * Utility classes
 *
 ***********************************************************************/

KAutoMount::KAutoMount( bool _readonly, const QString& _format, const QString& _device,
                        const QString&  _mountpoint, const QString & _desktopFile,
                        bool _show_filemanager_window )
  : m_strDevice( _device ),
    m_desktopFile( _desktopFile )
{
  m_bShowFilemanagerWindow = _show_filemanager_window;

  KIO::Job* job = KIO::mount( _readonly, _format, _device, _mountpoint );
  connect( job, SIGNAL( result( KIO::Job * ) ), this, SLOT( slotResult( KIO::Job * ) ) );
}

void KAutoMount::slotResult( KIO::Job * job )
{
  if ( job->error() )
    job->showErrorDialog();
  else
  {
    QString mp = KIO::findDeviceMountPoint( m_strDevice );

    if ( m_bShowFilemanagerWindow )
      KRun::runURL( mp, "inode/directory" );
    //      KFileManager::getFileManager()->openFileManagerWindow( mp );

    // Update of window which contains the desktop entry which is used for mount/unmount
    kdDebug(7015) << " mount finished : updating " << m_desktopFile << endl;
    KDirWatch::self()->setFileDirty( m_desktopFile );
  }
  delete this;
}

KAutoUnmount::KAutoUnmount( const QString & _mountpoint, const QString & _desktopFile )
  : m_desktopFile( _desktopFile )
{
  KIO::Job * job = KIO::unmount( _mountpoint );
  connect( job, SIGNAL( result( KIO::Job * ) ), this, SLOT( slotResult( KIO::Job * ) ) );
}

void KAutoUnmount::slotResult( KIO::Job * job )
{
  if ( job->error() )
    job->showErrorDialog();
  else
  {
    // Update of window which contains the desktop entry which is used for mount/unmount
    kdDebug(7015) << "unmount finished : updating " << m_desktopFile << endl;
    KDirWatch::self()->setFileDirty( m_desktopFile );
  }

  delete this;
}

#include "kautomount.moc"
