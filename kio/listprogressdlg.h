#ifndef __kio_listprogress_dlg_h__
#define __kio_listprogress_dlg_h__

#include <qwidget.h>

class KIOListBox;
class QTimer;

class KIOListProgressDlg : public QWidget
{
  Q_OBJECT

public:

  KIOListProgressDlg();

protected:

  QTimer* updateTimer;
  KIOListBox *listBox;

  readSettings();
  writeSettings();

protected slots:

  void slotUpdate();

};

#endif
