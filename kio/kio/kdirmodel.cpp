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
#include <kio/copyjob.h>
#include <kio/jobuidelegate.h>
#include <kurl.h>
#include <kdebug.h>
#include <QMimeData>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <sys/types.h>
#include <dirent.h>

class KDirModelNode;
class KDirModelDirNode;

// We create our own tree behind the scenes to have fast lookup from an item to its parent,
// and also to get the children of an item fast.
class KDirModelNode
{
public:
    KDirModelNode( KDirModelDirNode* parent, const KFileItem& item ) :
        m_item(item),
        m_parent(parent),
        m_preview()
    {
    }
    //KUrl url() const { return m_item->url(); }

    // m_item is KFileItem() for the root item
    const KFileItem& item() const { return m_item; }
    void setItem(const KFileItem& item) { m_item = item; }
    KDirModelDirNode* parent() const { return m_parent; }
    // linear search
    int rowNumber() const;
    QIcon preview() const { return m_preview; }
    void addPreview( const QPixmap& pix ) { m_preview.addPixmap(pix); }
    void setPreview( const QIcon& icn ) { m_preview = icn; }

private:
    KFileItem m_item;
    KDirModelDirNode* const m_parent;
    QIcon m_preview;
};

// Specialization for directory nodes
class KDirModelDirNode : public KDirModelNode
{
public:
    KDirModelDirNode( KDirModelDirNode* parent, const KFileItem& item)
        : KDirModelNode( parent, item),
          m_childNodes(),
          m_childCount(KDirModel::ChildCountUnknown),
          m_populated(false)
    {}
    ~KDirModelDirNode() {
        qDeleteAll(m_childNodes);
    }
    QList<KDirModelNode *> m_childNodes;

    // If we listed the directory, the child count is known. Otherwise it can be set via setChildCount.
    int childCount() const { return m_childNodes.isEmpty() ? m_childCount : m_childNodes.count(); }
    void setChildCount(int count) { m_childCount = count; }
    bool isPopulated() const { return m_populated; }
    void setPopulated( bool populated ) { m_populated = populated; }

private:
    int m_childCount:31;
    bool m_populated:1;
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
          m_rootNode(new KDirModelDirNode(0, KFileItem())),
          m_dropsAllowed(KDirModel::NoDrops)
    {
    }
    ~KDirModelPrivate() {
        delete m_rootNode;
    }

    void _k_slotNewItems(const KFileItemList&);
    void _k_slotDeleteItem(const KFileItem&);
    void _k_slotRefreshItems(const QList<QPair<KFileItem, KFileItem> >&);
    void _k_slotClear();

    void clear() {
        delete m_rootNode;
        m_rootNode = new KDirModelDirNode(0, KFileItem());
    }
    // Find the row number and node for a given url.
    // This has to drill down from the root node.
    // Returns (0,0) if there is no node for this url.
    // If returnLastParent is set, then return the last known parent if there is no node for this url
    // (special case for expandToUrl)
    QPair<int /*row*/, KDirModelNode*> nodeForUrl(const KUrl& url, bool returnLastParent = false) const;
    KDirModelNode* nodeForIndex(const QModelIndex& index) const;
    QModelIndex indexForNode(KDirModelNode* node, int rowNumber = -1 /*unknown*/) const;
    bool isDir(KDirModelNode* node) const {
        return (node == m_rootNode) || node->item().isDir();
    }

    KDirModel* q;
    KDirLister* m_dirLister;
    KDirModelDirNode* m_rootNode;
    KDirModel::DropsAllowed m_dropsAllowed;
    // key = current known parent node (always a KDirModelDirNode but KDirModelNode is more convenient),
    // value = final url[s] being fetched
    QMap<KDirModelNode*, KUrl::List> m_urlsBeingFetched;
};

