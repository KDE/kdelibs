#ifndef __KSPELLDLG_H__
#define __KSPELLDLG_H__

//#include "dialog1.h"
#include <qdialog.h>
#include <qpushbt.h>
#include <qbutton.h>
#include <qlabel.h>
#include <qlined.h>
#include <qlistbox.h>
#include <qlayout.h>


#include <klined.h>

//Possible result codes
#define KS_CANCEL      0 
#define KS_REPLACE     1
#define KS_REPLACEALL  2
#define KS_IGNORE      3
#define KS_IGNOREALL   4
#define KS_ADD         5

#define KS_STOP        7


class KSpellDlg : public QWidget
{
  Q_OBJECT;

  KLined *editbox;
  QListBox *listbox;
  QStrList *sugg;
  QString word;
  QString newword;
  QPushButton *qpbrep, *qpbrepa;
  QLabel *wordlabel;
  QList<QWidget> *children;
  QGridLayout *layout;

public:
  KSpellDlg::KSpellDlg (QWidget *parent, const char *name,
			char *ID);
  char *replacement (void)
    { return newword.data(); }

  void init (char *_word, QStrList *_sugg);
  void standby (void);

  ~KSpellDlg()
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
