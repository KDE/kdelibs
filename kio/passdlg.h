#ifndef __kio_pass_dlg_h__
#define __kio_pass_dlg_h__

#include <qstring.h>
#include <qdialog.h>
#include <qlineedit.h>

namespace KIO {

  class PassDlg : public QDialog {
    Q_OBJECT
  public:
    PassDlg( QWidget* parent, const char* name, bool modal, WFlags wflags,
	     const QString& head, const QString& user, const QString& pass );

      QString password() { return m_pPass->text(); }
      QString user() { return m_pUser->text(); }

  private:
      QLineEdit* m_pPass;
      QLineEdit* m_pUser;
  };

};

#endif