// If we want to support arbitrary trees like "home:/ as a child of system:/" then,
// we need to get the parent KFileItem in _k_slotNewItems, and then we can use a QHash<KFileItem,KDirModelNode*> cache.
// (well there isn't a parent kfileitem, rather a parent url... hmm, back to square one with hashes-of-urls..)
// For now we'll assume "child url = parent url + filename"
QPair<int /*row*/, KDirModelNode*> KDirModelPrivate::nodeForUrl(const KUrl& _url, bool returnLastParent) const // O(n*m)
{
    KUrl url(_url);
    url.adjustPath(KUrl::RemoveTrailingSlash);

    //kDebug(7008) << url;
    KUrl nodeUrl = m_dirLister->url();
    // For a URL without a path, like "applications:" or "settings://",
    // we want to resolve here "no path" to "/ assumed".
    // We don't do it before (e.g. in KDirLister) because we want to
    // give the ioslave a chance for a redirect (e.g. kio_ftp redirects "no path"
    // to the user's home dir)
    if (nodeUrl.path().isEmpty())
        nodeUrl.setPath("/");

    if (url == nodeUrl)
        return qMakePair(0, static_cast<KDirModelNode *>(m_rootNode));

    const QString pathStr = url.path();
    KDirModelDirNode* dirNode = m_rootNode;

    if ( !pathStr.startsWith(nodeUrl.path()) ) {
        return qMakePair(0, static_cast<KDirModelNode*>(0));
    }

    for (;;) {
        Q_ASSERT( pathStr.startsWith(nodeUrl.path()) );
        bool foundChild = false;
        QList<KDirModelNode *>::const_iterator it = dirNode->m_childNodes.begin();
        const QList<KDirModelNode *>::const_iterator end = dirNode->m_childNodes.end();
        int row = 0;
        for ( ; it != end ; ++it, ++row ) {
            const KUrl u = (*it)->item().url();
            if ( u == url ) {
                //kDebug(7008) << "Found! " << u;
                return qMakePair(row, *it);
            }
            // This used to be urlStr.startsWith(u.url()+'/'), but KUrl::url() is a slow operation.
            if ( (url.protocol() == u.protocol()) && (pathStr.startsWith(u.path()+'/')) ) {
                //kDebug(7008) << "going into " << node->item().url();
                Q_ASSERT( isDir(*it) );
                dirNode = static_cast<KDirModelDirNode *>( *it );
                foundChild = true;
                break;
            }
        }
        if (!foundChild) {
            //kDebug(7008) << "child equal or starting with " << url << " not found";
            if (returnLastParent)
                return qMakePair(-1 /*not implemented*/, static_cast<KDirModelNode*>(dirNode));
            else
                return qMakePair(0, static_cast<KDirModelNode*>(0));
        }
        nodeUrl = dirNode->item().url();
        //kDebug(7008) << " " << nodeUrl;
    }
    // NOTREACHED
    //return qMakePair(0, static_cast<KDirModelNode*>(0));
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
        str = "[index for " + node->item().url().pathOrUrl();
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
    delete d;
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
             this, SLOT(_k_slotNewItems(KFileItemList)) );
    connect( d->m_dirLister, SIGNAL(deleteItem(KFileItem)),
             this, SLOT(_k_slotDeleteItem(KFileItem)) );
    connect( d->m_dirLister, SIGNAL(refreshItems(QList<QPair<KFileItem, KFileItem> >)),
             this, SLOT(_k_slotRefreshItems(QList<QPair<KFileItem, KFileItem> >)) );
    connect( d->m_dirLister, SIGNAL(clear()),
             this, SLOT(_k_slotClear()) );
}

KDirLister* KDirModel::dirLister() const
{
    return d->m_dirLister;
}

void KDirModelPrivate::_k_slotNewItems(const KFileItemList& items)
{
    // Find parent item - it's the same for all the items
    // TODO (if Michael Brade agrees): add parent url to the newItems signal
    // This way we can finally support properly trees where the urls are using different protocols.
    KUrl dir( items.first().url().upUrl() );
    dir.adjustPath(KUrl::RemoveTrailingSlash);

    //kDebug(7008) << "dir=" << dir;

    const QPair<int, KDirModelNode*> result = nodeForUrl(dir); // O(n*m)
    Q_ASSERT(result.second); // Are you calling KDirLister::openUrl(url,true,false)? Please use expandToUrl() instead.
    Q_ASSERT(isDir(result.second));
    KDirModelDirNode* dirNode = static_cast<KDirModelDirNode *>(result.second);

    const QModelIndex index = indexForNode(dirNode, result.first); // O(1)
    const int newItemsCount = items.count();
    const int newRowCount = dirNode->m_childNodes.count() + newItemsCount;
#ifndef NDEBUG // debugIndex only defined in debug mode
    kDebug(7008) << items.count() << "in" << dir
             << "index=" << debugIndex(index) << "newRowCount=" << newRowCount;
#endif
    q->beginInsertRows( index, newRowCount - newItemsCount, newRowCount - 1 ); // parent, first, last

    const KUrl::List urlsBeingFetched = m_urlsBeingFetched.value(dirNode);
    //kDebug(7008) << "urlsBeingFetched for dir" << dirNode << dir << ":" << urlsBeingFetched;

    QList<QModelIndex> emitExpandFor;

    KFileItemList::const_iterator it = items.begin();
    KFileItemList::const_iterator end = items.end();
    for ( ; it != end ; ++it ) {
        const bool isDir = it->isDir();
        KDirModelNode* node = isDir
                              ? new KDirModelDirNode( dirNode, *it )
                              : new KDirModelNode( dirNode, *it );
        dirNode->m_childNodes.append(node);

        if (isDir && !urlsBeingFetched.isEmpty()) {
            const KUrl dirUrl = it->url();
            foreach(const KUrl& urlFetched, urlsBeingFetched) {
                if (dirUrl.isParentOf(urlFetched)) {
                    //kDebug(7008) << "Listing found" << dirUrl << "which is a parent of fetched url" << urlFetched;
                    const QModelIndex parentIndex = indexForNode(node, dirNode->m_childNodes.count()-1);
                    Q_ASSERT(parentIndex.isValid());
                    emitExpandFor.append(parentIndex);
                    if (dirUrl != urlFetched) {
                        q->fetchMore(parentIndex);
                        m_urlsBeingFetched[node].append(urlFetched);
                    }
                }
            }
        }
    }

    m_urlsBeingFetched.remove(dirNode);

    q->endInsertRows();

    // Emit expand signal after rowsInserted signal has been emitted,
    // so that any proxy model will have updated its mapping already
    Q_FOREACH(const QModelIndex& idx, emitExpandFor) {
        emit q->expand(idx);
    }
}

