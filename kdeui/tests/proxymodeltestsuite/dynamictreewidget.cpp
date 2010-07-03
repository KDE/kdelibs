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

#include <QComboBox>
#include <QTabWidget>
#include <QPlainTextEdit>
#include <QTreeView>
#include <QHBoxLayout>
#include <QPushButton>
#include <QRadioButton>

#include "dynamictreemodel.h"

static const char * const treePredefinesNames[] = {
  "Flat List",
  "Straight Line Tree",
  "Dragon Teeth 1",
  "Dragon Teeth 2",
  "Random Tree 1"
};

static const char * const treePredefinesContent[] = {
  " - 1"
  " - 1"
  " - 1"
  " - 1"
  " - 1"
  " - 1"
  " - 1"
  " - 1"
  " - 1"
  " - 1"
  " - 1"
  " - 1"
  " - 1"
  " - 1"
  " - 1"
  " - 1",

  " - 1"
  " - - 1"
  " - - - 1"
  " - - - - 1"
  " - - - - - 1"
  " - - - - - - 1"
  " - - - - - - - 1"
  " - - - - - - - - 1"
  " - - - - - - - - - 1"
  " - - - - - - - - - - 1"
  " - - - - - - - - - - - 1"
  " - - - - - - - - - - - - 1"
  " - - - - - - - - - - - - - 1"
  " - - - - - - - - - - - - - - 1"
  " - - - - - - - - - - - - - - - 1"
  " - - - - - - - - - - - - - - - - 1",

  " - 1"
  " - - 1"
  " - - - 1"
  " - - - - 1"
  " - 1"
  " - - 1"
  " - - - 1"
  " - - - - 1"
  " - 1"
  " - - 1"
  " - - - 1"
  " - - - - 1"
  " - 1"
  " - - 1"
  " - - - 1"
  " - - - - 1",

  " - 1"
  " - - 1"
  " - - - 1"
  " - - - - 1"
  " - - - - - 1"
  " - 1"
  " - - 1"
  " - - - 1"
  " - - - - 1"
  " - - - - - 1"
  " - 1"
  " - - 1"
  " - - - 1"
  " - - - - 1"
  " - - - - - 1"
  " - 1"
  " - - 1"
  " - - - 1"
  " - - - - 1"
  " - - - - - 1",

  " - 1"
  " - 2"
  " - - 3"
  " - - - 4"
  " - 5"
  " - 6"
  " - 7"
  " - - 8"
  " - - - 9"
  " - - - 10"
  " - - - - 11"
  " - - - 12"
  " - - - - 13"
  " - 14"
  " - 15"
};

static const char * const insertSubTreePredefinesNames[] = {
  "Flat List",
  "Straight Line Tree",
  "Dragon Teeth 1",
  "Dragon Teeth 2",
  "Random Tree 1"
};

static const char * const insertSubTreePredefinesContent[] = {
  " - 1\n"
  " - 1\n"
  " - 1\n"
  " - 1\n",

  " - 1\n"
  " - - 1\n"
  " - - - 1\n"
  " - - - - 1\n",

  " - 1\n"
  " - - 1\n"
  " - 1\n"
  " - - 1\n",

  " - 1\n"
  " - - 1\n"
  " - - - 1\n"
  " - 1\n"
  " - - 1\n"
  " - - - 1\n",

  " - 1\n"
  " - 2\n"
  " - - 3\n"
  " - - - 4\n"
  " - 5\n"
};

