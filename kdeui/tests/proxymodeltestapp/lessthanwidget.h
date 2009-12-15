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

#ifndef LESSTHANWIDGET_H
#define LESSTHANWIDGET_H

#include <QWidget>
#include <QItemSelectionModel>

#include "dynamictreemodel.h"


class ColoredTreeModel : public DynamicTreeModel
{
  Q_OBJECT
public:
  ColoredTreeModel(QObject* parent = 0);

  /* reimp */ QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;

  void setSelectionModel(QItemSelectionModel *selectionModel);

protected slots:
  void recolor(const QModelIndex &parent = QModelIndex());

private:
  QItemSelectionModel *m_selectionModel;
  QColor m_lessThanColour;
  QColor m_greaterThanColour;
};

class LessThanWidget : public QWidget
{
  Q_OBJECT
public:
  LessThanWidget(QWidget* parent = 0, Qt::WindowFlags f = 0);


  void insertGrid(QList<int> address);
private:
  ColoredTreeModel *m_coloredTreeModel;
};

#endif