void KDirModelPrivate::_k_slotDeleteItem(const KFileItem& item)
{
    //KUrl dir( item->url().upUrl() );
    //dir.adjustPath(KUrl::RemoveTrailingSlash);

    Q_ASSERT(!item.isNull());
    const QPair<int, KDirModelNode*> result = nodeForUrl(item.url()); // O(n*m)
    const int rowNumber = result.first;
    KDirModelNode* node = result.second;
    if (!node)
        return;

    KDirModelDirNode* dirNode = node->parent();
    if (!dirNode)
        return;

    QModelIndex parentIndex = indexForNode(dirNode); // O(n)
    q->beginRemoveRows( parentIndex, rowNumber, rowNumber );
    dirNode->m_childNodes.removeAt(rowNumber);
    q->endRemoveRows();
}

void KDirModelPrivate::_k_slotRefreshItems(const QList<QPair<KFileItem, KFileItem> >& items)
{
    QModelIndex topLeft, bottomRight;

    // Solution 1: we could emit dataChanged for one row (if items.size()==1) or all rows
    // Solution 2: more fine-grained, actually figure out the beginning and end rows.
    for ( QList<QPair<KFileItem, KFileItem> >::const_iterator fit = items.begin(), fend = items.end() ; fit != fend ; ++fit ) {
        const QModelIndex index = q->indexForUrl( fit->first.url() ); // O(n*m); maybe we could look up to the parent only once
        nodeForIndex(index)->setItem(fit->second);
        if (!topLeft.isValid() || index.row() < topLeft.row()) {
            topLeft = index;
        }
        if (!bottomRight.isValid() || index.row() > bottomRight.row()) {
            bottomRight = index;
        }
    }
#ifndef NDEBUG // debugIndex only defined in debug mode
    kDebug(7008) << "slotRefreshItems: dataChanged(" << debugIndex(topLeft) << " - " << debugIndex(bottomRight);
#endif
    bottomRight = bottomRight.sibling(bottomRight.row(), q->columnCount(QModelIndex())-1);
    emit q->dataChanged(topLeft, bottomRight);
}

