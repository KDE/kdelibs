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

#include <QtCore/QTimeLine>
#include <QtCore/QTimer>
#include <QtGui/QMenu>
#include <QtGui/QItemDelegate>
#include <QtGui/QKeyEvent>

#include <kdebug.h>

#include <kcomponentdata.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kjob.h>
#include <solid/storageaccess.h>
#include <solid/storagedrive.h>
#include <solid/storagevolume.h>
#include <solid/opticaldrive.h>
#include <solid/opticaldisc.h>

#include "kfileplaceeditdialog.h"
#include "kfileplacesmodel.h"

class KFilePlacesViewDelegate : public QItemDelegate
{
public:
    KFilePlacesViewDelegate(KFilePlacesView *parent);
    virtual ~KFilePlacesViewDelegate();
    virtual QSize sizeHint(const QStyleOptionViewItem &option,
                           const QModelIndex &index) const;
    virtual void paint(QPainter *painter,
                       const QStyleOptionViewItem &option,
                       const QModelIndex &index) const;

    int iconSize() const;
    void setIconSize(int newSize);

    void addAppearingItem(const QModelIndex &index);
    void setAppearingIconSize(int newSize);

private:
    KFilePlacesView *m_view;
    int m_iconSize;

    QList<QPersistentModelIndex> m_appearingItems;
    int m_appearingIconSize;
};

KFilePlacesViewDelegate::KFilePlacesViewDelegate(KFilePlacesView *parent) :
    QItemDelegate(parent), m_view(parent), m_iconSize(48)
{
}

KFilePlacesViewDelegate::~KFilePlacesViewDelegate()
{
}

QSize KFilePlacesViewDelegate::sizeHint(const QStyleOptionViewItem &option,
                                        const QModelIndex &index) const
{
    QSize size = QItemDelegate::sizeHint(option, index);
    int iconSize = m_iconSize;
    if (m_appearingItems.contains(index)) {
        iconSize = m_appearingIconSize;
    }

    size.setHeight(iconSize + KDialog::marginHint());
    return size;
}

void KFilePlacesViewDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    int iconSize = m_iconSize;
    if (m_appearingItems.contains(index)) {
        iconSize = m_appearingIconSize;
    }

    QStyleOptionViewItem opt = option;
    opt.decorationSize = QSize(iconSize, iconSize);
    opt.state|= QStyle::State_Active;
    QItemDelegate::paint(painter, opt, index);
}

int KFilePlacesViewDelegate::iconSize() const
{
    return m_iconSize;
}

void KFilePlacesViewDelegate::setIconSize(int newSize)
{
    m_iconSize = newSize;
}

void KFilePlacesViewDelegate::addAppearingItem(const QModelIndex &index)
{
    m_appearingItems << index;
}

void KFilePlacesViewDelegate::setAppearingIconSize(int newSize)
{
    m_appearingIconSize = newSize;

    if (m_appearingIconSize>=m_iconSize) {
        m_appearingItems.clear();
        m_appearingIconSize = m_iconSize;
    }
}

class KFilePlacesView::Private
{
public:
    Private(KFilePlacesView *parent) : q(parent) { }

    KFilePlacesView * const q;

    KUrl currentUrl;
    bool showAll;
    bool smoothItemResizing;
    Solid::StorageAccess *lastClickedStorage;
    QPersistentModelIndex lastClickedIndex;

    void setCurrentIndex(const QModelIndex &index);
    void adaptItemSize();
    void updateHiddenRows();

    void _k_placeClicked(const QModelIndex &index);
    void _k_placeActivated(const QModelIndex &index);
    void _k_storageSetupDone(const QModelIndex &index, bool success);
    void _k_adaptItemsUpdate(qreal value);
    void _k_itemAppearUpdate(qreal value);
    void _k_enableSmoothItemResizing();

    QTimeLine adaptItemsTimeline;
    int oldSize, endSize;

    QTimeLine itemAppearTimeline;
};

