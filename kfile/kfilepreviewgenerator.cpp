/*******************************************************************************
 *   Copyright (C) 2008-2009 by Peter Penz <peter.penz@gmx.at>                 *
 *                                                                             *
 *   This library is free software; you can redistribute it and/or             *
 *   modify it under the terms of the GNU Library General Public               *
 *   License as published by the Free Software Foundation; either              *
 *   version 2 of the License, or (at your option) any later version.          *
 *                                                                             *
 *   This library is distributed in the hope that it will be useful,           *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU         *
 *   Library General Public License for more details.                          *
 *                                                                             *
 *   You should have received a copy of the GNU Library General Public License *
 *   along with this library; see the file COPYING.LIB.  If not, write to      *
 *   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,      *
 *   Boston, MA 02110-1301, USA.                                               *
 *******************************************************************************/
#include <config.h> // for HAVE_XRENDER

#include "kfilepreviewgenerator.h"

#include "../kio/kio/defaultviewadapter_p.h"
#include "../kio/kio/imagefilter_p.h"
#include <kfileitem.h>
#include <kiconeffect.h>
#include <kio/previewjob.h>
#include <kdirlister.h>
#include <kdirmodel.h>
#include <kdebug.h>

#include <QApplication>
#include <QAbstractItemView>
#include <QAbstractProxyModel>
#include <QClipboard>
#include <QColor>
#include <QList>
#include <QListView>
#include <QPainter>
#include <QPixmap>
#include <QScrollBar>
#include <QIcon>

#if defined(Q_WS_X11) && defined(HAVE_XRENDER)
#  include <QX11Info>
#  include <X11/Xlib.h>
#  include <X11/extensions/Xrender.h>
#endif

/**
 * If the passed item view is an instance of QListView, expensive
 * layout operations are blocked in the constructor and are unblocked
 * again in the destructor.
 *
 * This helper class is a workaround for the following huge performance
 * problem when having directories with several 1000 items:
 * - each change of an icon emits a dataChanged() signal from the model
 * - QListView iterates through all items on each dataChanged() signal
 *   and invokes QItemDelegate::sizeHint()
 * - the sizeHint() implementation of KFileItemDelegate is quite complex,
 *   invoking it 1000 times for each icon change might block the UI
 *
 * QListView does not invoke QItemDelegate::sizeHint() when the
 * uniformItemSize property has been set to true, so this property is
 * set before exchanging a block of icons. It is important to reset
 * it again before the event loop is entered, otherwise QListView
 * would not get the correct size hints after dispatching the layoutChanged()
 * signal.
 */
class LayoutBlocker {
public:
    LayoutBlocker(QAbstractItemView* view) :
        m_uniformSizes(false),
        m_view(qobject_cast<QListView*>(view))
    {
        if (m_view != 0) {
            m_uniformSizes = m_view->uniformItemSizes();
            m_view->setUniformItemSizes(true);
        }
    }

    ~LayoutBlocker()
    {
        if (m_view != 0) {
            m_view->setUniformItemSizes(m_uniformSizes);
        }
    }

private:
    bool m_uniformSizes;
    QListView* m_view;
};

/** Helper class for drawing frames for image previews. */
class TileSet
{
public:
    enum { LeftMargin = 3, TopMargin = 2, RightMargin = 3, BottomMargin = 4 };

    enum Tile { TopLeftCorner = 0, TopSide, TopRightCorner, LeftSide,
                RightSide, BottomLeftCorner, BottomSide, BottomRightCorner,
                NumTiles };

    TileSet()
    {
        QImage image(8 * 3, 8 * 3, QImage::Format_ARGB32_Premultiplied);

        QPainter p(&image);
        p.setCompositionMode(QPainter::CompositionMode_Source);
        p.fillRect(image.rect(), Qt::transparent);
        p.fillRect(image.rect().adjusted(3, 3, -3, -3), Qt::black);
        p.end();

        KIO::ImageFilter::shadowBlur(image, 3, Qt::black);

        QPixmap pixmap = QPixmap::fromImage(image);
        m_tiles[TopLeftCorner]     = pixmap.copy(0, 0, 8, 8);
        m_tiles[TopSide]           = pixmap.copy(8, 0, 8, 8);
        m_tiles[TopRightCorner]    = pixmap.copy(16, 0, 8, 8);
        m_tiles[LeftSide]          = pixmap.copy(0, 8, 8, 8);
        m_tiles[RightSide]         = pixmap.copy(16, 8, 8, 8);
        m_tiles[BottomLeftCorner]  = pixmap.copy(0, 16, 8, 8);
        m_tiles[BottomSide]        = pixmap.copy(8, 16, 8, 8);
        m_tiles[BottomRightCorner] = pixmap.copy(16, 16, 8, 8);
    }

