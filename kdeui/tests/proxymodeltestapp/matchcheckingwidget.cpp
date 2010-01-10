/*
    Copyright (c) 2010 Stephen Kelly <steveire@gmail.com>

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

#include "matchcheckingwidget.h"

#include <QVBoxLayout>
#include <QLineEdit>
#include <QSplitter>
#include <QTreeView>
#include <QRadioButton>

#include "dynamictreemodel.h"
#include "dynamictreewidget.h"
#include <kselectionproxymodel.h>

MatchCheckingWidget::MatchCheckingWidget(QWidget* parent, Qt::WindowFlags f)
  : QWidget(parent, f)
{
  QVBoxLayout *layout = new QVBoxLayout(this);

  m_lineEdit = new QLineEdit();

  connect(m_lineEdit, SIGNAL(textChanged(QString)), SLOT(matchChanged(QString)));


  m_dynamicTreeRadioButton = new QRadioButton("Dynamic Tree Model", this);
  m_selectionModelRadioButton = new QRadioButton("Selection Model", this);

  layout->addWidget(m_lineEdit);
  layout->addWidget(m_dynamicTreeRadioButton);
  layout->addWidget(m_selectionModelRadioButton);

  QSplitter *splitter = new QSplitter(this);
  layout->addWidget(splitter);
  DynamicTreeModel *dynamicTreeModel = new DynamicTreeModel(this);

  m_dynamicTreeWidget = new DynamicTreeWidget(dynamicTreeModel, this);

  splitter->addWidget(m_dynamicTreeWidget);

  KSelectionProxyModel *selectionProxyModel = new KSelectionProxyModel(m_dynamicTreeWidget->treeView()->selectionModel(), this);
  selectionProxyModel->setSourceModel(dynamicTreeModel);

  m_selectionTreeView = new QTreeView(this);
  m_selectionTreeView->setModel(selectionProxyModel);
  splitter->addWidget(m_selectionTreeView);
}

void MatchCheckingWidget::matchChanged(const QString& matchData)
{
  bool ok;
  int id = matchData.toInt(&ok);
kDebug() << matchData << id <<  DynamicTreeModel::DynamicTreeModelId;
  if (!ok)
    return;

  QModelIndexList list;
  if (m_dynamicTreeRadioButton->isChecked())
  {
    m_dynamicTreeWidget->treeView()->selectionModel()->clearSelection();
    list = m_dynamicTreeWidget->model()->match(m_dynamicTreeWidget->model()->index(0, 0), DynamicTreeModel::DynamicTreeModelId, id);
    kDebug() << list;
    foreach (const QModelIndex &idx, list)
    {
      m_dynamicTreeWidget->treeView()->selectionModel()->select(idx, QItemSelectionModel::SelectCurrent);
    }
  } else if (m_selectionModelRadioButton->isChecked())
  {
    m_selectionTreeView->selectionModel()->clearSelection();
    list = m_selectionTreeView->model()->match(m_selectionTreeView->model()->index(0, 0), DynamicTreeModel::DynamicTreeModelId, id);
    kDebug() << list;
    foreach (const QModelIndex &idx, list)
    {
      m_selectionTreeView->selectionModel()->select(idx, QItemSelectionModel::SelectCurrent);
    }
  }
}

#include "matchcheckingwidget.moc"
