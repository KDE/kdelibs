#include <qmessagebox.h>
#include <kapp.h>

#include "kio_dlg.h"

bool open_CriticalDlg( const QString &_title, const QString &_message, const QString &_but1, const QString &_but2 )
{
  if ( kapp == 0L )
  {
    (void)new KApplication( );
  }

  if ( QMessageBox::critical( 0L, _title, _message, _but1, _but2 ) )
    return true;
  return false;
}
