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

#include "../kselectionproxymodel.h"

class ModelWatcher : public QObject
{
  Q_OBJECT
public:
  ModelWatcher(QObject *parent)
      : QObject(parent)
  {
  }

  void setWatchedModel(QAbstractItemModel *model)
  {
    m_model = model;
    setWatch(true);
  }

  void setSelectionModel(QItemSelectionModel *selectionModel)
  {
    m_selectionModel = selectionModel;
  }

  void setWatch(bool watch)
  {
    Q_ASSERT(m_model);
    if (watch)
      connect(m_model, SIGNAL(rowsInserted(const QModelIndex &, int, int)),
            SLOT(rowsInserted(const QModelIndex &, int, int)));
    else
      disconnect(m_model, SIGNAL(rowsInserted(const QModelIndex &, int, int)),
            this, SLOT(rowsInserted(const QModelIndex &, int, int)));
  }

  void setWatchedParent(const QModelIndex &parent)
  {
    m_parent = parent;
  }

  void setWatchedRow(int row)
  {
    m_row = row;
  }

signals:
  void newItem(const QModelIndex &idx);

public slots:
  void rowsInserted(const QModelIndex &parent, int start, int end)
  {
    if (m_parent != parent)
      return;

    if (start < m_row || m_row > end)
      return;

    QModelIndex idx = m_model->index(m_row, 0, parent);

    m_selectionModel->select(idx, QItemSelectionModel::SelectCurrent);

    setWatch(false);
  }

private:
  QAbstractItemModel *m_model;
  QItemSelectionModel *m_selectionModel;
  QModelIndex m_parent;
  int m_row;
};

