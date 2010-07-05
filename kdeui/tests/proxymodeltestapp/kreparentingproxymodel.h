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

#ifndef KREPARENTINGPROXYMODEL_H
#define KREPARENTINGPROXYMODEL_H

#include <QAbstractProxyModel>

class KReparentingProxyModelPrivate;

/**
  @brief Restructures a source model, changing the parents of items.

  Subclasses can change the structure of a source model by reimplementing
  the isDescendantOf method.

  For example, if the source model is a list,

  @verbatim
  0
  - A
  - B
  - C
  - D
  - E
  @endverbatim

  It could be converted to a tree by an implementation something like:

  @code
  bool MyReparentingModel::isDescedantOf(const QModelIndex& ancestor, const QModelIndex& descendant ) const
  {
     return (
             (ancestor.data().toString() == "A" && descendant.data().toString() == "B")
          || (ancestor.data().toString() == "A" && descendant.data().toString() == "C")
          || (ancestor.data().toString() == "B" && descendant.data().toString() == "C")
          || (ancestor.data().toString() == "A" && descendant.data().toString() == "D")
          )
    ? true : KReparentingProxyModel::isDescendantOf(ancestor, descendant);
  }
  @endcode

  to get this result:

  @verbatim
  0
  - A
  - - B
  - - - C
  - - D
  - E
  @endverbatim

  Note that the implementation returns true for a query if "C" is a descendant of "A".
  The implementation must return the correct value for all of its descendants, not only its direct parent.
  The actual location to insert the descendant in the tree is determined internally by a binary find algorithm.
*/
class KReparentingProxyModel : public QAbstractProxyModel
{
  Q_OBJECT
public:
  KReparentingProxyModel(QObject* parent = 0);

  virtual QModelIndex mapFromSource(const QModelIndex& sourceIndex) const;

  virtual QModelIndex mapToSource(const QModelIndex& proxyIndex) const;

  virtual void setSourceModel(QAbstractItemModel* sourceModel);

  /**
    Reimplement this to return whether @p descendant is a descendant of @p ancestor.
  */
  virtual bool isDescendantOf(const QModelIndex& ancestor, const QModelIndex& descendant ) const;

  virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;

  virtual QVariant data(const QModelIndex& proxyIndex, int role = Qt::DisplayRole) const;

  virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const;

  virtual QModelIndex parent(const QModelIndex& child) const;

  virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;

  virtual bool hasChildren(const QModelIndex& parent = QModelIndex()) const;

  virtual Qt::DropActions supportedDropActions() const;

  protected:
    void beginChangeRule();
    void endChangeRule();

private:
  Q_DECLARE_PRIVATE(KReparentingProxyModel)
  //@cond PRIVATE
  KReparentingProxyModelPrivate *d_ptr;

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

  //@endcond


};


#endif
