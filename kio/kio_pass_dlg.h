#ifndef __kio_pass_dlg_h__
#define __kio_pass_dlg_h__

#include <string>

bool open_PassDlg( const char *_head, string& _user, string& _pass );

#include <qdialog.h>
#include <qlineedit.h>

class KIOPassDlg : public QDialog
{
  Q_OBJECT   
public:
  KIOPassDlg( QWidget* parent, const char* name, bool modal, WFlags wflags,
	      const char *_head, const char *_user, const char *_pass );

  const char *password() { return m_pPass->text(); }
  const char *user() { return m_pUser->text(); }
    
private:
  QLineEdit* m_pPass;
  QLineEdit* m_pUser;
};


#endif


