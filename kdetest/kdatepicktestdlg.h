#ifndef _MAINDLG_H
#define _MAINDLG_H

#include <qdialog.h>

class QLabel;
class QDate;

class CMainDlg : public QDialog {
   Q_OBJECT
 public:
   CMainDlg(QWidget *parent=0, const char *name=0);

 private:
   QLabel *m_lab;

 private slots:
   void about();

 public slots:
   void quitapp();
   void printDate(QDate dt);
};

#endif // _MAINDLG_H
