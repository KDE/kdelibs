// $Id$


#ifndef __progressbase_h__
#define __progressbase_h__


#include <qwidget.h>

class KURL;
namespace KIO { class Job; }


namespace KIO
{
  enum Progress {
    DEFAULT = 1,
    STATUSBAR = 2,
    LIST = 3
  };

};

/**
* Base class for IO progress dialogs.
*
* This class does all initialization stuff for progress,
* like connecting signals to slots.
* All slots are implemented as pure virtual methods.
*
* All custom IO progress dialog should inherit this class.
* Add your GUI code to the constructor and implemement those virtual
* methods which you need in order to display progress.
*
* E.g. @ref #StatusbarProgress only implements @ref #slotTotalSize,
* @ref #slotPercent and @ref slotSpeed.
*
* Custom progress dialog will be used like this :
* <pre>
* // create job
* CopyJob* job = KIO::copy(...);
* // create a dialog
* MyCustomProgress *customProgress;
* customProgress = new MyCustomProgress();
* // connect progress with job
* customProgress->setJob( job );
* ...
* </pre>
*
* There is a special method @ref setStopOnClose that controls the behaviour of
* the dialog.
* @short Base class for IO progress dialogs.
* @author Matej Koss <koss@miesto.sk>
*/
class ProgressBase : public QWidget {

  Q_OBJECT

public:

  ProgressBase( QWidget *parent = 0L );
  ~ProgressBase() {}

  virtual void setJob( KIO::Job * );
  virtual void clean() {}

  bool onlyClean() { return m_bOnlyClean; }

  /**
   * Use this to set whether progress dialog should also kill the KIO::Job when closed.
   *
   * @param  stop   if true - KIO::Job will be killed.
   *                if false - Dialog will be closed without killing the job ( IO will continue ).
   */
  void setStopOnClose( bool stop ) { m_bStopOnClose = stop; }

public slots:
  void stop();

protected:

  void closeEvent( QCloseEvent * );
  void Connect();

  KIO::Job* m_pJob;

  /**
   * This variable controls wether the dialog should be deleted or only cleaned when
   * the KIO::Job is finished ( or canceled ).
   *
   * If your dialog is embedded widget and not a separate window, you should set this
   * variable to true in the constructor of your custom dialog.
   *
   * If true - Dialog will only call method @ref clean.
   * If false - Dialog will be deleted.
   */
  bool m_bOnlyClean;

  bool m_bStopOnClose;

protected slots:

  virtual void slotTotalSize( KIO::Job*, unsigned long ) {}
  virtual void slotTotalFiles( KIO::Job*, unsigned long ) {}
  virtual void slotTotalDirs( KIO::Job*, unsigned long ) {}

  virtual void slotProcessedSize( KIO::Job*, unsigned long ) {}
  virtual void slotProcessedFiles( KIO::Job*, unsigned long ) {}
  virtual void slotProcessedDirs( KIO::Job*, unsigned long ) {}

  virtual void slotSpeed( KIO::Job*, unsigned long ) {}
  virtual void slotPercent( KIO::Job*, unsigned int ) {}

  virtual void slotCopyingFile( KIO::Job*, const KURL&, const KURL& ) {}
  virtual void slotMovingFile( KIO::Job*, const KURL&, const KURL& ) {}
  virtual void slotDeletingFile( KIO::Job*, const KURL& ) {}
  virtual void slotCreatingDir( KIO::Job*, const KURL& ) {}

  virtual void slotCanResume( KIO::Job*, bool ) {}
};


#endif // __progressbase_h__
