#ifndef __KSPELLDLG_H__
#define __KSPELLDLG_H__

//#include "dialog1.h"
#include <qdialog.h>
#include <qpushbutton.h>
#include <qbutton.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qlistbox.h>
#include <qlayout.h>


#include <klined.h>

//Possible result codes
enum KS_RESULT {
  KS_CANCEL=     0,
  KS_REPLACE=    1,
  KS_REPLACEALL= 2,
  KS_IGNORE=     3,
  KS_IGNOREALL=  4,
  KS_ADD=        5,
  KS_STOP=       7
};

enum Geometries {buttonwidth=80, labelwidth=100};

class KSpellDlg : public QWidget
{
  Q_OBJECT

  KLined *editbox;
  QListBox *listbox;
  QStrList *sugg;
  QPushButton *qpbrep, *qpbrepa;
  QLabel *wordlabel;
  QList<QWidget> *children;
  QGridLayout *layout;
  QString word, newword;

public:
  KSpellDlg (QWidget *parent, const char *name,
			QString& ID);

  inline QString replacement (void)
    { return newword; }

  void init (const QString& _word, QStrList *_sugg);
  void standby (void);

  virtual ~KSpellDlg()
    {printf ("KSD killed\n");}

protected:
  void done (int i);

 signals:
    /**
      This signal is emitted when a button is pressed.
      */
  void command (int);

protected slots:
  void ignore();
  void add();
  void ignoreAll();  
  void cancel();
  void replace();
  void replaceAll();
  void stop();
  void help();

  void textChanged (const char *);

  void selected (int i);
  void highlighted (int i);

};

#endif
