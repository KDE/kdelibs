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

#ifndef SELECTIONPROXYMODEL_H
#define SELECTIONPROXYMODEL_H

#include <QAbstractProxyModel>

#include "kdeui_next_export.h"

class QItemSelectionModel;

class KSelectionProxyModelPrivate;

/**
@brief A Proxy Model which presents a subset of its source model to observers.

The KSelectionProxyModel is most useful as a convenience for displaying the selection in one view in another view.

For example, when a user clicks a mail folder in one view in an email applcation, the contained emails should be displayed in another view.

@code

MyModel *sourceModel = new MyModel(this);
QTreeView *leftView = new QTreeView(this);
leftView->setModel(sourceModel);

KSelectionProxyModel *selectionProxy = new KSelectionProxyModel(leftView->selectionModel(), this);

QTreeView *rightView = new QTreeView(this);
rightView->setModel(selectionProxy);
@endcode

This takes away the need for the developer to handle the selection between the views, including all the mapToSource, mapFromSource and setRootIndex calls.

Additionally, this class can be used to programmatically choose some items from the source model to display in the view. For example,
this is how KMails Favourite folder View works, and how the AmazingCompleter works.

@since 4.4
@author Stephen Kelly <steveire@gmail.com>

*/
class KDEUI_NEXT_EXPORT KSelectionProxyModel : public QAbstractProxyModel
{
  Q_OBJECT
public:
  /**
  ctor.

  @p selectionModel The selection model used to filter what is presented by the proxy.
  */
  
  explicit KSelectionProxyModel(QItemSelectionModel *selectionModel, QObject *parent = 0 );

  /**
  dtor
  */
  virtual ~KSelectionProxyModel();

  /**
  reimp.
  */
  virtual void setSourceModel ( QAbstractItemModel * sourceModel );

  QItemSelectionModel *selectionModel() const;

  enum FilterBehavior
  {
    SelectedBranches,
    SelectedBranchesRoots,
    SelectedBranchesChildren,
    OnlySelected,
    OnlySelectedChildren
  };
  Q_ENUMS(FilterBehavior);

  /**
    Set the filter behaviors of this model.
    The filter behaviors of the model govern the content of the model based on the selection of the contained QItemSelectionModel.

    See kdeui/proxymodeltestapp to try out the different proxy model behaviors.

    The most useful behaviors are ExclusiveRoots, OnlySelected and OnlySelectedChildren.

    The default behavior is ExclusiveRoots. This means that this proxy model will contain the roots of the items in the source model.
    Any descendants which are also selected have no additional effect.
    For example if the source model is like:

    @verbatim
    (root)
      - A
      - B
        - C
        - D
          - E
            - F
          - G
      - H
      - I
        - J
        - K
        - L
    @endverbatim

    And A, B and D are selected, the proxy will contain:

    @verbatim
    (root)
      - A
      - B
        - C
        - D
          - E
            - F
          - G
    @endverbatim

    That is, selecting 'D' or 'C' if 'B' is also selected has no effect. If 'B' is de-selected, then 'C' amd 'D' become top-level items:

    @verbatim
    (root)
      - A
      - C
      - D
        - E
          - F
        - G
    @endverbatim

    This is the behavior used by KJots when rendering books.

    If the behavior is set to OmitChildren, then the children of selected indexes are not part of the model. If 'A', 'B' and 'D' are selected,

    @verbatim
    (root)
      - A
      - B
    @endverbatim

    Note that although 'D' is selected, it is not part of the proxy model, because its parent 'B' is already selected.
    In most cases this will be combined with IncludeAllSelected (see below).

    OmitGrandChildren has the effect of showing the children of selected items, but not their grandchildren or further descendants. Again, if 'A', 'B', 'D' and 'E' are selected,

    @verbatim
    (root)
      - A
      - B
        - C
        - D
    @endverbatim

    Note that although 'E' is selected, it has no effect because its ancestor 'B' is also selected. (Keep reading :))

    StartWithChildTrees has the effect of not making the selected items part of the model, but making their children part of the model instead. If 'A', 'B' and 'I' are selected:

    @verbatim
    (root)
      - C
      - D
        - E
          - F
        - G
      - J
      - K
      - L
    @endverbatim

    Note that 'A' has no children, so selecting it has no effect on the model. This can be used together with OmitGrandChildren to get the following effect with the same selection:

    @verbatim
    (root)
      - C
      - D
      - J
      - K
      - L
    @endverbatim
    
    Note that selecting 'E' in this behavior would have no effect because its ancestor 'B' is already in the model. The ChildrenOfSelected behavior is provided for convenience.

    IncludeAllSelected has the effect of including all selected items in the tree even if an ancestor is already in the tree.
    This behavior can not be used on its own because it would cause duplicates in the proxy.
    It must be combined with either OmitGrandChildren or ChildrenOfSelected. The OnlySelected and OnlySelectedChildren behaviors are provide for convenience.

    The OnlySelectedChildren has an effect similar to ChilrenOfSelected. The difference is that children of selected items appear in the model even if their children are already
    part of the model. For example, if 'A', 'B', 'D' and 'I' are selected:

    @verbatim
    (root)
      - C
      - D
      - E
      - G
      - J
      - K
      - L
    @endverbatim    

    This would be useful for example if showing containers (for example maildirs) in one view and their items in another. Sub-maildirs would still appear in the proxy, but
    could be filtered out using a QSortfilterProxyModel.

    The OnlySelected behavior causes the selected items to be part of the proxy model, even if their descendants are already selected, but children of selected items are not included.

    Again, if 'A', 'B', 'D' and 'I' are selected:

    @verbatim
    (root)
      - A
      - B
      - D
      - I
    @endverbatim

    This is the behavior used by the Favourite Folder View in KMail.

  */
  void setFilterBehavior(FilterBehavior behavior);
  FilterBehavior filterBehavior() const;

