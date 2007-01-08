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
#include <kurl.h>
#include <QMimeData>
#include <QFile>
#include <QDir>
#include <sys/types.h>
#include <unistd.h> // ::access

class KDirModelNode;
class KDirModelDirNode;

// We create our own tree behind the scenes to have fast lookup from an item to its parent,
// and also to get the children of an item fast.
class KDirModelNode
{
public:
    KDirModelNode( KDirModelDirNode* parent, KFileItem* item ) :
        m_item(item),
        m_parent(parent),
        m_preview()
    {
    }
    //KUrl url() const { return m_item->url(); }
    // Careful, m_item can be 0 for the root item
    KFileItem* item() const { return m_item; }
    KDirModelDirNode* parent() const { return m_parent; }
    // linear search
    int rowNumber() const;
    QIcon preview() const { return m_preview; }
    void addPreview( const QPixmap& pix ) { m_preview.addPixmap(pix); }
    void setPreview( const QIcon& icn ) { m_preview = icn; }

private:
    KFileItem* m_item;
    KDirModelDirNode* const m_parent;
    QIcon m_preview;
};

// Specialization for directory nodes
class KDirModelDirNode : public KDirModelNode
{
public:
    KDirModelDirNode( KDirModelDirNode* parent, KFileItem* item )
        : KDirModelNode( parent, item),
          m_childNodes(),
          m_childCount(KDirModel::ChildCountUnknown)
    {}
    ~KDirModelDirNode() {
        qDeleteAll(m_childNodes);
    }
    QList<KDirModelNode *> m_childNodes;

    // If we listed the directory, the child count is known. Otherwise it can be set via setChildCount.
    int childCount() const { return m_childNodes.isEmpty() ? m_childCount : m_childNodes.count(); }
    void setChildCount(int count) { m_childCount = count; }

private:
    int m_childCount;
};

int KDirModelNode::rowNumber() const
{
    if (!m_parent) return 0;
    return m_parent->m_childNodes.indexOf(const_cast<KDirModelNode*>(this));
}

////

class KDirModelPrivate
{
public:
    KDirModelPrivate( KDirModel* model )
        : q(model), m_dirLister(0),
          m_rootNode(new KDirModelDirNode(0, 0)),
          m_dropsAllowed(KDirModel::NoDrops)
    {
    }
    ~KDirModelPrivate() {
        delete m_rootNode;
    }
    void clear() {
        delete m_rootNode;
        m_rootNode = new KDirModelDirNode(0, 0);
    }
    QPair<int /*row*/, KDirModelNode*> nodeForUrl(const KUrl& url) const;
    KDirModelNode* nodeForIndex(const QModelIndex& index) const;
    QModelIndex indexForNode(KDirModelNode* node, int rowNumber = -1 /*unknown*/) const;
    QModelIndex indexForUrl(const KUrl& url) const;
    bool isDir(KDirModelNode* node) const {
        return (node == m_rootNode) || node->item()->isDir();
    }

    KDirModel* q;
    KDirLister* m_dirLister;
    KDirModelDirNode* m_rootNode;
    KDirModel::DropsAllowed m_dropsAllowed;
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
        return qMakePair(0, static_cast<KDirModelNode *>(m_rootNode));

