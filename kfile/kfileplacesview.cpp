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
#include <QtGui/QPainter>
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
    void setAppearingItemProgress(qreal value);
    void addDisappearingItem(const QModelIndex &index);
    void setDisappearingItemProgress(qreal value);

private:
    KFilePlacesView *m_view;
    int m_iconSize;

    QList<QPersistentModelIndex> m_appearingItems;
    int m_appearingIconSize;
    qreal m_appearingOpacity;

    QList<QPersistentModelIndex> m_disappearingItems;
    int m_disappearingIconSize;
    qreal m_disappearingOpacity;
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
    } else if (m_disappearingItems.contains(index)) {
        iconSize = m_disappearingIconSize;
    }

    size.setHeight(iconSize + KDialog::marginHint());
    return size;
}

void KFilePlacesViewDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->save();

    if (m_appearingItems.contains(index)) {
        painter->setOpacity(m_appearingOpacity);
    } else if (m_disappearingItems.contains(index)) {
        painter->setOpacity(m_disappearingOpacity);
    }

    QStyleOptionViewItem opt = option;
    opt.decorationSize = QSize(m_iconSize, m_iconSize);
    QItemDelegate::paint(painter, opt, index);

    painter->restore();
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

void KFilePlacesViewDelegate::setAppearingItemProgress(qreal value)
{
    if (value<=0.25) {
        m_appearingOpacity = 0.0;
        m_appearingIconSize = iconSize()*value*4;

        if (m_appearingIconSize>=m_iconSize) {
            m_appearingIconSize = m_iconSize;
        }
    } else {
        m_appearingIconSize = m_iconSize;
        m_appearingOpacity = (value-0.25)*4/3;

        if (value>=1.0) {
            m_appearingItems.clear();
        }
    }
}

void KFilePlacesViewDelegate::addDisappearingItem(const QModelIndex &index)
{
    m_disappearingItems << index;
}

