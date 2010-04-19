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

#ifndef KPROXYITEMSELECTIONMODEL_H
#define KPROXYITEMSELECTIONMODEL_H

#include <QtGui/QItemSelectionModel>
#include <QtGui/QAbstractProxyModel>

#include "kdeui_export.h"

#include "kdebug.h"

class KProxyItemSelectionModelPrivate;

/**
  @brief Makes it possible to share a selection in multiple views which do not have the same source model

  Although <a href="http://doc.trolltech.com/4.6/model-view-view.html#sharing-selections-between-views">multiple views can share the same QItemSelectionModel</a>, the views then need to have the same source model.

  If there is a proxy model between the model and one of the views, or different proxy models in each, this class makes
  it possible to share the selection between the views.

  @image html kproxyitemselectionmodel-simple.png "Sharing a QItemSelectionModel between views on the same model is trivial"
  @image html kproxyitemselectionmodel-error.png "If a proxy model is used, it is no longer possible to share the QItemSelectionModel directly"
  @image html kproxyitemselectionmodel-solution.png "A KProxyItemSelectionModel can be used to map the selection through the proxy model"
  @image html kproxyitemselectionmodel-complex.png "Arbitrarily complex proxy configurations on the same root model can be used"

  @since 4.5

*/
class KDEUI_EXPORT KProxyItemSelectionModel : public QItemSelectionModel
{
  Q_OBJECT
public:
  /**
    Constructor.
  */
  KProxyItemSelectionModel( QAbstractItemModel *targetModel, QItemSelectionModel *proxySelector, QObject *parent = 0);

  /* reimp */ void select(const QModelIndex &index, QItemSelectionModel::SelectionFlags command);
  /* reimp */ void select(const QItemSelection &selection, QItemSelectionModel::SelectionFlags command);

private:
  /**
    Maps the @p sourceIndex though the proxy models to a selection in the proxySelector
  */
  QItemSelection mapSelectionFromSource( const QModelIndex &sourceIndex ) const;
  QItemSelection mapSelectionFromSource( const QItemSelection &sourceSelection ) const;
  QItemSelection mapSelectionToSource( const QModelIndex &sourceIndex ) const;
  QItemSelection mapSelectionToSource( const QItemSelection &sourceSelection ) const;

private slots:
  void sourceSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);

protected:
  KProxyItemSelectionModelPrivate * const d_ptr;

private:
  Q_DECLARE_PRIVATE(KProxyItemSelectionModel)
};

#endif
