/*
 * This file is part of the proxy model test suite.
 *
 * Copyright 2009  Stephen Kelly <steveire@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301  USA
 */

#include "descendantpmwidget.h"

#include <QTreeView>
#include <QSplitter>

#include "dynamictreemodel.h"
#include "kdescendantentitiesproxymodel.h"
#include <QHBoxLayout>
#include "kselectionproxymodel.h"
#include <QLineEdit>

DescendantProxyModelWidget::DescendantProxyModelWidget(QWidget* parent): QWidget(parent)
{
  QHBoxLayout *layout = new QHBoxLayout(this);
  QSplitter *vSplitter = new QSplitter( this );
  layout->addWidget(vSplitter);


  m_rootModel = new DynamicTreeModel(this);

  const int numCols = 2;

  m_descProxyModel = new KDescendantEntitiesProxyModel(this);
  m_descProxyModel->setSourceModel(m_rootModel);

  KDescendantEntitiesProxyModel *descProxyModel2 = new KDescendantEntitiesProxyModel(this);
  descProxyModel2->setSourceModel(m_rootModel);
  descProxyModel2->setDisplayAncestorData(true);

  m_itemSelectionModel = new QItemSelectionModel(m_descProxyModel, this);

  m_selectionProxyModel = new KSelectionProxyModel(m_itemSelectionModel, this);
  m_selectionProxyModel->setSourceModel(m_descProxyModel);

  QList<int> ancestorRows;

  ModelInsertCommand *insertCommand;
  int max_runs = 4;
  for (int i = 0; i < max_runs; i++)
  {
    insertCommand = new ModelInsertCommand(m_rootModel, this);
    insertCommand->setAncestorRowNumbers(ancestorRows);
    insertCommand->setNumCols(numCols);
    insertCommand->setStartRow(0);
    insertCommand->setEndRow(9);
    insertCommand->doCommand();
    ancestorRows << 9;
  }
  ancestorRows.clear();
  ancestorRows << 4;
  for (int i = 0; i < max_runs; i++)
  {
    insertCommand = new ModelInsertCommand(m_rootModel, this);
    insertCommand->setAncestorRowNumbers(ancestorRows);
    insertCommand->setNumCols(numCols);
    insertCommand->setStartRow(0);
    insertCommand->setEndRow(9);
    insertCommand->doCommand();
    ancestorRows << 4;
  }

  QTreeView *treeview = new QTreeView( vSplitter );
  treeview->setModel(m_rootModel);
  treeview->setSelectionMode(QAbstractItemView::ExtendedSelection);

  m_descView = new QTreeView( vSplitter );
  m_descView->setModel(m_descProxyModel);

  QTreeView *descView2 = new QTreeView( vSplitter );
  descView2->setModel(descProxyModel2);


  QWidget *w = new QWidget(vSplitter);
  QVBoxLayout *vLayout = new QVBoxLayout(w);
  QTreeView *matchView = new QTreeView(w);
  matchView->setModel(m_selectionProxyModel);
  m_lineEdit = new QLineEdit(w);
  connect(m_lineEdit, SIGNAL(textChanged(const QString &)), SLOT(doMatch(const QString &)));
  connect(m_descView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), SLOT(refreshMatch()));

  vLayout->addWidget(m_lineEdit);
  vLayout->addWidget(matchView);

  setLayout(layout);


}


void DescendantProxyModelWidget::doMatch(const QString &matchData)
{
  m_itemSelectionModel->clearSelection();

  if (matchData.isEmpty())
    return;

  QModelIndex start = m_descView->currentIndex();

  if (!start.isValid())
    start = m_descProxyModel->index(0, 0);

  // TODO: get from user.
  int hits = -1;

  QModelIndexList matches = m_descProxyModel->match(start, Qt::DisplayRole, matchData, hits, Qt::MatchContains);

  foreach(const QModelIndex &matchingIndex, matches)
  {
    m_itemSelectionModel->select(matchingIndex, QItemSelectionModel::Select | QItemSelectionModel::Rows);
  }
}


void DescendantProxyModelWidget::refreshMatch()
{
  doMatch(m_lineEdit->text());
}


