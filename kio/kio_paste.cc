#include "kio_paste.h"
#include "kio_linedit_dlg.h"
#include "kio_error.h"
#include "kio_interface.h"
#include "kio_job.h"

#include <kstring.h>
#include <kclipboard.h>
#include <k2url.h>
#include <kapp.h>

#include <qmsgbox.h>

#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <list>
#include <string>

void pasteClipboard( const char *_dest_url )
{
  list<K2URL> lst;
  if ( !K2URL::split( _dest_url, lst ) )
  {
    kioErrorDialog( ERR_MALFORMED_URL, _dest_url );
    return;
  }
  
  QStrList urls;
  if ( KClipboard::self()->urlList( urls ) )
  {
    if ( urls.count() == 0 )
    {
      QMessageBox::critical( 0L, i18n("Error"), i18n("The clipboard is empty"), i18n("Ok") );
      return;
    }

    KIOJob* job = new KIOJob;
    job->copy( urls, _dest_url );
    
    return;
  }
  
  if ( KClipboard::self()->size() == 0 )
  {
    QMessageBox::critical( 0L, i18n("Error"), i18n("The clipboard is empty"), i18n("Ok") );
    return;
  }
  
  QByteArray ba = KClipboard::self()->octetStream();

  pasteData( _dest_url, ba );
}

void pasteData( const char *_dest_url, QByteArray _data )
{
  K2URL u( _dest_url );
  if ( !u.isLocalFile() )
  {
    // TODO: Use KIO put command here for writing the data.
    QMessageBox::critical( 0L, i18n("Error"), i18n("Pasting clipboard data is only supported on the local hard disk currently"),
			   i18n("Ok") );
    return;
  }

  KLineEditDlg l( i18n("Filename for clipboard content:"), "", 0L, false );
  int x = l.exec();
  if ( x )
  {
    QString url = l.text();
    if ( url.isEmpty() )
    {
      QMessageBox::critical( 0L, i18n("Error"), i18n("You did not enter a filename"), i18n("Ok") );
      return;
    }
	
    u.addPath( l.text() );
    
    struct stat buff;
    if ( stat( u.path(), &buff ) == 0 )
    {
      QString tmp;
      ksprintf( &tmp, i18n("The file %s does already exist. Do you really want to overwrite it ?" ), u.path() );
      if ( QMessageBox::critical( 0L, i18n("Warning"), tmp,i18n("Yes"), i18n("No") ) == 1 )
	return;
    }
    
    FILE *f = fopen( u.path(), "wb" );
    if ( f == 0L )
    {
      kioErrorDialog( ERR_WRITE_ACCESS_DENIED, u.path() );
      return;
    }
    
    fwrite( _data.data(), 1, _data.size(), f );
    fclose( f );
  }
}
