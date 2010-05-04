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

#include "kmodelindexproxymapper.h"

class KProxyItemSelectionModelPrivate
{
public:
    KProxyItemSelectionModelPrivate(KProxyItemSelectionModel *proxySelectionModel, QAbstractItemModel *model,
                                    QItemSelectionModel *selectionModel)
      : q_ptr(proxySelectionModel),
        m_model(model),
        m_proxySelector(selectionModel),
        m_ignoreCurrentChanged(false),
        m_indexMapper(new KModelIndexProxyMapper(model, selectionModel->model(), proxySelectionModel))
    {
    }

    Q_DECLARE_PUBLIC(KProxyItemSelectionModel)
    KProxyItemSelectionModel * const q_ptr;

    QList<const QAbstractProxyModel *> m_proxyChainUp;
    QList<const QAbstractProxyModel *> m_proxyChainDown;

    QAbstractItemModel *m_model;
    QItemSelectionModel *m_proxySelector;
    bool m_ignoreCurrentChanged;
    KModelIndexProxyMapper *m_indexMapper;
};

KProxyItemSelectionModel::KProxyItemSelectionModel(QAbstractItemModel *model, QItemSelectionModel *proxySelector, QObject *parent)
        : QItemSelectionModel(model, parent),
        d_ptr(new KProxyItemSelectionModelPrivate(this, model, proxySelector))
{
    connect(proxySelector, SIGNAL(selectionChanged(QItemSelection, QItemSelection)), SLOT(sourceSelectionChanged(QItemSelection, QItemSelection)));
}

void KProxyItemSelectionModel::select(const QModelIndex &index, QItemSelectionModel::SelectionFlags command)
{
    Q_D(KProxyItemSelectionModel);
    // When an item is removed, the current index is set to the top index in the model.
    // That causes a selectionChanged signal with a selection which we do not want.
    if (d->m_ignoreCurrentChanged) {
        return;
    }
    QItemSelectionModel::select(index, command);
    if (index.isValid())
        d->m_proxySelector->select(d->m_indexMapper->mapSelectionLeftToRight(QItemSelection(index, index)), command);
    else {
        d->m_proxySelector->clearSelection();
    }
}

void KProxyItemSelectionModel::select(const QItemSelection &selection, QItemSelectionModel::SelectionFlags command)
{
    Q_D(KProxyItemSelectionModel);
    d->m_ignoreCurrentChanged = true;
    QItemSelectionModel::select(selection, command);
    d->m_proxySelector->select(d->m_indexMapper->mapSelectionLeftToRight(selection), command);
    d->m_ignoreCurrentChanged = false;
}

void KProxyItemSelectionModel::sourceSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
    Q_D(KProxyItemSelectionModel);
    const QItemSelection mappedDeselection = d->m_indexMapper->mapSelectionRightToLeft(deselected);
    const QItemSelection mappedSelection = d->m_indexMapper->mapSelectionRightToLeft(selected);

    QItemSelectionModel::select(mappedDeselection, Deselect);
    QItemSelectionModel::select(mappedSelection, Select);
}
