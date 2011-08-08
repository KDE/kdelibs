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

#include "selectionpmwidget.h"

#include <QSplitter>
#include <QTreeView>
#include <QHBoxLayout>
#include <QLabel>

#include "dynamictreemodel.h"
#include "dynamictreewidget.h"
#include "kselectionproxymodel.h"

SelectionProxyWidget::SelectionProxyWidget(QWidget* parent): QWidget(parent)
{
  QHBoxLayout *layout = new QHBoxLayout(this);
  QSplitter *splitter = new QSplitter( this );
  layout->addWidget(splitter);

  m_rootModel = new DynamicTreeModel(this);

  DynamicTreeWidget *dynTreeWidget = new DynamicTreeWidget(m_rootModel, splitter);

  dynTreeWidget->setInitialTree(
 "- 1"
 "- 2"
 "- - 3"
 "- - 3"
 "- - - 4"
 "- - - 4"
 "- - - - 4"
 "- - 4"
 "- - 5"
 "- - - 4"
 "- - - - 4"
 "- - 5"
 "- 6"
 "- 7"
 "- - 8"
 "- - - 9"
 "- - - 10"
 "- - - - 9"
 "- - - - - 10"
 "- - - - - - 9"
 "- - - - - - 10"
 "- - - - - - - 9"
 "- - - - - - - - 10"
 "- - - - - - - - 9"
 "- - - - - - - 10"
 "- - - - - 9"
 "- - - - - 9"
 "- - - - - 9"
 "- - - - - 10"
 "- - - - - - 9"
 "- - - - - - 10"
 "- - - - - 9"
 "- - - - - 9"
 "- - - - - 9"
 "- - - - - 10"
 "- - - - - - 9"
 "- - - - - - 10"
 "- - - - 10"
 "- - 11"
 "- - 12"
 "- 13"
 "- 14"
 "- 15"
 "- - 16"
 "- - - 17"
 "- - - 18"
 "- 19"
 "- 20"
 "- 21");

  QTreeView *selectionTree = createLabelledView("Selection", splitter);
  selectionTree->setSelectionMode( QAbstractItemView::ExtendedSelection );
  selectionTree->setModel(m_rootModel);
  selectionTree->expandAll();

#define SUBTREES
#define SUBTREEROOTS
#define SUBTREESWITHOUTROOTS
#define EXACTSELECTION
#define CHILDRENOFEXACTSELECTION

#ifdef SUBTREES
  KSelectionProxyModel *selectedBranchesModel = new KSelectionProxyModel(selectionTree->selectionModel(), this);
  selectedBranchesModel->setSourceModel(m_rootModel);
  selectedBranchesModel->setFilterBehavior(KSelectionProxyModel::SubTrees);

  QTreeView *selectedBranchesView = createLabelledView("SubTrees", splitter);
  selectedBranchesView->setModel(selectedBranchesModel);
#endif

#ifdef SUBTREEROOTS
  KSelectionProxyModel *selectedBranchesRootsModel = new KSelectionProxyModel(selectionTree->selectionModel(), this);
  selectedBranchesRootsModel->setSourceModel(m_rootModel);
  selectedBranchesRootsModel->setFilterBehavior(KSelectionProxyModel::SubTreeRoots);

  QTreeView *selectedBranchesRootsView = createLabelledView("SubTreeRoots", splitter);
  selectedBranchesRootsView->setModel(selectedBranchesRootsModel);
#endif

#ifdef SUBTREESWITHOUTROOTS
  KSelectionProxyModel *selectedBranchesChildrenModel = new KSelectionProxyModel(selectionTree->selectionModel(), this);
  selectedBranchesChildrenModel->setSourceModel(m_rootModel);
  selectedBranchesChildrenModel->setFilterBehavior(KSelectionProxyModel::SubTreesWithoutRoots);

  QTreeView *selectedBranchesChildrenView = createLabelledView("SubTreesWithoutRoots", splitter);
  selectedBranchesChildrenView->setModel(selectedBranchesChildrenModel);
#endif

#ifdef EXACTSELECTION
  KSelectionProxyModel *onlySelectedModel = new KSelectionProxyModel(selectionTree->selectionModel(), this);
  onlySelectedModel->setSourceModel(m_rootModel);
  onlySelectedModel->setFilterBehavior(KSelectionProxyModel::ExactSelection);

  QTreeView *onlySelectedView = createLabelledView("ExactSelection", splitter);
  onlySelectedView->setModel(onlySelectedModel);
#endif

#ifdef CHILDRENOFEXACTSELECTION
  KSelectionProxyModel *onlySelectedChildrenModel = new KSelectionProxyModel(selectionTree->selectionModel(), this);
  onlySelectedChildrenModel->setSourceModel(m_rootModel);
  onlySelectedChildrenModel->setFilterBehavior(KSelectionProxyModel::ChildrenOfExactSelection);

  QTreeView *onlySelectedChildrenView = createLabelledView("ChildrenOfExactSelection", splitter);
  onlySelectedChildrenView->setModel(onlySelectedChildrenModel);
#endif
}

QTreeView* SelectionProxyWidget::createLabelledView(const QString &labelText, QWidget *parent)
{
  QWidget *labelledTreeWidget = new QWidget(parent);
  QVBoxLayout *layout = new QVBoxLayout(labelledTreeWidget);

  QLabel *label = new QLabel(labelText, labelledTreeWidget);
  QTreeView *treeview = new QTreeView( labelledTreeWidget );

  layout->addWidget(label);
  layout->addWidget(treeview);
  return treeview;
}

