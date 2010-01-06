/*
    Copyright (c) 2009 Stephen Kelly <steveire@gmail.com>

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

#include "dynamictreewidget.h"

#include <QTabWidget>
#include <QPlainTextEdit>
#include <QTreeView>
#include <QHBoxLayout>

#include "dynamictreemodel.h"

DynamicTreeWidget::DynamicTreeWidget(DynamicTreeModel *rootModel, QWidget* parent, Qt::WindowFlags f)
  : QWidget(parent, f), m_dynamicTreeModel(rootModel)
{
  QTabWidget *tabWidget = new QTabWidget(this);

  QHBoxLayout *layout = new QHBoxLayout(this);
  layout->addWidget(tabWidget);

  m_textEdit = new QPlainTextEdit(tabWidget);

  m_treeView = new QTreeView(tabWidget);
  m_treeView->setModel(rootModel);
  m_treeView->setSelectionMode(QAbstractItemView::ExtendedSelection);
  m_treeView->setDragDropMode(QAbstractItemView::InternalMove);
  m_treeView->setDragEnabled(true);
  m_treeView->setAcceptDrops(true);
  m_treeView->setDropIndicatorShown(true);

  tabWidget->addTab(m_textEdit, "Edit");
  tabWidget->addTab(m_treeView, "View");

  tabWidget->setCurrentIndex(ViewTab);

  connect(tabWidget, SIGNAL(currentChanged(int)), SLOT(currentChanged(int)));
  stringToModel(
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
    " - - 12"
    " - 13"
    " - 14"
    " - 15"
    " - - 16"
    " - - - 17"
    " - - - 18"
    " - 19"
    " - 20"
    " - 21"
    );
}

void DynamicTreeWidget::setInitialTree(const QString &treeString)
{
  stringToModel(treeString);
}

void DynamicTreeWidget::currentChanged(int index)
{
  switch(index)
  {
  case EditTab:
    m_textEdit->setPlainText(modelTreeToString(0, QModelIndex()));
    break;
  case ViewTab:
    if (m_textEdit->document()->isModified())
      stringToModel(m_textEdit->toPlainText());
    m_textEdit->document()->setModified(false);
    break;
  }
}

void DynamicTreeWidget::stringToModel(const QString &treeString)
{
  if (treeString.isEmpty())
    return;

  m_dynamicTreeModel->clear();
  ModelInsertCommand *command = new ModelInsertCommand(m_dynamicTreeModel);
  command->setStartRow(0);
  command->interpret(treeString);
  command->doCommand();
  m_treeView->expandAll();
}

QString DynamicTreeWidget::modelTreeToString(int depth, const QModelIndex &parent)
{
  QString result;
  QModelIndex idx;
  static const int column = 0;
  QString prefix;

  for (int i = 0; i <= depth; ++i)
    prefix.append(" -");

  for (int row = 0; row < m_dynamicTreeModel->rowCount(parent); ++row)
  {
    idx = m_dynamicTreeModel->index(row, column, parent);
    result.append(prefix + " " + idx.data().toString() + "\n");
    if (m_dynamicTreeModel->hasChildren(idx))
      result.append(modelTreeToString(depth+1, idx));
  }
  return result;
}

#include "dynamictreewidget.moc"