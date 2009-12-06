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

#include <QLineEdit>
#include <QPushButton>
#include <QSplitter>
#include <QTreeView>
#include <QHBoxLayout>
#include <QLabel>

#include "dynamictreemodel.h"
#include <qstandarditemmodel.h>

RecursiveFilterProxyWidget::RecursiveFilterProxyWidget(QWidget* parent)
  : QWidget(parent),
    m_lineEdit(new QLineEdit(this)),
    m_label(new QLabel(this)),
    m_pushButton(new QPushButton(this))
{
  m_label->setText("Matching filter re: ");
  m_lineEdit->setText("12|13|37|4");

  QHBoxLayout *hLayout = new QHBoxLayout();
  QVBoxLayout *vLayout = new QVBoxLayout(this);
  QSplitter *splitter = new QSplitter(this);

  m_rootModel = new DynamicTreeModel(this);
  m_recursive = new KRecursiveFilterProxyModel(this);
  m_recursiveSubclass = new KRecursiveFilterProxyModelSubclass(this);

  QStandardItemModel *stdItemModel = new QStandardItemModel(this);
  QSortFilterProxyModel *qsf = new QSortFilterProxyModel(this);
  QSortFilterProxyModel *qsf2 = new QSortFilterProxyModel(this);


  QTreeView *rootView = new QTreeView(splitter);
  rootView->setModel(stdItemModel);
  QTreeView *recursiveView = new QTreeView(splitter);
  recursiveView->setModel(qsf);
  QTreeView *recursiveSubclassView = new QTreeView(splitter);
  recursiveSubclassView->setModel(m_recursiveSubclass);

  hLayout->addWidget(m_label);
  hLayout->addWidget(m_lineEdit);
  hLayout->addWidget(m_pushButton);

  vLayout->addLayout(hLayout);
  vLayout->addWidget(splitter);

  connect(m_lineEdit, SIGNAL(textChanged(QString)), SLOT(reset()));
  connect(m_pushButton, SIGNAL(clicked(bool)), SLOT(actionClicked()));


//   m_recursive->setSourceModel(m_rootModel);
  qsf2->setSourceModel(m_rootModel);
//   qsf->setSourceModel(stdItemModel);

  QStandardItem *topLevel = new QStandardItem("r");
  stdItemModel->appendRow(topLevel);
  topLevel->appendRow( new QStandardItem("a") );

//   m_recursiveSubclass->setSourceModel(m_rootModel);

//   reset();

  QList<int> ancestorRows;
  ModelInsertCommand *ins;
  int max_runs = 2;

  for (int i = 0; i < max_runs; i++)
  {
    ins = new ModelInsertCommand(m_rootModel, this);
    ins->setAncestorRowNumbers(ancestorRows);
    ins->setStartRow(0);
    ins->setEndRow(0);
    ins->doCommand();
    ancestorRows << 0;
  }

//   m_rootModel->clear();
//
//   ins = new ModelInsertCommand(m_rootModel, this);
//   ins->setStartRow(0);
//   ins->setEndRow(4);
//   ins->doCommand();
}

void RecursiveFilterProxyWidget::reset()
{
  m_rootModel->clear();

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

  m_recursive->setFilterRegExp(m_lineEdit->text());
//   m_recursiveSubclass->setRegExp(QRegExp(m_lineEdit->text()));

  m_nextAction = InsertAction;
  m_pushButton->setText("Insert");
}

void RecursiveFilterProxyWidget::insertRows()
{
  QList<int> ancestorRows;
  ModelInsertCommand *ins;

  ancestorRows << 3 << 3 << 2;
  ins = new ModelInsertCommand(m_rootModel, this);
  ins->setAncestorRowNumbers(ancestorRows);
  ins->setStartRow(0);
  ins->setEndRow(4);
  ins->doCommand();

  m_nextAction = RemoveAction;
  m_pushButton->setText("Remove");
}

void RecursiveFilterProxyWidget::insertRows2()
{
  QList<int> ancestorRows;
  ModelInsertCommand *ins;

  ancestorRows << 4;
  ins = new ModelInsertCommand(m_rootModel, this);
  ins->setAncestorRowNumbers(ancestorRows);
  ins->setStartRow(0);
  ins->setEndRow(4);
  ins->doCommand();

  m_nextAction = ResetAction;
  m_pushButton->setText("Reset");
}

void RecursiveFilterProxyWidget::removeRows()
{
  QList<int> ancestorRows;

  ancestorRows << 3 << 3 << 2;
  ModelRemoveCommand *rem = new ModelRemoveCommand(m_rootModel, this);
  rem->setAncestorRowNumbers(ancestorRows);
  rem->setStartRow(0);
  rem->setEndRow(4);
  rem->doCommand();

  m_nextAction = Insert2Action;
  m_pushButton->setText("Insert2");
}

void RecursiveFilterProxyWidget::actionClicked()
{
  switch(m_nextAction)
  {
  case InsertAction:
  {
    insertRows();
    break;
  }
  case Insert2Action:
  {
    insertRows2();
    break;
  }
  case RemoveAction:
  {
    removeRows();
    break;
  }
  case ResetAction:
  {
    reset();
    break;
  }
  }
}

