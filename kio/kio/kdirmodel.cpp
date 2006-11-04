/* This file is part of the KDE project
   Copyright (C) 2006 David Faure <faure@kde.org>

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

#include "kdirmodel.h"
#include "kdirlister.h"
#include "kfileitem.h"
#include <kdatetime.h>
#include <kicon.h>
#include <klocale.h>
#include <kglobal.h>
#include <QMimeData>

class KDirModelNode;

// We create our own tree behind the scenes to have fast lookup from an item to its parent,
// and also to get the children of an item fast.
class KDirModelNode
{
public:
    KDirModelNode( KDirModelNode* parent, KFileItem* item ) :
        m_childNodes(),
        m_item(item),
        m_parent(parent),
        m_preview()
    {
    }
    ~KDirModelNode() {
        qDeleteAll(m_childNodes);
    }
    //KUrl url() const { return m_item->url(); }
    // Careful, m_item can be 0 for the root item
    KFileItem* item() const { return m_item; }
    KDirModelNode* parent() const { return m_parent; }
    // linear search
    int rowNumber() const {
        if (!m_parent) return 0;
        return m_parent->m_childNodes.indexOf(const_cast<KDirModelNode*>(this));
    }
    QIcon preview() const { return m_preview; }
    void addPreview( const QPixmap& pix ) { m_preview.addPixmap(pix); }
    void setPreview( const QIcon& icn ) { m_preview = icn; }

    // TODO maybe move this field to a DirNode subclass to save 4 bytes per file?
    QList<KDirModelNode *> m_childNodes;
private:
    KFileItem* m_item;
    KDirModelNode* const m_parent;
    QIcon m_preview;
};

class KDirModelPrivate
{
public:
    KDirModelPrivate( KDirModel* model )
        : q(model), m_dirLister(0), m_rootNode(new KDirModelNode(0, 0)) {
    }
    ~KDirModelPrivate() {
        delete m_rootNode;
    }
    void clear() {
        delete m_rootNode;
        m_rootNode = new KDirModelNode(0, 0);
    }
    QPair<int /*row*/, KDirModelNode*> nodeForUrl(const KUrl& url) const;
    KDirModelNode* nodeForIndex(const QModelIndex& index) const;
    QModelIndex indexForNode(KDirModelNode* node, int rowNumber = -1 /*unknown*/) const;
    QModelIndex indexForUrl(const KUrl& url) const;

    KDirModel* q;
    KDirLister* m_dirLister;
    KDirModelNode* m_rootNode;
};

// If we want to support arbitrary trees like "home:/ as a child of system:/" then,
// we need to get the parent KFileItem in slotNewItems, and then we can use a QHash<KFileItem*,KDirModelNode*> cache.
// For now we'll assume "child url = parent url + filename"
QPair<int /*row*/, KDirModelNode*> KDirModelPrivate::nodeForUrl(const KUrl& _url) const // O(n*m)
{
    KUrl url(_url);
    url.adjustPath(KUrl::RemoveTrailingSlash);
    //kDebug() << k_funcinfo << url << endl;
    if (url == m_dirLister->url())
        return qMakePair(0, m_rootNode);

    const QString urlStr = url.url();
    KDirModelNode* node = m_rootNode;
    KUrl nodeUrl = m_dirLister->url();
    while ( nodeUrl != url ) {
        Q_ASSERT( urlStr.startsWith(nodeUrl.url()) );
        bool foundChild = false;
        QList<KDirModelNode *>::const_iterator it = node->m_childNodes.begin();
        const QList<KDirModelNode *>::const_iterator end = node->m_childNodes.end();
        int row = 0;
        for ( ; it != end ; ++it, ++row ) {
            const KUrl u = (*it)->item()->url();
            if ( u == url ) {
                //kDebug() << "Found! " << u << endl;
                return qMakePair(row, *it);
            }
            if ( urlStr.startsWith(u.url()+'/') ) {
                node = *it;
                foundChild = true;
                //kDebug() << "going into " << node->item()->url() << endl;
                Q_ASSERT( node->item()->isDir() );
                break;
            }
        }
        if (!foundChild) {
            //kDebug() << "child equal or starting with " << url << " not found" << endl;
            return qMakePair(0, static_cast<KDirModelNode*>(0));
        }
        nodeUrl = node->item()->url();
        //kDebug() << " " << nodeUrl << endl;
    }
    return qMakePair(0, static_cast<KDirModelNode*>(0));
}

// node -> index. If rowNumber is set (or node is root): O(1). Otherwise: O(n).
QModelIndex KDirModelPrivate::indexForNode(KDirModelNode* node, int rowNumber) const
{
    if (node == m_rootNode)
        return QModelIndex();

    Q_ASSERT(node->parent());
    return q->createIndex(rowNumber == -1 ? node->rowNumber() : rowNumber, 0, node);
}

