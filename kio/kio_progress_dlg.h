#ifndef __kio_progress_dlg_h__
#define __kio_progress_dlg_h__

#include <qdialog.h>
#include <qlabel.h>
#include <qlayout.h>

#include <kprogress.h>

class KIOJob;

class KIOCopyProgressDlg : public QDialog
{
  Q_OBJECT
public:
  KIOCopyProgressDlg( KIOJob* );

  void totalSize( unsigned long );
  void totalDirs( unsigned long );
  void totalFiles( unsigned long );
  void processedSize( unsigned long );
  void processedDirs( unsigned long );
  void processedFiles( unsigned long );
  void speed( unsigned long );
  void scanningDir( const char *_dir );
  void copyingFile( const char *_from, const char *_to );
  void makingDir( const char *_dir );
  // Get Command
  void gettingFile( const char *_url );
  
protected:
  QVBoxLayout *m_pLayout;
  QLabel* m_pLine1;
  QLabel* m_pLine2;
  QLabel* m_pLine3;
  QLabel* m_pLine4;
  KProgress* m_pProgressBar;

  unsigned long m_iTotalSize;
  unsigned long m_iTotalFiles;
  unsigned long m_iTotalDirs;
  unsigned long m_iProcessedSize;

  KIOJob* m_pJob;
};

#endif