    const QString urlStr = url.url();
    KDirModelDirNode* dirNode = m_rootNode;
    KUrl nodeUrl = m_dirLister->url();
    while ( nodeUrl != url ) {
        Q_ASSERT( urlStr.startsWith(nodeUrl.url()) );
        bool foundChild = false;
        QList<KDirModelNode *>::const_iterator it = dirNode->m_childNodes.begin();
        const QList<KDirModelNode *>::const_iterator end = dirNode->m_childNodes.end();
        int row = 0;
        for ( ; it != end ; ++it, ++row ) {
            const KUrl u = (*it)->item()->url();
            if ( u == url ) {
                //kDebug() << "Found! " << u << endl;
                return qMakePair(row, *it);
            }
            if ( urlStr.startsWith(u.url()+'/') ) {
                //kDebug() << "going into " << node->item()->url() << endl;
                Q_ASSERT( isDir(*it) );
                dirNode = static_cast<KDirModelDirNode *>( *it );
                foundChild = true;
                break;
            }
        }
        if (!foundChild) {
            //kDebug() << "child equal or starting with " << url << " not found" << endl;
            return qMakePair(0, static_cast<KDirModelNode*>(0));
        }
        nodeUrl = dirNode->item()->url();
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
    setDirLister(new KDirLister(this));
}

KDirModel::~KDirModel()
{
}

void KDirModel::setDirLister(KDirLister* dirLister)
{
    if (d->m_dirLister) {
        d->clear();
        delete d->m_dirLister;
    }
    d->m_dirLister = dirLister;
    d->m_dirLister->setParent(this);
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
    Q_ASSERT(result.second);
    Q_ASSERT(d->isDir(result.second));
    KDirModelDirNode* dirNode = static_cast<KDirModelDirNode *>(result.second);

    KFileItemList::const_iterator it = items.begin();
    const KFileItemList::const_iterator end = items.end();
    for ( ; it != end ; ++it ) {
        KDirModelNode* node = (*it)->isDir()
                              ? new KDirModelDirNode( dirNode, *it )
                              : new KDirModelNode( dirNode, *it );
        dirNode->m_childNodes.append( node );
    }

    const QModelIndex index = d->indexForNode(dirNode, result.first); // O(1)
    const int newRowCount = dirNode->m_childNodes.count();
#ifndef NDEBUG // debugIndex only defined in debug mode
    kDebug() << k_funcinfo << items.count() << " in " << dir
             << " index=" << debugIndex(index) << " newRowCount=" << newRowCount << endl;
#endif
    const int newItemsCount = items.count();
    beginInsertRows( index, newRowCount - newItemsCount, newRowCount - 1 ); // parent, first, last
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

    KDirModelDirNode* dirNode = node->parent();
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
#ifndef NDEBUG // debugIndex only defined in debug mode
    kDebug() << "slotRefreshItems: dataChanged(" << debugIndex(topLeft) << " - " << debugIndex(bottomRight) << endl;
#endif
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

void KDirModel::itemChanged( const QModelIndex& index )
{
    emit dataChanged(index, index);
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
        case FileItemRole:
            return QVariant::fromValue(item);
        case ChildCountRole:
            if (!item->isDir())
                return ChildCountUnknown;
            else {
                KDirModelDirNode* dirNode = static_cast<KDirModelDirNode *>(node);
                int count = dirNode->childCount();
                if (count == ChildCountUnknown && item->isReadable()) {
                    const QString path = item->localPath();
                    if (!path.isEmpty()) {
                        QDir dir(path);
                        count = dir.entryList(QDir::AllEntries|QDir::NoDotAndDotDot|QDir::System).count();
                        kDebug() << k_funcinfo << "child count for " << path << ":" << count << endl;
                        dirNode->setChildCount(count);
                    }
                }
                return count;
            }
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
    KDirModelDirNode* parentNode = static_cast<KDirModelDirNode *>(d->nodeForIndex(parent));
    Q_ASSERT(parentNode);
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
    return d->indexForNode(parentNode); // O(n)
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
    // Note that we can only use the URL here, not the pointer.
    // KFileItems can be copied.
    return d->indexForUrl(item->url()); // O(n*m)
}

QModelIndex KDirModel::indexForItem( const KFileItem& item ) const
{
    // Note that we can only use the URL here, not the pointer.
    // KFileItems can be copied.
    return d->indexForUrl(item.url()); // O(n*m)
}

QModelIndex KDirModel::index( int row, int column, const QModelIndex & parent ) const
{
    KDirModelNode* parentNode = d->nodeForIndex(parent); // O(1)
    Q_ASSERT(parentNode);
    Q_ASSERT(d->isDir(parentNode));
    KDirModelNode* childNode = static_cast<KDirModelDirNode *>(parentNode)->m_childNodes.value(row); // O(1)
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

    // Allow dropping onto this item?
    if (d->m_dropsAllowed != NoDrops) {
        KFileItem* item = itemForIndex(index);
        if (!item || item->isDir()) {
            if (d->m_dropsAllowed & DropOnDirectory) {
                f |= Qt::ItemIsDropEnabled;
            }
        } else { // regular file item
            if (d->m_dropsAllowed & DropOnAnyFile)
                f |= Qt::ItemIsDropEnabled;
            else if (d->m_dropsAllowed & DropOnLocalExecutable) {
                if (item->isLocalFile()) {
                    // Desktop file?
                    if (item->mimeTypePtr()->is("application/x-desktop"))
                        f |= Qt::ItemIsDropEnabled;
                    // Executable, shell script ... ?
                    else if ( ::access( QFile::encodeName(item->localPath()), X_OK ) == 0 )
                        f |= Qt::ItemIsDropEnabled;
                }
            }
        }
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
        && static_cast<KDirModelDirNode *>(node)->m_childNodes.isEmpty();
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
    // Not sure we want to implement any drop handling at this level,
    // but for sure the default QAbstractItemModel implementation makes no sense for a dir model.
    Q_UNUSED(data);
    Q_UNUSED(action);
    Q_UNUSED(row);
    Q_UNUSED(column);
    Q_UNUSED(parent);
    return false;
}

void KDirModel::setDropsAllowed(DropsAllowed dropsAllowed)
{
    d->m_dropsAllowed = dropsAllowed;
}

#include "kdirmodel.moc"
