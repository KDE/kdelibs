#include "kio_interface.h"
#include "kio_error.h"

#include <kmessageboxwrapper.h>

void kioErrorDialog( int _errid, const char *_errortext )
{
  QString tmp = KIO::kioErrorString( _errid, _errortext );
  
  KMessageBoxWrapper::error( 0, tmp);
}
