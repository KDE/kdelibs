#ifndef __defaultprogress_h__
#define __defaultprogress_h__

#include <qlabel.h>

#include <kprogress.h>

#include "progressbase.h"

class DefaultProgress : public ProgressBase {

  Q_OBJECT

public:

  DefaultProgress();
  ~DefaultProgress() {}

protected slots:

  virtual void slotTotalSize( KIO::Job*, unsigned long _bytes );
  virtual void slotTotalFiles( KIO::Job*, unsigned long _files );
  virtual void slotTotalDirs( KIO::Job*, unsigned long _dirs );

  virtual void slotProcessedSize( KIO::Job*, unsigned long _bytes );
  virtual void slotProcessedFiles( KIO::Job*, unsigned long _files );
  virtual void slotProcessedDirs( KIO::Job*, unsigned long _dirs );

  virtual void slotSpeed( KIO::Job*, unsigned long _bytes_per_second );
  virtual void slotPercent( KIO::Job*, unsigned int _percent );

  virtual void slotCopyingFile( KIO::Job*, const KURL& _src, const KURL& _dest );
  virtual void slotMovingFile( KIO::Job*, const KURL& _src, const KURL& _dest );
  virtual void slotDeletingFile( KIO::Job*, const KURL& _file );
  virtual void slotCreatingDir( KIO::Job*, const KURL& _dir );

  virtual void slotCanResume( KIO::Job*, bool );

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

  enum ModeType { Copy, Move, Delete, Create };
  ModeType mode;
};

#endif // __defaultprogress_h__
