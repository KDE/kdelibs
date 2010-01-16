/* This file is part of the KDE libraries
   Copyright (C) 2000, 2006 David Faure <faure@kde.org>
   Copyright (C) 2000 Rik Hemsley <rik@kde.org>
   Copyright (C) 2002 Carsten Pfeiffer <pfeiffer@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "kmimetyperesolver.h"
#include <kdebug.h>
#include "defaultviewadapter_p.h"
#include <kdirmodel.h>
#include <kfileitem.h>
#include <kdirlister.h>
#include <QAbstractItemView>
#include <QAbstractProxyModel>
#include <QScrollBar>
#include <QTimer>

class KMimeTypeResolverPrivate
{
public:
    KMimeTypeResolverPrivate(KMimeTypeResolver *parent)
        : q(parent),
          m_delayForNonVisibleIcons(10), // TODO set me to 0 when image preview is enabled
          m_noVisibleIcon(false)
    {
        m_timer.setSingleShot(true);
    }


    void _k_slotRowsInserted(const QModelIndex&,int,int);
    void _k_slotViewportAdjusted();
    void _k_slotProcessMimeIcons();

    void init();
    QModelIndex findVisibleIcon();

    KMimeTypeResolver* q;
    KAbstractViewAdapter* m_adapter;
    QAbstractProxyModel* m_proxyModel;
    KDirModel* m_dirModel;
    int m_delayForNonVisibleIcons;
    QList<QPersistentModelIndex> m_pendingIndexes;
    QTimer m_timer;
    // Set to true when findVisibleIcon found no visible index in m_pendingIndexes.
    // This makes further calls to findVisibleIcon no-ops until this bool is reset to false.
    bool m_noVisibleIcon;
};

void KMimeTypeResolverPrivate::init()
{
    QObject::connect(m_dirModel, SIGNAL(rowsInserted(QModelIndex,int,int)),
                     q, SLOT(_k_slotRowsInserted(QModelIndex,int,int)));
    QObject::connect(&m_timer, SIGNAL(timeout()),
                     q, SLOT(_k_slotProcessMimeIcons()));
    m_adapter->connect(KAbstractViewAdapter::ScrollBarValueChanged, q, SLOT(_k_slotViewportAdjusted()));
}

QModelIndex KMimeTypeResolverPrivate::findVisibleIcon()
{
    if (m_noVisibleIcon)
        return QModelIndex();

    if (m_pendingIndexes.count() < 20) { // for few items, it's faster to not bother
        //kDebug() << "Few items, returning first one";
        return QModelIndex(m_pendingIndexes.first());
    }

    const QRect visibleArea = m_adapter->visibleArea();
    QList<QPersistentModelIndex>::const_iterator it = m_pendingIndexes.constBegin();
    const QList<QPersistentModelIndex>::const_iterator end = m_pendingIndexes.constEnd();
    bool layoutDone = true;
    for ( ; it != end ; ++it ) {
        const QModelIndex index = m_proxyModel ? m_proxyModel->mapFromSource(*it) : QModelIndex(*it);
        const QRect rect = m_adapter->visualRect(index);
        if (rect.isNull())
            layoutDone = false;
        else if (rect.intersects(visibleArea)) {
            //kDebug() << "found item at " << rect << " in visibleArea " << visibleArea;
            return QModelIndex(*it);
        }
    }

    if (layoutDone) {
        //kDebug() << "no more visible icon found";
        m_noVisibleIcon = true;
        return QModelIndex();
    } else {
        // Return a random index if the layout is still ongoing
        return QModelIndex(m_pendingIndexes.first());
    }
}

////

KMimeTypeResolver::KMimeTypeResolver(QAbstractItemView* view, KDirModel* model)
    : QObject(view), d(new KMimeTypeResolverPrivate(this))
{
    d->m_adapter = new KIO::DefaultViewAdapter(view, this);
    d->m_proxyModel = 0;
    d->m_dirModel = model;
    d->init();
}

KMimeTypeResolver::KMimeTypeResolver(QAbstractItemView* view, QAbstractProxyModel* model)
    : QObject(view), d(new KMimeTypeResolverPrivate(this))
{
    d->m_adapter = new KIO::DefaultViewAdapter(view, this);
    d->m_proxyModel = model;
    d->m_dirModel = static_cast<KDirModel*>(model->sourceModel());
    d->init();
}

KMimeTypeResolver::KMimeTypeResolver(KAbstractViewAdapter* adapter)
    : QObject(adapter), d(new KMimeTypeResolverPrivate(this))
{
    QAbstractItemModel *model = adapter->model();
    d->m_adapter = adapter;
    d->m_proxyModel = qobject_cast<QAbstractProxyModel*>(model);
    d->m_dirModel = d->m_proxyModel ? qobject_cast<KDirModel*>(d->m_proxyModel->sourceModel())
                                    : qobject_cast<KDirModel*>(model);
    Q_ASSERT(d->m_dirModel);
    d->init();
}

KMimeTypeResolver::~KMimeTypeResolver()
{
    delete d;
}

void KMimeTypeResolverPrivate::_k_slotProcessMimeIcons()
{
    if (m_pendingIndexes.isEmpty()) {
        // Finished
        return;
    }

    int nextDelay = 0;
    QModelIndex index = findVisibleIcon();
    if (index.isValid()) {
        // Found a visible item.
        const int numFound = m_pendingIndexes.removeAll(index);
        Q_UNUSED(numFound); // prevent warning in release builds.
        Q_ASSERT(numFound == 1);
    } else {
        // No more visible items.
        // Do the unvisible ones, then, but with a bigger delay, if so configured
        index = m_pendingIndexes.takeFirst();
        nextDelay = m_delayForNonVisibleIcons;
    }
    KFileItem item = m_dirModel->itemForIndex(index);
    if (!item.isNull()) { // check that item still exists
        if (!item.isMimeTypeKnown()) { // check if someone did it meanwhile
            item.determineMimeType();
            m_dirModel->itemChanged(index);
        }
    }
    m_timer.start(nextDelay); // singleshot
}

void KMimeTypeResolverPrivate::_k_slotRowsInserted(const QModelIndex& parent, int first, int last)
{
    KDirModel* model = m_dirModel;
    for (int row = first; row <= last; ++row) {
        QModelIndex idx = model->index(row, 0, parent);
        KFileItem item = model->itemForIndex(idx);
        if (!item.isMimeTypeKnown())
            m_pendingIndexes.append(idx);
        // TODO else if (item->isDir() && !item->isLocalFile() /*nor pseudo local...*/ &&
        // TODO   model->data(idx, ChildCountRole).toInt() == KDirModel::ChildCountUnknown)
        // TODO d->m_pendingIndexes.append(idx);
    }
    m_noVisibleIcon = false;
    m_timer.start(m_delayForNonVisibleIcons); // singleshot
}

void KMimeTypeResolverPrivate::_k_slotViewportAdjusted()
{
    m_noVisibleIcon = false;
    m_timer.start(0);
}

#include "kmimetyperesolver.moc"
