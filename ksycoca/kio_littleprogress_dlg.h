#ifndef __kio_littleprogress_dlg_h__
#define __kio_littleprogress_dlg_h__

#include <qwidget.h>
#include <qlabel.h>
#include <qpopupmenu.h>

#include <kprogress.h>

class KIOJob;

class KIOLittleProgressDlg : public QWidget
{
  Q_OBJECT
public:

  KIOLittleProgressDlg( QWidget* parent );

  void processedSize();
  void speed();

  void setJob( KIOJob *job );

  void clean();

protected:
  KProgress* m_pProgressBar;
  QLabel* m_pLabel;
  QPopupMenu* m_pMenu;

  KIOJob* m_pJob;

  int m_iPercent;
  bool mode;

  void setMode( bool _mode );

  bool eventFilter( QObject *, QEvent * );

};

#endif
