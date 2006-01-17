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

#include <kmainwindow.h>

#include "kio/job.h"
#include "kio/global.h"
#include "kio/statusbarprogress.h"
#include "kio/slave.h"

class KioslaveTest : public KMainWindow {
  Q_OBJECT

public:
  KioslaveTest( QString src, QString dest, uint op, uint pr );
  ~KioslaveTest() {}

  enum Operations { List = 0, ListRecursive, Stat, Get, Put, Copy, Move, Delete, Shred, Mkdir, Mimetype };

  enum ProgressModes { ProgressNone = 0, ProgressDefault, ProgressStatus };

protected:

  void closeEvent( QCloseEvent * );

  void printUDSEntry( const KIO::UDSEntry & entry );

  // info stuff
  QLabel *lb_from;
  QLineEdit *le_source;

  QLabel *lb_to;
  QLineEdit *le_dest;

  // operation stuff
  QButtonGroup *opButtons;

  QRadioButton *rbList;
  QRadioButton *rbListRecursive;
  QRadioButton *rbStat;
  QRadioButton *rbGet;
  QRadioButton *rbPut;
  QRadioButton *rbCopy;
  QRadioButton *rbMove;
  QRadioButton *rbDelete;
  QRadioButton *rbShred;
  QRadioButton *rbMkdir;
  QRadioButton *rbMimetype;

  // progress stuff
  QButtonGroup *progressButtons;

  QRadioButton *rbProgressNone;
  QRadioButton *rbProgressDefault;
  QRadioButton *rbProgressStatus;

  QPushButton *pbStart;
  QPushButton *pbStop;

  QPushButton *close;

protected Q_SLOTS:
  void changeOperation( QAbstractButton *b );
  void changeProgressMode( QAbstractButton *b );

  void startJob();
  void stopJob();

  void slotResult( KIO::Job * );
  void slotEntries( KIO::Job *, const KIO::UDSEntryList& );
  void slotData( KIO::Job *, const QByteArray &data );
  void slotDataReq( KIO::Job *, QByteArray &data );

  void slotQuit();
  void slotSlaveConnected();
  void slotSlaveError();

private:
  KIO::Job *job;
  QWidget *main_widget;

  KIO::StatusbarProgress *statusProgress;

  int selectedOperation;
  int progressMode;
  int putBuffer;
  KIO::Slave *slave;
};

#endif // _KIOSLAVETEST_H