// index -> node. O(1)
KDirModelNode* KDirModelPrivate::nodeForIndex(const QModelIndex& index) const
{
    return index.isValid()
        ? static_cast<KDirModelNode*>(index.internalPointer())
        : m_rootNode;
}

// url -> index. O(n*m)
QModelIndex KDirModelPrivate::indexForUrl(const KUrl& url) const
{
    const QPair<int, KDirModelNode*> result = nodeForUrl(url); // O(n*m) (m is the depth from the root)
    if (!result.second) {
        kWarning() << "KDirModelPrivate::indexForUrl: " << url << " not found" << endl;
        return QModelIndex();
    }
    return indexForNode(result.second, result.first); // O(1)
}


// We don't use QHash<KUrl,...> anymore, it's too slow.
// Idea from George, to make QHash<KUrl,...> fast: - cache hash value into QUrl or KUrl
// This also helps making operator== fast [which means operator== has to call qHash if cached value isn't there]
// But it means invalidating the cached hash value when the url is modified,
// so it can't be done in the current KUrl due to the public inheritance from QUrl.


/*
 * This model wraps the data held by KDirLister.
 *
 * The internal pointer of the QModelIndex for a given file is the node for that file in our own tree.
 * E.g. index(2,0) returns a QModelIndex with row=2 internalPointer=<KDirModelNode for the 3rd child of the root>
 *
 * Invalid parent index means root of the tree, m_rootNode
 */

#ifndef NDEBUG
static QString debugIndex(const QModelIndex& index)
{
    QString str;
    if (!index.isValid())
        str = "[invalid index, i.e. root]";
    else {
        KDirModelNode* node = static_cast<KDirModelNode*>(index.internalPointer());
        str = "[index for " + node->item()->url().pathOrUrl();
        if (index.column() > 0)
            str += ", column " + QString::number(index.column());
        str += ']';
    }
    return str;
}
#endif

KDirModel::KDirModel(QObject* parent)
    : QAbstractItemModel(parent),
      d(new KDirModelPrivate(this))
{
    setDirLister(new KDirLister);
}

KDirModel::~KDirModel()
{
    delete d->m_dirLister;
}

void KDirModel::setDirLister(KDirLister* dirLister)
{
    if (d->m_dirLister) {
        d->clear();
        delete d->m_dirLister;
    }
    d->m_dirLister = dirLister;
    connect( d->m_dirLister, SIGNAL(newItems(KFileItemList)),
             this, SLOT(slotNewItems(KFileItemList)) );
    connect( d->m_dirLister, SIGNAL(deleteItem(KFileItem*)),
             this, SLOT(slotDeleteItem(KFileItem*)) );
    connect( d->m_dirLister, SIGNAL(refreshItems(KFileItemList)),
             this, SLOT(slotRefreshItems(KFileItemList)) );
    connect( d->m_dirLister, SIGNAL(clear()),
             this, SLOT(slotClear()) );
}

KDirLister* KDirModel::dirLister() const
{
    return d->m_dirLister;
}

void KDirModel::slotNewItems( const KFileItemList& items )
{
    // Find parent item - it's the same for all the items
    // TODO (if Michael Brade agrees): add parent KFileItem or url to the newItems signal
    // This way we can finally support properly trees where the urls are using different protocols.
    KUrl dir( items.first()->url().upUrl() );
    dir.adjustPath(KUrl::RemoveTrailingSlash);

    //kDebug() << k_funcinfo << "dir=" << dir << endl;

    const QPair<int, KDirModelNode*> result = d->nodeForUrl(dir); // O(n*m)
    KDirModelNode* dirNode = result.second;

    Q_ASSERT(dirNode);
    KFileItemList::const_iterator it = items.begin();
    const KFileItemList::const_iterator end = items.end();
    for ( ; it != end ; ++it ) {
        KDirModelNode* node = new KDirModelNode( dirNode, *it );
        dirNode->m_childNodes.append( node );
    }

    const QModelIndex index = d->indexForNode(dirNode, result.first); // O(1)
    const int newRowCount = dirNode->m_childNodes.count();
    kDebug() << k_funcinfo << items.count() << " in " << dir
             << " index=" << debugIndex(index) << " newRowCount=" << newRowCount << endl;
    const int newItemsCount = items.count();
    beginInsertRows( index, newRowCount - newItemsCount, newRowCount - 1 );
    endInsertRows();
}

