#ifndef __KSPELLDLG_H__
#define __KSPELLDLG_H__

//#include "dialog1.h"
#include <qdialog.h>
#include <qpushbutton.h>
#include <qbutton.h>
#include <qlabel.h>
#include <qlistbox.h>
#include <qlayout.h>

#include <klined.h>
#include <kprogress.h>

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

  KLineEdit *editbox;
  KProgress *progbar;
  QListBox *listbox;
  QStrList *sugg;
  QPushButton *qpbrep, *qpbrepa;
  QLabel *wordlabel;
  QList<QWidget> *children;
  QGridLayout *layout;
  QString word, newword;
  bool progressbar;

public:
  KSpellDlg (QWidget *parent, const char *name,
	     bool _progressbar = FALSE, bool _modal = FALSE );

  inline QString replacement (void)
    { return newword; }

  /**
   * Change the misspelled word and suggested replacements
   *  and enable the disabled buttons on the dialog box.
   * (Buttons are disabled by @ref standby().)
   **/
  void init (const QString& _word, QStrList *_sugg);

  /**
   * Disable some buttons and gray out the misspelled word.
   **/
  void standby (void);


  public slots:
  /**
   * Adjust the progress bar to <I>p</I> percent.
   **/
  void slotProgress (unsigned int p);

protected:
	virtual void closeEvent ( QCloseEvent * e );
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

  void textChanged (const QString &);

  void selected (int i);
  void highlighted (int i);


};

#endif