    void paint(QPainter* p, const QRect& r)
    {
        p->drawPixmap(r.topLeft(), m_tiles[TopLeftCorner]);
        if (r.width() - 16 > 0) {
            p->drawTiledPixmap(r.x() + 8, r.y(), r.width() - 16, 8, m_tiles[TopSide]);
        }
        p->drawPixmap(r.right() - 8 + 1, r.y(), m_tiles[TopRightCorner]);
        if (r.height() - 16 > 0) {
            p->drawTiledPixmap(r.x(), r.y() + 8, 8, r.height() - 16,  m_tiles[LeftSide]);
            p->drawTiledPixmap(r.right() - 8 + 1, r.y() + 8, 8, r.height() - 16, m_tiles[RightSide]);
        }
        p->drawPixmap(r.x(), r.bottom() - 8 + 1, m_tiles[BottomLeftCorner]);
        if (r.width() - 16 > 0) {
            p->drawTiledPixmap(r.x() + 8, r.bottom() - 8 + 1, r.width() - 16, 8, m_tiles[BottomSide]);
        }
        p->drawPixmap(r.right() - 8 + 1, r.bottom() - 8 + 1, m_tiles[BottomRightCorner]);

        const QRect contentRect = r.adjusted(LeftMargin + 1, TopMargin + 1,
                                             -(RightMargin + 1), -(BottomMargin + 1));
        p->fillRect(contentRect, Qt::transparent);
    }

private:
    QPixmap m_tiles[NumTiles];
};

class KFilePreviewGenerator::Private
{
public:
    Private(KFilePreviewGenerator* parent,
            KAbstractViewAdapter* viewAdapter,
            QAbstractItemModel* model);
    ~Private();

    /**
     * Generates previews for the items \a items asynchronously.
     */
    void updateIcons(const KFileItemList& items);

    /**
     * Generates previews for the indices within \a topLeft
     * and \a bottomRight asynchronously.
     */
    void updateIcons(const QModelIndex& topLeft, const QModelIndex& bottomRight);

    /**
     * Adds the preview \a pixmap for the item \a item to the preview
     * queue and starts a timer which will dispatch the preview queue
     * later.
     */
    void addToPreviewQueue(const KFileItem& item, const QPixmap& pixmap);

    /**
     * Is invoked when the preview job has been finished and
     * removes the job from the m_previewJobs list.
     */
    void slotPreviewJobFinished(KJob* job);

    /** Synchronizes the item icon with the clipboard of cut items. */
    void updateCutItems();

    /**
     * Dispatches the preview queue  block by block within
     * time slices.
     */
    void dispatchIconUpdateQueue();

    /**
     * Pauses all icon updates and invokes KFilePreviewGenerator::resumeIconUpdates()
     * after a short delay. Is invoked as soon as the user has moved
     * a scrollbar.
     */
    void pauseIconUpdates();

    /**
     * Resumes the icons updates that have been paused after moving the
     * scrollbar. The previews for the current visible area are
     * generated first.
     */
    void resumeIconUpdates();

    /**
     * Starts the resolving of the MIME types from
     * the m_pendingItems queue.
     */
    void startMimeTypeResolving();

    /**
     * Resolves the MIME type for exactly one item of the
     * m_pendingItems queue.
     */
    void resolveMimeType();

    /**
     * Returns true, if the item \a item has been cut into
     * the clipboard.
     */
    bool isCutItem(const KFileItem& item) const;

    /** Applies an item effect to all cut items. */
    void applyCutItemEffect();

    /**
     * Applies a frame around the icon. False is returned if
     * no frame has been added because the icon is too small.
     */
    bool applyImageFrame(QPixmap& icon);

    /**
     * Resizes the icon to \a maxSize if the icon size does not
     * fit into the maximum size. The aspect ratio of the icon
     * is kept.
     */
    void limitToSize(QPixmap& icon, const QSize& maxSize);

    /**
     * Starts a new preview job for the items \a to m_previewJobs
     * and triggers the preview timer.
     */
    void startPreviewJob(const KFileItemList& items);

    /** Kills all ongoing preview jobs. */
    void killPreviewJobs();

