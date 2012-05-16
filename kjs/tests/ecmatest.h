/*
 * Copyright (C) 2012 Rolf Eike Beer <kde@opensource.sf-tec.de>
 */

#ifndef ECMATEST_H
#define ECMATEST_H

#include <kio/global.h>
#include <qtest_kde.h>

#include <QtCore/QObject>
#include <QtCore/QString>

class ECMAscriptTest : public QObject {
  Q_OBJECT
private Q_SLOTS:
  void initTestCase();
  void runAllTests();
  void runAllTests_data();
};

#endif /* ECMATEST_H */
