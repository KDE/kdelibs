/*
    Copyright (C) 2010 Klarälvdalens Datakonsult AB,
        a KDAB Group company, info@kdab.net,
        author Stephen Kelly <stephen@kdab.com>

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

#ifndef BREADCRUMBNAVIGATION_WIDGET_H
#define BREADCRUMBNAVIGATION_WIDGET_H

#include <QtGui/QWidget>
#include <QItemSelection>
#include <QLabel>
#include <QListView>
#include <kselectionproxymodel.h>

#include "klinkitemselectionmodel.h"

class CurrentItemLabel : public QLabel
{
  Q_OBJECT
public:
  CurrentItemLabel(QAbstractItemModel *model, QWidget* parent = 0, Qt::WindowFlags f = 0);

private slots:
  void rowsInserted(const QModelIndex &parent, int start, int end);
  void rowsRemoved(const QModelIndex &parent, int start, int end);
  void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
  void modelReset();

private:
  QAbstractItemModel *m_model;
};

class KBreadcrumbNavigationProxyModel : public KSelectionProxyModel
{
  Q_OBJECT
public:
  KBreadcrumbNavigationProxyModel(QItemSelectionModel* selectionModel, QObject* parent = 0);

  void setShowHiddenAscendantData(bool showHiddenAscendantData);
  bool showHiddenAscendantData() const;

  virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;

private:
  bool m_showHiddenAscendantData;

};

class KNavigatingProxyModel : public KSelectionProxyModel
{
  Q_OBJECT
public:
  KNavigatingProxyModel(QItemSelectionModel* selectionModel, QObject* parent = 0);

  virtual void setSourceModel(QAbstractItemModel* sourceModel);

  virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;

private slots:
  void modelReset();
  void updateNavigation();
  void navigationSelectionChanged( const QItemSelection &, const QItemSelection & );

private:

private:
  using KSelectionProxyModel::setFilterBehavior;

  QItemSelectionModel *m_selectionModel;

};

class KForwardingItemSelectionModel : public QItemSelectionModel
{
  Q_OBJECT
public:
  enum Direction
  {
    Forward,
    Reverse
  };
  KForwardingItemSelectionModel(QAbstractItemModel* model, QItemSelectionModel *selectionModel, QObject *parent = 0);
  KForwardingItemSelectionModel(QAbstractItemModel* model, QItemSelectionModel *selectionModel, Direction direction, QObject *parent = 0);

  virtual void select(const QModelIndex& index, SelectionFlags command);
  virtual void select(const QItemSelection& selection, SelectionFlags command);

private slots:
  void navigationSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);

private:
  QItemSelectionModel *m_selectionModel;
  Direction m_direction;
};

class BreadcrumbNavigationWidget : public QWidget
{
  Q_OBJECT
public:
  BreadcrumbNavigationWidget(QWidget* parent = 0, Qt::WindowFlags f = 0);

};

#endif

