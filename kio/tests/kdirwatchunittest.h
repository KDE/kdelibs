 /*
  This file is or will be part of KDE desktop environment

  Copyright 1998 Sven Radej <sven@lisa.exp.univie.ac.at>

  It is licensed under GPL version 2.

  If it is part of KDE libraries than this file is licensed under
  LGPL version 2.
 */

#ifndef _KDIRWATCHTEST_H_
#define _KDIRWATCHTEST_H_

#include <stdlib.h>
#include <stdio.h>
#include <qobject.h>

#include "kdirwatch.h"
#include "kapplication.h"

#include <kunittest/tester.h>
#include <kunittest/module.h>

class KDirWatchTest : public KUnitTest::Tester
{
  Q_OBJECT

public:
  enum { sigDirty, sigCreated, sigDeleted };

  KDirWatchTest()
  {
    m_signals[sigDirty] = m_signals[sigCreated] = m_signals[sigDeleted] = 0;
    m_workingDir = "/tmp/kdirwatchtest";
  }

  virtual void allTests();

private slots:
  void slotDirty (const QString& s) { m_signals[sigDirty]++; m_lastSignal = s; }
  void slotCreated (const QString& s) { m_signals[sigCreated]++; m_lastSignal = s; }
  void slotDeleted (const QString& s) { m_signals[sigDeleted]++; m_lastSignal = s; }

private:
  unsigned m_signals[3];

  /* verify nothing happens */
  void VERIFY_NOTHING();
  /* verify that dirty got emitted */
  void VERIFY_DIRTY (const QString&);
  /* verify that created got emitted */
  void VERIFY_CREATED (const QString&);
  /* verify that deleted got emitted */
  void VERIFY_DELETED (const QString&);

  QString  m_lastSignal;
  QString m_workingDir;
  KDirWatch* d;
};

#endif
