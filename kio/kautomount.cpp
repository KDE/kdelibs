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

  KIO::Job* job;

  if ( !_format )
      job = KIO::mount( false, 0L, _device, 0L );
  else
      job = KIO::mount( _readonly, _format, _device, _mountpoint );
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
      KFileManager::getFileManager()->openFileManagerWindow( mp );

    // Update of window which contains the desktop entry which is used for mount/unmount
    kDebugInfo( 7015, " mount finished : updating %s", debugString(m_desktopFile));
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
    kDebugInfo( 7015, "unmount finished : updating %s", debugString(m_desktopFile));
    KDirWatch::self()->setFileDirty( m_desktopFile );
  }

  delete this;
}

#include "kautomount.moc"
