#ifndef __kio_pass_dlg_h__
#define __kio_pass_dlg_h__

#include <qstring.h>
#include <qdialog.h>
#include <qlineedit.h>

namespace KIO {

bool open_PassDlg( const QString& _head, QString& _user, QString& _pass );

class PassDlg : public QDialog
{
  Q_OBJECT
public:
  PassDlg( QWidget* parent, const char* name, bool modal, WFlags wflags,
           const QString& _head, const QString& _user, const QString& _pass );

  QString password() { return m_pPass->text(); }
  QString user() { return m_pUser->text(); }

private:
  QLineEdit* m_pPass;
  QLineEdit* m_pUser;
};

};

#endif


