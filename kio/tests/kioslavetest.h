 /*
  This file is or will be part of KDE desktop environment

  Copyright 1999 Matt Koss <koss@miesto.sk>

  It is licensed under GPL version 2.

  If it is part of KDE libraries than this file is licensed under
  LGPL version 2.
 */

#ifndef _KIOSLAVETEST_H
#define _KIOSLAVETEST_H

#include <qlabel.h>
#include <qlineedit.h>
#include <qradiobutton.h>
#include <qpushbutton.h>
#include <qbuttongroup.h>
#include <qwidget.h>

#include <ktmainwindow.h>

#include "kio_littleprogress_dlg.h"
#include "kio_job.h"

class KioslaveTest : public KTMainWindow {
  Q_OBJECT

public:
  KioslaveTest( QString src, QString dest, uint op, uint pr );
  ~KioslaveTest() {}

  enum Operations { List, Get, Copy, Move, Delete };

  enum ProgressModes { ProgressNone, ProgressSimple,
		       ProgressList, ProgressLittle };

protected:

  void closeEvent( QCloseEvent * );

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
  void slotFinished();

private:
  KIOJob *job;
  QWidget *main_widget;

  KIOLittleProgressDlg *littleProgress;

  int selectedOperation;
  int progressMode;
};

#endif // _KIOSLAVETEST_H