    /**
     * Orders the items \a items in a way that the visible items
     * are moved to the front of the list. When passing this
     * list to a preview job, the visible items will get generated
     * first.
     */
    void orderItems(KFileItemList& items);

    /**
     * Returns true, if \a mimeData represents a selection that has
     * been cut.
     */
    bool decodeIsCutSelection(const QMimeData* mimeData);

    /** Remembers the pixmap for an item specified by an URL. */
    struct ItemInfo
    {
        KUrl url;
        QPixmap pixmap;
    };

    /**
     * During the lifetime of a DataChangeObtainer instance changing
     * the data of the model won't trigger generating a preview.
     */
    class DataChangeObtainer
    {
    public:
        DataChangeObtainer(KFilePreviewGenerator::Private* generator) :
            m_gen(generator)  { ++m_gen->m_internalDataChange; }
        ~DataChangeObtainer() { --m_gen->m_internalDataChange; }
    private:
        KFilePreviewGenerator::Private* m_gen;
    };

    bool m_previewShown;

    /**
     * True, if m_pendingItems and m_dispatchedItems should be
     * cleared when the preview jobs have been finished.
     */
    bool m_clearItemQueues;

    /**
     * True if a selection has been done which should cut items.
     */
    bool m_hasCutSelection;

    /**
     * True if the updates of icons has been paused by pauseIconUpdates().
     * The value is reset by resumeIconUpdates().
     */
    bool m_iconUpdatesPaused;

    /**
     * If the value is 0, the slot
     * updateIcons(const QModelIndex&, const QModelIndex&) has
     * been triggered by an external data change.
     */
    int m_internalDataChange;

    int m_pendingVisibleIconUpdates;

    KAbstractViewAdapter* m_viewAdapter;
    QAbstractItemView* m_itemView;
    QTimer* m_iconUpdateTimer;
    QTimer* m_scrollAreaTimer;
    QList<KJob*> m_previewJobs;
    KDirModel* m_dirModel;
    QAbstractProxyModel* m_proxyModel;

    QList<ItemInfo> m_cutItemsCache;
    QList<ItemInfo> m_previews;

    /**
     * Contains all items where a preview must be generated, but
     * where the preview job has not dispatched the items yet.
     */
    KFileItemList m_pendingItems;

    /**
     * Contains all items, where a preview has already been
     * generated by the preview jobs.
     */
    KFileItemList m_dispatchedItems;

    KFileItemList m_resolvedMimeTypes;

    QStringList m_enabledPlugins;

    TileSet* m_tileSet;

private:
    KFilePreviewGenerator* const q;

};

KFilePreviewGenerator::Private::Private(KFilePreviewGenerator* parent,
                                        KAbstractViewAdapter* viewAdapter,
                                        QAbstractItemModel* model) :
    m_previewShown(true),
    m_clearItemQueues(true),
    m_hasCutSelection(false),
    m_iconUpdatesPaused(false),
    m_internalDataChange(0),
    m_pendingVisibleIconUpdates(0),
    m_viewAdapter(viewAdapter),
    m_itemView(0),
    m_iconUpdateTimer(0),
    m_scrollAreaTimer(0),
    m_previewJobs(),
    m_dirModel(0),
    m_proxyModel(0),
    m_cutItemsCache(),
    m_previews(),
    m_pendingItems(),
    m_dispatchedItems(),
    m_resolvedMimeTypes(),
    m_tileSet(0),
    q(parent)
{
    if (!m_viewAdapter->iconSize().isValid()) {
        m_previewShown = false;
    }

    m_proxyModel = qobject_cast<QAbstractProxyModel*>(model);
    m_dirModel = (m_proxyModel == 0) ?
                 qobject_cast<KDirModel*>(model) :
                 qobject_cast<KDirModel*>(m_proxyModel->sourceModel());
    if (m_dirModel == 0) {
        // previews can only get generated for directory models
        m_previewShown = false;
    } else {
        connect(m_dirModel->dirLister(), SIGNAL(newItems(const KFileItemList&)),
                q, SLOT(updateIcons(const KFileItemList&)));
        connect(m_dirModel, SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)),
                q, SLOT(updateIcons(const QModelIndex&, const QModelIndex&)));
    }

    QClipboard* clipboard = QApplication::clipboard();
    connect(clipboard, SIGNAL(dataChanged()),
            q, SLOT(updateCutItems()));

    m_iconUpdateTimer = new QTimer(q);
    m_iconUpdateTimer->setSingleShot(true);
    connect(m_iconUpdateTimer, SIGNAL(timeout()), q, SLOT(dispatchIconUpdateQueue()));

    // Whenever the scrollbar values have been changed, the pending previews should
    // be reordered in a way that the previews for the visible items are generated
    // first. The reordering is done with a small delay, so that during moving the
    // scrollbars the CPU load is kept low.
    m_scrollAreaTimer = new QTimer(q);
    m_scrollAreaTimer->setSingleShot(true);
    m_scrollAreaTimer->setInterval(200);
    connect(m_scrollAreaTimer, SIGNAL(timeout()),
            q, SLOT(resumeIconUpdates()));
    m_viewAdapter->connect(KAbstractViewAdapter::ScrollBarValueChanged,
                           q, SLOT(pauseIconUpdates()));
}

