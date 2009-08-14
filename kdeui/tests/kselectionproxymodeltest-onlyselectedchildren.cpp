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

class OnlySelectedChildrenTest : public SelectionProxyModelTest
{
  Q_OBJECT
public:
  OnlySelectedChildrenTest(QObject *parent = 0)
      : SelectionProxyModelTest( parent )
  {
    m_proxyModel->setFilterBehavior(KSelectionProxyModel::OnlySelectedChildren);
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

    // This selection will not cause any signals to be emitted. Only inserting
    // child indexes into it will.
    QTest::newRow("insert01") << signalList << persistentList;
    signalList.clear();

    signalList << getSignal(RowsAboutToBeInserted, indexFinder, 0, 9);
    signalList << getSignal(RowsInserted, indexFinder, 0, 9);

    QTest::newRow("insert02") << signalList << persistentList;
    signalList.clear();

    QTest::newRow("insert03") << signalList << persistentList;
    signalList.clear();

    QTest::newRow("insert04") << signalList << persistentList;
    signalList.clear();

    signalList << getSignal(RowsAboutToBeInserted, indexFinder, 0, 9);
    signalList << getSignal(RowsInserted, indexFinder, 0, 9);

    persistentList << getChange(indexFinder, 0, 9, 10);

    QTest::newRow("insert05") << signalList << persistentList;
    signalList.clear();
    persistentList.clear();

    signalList << getSignal(RowsAboutToBeInserted, indexFinder, 20, 29);
    signalList << getSignal(RowsInserted, indexFinder, 20, 29);

    QTest::newRow("insert06") << signalList << persistentList;
    signalList.clear();
    persistentList.clear();

    signalList << getSignal(RowsAboutToBeInserted, indexFinder, 10, 19);
    signalList << getSignal(RowsInserted, indexFinder, 10, 19);

    persistentList << getChange(indexFinder, 10, 29, 10);

    QTest::newRow("insert07") << signalList << persistentList;
    signalList.clear();
    persistentList.clear();

    QTest::newRow("insert08") << signalList << persistentList;
    QTest::newRow("insert09") << signalList << persistentList;

    IndexFinder topLeftFinder = IndexFinder(m_proxyModel, QList<int>() << 0 );
    IndexFinder bottomRightFinder = IndexFinder(m_proxyModel, QList<int>() << 0 );

    signalList << ( QVariantList() << DataChanged << QVariant::fromValue(topLeftFinder) << QVariant::fromValue(bottomRightFinder) );

    QTest::newRow("change01") << signalList << persistentList;
    signalList.clear();
    persistentList.clear();

    topLeftFinder = IndexFinder(m_proxyModel, QList<int>() << 4 );
    bottomRightFinder = IndexFinder(m_proxyModel, QList<int>() << 7 );

    signalList << ( QVariantList() << DataChanged << QVariant::fromValue(topLeftFinder) << QVariant::fromValue(bottomRightFinder) );

    QTest::newRow("change02") << signalList << persistentList;
    signalList.clear();
    persistentList.clear();

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

    // The removed indexes go invalid.
    persistentList << getChange(indexFinder, 6, 6, -1, true);

    // Rows after it will be moved up.
    persistentList << getChange(indexFinder, 7, 38, -1 );

    QTest::newRow("remove02") << signalList << persistentList;
    signalList.clear();
    persistentList.clear();

    QTest::newRow("remove03") << signalList << persistentList;
    QTest::newRow("remove04") << signalList << persistentList;
    QTest::newRow("remove05") << signalList << persistentList;
    QTest::newRow("remove06") << signalList << persistentList;

    signalList << getSignal(RowsAboutToBeRemoved, indexFinder, 4, 4 );
    signalList << getSignal(RowsRemoved, indexFinder, 4, 4 );

    // The removed indexes go invalid.
    persistentList << getChange(indexFinder, 4, 4, -1, true);

    // Rows after it will be moved up.
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


QTEST_KDEMAIN(OnlySelectedChildrenTest, GUI)
#include "kselectionproxymodeltest-onlyselectedchildren.moc"

