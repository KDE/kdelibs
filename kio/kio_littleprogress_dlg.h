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

  void totalSize( unsigned long );
  void processedSize( unsigned long );
  void speed( unsigned long );

  void setJob( KIOJob *job );

  void clean();

protected:
  KProgress* m_pProgressBar;
  QLabel* m_pLabel;

  QPopupMenu* m_pMenu;

  unsigned long m_iTotalSize;
  unsigned long m_iProcessedSize;
  unsigned long m_iSpeed;

  int m_iPercent;

  KIOJob* m_pJob;

  bool mode;

  void setMode( bool _mode );

  bool eventFilter( QObject *, QEvent * );

};

#endif
