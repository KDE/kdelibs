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

#include "proxyitemselectionwidget.h"

#include <QtGui/QSplitter>
#include <QtGui/QHBoxLayout>
#include <QtGui/QTreeView>
#include <QtGui/QSortFilterProxyModel>

#include "dynamictreemodel.h"
#include "dynamictreewidget.h"
#include "klinkitemselectionmodel.h"

#define SON(object) object->setObjectName(#object)

ProxyItemSelectionWidget::ProxyItemSelectionWidget(QWidget* parent, Qt::WindowFlags f)
  : QWidget(parent, f)
{
  QSplitter *splitter = new QSplitter(this);
  QHBoxLayout *layout = new QHBoxLayout(this);
  layout->addWidget(splitter);

  DynamicTreeModel *rootModel = new DynamicTreeModel(this);

  DynamicTreeWidget *dynamicTreeWidget = new DynamicTreeWidget(rootModel, splitter);

  QSortFilterProxyModel *proxy1 = new QSortFilterProxyModel(this);
  SON(proxy1);
  QSortFilterProxyModel *proxy2 = new QSortFilterProxyModel(this);
  SON(proxy2);
  QSortFilterProxyModel *proxy3 = new QSortFilterProxyModel(this);
  SON(proxy3);
  QSortFilterProxyModel *proxy4 = new QSortFilterProxyModel(this);
  SON(proxy4);
  QSortFilterProxyModel *proxy5 = new QSortFilterProxyModel(this);
  SON(proxy5);

  QTreeView *view1 = new QTreeView(splitter);
  QTreeView *view2 = new QTreeView(splitter);

  proxy1->setSourceModel( rootModel );
  proxy2->setSourceModel( proxy1 );
  proxy3->setSourceModel( proxy2 );

  proxy4->setSourceModel( rootModel );
  proxy5->setSourceModel( proxy4 );

  view1->setModel( proxy3 );
  view2->setModel( proxy5 );

  QItemSelectionModel *rootSelectionModel = dynamicTreeWidget->treeView()->selectionModel();

  KLinkItemSelectionModel *view1SelectionModel = new KLinkItemSelectionModel(view1->model(), rootSelectionModel, this );
  view1->setSelectionModel( view1SelectionModel );

  KLinkItemSelectionModel *view2SelectionModel = new KLinkItemSelectionModel(view2->model(), view1->selectionModel(), this );
  view2->setSelectionModel( view2SelectionModel );

  view1->expandAll();
  view2->expandAll();
}