class SelectionProxyModelTest : public ProxyModelTest
{
  Q_OBJECT
public:
  SelectionProxyModelTest(QObject *parent = 0)
      : ProxyModelTest( parent ),
      m_selectionModel(0)
  {
    // Make different selections and run all of the tests.
    m_selectionModel = new QItemSelectionModel(sourceModel());
    m_proxyModel = new KSelectionProxyModel(m_selectionModel, this);
    setProxyModel(m_proxyModel);
  }

private slots:
  void init()
  {

    QList<QVariantList> signalList;
    QVariantList expected;
    QList<PersistentIndexChange> persistentList;
    IndexFinder indexFinder;

    int startRow = 0;
    int rowsInserted = 1;
    int rowCount = 0;

    QString currentTag = QTest::currentDataTag();
    if ("insert01" == currentTag)
    {
      noSignal("insert01");
      return;
    }

    if ("insert02" == currentTag)
    {

      // The selection will cause some signals to be emitted.
      signalList << getSignal(RowsAboutToBeInserted, indexFinder, startRow, startRow + rowsInserted - 1);
      signalList << getSignal(RowsInserted, indexFinder, startRow, startRow + rowsInserted - 1);
      m_selectionModel->select(sourceModel()->index(0, 0), QItemSelectionModel::SelectCurrent);

      indexFinder = IndexFinder( m_proxyModel, QList<int>() << 0);
      rowsInserted = 10;

      signalList << getSignal(RowsAboutToBeInserted, indexFinder, startRow, startRow + rowsInserted - 1);
      signalList << getSignal(RowsInserted, indexFinder, startRow, startRow + rowsInserted - 1);

      setExpected("insert02", signalList, persistentList);
      return;
    }

    indexFinder = IndexFinder( m_proxyModel, QList<int>() << 0);
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

    rowCount = 10;
    if ("insert05" == currentTag)
    {
      signalInsertion("insert05", indexFinder, startRow, rowsInserted, rowCount);
      return;
    }

    startRow = 20;

    if ("insert06" == currentTag)
    {
      signalInsertion("insert06", indexFinder, startRow, rowsInserted, rowCount);
      return;
    }

    startRow = 10;
    rowCount = 30;
    if ("insert07" == currentTag)
    {
      signalInsertion("insert07", indexFinder, startRow, rowsInserted, rowCount);
      return;
    }

    startRow = 0;
    rowCount = 0;
    if ("insert08" == currentTag)
    {
      indexFinder = IndexFinder(m_proxyModel, QList<int>() << 0 << 5);
      signalList << getSignal(RowsAboutToBeInserted, indexFinder, startRow, startRow + rowsInserted - 1);
      signalList << getSignal(RowsInserted, indexFinder, startRow, startRow + rowsInserted - 1);

      indexFinder = IndexFinder(m_proxyModel, QList<int>() << 0 << 5 << 5);
      signalList << getSignal(RowsAboutToBeInserted, indexFinder, startRow, startRow + rowsInserted - 1);
      signalList << getSignal(RowsInserted, indexFinder, startRow, startRow + rowsInserted - 1);

      indexFinder = IndexFinder(m_proxyModel, QList<int>() << 0 << 5 << 5 << 5);
      signalList << getSignal(RowsAboutToBeInserted, indexFinder, startRow, startRow + rowsInserted - 1);
      signalList << getSignal(RowsInserted, indexFinder, startRow, startRow + rowsInserted - 1);

      indexFinder = IndexFinder(m_proxyModel, QList<int>() << 0 << 6);
      signalList << getSignal(RowsAboutToBeInserted, indexFinder, startRow, startRow + rowsInserted - 1);
      signalList << getSignal(RowsInserted, indexFinder, startRow, startRow + rowsInserted - 1);

      indexFinder = IndexFinder(m_proxyModel, QList<int>() << 0 << 7);
      signalList << getSignal(RowsAboutToBeInserted, indexFinder, startRow, startRow + rowsInserted - 1);
      signalList << getSignal(RowsInserted, indexFinder, startRow, startRow + rowsInserted - 1);

      setExpected("insert08", signalList, persistentList);
      return;
    }

    if ("insert09" == currentTag)
    {
      // Although a tree of items is inserted, only ten base items are relevant to the model.
      indexFinder = IndexFinder( m_proxyModel, QList<int>() << 0 << 2);
      signalInsertion("insert09", indexFinder, startRow, rowsInserted, rowCount);
      return;
    }

//     IndexFinder sourceParent(m_proxyModel, QList<int>() << 0 );
//     IndexFinder destParent(m_proxyModel, QList<int>() << 0 );
//     startRow = 20;
//     int endRow = 20;
//     int destRow = 25;
//     signalMove("move01", sourceParent, startRow, endRow, destParent, destRow );

    IndexFinder topLeftFinder;
    IndexFinder bottomRightFinder;
    if ("change01" == currentTag)
    {
      topLeftFinder = IndexFinder(m_proxyModel, QList<int>() << 0 << 0 );
      bottomRightFinder = IndexFinder(m_proxyModel, QList<int>() << 0 << 0 );

      signalDataChange("change01", topLeftFinder, bottomRightFinder);
      return;
    }

    if ("change02" == currentTag)
    {
      topLeftFinder = IndexFinder(m_proxyModel, QList<int>() << 0 << 4 );
      bottomRightFinder = IndexFinder(m_proxyModel, QList<int>() << 0 << 7 );

      signalDataChange("change02", topLeftFinder, bottomRightFinder);
      return;
    }

    indexFinder = IndexFinder(m_proxyModel, QList<int>() << 0);
    int rowsRemoved = 1;

    if ("remove01" == currentTag)
    {
      startRow = 0;
      rowCount = 40;
      signalRemoval("remove01", indexFinder, startRow, rowsRemoved, rowCount);
      return;
    }

    if ("remove02" == currentTag)
    {
      startRow = 6;
      rowCount = 39;
      signalRemoval("remove02", indexFinder, startRow, rowsRemoved, rowCount);
      return;
    }

    indexFinder = IndexFinder(m_proxyModel, QList<int>() << 0 << 5);

    if ("remove03" == currentTag)
    {
      startRow = 0;
      rowCount = 10;
      signalRemoval("remove03", indexFinder, startRow, rowsRemoved, rowCount);
      return;
    }

    if ("remove04" == currentTag)
    {
      startRow = 8;
      rowCount = 9;
      signalRemoval("remove04", indexFinder, startRow, rowsRemoved, rowCount);
      return;
    }

    if ("remove05" == currentTag)
    {
      startRow = 3;
      rowCount = 8;
      signalRemoval("remove05", indexFinder, startRow, rowsRemoved, rowCount);
      return;
    }

    if ("remove06" == currentTag)
    {
      indexFinder = IndexFinder(m_proxyModel, QList<int>() << 0 << 5);
      startRow = 0;
      rowCount = 7;
      rowsRemoved = 7;
      signalRemoval("remove06", indexFinder, startRow, rowsRemoved, rowCount);
      return;
    }

    indexFinder = IndexFinder(m_proxyModel, QList<int>() << 0);

    if ("remove07" == currentTag)
    {
      startRow = 4;
      rowCount = 38;
      rowsRemoved = 1;
      signalRemoval("remove07", indexFinder, startRow, rowsRemoved, rowCount);
      return;
    }

  }

private:
  QItemSelectionModel *m_selectionModel;
  KSelectionProxyModel *m_proxyModel;
};


QTEST_KDEMAIN(SelectionProxyModelTest, GUI)
#include "kselectionproxymodeltest.moc"