void KDirModel::slotDeleteItem( KFileItem *item )
{
    kDebug() << k_funcinfo << item << endl;
    //KUrl dir( item->url().upUrl() );
    //dir.adjustPath(KUrl::RemoveTrailingSlash);

    const QPair<int, KDirModelNode*> result = d->nodeForUrl(item->url()); // O(n*m)
    const int rowNumber = result.first;
    KDirModelNode* node = result.second;
    Q_ASSERT(node);
    if (!node)
        return;

    KDirModelNode* dirNode = node->parent();
    Q_ASSERT(dirNode);
    dirNode->m_childNodes.removeAt(rowNumber);

    QModelIndex parentIndex = d->indexForNode(dirNode); // O(n)
    beginRemoveRows( parentIndex, rowNumber, rowNumber );
    endRemoveRows();
}

void KDirModel::slotRefreshItems( const KFileItemList& items )
{
    QModelIndex topLeft, bottomRight;

    // Solution 1: we could emit dataChanged for one row (if items.size()==1) or all rows
    // Solution 2: more fine-grained, actually figure out the beginning and end rows.
    for ( KFileItemList::const_iterator fit = items.begin(), fend = items.end() ; fit != fend ; ++fit ) {
        const QModelIndex index = d->indexForUrl( (*fit)->url() ); // O(n*m); maybe we could look up to the parent only once
        if (!topLeft.isValid() || index.row() < topLeft.row()) {
            topLeft = index;
        }
        if (!bottomRight.isValid() || index.row() > bottomRight.row()) {
            bottomRight = index;
        }
    }
    kDebug() << "slotRefreshItems: dataChanged(" << debugIndex(topLeft) << " - " << debugIndex(bottomRight) << endl;
    bottomRight = bottomRight.sibling(bottomRight.row(), ColumnCount-1);
    emit dataChanged(topLeft, bottomRight);
}

void KDirModel::slotClear()
{
    const int numRows = d->m_rootNode->m_childNodes.count();
    beginRemoveRows( QModelIndex(), 0, numRows );
    endRemoveRows();

    //emit layoutAboutToBeChanged();
    d->clear();
    //emit layoutChanged();
}

int KDirModel::columnCount( const QModelIndex & ) const
{
    return ColumnCount;
}

QVariant KDirModel::data( const QModelIndex & index, int role ) const
{
    if (index.isValid()) {
        KDirModelNode* node = static_cast<KDirModelNode*>(index.internalPointer());
        KFileItem* item = node->item();
        switch (role) {
        case Qt::DisplayRole:
            switch (index.column()) {
            case Name:
                return item->name();
            case Size:
                //
                //return KIO::convertSize(item->size());
                // Default to "file size in bytes" like in kde3's filedialog
                return KGlobal::locale()->formatNumber(item->size(), 0);
            case ModifiedTime: {
                KDateTime dt;
                dt.setTime_t(item->time(KIO::UDS_MODIFICATION_TIME));
                return KGlobal::locale()->formatDateTime(dt);
            }
            case Permissions:
                return item->permissionsString();
            case Owner:
                return item->user();
            case Group:
                return item->group();
            case FileItemRole:
                return item;
            }
            break;
        case Qt::DecorationRole:
            if (index.column() == Name) {
                if (!node->preview().isNull()) {
                    //kDebug() << item->url() << " preview found" << endl;
                    return node->preview();
                }
                Q_ASSERT(item);
                const int overlays = item->overlays();
                //kDebug() << item->url() << " overlays=" << overlays << endl;
                return KIcon(item->iconName(), 0, overlays);
            }
            break;
        }
    }
    return QVariant();
}

void KDirModel::sort( int column, Qt::SortOrder order )
{
    // Not implemented - we should probably use QSortFilterProxyModel instead.
    return QAbstractItemModel::sort(column, order);
}

bool KDirModel::setData( const QModelIndex & index, const QVariant & value, int role )
{
    switch (role) {
    case Qt::DisplayRole:
        // TODO handle renaming here?
        break;
    case Qt::DecorationRole:
        if (index.column() == Name) {
            Q_ASSERT(index.isValid());
            // Set new pixmap - e.g. preview
            KDirModelNode* node = static_cast<KDirModelNode*>(index.internalPointer());
            //kDebug() << "setting icon for " << node->item()->url() << endl;
            Q_ASSERT(node);
            if (value.type() == QVariant::Icon) {
                const QIcon icon(qvariant_cast<QIcon>(value));
                Q_ASSERT(!icon.isNull());
                node->setPreview(icon);
            } else if (value.type() == QVariant::Pixmap) {
                node->addPreview(qvariant_cast<QPixmap>(value));
            }
            emit dataChanged(index, index);
            return true;
        }
        break;
    default:
        break;
    }
    return false;
}