DynamicTreeWidget::DynamicTreeWidget(DynamicTreeModel *rootModel, QWidget* parent, Qt::WindowFlags f)
  : QWidget(parent, f), m_dynamicTreeModel(rootModel)
{
  QTabWidget *tabWidget = new QTabWidget(this);

  QHBoxLayout *layout = new QHBoxLayout(this);
  layout->addWidget(tabWidget);

  QWidget *editContainer = new QWidget(tabWidget);
  QVBoxLayout *editLayout = new QVBoxLayout(editContainer);

  m_treePredefines = new QComboBox(editContainer);
  for (uint i = 0; i < sizeof treePredefinesNames / sizeof *treePredefinesNames; ++i)
    m_treePredefines->addItem(*(treePredefinesNames + i), *(treePredefinesContent + i));
  editLayout->addWidget(m_treePredefines);
  connect(m_treePredefines, SIGNAL(currentIndexChanged(int)), SLOT(setTreePredefine(int)));

  m_textEdit = new QPlainTextEdit(editContainer);
  editLayout->addWidget(m_textEdit);

  QWidget *viewContainer = new QWidget(tabWidget);

  QVBoxLayout *viewLayout = new QVBoxLayout(viewContainer);

  m_treeView = new QTreeView(tabWidget);
  m_treeView->setModel(rootModel);
  m_treeView->setSelectionMode(QAbstractItemView::ExtendedSelection);
  m_treeView->setDragDropMode(QAbstractItemView::InternalMove);
  m_treeView->setDragEnabled(true);
  m_treeView->setAcceptDrops(true);
  m_treeView->setDropIndicatorShown(true);

  QPushButton *m_removeButton = new QPushButton("Remove", tabWidget);

  connect(m_removeButton, SIGNAL(clicked(bool)), SLOT(removeSelected()));

  m_insertSubTreePredefines = new QComboBox(this);
  for (uint i = 0; i < sizeof insertSubTreePredefinesNames / sizeof *insertSubTreePredefinesNames; ++i)
    m_insertSubTreePredefines->addItem(*(insertSubTreePredefinesNames + i), *(insertSubTreePredefinesContent + i));
  editLayout->addWidget(m_insertSubTreePredefines);
  connect(m_insertSubTreePredefines, SIGNAL(currentIndexChanged(int)), SLOT(setInsertSubTreePredefine(int)));

  m_insertPatternTextEdit = new QPlainTextEdit(tabWidget);
  m_insertPatternTextEdit->setMaximumHeight(100);

  m_insertChildren = new QRadioButton("Insert Children", tabWidget);
  m_insertSiblingsAbove = new QRadioButton("Insert Siblings Above", tabWidget);
  m_insertSiblingsBelow = new QRadioButton("Insert Siblings Below", tabWidget);

  m_insertChildren->setChecked(true);

  QPushButton *m_insertButton = new QPushButton("Insert", tabWidget);

  connect(m_insertButton, SIGNAL(clicked(bool)), SLOT(insertSelected()));

  QPushButton *m_resetButton = new QPushButton("Reset", tabWidget);

  connect(m_resetButton, SIGNAL(clicked(bool)), SLOT(resetModel()));

  viewLayout->addWidget(m_treeView);

  viewLayout->addWidget(m_removeButton);

  viewLayout->addWidget(m_insertSubTreePredefines);
  viewLayout->addWidget(m_insertPatternTextEdit);
  viewLayout->addWidget(m_insertChildren);
  viewLayout->addWidget(m_insertSiblingsAbove);
  viewLayout->addWidget(m_insertSiblingsBelow);
  viewLayout->addWidget(m_insertButton);
  viewLayout->addWidget(m_resetButton);

  tabWidget->addTab(editContainer, "Edit");
  tabWidget->addTab(viewContainer, "View");

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

void DynamicTreeWidget::removeSelected()
{
  QModelIndex parent;
  ModelRemoveCommand *removeCommand = new ModelRemoveCommand(m_dynamicTreeModel, this);
  QItemSelection selection = m_treeView->selectionModel()->selection();
  while (!selection.isEmpty())
  {
    const QItemSelectionRange range = selection.takeFirst(); // The selection model will take care of updating persistent indexes.
    Q_ASSERT(range.isValid());
    kDebug() << range.parent() << range.top() << range.bottom();
    removeCommand->setAncestorRowNumbers(m_dynamicTreeModel->indexToPath(range.parent()));
    removeCommand->setStartRow(range.top());
    removeCommand->setEndRow(range.bottom());

    kDebug() << m_dynamicTreeModel->indexToPath(range.parent());

    removeCommand->doCommand();
    selection = m_treeView->selectionModel()->selection();
  }
}

void DynamicTreeWidget::insertSelected()
{
  const QModelIndexList selectedRows = m_treeView->selectionModel()->selectedRows();

  if (selectedRows.size() != 1)
    return;

  const QModelIndex selectedRow = selectedRows.first();

  ModelInsertCommand *ins = new ModelInsertCommand(m_dynamicTreeModel, this);
  if (m_insertChildren->isChecked())
  {
    ins->setAncestorRowNumbers(m_dynamicTreeModel->indexToPath(selectedRow));
    ins->setStartRow(0);
  } else if (m_insertSiblingsAbove->isChecked())
  {
    ins->setAncestorRowNumbers(m_dynamicTreeModel->indexToPath(selectedRow.parent()));
    ins->setStartRow(selectedRow.row());
  } else {
    Q_ASSERT(m_insertSiblingsBelow->isChecked());
    ins->setAncestorRowNumbers(m_dynamicTreeModel->indexToPath(selectedRow.parent()));
    ins->setStartRow(selectedRow.row() + 1);
  }
  ins->interpret(m_insertPatternTextEdit->toPlainText());
  ins->doCommand();
}

void DynamicTreeWidget::resetModel()
{
  ModelResetCommand *resetCommand = new ModelResetCommand(m_dynamicTreeModel, this);

  resetCommand->setInitialTree(m_insertPatternTextEdit->toPlainText().trimmed());
  resetCommand->doCommand();
}

void DynamicTreeWidget::setTreePredefine(int index)
{
  stringToModel(m_treePredefines->itemData(index).toString());
  m_textEdit->setPlainText(modelTreeToString(0, QModelIndex()));
}

void DynamicTreeWidget::setInsertSubTreePredefine(int index)
{
  m_insertPatternTextEdit->setPlainText(m_insertSubTreePredefines->itemData(index).toString());
}


#include "dynamictreewidget.moc"
