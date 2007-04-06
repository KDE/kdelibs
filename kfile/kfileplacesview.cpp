/*  This file is part of the KDE project
    Copyright (C) 2007 Kevin Ottens <ervin@kde.org>

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

#include "kfileplacesview.h"

#include <QMenu>
#include <QContextMenuEvent>

#include <kdebug.h>

#include <kcomponentdata.h>
#include <klocale.h>
#include <kjob.h>
#include <solid/volume.h>

#include "kfileplaceeditdialog.h"
#include "kfileplacesmodel.h"

class KFilePlacesView::Private
{
public:
    Private(KFilePlacesView *parent) : q(parent) { }

    KFilePlacesView * const q;

    KUrl currentUrl;

    void _k_placeClicked(const QModelIndex &index);
};

KFilePlacesView::KFilePlacesView(QWidget *parent)
    : QListView(parent), d(new Private(this))
{
    setSelectionRectVisible(false);
    setSelectionMode(QAbstractItemView::SingleSelection);

    setDragEnabled(true);
    setAcceptDrops(true);
    setDropIndicatorShown(true);

    connect(this, SIGNAL(clicked(const QModelIndex&)),
            this, SLOT(_k_placeClicked(const QModelIndex&)));
}

KFilePlacesView::~KFilePlacesView()
{
    delete d;
}

void KFilePlacesView::setUrl(const KUrl &url)
{
    KFilePlacesModel *placesModel = qobject_cast<KFilePlacesModel*>(model());

    if (placesModel==0) return;

    QModelIndex index = placesModel->closestItem(url);

    if (index.isValid()) {
        d->currentUrl = url;
        selectionModel()->setCurrentIndex(index, QItemSelectionModel::ClearAndSelect);
    } else {
        d->currentUrl = KUrl();
        selectionModel()->clear();
    }
}

void KFilePlacesView::contextMenuEvent(QContextMenuEvent *event)
{
    KFilePlacesModel *placesModel = qobject_cast<KFilePlacesModel*>(model());

    if (placesModel==0) return;

    QModelIndex index = indexAt(event->pos());

    QMenu menu;

    QAction *edit = 0;
    if (index.isValid() && !placesModel->isDevice(index)) {
        edit = menu.addAction(KIcon("edit"), i18n("&Edit Entry..."));
        menu.addSeparator();
    }

    QAction *add = menu.addAction(KIcon("document-new"), i18n("&Add Entry..."));

    QAction* remove = 0L;
    if (index.isValid() && !placesModel->isDevice(index)) {
        remove = menu.addAction( KIcon("edit-delete"), i18n("&Remove Entry"));
    }

    QAction *result = menu.exec(event->globalPos());

    if (add != 0 &&result == add) {
        KUrl url;
        QString description;
        QString iconName;
        bool appLocal = false;

        if (KFilePlaceEditDialog::getInformation(true, url, description,
                                                 iconName, appLocal, 64, this))
        {
            QString appName;
            if (appLocal) appName = KGlobal::mainComponent().componentName();

            placesModel->addPlace(description, url, iconName, appName);
        }

    } else if (edit != 0 && result == edit) {
        KBookmark bookmark = placesModel->bookmarkForIndex(index);
        KUrl url = bookmark.url();
        QString description = bookmark.text();
        QString iconName = bookmark.icon();
        bool appLocal = !bookmark.metaDataItem("OnlyInApp").isEmpty();

        if (KFilePlaceEditDialog::getInformation(true, url, description,
                                                 iconName, appLocal, 64, this))
        {
            QString appName;
            if (appLocal) appName = KGlobal::mainComponent().componentName();

            placesModel->editPlace(index, description, url, iconName, appName);
        }

    } else if (remove != 0 && result == remove) {
        placesModel->removePlace(index);
    }

    setUrl(d->currentUrl);
}

void KFilePlacesView::rowsInserted(const QModelIndex &parent, int start, int end)
{
    QListView::rowsInserted(parent, start, end);
    setUrl(d->currentUrl);
}

void KFilePlacesView::Private::_k_placeClicked(const QModelIndex &index)
{
    KFilePlacesModel *placesModel = qobject_cast<KFilePlacesModel*>(q->model());

    if (placesModel==0) return;

    if (placesModel->isDevice(index)) {
        Solid::Device device = placesModel->deviceForIndex(index);
        if (device.is<Solid::Volume>() && !device.as<Solid::Volume>()->isMounted()) {
            KJob *job = device.as<Solid::Volume>()->mount();
            job->exec(); // FIXME: Do this asynchronously
        }
    }

    KUrl url = placesModel->url(index);

    if (url.isValid()) {
        currentUrl = url;
        emit q->urlChanged(url);
    } else {
        q->setUrl(currentUrl);
    }
}

#include "kfileplacesview.moc"
