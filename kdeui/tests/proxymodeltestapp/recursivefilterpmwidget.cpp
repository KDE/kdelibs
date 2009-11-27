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

#include "recursivefilterpmwidget.h"

#include <QSplitter>
#include <QTreeView>
#include <QHBoxLayout>
#include "dynamictreemodel.h"
#include "recursivefilterproxymodel.h"
#include "ksortfilterproxymodel.h"
#include <QLabel>

#include "modeltest.h"


TestRecursionModel::TestRecursionModel(QObject* parent)
    : KRecursiveFilterProxyModel(parent)
{

}

bool TestRecursionModel::acceptRow(int sourceRow, const QModelIndex& sourceParent) const
{
  static const int column = 0;
  return (sourceModel()->index(sourceRow, column, sourceParent).data() == "12"
       || sourceModel()->index(sourceRow, column, sourceParent).data() == "13"
       || sourceModel()->index(sourceRow, column, sourceParent).data() == "37"
       || sourceModel()->index(sourceRow, column, sourceParent).data() == "43");
}

RecursiveFilterProxyWidget::RecursiveFilterProxyWidget(QWidget* parent): QWidget(parent)
{

  QHBoxLayout *layout = new QHBoxLayout(this);
  QSplitter *vSplitter = new QSplitter( this );
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

  QTreeView *rootView = new QTreeView( vSplitter);
  rootView->setModel(m_rootModel);

  KSortFilterProxyModel *p = new KSortFilterProxyModel(this);
  p->setFilterRegExp("12|13|37|43");
  p->setSourceModel(m_rootModel);

  TestRecursionModel *recursiveFilterModel = new TestRecursionModel(this);
  recursiveFilterModel->setSourceModel(m_rootModel);

  QTreeView *ksfpmView = new QTreeView(vSplitter);
  ksfpmView->setModel(p);

  QTreeView *recursivePMView = new QTreeView(vSplitter);
  recursivePMView->setModel(recursiveFilterModel);

  setLayout(layout);

  ancestorRows.clear();

  ancestorRows << 3 << 3 << 2;
  ins = new ModelInsertCommand(m_rootModel, this);
  ins->setAncestorRowNumbers(ancestorRows);
  ins->setStartRow(0);
  ins->setEndRow(4);
  ins->doCommand();

  ModelRemoveCommand *rem = new ModelRemoveCommand(m_rootModel, this);
  rem->setAncestorRowNumbers(ancestorRows);
  rem->setStartRow(0);
  rem->setEndRow(4);
  rem->doCommand();

  ins = new ModelInsertCommand(m_rootModel, this);
  ins->setAncestorRowNumbers(ancestorRows);
  ins->setStartRow(0);
  ins->setEndRow(4);
  ins->doCommand();

}
