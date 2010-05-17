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


#include "klinkitemselectionmodel.h"

#include "kmodelindexproxymapper.h"

class KLinkItemSelectionModelPrivate
{
public:
    KLinkItemSelectionModelPrivate(KLinkItemSelectionModel *proxySelectionModel, QAbstractItemModel *model,
                                    QItemSelectionModel *linkedItemSelectionModel)
      : q_ptr(proxySelectionModel),
        m_model(model),
        m_linkedItemSelectionModel(linkedItemSelectionModel),
        m_ignoreCurrentChanged(false),
        m_indexMapper(new KModelIndexProxyMapper(model, linkedItemSelectionModel->model(), proxySelectionModel))
    {
    }

    Q_DECLARE_PUBLIC(KLinkItemSelectionModel)
    KLinkItemSelectionModel * const q_ptr;

    void sourceSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

    QList<const QAbstractProxyModel *> m_proxyChainUp;
    QList<const QAbstractProxyModel *> m_proxyChainDown;

    QAbstractItemModel * const m_model;
    QItemSelectionModel * const m_linkedItemSelectionModel;
    bool m_ignoreCurrentChanged;
    KModelIndexProxyMapper * const m_indexMapper;
};

KLinkItemSelectionModel::KLinkItemSelectionModel(QAbstractItemModel *model, QItemSelectionModel *proxySelector, QObject *parent)
        : QItemSelectionModel(model, parent),
        d_ptr(new KLinkItemSelectionModelPrivate(this, model, proxySelector))
{
    connect(proxySelector, SIGNAL(selectionChanged(QItemSelection, QItemSelection)), SLOT(sourceSelectionChanged(QItemSelection, QItemSelection)));
}

KLinkItemSelectionModel::~KLinkItemSelectionModel()
{
    delete d_ptr;
}

void KLinkItemSelectionModel::select(const QModelIndex &index, QItemSelectionModel::SelectionFlags command)
{
    Q_D(KLinkItemSelectionModel);
    // When an item is removed, the current index is set to the top index in the model.
    // That causes a selectionChanged signal with a selection which we do not want.
    if (d->m_ignoreCurrentChanged) {
        return;
    }
    QItemSelectionModel::select(index, command);
    if (index.isValid())
        d->m_linkedItemSelectionModel->select(d->m_indexMapper->mapSelectionLeftToRight(QItemSelection(index, index)), command);
    else {
        d->m_linkedItemSelectionModel->clearSelection();
    }
}

void KLinkItemSelectionModel::select(const QItemSelection &selection, QItemSelectionModel::SelectionFlags command)
{
    Q_D(KLinkItemSelectionModel);
    d->m_ignoreCurrentChanged = true;
    QItemSelectionModel::select(selection, command);
    d->m_linkedItemSelectionModel->select(d->m_indexMapper->mapSelectionLeftToRight(selection), command);
    d->m_ignoreCurrentChanged = false;
}

void KLinkItemSelectionModelPrivate::sourceSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
    Q_Q(KLinkItemSelectionModel);
    const QItemSelection mappedDeselection = m_indexMapper->mapSelectionRightToLeft(deselected);
    const QItemSelection mappedSelection = m_indexMapper->mapSelectionRightToLeft(selected);

    q->QItemSelectionModel::select(mappedDeselection, QItemSelectionModel::Deselect);
    q->QItemSelectionModel::select(mappedSelection, QItemSelectionModel::Select);
}

#include "klinkitemselectionmodel.moc"
