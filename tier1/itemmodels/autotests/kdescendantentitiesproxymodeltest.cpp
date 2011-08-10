/*
    Copyright (c) 2009 Stephen Kelly <steveire@gmail.com>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#include <QtTest>
#include <QtCore>
#include <qtest_kde.h>
#include <qtestevent.h>

#include "dynamictreemodel.h"
#include "kdescendantsproxymodel.h"
#include "proxymodeltest.h"

#include <QTreeView>

#include <kdebug.h>

class DescendantsProxyModelTest : public ProxyModelTest
{
  Q_OBJECT
public:
  DescendantsProxyModelTest( QObject *parent = 0 )
      : ProxyModelTest( parent )
  {
  }

protected:
  virtual void testData();

private slots:
  void init()
  {
    ProxyModelTest::doInit();
  }

  void initTestCase();
  void cleanupTestCase();

private:
  KDescendantsProxyModel *m_proxyModel;
  IndexFinder m_rootIdxFinder;
  int m_rowCount;
};

void DescendantsProxyModelTest::testData()
{

  QTest::addColumn<SignalList>("signalList");
  QTest::addColumn<PersistentChangeList>("changeList");

  CommandList commandList;
  SignalList signalList;
  PersistentChangeList persistentList;
  IndexFinder indexFinder;

  // This selection will not cause any signals to be emitted. Only inserting
  // child indexes into it will.

  signalList << getSignal(RowsAboutToBeInserted, indexFinder, 0, 0);
  signalList << getSignal(RowsInserted, indexFinder, 0, 0);

  QTest::newRow("insert01") << signalList << persistentList;
  signalList.clear();

  signalList << getSignal(RowsAboutToBeInserted, indexFinder, 1, 10);
  signalList << getSignal(RowsInserted, indexFinder, 1, 10);

  QTest::newRow("insert02") << signalList << persistentList;
  signalList.clear();

  signalList << getSignal(RowsAboutToBeInserted, indexFinder, 0, 9);
  signalList << getSignal(RowsInserted, indexFinder, 0, 9);

  persistentList << getChange( indexFinder, 0, 10, 10 );

  QTest::newRow("insert03") << signalList << persistentList;
  signalList.clear();
  persistentList.clear();

  signalList << (QVariantList() << LayoutAboutToBeChanged);
  signalList << (QVariantList() << LayoutChanged);

  persistentList << getChange( indexFinder, 11, 11, 4 );
  persistentList << getChange( indexFinder, 12, 15, -1 );

  QTest::newRow("move01") << signalList << persistentList;
  signalList.clear();
  persistentList.clear();

  signalList << (QVariantList() << LayoutAboutToBeChanged);
  signalList << (QVariantList() << LayoutChanged);

  persistentList << getChange( indexFinder, 15, 15, -4 );
  persistentList << getChange( indexFinder, 11, 14, 1 );

  QTest::newRow("move02") << signalList << persistentList;
  signalList.clear();
  persistentList.clear();

  signalList << (QVariantList() << LayoutAboutToBeChanged);
  signalList << (QVariantList() << LayoutChanged);

  persistentList << getChange( indexFinder, 0, 0, 15 );
  persistentList << getChange( indexFinder, 1, 15, -1 );

  QTest::newRow("move03") << signalList << persistentList;
  signalList.clear();
  persistentList.clear();

  signalList << (QVariantList() << LayoutAboutToBeChanged);
  signalList << (QVariantList() << LayoutChanged);

  persistentList << getChange( indexFinder, 15, 15, -15 );
  persistentList << getChange( indexFinder, 0, 14, 1 );

  QTest::newRow("move04") << signalList << persistentList;
  signalList.clear();
  persistentList.clear();

  signalList << (QVariantList() << LayoutAboutToBeChanged);
  signalList << (QVariantList() << LayoutChanged);

  persistentList << getChange( indexFinder, 4, 4, 6 );
  persistentList << getChange( indexFinder, 5, 10, -1 );

  QTest::newRow("move05") << signalList << persistentList;
  signalList.clear();
  persistentList.clear();

  signalList << (QVariantList() << LayoutAboutToBeChanged);
  signalList << (QVariantList() << LayoutChanged);

  persistentList << getChange( indexFinder, 10, 10, -6 );
  persistentList << getChange( indexFinder, 4, 9, 1 );

  QTest::newRow("move06") << signalList << persistentList;
  signalList.clear();
  persistentList.clear();

  signalList << getSignal(RowsAboutToBeInserted, indexFinder, 21, 30);
  signalList << getSignal(RowsInserted, indexFinder, 21, 30);

  QTest::newRow("insert04") << signalList << persistentList;
  signalList.clear();
  persistentList.clear();

  signalList << getSignal(RowsAboutToBeInserted, indexFinder, 11, 20);
  signalList << getSignal(RowsInserted, indexFinder, 11, 20);

  persistentList << getChange( indexFinder, 11, 30, 10 );

  QTest::newRow("insert05") << signalList << persistentList;
  signalList.clear();
  persistentList.clear();

  signalList << getSignal(RowsAboutToBeInserted, indexFinder, 31, 40);
  signalList << getSignal(RowsInserted, indexFinder, 31, 40);

  persistentList << getChange( indexFinder, 31, 40, 10 );

  QTest::newRow("insert06") << signalList << persistentList;
  signalList.clear();
  persistentList.clear();

  signalList << getSignal(RowsAboutToBeInserted, indexFinder, 21, 30);
  signalList << getSignal(RowsInserted, indexFinder, 21, 30);

  persistentList << getChange( indexFinder, 21, 50, 10 );

  QTest::newRow("insert07") << signalList << persistentList;
  signalList.clear();
  persistentList.clear();

  // When this proxy recieves a rowsAboutToBeInserted signal, it can't know
  // how many rows need to be inserted (total descendants).
  // So, it first inserts only the rows signaled by the source model (and not
  // the descendants). When the source model signals rowsInserted, we can
  // examine the new rows for descendants. These need to be signalled separately
  // by this proxy

  signalList << getSignal(RowsAboutToBeInserted, indexFinder, 17, 26);
  signalList << getSignal(RowsInserted, indexFinder, 17, 26);

  signalList << getSignal(RowsAboutToBeInserted, indexFinder, 23, 32);
  signalList << getSignal(RowsInserted, indexFinder, 23, 32);

  signalList << getSignal(RowsAboutToBeInserted, indexFinder, 29, 38);
  signalList << getSignal(RowsInserted, indexFinder, 29, 38);

  persistentList << getChange( indexFinder, 17, 17, 30 );

  signalList << getSignal(RowsAboutToBeInserted, indexFinder, 48, 57);
  signalList << getSignal(RowsInserted, indexFinder, 48, 57);

  persistentList << getChange( indexFinder, 18, 18, 40 );

  signalList << getSignal(RowsAboutToBeInserted, indexFinder, 59, 68);
  signalList << getSignal(RowsInserted, indexFinder, 59, 68);

  persistentList << getChange( indexFinder, 19, 60, 50 );

  QTest::newRow("insert08") << signalList << persistentList;
  signalList.clear();
  persistentList.clear();

  signalList << getSignal(RowsAboutToBeInserted, indexFinder, 14, 23);
  signalList << getSignal(RowsInserted, indexFinder, 14, 23);

  signalList << getSignal(RowsAboutToBeInserted, indexFinder, 17, 36);
  signalList << getSignal(RowsInserted, indexFinder, 17, 36);

  signalList << getSignal(RowsAboutToBeInserted, indexFinder, 40, 59);
  signalList << getSignal(RowsInserted, indexFinder, 40, 59);

  persistentList << getChange(indexFinder, 14, 110, 50);

  QTest::newRow("insert09") << signalList << persistentList;
  signalList.clear();
  persistentList.clear();

  IndexFinder topLeftFinder = IndexFinder(m_proxyModel, QList<int>() << 11 );
  IndexFinder bottomRightFinder = IndexFinder(m_proxyModel, QList<int>() << 11 );

  signalList << ( QVariantList() << DataChanged << QVariant::fromValue(topLeftFinder) << QVariant::fromValue(bottomRightFinder) );

  QTest::newRow("change01") << signalList << persistentList;
  signalList.clear();
  persistentList.clear();


  // Although the source model emits only one range is changed, this proxy model puts children indexes
  // in the way, breaking the continuous range.
  // Currently separate signals are emitted for each changed row.
  // This should really emit one signal for each continuous range instead. That's a TODO.

  topLeftFinder = IndexFinder(m_proxyModel, QList<int>() << 65 );
  bottomRightFinder = IndexFinder(m_proxyModel, QList<int>() << 65 );
  signalList << ( QVariantList() << DataChanged << QVariant::fromValue(topLeftFinder) << QVariant::fromValue(bottomRightFinder) );

  topLeftFinder = IndexFinder(m_proxyModel, QList<int>() << 66 );
  bottomRightFinder = IndexFinder(m_proxyModel, QList<int>() << 66 );
  signalList << ( QVariantList() << DataChanged << QVariant::fromValue(topLeftFinder) << QVariant::fromValue(bottomRightFinder) );

  topLeftFinder = IndexFinder(m_proxyModel, QList<int>() << 97 );
  bottomRightFinder = IndexFinder(m_proxyModel, QList<int>() << 97 );
  signalList << ( QVariantList() << DataChanged << QVariant::fromValue(topLeftFinder) << QVariant::fromValue(bottomRightFinder) );

  topLeftFinder = IndexFinder(m_proxyModel, QList<int>() << 108 );
  bottomRightFinder = IndexFinder(m_proxyModel, QList<int>() << 108 );
  signalList << ( QVariantList() << DataChanged << QVariant::fromValue(topLeftFinder) << QVariant::fromValue(bottomRightFinder) );

  QTest::newRow("change02") << signalList << persistentList;
  signalList.clear();
  persistentList.clear();

  signalList << getSignal(RowsAboutToBeRemoved, indexFinder, 11, 11);
  signalList << getSignal(RowsRemoved, indexFinder, 11, 11);

  persistentList << getChange( indexFinder, 11, 11, -1, true );
  persistentList << getChange( indexFinder, 12, 160, -1);

  QTest::newRow("remove01") << signalList << persistentList;
  signalList.clear();
  persistentList.clear();

  signalList << getSignal(RowsAboutToBeRemoved, indexFinder, 107, 117);
  signalList << getSignal(RowsRemoved, indexFinder, 107, 117);

  persistentList << getChange( indexFinder, 107, 117, -1, true );
  persistentList << getChange( indexFinder, 118, 159, -11);

  QTest::newRow("remove02") << signalList << persistentList;
  signalList.clear();
  persistentList.clear();

  signalList << getSignal(RowsAboutToBeRemoved, indexFinder, 97, 97);
  signalList << getSignal(RowsRemoved, indexFinder, 97, 97);

  persistentList << getChange( indexFinder, 97, 97, -1, true );
  persistentList << getChange( indexFinder, 98, 148, -1);

  QTest::newRow("remove03") << signalList << persistentList;
  signalList.clear();
  persistentList.clear();

  signalList << getSignal(RowsAboutToBeRemoved, indexFinder, 105, 105);
  signalList << getSignal(RowsRemoved, indexFinder, 105, 105);

  persistentList << getChange( indexFinder, 105, 105, -1, true );
  persistentList << getChange( indexFinder, 106, 147, -1);

  QTest::newRow("remove04") << signalList << persistentList;
  signalList.clear();
  persistentList.clear();

  signalList << getSignal(RowsAboutToBeRemoved, indexFinder, 100, 100);
  signalList << getSignal(RowsRemoved, indexFinder, 100, 100);

  persistentList << getChange( indexFinder, 100, 100, -1, true );
  persistentList << getChange( indexFinder, 101, 146, -1);

  QTest::newRow("remove05") << signalList << persistentList;
  signalList.clear();
  persistentList.clear();

  signalList << getSignal(RowsAboutToBeRemoved, indexFinder, 97, 103);
  signalList << getSignal(RowsRemoved, indexFinder, 97, 103);

  persistentList << getChange( indexFinder, 97, 103, -1, true );
  persistentList << getChange( indexFinder, 104, 145, -7);

  QTest::newRow("remove06") << signalList << persistentList;
  signalList.clear();
  persistentList.clear();

  signalList << getSignal(RowsAboutToBeRemoved, indexFinder, 65, 95);
  signalList << getSignal(RowsRemoved, indexFinder, 65, 95);

  persistentList << getChange( indexFinder, 65, 95, -1, true );
  persistentList << getChange( indexFinder, 96, 138, -31);

  QTest::newRow("remove07") << signalList << persistentList;
  signalList.clear();
  persistentList.clear();
}


void DescendantsProxyModelTest::initTestCase()
{
  m_proxyModel = new KDescendantsProxyModel(this);
  setProxyModel(m_proxyModel);
  ProxyModelTest::doInitTestCase();
}


void DescendantsProxyModelTest::cleanupTestCase()
{
  ProxyModelTest::doCleanupTestCase();
}


QTEST_KDEMAIN_CORE(DescendantsProxyModelTest)
#include "kdescendantentitiesproxymodeltest.moc"
