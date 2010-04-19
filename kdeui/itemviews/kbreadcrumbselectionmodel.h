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

#ifndef KBREADCRUMBSPROXYMODEL_H
#define KBREADCRUMBSPROXYMODEL_H

#include <QtGui/QItemSelectionModel>
#include <QtCore/QAbstractItemModel>

#include "kdeui_export.h"

#include "kdebug.h"

class KBreadcrumbSelectionModelPrivate;

/**
  @class KBreadcrumbSelectionModel kbreadcrumbselectionmodel.h

  @brief Selects the parents of selected items to create breadcrumbs

  For example, if the tree is
  @verbatim
    - A
    - B
    - - C
    - - D
    - - - E
    - - - - F
  @endverbatim

  and E is selected, the selection can contain

  @verbatim
    - B
    - D
  @endverbatim

  or

  @verbatim
    - B
    - D
    - E
  @endverbatim

  if includeActualSelection is true.

  The depth of the selection may also be set. For example if the selectionDepth is 1:

  @verbatim
    - D
    - E
  @endverbatim

  And if selectionDepth is 2:

  @verbatim
    - B
    - D
    - E
  @endverbatim

  A KBreadcrumbsProxyModel with a selectionDepth of 0 and including the actual selection is
  the same as a KSelectionProxyModel in the KSelectionProxyModel::ExactSelection configuration.

  @code
    view1->setModel(rootModel);

    QItemSelectionModel *breadcrumbSelectionModel = new QItemSelectionModel(rootModel, this);

    KBreadcrumbSelectionModel *breadcrumbProxySelector = new KBreadcrumbSelectionModel(breadcrumbSelectionModel, rootModel, this);

    view1->setSelectionModel(breadcrumbProxySelector);

    KSelectionProxyModel *breadcrumbSelectionProxyModel = new KSelectionProxyModel( breadcrumbSelectionModel, this);
    breadcrumbSelectionProxyModel->setSourceModel( rootModel );
    breadcrumbSelectionProxyModel->setFilterBehavior( KSelectionProxyModel::ExactSelection );

    view2->setModel(breadcrumbSelectionProxyModel);
  @endcode

  @image html kbreadcrumbselectionmodel.png "KBreadcrumbSelectionModel in several configurations"

  This can work in two directions. One option is for a single selection in the KBreadcrumbSelectionModel to invoke
  the breadcrumb selection in its constructor argument.

  The other is for a selection in the itemselectionmodel in the constructor argument to cause a breadcrumb selection
  in @p this.

  @since 4.5

*/
class KDEUI_EXPORT KBreadcrumbSelectionModel : public QItemSelectionModel
{
  Q_OBJECT
public:
  enum Direction
  {
    Forward,
    Reverse
  };

  explicit KBreadcrumbSelectionModel(QItemSelectionModel *selectionModel, QObject* parent = 0);
  KBreadcrumbSelectionModel(QItemSelectionModel *selectionModel, Direction direction, QObject* parent = 0);
  virtual ~KBreadcrumbSelectionModel();

  /**
    Returns whether the actual selection in included in the proxy.

    The default is true.
  */
  bool includeActualSelection() const;

  /**
    Set whether the actual selection in included in the proxy to @p includeActualSelection.
  */
  void setIncludeActualSelection(bool includeActualSelection);

  /**
    Returns the depth that the breadcrumb selection should go to.
  */
  int selectionDepth() const;

  /**
    Sets the depth that the breadcrumb selection should go to.

    If the @p selectionDepth is -1, all breadcrumbs are selected.
    The default is -1
  */
  void setSelectionDepth(int selectionDepth);

  /* reimp */ void select(const QModelIndex &index, QItemSelectionModel::SelectionFlags command);

  /* reimp */ void select(const QItemSelection &selection, QItemSelectionModel::SelectionFlags command);

protected:
  /**
    Returns a selection containing the breadcrumbs for @p index
  */
  QItemSelection getBreadcrumbSelection(const QModelIndex &index);

  /**
    Returns a selection containing the breadcrumbs for @p selection
  */
  QItemSelection getBreadcrumbSelection(const QItemSelection &selection);

private slots:
  void sourceSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);

protected:
  KBreadcrumbSelectionModelPrivate * const d_ptr;
private:
  Q_DECLARE_PRIVATE(KBreadcrumbSelectionModel)
};


#endif
