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

#ifndef MODELSELECTOR_H
#define MODELSELECTOR_H

#include <QItemSelectionModel>

#include <kselectionproxymodel.h>

#include "proxymodeltest.h"

#include "proxymodeltestsuite_export.h"

class ModelSpy;

class OnlySelectedChildrenTest;

class PROXYMODELTESTSUITE_EXPORT ModelSelector : public ProxyModelTestData
{
  Q_OBJECT
public:
  ModelSelector(ProxyModelTest *ProxyModelTest = 0);

  void setWatchedModel(QAbstractItemModel *model);

  void setSelectionModel(QItemSelectionModel *selectionModel);

  void setRootModel(DynamicTreeModel *rootModel);

  QItemSelectionModel* selectionModel() const { return m_selectionModel; }
  QAbstractItemModel *watchedModel() { return m_model; }

  void setWatch(bool watch);

  virtual KSelectionProxyModel::FilterBehavior filterBehaviour() = 0;

public slots:
  void rowsInserted(const QModelIndex &parent, int start, int end);

  void testInsertWhenEmptyData()
  {
    dummyTestData();
  }

  void testInsertInRootData()
  {
    dummyTestData();
  }

  void testInsertInTopLevelData()
  {
    dummyTestData();
  }

  void testInsertInSecondLevelData()
  {
    dummyTestData();
  }

  void testRemoveFromRootData()
  {
    dummyTestData();
  }

  void testRemoveFromTopLevelData()
  {
    dummyTestData();
  }

  void testRemoveFromSecondLevelData()
  {
    dummyTestData();
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

protected slots:
  void modelDestroyed() {
    m_model = 0;
    m_selectionModel = 0;
  }

protected:
  QAbstractItemModel *m_model;
  QItemSelectionModel *m_selectionModel;
  DynamicTreeModel *m_rootModel;
  ModelSpy *m_modelSpy;

  QList<int> m_selectedRows;
};

#endif

