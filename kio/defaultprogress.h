#ifndef __kio_simpleprogress_dlg_h__
#define __kio_simpleprogress_dlg_h__

#include <qlabel.h>

#include <kprogress.h>

#include "progressbase.h"

class DefaultProgress : public KIOProgressBase {

  Q_OBJECT

public:

  DefaultProgress();
  ~DefaultProgress() {}

protected slots:

  virtual void slotTotalSize( int, unsigned long _bytes );
  virtual void slotTotalFiles( int, unsigned long _files );
  virtual void slotTotalDirs( int, unsigned long _dirs );
  virtual void slotPercent( int, unsigned long _bytes );
  virtual void slotProcessedSize( int, unsigned long _bytes );
  virtual void slotProcessedFiles( int, unsigned long _files );
  virtual void slotProcessedDirs( int, unsigned long _dirs );
  virtual void slotScanningDir( int, const char *_dir );
  virtual void slotSpeed( int, unsigned long _bytes_per_second );
  virtual void slotCopyingFile( int, const char *_from, const char *_to );
  virtual void slotMakingDir( int, const char *_dir );
  virtual void slotGettingFile( int, const char *_url );
  virtual void slotDeletingFile( int, const char *_url );
  virtual void slotCanResume( int, bool );

protected:

  QLabel* progressLabel;
  QLabel* sourceLabel;
  QLabel* destLabel;
  QLabel* speedLabel;
  QLabel* sizeLabel;
  QLabel* resumeLabel;

  KProgress* m_pProgressBar;

  unsigned long m_iTotalSize;
  unsigned long m_iTotalFiles;
  unsigned long m_iTotalDirs;

  unsigned long m_iProcessedDirs;
  unsigned long m_iProcessedFiles;

  enum ModeType { Copy, Delete, Create, Scan, Fetch };
  ModeType mode;
};

#endif
