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
                                          QAbstractProxyModel *proxy,
                                          QItemSelectionModel *selectionModel)
    : q_ptr( proxySelectionModel ), m_model(model), m_proxy(proxy), m_proxySelector(selectionModel)
  {
    createProxyChain();
  }

  void createProxyChain();

  Q_DECLARE_PUBLIC(KProxyItemSelectionModel)
  KProxyItemSelectionModel * const q_ptr;

  QList<const QAbstractProxyModel *> m_proxyChain;

  QAbstractItemModel *m_model;
  QAbstractProxyModel *m_proxy;
  QItemSelectionModel *m_proxySelector;
  bool m_ignoreCurrentChanged;
};

void KProxyItemSelectionModelPrivate::createProxyChain()
{
  const QAbstractProxyModel *proxyModel = m_proxy;

  const QAbstractProxyModel *nextProxyModel;

  while (proxyModel)
  {
    if (proxyModel == m_model)
      break;

    m_proxyChain << proxyModel;

    nextProxyModel = qobject_cast<const QAbstractProxyModel*>(proxyModel->sourceModel());

    if (!nextProxyModel)
    {
      // It's the final model in the chain, so it is necessarily the sourceModel.
      Q_ASSERT(qobject_cast<QAbstractItemModel*>(proxyModel->sourceModel()) == m_model);
      break;
    }
    proxyModel = nextProxyModel;
  }
}

KProxyItemSelectionModel::KProxyItemSelectionModel(QItemSelectionModel *proxySelector, QAbstractItemModel *model, QAbstractProxyModel *proxy, QObject *parent)
  : QItemSelectionModel(proxy, parent),
    d_ptr(new KProxyItemSelectionModelPrivate(this, model, proxy, proxySelector))
{
  connect(proxySelector, SIGNAL(selectionChanged(QItemSelection,QItemSelection)), SLOT(sourceSelectionChanged(QItemSelection,QItemSelection)));
}

QItemSelection KProxyItemSelectionModel::mapSelectionFromSource(const QModelIndex& sourceIndex) const
{
  Q_D(const KProxyItemSelectionModel);

  return mapSelectionFromSource(QItemSelection(sourceIndex, sourceIndex));
}

QItemSelection KProxyItemSelectionModel::mapSelectionFromSource(const QItemSelection& sourceSelection) const
{
  Q_D(const KProxyItemSelectionModel);

  QItemSelection seekSelection = sourceSelection;
  QListIterator<const QAbstractProxyModel*> i(d->m_proxyChain);

  while (i.hasNext())
  {
    const QAbstractProxyModel *proxy = i.next();
    seekSelection = proxy->mapSelectionToSource(seekSelection);
  }

  return seekSelection;
}

QItemSelection KProxyItemSelectionModel::mapSelectionToSource(const QModelIndex& sourceIndex) const
{
  Q_D(const KProxyItemSelectionModel);

  return mapSelectionToSource(QItemSelection(sourceIndex, sourceIndex));
}

QItemSelection KProxyItemSelectionModel::mapSelectionToSource(const QItemSelection& sourceSelection) const
{
  Q_D(const KProxyItemSelectionModel);

  QItemSelection seekSelection = sourceSelection;
  QListIterator<const QAbstractProxyModel*> i(d->m_proxyChain);

  i.toBack();
  while (i.hasPrevious())
  {
    const QAbstractProxyModel *proxy = i.previous();
    seekSelection = proxy->mapSelectionFromSource(seekSelection);
  }
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
  Q_D(KProxyItemSelectionModel);

  // TODO: Implement
  qDebug() << selected << deselected;

}
