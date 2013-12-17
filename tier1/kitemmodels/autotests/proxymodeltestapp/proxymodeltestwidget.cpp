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


#include "proxymodeltestwidget.h"

#include <QSplitter>
#include <QTreeView>
#include <QHBoxLayout>
#include <QPushButton>

#include "modelcommander.h"
#include "dynamictreemodel.h"
#include "kselectionproxymodel.h"
#if 0
#include "kdescendantsproxymodel.h"
#endif
#include "modelcommanderwidget.h"


ProxyModelTestWidget::ProxyModelTestWidget(QWidget* parent, Qt::WindowFlags f)
  : QWidget(parent, f)
{
  QVBoxLayout *layout = new QVBoxLayout(this);
  QSplitter *splitter = new QSplitter(this);

  m_rootModel = new DynamicTreeModel(this);

  (void) new ModelCommanderWidget(m_rootModel, splitter);


  QTreeView *rootModelView = new QTreeView(splitter);
  rootModelView->setModel(m_rootModel);
  rootModelView->setSelectionMode(QTreeView::ExtendedSelection);

  KSelectionProxyModel *selProxyModel = new KSelectionProxyModel(rootModelView->selectionModel(), this);
  selProxyModel->setSourceModel(m_rootModel);
  selProxyModel->setFilterBehavior(KSelectionProxyModel::ChildrenOfExactSelection);

  QTreeView *selModelView = new QTreeView(splitter);
  selModelView->setModel(selProxyModel);


#if 0
  KDescendantsProxyModel *descProxyModel = new KDescendantsProxyModel(this);
  descProxyModel->setSourceModel(m_rootModel);
  QTreeView *descProxyModelView = new QTreeView(splitter);
  descProxyModelView ->setModel(descProxyModel);
#endif

  // Your Proxy Here?

  layout->addWidget(splitter);

}
