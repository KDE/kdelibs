// $Id$


#ifndef __kio_progressbase_h__
#define __kio_progressbase_h__


#include <qwidget.h>

class KIOJob;


/** 
* Base class for IO progress dialogs.
*
* This class does all initialization stuff for progress,
* like connecting signals to slots.
* All slots are implemented as pure virtual methods.
*
* All custom IO progress dialog should inherit this class.
* Add your GUI code to the constructor and implmement those virtual
* methods which you need in order to display progress.
*
* E.g. @ref #KIOLittleProgressDialog only implements @ref #slotTotalSize,
* @ref #slotPercent and @ref slotSpeed.
*
* Custom progress dialog will be used like this :
* <pre>
* // create a dialog
* MyCustomProgressDlg *customProgress;
* customProgress = new MyCustomProgressDlg();
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

  virtual void setJob( KIOJob * );
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

  KIOJob* m_pJob;

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

  virtual void slotTotalSize( int, unsigned long ) {}
  virtual void slotTotalFiles( int, unsigned long ) {}
  virtual void slotTotalDirs( int, unsigned long ) {}
  virtual void slotPercent( int, unsigned long ) {}
  virtual void slotProcessedSize( int, unsigned long ) {}
  virtual void slotProcessedFiles( int, unsigned long ) {}
  virtual void slotProcessedDirs( int, unsigned long ) {}
  virtual void slotScanningDir( int, const char* ) {}
  virtual void slotSpeed( int, unsigned long ) {}
  virtual void slotCopyingFile( int, const char*, const char* ) {}
  virtual void slotMakingDir( int, const char* ) {}
  virtual void slotGettingFile( int, const char* ) {}
  virtual void slotDeletingFile( int, const char* ) {}
  virtual void slotCanResume( int, bool ) {}

};


#endif // __kio_progressbase_h__
