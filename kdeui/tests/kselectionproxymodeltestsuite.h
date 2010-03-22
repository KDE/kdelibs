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

#ifndef SELECTIONPROXYMODELTEST_H
#define SELECTIONPROXYMODELTEST_H

#include "proxymodeltest.h"

#include "kselectionproxymodel.h"
#include "modelselector.h"

class SelectionProxyModelTest : public ProxyModelTest
{
  Q_OBJECT
public:
  SelectionProxyModelTest(QObject *parent = 0)
      : ProxyModelTest(parent),
        m_selectionModel(0),
        m_modelSelector(0)
  {
  }

  void setTestData(ModelSelector *modelSelector)
  {
    disconnectTestSignals(modelSelector);
    m_modelSelector = modelSelector;
    connectTestSignals(m_modelSelector);
  }

protected:
  /* reimp */ QAbstractProxyModel* getProxy()
  {
    Q_ASSERT(sourceModel());
    m_selectionModel = new QItemSelectionModel(sourceModel());

    m_modelSelector->setWatchedModel(sourceModel());
    m_modelSelector->setSelectionModel(m_selectionModel);
    m_modelSelector->setWatch(true);

    m_proxyModel = new KSelectionProxyModel(m_selectionModel, this);
    m_proxyModel->setFilterBehavior(m_modelSelector->filterBehaviour());
    return m_proxyModel;
  }

private slots:
  void cleanupTestCase()
  {
    doCleanupTestCase();
    delete m_modelSelector;
    m_modelSelector = 0;
  }

  void cleanup()
  {
    doCleanup();
    if (m_modelSelector->selectionModel())
      m_modelSelector->selectionModel()->clearSelection();
    m_modelSelector->setWatch(false);
  }

private:
  QItemSelectionModel *m_selectionModel;
  KSelectionProxyModel *m_proxyModel;
  ModelSelector *m_modelSelector;
};

template<typename SelectorStrategy, KSelectionProxyModel::FilterBehavior _filterBehaviour>
class TestData : public SelectorStrategy
{
public:
  TestData(ProxyModelTest *proxyModelTest)
      : SelectorStrategy(proxyModelTest)
  {
  }

  /* reimp */ KSelectionProxyModel::FilterBehavior filterBehaviour() { return _filterBehaviour; }

  /* reimp */ void testInsertWhenEmptyData() { SelectorStrategy::testInsertWhenEmptyData(); }
  /* reimp */ void testInsertInRootData() { SelectorStrategy::testInsertInRootData(); }
  /* reimp */ void testInsertInTopLevelData() { SelectorStrategy::testInsertInTopLevelData(); }
  /* reimp */ void testInsertInSecondLevelData() { SelectorStrategy::testInsertInSecondLevelData(); }
};

/**
  @brief Selects only one item in the source model which does not change
*/
template< int num1 = 0,
          int num2 = 0,
          int num3 = 0,
          int num4 = 0,
          int num5 = 0>
class ImmediateSelectionStrategy : public ModelSelector
{
public:
  ImmediateSelectionStrategy(ProxyModelTest* parent = 0)
      : ModelSelector(parent)
  {
    if (num1 > 0)
      m_selectedRows << num1;
    if (num2 > 0)
      m_selectedRows << num2;
    if (num3 > 0)
      m_selectedRows << num3;
    if (num4 > 0)
      m_selectedRows << num4;
    if (num5 > 0)
      m_selectedRows << num5;
  }
};

/**
  For testing the proxy when it has no selection.
*/
typedef ImmediateSelectionStrategy<0, 0, 0, 0, 0> NoSelectionStrategy;



#endif