KFilePreviewGenerator::Private::~Private()
{
    killPreviewJobs();
    m_pendingItems.clear();
    m_dispatchedItems.clear();
    delete m_tileSet;
}

void KFilePreviewGenerator::Private::updateIcons(const KFileItemList& items)
{
    applyCutItemEffect();

    KFileItemList orderedItems = items;
    orderItems(orderedItems);

    foreach (const KFileItem& item, orderedItems) {
        m_pendingItems.append(item);
    }

    if (m_previewShown) {
        startPreviewJob(orderedItems);
    } else {
        startMimeTypeResolving();
    }
}

void KFilePreviewGenerator::Private::updateIcons(const QModelIndex& topLeft,
                                                 const QModelIndex& bottomRight)
{
    if (m_internalDataChange > 0) {
        // QAbstractItemModel::setData() has been invoked internally by the KFilePreviewGenerator.
        // The signal dataChanged() is connected with this method, but previews only need
        // to be generated when an external data change has occured.
        return;
    }

    KFileItemList itemList;
    for (int row = topLeft.row(); row <= bottomRight.row(); ++row) {
        const QModelIndex index = m_dirModel->index(row, 0);
        const KFileItem item = m_dirModel->itemForIndex(index);
        itemList.append(item);
    }
    updateIcons(itemList);
}

void KFilePreviewGenerator::Private::addToPreviewQueue(const KFileItem& item, const QPixmap& pixmap)
{
    if (!m_previewShown) {
        // the preview has been canceled in the meantime
        return;
    }
    const KUrl url = item.url();

    // check whether the item is part of the directory lister (it is possible
    // that a preview from an old directory lister is received)
    KDirLister* dirLister = m_dirModel->dirLister();
    bool isOldPreview = true;
    const KUrl::List dirs = dirLister->directories();
    const QString itemDir = url.directory();
    foreach (const KUrl& url, dirs) {
        if (url.path() == itemDir) {
            isOldPreview = false;
            break;
        }
    }
    if (isOldPreview) {
        return;
    }

    QPixmap icon = pixmap;

    const QString mimeType = item.mimetype();
    const QString mimeTypeGroup = mimeType.left(mimeType.indexOf('/'));
    if ((mimeTypeGroup != "image") || !applyImageFrame(icon)) {
        limitToSize(icon, m_viewAdapter->iconSize());
    }

    if (m_hasCutSelection && isCutItem(item)) {
        // Remember the current icon in the cache for cut items before
        // the disabled effect is applied. This makes it possible restoring
        // the uncut version again when cutting other items.
        QList<ItemInfo>::iterator begin = m_cutItemsCache.begin();
        QList<ItemInfo>::iterator end   = m_cutItemsCache.end();
        for (QList<ItemInfo>::iterator it = begin; it != end; ++it) {
            if ((*it).url == item.url()) {
                (*it).pixmap = icon;
                break;
            }
        }

        // apply the disabled effect to the icon for marking it as "cut item"
        // and apply the icon to the item
        KIconEffect iconEffect;
        icon = iconEffect.apply(icon, KIconLoader::Desktop, KIconLoader::DisabledState);
    }

    // remember the preview and URL, so that it can be applied to the model
    // in KFilePreviewGenerator::dispatchIconUpdateQueue()
    ItemInfo preview;
    preview.url = url;
    preview.pixmap = icon;
    m_previews.append(preview);

    m_dispatchedItems.append(item);
}

void KFilePreviewGenerator::Private::slotPreviewJobFinished(KJob* job)
{
    const int index = m_previewJobs.indexOf(job);
    m_previewJobs.removeAt(index);

    if ((m_previewJobs.count() == 0) && m_clearItemQueues) {
        m_pendingItems.clear();
        m_dispatchedItems.clear();
        m_pendingVisibleIconUpdates = 0;
        QMetaObject::invokeMethod(q, "dispatchIconUpdateQueue", Qt::QueuedConnection);
    }
}