KFilePlacesView::KFilePlacesView(QWidget *parent)
    : QListView(parent), d(new Private(this))
{
    d->showAll = false;
    d->smoothItemResizing = false;
    d->lastClickedStorage = 0;

    setSelectionRectVisible(false);
    setSelectionMode(SingleSelection);

    setDragEnabled(true);
    setAcceptDrops(true);
    setDropIndicatorShown(true);
    setFrameStyle(QFrame::NoFrame);

    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    setResizeMode(Adjust);
    setItemDelegate(new KFilePlacesViewDelegate(this));

    QPalette palette = viewport()->palette();
    palette.setColor(viewport()->backgroundRole(), Qt::transparent);
    viewport()->setPalette(palette);

    connect(this, SIGNAL(clicked(const QModelIndex&)),
            this, SLOT(_k_placeClicked(const QModelIndex&)));
    connect(this, SIGNAL(activated(const QModelIndex&)),
            this, SLOT(_k_placeActivated(const QModelIndex&)));

    connect(&d->adaptItemsTimeline, SIGNAL(valueChanged(qreal)),
            this, SLOT(_k_adaptItemsUpdate(qreal)));
    d->adaptItemsTimeline.setDuration(150);
    d->adaptItemsTimeline.setUpdateInterval(30);
    d->adaptItemsTimeline.setCurveShape(QTimeLine::EaseInOutCurve);

    connect(&d->itemAppearTimeline, SIGNAL(valueChanged(qreal)),
            this, SLOT(_k_itemAppearUpdate(qreal)));
    d->itemAppearTimeline.setDuration(150);
    d->itemAppearTimeline.setUpdateInterval(30);
    d->itemAppearTimeline.setCurveShape(QTimeLine::EaseOutCurve);
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
        if (d->currentUrl!=url && placesModel->isHidden(index)) {
            KFilePlacesViewDelegate *delegate = dynamic_cast<KFilePlacesViewDelegate*>(itemDelegate());
            delegate->addAppearingItem(index);

            if (d->itemAppearTimeline.state()!=QTimeLine::Running) {
                delegate->setAppearingIconSize(1);
                d->itemAppearTimeline.start();
            }
        }

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

    KFilePlacesViewDelegate *delegate = dynamic_cast<KFilePlacesViewDelegate*>(itemDelegate());

    int rowCount = placesModel->rowCount();
    QModelIndex current = placesModel->closestItem(d->currentUrl);

    if (showAll) {
        for (int i=0; i<rowCount; ++i) {
            QModelIndex index = placesModel->index(i, 0);
            if (index!=current && placesModel->isHidden(index)) {
                delegate->addAppearingItem(index);
            }
        }

        if (d->itemAppearTimeline.state()!=QTimeLine::Running) {
            delegate->setAppearingIconSize(1);
            d->itemAppearTimeline.start();
        }
    }

}

void KFilePlacesView::contextMenuEvent(QContextMenuEvent *event)
{
    KFilePlacesModel *placesModel = qobject_cast<KFilePlacesModel*>(model());

    if (placesModel==0) return;

    QModelIndex index = indexAt(event->pos());
    QString label = placesModel->text(index);

    QMenu menu;

    QAction *edit = 0;
    QAction *hide = 0;
    if (index.isValid()) {
        if (!placesModel->isDevice(index)) {
            edit = menu.addAction(KIcon("edit"), i18n("&Edit '%1'...", label));
        }

        hide = menu.addAction(i18n("&Hide '%1'", label));
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

    QAction* remove = 0L;
    QAction* teardown = 0L;
    if (index.isValid()) {
        if (!placesModel->isDevice(index)) {
            remove = menu.addAction( KIcon("edit-delete"), i18n("&Remove '%1'", label));
        } else {
            teardown = placesModel->teardownActionForIndex(index);
            if (teardown!=0) {
                teardown->setParent(&menu);
                menu.addAction(teardown);
            }
        }
    }

    QAction *result = menu.exec(event->globalPos());

    if (edit != 0 && result == edit) {
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
    } else if (teardown != 0 && result == teardown) {
        placesModel->requestTeardown(index);
    }

    setUrl(d->currentUrl);
}

void KFilePlacesView::resizeEvent(QResizeEvent *event)
{
    QListView::resizeEvent(event);
    d->adaptItemSize();
}

void KFilePlacesView::showEvent(QShowEvent *event)
{
    QListView::showEvent(event);
    QTimer::singleShot(100, this, SLOT(_k_enableSmoothItemResizing()));
}

void KFilePlacesView::hideEvent(QHideEvent *event)
{
    QListView::hideEvent(event);
    d->smoothItemResizing = false;
}

void KFilePlacesView::rowsInserted(const QModelIndex &parent, int start, int end)
{
    QListView::rowsInserted(parent, start, end);
    setUrl(d->currentUrl);

#if 0 // Disable until the model is fixed and sane again signals wise
    KFilePlacesViewDelegate *delegate = dynamic_cast<KFilePlacesViewDelegate*>(itemDelegate());

    for (int i=start; i<=end; ++i) {
        QModelIndex index = model()->index(i, 0, parent);
        delegate->addAppearingItem(index);
    }

    if (d->itemAppearTimeline.state()!=QTimeLine::Running) {
        delegate->setAppearingIconSize(1);
        d->itemAppearTimeline.start();
    }
#endif
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
           textWidth = qMax(textWidth,fm.width(index.data(Qt::DisplayRole).toString()));
    }

    const int iconSize = KIconLoader::global()->currentSize(KIconLoader::Dialog);
    return QSize(iconSize + textWidth + 2*KDialog::marginHint(), height);
}

