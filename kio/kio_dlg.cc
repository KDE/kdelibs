#include "kio_dlg.h"

#include <qmessagebox.h>

bool open_CriticalDlg( const char *_titel, const char *_message, const char *_but1, const char *_but2 )
{
  if ( QMessageBox::critical( 0L, _titel, _message, _but1, _but2 ) )
    return true;
  return false;
}