void KFilePreviewGenerator::Private::updateCutItems()
{
    DataChangeObtainer obt(this);

    // restore the icons of all previously selected items to the
    // original state...
    foreach (const ItemInfo& cutItem, m_cutItemsCache) {
        const QModelIndex index = m_dirModel->indexForUrl(cutItem.url);
        if (index.isValid()) {
            m_dirModel->setData(index, QIcon(cutItem.pixmap), Qt::DecorationRole);
        }
    }
    m_cutItemsCache.clear();

    // ... and apply an item effect to all currently cut items
    applyCutItemEffect();
}

void KFilePreviewGenerator::Private::dispatchIconUpdateQueue()
{
    const int count = m_previewShown ? m_previews.count()
                                     : m_resolvedMimeTypes.count();
    if (count > 0) {
        LayoutBlocker blocker(m_itemView);
        DataChangeObtainer obt(this);

        if (m_previewShown) {
            // dispatch preview queue
            foreach (const ItemInfo& preview, m_previews) {
                const QModelIndex idx = m_dirModel->indexForUrl(preview.url);
                if (idx.isValid() && (idx.column() == 0)) {
                    m_dirModel->setData(idx, QIcon(preview.pixmap), Qt::DecorationRole);
                }
            }
            m_previews.clear();
        } else {
            // dispatch mime type queue
            foreach (const KFileItem& item, m_resolvedMimeTypes) {
                const QModelIndex idx = m_dirModel->indexForItem(item);
                m_dirModel->itemChanged(idx);
            }
            m_resolvedMimeTypes.clear();
        }

        m_pendingVisibleIconUpdates -= count;
        if (m_pendingVisibleIconUpdates < 0) {
            m_pendingVisibleIconUpdates = 0;
        }
    }

    if (m_pendingVisibleIconUpdates > 0) {
        // As long as there are pending previews for visible items, poll
        // the preview queue each 200 ms. If there are no pending previews,
        // the queue is dispatched in slotPreviewJobFinished().
        m_iconUpdateTimer->start(200);
    }
}

void KFilePreviewGenerator::Private::pauseIconUpdates()
{
    m_iconUpdatesPaused = true;
    foreach (KJob* job, m_previewJobs) {
        Q_ASSERT(job != 0);
        job->suspend();
    }
    m_scrollAreaTimer->start();
}

void KFilePreviewGenerator::Private::resumeIconUpdates()
{
    m_iconUpdatesPaused = false;

    // Before creating new preview jobs the m_pendingItems queue must be
    // cleaned up by removing the already dispatched items. Implementation
    // note: The order of the m_dispatchedItems queue and the m_pendingItems
    // queue is usually equal. So even when having a lot of elements the
    // nested loop is no performance bottle neck, as the inner loop is only
    // entered once in most cases.
    foreach (const KFileItem& item, m_dispatchedItems) {
        KFileItemList::iterator begin = m_pendingItems.begin();
        KFileItemList::iterator end   = m_pendingItems.end();
        for (KFileItemList::iterator it = begin; it != end; ++it) {
            if ((*it).url() == item.url()) {
                m_pendingItems.erase(it);
                break;
            }
        }
    }
    m_dispatchedItems.clear();

    m_pendingVisibleIconUpdates = 0;
    dispatchIconUpdateQueue();


    if (m_previewShown) {
        KFileItemList orderedItems = m_pendingItems;
        orderItems(orderedItems);

        // Kill all suspended preview jobs. Usually when a preview job
        // has been finished, slotPreviewJobFinished() clears all item queues.
        // This is not wanted in this case, as a new job is created afterwards
        // for m_pendingItems.
        m_clearItemQueues = false;
        killPreviewJobs();
        m_clearItemQueues = true;

        startPreviewJob(orderedItems);
    } else {
        orderItems(m_pendingItems);
        startMimeTypeResolving();
    }
}

void KFilePreviewGenerator::Private::startMimeTypeResolving()
{
    resolveMimeType();
    m_iconUpdateTimer->start(200);
}

