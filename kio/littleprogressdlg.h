#ifndef __kio_littleprogress_dlg_h__
#define __kio_littleprogress_dlg_h__

#include <qwidget.h>
#include <qlabel.h>
#include <qpushbutton.h>

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
* Usage of KIOLittleProgressDialog is little different.
* This dialog will be a part of some application.
*
* KIOLittleProgressDlg *littleProgress;
* littleProgress = new KIOLittleProgressDlg( statusBar() );
* statusBar()->insertWidget( littleProgress, littleProgress->width() , 0 );
*
* KIOJob* job;
* // set the KIOJob's GUI type to little progress widget
* job->setGUImode( KIOJob::LITTLE );

* // this connects job with a little progress dialog
* job->connectProgress( littleProgress );
*
* Call @ref #clean() method when you are finished with IO ( e.g. in a slot that
* is connected to KIOJob's signal @ref #sigFinished or @ref #sigError
*
* @short IO progress widget for embedding in a statusbar.
*/ 
class KIOLittleProgressDlg : public QWidget {

  Q_OBJECT

public:

  KIOLittleProgressDlg( QWidget* parent );
  ~KIOLittleProgressDlg() {}

  void setJob( KIOJob *job );
  void clean();

public slots:

  void slotTotalSize( int, unsigned long _bytes );
  void slotPercent( int, unsigned long _bytes );
  void slotSpeed( int, unsigned long _bytes_per_second );

protected:
  KProgress* m_pProgressBar;
  QLabel* m_pLabel;
  QPushButton* m_pButton;

  KIOJob* m_pJob;

  unsigned long m_iTotalSize;

  enum Mode { None, Label, Progress };

  uint mode;

  void setMode();

  bool eventFilter( QObject *, QEvent * );

};

#endif