int KDirModel::rowCount( const QModelIndex & parent ) const
{
    KDirModelNode* parentNode = d->nodeForIndex(parent);
    const int count = parentNode->m_childNodes.count();
    //kDebug() << "rowCount for " << parentUrl << ": " << count << endl;;
    return count;
}

// sibling() calls parent() and isn't virtual! So parent() should be fast...
QModelIndex KDirModel::parent( const QModelIndex & index ) const
{
    if (!index.isValid())
        return QModelIndex();
    KDirModelNode* childNode = static_cast<KDirModelNode*>(index.internalPointer());
    Q_ASSERT(childNode);
    KDirModelNode* parentNode = childNode->parent();
    Q_ASSERT(parentNode);
    return d->indexForNode(parentNode);
}

QStringList KDirModel::mimeTypes( ) const
{
    return QStringList() << QLatin1String("text/uri-list")
                         << QLatin1String( "application/x-kde-cutselection" ) // TODO
                         << QLatin1String( "text/plain" )
                         << QLatin1String( "application/x-kde-urilist" );
}

QMimeData * KDirModel::mimeData( const QModelIndexList & indexes ) const
{
    KUrl::List urls;
    foreach ( QModelIndex index, indexes ) {
        urls << d->nodeForIndex( index )->item()->url();
    }
    QMimeData *data = new QMimeData();
    urls.populateMimeData( data );
    return data;
}

// Public API; not much point in calling it internally
KFileItem* KDirModel::itemForIndex( const QModelIndex& index ) const
{
    if (!index.isValid()) {
        return d->m_dirLister->rootItem();
    } else {
        return static_cast<KDirModelNode*>(index.internalPointer())->item();
    }
}

QModelIndex KDirModel::indexForItem( const KFileItem* item ) const
{
    return d->indexForUrl(item->url()); // O(n*m)
}

QModelIndex KDirModel::index( int row, int column, const QModelIndex & parent ) const
{
    KDirModelNode* parentNode = d->nodeForIndex(parent);
    Q_ASSERT(parentNode);
    KDirModelNode* childNode = parentNode->m_childNodes.value(row);
    if (childNode)
        return createIndex(row, column, childNode);
    else
        return QModelIndex();
}

QVariant KDirModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
    Q_UNUSED(orientation);
    switch (role) {
    case Qt::DisplayRole:
        switch (section) {
        case Name:
            return i18n("Name");
        case Size:
            return i18n("Size");
        case ModifiedTime:
            return i18n("Modified"); // was called "Date" in kde3's kfiledialog - which one is better?
        case Permissions:
            return i18n("Permissions");
        case Owner:
            return i18n("Owner");
        case Group:
            return i18n("Group");
        }
    }
    return QVariant();
}

bool KDirModel::hasChildren( const QModelIndex & parent ) const
{
    if (!parent.isValid())
        return true;

    KFileItem* parentItem = static_cast<KDirModelNode*>(parent.internalPointer())->item();
    Q_ASSERT(parentItem);
    return parentItem->isDir();
}

Qt::ItemFlags KDirModel::flags( const QModelIndex & index ) const
{
    Qt::ItemFlags f = Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled;

    if (index.isValid()) {
        KFileItem* item = static_cast<KDirModelNode*>(index.internalPointer())->item();
        if (item->isDir())
            f |= Qt::ItemIsDropEnabled;
    } else {
        f |= Qt::ItemIsDropEnabled;
    }

    return f;
}

bool KDirModel::canFetchMore( const QModelIndex & parent ) const
{
    if (!parent.isValid())
        return false;

    // We could have a bool KDirModelNode::m_populated,
    // to avoid calling fetchMore more than once on empty dirs.
    // But this wastes memory, and how often does someone open and re-open an empty dir in a treeview?
    // Maybe we can ask KDirLister "have you listed <url> already"? (to discuss with M. Brade)

    KDirModelNode* node = static_cast<KDirModelNode*>(parent.internalPointer());
    KFileItem* item = node->item();
    return item->isDir() /*&& !node->m_populated*/
        && node->m_childNodes.isEmpty();
}

void KDirModel::fetchMore( const QModelIndex & parent )
{
    if (!parent.isValid())
        return;

    //const KUrl url = urlForIndex(parent);
    //kDebug() << k_funcinfo << url << endl;

    KDirModelNode* parentNode = static_cast<KDirModelNode*>(parent.internalPointer());
    //parentNode->m_populated = true;
    KFileItem* parentItem = parentNode->item();
    Q_ASSERT(parentItem->isDir());

    const KUrl url = parentItem->url();
    d->m_dirLister->openUrl(url, true, true);
}

bool KDirModel::dropMimeData( const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex & parent )
{
    // TODO
    return QAbstractItemModel::dropMimeData(data, action, row, column, parent);
}

#include "kdirmodel.moc"
