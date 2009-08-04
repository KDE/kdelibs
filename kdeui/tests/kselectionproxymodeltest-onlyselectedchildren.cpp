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

#include "proxymodeltest.h"

#include <QTimer>

#include "../itemviews/kselectionproxymodel.h"

class SelectionProxyModelTest : public ProxyModelTest
{
  Q_OBJECT
public:
  SelectionProxyModelTest(QObject *parent = 0)
      : ProxyModelTest( parent ),
      m_selectionModel(0),
      m_rowCount(0)
  {
    // Make different selections and run all of the tests.
    m_selectionModel = new QItemSelectionModel(sourceModel());
    m_proxyModel = new KSelectionProxyModel(m_selectionModel, this);
    m_proxyModel->setFilterBehavior(KSelectionProxyModel::OnlySelectedChildren);
    setProxyModel(m_proxyModel);
  }

protected:
    QVariantList getSignal(SignalType type, int start, int end)
    {
      return ProxyModelTest::getSignal(type, IndexFinder(), start, end);
    }

    void signalInsertion(const QString &name, int startRow, int rowsAffected, QList<QVariantList> signalList = QList<QVariantList>())
    {
      ProxyModelTest::signalInsertion(name, IndexFinder(), startRow, rowsAffected, m_rowCount, signalList);
    }

    void signalMove(const QString &name, int startRow, int endRow, int destRow, QList<QVariantList> signalList = QList<QVariantList>())
    {
      ProxyModelTest::signalMove(name, IndexFinder(), startRow, endRow, IndexFinder(), destRow, signalList);
    }

    void signalRemoval(const QString &name, int startRow, int rowsAffected, QList<QVariantList> signalList = QList<QVariantList>())
    {
      ProxyModelTest::signalRemoval(name, IndexFinder(), startRow, rowsAffected, m_rowCount, signalList);
    }

