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
#include <QTimer>

class KMimeTypeResolverPrivate
{
public:
    KMimeTypeResolverPrivate()
        : m_delayForNonVisibleIcons(10) // TODO set me to 0 when image preview is enabled
    {
        m_timer.setSingleShot( true );
    }

    QAbstractItemView* m_view;
    KDirModel* m_dirModel;
    int m_delayForNonVisibleIcons;
    QList<KFileItem> m_pendingFileItems;
    QTimer m_timer;
};

KMimeTypeResolver::KMimeTypeResolver(QAbstractItemView* view, KDirModel* model)
    : QObject(view), d(new KMimeTypeResolverPrivate)
{
    d->m_view = view;
    d->m_dirModel = model;
    // We could use the rowInserted type of signals, but then we'd need a lot of
    // "creating an index for each row and calling itemForIndex for each".
    // Well that's O(1) in itself, but it would have to be done for N items...
    connect(d->m_dirModel->dirLister(), SIGNAL(newItems(KFileItemList)),
            this, SLOT(slotNewItems(KFileItemList)));

    connect(&d->m_timer, SIGNAL(timeout()), this, SLOT(slotProcessMimeIcons()));
}

KMimeTypeResolver::~KMimeTypeResolver()
{
    delete d;
}

void KMimeTypeResolver::slotProcessMimeIcons()
{
    KFileItem item;
    int nextDelay = 0;
    if (!d->m_pendingFileItems.isEmpty())
    {
        // findVisibleIcon();
        //QModelIndex index; // TODO
        //item = d->m_dirModel->itemForIndex(index);
        //d->m_pendingFileItems.removeAll(item);
    }
    // No more visible items.
    if (item.isNull())
    {
        // Do the unvisible ones, then, but with a bigger delay, if so configured
        if (!d->m_pendingFileItems.isEmpty())
        {
            item = d->m_pendingFileItems.takeFirst();
            nextDelay = d->m_delayForNonVisibleIcons;
        } else {
            return;
        }
    }
    if (!item.isMimeTypeKnown()) { // check if someone did it meanwhile
        kDebug() << k_funcinfo << "Determining mimetype for " << item.url() << endl;
        item.determineMimeType();
        d->m_dirModel->itemChanged(item);
    }
    d->m_timer.start( nextDelay );
}

void KMimeTypeResolver::slotNewItems( const KFileItemList& items )
{
    // KFileItemList is still KFileItem*, we want QList<KFileItem> instead.
    KFileItemList::const_iterator it = items.begin();
    const KFileItemList::const_iterator end = items.end();
    for ( ; it != end ; ++it ) {
        KFileItem* item = *it;
        if (!item->isMimeTypeKnown())
            d->m_pendingFileItems.append(*item);
    }
    d->m_timer.start( d->m_delayForNonVisibleIcons );
}

#include "kmimetyperesolver.moc"