  QModelIndex mapFromSource ( const QModelIndex & sourceIndex ) const;
  QModelIndex mapToSource ( const QModelIndex & proxyIndex ) const;

  virtual Qt::ItemFlags flags( const QModelIndex &index ) const;
  QVariant data( const QModelIndex & index, int role = Qt::DisplayRole ) const;
  virtual int rowCount( const QModelIndex & parent = QModelIndex() ) const;
  virtual QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;

  virtual bool hasChildren ( const QModelIndex & parent = QModelIndex() ) const;
  virtual QModelIndex index(int, int, const QModelIndex& = QModelIndex() ) const;
  virtual QModelIndex parent(const QModelIndex&) const;
  virtual int columnCount(const QModelIndex& = QModelIndex() ) const;

private:
  Q_DECLARE_PRIVATE(KSelectionProxyModel)
  //@cond PRIVATE
  KSelectionProxyModelPrivate *d_ptr;

  Q_PRIVATE_SLOT(d_func(), void sourceRowsAboutToBeInserted(const QModelIndex &, int, int))
  Q_PRIVATE_SLOT(d_func(), void sourceRowsInserted(const QModelIndex &, int, int))
  Q_PRIVATE_SLOT(d_func(), void sourceRowsAboutToBeRemoved(const QModelIndex &, int, int))
  Q_PRIVATE_SLOT(d_func(), void sourceRowsRemoved(const QModelIndex &, int, int))
  Q_PRIVATE_SLOT(d_func(), void sourceRowsAboutToBeMoved(const QModelIndex &, int, int, const QModelIndex &, int))
  Q_PRIVATE_SLOT(d_func(), void sourceRowsMoved(const QModelIndex &, int, int, const QModelIndex &, int))
  Q_PRIVATE_SLOT(d_func(), void sourceModelAboutToBeReset())
  Q_PRIVATE_SLOT(d_func(), void sourceModelReset())
  Q_PRIVATE_SLOT(d_func(), void sourceLayoutAboutToBeChanged())
  Q_PRIVATE_SLOT(d_func(), void sourceLayoutChanged())
  Q_PRIVATE_SLOT(d_func(), void sourceDataChanged(const QModelIndex &, const QModelIndex &))
  Q_PRIVATE_SLOT(d_func(), void selectionChanged( const QItemSelection & selected, const QItemSelection & deselected ) )

  //@endcond

};

#endif
