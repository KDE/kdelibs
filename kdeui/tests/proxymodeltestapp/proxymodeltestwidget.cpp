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
#include "kdescendantsproxymodel.h"

ProxyModelTestWidget::ProxyModelTestWidget(QWidget* parent, Qt::WindowFlags f): QWidget(parent, f)
{
  QVBoxLayout *layout = new QVBoxLayout(this);
  m_nextCommandButton = new QPushButton(this);
  QSplitter *splitter = new QSplitter(this);

  connect(m_nextCommandButton, SIGNAL(clicked(bool)), this, SLOT(slotNextCommand()));

  m_rootModel = new DynamicTreeModel(this);

  m_commander = new ModelCommander(m_rootModel, this);

  m_commander->executeNextCommand();
  m_nextCommandButton->setText("Next (" + m_commander->nextCommand().first + ')');

  QTreeView *rootModelView = new QTreeView(splitter);
  rootModelView->setModel(m_rootModel);

  KSelectionProxyModel *selProxyModel = new KSelectionProxyModel(rootModelView->selectionModel(), this);
  selProxyModel->setSourceModel(m_rootModel);
  QTreeView *selProxyModelView = new QTreeView(splitter);
  selProxyModelView->setModel(selProxyModel);

  KDescendantsProxyModel *descProxyModel = new KDescendantsProxyModel(this);
  descProxyModel->setSourceModel(m_rootModel);
  QTreeView *descProxyModelView = new QTreeView(splitter);
  descProxyModelView ->setModel(descProxyModel);

  // Your Proxy Here?

  layout->addWidget(m_nextCommandButton);
  layout->addWidget(splitter);

}

void ProxyModelTestWidget::slotNextCommand()
{
  m_commander->executeNextCommand();
  if (!m_commander->hasNextCommand())
  {
    m_nextCommandButton->setText("Reset");
    disconnect(m_nextCommandButton, SIGNAL(clicked(bool)), this, SLOT(slotNextCommand()));
    connect(m_nextCommandButton, SIGNAL(clicked(bool)), this, SLOT(slotReset()));
  } else {
    m_nextCommandButton->setText("Next (" + m_commander->nextCommand().first + ')');
  }
}

void ProxyModelTestWidget::slotReset()
{
  m_rootModel->clear();
  m_commander->setDefaultCommands();
  m_commander->executeNextCommand();
  m_nextCommandButton->setText("Next (" + m_commander->nextCommand().first + ')');
  disconnect(m_nextCommandButton, SIGNAL(clicked(bool)), this, SLOT(slotReset()));
  connect(m_nextCommandButton, SIGNAL(clicked(bool)), this, SLOT(slotNextCommand()));
}

