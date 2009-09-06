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
#include <QPushButton>

#include "dynamictreemodel.h"
#include <QHBoxLayout>

CustomReparent::CustomReparent(QObject* parent)
    : KReparentingProxyModel(parent)
{

}

bool CustomReparent::isDescendantOf(const QModelIndex& ancestor, const QModelIndex& descendant) const
{
  bool r = (
             (ancestor.data().toString() == "1" && descendant.data().toString() == "3")
          || (ancestor.data().toString() == "1" && descendant.data().toString() == "5")
          || (ancestor.data().toString() == "1" && descendant.data().toString() == "7")
          || (ancestor.data().toString() == "1" && descendant.data().toString() == "9")
          || (ancestor.data().toString() == "3" && descendant.data().toString() == "5")
          || (ancestor.data().toString() == "3" && descendant.data().toString() == "7")
          || (ancestor.data().toString() == "13" && descendant.data().toString() == "15")

          || (ancestor.data().toString() == "21" && descendant.data().toString() == "1")
          || (ancestor.data().toString() == "21" && descendant.data().toString() == "11")
          || (ancestor.data().toString() == "21" && descendant.data().toString() == "13")
          || (ancestor.data().toString() == "21" && descendant.data().toString() == "17")

          || (ancestor.data().toString() == "21" && descendant.data().toString() == "23")

          || (ancestor.data().toString() == "23" && descendant.data().toString() == "1")
          || (ancestor.data().toString() == "23" && descendant.data().toString() == "11")


          )
    ? true : KReparentingProxyModel::isDescendantOf(ancestor, descendant);

//     kDebug() << "###" << r << ancestor.data() << descendant.data();
    return r;
}

ReparentingProxyModelWidget::ReparentingProxyModelWidget(QWidget* parent): QWidget(parent)
{
  QVBoxLayout *layout = new QVBoxLayout(this);

  m_button = new QPushButton("Add more", this);
  layout->addWidget(m_button);

  QSplitter *vSplitter = new QSplitter( this );
  layout->addWidget(vSplitter);

  m_numCols = 2;
  m_rootModel = new DynamicTreeModel(this);

  m_rootView = new QTreeView( vSplitter );
  m_rootView->setModel(m_rootModel);
  m_rootView->setSelectionMode(QAbstractItemView::ExtendedSelection);


  m_reparentingProxyModel = new CustomReparent(this);
  m_reparentingProxyModel->setSourceModel(m_rootModel);

  m_reparentingView = new QTreeView( vSplitter );
  m_reparentingView->setModel( m_reparentingProxyModel );

  setLayout(layout);

  reset();
}

void ReparentingProxyModelWidget::reset()
{
  m_rootModel->clear();

  disconnect(m_button, SIGNAL(clicked(bool)), this, SLOT(reset()));
  ModelInsertCommand *insertCommand;
  int max_runs = 1;
  for (int i = 0; i < max_runs; i++)
  {
    insertCommand = new ModelInsertCommand(m_rootModel, this);
    insertCommand->setNumCols(m_numCols);
    insertCommand->setStartRow(0);
    insertCommand->setEndRow(9);
    insertCommand->doCommand();
  }

  m_reparentingView->expandAll();

  m_button->setText("Add more");
  connect(m_button, SIGNAL(clicked(bool)), SLOT(insertNewParent()));

}

void ReparentingProxyModelWidget::insertNewParent()
{

  disconnect(m_button, SIGNAL(clicked(bool)), this, SLOT(insertNewParent()));
  ModelInsertCommand *insertCommand = new ModelInsertCommand(m_rootModel, this);
  insertCommand->setNumCols(m_numCols);
  insertCommand->setStartRow(0);
  insertCommand->setEndRow(1);
  insertCommand->doCommand();
  m_reparentingView->expandAll();

  m_button->setText("Reset");
  connect(m_button, SIGNAL(clicked(bool)), SLOT(reset()));

}

#include "reparentingpmwidget.moc"

