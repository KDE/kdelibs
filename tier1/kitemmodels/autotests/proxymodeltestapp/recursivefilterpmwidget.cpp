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
#include <QSplitter>
#include <QTreeView>
#include <QHBoxLayout>
#include <QLabel>
#include <QPlainTextEdit>

#include "dynamictreemodel.h"
#include "dynamictreewidget.h"

RecursiveFilterProxyWidget::RecursiveFilterProxyWidget(QWidget* parent)
  : QWidget(parent),
    m_lineEdit(new QLineEdit(this)),
    m_label(new QLabel(this))
{
  m_label->setText("Matching filter re: ");
  m_lineEdit->setText("12|13|37|4");

  QHBoxLayout *hLayout = new QHBoxLayout();
  QVBoxLayout *vLayout = new QVBoxLayout(this);
  QSplitter *splitter = new QSplitter(this);

  m_rootModel = new DynamicTreeModel(this);
  m_recursive = new KRecursiveFilterProxyModel(this);
  m_recursiveSubclass = new KRecursiveFilterProxyModelSubclass(this);

  DynamicTreeWidget *dynamicTreeWidget = new DynamicTreeWidget(m_rootModel, splitter);

  QTreeView *recursiveView = new QTreeView(splitter);
  recursiveView->setModel(m_recursive);
  QTreeView *recursiveSubclassView = new QTreeView(splitter);
  recursiveSubclassView->setModel(m_recursiveSubclass);

  hLayout->addWidget(m_label);
  hLayout->addWidget(m_lineEdit);

  vLayout->addLayout(hLayout);
  vLayout->addWidget(splitter);

  connect(m_lineEdit, SIGNAL(textChanged(QString)), SLOT(reset()));

  connect(m_lineEdit, SIGNAL(textChanged(QString)), recursiveView, SLOT(expandAll()));
  connect(m_lineEdit, SIGNAL(textChanged(QString)), recursiveSubclassView, SLOT(expandAll()));
  connect(dynamicTreeWidget->textEdit(), SIGNAL(textChanged()), recursiveView, SLOT(expandAll()));
  connect(dynamicTreeWidget->textEdit(), SIGNAL(textChanged()), recursiveSubclassView, SLOT(expandAll()));
  connect(m_recursive, SIGNAL(modelReset()), recursiveView, SLOT(expandAll()), Qt::QueuedConnection);
  connect(m_recursiveSubclass, SIGNAL(modelReset()), recursiveSubclassView, SLOT(expandAll()), Qt::QueuedConnection);

  m_recursive->setSourceModel(m_rootModel);
  m_recursiveSubclass->setSourceModel(m_rootModel);

  reset();
}

void RecursiveFilterProxyWidget::reset()
{
  m_recursive->setFilterRegExp(m_lineEdit->text());
  m_recursiveSubclass->setRegExp(QRegExp(m_lineEdit->text()));
}