void KDirModelPrivate::_k_slotClear()
{
    const int numRows = m_rootNode->m_childNodes.count();
    q->beginRemoveRows( QModelIndex(), 0, numRows );
    q->endRemoveRows();

    //emit layoutAboutToBeChanged();
    clear();
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
        const KFileItem& item( node->item() );
        switch (role) {
        case Qt::DisplayRole:
            switch (index.column()) {
            case Name:
                return item.text();
            case Size:
                //
                //return KIO::convertSize(item->size());
                // Default to "file size in bytes" like in kde3's filedialog
                return KGlobal::locale()->formatNumber(item.size(), 0);
            case ModifiedTime: {
                KDateTime dt = item.time(KFileItem::ModificationTime);
                return KGlobal::locale()->formatDateTime(dt);
            }
            case Permissions:
                return item.permissionsString();
            case Owner:
                return item.user();
            case Group:
                return item.group();
            case Type:
                return item.mimeComment();
            }
            break;
        case Qt::EditRole:
            switch (index.column()) {
            case Name:
                return item.text();
            }
            break;
        case Qt::DecorationRole:
            if (index.column() == Name) {
                if (!node->preview().isNull()) {
                    //kDebug(7008) << item->url() << " preview found";
                    return node->preview();
                }
                Q_ASSERT(!item.isNull());
                //kDebug(7008) << item->url() << " overlays=" << item->overlays();
                return KIcon(item.iconName(), 0, item.overlays());
            }
            break;
        case Qt::TextAlignmentRole:
            if (index.column() == Size) {
                // use a right alignment for L2R and R2L languages
                const Qt::Alignment alignment = Qt::AlignRight | Qt::AlignVCenter;
                return int(alignment);
            }
            break;
        case FileItemRole:
            return QVariant::fromValue(item);
        case ChildCountRole:
            if (!item.isDir())
                return ChildCountUnknown;
            else {
                KDirModelDirNode* dirNode = static_cast<KDirModelDirNode *>(node);
                int count = dirNode->childCount();
                if (count == ChildCountUnknown && item.isReadable()) {
                    const QString path = item.localPath();
                    if (!path.isEmpty()) {
#if 0 // slow
                        QDir dir(path);
                        count = dir.entryList(QDir::AllEntries|QDir::NoDotAndDotDot|QDir::System).count();
#else
                        DIR* dir = ::opendir(QFile::encodeName(path));
                        if (dir) {
                            count = 0;
                            struct dirent *dirEntry = 0;
                            while ((dirEntry = ::readdir(dir))) {
                                if (dirEntry->d_name[0] == '.') {
                                    if (dirEntry->d_name[1] == '\0') // skip "."
                                        continue;
                                    if (dirEntry->d_name[1] == '.' && dirEntry->d_name[2] == '\0') // skip ".."
                                        continue;
                                }
                                ++count;
                            }
                            ::closedir(dir);
                        }
#endif
                        //kDebug(7008) << "child count for " << path << ":" << count;
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
    case Qt::EditRole:
        if (index.column() == Name && value.type() == QVariant::String) {
            Q_ASSERT(index.isValid());
            KDirModelNode* node = static_cast<KDirModelNode*>(index.internalPointer());
            const KFileItem& item = node->item();
            const QString newName = value.toString();
            if (newName.isEmpty() || newName == item.text())
                return true;
            KUrl newurl(item.url());
            newurl.setPath(newurl.directory(KUrl::AppendTrailingSlash) + newName);
            KIO::Job * job = KIO::moveAs(item.url(), newurl, newurl.isLocalFile() ? KIO::HideProgressInfo : KIO::DefaultFlags);
            job->ui()->setAutoErrorHandlingEnabled(true);
            // TODO undo handling
            return true;
        }
        break;
    case Qt::DecorationRole:
        if (index.column() == Name) {
            Q_ASSERT(index.isValid());
            // Set new pixmap - e.g. preview
            KDirModelNode* node = static_cast<KDirModelNode*>(index.internalPointer());
            //kDebug(7008) << "setting icon for " << node->item()->url();
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
    //kDebug(7008) << "rowCount for " << parentUrl << ": " << count;;
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
    foreach ( const QModelIndex &index, indexes ) {
        urls << d->nodeForIndex( index )->item().url();
    }
    QMimeData *data = new QMimeData();
    urls.populateMimeData( data );
    return data;
}

// Public API; not much point in calling it internally
KFileItem KDirModel::itemForIndex( const QModelIndex& index ) const
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
    return indexForUrl(item->url()); // O(n*m)
}

QModelIndex KDirModel::indexForItem( const KFileItem& item ) const
{
    // Note that we can only use the URL here, not the pointer.
    // KFileItems can be copied.
    return indexForUrl(item.url()); // O(n*m)
}

// url -> index. O(n*m)
QModelIndex KDirModel::indexForUrl(const KUrl& url) const
{
    const QPair<int, KDirModelNode*> result = d->nodeForUrl(url); // O(n*m) (m is the depth from the root)
    if (!result.second) {
        kDebug(7007) << url << "not found";
        return QModelIndex();
    }
    return d->indexForNode(result.second, result.first); // O(1)
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
            return i18nc("@title:column","Name");
        case Size:
            return i18nc("@title:column","Size");
        case ModifiedTime:
            return i18nc("@title:column","Date");
        case Permissions:
            return i18nc("@title:column","Permissions");
        case Owner:
            return i18nc("@title:column","Owner");
        case Group:
            return i18nc("@title:column","Group");
        case Type:
            return i18nc("@title:column","Type");
        }
    }
    return QVariant();
}

bool KDirModel::hasChildren( const QModelIndex & parent ) const
{
    if (!parent.isValid())
        return true;

    const KFileItem& parentItem = static_cast<KDirModelNode*>(parent.internalPointer())->item();
    Q_ASSERT(!parentItem.isNull());
    return parentItem.isDir();
}

Qt::ItemFlags KDirModel::flags( const QModelIndex & index ) const
{
    Qt::ItemFlags f = Qt::ItemIsEnabled;
    if (index.column() == Name) {
        f |= Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsDragEnabled;
    }

    // Allow dropping onto this item?
    if (d->m_dropsAllowed != NoDrops) {
        if(!index.isValid()) {
            if (d->m_dropsAllowed & DropOnDirectory) {
                f |= Qt::ItemIsDropEnabled;
            }
        } else {
            KFileItem item = itemForIndex(index);
            if (item.isNull()) {
                kWarning(7007) << "Invalid item returned for index";
            } else if (item.isDir()) {
                if (d->m_dropsAllowed & DropOnDirectory) {
                    f |= Qt::ItemIsDropEnabled;
                }
            } else { // regular file item
                if (d->m_dropsAllowed & DropOnAnyFile)
                    f |= Qt::ItemIsDropEnabled;
                else if (d->m_dropsAllowed & DropOnLocalExecutable) {
                    if (item.isLocalFile()) {
                        // Desktop file?
                        if (item.mimeTypePtr()->is("application/x-desktop"))
                            f |= Qt::ItemIsDropEnabled;
                        // Executable, shell script ... ?
                        else if ( QFileInfo( item.localPath() ).isExecutable() )
                            f |= Qt::ItemIsDropEnabled;
                    }
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

    // We now have a bool KDirModelNode::m_populated,
    // to avoid calling fetchMore more than once on empty dirs.
    // But this wastes memory, and how often does someone open and re-open an empty dir in a treeview?
    // Maybe we can ask KDirLister "have you listed <url> already"? (to discuss with M. Brade)

    KDirModelNode* node = static_cast<KDirModelNode*>(parent.internalPointer());
    const KFileItem& item = node->item();
    return item.isDir() && !static_cast<KDirModelDirNode *>(node)->isPopulated()
        && static_cast<KDirModelDirNode *>(node)->m_childNodes.isEmpty();
}

void KDirModel::fetchMore( const QModelIndex & parent )
{
    if (!parent.isValid())
        return;

    KDirModelNode* parentNode = static_cast<KDirModelNode*>(parent.internalPointer());

    KFileItem parentItem = parentNode->item();
    Q_ASSERT(!parentItem.isNull());
    Q_ASSERT(parentItem.isDir());
    KDirModelDirNode* dirNode = static_cast<KDirModelDirNode *>(parentNode);
    if( dirNode->isPopulated() )
        return;
    dirNode->setPopulated( true );

    //const KUrl url = parentItem.url();
    //kDebug(7008) << "listing" << url;
    d->m_dirLister->openUrl(parentItem.url(), KDirLister::Keep | KDirLister::Reload);
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

void KDirModel::expandToUrl(const KUrl& url)
{
    const QPair<int, KDirModelNode*> result = d->nodeForUrl(url, true /*return last parent*/); // O(n*m)

    if (!result.second) // doesn't seem related to our base url?
        return;
    if (!(result.second->item().isNull()) && result.second->item().url() == url) {
        // We have it already, nothing to do
        kDebug(7008) << "have it already item=" <<url /*result.second->item()*/;
        return;
    }

    d->m_urlsBeingFetched[result.second].append(url);

    if (result.second == d->m_rootNode) {
        kDebug(7008) << "Remembering to emit expand after listing the root url";
        // the root is fetched by default, so it must be currently being fetched
        return;
    }

    kDebug(7008) << "Remembering to emit expand after listing" << result.second->item().url();

    // start a new fetch to look for the next level down the URL
    const QModelIndex parentIndex = d->indexForNode(result.second, result.first);
    Q_ASSERT(parentIndex.isValid());
    fetchMore(parentIndex);
}

bool KDirModel::insertRows(int , int, const QModelIndex&)
{
    return false;
}

bool KDirModel::insertColumns(int, int, const QModelIndex&)
{
    return false;
}

bool KDirModel::removeRows(int, int, const QModelIndex&)
{
    return false;
}

bool KDirModel::removeColumns(int, int, const QModelIndex&)
{
    return false;
}

#include "kdirmodel.moc"
