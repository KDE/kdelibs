#ifndef _KIOSLAVETEST_H
#define _KIOSLAVETEST_H

#include <qwidget.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qradiobutton.h>
#include <qpushbutton.h>
#include <qbuttongroup.h>

#include "kio_job.h"

class KioslaveTest : public QWidget {
  Q_OBJECT

public:
  KioslaveTest();
  ~KioslaveTest();

  enum Operations { List, Get, Copy, Move, Delete };

  enum ProgressModes { ProgressNone, ProgressSimple,
		       ProgressList, ProgressLittle };

protected:

  // info stuff
  QLabel *lb_from;
  QLineEdit *le_source;

  QLabel *lb_to;
  QLineEdit *le_dest;

  // operation stuff
  QButtonGroup *opButtons;

  QRadioButton *rbList;
  QRadioButton *rbGet;
  QRadioButton *rbCopy;
  QRadioButton *rbMove;
  QRadioButton *rbDelete;

  // progress stuff
  QButtonGroup *progressButtons;

  QRadioButton *rbProgressNone;
  QRadioButton *rbProgressSimple;
  QRadioButton *rbProgressList;
  QRadioButton *rbProgressLittle;

  QPushButton *pbStart;
  QPushButton *pbStop;

  QPushButton *close;

protected slots:
  void changeOperation( int id );
  void changeProgressMode( int id );

  void startJob();
  void stopJob();

  void slotError( int, int errid, const char* errortext );
  void slotFinished( int );

private:
  KIOJob *job;

  int selectedOperation;
  int progressMode;
};

#endif // _KIOSLAVETEST_H
