// $Id$

#include "kio_job.h" 
#include "kio_paste.h"
#include "kio_error.h"
#include "kio_interface.h"

#include <qclipboard.h>
#include <qdragobject.h>
#include <kurl.h>
#include <kapp.h>
#include <klocale.h>
#include <klineeditdlg.h>

#include <kmessagebox.h>

#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

bool isClipboardEmpty()
{
  QStringList urls;
  QMimeSource *data = QApplication::clipboard()->data();
  
  if ( QUriDrag::canDecode( data ) && QUriDrag::decodeToUnicodeUris( data, urls ) ){
    if ( urls.count() == 0 )
      return true;
    return false;
  }

  return true;
}

void pasteClipboard( const char *_dest_url )
{
  if ( KURL::split( _dest_url ).isEmpty() ) {
    kioErrorDialog( KIO::ERR_MALFORMED_URL, _dest_url );
    return;
  }
  
  QMimeSource *data = QApplication::clipboard()->data();
  
  QStringList urls;
  if ( QUriDrag::canDecode( data ) && QUriDrag::decodeToUnicodeUris( data, urls ) ) {
    if ( urls.count() == 0 ) {
      KMessageBox::error( 0L, i18n("The clipboard is empty"));
      return;
    }

    KIOJob* job = new KIOJob;
    job->copy( urls, _dest_url );

    return;
  }

  QByteArray ba = data->encodedData( data->format() );

  if ( ba.size() == 0 )
  {
    KMessageBox::sorry(0, i18n("The clipboard is empty"));
    return;
  }

  pasteData( _dest_url, ba );
}

void pasteData( const char *_dest_url, QByteArray _data )
{
  KURL u( _dest_url );
  if ( !u.isLocalFile() ) {
    // TODO: Use KIO put command here for writing the data.
    KMessageBox::sorry( 0L, i18n("Pasting clipboard data is only supported on the local hard disk currently"));
    return;
  }

  KLineEditDlg l( i18n("Filename for clipboard content:"), "", 0L, false );
  int x = l.exec();
  if ( x ) {
    QString url = l.text();
    if ( url.isEmpty() ) {
      KMessageBox::error( 0L, i18n("You did not enter a filename"));
      return;
    }
	
    u.addPath( l.text() );
    
    struct stat buff;
    if ( stat( u.path().ascii(), &buff ) == 0 ) {
      QString tmp = i18n("The file %1 does already exist. Do you really want to overwrite it ?" ).arg( u.path() );
      if ( KMessageBox::warningYesNo( 0L, tmp) == 1 )
	return;
    }
    
    FILE *f = fopen( u.path().ascii(), "wb" );
    if ( f == 0L ) {
      kioErrorDialog( KIO::ERR_WRITE_ACCESS_DENIED, u.path().ascii() );
      return;
    }

    fwrite( _data.data(), 1, _data.size(), f );
    fclose( f );
  }
}
