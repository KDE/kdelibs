#ifndef __kio_rename_dlg__
#define __kio_rename_dlg__

#include <qwidget.h>
#include <qdialog.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qlayout.h>
#include <qstring.h>

#include <string>

enum RenameDlg_Mode { M_OVERWRITE = 1, M_OVERWRITE_ITSELF = 2, M_SKIP = 4, M_SINGLE = 8, M_MULTI = 16, M_RESUME = 32 };
enum RenameDlg_Result { R_RESUME = 6, R_RESUME_ALL = 7, R_OVERWRITE = 4, R_OVERWRITE_ALL = 5, R_SKIP = 2, R_AUTO_SKIP = 3, R_RENAME = 1, R_CANCEL = 0 };

class KIORenameDlg : public QDialog
{
  Q_OBJECT
public:
  KIORenameDlg( QWidget *parent, const char *_src, const char *_dest, RenameDlg_Mode _mode,  bool _modal = FALSE );
  ~KIORenameDlg();
    
  const char* newName() { return m_pLineEdit->text(); }

protected:
  QPushButton *b0, *b1, *b2, *b3, *b4, *b5, *b6, *b7;
  QLineEdit* m_pLineEdit;
  QVBoxLayout* m_pLayout;
  
  string src;
  string dest;
    
  bool modal;

public slots:
  void b0Pressed();
  void b1Pressed();
  void b2Pressed();
  void b3Pressed();
  void b4Pressed();
  void b5Pressed();
  void b6Pressed();
  void b7Pressed();

signals:
  void result( QWidget* _widget, int _button, const char* _src, const char* _data );
};

RenameDlg_Result open_RenameDlg( const char* _src, const char *_dest, RenameDlg_Mode _mode, QString& _new );

unsigned long getOffset( QString dest );


#endif

