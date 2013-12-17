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

#include "checkablewidget.h"

#include <QTreeView>
#include <QHBoxLayout>
#include <QSplitter>

#include <kselectionproxymodel.h>
#include <kcheckableproxymodel.h>
#include "dynamictreemodel.h"

CheckableWidget::CheckableWidget(QWidget* parent, Qt::WindowFlags f)
  : QWidget(parent, f)
{
  QHBoxLayout *layout = new QHBoxLayout(this);
  QSplitter *vSplitter = new QSplitter( this );
  layout->addWidget(vSplitter);

  DynamicTreeModel *rootModel = new DynamicTreeModel(this);

  ModelInsertCommand *insert = new ModelInsertCommand(rootModel, this);
  insert->setStartRow(0);
  insert->interpret(
    "- 1"
    "- 1"
    "- 1"
    "- - 2"
    "- - 2"
    "- - 2"
    "- - 2"
    "- 1"
    "- 1"
    "- 1"
    "- - 2"
    "- - - 3"
    "- - - - 4"
    "- - - - 4"
    "- - - 3"
    "- - - 3"
    "- - - 3"
    "- - 2"
    "- - 2"
    "- - 2"
    "- 1"
    "- 1"
  );
  insert->doCommand();

  QItemSelectionModel *checkModel = new QItemSelectionModel(rootModel, this);
  KCheckableProxyModel *checkable = new KCheckableProxyModel(this);
  checkable->setSourceModel(rootModel);
  checkable->setSelectionModel(checkModel);

  QTreeView *tree1 = new QTreeView(vSplitter);
  tree1->setModel(checkable);
  tree1->expandAll();

  KSelectionProxyModel *selectionProxy = new KSelectionProxyModel(checkModel, this);
  selectionProxy->setFilterBehavior(KSelectionProxyModel::ExactSelection);
  selectionProxy->setSourceModel(rootModel);

  QTreeView *tree2 = new QTreeView(vSplitter);
  tree2->setModel(selectionProxy);
}

