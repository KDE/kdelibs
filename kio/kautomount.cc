#include "kautomount.h"
#include "kio_job.h"
#include "kio_error.h"

// This function is implemeneted in kfmgui.cc and in kfmlib
extern void openFileManagerWindow( const char *_url );

/***********************************************************************
 *
 * Utility classes
 *
 ***********************************************************************/

KAutoMount::KAutoMount( bool _readonly, const char *_format, const char *_device, const char *_mountpoint,
			bool _show_filemanager_window )
{
  m_strDevice = _device;
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
  QString mp = KIOJob::findDeviceMountPoint( m_strDevice );

  if ( m_bShowFilemanagerWindow )
    openFileManagerWindow( mp );

  // TODO: Fake update of window which contains the kdelnk file which is used for
  //       mount/unmount

  delete this;
}

void KAutoMount::slotError( int, int _errid, const char* _errortext )
{
  kioErrorDialog( _errid, _errortext );

  delete this;
}

KAutoUnmount::KAutoUnmount( const char *_mountpoint )
{
  KIOJob* job = new KIOJob();
  connect( job, SIGNAL( sigFinished( int ) ), this, SLOT( slotFinished( int ) ) );
  connect( job, SIGNAL( sigError( int, int, const char* ) ), this, SLOT( slotError( int, int, const char* ) ) );

  job->unmount( _mountpoint );
}

void KAutoUnmount::slotFinished( int )
{
  // TODO: Fake update of window which contains the kdelnk file which is used for
  //       mount/unmount

  delete this;
}

void KAutoUnmount::slotError( int, int _errid, const char* _errortext )
{
  kioErrorDialog( _errid, _errortext );

  delete this;
}

#include "kautomount.moc"
