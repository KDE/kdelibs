#ifndef __kio_littleprogress_dlg_h__
#define __kio_littleprogress_dlg_h__

#include <qwidget.h>
#include <qlabel.h>
#include <qpopupmenu.h>

#include <kprogress.h>

class KIOJob;

/** 
* This is a special IO progress widget.
*
* Similarly to @ref #KIOSimpleProgressDlg and @ref #KIOListProgressDlg,
* it's purpose is to show a progress of the IO operation.
*
* Instead of creating a separate window, this is only a widget that can be
* easily embedded in a statusbar.
*
* @short IO progress widget for embedding in a statusbar.
*/ 
class KIOLittleProgressDlg : public QWidget {

  Q_OBJECT

public:

  KIOLittleProgressDlg( QWidget* parent );

  void processedSize();
  void speed();

  void setJob( KIOJob *job );

  void clean();

public slots:

  void slotTotalSize( int, unsigned long _bytes );
  void slotPercent( int, unsigned long _bytes );
  void slotSpeed( int, unsigned long _bytes_per_second );

protected:
  KProgress* m_pProgressBar;
  QLabel* m_pLabel;
  QPopupMenu* m_pMenu;

  KIOJob* m_pJob;

  unsigned long m_iTotalSize;

  bool mode;

  void setMode( bool _mode );

  bool eventFilter( QObject *, QEvent * );

};

#endif