void KFilePlacesViewDelegate::setDisappearingItemProgress(qreal value)
{
    value = 1.0 - value;

    if (value<=0.25) {
        m_disappearingOpacity = 0.0;
        m_disappearingIconSize = iconSize()*value*4;

        if (m_disappearingIconSize>=m_iconSize) {
            m_disappearingIconSize = m_iconSize;
        }

        if (value<=0.0) {
            m_disappearingItems.clear();
        }
    } else {
        m_disappearingIconSize = m_iconSize;
        m_disappearingOpacity = (value-0.25)*4/3;
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
    void _k_itemDisappearUpdate(qreal value);
    void _k_enableSmoothItemResizing();

    QTimeLine adaptItemsTimeline;
    int oldSize, endSize;

    QTimeLine itemAppearTimeline;
    QTimeLine itemDisappearTimeline;
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
    d->adaptItemsTimeline.setDuration(500);
    d->adaptItemsTimeline.setUpdateInterval(5);
    d->adaptItemsTimeline.setCurveShape(QTimeLine::EaseInOutCurve);

    connect(&d->itemAppearTimeline, SIGNAL(valueChanged(qreal)),
            this, SLOT(_k_itemAppearUpdate(qreal)));
    d->itemAppearTimeline.setDuration(500);
    d->itemAppearTimeline.setUpdateInterval(5);
    d->itemAppearTimeline.setCurveShape(QTimeLine::EaseInOutCurve);

    connect(&d->itemDisappearTimeline, SIGNAL(valueChanged(qreal)),
            this, SLOT(_k_itemDisappearUpdate(qreal)));
    d->itemDisappearTimeline.setDuration(500);
    d->itemDisappearTimeline.setUpdateInterval(5);
    d->itemDisappearTimeline.setCurveShape(QTimeLine::EaseInOutCurve);
}

KFilePlacesView::~KFilePlacesView()
{
    delete d;
}

void KFilePlacesView::setUrl(const KUrl &url)
{
    KUrl oldUrl = d->currentUrl;
    KFilePlacesModel *placesModel = qobject_cast<KFilePlacesModel*>(model());

    if (placesModel==0) return;

    QModelIndex index = placesModel->closestItem(url);
    QModelIndex current = selectionModel()->currentIndex();

    if (index.isValid()) {
        if (current!=index && placesModel->isHidden(current)) {
            KFilePlacesViewDelegate *delegate = dynamic_cast<KFilePlacesViewDelegate*>(itemDelegate());
            delegate->addDisappearingItem(current);

            if (d->itemDisappearTimeline.state()!=QTimeLine::Running) {
                delegate->setDisappearingItemProgress(0.0);
                d->itemDisappearTimeline.start();
            }
        }

        if (current!=index && placesModel->isHidden(index)) {
            KFilePlacesViewDelegate *delegate = dynamic_cast<KFilePlacesViewDelegate*>(itemDelegate());
            delegate->addAppearingItem(index);

            if (d->itemAppearTimeline.state()!=QTimeLine::Running) {
                delegate->setAppearingItemProgress(0.0);
                d->itemAppearTimeline.start();
            }

            setRowHidden(index.row(), false);
        }

        d->currentUrl = url;
        selectionModel()->setCurrentIndex(index, QItemSelectionModel::ClearAndSelect);
    } else {
        d->currentUrl = KUrl();
        selectionModel()->clear();
    }

    if (!current.isValid()) {
        d->updateHiddenRows();
    }
}

void KFilePlacesView::setShowAll(bool showAll)
{
    KFilePlacesModel *placesModel = qobject_cast<KFilePlacesModel*>(model());

    if (placesModel==0) return;

    d->showAll = showAll;

    KFilePlacesViewDelegate *delegate = dynamic_cast<KFilePlacesViewDelegate*>(itemDelegate());

    int rowCount = placesModel->rowCount();
    QModelIndex current = placesModel->closestItem(d->currentUrl);

    if (showAll) {
        d->updateHiddenRows();

        for (int i=0; i<rowCount; ++i) {
            QModelIndex index = placesModel->index(i, 0);
            if (index!=current && placesModel->isHidden(index)) {
                delegate->addAppearingItem(index);
            }
        }

        if (d->itemAppearTimeline.state()!=QTimeLine::Running) {
            delegate->setAppearingItemProgress(0.0);
            d->itemAppearTimeline.start();
        }
    } else {
        for (int i=0; i<rowCount; ++i) {
            QModelIndex index = placesModel->index(i, 0);
            if (index!=current && placesModel->isHidden(index)) {
                delegate->addDisappearingItem(index);
            }
        }

        if (d->itemDisappearTimeline.state()!=QTimeLine::Running) {
            delegate->setDisappearingItemProgress(0.0);
            d->itemDisappearTimeline.start();
        }
    }
}

void KFilePlacesView::contextMenuEvent(QContextMenuEvent *event)
{
    KFilePlacesModel *placesModel = qobject_cast<KFilePlacesModel*>(model());
    KFilePlacesViewDelegate *delegate = dynamic_cast<KFilePlacesViewDelegate*>(itemDelegate());

    if (placesModel==0) return;

    QModelIndex index = indexAt(event->pos());
    QString label = placesModel->text(index).replace('&',"&&");

    QMenu menu;

    QAction *edit = 0;
    QAction *hide = 0;
    if (index.isValid()) {
        if (!placesModel->isDevice(index)) {
            edit = menu.addAction(KIcon("document-properties"), i18n("&Edit '%1'...", label));
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
        QModelIndex current = placesModel->closestItem(d->currentUrl);

        if (index!=current && !d->showAll && hide->isChecked()) {
            delegate->addDisappearingItem(index);

            if (d->itemDisappearTimeline.state()!=QTimeLine::Running) {
                delegate->setDisappearingItemProgress(0.0);
                d->itemDisappearTimeline.start();
            }
        }
    } else if (showAll != 0 && result == showAll) {
        setShowAll(showAll->isChecked());
    } else if (teardown != 0 && result == teardown) {
        placesModel->requestTeardown(index);
    }

    index = placesModel->closestItem(d->currentUrl);
    selectionModel()->setCurrentIndex(index, QItemSelectionModel::ClearAndSelect);
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

void KFilePlacesView::setModel(QAbstractItemModel *model)
{
    QListView::setModel(model);
    d->updateHiddenRows();
}

void KFilePlacesView::rowsInserted(const QModelIndex &parent, int start, int end)
{
    QListView::rowsInserted(parent, start, end);
    setUrl(d->currentUrl);

    KFilePlacesViewDelegate *delegate = dynamic_cast<KFilePlacesViewDelegate*>(itemDelegate());
    KFilePlacesModel *placesModel = qobject_cast<KFilePlacesModel*>(model());

    for (int i=start; i<=end; ++i) {
        QModelIndex index = placesModel->index(i, 0, parent);
        if (d->showAll || !placesModel->isHidden(index)) {
            delegate->addAppearingItem(index);
        } else {
            setRowHidden(i, true);
        }
    }

    if (d->itemAppearTimeline.state()!=QTimeLine::Running) {
        delegate->setAppearingItemProgress(0.0);
        d->itemAppearTimeline.start();
    }

    d->adaptItemSize();
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
    _k_placeActivated(index);
}

void KFilePlacesView::Private::_k_placeActivated(const QModelIndex &index)
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

    delegate->setAppearingItemProgress(value);
    q->scheduleDelayedItemsLayout();
}

void KFilePlacesView::Private::_k_itemDisappearUpdate(qreal value)
{
    KFilePlacesViewDelegate *delegate = dynamic_cast<KFilePlacesViewDelegate*>(q->itemDelegate());

    delegate->setDisappearingItemProgress(value);

    if (value>=1.0) {
        updateHiddenRows();
    }

    q->scheduleDelayedItemsLayout();
}

void KFilePlacesView::Private::_k_enableSmoothItemResizing()
{
    smoothItemResizing = true;
}

void KFilePlacesView::dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    QListView::dataChanged(topLeft, bottomRight);
}

#include "kfileplacesview.moc"