void KFilePlacesView::Private::setCurrentIndex(const QModelIndex &index)
{
    KFilePlacesModel *placesModel = qobject_cast<KFilePlacesModel*>(q->model());

    if (placesModel==0) return;

    KUrl url = placesModel->url(index);

    if (url.isValid()) {
        currentUrl = url;
        updateHiddenRows();
        emit q->urlChanged(url);
    } else {
        q->setUrl(currentUrl);
    }
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
           textWidth = qMax(textWidth,fm.width(index.data(Qt::DisplayRole).toString()));
    }

    const int maxWidth = q->width() - textWidth - 2 * KDialog::marginHint();
    const int maxHeight = ((q->height() - KDialog::marginHint() * rowCount) / rowCount) - 1;

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

    KFilePlacesViewDelegate *delegate = dynamic_cast<KFilePlacesViewDelegate*>(q->itemDelegate());
    if (!delegate || size==delegate->iconSize()) return;

    if (smoothItemResizing) {
        oldSize = delegate->iconSize();
        endSize = size;
        if (adaptItemsTimeline.state()!=QTimeLine::Running) {
            adaptItemsTimeline.start();
        }
    } else {
        delegate->setIconSize(size);
        q->scheduleDelayedItemsLayout();
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
        if (index!=current && placesModel->isHidden(index) && !showAll) {
            q->setRowHidden(i, true);
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

    lastClickedIndex = QPersistentModelIndex();

    if (placesModel->setupNeeded(index)) {
        QObject::connect(placesModel, SIGNAL(setupDone(const QModelIndex &, bool)),
                         q, SLOT(_k_storageSetupDone(const QModelIndex &, bool)));

        lastClickedIndex = index;
        placesModel->requestSetup(index);
        return;
    }

    setCurrentIndex(index);
}

void KFilePlacesView::Private::_k_placeActivated(const QModelIndex &index)
{
    KFilePlacesModel *placesModel = qobject_cast<KFilePlacesModel*>(q->model());

    if (placesModel==0) return;

    lastClickedIndex = QPersistentModelIndex();

    if (placesModel->isDevice(index)) {
        QObject::connect(placesModel, SIGNAL(setupDone(const QModelIndex &, bool)),
                         q, SLOT(_k_storageSetupDone(const QModelIndex &, bool)));

        lastClickedIndex = index;
        placesModel->requestSetup(index);
        return;
    }

    setCurrentIndex(index);
}

void KFilePlacesView::Private::_k_storageSetupDone(const QModelIndex &index, bool success)
{
    if (index!=lastClickedIndex) {
        return;
    }

    KFilePlacesModel *placesModel = qobject_cast<KFilePlacesModel*>(q->model());

    QObject::disconnect(placesModel, SIGNAL(setupDone(const QModelIndex &, bool)),
                        q, SLOT(_k_storageSetupDone(const QModelIndex &, bool)));

    if (success) {
        setCurrentIndex(lastClickedIndex);
    } else {
        q->setUrl(currentUrl);
    }

    lastClickedIndex = QPersistentModelIndex();
}

void KFilePlacesView::Private::_k_adaptItemsUpdate(qreal value)
{
    int add = (endSize-oldSize)*value;

    int size = oldSize+add;

    KFilePlacesViewDelegate *delegate = dynamic_cast<KFilePlacesViewDelegate*>(q->itemDelegate());
    delegate->setIconSize(size);
    q->scheduleDelayedItemsLayout();
}

void KFilePlacesView::Private::_k_itemAppearUpdate(qreal value)
{
    KFilePlacesViewDelegate *delegate = dynamic_cast<KFilePlacesViewDelegate*>(q->itemDelegate());
    int size = delegate->iconSize()*value;

    delegate->setAppearingIconSize(size);
    q->scheduleDelayedItemsLayout();
}

void KFilePlacesView::Private::_k_enableSmoothItemResizing()
{
    smoothItemResizing = true;
}

void KFilePlacesView::dataChanged(const QModelIndex &/*topLeft*/, const QModelIndex &/*bottomRight*/)
{
    d->updateHiddenRows();
}

#include "kfileplacesview.moc"
