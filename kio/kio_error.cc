#include "kio_interface.h"
#include "kio_error.h"

#include <kmessagebox.h>

void kioErrorDialog( int _errid, const char *_errortext )
{
  QString tmp = KIO::kioErrorString( _errid, _errortext );
  
  KMessageBox::error( 0, tmp);
}
