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


#include "lessthanwidget.h"

#include <QLabel>
#include <QTreeView>
#include <QVBoxLayout>


ColoredTreeModel::ColoredTreeModel(QObject* parent)
    : DynamicTreeModel(parent),
      m_selectionModel(0),
      m_lessThanColour(Qt::yellow),
      m_greaterThanColour(Qt::red)
{
}

void ColoredTreeModel::setSelectionModel(QItemSelectionModel *selectionModel)
{
  m_selectionModel = selectionModel;
  connect(selectionModel, SIGNAL(selectionChanged(QItemSelection,QItemSelection)), SLOT(recolor()));
}

void ColoredTreeModel::recolor(const QModelIndex &parent)
{
  const QModelIndex topLeft = index(0, 0, parent);
  const int _rowCount = rowCount(parent);
  const QModelIndex bottomRight = index(_rowCount - 1, columnCount(parent) -1, parent );
  emit dataChanged(topLeft, bottomRight);

  static const int column = 0;
  QModelIndex idx;
  for (int row = 0; row < _rowCount; ++row)
  {
    idx = index(row, column, parent);
    if (hasChildren(idx))
      recolor(idx);
  }
}


QVariant ColoredTreeModel::data(const QModelIndex& index, int role) const
{
  if (role != Qt::BackgroundRole || !m_selectionModel || m_selectionModel->selection().indexes().size() != 1)
    return DynamicTreeModel::data(index, role);

  const QModelIndex selectedIndex = m_selectionModel->selection().indexes().first();

  if (index == selectedIndex)
    return DynamicTreeModel::data(index, role);

  if (index < selectedIndex)
    return m_lessThanColour;

  Q_ASSERT(selectedIndex < index);

  return m_greaterThanColour;
}

void LessThanWidget::insertGrid(QList<int> address)
{
  ModelInsertCommand *ins = new ModelInsertCommand(m_coloredTreeModel, this);
  ins->setAncestorRowNumbers(address);
  ins->setNumCols(5);
  ins->setStartRow(0);
  ins->setEndRow(5);
  ins->doCommand();
}

LessThanWidget::LessThanWidget(QWidget* parent, Qt::WindowFlags f)
    : QWidget(parent, f)
{
  QLabel *explanation = new QLabel(this);
  explanation->setText("The yellow items are 'less than' the selected item according to QModelIndex::operator<().\n"
                       "The red items are greater than the selected item (i.e, not less than and not equal).");

  m_coloredTreeModel = new ColoredTreeModel(this);
  QTreeView *treeView = new QTreeView(this);
  treeView->setModel(m_coloredTreeModel);
  treeView->setSelectionBehavior(QAbstractItemView::SelectItems);
  treeView->setSelectionMode(QTreeView::SingleSelection);

  m_coloredTreeModel->setSelectionModel(treeView->selectionModel());

  QVBoxLayout *layout = new QVBoxLayout(this);
  layout->addWidget(explanation);
  layout->addWidget(treeView);

  insertGrid(QList<int>());
  insertGrid(QList<int>() << 2);
  insertGrid(QList<int>() << 3);
  insertGrid(QList<int>() << 4);
  insertGrid(QList<int>() << 3 << 2);
  insertGrid(QList<int>() << 3 << 3);
  insertGrid(QList<int>() << 3 << 4);
}