void KFilePreviewGenerator::Private::resolveMimeType()
{
    if (m_pendingItems.isEmpty()) {
        return;
    }

    // resolve at least one MIME type
    bool resolved = false;
    do {
        KFileItem item = m_pendingItems.takeFirst();
        if (item.isMimeTypeKnown()) {
            if (m_pendingVisibleIconUpdates > 0) {
                // The item is visible and the MIME type already known.
                // Decrease the update counter for dispatchIconUpdateQueue():
                --m_pendingVisibleIconUpdates;
            }
        } else {
            // The MIME type is unknown and must get resolved. The
            // directory model is not informed yet, as a single update
            // would be very expensive. Instead the item is remembered in
            // m_resolvedMimeTypes and will be dispatched later
            // by dispatchIconUpdateQueue().
            item.determineMimeType();
            m_resolvedMimeTypes.append(item);
            resolved = true;
        }
    } while (!resolved && !m_pendingItems.isEmpty());

    if (m_pendingItems.isEmpty()) {
        // All MIME types have been resolved now. Assure
        // that the directory model gets informed about
        // this, so that an update of the icons is done.
        dispatchIconUpdateQueue();
    } else if (!m_iconUpdatesPaused) {
        // assure that the MIME type of the next
        // item will be resolved asynchronously
        QMetaObject::invokeMethod(q, "resolveMimeType", Qt::QueuedConnection);
    }
}

bool KFilePreviewGenerator::Private::isCutItem(const KFileItem& item) const
{
    const QMimeData* mimeData = QApplication::clipboard()->mimeData();
    const KUrl::List cutUrls = KUrl::List::fromMimeData(mimeData);
    return cutUrls.contains(item.url());
}

void KFilePreviewGenerator::Private::applyCutItemEffect()
{
    const QMimeData* mimeData = QApplication::clipboard()->mimeData();
    m_hasCutSelection = decodeIsCutSelection(mimeData);
    if (!m_hasCutSelection) {
        return;
    }

    const QSet<KUrl> cutUrls = KUrl::List::fromMimeData(mimeData).toSet();

    KFileItemList items;
    KDirLister* dirLister = m_dirModel->dirLister();
    const KUrl::List dirs = dirLister->directories();
    foreach (const KUrl& url, dirs) {
        items << dirLister->itemsForDir(url);
    }

    DataChangeObtainer obt(this);
    foreach (const KFileItem& item, items) {
        if (cutUrls.contains(item.url())) {
            const QModelIndex index = m_dirModel->indexForItem(item);
            const QVariant value = m_dirModel->data(index, Qt::DecorationRole);
            if (value.type() == QVariant::Icon) {
                const QIcon icon(qvariant_cast<QIcon>(value));
                const QSize actualSize = icon.actualSize(m_viewAdapter->iconSize());
                QPixmap pixmap = icon.pixmap(actualSize);

                // remember current pixmap for the item to be able
                // to restore it when other items get cut
                ItemInfo cutItem;
                cutItem.url = item.url();
                cutItem.pixmap = pixmap;
                m_cutItemsCache.append(cutItem);

                // apply icon effect to the cut item
                KIconEffect iconEffect;
                pixmap = iconEffect.apply(pixmap, KIconLoader::Desktop, KIconLoader::DisabledState);
                m_dirModel->setData(index, QIcon(pixmap), Qt::DecorationRole);
            }
        }
    }
}

bool KFilePreviewGenerator::Private::applyImageFrame(QPixmap& icon)
{
    const QSize maxSize = m_viewAdapter->iconSize();
    const bool applyFrame = (maxSize.width()  > KIconLoader::SizeSmallMedium) &&
                            (maxSize.height() > KIconLoader::SizeSmallMedium) &&
                            ((icon.width()  > KIconLoader::SizeLarge) ||
                             (icon.height() > KIconLoader::SizeLarge));
    if (!applyFrame) {
        // the maximum size or the image itself is too small for a frame
        return false;
    }

    // resize the icon to the maximum size minus the space required for the frame
    const QSize size(maxSize.width() - TileSet::LeftMargin - TileSet::RightMargin,
                     maxSize.height() - TileSet::TopMargin - TileSet::BottomMargin);
    limitToSize(icon, size);

    if (m_tileSet == 0) {
        m_tileSet = new TileSet();
    }

    QPixmap framedIcon(icon.size().width() + TileSet::LeftMargin + TileSet::RightMargin,
                       icon.size().height() + TileSet::TopMargin + TileSet::BottomMargin);
    framedIcon.fill(Qt::transparent);

    QPainter painter;
    painter.begin(&framedIcon);
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    m_tileSet->paint(&painter, framedIcon.rect());
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
    painter.drawPixmap(TileSet::LeftMargin, TileSet::TopMargin, icon);
    painter.end();

    icon = framedIcon;
    return true;
}

