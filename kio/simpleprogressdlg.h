#ifndef __kio_simpleprogress_dlg_h__
#define __kio_simpleprogress_dlg_h__

#include <qdialog.h>
#include <qlabel.h>
#include <qlayout.h>

#include <kprogress.h>

class KIOJob;

class KIOSimpleProgressDlg : public QDialog
{
  Q_OBJECT
public:
  KIOSimpleProgressDlg( KIOJob*, bool m_bStartIconified = false );
  ~KIOSimpleProgressDlg() {}

  void processedSize();
  void processedDirs();
  void processedFiles();
  void speed();
  void scanningDir();
  void copyingFile();
  void makingDir();
  void gettingFile();
  void deletingFile();
  
protected slots:
  virtual void done ( int r );

protected:
  QVBoxLayout *m_pLayout;
  QLabel* m_pLine1;
  QLabel* m_pLine2;
  QLabel* m_pLine3;
  QLabel* m_pLine4;
  QLabel* m_pLine5;
  KProgress* m_pProgressBar;

  int m_iPercent;

  KIOJob* m_pJob;
};

#endif
