#ifndef __kio_skip_dlg__
#define __kio_skip_dlg__

enum SkipDlg_Result { S_SKIP = 1, S_AUTO_SKIP = 2, S_CANCEL = 0 };
SkipDlg_Result open_SkipDlg( const char *_dest, bool _multi );

#include <qwidget.h>
#include <qdialog.h>
#include <qpushbt.h>
#include <qlabel.h>
#include <qlined.h>
#include <qlayout.h>

#include <string>

class KIOSkipDlg : public QDialog
{
  Q_OBJECT
public:
  KIOSkipDlg( QWidget *parent, const char *_dest, bool _multi, bool _modal = false );
  ~KIOSkipDlg();
    
protected:
  QPushButton *b0, *b1, *b2;
    
  bool modal;
  
public slots:
  void b0Pressed();
  void b1Pressed();
  void b2Pressed();

signals:
  void result( KIOSkipDlg *_this, int _button );
};

#endif
