#ifndef __kio_skip_dlg__
#define __kio_skip_dlg__ "$Id$"

#include <kdialog.h>

class QPushButton;
class QWidget;

namespace KIO {

  enum SkipDlg_Result { S_SKIP = 1, S_AUTO_SKIP = 2, S_CANCEL = 0 };

  SkipDlg_Result open_SkipDlg( bool _multi, const QString& _error_text = QString::null );

class SkipDlg : public KDialog
{
  Q_OBJECT
public:
  SkipDlg( QWidget *parent, bool _multi, const QString& _error_text, bool _modal = false );
  ~SkipDlg();

protected:
  QPushButton *b0;
  QPushButton *b1;
  QPushButton *b2;

  bool modal;

public slots:
  void b0Pressed();
  void b1Pressed();
  void b2Pressed();

signals:
  void result( SkipDlg *_this, int _button );
};

};
#endif
