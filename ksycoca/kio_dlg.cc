#include <qmessagebox.h>
#include <kapp.h>

#include "kio_dlg.h"

bool open_CriticalDlg( const char *_title, const char *_message, const char *_but1, const char *_but2 )
{
  if ( kapp == 0L )
  {
    const char* a[2] = { "dummy", 0L };
    int b = 1;
    (void)new KApplication( b, const_cast<char**>(a));
  }

  if ( QMessageBox::critical( 0L, _title, _message, _but1, _but2 ) )
    return true;
  return false;
}