void KFilePreviewGenerator::Private::limitToSize(QPixmap& icon, const QSize& maxSize)
{
    if ((icon.width() > maxSize.width()) || (icon.height() > maxSize.height())) {
#if defined(Q_WS_X11) && defined(HAVE_XRENDER)
        // Assume that the texture size limit is 2048x2048
        if ((icon.width() <= 2048) && (icon.height() <= 2048) && icon.x11PictureHandle()) {
            QSize size = icon.size();
            size.scale(maxSize, Qt::KeepAspectRatio);

            const qreal factor = size.width() / qreal(icon.width());

            XTransform xform = {{
                { XDoubleToFixed(1 / factor), 0, 0 },
                { 0, XDoubleToFixed(1 / factor), 0 },
                { 0, 0, XDoubleToFixed(1) }
            }};

            QPixmap pixmap(size);
            pixmap.fill(Qt::transparent);

            Display* dpy = QX11Info::display();
            XRenderSetPictureFilter(dpy, icon.x11PictureHandle(), FilterBilinear, 0, 0);
            XRenderSetPictureTransform(dpy, icon.x11PictureHandle(), &xform);
            XRenderComposite(dpy, PictOpOver, icon.x11PictureHandle(), None, pixmap.x11PictureHandle(),
                             0, 0, 0, 0, 0, 0, pixmap.width(), pixmap.height());
            icon = pixmap;
        } else {
            icon = icon.scaled(maxSize, Qt::KeepAspectRatio, Qt::FastTransformation);
        }
#else
        icon = icon.scaled(maxSize, Qt::KeepAspectRatio, Qt::FastTransformation);
#endif
    }
}

void KFilePreviewGenerator::Private::startPreviewJob(const KFileItemList& items)
{
    if (items.count() == 0) {
        return;
    }

    const QMimeData* mimeData = QApplication::clipboard()->mimeData();
    m_hasCutSelection = decodeIsCutSelection(mimeData);

    const QSize size = m_viewAdapter->iconSize();

    // PreviewJob internally caches items always with the size of
    // 128 x 128 pixels or 256 x 256 pixels. A downscaling is done
    // by PreviewJob if a smaller size is requested. As the KFilePreviewGenerator must
    // do a downscaling anyhow because of the frame, only the provided
    // cache sizes are requested.
    const int cacheSize = (size.width() > 128) || (size.height() > 128) ? 256 : 128;
    KIO::PreviewJob* job = KIO::filePreview(items, cacheSize, cacheSize, 0, 70, true, true, &m_enabledPlugins);
    connect(job, SIGNAL(gotPreview(const KFileItem&, const QPixmap&)),
            q, SLOT(addToPreviewQueue(const KFileItem&, const QPixmap&)));
    connect(job, SIGNAL(finished(KJob*)),
            q, SLOT(slotPreviewJobFinished(KJob*)));

    m_previewJobs.append(job);
    m_iconUpdateTimer->start(200);
}

void KFilePreviewGenerator::Private::killPreviewJobs()
{
    foreach (KJob* job, m_previewJobs) {
        Q_ASSERT(job != 0);
        job->kill();
    }
    m_previewJobs.clear();
}

