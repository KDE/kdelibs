// $Id$


#ifndef __kio_progressbase_h__
#define __kio_progressbase_h__


#include <qwidget.h>

class KIOJob;


class KIOProgressBase : public QWidget {

  Q_OBJECT

public:

  KIOProgressBase( QWidget *parent = 0L );
  ~KIOProgressBase() {}

  virtual void setJob( KIOJob * );
  virtual void clean() {}

  void iconify( bool );
  void refill();

  bool onlyClean() { return m_bOnlyClean; }
  void setStopOnClose( bool stop ) { m_bStopOnClose = stop; }

protected:

  void closeEvent( QCloseEvent * );
  void Connect();

  KIOJob* m_pJob;
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

  void stop();
};


#endif // __kio_progressbase_h__
