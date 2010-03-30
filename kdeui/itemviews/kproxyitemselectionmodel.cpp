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


#include "kproxyitemselectionmodel.h"

class KProxyItemSelectionModelPrivate
{
public:
  KProxyItemSelectionModelPrivate(KProxyItemSelectionModel *proxySelectionModel, QAbstractItemModel *model,
                                          QItemSelectionModel *selectionModel)
    : q_ptr( proxySelectionModel ), m_model(model), m_proxySelector(selectionModel)
  {
    createProxyChain();
  }

  void createProxyChain();

  bool assertValid();

  Q_DECLARE_PUBLIC(KProxyItemSelectionModel)
  KProxyItemSelectionModel * const q_ptr;

  QList<const QAbstractProxyModel *> m_proxyChainUp;
  QList<const QAbstractProxyModel *> m_proxyChainDown;

  QAbstractItemModel *m_model;
  QItemSelectionModel *m_proxySelector;
  bool m_ignoreCurrentChanged;
};

/*

  The idea here is that <tt>this</tt> selection model and proxySelectionModel might be in different parts of the
  proxy chain. We need to build up to two chains of proxy models to create mappings between them.

  Example 1:

     Root model
          |
        /    \
    Proxy 1   Proxy 3
       |       |
    Proxy 2   Proxy 4

  Need Proxy 1 and Proxy 2 in one chain, and Proxy 3 and 4 in the other.

  Example 2:

     Root model
          |
        Proxy 1
          |
        Proxy 2
        /     \
    Proxy 3   Proxy 6
       |       |
    Proxy 4   Proxy 7
       |
    Proxy 5

  We first build the chain from 1 to 5, then start building the chain from 7 to 1. We stop when we find that proxy 2 is
  already in the first chain.

  Stephen Kelly, 30 March 2010.
*/

void KProxyItemSelectionModelPrivate::createProxyChain()
{
  const QAbstractItemModel *selectionTargetModel = m_proxySelector->model();

  if (m_model == selectionTargetModel)
    return;

  QList<const QAbstractProxyModel *> proxyChainDown;

  const QAbstractProxyModel *selectionTargetProxyModel = qobject_cast<const QAbstractProxyModel*>( selectionTargetModel );

  while( selectionTargetProxyModel )
  {
    proxyChainDown.prepend( selectionTargetProxyModel );

    selectionTargetProxyModel = qobject_cast<const QAbstractProxyModel*>(selectionTargetProxyModel->sourceModel());

    if ( selectionTargetProxyModel == m_model )
    {
      m_proxyChainDown = proxyChainDown;
      return;
    }
  }

  const QAbstractItemModel *sourceModel = m_model;
  const QAbstractProxyModel *sourceProxyModel = qobject_cast<const QAbstractProxyModel*>( sourceModel );

  while( sourceProxyModel )
  {
    m_proxyChainUp.append( sourceProxyModel );

    sourceProxyModel = qobject_cast<const QAbstractProxyModel*>(sourceProxyModel->sourceModel());

    int targetIndex = proxyChainDown.indexOf(sourceProxyModel );

    if ( targetIndex != -1 )
    {
      m_proxyChainDown = proxyChainDown.mid(targetIndex, proxyChainDown.size());
      return;
    }
  }
  m_proxyChainDown = proxyChainDown;
  Q_ASSERT(assertValid());
}

bool KProxyItemSelectionModelPrivate::assertValid()
{
  qDebug() << m_proxyChainDown << m_proxyChainUp;
  if ( m_proxyChainDown.isEmpty())
  {
    Q_ASSERT( !m_proxyChainUp.isEmpty() );
    Q_ASSERT( m_proxyChainUp.last()->sourceModel() == m_proxySelector->model() );
  }
  else if ( m_proxyChainUp.isEmpty())
  {
    Q_ASSERT( !m_proxyChainDown.isEmpty() );
    Q_ASSERT( m_proxyChainDown.first()->sourceModel() == m_model );
  } else {
    Q_ASSERT( m_proxyChainDown.first()->sourceModel() == m_proxyChainUp.last()->sourceModel() );
  }
  return true;
}

