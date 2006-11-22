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
#include <kdirmodel.h>
#include <kfileitem.h>
#include <kdirlister.h>
#include <QAbstractItemView>
#include <QScrollBar>
#include <QTimer>

class KMimeTypeResolverPrivate
{
public:
    KMimeTypeResolverPrivate()
        : m_delayForNonVisibleIcons(10), // TODO set me to 0 when image preview is enabled
          m_noVisibleIcon(false)
    {
        m_timer.setSingleShot(true);
    }

    QModelIndex findVisibleIcon();

    QAbstractItemView* m_view;
    KDirModel* m_dirModel;
    int m_delayForNonVisibleIcons;
    QList<QPersistentModelIndex> m_pendingIndexes;
    QTimer m_timer;
    // Set to true when findVisibleIcon found no visible index in m_pendingIndexes.
    // This makes further calls to findVisibleIcon no-ops until this bool is reset to false.
    bool m_noVisibleIcon;
};


QModelIndex KMimeTypeResolverPrivate::findVisibleIcon()
{
    if (m_noVisibleIcon)
        return QModelIndex();

    if (m_pendingIndexes.count() < 20) { // for few items, it's faster to not bother
        kDebug() << k_funcinfo << "Few items, returning first one" << endl;
        return m_pendingIndexes.first();
    }

    const QRect visibleArea = m_view->viewport()->rect();
    QList<QPersistentModelIndex>::const_iterator it = m_pendingIndexes.begin();
    const QList<QPersistentModelIndex>::const_iterator end = m_pendingIndexes.end();
    for ( ; it != end ; ++it ) {
        const QRect rect = m_view->visualRect(*it);
        if (rect.intersects(visibleArea)) {
            kDebug() << k_funcinfo << "found item at " << rect << " in visibleArea " << visibleArea << endl;
            return *it;
        }
    }

    kDebug() << k_funcinfo << "no more visible icon found" << endl;
    m_noVisibleIcon = true;
    return QModelIndex();
}

////

KMimeTypeResolver::KMimeTypeResolver(QAbstractItemView* view, KDirModel* model)
    : QObject(view), d(new KMimeTypeResolverPrivate)
{
    d->m_view = view;
    d->m_dirModel = model;
    connect(d->m_dirModel, SIGNAL(rowsInserted(QModelIndex,int,int)),
            this, SLOT(slotRowsInserted(QModelIndex,int,int)));
    connect(&d->m_timer, SIGNAL(timeout()),
            this, SLOT(slotProcessMimeIcons()));
    connect(d->m_view->horizontalScrollBar(), SIGNAL(valueChanged(int)),
            this, SLOT(slotViewportAdjusted()));
    connect(d->m_view->verticalScrollBar(), SIGNAL(valueChanged(int)),
            this, SLOT(slotViewportAdjusted()));
}

KMimeTypeResolver::~KMimeTypeResolver()
{
    delete d;
}

void KMimeTypeResolver::slotProcessMimeIcons()
{
    if (d->m_pendingIndexes.isEmpty()) {
        // Finished
        return;
    }

    int nextDelay = 0;
    QModelIndex index = d->findVisibleIcon();
    if (index.isValid()) {
        // Found a visible item.
        const int numFound = d->m_pendingIndexes.removeAll(index);
        Q_ASSERT(numFound == 1);
    }
    if (!index.isValid())
    {
        // No more visible items.
        // Do the unvisible ones, then, but with a bigger delay, if so configured
        index = d->m_pendingIndexes.takeFirst();
        nextDelay = d->m_delayForNonVisibleIcons;
    }
    KFileItem item = * d->m_dirModel->itemForIndex(index);
    if (!item.isMimeTypeKnown()) { // check if someone did it meanwhile
        kDebug() << k_funcinfo << "Determining mimetype for " << item.url() << endl;
        item.determineMimeType();
        d->m_dirModel->itemChanged(index);
    }
    d->m_timer.start(nextDelay); // singleshot
}

void KMimeTypeResolver::slotRowsInserted(const QModelIndex& parent, int first, int last)
{
    for (int row = first; row <= last; ++row) {
        QModelIndex idx = d->m_dirModel->index(row, 0, parent);
        KFileItem* item = d->m_dirModel->itemForIndex(idx);
        if (!item->isMimeTypeKnown())
            d->m_pendingIndexes.append(idx);
    }
    d->m_noVisibleIcon = false;
    d->m_timer.start(d->m_delayForNonVisibleIcons); // singleshot
}

void KMimeTypeResolver::slotViewportAdjusted()
{
    d->m_noVisibleIcon = false;
    d->m_timer.start(0);
}

#include "kmimetyperesolver.moc"
