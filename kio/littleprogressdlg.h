#ifndef __kio_littleprogress_dlg_h__
#define __kio_littleprogress_dlg_h__

#include <qlabel.h>
#include <qpushbutton.h>

#include <kprogress.h>

#include "kio/progressbase.h"

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
* <pre>
* // create a dialog
* KIOLittleProgressDlg *littleProgress;
* littleProgress = new KIOLittleProgressDlg( statusBar() );
* statusBar()->insertWidget( littleProgress, littleProgress->width() , 0 );
* ...
* // create KIOJob and set the progress
* KIOJob* job;
* job->setGUImode( KIOJob::CUSTOM );
* job->setProgressDlg( littleProgress );
* ...
* </pre>
*
* @short IO progress widget for embedding in a statusbar.
* @author Matej Koss <koss@miesto.sk>
*/
class KIOLittleProgressDlg : public KIOProgressBase {

  Q_OBJECT

public:

  KIOLittleProgressDlg( QWidget* parent, bool button = true );
  ~KIOLittleProgressDlg() {}

  virtual void setJob( KIOJob * );
  virtual void clean();

protected slots:

  virtual void slotTotalSize( int, unsigned long _bytes );
  virtual void slotPercent( int, unsigned long _bytes );
  virtual void slotSpeed( int, unsigned long _bytes_per_second );

protected:
  KProgress* m_pProgressBar;
  QLabel* m_pLabel;
  QPushButton* m_pButton;

  unsigned long m_iTotalSize;

  enum Mode { None, Label, Progress };

  uint mode;
  bool m_bShowButton;

  void setMode();

  bool eventFilter( QObject *, QEvent * );

};

#endif
