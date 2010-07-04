/*
    Copyright (C) 2010 Klarälvdalens Datakonsult AB,
        a KDAB Group company, info@kdab.net,
        author Stephen Kelly <stephen@kdab.com>

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


#include "proxymodeltestsuite/proxymodeltest.h"
#include <kdescendantsproxymodel.h>


class TestData : public ProxyModelTestData
{
  Q_OBJECT
public:
  TestData(ProxyModelTest* parent = 0)
    : ProxyModelTestData(parent)
  {

  }

public slots:
  void testInsertWhenEmptyData()
  {
    QTest::addColumn<SignalList>("signalList");
    QTest::addColumn<PersistentChangeList>("changeList");

    static const IndexFinder indexFinder;

    newInsertTest("insert01", indexFinder, 0, 0, 0);
    newInsertTest("insert02", indexFinder, 0, 9, 0);

    // The test suite can't handle tree insertion yet, so we skip it for now.
    skipTestData("insert03");

//     processTestName("insert03");

//     SignalList signalList;
//     PersistentChangeList persistentList;

//     partialTest(&signalList, &persistentList, indexFinder, 0, 4, 0);
//     partialTest(&signalList, &persistentList, indexFinder, 1, 1, 5);
//     partialTest(&signalList, &persistentList, indexFinder, 4, 4, 6);
//     partialTest(&signalList, &persistentList, indexFinder, 7, 7, 7);
//     partialTest(&signalList, &persistentList, indexFinder, 5, 5, 8);

//     QTest::newRow("insert03") << signalList << persistentList;
  }

  void testInsertInRootData()
  {
    QTest::addColumn<SignalList>("signalList");
    QTest::addColumn<PersistentChangeList>("changeList");

    static const IndexFinder indexFinder;

    newInsertTest("insert01", indexFinder, 0, 0, 43);
    newInsertTest("insert02", indexFinder, 0, 9, 43);
    newInsertTest("insert03", indexFinder, 43, 43, 43);
    newInsertTest("insert04", indexFinder, 43, 52, 43);
    newInsertTest("insert05", indexFinder, 7, 7, 43);
    newInsertTest("insert06", indexFinder, 7, 16, 43);
    skipTestData("insert07");
    skipTestData("insert08");
    skipTestData("insert09");
    skipTestData("insert10");
    skipTestData("insert11");
    skipTestData("insert12");
    skipTestData("insert13");
    skipTestData("insert14");
    skipTestData("insert15");
    skipTestData("insert16");
    skipTestData("insert17");
    skipTestData("insert18");
  }

  void testInsertInTopLevelData()
  {
    QTest::addColumn<SignalList>("signalList");
    QTest::addColumn<PersistentChangeList>("changeList");

    static const IndexFinder indexFinder;

    newInsertTest("insert01", indexFinder, 9, 9, 43);
    newInsertTest("insert02", indexFinder, 9, 18, 43);
    newInsertTest("insert03", indexFinder, 37, 37, 43);
    newInsertTest("insert04", indexFinder, 37, 46, 43);
    newInsertTest("insert05", indexFinder, 15, 15, 43);
    newInsertTest("insert06", indexFinder, 15, 24, 43);
    skipTestData("insert07");
    skipTestData("insert08");
    skipTestData("insert09");
    skipTestData("insert10");
    skipTestData("insert11");
    skipTestData("insert12");
    skipTestData("insert13");
    skipTestData("insert14");
    skipTestData("insert15");
    skipTestData("insert16");
    skipTestData("insert17");
    skipTestData("insert18");
  }

  void testInsertInSecondLevelData()
  {
    QTest::addColumn<SignalList>("signalList");
    QTest::addColumn<PersistentChangeList>("changeList");

    static const IndexFinder indexFinder;

    newInsertTest("insert01", indexFinder, 17, 17, 43);
    newInsertTest("insert02", indexFinder, 17, 26, 43);
    newInsertTest("insert03", indexFinder, 32, 32, 43);
    newInsertTest("insert04", indexFinder, 32, 41, 43);
    newInsertTest("insert05", indexFinder, 23, 23, 43);
    newInsertTest("insert06", indexFinder, 23, 32, 43);
    skipTestData("insert07");
    skipTestData("insert08");
    skipTestData("insert09");
    skipTestData("insert10");
    skipTestData("insert11");
    skipTestData("insert12");
    skipTestData("insert13");
    skipTestData("insert14");
    skipTestData("insert15");
    skipTestData("insert16");
    skipTestData("insert17");
    skipTestData("insert18");
  }

  void testRemoveFromRootData()
  {
    QTest::addColumn<SignalList>("signalList");
    QTest::addColumn<PersistentChangeList>("changeList");

    static const IndexFinder indexFinder;

    newRemoveTest("remove01", indexFinder, 0, 0, 43);
    newRemoveTest("remove02", indexFinder, 0, 7, 43);
    newRemoveTest("remove03", indexFinder, 42, 42, 43);
  }

  void testRemoveFromTopLevelData()
  {
    QTest::addColumn<SignalList>("signalList");
    QTest::addColumn<PersistentChangeList>("changeList");

    static const IndexFinder indexFinder;

    newRemoveTest("remove01", indexFinder, 9, 9, 43);
    newRemoveTest("remove02", indexFinder, 9, 15, 43);
    newRemoveTest("remove03", indexFinder, 36, 36, 43);
  }

  void testRemoveFromSecondLevelData()
  {
    QTest::addColumn<SignalList>("signalList");
    QTest::addColumn<PersistentChangeList>("changeList");

    static const IndexFinder indexFinder;

    newRemoveTest("remove01", indexFinder, 17, 17, 43);
    newRemoveTest("remove02", indexFinder, 17, 23, 43);
    newRemoveTest("remove03", indexFinder, 31, 31, 43);
  }

  void testMoveFromRootData()
  {
    dummyTestData();
  }

  void testMoveFromTopLevelData()
  {
    dummyTestData();
  }

  void testMoveFromSecondLevelData()
  {
    dummyTestData();
  }

  void testModifyInRootData()
  {
    dummyTestData();
  }

  void testModifyInTopLevelData()
  {
    dummyTestData();
  }

  void testModifyInSecondLevelData()
  {
    dummyTestData();
  }

};

class DescendantsProxyModelTest : public ProxyModelTest
{
  Q_OBJECT
public:
  DescendantsProxyModelTest(QObject *parent = 0)
      : ProxyModelTest(parent)
  {
  }

  void setTestData(TestData *data)
  {
    kDebug() << data;
    connectTestSignals(data);
  }

protected:
  /* reimp */ QAbstractProxyModel* getProxy()
  {
    return new KDescendantsProxyModel(this);
  }

private slots:
  void cleanupTestCase()
  {
    doCleanupTestCase();
  }

  void cleanup()
  {
    doCleanup();
  }

private:
  KDescendantsProxyModel *m_proxyModel;
};

PROXYMODELTEST_MAIN(DescendantsProxyModelTest,
  PROXYMODELTEST_CUSTOM(new TestData, DynamicTree, ImmediatePersistence, "")
  PROXYMODELTEST_CUSTOM(new TestData, DynamicTree, LazyPersistence, "")
  PROXYMODELTEST_CUSTOM(new TestData, IntermediateProxy, ImmediatePersistence, "")
  PROXYMODELTEST_CUSTOM(new TestData, IntermediateProxy, LazyPersistence, "")
)

#include "kdescendantsproxymodeltest.moc"