    void signalDataChange(const QString &name, int startRow, int endRow, QList<QVariantList> signalList = QList<QVariantList>())
    {
      IndexFinder topLeftFinder(m_proxyModel, QList<int>() << startRow );
      IndexFinder bottomRightFinder(m_proxyModel, QList<int>() << endRow );
      ProxyModelTest::signalDataChange(name, topLeftFinder, bottomRightFinder, signalList);
    }

private slots:
  void init()
  {
    m_rowCount = 0;
    ProxyModelTest::doInit();

    QList<QVariantList> signalList;
    QVariantList expected;
    QList<PersistentIndexChange> persistentList;
    IndexFinder indexFinder;

    int startRow = 0;
    int rowsInserted = 10;

    QString currentTag = QTest::currentDataTag();
    if ("insert01" == currentTag)
    {
      noSignal("insert01");
      return;
    }

    QModelIndexList listOfOne = sourceModel()->match(sourceModel()->index(0, 0), Qt::DisplayRole, "1", 1, Qt::MatchExactly);

    kDebug() << "listOfOne" << listOfOne;
    QModelIndex idx = listOfOne.at(0);

    m_selectionModel->select(idx, QItemSelectionModel::SelectCurrent);

    // This selection will not cause any signals to be emitted. Only inserting
    // child indexes into it will.

    if ("insert02" == currentTag)
    {
      rowsInserted = 10;

      signalList << getSignal(RowsAboutToBeInserted, startRow, startRow + rowsInserted - 1);
      signalList << getSignal(RowsInserted, startRow, startRow + rowsInserted - 1);

      setExpected("insert02", signalList, persistentList);
      return;
    }
    // The selected index got some child items.
    signalList << getSignal(RowsAboutToBeInserted, startRow, startRow + rowsInserted - 1);
    signalList << getSignal(RowsInserted, startRow, startRow + rowsInserted - 1);


    m_rowCount += 10;
    rowsInserted = 10;

    if ("insert03" == currentTag)
    {
      setExpected("insert03", signalList, persistentList);
      return;
    }

    if ("insert04" == currentTag)
    {
      setExpected("insert04", signalList, persistentList);
      return;
    }

    PersistentIndexChange change;

    if ("insert05" == currentTag)
    {
      signalInsertion("insert05", startRow, rowsInserted, signalList);
      return;
    }
    m_rowCount += 10;

    // From here there are 20 items below the "1" item when it is selected.
    signalList.clear();
    startRow = 0;
    rowsInserted = 20;
    // The selected index got some child items.
    signalList << getSignal(RowsAboutToBeInserted, startRow, startRow + rowsInserted - 1);
    signalList << getSignal(RowsInserted, startRow, startRow + rowsInserted - 1);

    rowsInserted = 10;

    startRow = 20;

    if ("insert06" == currentTag)
    {
      signalInsertion("insert06", startRow, rowsInserted, signalList);
      return;
    }
    m_rowCount += 10;

    // From here there are 30 items below the "1" item when it is selected.
    signalList.clear();
    startRow = 0;
    rowsInserted = 30;
    // The selected index got some child items.
    signalList << getSignal(RowsAboutToBeInserted, startRow, startRow + rowsInserted - 1);
    signalList << getSignal(RowsInserted, startRow, startRow + rowsInserted - 1);

    rowsInserted = 10;

    startRow = 10;
    if ("insert07" == currentTag)
    {
      signalInsertion("insert07", startRow, rowsInserted, signalList);
      return;
    }
    m_rowCount += 10;

    // From here there are 40 items below the "1" item when it is selected.
    signalList.clear();
    startRow = 0;
    rowsInserted = 40;
    // The selected index got some child items.
    signalList << getSignal(RowsAboutToBeInserted, startRow, startRow + rowsInserted - 1);
    signalList << getSignal(RowsInserted, startRow, startRow + rowsInserted - 1);

    startRow = 0;
    if ("insert08" == currentTag)
    {
      setExpected("insert08", signalList, persistentList);
      return;
    }

    if ("insert09" == currentTag)
    {
      // insert09 inserts cihld items into items which are already in the model.
      // That has no effect in this configuration.
      setExpected("insert09", signalList, persistentList);
      return;
    }

    IndexFinder topLeftFinder;
    IndexFinder bottomRightFinder;
    if ("change01" == currentTag)
    {
      topLeftFinder = IndexFinder(m_proxyModel, QList<int>() << 0 );
      bottomRightFinder = IndexFinder(m_proxyModel, QList<int>() << 0 );

      signalDataChange("change01", 0, 0, signalList);
      return;
    }

    if ("change02" == currentTag)
    {
      topLeftFinder = IndexFinder(m_proxyModel, QList<int>() << 4 );
      bottomRightFinder = IndexFinder(m_proxyModel, QList<int>() << 7 );

      signalDataChange("change02", 4, 7, signalList);
      return;
    }

    int rowsRemoved = 1;

    if ("remove01" == currentTag)
    {
      startRow = 0;
      signalRemoval("remove01", startRow, rowsRemoved, signalList);
      return;
    }
    m_rowCount -= 1;

    // From here there are 39 items below the "1" item when it is selected.
    signalList.clear();
    startRow = 0;
    rowsInserted = 39;
    // The selected index got some child items.
    signalList << getSignal(RowsAboutToBeInserted, startRow, startRow + rowsInserted - 1);
    signalList << getSignal(RowsInserted, startRow, startRow + rowsInserted - 1);


    if ("remove02" == currentTag)
    {
      startRow = 6;
      signalRemoval("remove02", startRow, rowsRemoved, signalList);
      return;
    }
    m_rowCount -= 1;

    // From here there are 38 items below the "1" item when it is selected.
    signalList.clear();
    startRow = 0;
    rowsInserted = 38;
    // The selected index got some child items.
    signalList << getSignal(RowsAboutToBeInserted, startRow, startRow + rowsInserted - 1);
    signalList << getSignal(RowsInserted, startRow, startRow + rowsInserted - 1);

    QModelIndex nextIndex = sourceModel()->index(5, 0, idx);


    m_selectionModel->select(nextIndex, QItemSelectionModel::SelectCurrent);

    rowsRemoved = 38;
    // Changing the selection causes some removes and inserts
    signalList << getSignal(RowsAboutToBeRemoved, startRow, startRow + rowsRemoved - 1);
    signalList << getSignal(RowsRemoved, startRow, startRow + rowsRemoved - 1);

    m_rowCount = 0;
    rowsInserted = 10;

    if ("remove03" == currentTag)
    {
    signalList << getSignal(RowsAboutToBeInserted, startRow, startRow + rowsInserted - 1);
    signalList << getSignal(RowsInserted, startRow, startRow + rowsInserted - 1);

    rowsRemoved = 1;
    m_rowCount = 10;

      signalRemoval("remove03", startRow, rowsRemoved, signalList);
      return;
    }
    m_rowCount = 10;
    m_rowCount -= 1;

    rowsRemoved = 1;
    rowsInserted = 9;
    if ("remove04" == currentTag)
    {
      signalList << getSignal(RowsAboutToBeInserted, startRow, startRow + rowsInserted - 1);
      signalList << getSignal(RowsInserted, startRow, startRow + rowsInserted - 1);

      startRow = 8;
      signalRemoval("remove04", startRow, rowsRemoved, signalList);
      return;
    }
    m_rowCount -= 1;

    rowsInserted = 8;
    if ("remove05" == currentTag)
    {
      signalList << getSignal(RowsAboutToBeInserted, startRow, startRow + rowsInserted - 1);
      signalList << getSignal(RowsInserted, startRow, startRow + rowsInserted - 1);

      startRow = 3;
      signalRemoval("remove05", startRow, rowsRemoved, signalList);
      return;
    }
    m_rowCount -= 1;

    rowsInserted = 7;
    if ("remove06" == currentTag)
    {
      signalList << getSignal(RowsAboutToBeInserted, startRow, startRow + rowsInserted - 1);
      signalList << getSignal(RowsInserted, startRow, startRow + rowsInserted - 1);

      startRow = 0;
      rowsRemoved = 7;
      signalRemoval("remove06", startRow, rowsRemoved, signalList);

      return;
    }

    // select an index whose descendant will be removed.
    nextIndex = sourceModel()->index(4, 0, idx);
    nextIndex = sourceModel()->index(5, 0, nextIndex);

    m_selectionModel->select(nextIndex, QItemSelectionModel::SelectCurrent);

    // Doesn't cause a remove signal to be emitted. The previous parent is now empty.

    startRow = 0;
    rowsInserted = 10;
    if ("remove07" == currentTag)
    {
      signalList << getSignal(RowsAboutToBeInserted, startRow, startRow + rowsInserted - 1);
      signalList << getSignal(RowsInserted, startRow, startRow + rowsInserted - 1);

      rowsRemoved = 10;
      signalRemoval("remove07", startRow, rowsRemoved, signalList);
      return;
    }

  }

private:
  QItemSelectionModel *m_selectionModel;
  KSelectionProxyModel *m_proxyModel;
  int m_rowCount;
};


QTEST_KDEMAIN(SelectionProxyModelTest, GUI)
#include "kselectionproxymodeltest-onlyselectchildren.moc"

