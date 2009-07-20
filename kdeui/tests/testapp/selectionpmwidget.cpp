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
#include "dynamictreemodel.h"
#include "kselectionproxymodel.h"
#include <QLabel>

SelectionProxyWidget::SelectionProxyWidget(QWidget* parent): QWidget(parent)
{

  QHBoxLayout *layout = new QHBoxLayout(this);
  QSplitter *vSplitter = new QSplitter( this );
  QSplitter *hSplitter1 = new QSplitter ( Qt::Vertical, vSplitter );
  QSplitter *hSplitter2 = new QSplitter ( Qt::Vertical, vSplitter );
  layout->addWidget(vSplitter);

  m_rootModel = new DynamicTreeModel(this);

  QList<int> ancestorRows;

  ModelInsertCommand *ins;
  int max_runs = 4;
  for (int i = 0; i < max_runs; i++)
  {
    ins = new ModelInsertCommand(m_rootModel, this);
    ins->setAncestorRowNumbers(ancestorRows);
    ins->setStartRow(0);
    ins->setEndRow(4);
    ins->doCommand();
    ancestorRows << 2;
  }

  ancestorRows.clear();
  ancestorRows << 3;
  for (int i = 0; i < max_runs - 1; i++)
  {
    ins = new ModelInsertCommand(m_rootModel, this);
    ins->setAncestorRowNumbers(ancestorRows);
    ins->setStartRow(0);
    ins->setEndRow(4);
    ins->doCommand();
    ancestorRows << 3;
  }

  ModelDataChangeCommand *dataChCmd = new ModelDataChangeCommand(m_rootModel, this);
  dataChCmd->setStartRow(0);
  dataChCmd->setEndRow(4);
  dataChCmd->doCommand();

  QTreeView *rootView = createLabelledView("Dynamic Tree Model", hSplitter1);
  rootView->setModel(m_rootModel);
  rootView->setSelectionMode(QAbstractItemView::ExtendedSelection);

  KSelectionProxyModel *selectedBranchesModel = new KSelectionProxyModel(rootView->selectionModel(), this);
  selectedBranchesModel->setSourceModel(m_rootModel);
  selectedBranchesModel->setFilterBehavior(KSelectionProxyModel::SelectedBranches);

  QTreeView *selectedBranchesView = createLabelledView("Selected Branches", hSplitter2);
  selectedBranchesView->setModel(selectedBranchesModel);

  KSelectionProxyModel *selectedBranchesRootsModel = new KSelectionProxyModel(rootView->selectionModel(), this);
  selectedBranchesRootsModel->setSourceModel(m_rootModel);
  selectedBranchesRootsModel->setFilterBehavior(KSelectionProxyModel::SelectedBranchesRoots);

  QTreeView *selectedBranchesRootsView = createLabelledView("Selected branches roots", hSplitter2 );
  selectedBranchesRootsView->setModel(selectedBranchesRootsModel);

  KSelectionProxyModel *selectedBranchesChildrenModel = new KSelectionProxyModel(rootView->selectionModel(), this);
  selectedBranchesChildrenModel->setSourceModel(m_rootModel);
  selectedBranchesChildrenModel->setFilterBehavior(KSelectionProxyModel::SelectedBranchesChildren);

  QTreeView *selectedBranchesChildrenView = createLabelledView("Selected Branches Children", hSplitter1 );
  selectedBranchesChildrenView->setModel(selectedBranchesChildrenModel);

  KSelectionProxyModel *onlySelectedModel = new KSelectionProxyModel(rootView->selectionModel(), this);
  onlySelectedModel->setSourceModel(m_rootModel);
  onlySelectedModel->setFilterBehavior(KSelectionProxyModel::OnlySelected);

  QTreeView *onlySelectedView = createLabelledView("Only selected", hSplitter1 );
  onlySelectedView->setModel(onlySelectedModel);

  KSelectionProxyModel *onlySelectedChildrenModel = new KSelectionProxyModel(rootView->selectionModel(), this);
  onlySelectedChildrenModel->setSourceModel(m_rootModel);
  onlySelectedChildrenModel->setFilterBehavior(KSelectionProxyModel::OnlySelectedChildren);

  QTreeView *onlySelectedChildrenView = createLabelledView("Only Selected Children", hSplitter2 );
  onlySelectedChildrenView->setModel(onlySelectedChildrenModel);

  setLayout(layout);

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
