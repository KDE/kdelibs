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

#include "breadcrumbswidget.h"

#include "dynamictreemodel.h"
#include "dynamictreewidget.h"

#include <QSplitter>
#include <QListView>
#include <QTreeView>
#include <QBoxLayout>

#include "kselectionproxymodel.h"
#include "kbreadcrumbselectionmodel.h"


MultiSelectionModel::MultiSelectionModel(QAbstractItemModel* model, QList< QItemSelectionModel* > selectionModels, QObject *parent)
  : QItemSelectionModel(model, parent), m_selectionModels(selectionModels)
{

}

void MultiSelectionModel::select(const QModelIndex& index, QItemSelectionModel::SelectionFlags command)
{
  foreach(QItemSelectionModel *selectionModel, m_selectionModels)
    selectionModel->select(index, command);
  QItemSelectionModel::select(index, command);
}


void MultiSelectionModel::select(const QItemSelection& selection, QItemSelectionModel::SelectionFlags command)
{
  foreach(QItemSelectionModel *selectionModel, m_selectionModels)
    selectionModel->select(selection, command);
  QItemSelectionModel::select(selection, command);
}

BreadcrumbsWidget::BreadcrumbsWidget(QWidget* parent, Qt::WindowFlags f)
  : QWidget(parent, f)
{
  DynamicTreeModel *rootModel = new DynamicTreeModel(this);
  QSplitter *splitter = new QSplitter(this);
  QHBoxLayout *layout = new QHBoxLayout(this);
  layout->addWidget(splitter);

  DynamicTreeWidget *dynamicTree = new DynamicTreeWidget(rootModel, splitter);
  dynamicTree->treeView()->setSelectionMode(QAbstractItemView::SingleSelection);
  dynamicTree->setInitialTree(
 "- 1"
 "- - 2"
 "- - 2"
 "- - - 3"
 "- - - - 4"
 "- - - - - 5"
 "- - 2"
 "- 6"
 "- 6"
 "- 6"
 "- - 7"
 "- - - 8"
 "- - - 8"
 "- - - - 9"
 "- - - - - 10"
 "- - - 8"
 "- - - 8"
 "- - 8"
 "- 16"
 "- - 17"
 "- - - 18"
 "- - - - 19"
 "- - - - - 20");

  QList<QItemSelectionModel*> selectionModelList;
  QItemSelectionModel *fullBreadcrumbSelectionModel = new QItemSelectionModel(rootModel, this);

  KBreadcrumbSelectionModel *fullBreadcrumbProxySelector = new KBreadcrumbSelectionModel(fullBreadcrumbSelectionModel, this);
  selectionModelList << fullBreadcrumbProxySelector;

  KSelectionProxyModel *fullBreadCrumbSelectionProxyModel = new KSelectionProxyModel( fullBreadcrumbSelectionModel, this);
  fullBreadCrumbSelectionProxyModel->setSourceModel( rootModel );
  fullBreadCrumbSelectionProxyModel->setFilterBehavior( KSelectionProxyModel::ExactSelection );

  QListView *fullBreadcrumbProxyView = new QListView(splitter);
  fullBreadcrumbProxyView->setModel( fullBreadCrumbSelectionProxyModel );

  QItemSelectionModel *breadcrumbOnlySelectionModel = new QItemSelectionModel(rootModel, this);

  KBreadcrumbSelectionModel *breadcrumbOnlyProxySelector = new KBreadcrumbSelectionModel(breadcrumbOnlySelectionModel, this);
  breadcrumbOnlyProxySelector->setActualSelectionIncluded(false);
  selectionModelList << breadcrumbOnlyProxySelector;

  KSelectionProxyModel *breadcrumbOnlySelectionProxyModel = new KSelectionProxyModel( breadcrumbOnlySelectionModel, this);
  breadcrumbOnlySelectionProxyModel->setSourceModel( rootModel );
  breadcrumbOnlySelectionProxyModel->setFilterBehavior( KSelectionProxyModel::ExactSelection );

  QListView *breadcrumbOnlyProxyView = new QListView(splitter);
  breadcrumbOnlyProxyView->setModel( breadcrumbOnlySelectionProxyModel );

  int selectionDepth = 2;

  QItemSelectionModel *thisAndAscendantsSelectionModel = new QItemSelectionModel(rootModel, this);

  KBreadcrumbSelectionModel *thisAndAscendantsProxySelector = new KBreadcrumbSelectionModel(thisAndAscendantsSelectionModel, this);
  thisAndAscendantsProxySelector->setBreadcrumbLength(selectionDepth);
  selectionModelList << thisAndAscendantsProxySelector;

  KSelectionProxyModel *thisAndAscendantsSelectionProxyModel = new KSelectionProxyModel( thisAndAscendantsSelectionModel, this);
  thisAndAscendantsSelectionProxyModel->setSourceModel( rootModel );
  thisAndAscendantsSelectionProxyModel->setFilterBehavior( KSelectionProxyModel::ExactSelection );

  QListView *thisAndAscendantsProxyView = new QListView(splitter);
  thisAndAscendantsProxyView->setModel( thisAndAscendantsSelectionProxyModel );

  QItemSelectionModel *ascendantsOnlySelectionModel = new QItemSelectionModel(rootModel, this);

  KBreadcrumbSelectionModel *ascendantsOnlyProxySelector = new KBreadcrumbSelectionModel(ascendantsOnlySelectionModel, this);
  ascendantsOnlyProxySelector->setActualSelectionIncluded(false);
  ascendantsOnlyProxySelector->setBreadcrumbLength(selectionDepth);
  selectionModelList << ascendantsOnlyProxySelector;

  KSelectionProxyModel *ascendantsOnlySelectionProxyModel = new KSelectionProxyModel( ascendantsOnlySelectionModel, this);
  ascendantsOnlySelectionProxyModel->setSourceModel( rootModel );
  ascendantsOnlySelectionProxyModel->setFilterBehavior( KSelectionProxyModel::ExactSelection );

  QListView *ascendantsOnlyProxyView = new QListView(splitter);
  ascendantsOnlyProxyView->setModel( ascendantsOnlySelectionProxyModel );

  MultiSelectionModel *multiSelectionModel = new MultiSelectionModel(rootModel, selectionModelList, this);
  dynamicTree->treeView()->setSelectionModel(multiSelectionModel);
}

