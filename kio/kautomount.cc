#include "kio_job.h"
#include "kautomount.h"
#include "kio_error.h"
#include "krun.h"
#include "kdirwatch.h"
#include <kdebug.h>


/***********************************************************************
 *
 * Utility classes
 *
 ***********************************************************************/

KAutoMount::KAutoMount( bool _readonly, const char *_format, const char *_device, 
                        const char * _mountpoint, const QString & _desktopFileDir, 
                        bool _show_filemanager_window )
  : m_strDevice( _device ),
    m_desktopFileDir( _desktopFileDir )
{
  m_bShowFilemanagerWindow = _show_filemanager_window;
  
  KIOJob* job = new KIOJob();
  connect( job, SIGNAL( sigFinished( int ) ), this, SLOT( slotFinished( int ) ) );
  connect( job, SIGNAL( sigError( int, int, const char* ) ), this, SLOT( slotError( int, int, const char* ) ) );
    
  if ( !_format )
    job->mount( false, 0L, _device, 0L );
  else
    job->mount( _readonly, _format, _device, _mountpoint );
}

void KAutoMount::slotFinished( int )
{
  QString mp = KIOJob::findDeviceMountPoint( m_strDevice.ascii() );

  if ( m_bShowFilemanagerWindow )
    KFileManager::getFileManager()->openFileManagerWindow( mp.ascii() );

  // Update of window which contains the desktop entry which is used for mount/unmount
  kdebug( KDEBUG_INFO, 7015, " mount finished : updating %s", m_desktopFileDir.ascii());
  KDirWatch::self()->setVeryDirty( m_desktopFileDir );

  delete this;
}

void KAutoMount::slotError( int, int _errid, const char* _errortext )
{
  kioErrorDialog( _errid, _errortext );

  delete this;
}

KAutoUnmount::KAutoUnmount( const QString & _mountpoint, const QString & _desktopFileDir )
  : m_desktopFileDir( _desktopFileDir )
{
  KIOJob* job = new KIOJob();
  connect( job, SIGNAL( sigFinished( int ) ), this, SLOT( slotFinished( int ) ) );
  connect( job, SIGNAL( sigError( int, int, const char* ) ), this, SLOT( slotError( int, int, const char* ) ) );

  job->unmount( _mountpoint.ascii() );
}

void KAutoUnmount::slotFinished( int )
{
  // Update of window which contains the desktop entry which is used for mount/unmount
  kdebug( KDEBUG_INFO, 7015, "unmount finished : updating %s", m_desktopFileDir.ascii());
  KDirWatch::self()->setVeryDirty( m_desktopFileDir );

  delete this;
}

void KAutoUnmount::slotError( int, int _errid, const char* _errortext )
{
  kioErrorDialog( _errid, _errortext );

  delete this;
}

#include "kautomount.moc"
