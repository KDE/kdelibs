/*
   This file is part of the KDE project

   Copyright (C) 2007 Tobias Koenig <tokoe@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "kfiletreeview.h"

#include <QtCore/QDir>

#include <kdirlister.h>
#include <kdirmodel.h>
#include <kfileitemdelegate.h>
#include <kurl.h>

class KFileTreeView::Private
{
    public:
        Private(KFileTreeView *parent)
            : mParent(parent)
        {
        }

        KUrl urlForIndex(const QModelIndex &index) const;

        void _k_activated(const QModelIndex&);
        void _k_currentChanged(const QModelIndex&, const QModelIndex&);
        void _k_expanded(const QModelIndex&);

        KFileTreeView *mParent;
        KDirModel *mModel;
};

KUrl KFileTreeView::Private::urlForIndex(const QModelIndex &index) const
{
    const KFileItem item = mModel->itemForIndex(index);

    return !item.isNull() ? item.url() : KUrl();
}

void KFileTreeView::Private::_k_activated(const QModelIndex &index)
{
    const KUrl url = urlForIndex(index);
    if (url.isValid())
        emit mParent->activated(url);
}

void KFileTreeView::Private::_k_currentChanged(const QModelIndex &currentIndex, const QModelIndex&)
{
    const KUrl url = urlForIndex(currentIndex);
    if (url.isValid())
        emit mParent->currentChanged(url);
}

void KFileTreeView::Private::_k_expanded(const QModelIndex &index)
{
    mParent->selectionModel()->clearSelection();
    mParent->selectionModel()->setCurrentIndex(index, QItemSelectionModel::SelectCurrent);
    mParent->scrollTo(index);
}

KFileTreeView::KFileTreeView(QWidget *parent)
    : QTreeView(parent), d(new Private(this))
{
    d->mModel = new KDirModel(this);

    setModel(d->mModel);
    setItemDelegate(new KFileItemDelegate(this));

    d->mModel->dirLister()->openUrl(KUrl(QDir::root().absolutePath()), KDirLister::Keep);

    connect(this, SIGNAL(activated(const QModelIndex&)),
            this, SLOT(_k_activated(const QModelIndex&)));
    connect(selectionModel(), SIGNAL(currentChanged(const QModelIndex&, const QModelIndex&)),
            this, SLOT(_k_currentChanged(const QModelIndex&, const QModelIndex&)));
    connect(d->mModel, SIGNAL(expand(const QModelIndex&)),
            this, SLOT(_k_expanded(const QModelIndex&)));
}

KFileTreeView::~KFileTreeView()
{
    delete d;
}

KUrl KFileTreeView::currentUrl() const
{
    return d->urlForIndex(currentIndex());
}

KUrl KFileTreeView::selectedUrl() const
{
    if (!selectionModel()->hasSelection())
        return KUrl();

    const QItemSelection selection = selectionModel()->selection();
    const QModelIndex firstIndex = selection.indexes().first();

    return d->urlForIndex(firstIndex);
}

KUrl::List KFileTreeView::selectedUrls() const
{
    KUrl::List urls;

    if (!selectionModel()->hasSelection())
        return urls;

    const QModelIndexList indexes = selectionModel()->selection().indexes();
    foreach (const QModelIndex index, indexes) {
        const KUrl url = d->urlForIndex(index);
        if (url.isValid())
            urls.append(url);
    }

    return urls;
}

KUrl KFileTreeView::rootUrl() const
{
    return d->mModel->dirLister()->url();
}

void KFileTreeView::setDirOnlyMode(bool enabled)
{
    d->mModel->dirLister()->setDirOnlyMode(enabled);
    d->mModel->dirLister()->openUrl(d->mModel->dirLister()->url());
}

void KFileTreeView::setShowHiddenFiles(bool enabled)
{
    d->mModel->dirLister()->setShowingDotFiles(enabled);
    d->mModel->dirLister()->openUrl(d->mModel->dirLister()->url());
}

void KFileTreeView::setCurrentUrl(const KUrl &url)
{
    QModelIndex index = d->mModel->indexForUrl(url);

    if (!index.isValid()) {
        d->mModel->expandToUrl(url);
        return;
    }
    selectionModel()->clearSelection();
    selectionModel()->setCurrentIndex(index, QItemSelectionModel::SelectCurrent);
    scrollTo(index);
}

void KFileTreeView::setRootUrl(const KUrl &url)
{
    d->mModel->dirLister()->openUrl(url);
}

#include "kfiletreeview.moc"
