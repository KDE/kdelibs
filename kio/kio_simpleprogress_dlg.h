#ifndef __kio_simpleprogress_dlg_h__
#define __kio_simpleprogress_dlg_h__

#include <qdialog.h>
#include <qlabel.h>
#include <qlayout.h>

#include <kprogress.h>

class KIOJob;

class KIOSimpleProgressDlg : public QDialog {

  Q_OBJECT

public:
  KIOSimpleProgressDlg( KIOJob*, bool m_bStartIconified = false );
  ~KIOSimpleProgressDlg() {}

public slots:

  void slotTotalSize( int, unsigned long _bytes );
  void slotTotalFiles( int, unsigned long _files );
  void slotTotalDirs( int, unsigned long _dirs );
  void slotPercent( int, unsigned long _bytes );
  void slotProcessedSize( int, unsigned long _bytes );
  void slotProcessedFiles( int, unsigned long _files );
  void slotProcessedDirs( int, unsigned long _dirs );
  void slotScanningDir( int, const char *_dir );
  void slotSpeed( int, unsigned long _bytes_per_second );
  void slotCopyingFile( int, const char *_from, const char *_to );
  void slotMakingDir( int, const char *_dir );
  void slotGettingFile( int, const char *_url );
  void slotDeletingFile( int, const char *_url );
  void slotCanResume( int, bool );

protected slots:
  virtual void done ( int r );

protected:
  QVBoxLayout *m_pLayout;
  QLabel* m_pLine1;
  QLabel* m_pLine2;
  QLabel* m_pLine3;
  QLabel* m_pLine4;
  QLabel* m_pLine5;
  QLabel* m_pLine6;
  KProgress* m_pProgressBar;

  unsigned long m_iTotalSize;
  unsigned long m_iTotalFiles;
  unsigned long m_iTotalDirs;

  KIOJob* m_pJob;
};

#endif