KProxyItemSelectionModel::KProxyItemSelectionModel( QAbstractItemModel *model, QItemSelectionModel *proxySelector, QObject *parent)
  : QItemSelectionModel(model, parent),
    d_ptr(new KProxyItemSelectionModelPrivate(this, model, proxySelector))
{
  connect(proxySelector, SIGNAL(selectionChanged(QItemSelection,QItemSelection)), SLOT(sourceSelectionChanged(QItemSelection,QItemSelection)));
}

QItemSelection KProxyItemSelectionModel::mapSelectionFromSource(const QModelIndex& sourceIndex) const
{
  return mapSelectionFromSource(QItemSelection(sourceIndex, sourceIndex));
}

QItemSelection KProxyItemSelectionModel::mapSelectionFromSource(const QItemSelection& sourceSelection) const
{
  Q_D(const KProxyItemSelectionModel);

  QItemSelection seekSelection = sourceSelection;
  QListIterator<const QAbstractProxyModel*> iUp(d->m_proxyChainUp);

  while (iUp.hasNext())
  {
    const QAbstractProxyModel *proxy = iUp.next();
    seekSelection = proxy->mapSelectionToSource(seekSelection);
  }

  QListIterator<const QAbstractProxyModel*> iDown(d->m_proxyChainDown);

  while (iDown.hasNext())
  {
    const QAbstractProxyModel *proxy = iDown.next();
    seekSelection = proxy->mapSelectionFromSource(seekSelection);
  }

  Q_ASSERT( ( !seekSelection.isEmpty() && seekSelection.first().model() == d->m_proxySelector->model() ) || true );
  return seekSelection;
}

QItemSelection KProxyItemSelectionModel::mapSelectionToSource(const QModelIndex& sourceIndex) const
{
  return mapSelectionToSource(QItemSelection(sourceIndex, sourceIndex));
}

QItemSelection KProxyItemSelectionModel::mapSelectionToSource(const QItemSelection& proxySelection) const
{
  Q_D(const KProxyItemSelectionModel);

  QItemSelection seekSelection = proxySelection;
  QListIterator<const QAbstractProxyModel*> iDown(d->m_proxyChainDown);

  iDown.toBack();
  while (iDown.hasPrevious())
  {
    const QAbstractProxyModel *proxy = iDown.previous();
    seekSelection = proxy->mapSelectionToSource(seekSelection);
  }

  QListIterator<const QAbstractProxyModel*> iUp(d->m_proxyChainUp);

  iUp.toBack();
  while (iUp.hasPrevious())
  {
    const QAbstractProxyModel *proxy = iUp.previous();
    seekSelection = proxy->mapSelectionFromSource(seekSelection);
  }

  Q_ASSERT( ( !seekSelection.isEmpty() && seekSelection.first().model() == d->m_model ) || true );
  return seekSelection;
}

void KProxyItemSelectionModel::select(const QModelIndex &index, QItemSelectionModel::SelectionFlags command)
{
  Q_D(KProxyItemSelectionModel);
  // When an item is removed, the current index is set to the top index in the model.
  // That causes a selectionChanged signal with a selection which we do not want.
  if ( d->m_ignoreCurrentChanged )
  {
    return;
  }
  QItemSelectionModel::select(index, command);
  if (index.isValid())
    d->m_proxySelector->select(mapSelectionFromSource(index), command);
  else
  {
    d->m_proxySelector->clearSelection();
  }
}

void KProxyItemSelectionModel::select(const QItemSelection &selection, QItemSelectionModel::SelectionFlags command)
{
  Q_D(KProxyItemSelectionModel);
  d->m_ignoreCurrentChanged = true;
  QItemSelectionModel::select(selection, command);
  d->m_proxySelector->select(mapSelectionFromSource(selection), command);
  d->m_ignoreCurrentChanged = false;
}

void KProxyItemSelectionModel::sourceSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
  QItemSelection mappedDeselection = mapSelectionToSource(deselected);
  QItemSelection mappedSelection = mapSelectionToSource(selected);

  QItemSelectionModel::select(mappedDeselection, Deselect);
  QItemSelectionModel::select(mappedSelection, Select);
}