void KFilePreviewGenerator::Private::orderItems(KFileItemList& items)
{
    // Order the items in a way that the preview for the visible items
    // is generated first, as this improves the feeled performance a lot.
    //
    // Implementation note: 2 different algorithms are used for the sorting.
    // Algorithm 1 is faster when having a lot of items in comparison
    // to the number of rows in the model. Algorithm 2 is faster
    // when having quite less items in comparison to the number of rows in
    // the model. Choosing the right algorithm is important when having directories
    // with several hundreds or thousands of items.

    const bool hasProxy = (m_proxyModel != 0);
    const int itemCount = items.count();
    const int rowCount = hasProxy ? m_proxyModel->rowCount() : m_dirModel->rowCount();
    const QRect visibleArea = m_viewAdapter->visibleArea();

    QModelIndex dirIndex;
    QRect itemRect;
    int insertPos = 0;
    if (itemCount * 10 > rowCount) {
        // Algorithm 1: The number of items is > 10 % of the row count. Parse all rows
        // and check whether the received row is part of the item list.
        for (int row = 0; row < rowCount; ++row) {
            if (hasProxy) {
                const QModelIndex proxyIndex = m_proxyModel->index(row, 0);
                itemRect = m_viewAdapter->visualRect(proxyIndex);
                dirIndex = m_proxyModel->mapToSource(proxyIndex);
            } else {
                dirIndex = m_dirModel->index(row, 0);
                itemRect = m_viewAdapter->visualRect(dirIndex);
            }

            KFileItem item = m_dirModel->itemForIndex(dirIndex);  // O(1)
            const KUrl url = item.url();

            // check whether the item is part of the item list 'items'
            int index = -1;
            for (int i = 0; i < itemCount; ++i) {
                if (items.at(i).url() == url) {
                    index = i;
                    break;
                }
            }

            if ((index > 0) && itemRect.intersects(visibleArea)) {
                // The current item is (at least partly) visible. Move it
                // to the front of the list, so that the preview is
                // generated earlier.
                items.removeAt(index);
                items.insert(insertPos, item);
                ++insertPos;
                ++m_pendingVisibleIconUpdates;
            }
        }
    } else {
        // Algorithm 2: The number of items is <= 10 % of the row count. In this case iterate
        // all items and receive the corresponding row from the item.
        for (int i = 0; i < itemCount; ++i) {
            dirIndex = m_dirModel->indexForItem(items.at(i)); // O(n) (n = number of rows)
            if (hasProxy) {
                const QModelIndex proxyIndex = m_proxyModel->mapFromSource(dirIndex);
                itemRect = m_viewAdapter->visualRect(proxyIndex);
            } else {
                itemRect = m_viewAdapter->visualRect(dirIndex);
            }

            if (itemRect.intersects(visibleArea)) {
                // The current item is (at least partly) visible. Move it
                // to the front of the list, so that the preview is
                // generated earlier.
                items.insert(insertPos, items.at(i));
                items.removeAt(i + 1);
                ++insertPos;
                ++m_pendingVisibleIconUpdates;
            }
        }
    }
}

bool KFilePreviewGenerator::Private::decodeIsCutSelection(const QMimeData* mimeData)
{
    const QByteArray data = mimeData->data("application/x-kde-cutselection");
    if (data.isEmpty()) {
        return false;
    } else {
        return data.at(0) == '1';
    }
}

KFilePreviewGenerator::KFilePreviewGenerator(QAbstractItemView* parent) :
    QObject(parent),
    d(new Private(this, new KIO::DefaultViewAdapter(parent, this), parent->model()))
{
    d->m_itemView = parent;
}

KFilePreviewGenerator::KFilePreviewGenerator(KAbstractViewAdapter* parent, QAbstractProxyModel* model) :
    QObject(parent),
    d(new Private(this, parent, model))
{
}

KFilePreviewGenerator::~KFilePreviewGenerator()
{
    delete d;
}

void KFilePreviewGenerator::setPreviewShown(bool show)
{
    if (show && (!d->m_viewAdapter->iconSize().isValid() || (d->m_dirModel == 0))) {
        // the view must provide an icon size and a directory model,
        // otherwise the showing the previews will get ignored
        return;
    }

    if (d->m_previewShown != show) {
        d->m_previewShown = show;
        d->m_cutItemsCache.clear();
        d->updateCutItems();
        if (show) {
            updatePreviews();
        }
    }
}

bool KFilePreviewGenerator::isPreviewShown() const
{
    return d->m_previewShown;
}

void KFilePreviewGenerator::updatePreviews()
{
    if (!d->m_previewShown) {
        return;
    }

    d->killPreviewJobs();
    d->m_cutItemsCache.clear();
    d->m_pendingItems.clear();
    d->m_dispatchedItems.clear();

    KFileItemList itemList;
    const int rowCount = d->m_dirModel->rowCount();
    for (int row = 0; row < rowCount; ++row) {
        const QModelIndex index = d->m_dirModel->index(row, 0);
        KFileItem item = d->m_dirModel->itemForIndex(index);
        itemList.append(item);
    }

    d->updateIcons(itemList);
    d->updateCutItems();
}

void KFilePreviewGenerator::cancelPreviews()
{
    d->killPreviewJobs();
    d->m_cutItemsCache.clear();
    d->m_pendingItems.clear();
    d->m_dispatchedItems.clear();
}

void KFilePreviewGenerator::setEnabledPlugins(const QStringList& plugins)
{
    d->m_enabledPlugins = plugins;
}

QStringList KFilePreviewGenerator::enabledPlugins() const
{
    return d->m_enabledPlugins;
}

#include "kfilepreviewgenerator.moc"
