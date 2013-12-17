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

#include "kidentityproxymodelwidget.h"

#include <dynamictreemodel.h>
#include <kbreadcrumbselectionmodel.h>
#include <QBoxLayout>
#include <QSplitter>
#include <QTreeView>
#include <QEvent>
#include <QIdentityProxyModel>

#include "dynamictreewidget.h"

#include "modeltest.h"

KIdentityProxyModelWidget::KIdentityProxyModelWidget(QWidget* parent, Qt::WindowFlags f)
  : QWidget(parent, f)
{
  QHBoxLayout *layout = new QHBoxLayout(this);
  QSplitter *splitter = new QSplitter(this);
  layout->addWidget(splitter);

  DynamicTreeModel *rootModel = new DynamicTreeModel(this);

  DynamicTreeWidget *treeWidget = new DynamicTreeWidget(rootModel, splitter);
  treeWidget->setInitialTree(
    " - 1"
    " - 2"
    " - - 3"
    " - - 4"
    " - - 5"
    " - 6"
    " - 7"
    " - - 8"
    " - - - 9"
    " - - - 10"
    " - - 11"
//     " - - 12"
//     " - 13"
//     " - 14"
//     " - 15"
//     " - - 16"
//     " - - - 17"
//     " - - - 18"
//     " - 19"
//     " - 20"
//     " - 21"
  );

  QIdentityProxyModel *proxy = new QIdentityProxyModel(this);
  proxy->setSourceModel(rootModel);

  QTreeView *view1 = new QTreeView(splitter);
  view1->setModel(proxy);
  view1->expandAll();
}
