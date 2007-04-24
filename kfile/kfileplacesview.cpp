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
    bool showAll;

    void adaptItemSize();
    void updateHiddenRows();

    void _k_placeClicked(const QModelIndex &index);
};

KFilePlacesView::KFilePlacesView(QWidget *parent)
    : QListView(parent), d(new Private(this))
{
    d->showAll = false;

    setSelectionRectVisible(false);
    setSelectionMode(SingleSelection);

    setDragEnabled(true);
    setAcceptDrops(true);
    setDropIndicatorShown(true);

    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

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
    d->updateHiddenRows();
}

void KFilePlacesView::setShowAll(bool showAll)
{
    KFilePlacesModel *placesModel = qobject_cast<KFilePlacesModel*>(model());

    if (placesModel==0) return;

    d->showAll = showAll;
    d->updateHiddenRows();
}

void KFilePlacesView::contextMenuEvent(QContextMenuEvent *event)
{
    KFilePlacesModel *placesModel = qobject_cast<KFilePlacesModel*>(model());

    if (placesModel==0) return;

    QModelIndex index = indexAt(event->pos());

    QMenu menu;

    QAction *edit = 0;
    QAction *hide = 0;
    if (index.isValid()) {
        if (!placesModel->isDevice(index)) {
            edit = menu.addAction(KIcon("edit"), i18n("&Edit Entry..."));
        }

        menu.addSeparator();

        hide = menu.addAction(i18n("&Hide Entry"));
        hide->setCheckable(true);
        hide->setChecked(placesModel->isHidden(index));
    }

    QAction *showAll = 0;
    if (placesModel->hiddenCount()>0) {
        showAll = menu.addAction(i18n("&Show All Entries"));
        showAll->setCheckable(true);
        showAll->setChecked(d->showAll);
    }

    menu.addSeparator();

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
    } else if (hide != 0 && result == hide) {
        placesModel->setPlaceHidden(index, hide->isChecked());
    } else if (showAll != 0 && result == showAll) {
        setShowAll(showAll->isChecked());
    }

    setUrl(d->currentUrl);
}

void KFilePlacesView::resizeEvent(QResizeEvent *event)
{
    d->adaptItemSize();
    QListView::resizeEvent(event);
}

void KFilePlacesView::rowsInserted(const QModelIndex &parent, int start, int end)
{
    QListView::rowsInserted(parent, start, end);
    setUrl(d->currentUrl);
}

QSize KFilePlacesView::sizeHint() const
{
    KFilePlacesModel *placesModel = qobject_cast<KFilePlacesModel*>(model());
    const int height = QListView::sizeHint().height();
    QFontMetrics fm = d->q->fontMetrics();
    int textWidth = 0;

    for (int i=0; i<placesModel->rowCount(); ++i) {
        QModelIndex index = placesModel->index(i, 0);
        if (!placesModel->isHidden(index))
           textWidth = qMax(textWidth,fm.width(placesModel->bookmarkForIndex(index).text()));
    }

    return QSize(48 + 10 + textWidth + 2*KDialog::marginHint(), height);
}

void KFilePlacesView::Private::adaptItemSize()
{
    KFilePlacesModel *placesModel = qobject_cast<KFilePlacesModel*>(q->model());

    if (placesModel==0) return;

    int rowCount = placesModel->rowCount();

    if (!showAll) {
        rowCount-= placesModel->hiddenCount();

        QModelIndex current = placesModel->closestItem(currentUrl);

        if (placesModel->isHidden(current)) {
            rowCount++;
        }
    }

    if (rowCount==0) return; // We've nothing to display anyway

    const int minSize = 16;
    const int maxSize = 64;

    int textWidth = 0;
    QFontMetrics fm = q->fontMetrics();
    for (int i=0; i<rowCount; ++i) {
        QModelIndex index = placesModel->index(i, 0);
        if (!placesModel->isHidden(index))
           textWidth = qMax(textWidth,fm.width(placesModel->bookmarkForIndex(index).text()));
    }

    const int maxWidth = q->width() - textWidth - 2 * KDialog::marginHint();
    const int maxHeight = (q->height()/rowCount)-1;

    int size = qMin(maxHeight, maxWidth);

    if (size<minSize) {
        size = minSize;
    } else if (size>maxSize) {
        size = maxSize;
    } else {
        // Make it a multiple of 16
        size>>= 4;
        size<<= 4;
    }

    if (size!=q->iconSize().height()) {
        q->setIconSize(QSize(size, size));
    }
}

void KFilePlacesView::Private::updateHiddenRows()
{
    KFilePlacesModel *placesModel = qobject_cast<KFilePlacesModel*>(q->model());

    if (placesModel==0) return;

    int rowCount = placesModel->rowCount();
    QModelIndex current = placesModel->closestItem(currentUrl);

    for (int i=0; i<rowCount; ++i) {
        QModelIndex index = placesModel->index(i, 0);
        if (index!=current && placesModel->isHidden(index)) {
            q->setRowHidden(i, !showAll);
        } else {
            q->setRowHidden(i, false);
        }
    }

    adaptItemSize();
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
        updateHiddenRows();
        emit q->urlChanged(url);
    } else {
        q->setUrl(currentUrl);
    }
}

void KFilePlacesView::dataChanged(const QModelIndex &/*topLeft*/, const QModelIndex &/*bottomRight*/)
{
    d->updateHiddenRows();
}

#include "kfileplacesview.moc"
