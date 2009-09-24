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

#include "selectionproxymodeltest.h"

#include <QTimer>

#include "../itemviews/kselectionproxymodel.h"

#include "modelselector.h"

class SelectedBranchesTest : public SelectionProxyModelTest
{
  Q_OBJECT
public:
  SelectedBranchesTest(QObject *parent = 0)
      : SelectionProxyModelTest( parent )
  {
  }

protected:
  virtual void testData()
  {
    QTest::addColumn<SignalList>("signalList");
    QTest::addColumn<PersistentChangeList>("changeList");

    CommandList commandList;
    SignalList signalList;
    PersistentChangeList persistentList;
    IndexFinder indexFinder;

    signalList << getSignal(RowsAboutToBeInserted, indexFinder, 0, 0);
    signalList << getSignal(RowsInserted, indexFinder, 0, 0);

    QTest::newRow("insert01") << signalList << persistentList;
    signalList.clear();

    indexFinder = IndexFinder( m_proxyModel, QList<int>() << 0);

    signalList << getSignal(RowsAboutToBeInserted, indexFinder, 0, 9);
    signalList << getSignal(RowsInserted, indexFinder, 0, 9);

    QTest::newRow("insert02") << signalList << persistentList;
    signalList.clear();

    QTest::newRow("insert03") << signalList << persistentList;
    signalList.clear();

    signalList << (QVariantList() << LayoutAboutToBeChanged);
    signalList << (QVariantList() << LayoutChanged);

    persistentList << getChange( indexFinder, 0, 0, 4 );
    persistentList << getChange( indexFinder, 1, 4, -1 );

    QTest::newRow("move01") << signalList << persistentList;
    signalList.clear();
    persistentList.clear();

    signalList << (QVariantList() << LayoutAboutToBeChanged);
    signalList << (QVariantList() << LayoutChanged);

    persistentList << getChange( indexFinder, 4, 4, -4 );
    persistentList << getChange( indexFinder, 0, 3, 1 );

    QTest::newRow("move02") << signalList << persistentList;
    signalList.clear();
    persistentList.clear();

    signalList << (QVariantList() << LayoutAboutToBeChanged);
    signalList << (QVariantList() << LayoutChanged);

    persistentList << getChange( indexFinder, 5, 9, 1 );

    QTest::newRow("move03") << signalList << persistentList;
    signalList.clear();
    persistentList.clear();

    signalList << (QVariantList() << LayoutAboutToBeChanged);
    signalList << (QVariantList() << LayoutChanged);

    persistentList << getChange( indexFinder, 5, 5, -1, true );
    persistentList << getChange( indexFinder, 6, 10, -1 );

    QTest::newRow("move04") << signalList << persistentList;
    signalList.clear();
    persistentList.clear();

    signalList << (QVariantList() << LayoutAboutToBeChanged);
    signalList << (QVariantList() << LayoutChanged);

    persistentList << getChange( indexFinder, 0, 9, 1 );

    QTest::newRow("move05") << signalList << persistentList;
    signalList.clear();
    persistentList.clear();

    signalList << (QVariantList() << LayoutAboutToBeChanged);
    signalList << (QVariantList() << LayoutChanged);

    persistentList << getChange( indexFinder, 0, 0, -1, true );
    persistentList << getChange( indexFinder, 1, 10, -1 );

    QTest::newRow("move06") << signalList << persistentList;
    signalList.clear();
    persistentList.clear();

    QTest::newRow("insert04") << signalList << persistentList;
    signalList.clear();

    indexFinder = IndexFinder( m_proxyModel, QList<int>() << 0);

    signalList << getSignal(RowsAboutToBeInserted, indexFinder, 0, 9);
    signalList << getSignal(RowsInserted, indexFinder, 0, 9);

    // When rows are inserted, the rows from startRow to rowCount() -1 (ie, the last row)
    // will be moved down.
    persistentList << getChange(indexFinder, 0, 9, 10);

    QTest::newRow("insert05") << signalList << persistentList;
    signalList.clear();
    persistentList.clear();

    indexFinder = IndexFinder( m_proxyModel, QList<int>() << 0);

    signalList << getSignal(RowsAboutToBeInserted, indexFinder, 20, 29);
    signalList << getSignal(RowsInserted, indexFinder, 20, 29);

    QTest::newRow("insert06") << signalList << persistentList;
    signalList.clear();
    persistentList.clear();

    indexFinder = IndexFinder( m_proxyModel, QList<int>() << 0);

    signalList << getSignal(RowsAboutToBeInserted, indexFinder, 10, 19);
    signalList << getSignal(RowsInserted, indexFinder, 10, 19);

    persistentList << getChange(indexFinder, 10, 29, 10);

    QTest::newRow("insert07") << signalList << persistentList;
    signalList.clear();
    persistentList.clear();

    indexFinder = IndexFinder(m_proxyModel, QList<int>() << 0 << 5);

    signalList << getSignal(RowsAboutToBeInserted, indexFinder, 0, 9);
    signalList << getSignal(RowsInserted, indexFinder, 0, 9);

    indexFinder = IndexFinder(m_proxyModel, QList<int>() << 0 << 5 << 5);

    signalList << getSignal(RowsAboutToBeInserted, indexFinder, 0, 9);
    signalList << getSignal(RowsInserted, indexFinder, 0, 9);

    indexFinder = IndexFinder(m_proxyModel, QList<int>() << 0 << 5 << 5 << 5);

    signalList << getSignal(RowsAboutToBeInserted, indexFinder, 0, 9);
    signalList << getSignal(RowsInserted, indexFinder, 0, 9);

    indexFinder = IndexFinder(m_proxyModel, QList<int>() << 0 << 6);

    signalList << getSignal(RowsAboutToBeInserted, indexFinder, 0, 9);
    signalList << getSignal(RowsInserted, indexFinder, 0, 9);

    indexFinder = IndexFinder(m_proxyModel, QList<int>() << 0 << 7);

    signalList << getSignal(RowsAboutToBeInserted, indexFinder, 0, 9);
    signalList << getSignal(RowsInserted, indexFinder, 0, 9);

    QTest::newRow("insert08") << signalList << persistentList;
    signalList.clear();
    persistentList.clear();

    indexFinder = IndexFinder(m_proxyModel, QList<int>() << 0 << 2);

    // Although a tree of items is inserted, only ten base items are relevant to the model.
    signalList << getSignal(RowsAboutToBeInserted, indexFinder, 0, 9);
    signalList << getSignal(RowsInserted, indexFinder, 0, 9);

    QTest::newRow("insert09") << signalList << persistentList;
    signalList.clear();
    persistentList.clear();

    IndexFinder topLeftFinder = IndexFinder(m_proxyModel, QList<int>() << 0 << 0 );
    IndexFinder bottomRightFinder = IndexFinder(m_proxyModel, QList<int>() << 0 << 0 );

    signalList << ( QVariantList() << DataChanged << QVariant::fromValue(topLeftFinder) << QVariant::fromValue(bottomRightFinder) );

    QTest::newRow("change01") << signalList << persistentList;
    signalList.clear();
    persistentList.clear();

    topLeftFinder = IndexFinder(m_proxyModel, QList<int>() << 0 << 4 );
    bottomRightFinder = IndexFinder(m_proxyModel, QList<int>() << 0 << 7 );

    signalList << ( QVariantList() << DataChanged << QVariant::fromValue(topLeftFinder) << QVariant::fromValue(bottomRightFinder) );

    QTest::newRow("change02") << signalList << persistentList;
    signalList.clear();
    persistentList.clear();

    indexFinder = IndexFinder(m_proxyModel, QList<int>() << 0 );

    signalList << getSignal(RowsAboutToBeRemoved, indexFinder, 0, 0 );
    signalList << getSignal(RowsRemoved, indexFinder, 0, 0 );

    // The removed indexes go invalid.
    persistentList << getChange(indexFinder, 0, 0, -1, true);

    // Rows after it will be moved up.
    persistentList << getChange(indexFinder, 1, 39, -1 );

    QTest::newRow("remove01") << signalList << persistentList;
    signalList.clear();
    persistentList.clear();

    signalList << getSignal(RowsAboutToBeRemoved, indexFinder, 6, 6 );
    signalList << getSignal(RowsRemoved, indexFinder, 6, 6 );

    persistentList << getChange(indexFinder, 6, 6, -1, true);

    persistentList << getChange(indexFinder, 7, 38, -1 );

    QTest::newRow("remove02") << signalList << persistentList;
    signalList.clear();
    persistentList.clear();

    indexFinder = IndexFinder(m_proxyModel, QList<int>() << 0 << 5);

    signalList << getSignal(RowsAboutToBeRemoved, indexFinder, 0, 0 );
    signalList << getSignal(RowsRemoved, indexFinder, 0, 0 );

    persistentList << getChange(indexFinder, 0, 0, -1, true);

    persistentList << getChange(indexFinder, 1, 9, -1 );

    QTest::newRow("remove03") << signalList << persistentList;
    signalList.clear();
    persistentList.clear();

    indexFinder = IndexFinder(m_proxyModel, QList<int>() << 0 << 5);

    signalList << getSignal(RowsAboutToBeRemoved, indexFinder, 8, 8 );
    signalList << getSignal(RowsRemoved, indexFinder, 8, 8 );

    persistentList << getChange(indexFinder, 8, 8, -1, true);

    QTest::newRow("remove04") << signalList << persistentList;
    signalList.clear();
    persistentList.clear();

    indexFinder = IndexFinder(m_proxyModel, QList<int>() << 0 << 5);

    signalList << getSignal(RowsAboutToBeRemoved, indexFinder, 3, 3 );
    signalList << getSignal(RowsRemoved, indexFinder, 3, 3 );

    persistentList << getChange(indexFinder, 3, 3, -1, true);
    persistentList << getChange(indexFinder, 4, 7, -1 );

    QTest::newRow("remove05") << signalList << persistentList;
    signalList.clear();
    persistentList.clear();

    indexFinder = IndexFinder(m_proxyModel, QList<int>() << 0 << 5);

    signalList << getSignal(RowsAboutToBeRemoved, indexFinder, 0, 6 );
    signalList << getSignal(RowsRemoved, indexFinder, 0, 6 );

    persistentList << getChange(indexFinder, 0, 6, -1, true);

    QTest::newRow("remove06") << signalList << persistentList;
    signalList.clear();
    persistentList.clear();

    indexFinder = IndexFinder(m_proxyModel, QList<int>() << 0);

    signalList << getSignal(RowsAboutToBeRemoved, indexFinder, 4, 4 );
    signalList << getSignal(RowsRemoved, indexFinder, 4, 4 );

    persistentList << getChange(indexFinder, 4, 4, -1, true);

    persistentList << getChange(indexFinder, 5, 37, -1 );

    QTest::newRow("remove07") << signalList << persistentList;
  }

private slots:
  void init()
  {
    SelectionProxyModelTest::doInit();
  }

  void initTestCase()
  {
    SelectionProxyModelTest::doInitTestCase();
  }

  void cleanupTestCase()
  {
    SelectionProxyModelTest::doCleanupTestCase();
  }

};


QTEST_KDEMAIN_CORE(SelectedBranchesTest)
#include "kselectionproxymodeltest-selectedbranches.moc"

