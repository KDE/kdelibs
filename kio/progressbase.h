// $Id$


#ifndef __kio_progressbase_h__
#define __kio_progressbase_h__


#include <qwidget.h>

class KURL;
class KIO::Job;


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
* E.g. @ref #KIOStatusbarProgress only implements @ref #slotTotalSize,
* @ref #slotPercent and @ref slotSpeed.
*
* Custom progress dialog will be used like this :
* <pre>
* // create a dialog
* MyCustomProgress *customProgress;
* customProgress = new MyCustomProgress();
* ...
* // create KIOJob and set the progress
* KIOJob* job;
* job->setGUImode( KIOJob::CUSTOM );
* job->setProgressDlg( customProgress );
* ...
* </pre>
*
* You can either keep the pointer to the dialog, or simply call @ref KIOJob::progressDlg()
* in order to obtain this pointer.
*
* There is a special method @ref setStopOnClose that controls the behaviour of
* the dialog.
* @short Base class for IO progress dialogs.
* @author Matej Koss <koss@miesto.sk>
*/
class KIOProgressBase : public QWidget {

  Q_OBJECT

public:

  KIOProgressBase( QWidget *parent = 0L );
  ~KIOProgressBase() {}

  virtual void setJob( KIO::Job * );
  virtual void clean() {}

  /**
   * Use this to iconify / deiconify the dialog.
   *
   * @param  stop   if true - KIOJob will be killed.
   *                if false - Dialog will be closed without killing the job ( IO will continue ).
   */
  void iconify( bool );

  /**
   * Use this method to get the values from KIOJob and refill the fields.
   * This method automatically calls all slots, and thus it refills only those values,
   * for which the slots were reimplemented.
   *
   * Use this method when you open the dialog in the middle of IO.
   */
  void refill();

  bool onlyClean() { return m_bOnlyClean; }

  /**
   * Use this to set whether progress dialog should also kill the KIOJob when closed.
   *
   * @param  stop   if true - KIOJob will be killed.
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
   * the KIOJob is finished ( or canceled ).
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
  virtual void slotPercent( KIO::Job*, unsigned long ) {}

  virtual void slotCopyingFile( KIO::Job*, const KURL&, const KURL& ) {}
  virtual void slotDeletingFile( KIO::Job*, const KURL& ) {}
  virtual void slotCreatingDir( KIO::Job*, const KURL& ) {}

  virtual void slotCanResume( KIO::Job*, bool ) {}

//     virtual void slotGettingFile( KIO::Job*, const KURL& ) {}

};


#endif // __kio_progressbase_h__
