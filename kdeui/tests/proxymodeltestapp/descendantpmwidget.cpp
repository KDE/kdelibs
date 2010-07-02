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

#include "descendantpmwidget.h"

#include <QTreeView>
#include <QSplitter>

#include "dynamictreemodel.h"
#include "dynamictreewidget.h"
#include "kdescendantsproxymodel.h"
#include <QHBoxLayout>
#include <QLineEdit>
#include <QApplication>

#include "modeleventlogger.h"

DescendantProxyModelWidget::DescendantProxyModelWidget(QWidget* parent): QWidget(parent)
{
  QHBoxLayout *layout = new QHBoxLayout(this);
  QSplitter *vSplitter = new QSplitter( this );
  layout->addWidget(vSplitter);

  m_rootModel = new DynamicTreeModel(this);

  DynamicTreeWidget *dynTreeWidget = new DynamicTreeWidget(m_rootModel, vSplitter);

  dynTreeWidget->setInitialTree(
 "- 1"
 "- 2"
 "- - 3"
 "- - 3"
 "- - - 4"
 "- - - 4"
 "- - - - 4"
 "- - 4"
 "- - 5"
 "- - - 4"
 "- - - - 4"
 "- - 5"
 "- 6"
 "- 7"
 "- - 8"
 "- - - 9"
 "- - - 10"
 "- - - - 9"
 "- - - - - 10"
 "- - - - - - 9"
 "- - - - - - 10"
 "- - - - - - - 9"
 "- - - - - - - - 10"
 "- - - - - - - - 9"
 "- - - - - - - 10"
 "- - - - - 9"
 "- - - - - 9"
 "- - - - - 9"
 "- - - - - 10"
 "- - - - - - 9"
 "- - - - - - 10"
 "- - - - - 9"
 "- - - - - 9"
 "- - - - - 9"
 "- - - - - 10"
 "- - - - - - 9"
 "- - - - - - 10"
 "- - - - 10"
 "- - 11"
 "- - 12"
 "- 13"
 "- 14"
 "- 15"
 "- - 16"
 "- - - 17"
 "- - - 18"
 "- 19"
 "- 20"
 "- 21");

  m_eventLogger = new ModelEventLogger(m_rootModel, this);

  m_descProxyModel = new KDescendantsProxyModel(this);
  m_descProxyModel->setSourceModel(m_rootModel);

//   KDescendantsProxyModel *descProxyModel2 = new KDescendantsProxyModel(this);
//   descProxyModel2->setSourceModel(m_rootModel);
//   descProxyModel2->setDisplayAncestorData(true);

//   QTreeView *treeview = new QTreeView( vSplitter );
//   treeview->setModel(m_rootModel);
//   treeview->setSelectionMode(QAbstractItemView::ExtendedSelection);

  m_descView = new QTreeView( vSplitter );
  m_descView->setModel(m_descProxyModel);

//   QTreeView *descView2 = new QTreeView( vSplitter );
//   descView2->setModel(descProxyModel2);


//   QWidget *w = new QWidget(vSplitter);
//   QVBoxLayout *vLayout = new QVBoxLayout(w);
//   QTreeView *matchView = new QTreeView(w);
//   matchView->setModel(m_selectionProxyModel);
//   m_lineEdit = new QLineEdit(w);
//   connect(m_lineEdit, SIGNAL(textChanged(const QString &)), SLOT(doMatch(const QString &)));
//   connect(m_descView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), SLOT(refreshMatch()));

//   vLayout->addWidget(m_lineEdit);
//   vLayout->addWidget(matchView);

  setLayout(layout);
}

DescendantProxyModelWidget::~DescendantProxyModelWidget()
{
}


void DescendantProxyModelWidget::doMatch(const QString &matchData)
{
  Q_UNUSED(matchData);
#if 0
  m_itemSelectionModel->clearSelection();

  if (matchData.isEmpty())
    return;

  QModelIndex start = m_descView->currentIndex();

  if (!start.isValid())
    start = m_descProxyModel->index(0, 0);

  // TODO: get from user.
  int hits = -1;

  QModelIndexList matches = m_descProxyModel->match(start, Qt::DisplayRole, matchData, hits, Qt::MatchContains);

  foreach(const QModelIndex &matchingIndex, matches)
  {
    m_itemSelectionModel->select(matchingIndex, QItemSelectionModel::Select | QItemSelectionModel::Rows);
  }
#endif
}


void DescendantProxyModelWidget::refreshMatch()
{
  doMatch(m_lineEdit->text());
}


