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

#include "reparentingpmwidget.h"

#include <QTreeView>
#include <QSplitter>
#include <QVBoxLayout>

#include "dynamictreemodel.h"
#include "dynamictreewidget.h"
#include "scriptablereparentingwidget.h"

ReparentingProxyModelWidget::ReparentingProxyModelWidget(QWidget* parent): QWidget(parent)
{
  QVBoxLayout *layout = new QVBoxLayout(this);

  QSplitter *vSplitter = new QSplitter( this );
  layout->addWidget(vSplitter);

  m_rootModel = new DynamicTreeModel(this);

  DynamicTreeWidget *dynamicTreeWidget = new DynamicTreeWidget(m_rootModel, vSplitter);
  dynamicTreeWidget->setInitialTree(
    "- 1"
    "- 2"
    "- - 3"
    "- - - 4"
    "- 5"
    "- 6"
    "- 7"
  );

  ScriptableReparentingWidget *reparentingWidget = new ScriptableReparentingWidget(m_rootModel, vSplitter);

  setLayout(layout);
}

#include "reparentingpmwidget.moc"

