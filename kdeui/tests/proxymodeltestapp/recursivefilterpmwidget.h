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

#ifndef RECURSIVE_PM_WIDGET_H
#define RECURSIVE_PM_WIDGET_H

#include <QWidget>

#include "recursivefilterproxymodel.h"

class QTreeView;

class DynamicTreeModel;


class TestRecursionModel : public KRecursiveFilterProxyModel
{
  Q_OBJECT
public:
  TestRecursionModel(QObject* parent = 0);

  /* reimp */ bool acceptRow(int sourceRow, const QModelIndex &sourceParent) const;

};

class RecursiveFilterProxyWidget : public QWidget
{
  Q_OBJECT
public:
  RecursiveFilterProxyWidget(QWidget* parent = 0);

private:
  DynamicTreeModel *m_rootModel;

};

#endif

