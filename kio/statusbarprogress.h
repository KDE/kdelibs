#ifndef __statusbarprogress_h__
#define __statusbarprogress_h__

#include <qlabel.h>
#include <qpushbutton.h>

#include <kprogress.h>

#include "progressbase.h"

namespace KIO { class Job; }

/**
* This is a special IO progress widget.
*
* Similarly to @ref #DefaultProgress,
* it's purpose is to show a progress of the IO operation.
*
* Instead of creating a separate window, this is only a widget that can be
* easily embedded in a statusbar.
*
* Usage of StatusbarProgress is little different.
* This dialog will be a part of some application.
* <pre>
* // create a dialog
* StatusbarProgress *statusProgress;
* statusProgress = new StatusbarProgress( statusBar() );
* statusBar()->insertWidget( statusProgress, statusProgress->width() , 0 );
* ...
* // create job and connect it to the progress
* CopyJob* job = KIO::copy(...);
* statusProgress->setJob( job );
* ...
* </pre>
*
* @short IO progress widget for embedding in a statusbar.
* @author Matej Koss <koss@miesto.sk>
*/
class StatusbarProgress : public ProgressBase {

  Q_OBJECT

public:

  StatusbarProgress( QWidget* parent, bool button = true );
  ~StatusbarProgress() {}

  virtual void setJob( KIO::Job * );
  virtual void clean();

protected slots:

  virtual void slotTotalSize( KIO::Job*, unsigned long _bytes );
  virtual void slotPercent( KIO::Job*, unsigned long _bytes );
  virtual void slotSpeed( KIO::Job*, unsigned long _bytes_per_second );

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

#endif  //  __statusbarprogress_h__
