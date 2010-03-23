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
#include <kio/fileundomanager.h>
#include <kio/jobuidelegate.h>
#include <kio/joburlcache_p.h>
#include <kurl.h>
#include <kdebug.h>
#include <QMimeData>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <sys/types.h>
#include <dirent.h>

#ifdef Q_WS_WIN
#include <windows.h>
#endif

class KDirModelNode;
class KDirModelDirNode;

static KUrl cleanupUrl(const KUrl& url) {
    KUrl u = url;
    u.cleanPath(); // remove double slashes in the path, simplify "foo/." to "foo/", etc.
    u.adjustPath(KUrl::RemoveTrailingSlash); // KDirLister does this too, so we remove the slash before comparing with the root node url.
    u.setQuery(QString());
    u.setRef(QString());
    return u;
}

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
    // m_item is KFileItem() for the root item
    const KFileItem& item() const { return m_item; }
    void setItem(const KFileItem& item) { m_item = item; }
    KDirModelDirNode* parent() const { return m_parent; }
    // linear search
    int rowNumber() const; // O(n)
    QIcon preview() const { return m_preview; }
    void setPreview( const QPixmap& pix ) {  m_preview = QIcon(); m_preview.addPixmap(pix); }
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
    QList<KDirModelNode *> m_childNodes; // owns the nodes

    // If we listed the directory, the child count is known. Otherwise it can be set via setChildCount.
    int childCount() const { return m_childNodes.isEmpty() ? m_childCount : m_childNodes.count(); }
    void setChildCount(int count) { m_childCount = count; }
    bool isPopulated() const { return m_populated; }
    void setPopulated( bool populated ) { m_populated = populated; }

    // For removing all child urls from the global hash.
    void collectAllChildUrls(KUrl::List &urls) const {
        Q_FOREACH(KDirModelNode* node, m_childNodes) {
            const KFileItem& item = node->item();
            urls.append(cleanupUrl(item.url()));
            if (item.isDir())
                static_cast<KDirModelDirNode*>(node)->collectAllChildUrls(urls);
        }
    }

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
          m_dropsAllowed(KDirModel::NoDrops), m_jobTransfersVisible(false)
    {
    }
    ~KDirModelPrivate() {
        delete m_rootNode;
    }

    void _k_slotNewItems(const KUrl& directoryUrl, const KFileItemList&);
    void _k_slotDeleteItems(const KFileItemList&);
    void _k_slotRefreshItems(const QList<QPair<KFileItem, KFileItem> >&);
    void _k_slotClear();
    void _k_slotRedirection(const KUrl& oldUrl, const KUrl& newUrl);
    void _k_slotJobUrlsChanged(const QStringList& urlList);

    void clear() {
        delete m_rootNode;
        m_rootNode = new KDirModelDirNode(0, KFileItem());
    }
    // Emit expand for each parent and then return the
    // last known parent if there is no node for this url
    KDirModelNode* expandAllParentsUntil(const KUrl& url) const;

    // Return the node for a given url, using the hash.
    KDirModelNode* nodeForUrl(const KUrl& url) const;
    KDirModelNode* nodeForIndex(const QModelIndex& index) const;
    QModelIndex indexForNode(KDirModelNode* node, int rowNumber = -1 /*unknown*/) const;
    bool isDir(KDirModelNode* node) const {
        return (node == m_rootNode) || node->item().isDir();
    }
    KUrl urlForNode(KDirModelNode* node) const {
        /**
         * Queries and fragments are removed from the URL, so that the URL of
         * child items really starts with the URL of the parent.
         *
         * For instance ksvn+http://url?rev=100 is the parent for ksvn+http://url/file?rev=100
         * so we have to remove the query in both to be able to compare the URLs
         */
        KUrl url(node == m_rootNode ? m_dirLister->url() : node->item().url());
        if (url.hasQuery() || url.hasRef()) { // avoid detach if not necessary.
            url.setQuery(QString());
            url.setRef(QString()); // kill ref (#171117)
        }
        return url;
    }
    void removeFromNodeHash(KDirModelNode* node, const KUrl& url);
#ifndef NDEBUG
    void dump();
#endif

    KDirModel* q;
    KDirLister* m_dirLister;
    KDirModelDirNode* m_rootNode;
    KDirModel::DropsAllowed m_dropsAllowed;
    bool m_jobTransfersVisible;
    // key = current known parent node (always a KDirModelDirNode but KDirModelNode is more convenient),
    // value = final url[s] being fetched
    QMap<KDirModelNode*, KUrl::List> m_urlsBeingFetched;
    QHash<KUrl, KDirModelNode *> m_nodeHash; // global node hash: url -> node
    QStringList m_allCurrentDestUrls; //list of all dest urls that have jobs on them (e.g. copy, download)
};

KDirModelNode* KDirModelPrivate::nodeForUrl(const KUrl& _url) const // O(1), well, O(length of url as a string)
{
    KUrl url = cleanupUrl(_url);
    if (url == urlForNode(m_rootNode))
        return m_rootNode;
    return m_nodeHash.value(url);
}

void KDirModelPrivate::removeFromNodeHash(KDirModelNode* node, const KUrl& url)
{
    if (node->item().isDir()) {
        KUrl::List urls;
        static_cast<KDirModelDirNode *>(node)->collectAllChildUrls(urls);
        Q_FOREACH(const KUrl& u, urls) {
            m_nodeHash.remove(u);
        }
    }
    m_nodeHash.remove(cleanupUrl(url));
}

KDirModelNode* KDirModelPrivate::expandAllParentsUntil(const KUrl& _url) const // O(depth)
{
    KUrl url = cleanupUrl(_url);

    //kDebug(7008) << url;
    KUrl nodeUrl = urlForNode(m_rootNode);
    if (url == nodeUrl)
        return m_rootNode;

    // Protocol mismatch? Don't even start comparing paths then. #171721
    if (url.protocol() != nodeUrl.protocol())
        return 0;

    const QString pathStr = url.path(); // no trailing slash
    KDirModelDirNode* dirNode = m_rootNode;

    if (!pathStr.startsWith(nodeUrl.path())) {
        return 0;
    }

    for (;;) {
        const QString nodePath = nodeUrl.path(KUrl::AddTrailingSlash);
        if(!pathStr.startsWith(nodePath)) {
            kError(7008) << "The kioslave for" << url.protocol() << "violates the hierarchy structure:"
                         << "I arrived at node" << nodePath << ", but" << pathStr << "does not start with that path.";
            return 0;
        }

        // E.g. pathStr is /a/b/c and nodePath is /a/. We want to find the node with url /a/b
        const int nextSlash = pathStr.indexOf('/', nodePath.length());
        const QString newPath = pathStr.left(nextSlash); // works even if nextSlash==-1
        nodeUrl.setPath(newPath);
        nodeUrl.adjustPath(KUrl::RemoveTrailingSlash); // #172508
        KDirModelNode* node = nodeForUrl(nodeUrl);
        if (!node) {
            //kDebug(7008) << "child equal or starting with" << url << "not found";
            // return last parent found:
            return dirNode;
        }

        emit q->expand(indexForNode(node));

        //kDebug(7008) << " nodeUrl=" << nodeUrl;
        if (nodeUrl == url) {
            //kDebug(7008) << "Found node" << node << "for" << url;
            return node;
        }
        //kDebug(7008) << "going into" << node->item().url();
        Q_ASSERT(isDir(node));
        dirNode = static_cast<KDirModelDirNode *>(node);
    }
    // NOTREACHED
    //return 0;
}

#ifndef NDEBUG
void KDirModelPrivate::dump()
{
    kDebug() << "Dumping contents of KDirModel" << q << "dirLister url:" << m_dirLister->url();
    QHashIterator<KUrl, KDirModelNode *> it(m_nodeHash);
    while (it.hasNext()) {
        it.next();
        kDebug() << it.key() << it.value();
    }
}
#endif

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
    connect( d->m_dirLister, SIGNAL(itemsAdded(KUrl,KFileItemList)),
             this, SLOT(_k_slotNewItems(KUrl,KFileItemList)) );
    connect( d->m_dirLister, SIGNAL(itemsDeleted(KFileItemList)),
             this, SLOT(_k_slotDeleteItems(KFileItemList)) );
    connect( d->m_dirLister, SIGNAL(refreshItems(QList<QPair<KFileItem, KFileItem> >)),
             this, SLOT(_k_slotRefreshItems(QList<QPair<KFileItem, KFileItem> >)) );
    connect( d->m_dirLister, SIGNAL(clear()),
             this, SLOT(_k_slotClear()) );
    connect(d->m_dirLister, SIGNAL(redirection(KUrl, KUrl)),
            this, SLOT(_k_slotRedirection(KUrl, KUrl)));
}

KDirLister* KDirModel::dirLister() const
{
    return d->m_dirLister;
}

void KDirModelPrivate::_k_slotNewItems(const KUrl& directoryUrl, const KFileItemList& items)
{
    //kDebug(7008) << "directoryUrl=" << directoryUrl;

    KDirModelNode* result = nodeForUrl(directoryUrl); // O(depth)
    // If the directory containing the items wasn't found, then we have a big problem.
    // Are you calling KDirLister::openUrl(url,true,false)? Please use expandToUrl() instead.
    if (!result) {
        kError(7008) << "Items emitted in directory" << directoryUrl
                     << "but that directory isn't in KDirModel!"
                     << "Root directory:" << urlForNode(m_rootNode);
        Q_FOREACH(const KFileItem& item, items) {
            kDebug() << "Item:" << item.url();
        }
#ifndef NDEBUG
        dump();
#endif
        Q_ASSERT(result);
    }
    Q_ASSERT(isDir(result));
    KDirModelDirNode* dirNode = static_cast<KDirModelDirNode *>(result);

    const QModelIndex index = indexForNode(dirNode); // O(n)
    const int newItemsCount = items.count();
    const int newRowCount = dirNode->m_childNodes.count() + newItemsCount;
#if 0
#ifndef NDEBUG // debugIndex only defined in debug mode
    kDebug(7008) << items.count() << "in" << directoryUrl
             << "index=" << debugIndex(index) << "newRowCount=" << newRowCount;
#endif
#endif

    q->beginInsertRows( index, newRowCount - newItemsCount, newRowCount - 1 ); // parent, first, last

    const KUrl::List urlsBeingFetched = m_urlsBeingFetched.value(dirNode);
    //kDebug(7008) << "urlsBeingFetched for dir" << dirNode << directoryUrl << ":" << urlsBeingFetched;

    QList<QModelIndex> emitExpandFor;

    KFileItemList::const_iterator it = items.begin();
    KFileItemList::const_iterator end = items.end();
    for ( ; it != end ; ++it ) {
        const bool isDir = it->isDir();
        KDirModelNode* node = isDir
                              ? new KDirModelDirNode( dirNode, *it )
                              : new KDirModelNode( dirNode, *it );
#ifndef NDEBUG
        // Test code for possible duplication of items in the childnodes list,
        // not sure if/how it ever happened.
        //if (dirNode->m_childNodes.count() &&
        //    dirNode->m_childNodes.last()->item().name() == (*it).name())
        //    kFatal() << "Already having" << (*it).name() << "in" << directoryUrl
        //             << "url=" << dirNode->m_childNodes.last()->item().url();
#endif
        dirNode->m_childNodes.append(node);
        const KUrl url = it->url();
        m_nodeHash.insert(cleanupUrl(url), node);
        //kDebug(7008) << url;

        if (!urlsBeingFetched.isEmpty()) {
            const KUrl dirUrl = url;
            foreach(const KUrl& urlFetched, urlsBeingFetched) {
                if (dirUrl.isParentOf(urlFetched)) {
                    kDebug(7008) << "Listing found" << dirUrl << "which is a parent of fetched url" << urlFetched;
                    const QModelIndex parentIndex = indexForNode(node, dirNode->m_childNodes.count()-1);
                    Q_ASSERT(parentIndex.isValid());
                    emitExpandFor.append(parentIndex);
                    if (isDir && dirUrl != urlFetched) {
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

void KDirModelPrivate::_k_slotDeleteItems(const KFileItemList& items)
{
    //kDebug(7008) << items.count();

    // I assume all items are from the same directory.
    // From KDirLister's code, this should be the case, except maybe emitChanges?
    const KFileItem item = items.first();
    Q_ASSERT(!item.isNull());
    KUrl url = item.url();
    KDirModelNode* node = nodeForUrl(url); // O(depth)
    if (!node) {
        kWarning(7008) << "No node found for item that was just removed:" << url;
        return;
    }

    KDirModelDirNode* dirNode = node->parent();
    if (!dirNode)
        return;

    QModelIndex parentIndex = indexForNode(dirNode); // O(n)

    // Short path for deleting a single item
    if (items.count() == 1) {
        const int r = node->rowNumber();
        q->beginRemoveRows(parentIndex, r, r);
        removeFromNodeHash(node, url);
        delete dirNode->m_childNodes.takeAt(r);
        q->endRemoveRows();
        return;
    }

    // We need to make lists of consecutive row numbers, for the beginRemoveRows call.
    // Let's use a bit array where each bit represents a given child node.
    const int childCount = dirNode->m_childNodes.count();
    QBitArray rowNumbers(childCount, false);
    Q_FOREACH(const KFileItem& item, items) {
        if (!node) { // don't lookup the first item twice
            url = item.url();
            node = nodeForUrl(url);
            if (!node) {
                kWarning(7008) << "No node found for item that was just removed:" << url;
                continue;
            }
        }
        rowNumbers.setBit(node->rowNumber(), 1); // O(n)
        removeFromNodeHash(node, url);
        node = 0;
    }

    int start = -1;
    int end = -1;
    bool lastVal = false;
    // Start from the end, otherwise all the row numbers are offset while we go
    for (int i = childCount - 1; i >= 0; --i) {
        const bool val = rowNumbers.testBit(i);
        if (!lastVal && val) {
            end = i;
            //kDebug(7008) << "end=" << end;
        }
        if ((lastVal && !val) || (i == 0 && val)) {
            start = val ? i : i + 1;
            //kDebug(7008) << "beginRemoveRows" << start << end;
            q->beginRemoveRows(parentIndex, start, end);
            for (int r = end; r >= start; --r) { // reverse because takeAt changes indexes ;)
                //kDebug(7008) << "Removing from m_childNodes at" << r;
                delete dirNode->m_childNodes.takeAt(r);
            }
            q->endRemoveRows();
        }
        lastVal = val;
    }
}

void KDirModelPrivate::_k_slotRefreshItems(const QList<QPair<KFileItem, KFileItem> >& items)
{
    QModelIndex topLeft, bottomRight;

    // Solution 1: we could emit dataChanged for one row (if items.size()==1) or all rows
    // Solution 2: more fine-grained, actually figure out the beginning and end rows.
    for ( QList<QPair<KFileItem, KFileItem> >::const_iterator fit = items.begin(), fend = items.end() ; fit != fend ; ++fit ) {
        Q_ASSERT(!fit->first.isNull());
        Q_ASSERT(!fit->second.isNull());
        const KUrl oldUrl = fit->first.url();
        const KUrl newUrl = fit->second.url();
        KDirModelNode* node = nodeForUrl(oldUrl); // O(n); maybe we could look up to the parent only once
        //kDebug(7008) << "in model for" << m_dirLister->url() << ":" << oldUrl << "->" << newUrl << "node=" << node;
        if (!node) // not found [can happen when renaming a dir, redirection was emitted already]
            continue;
        if (node != m_rootNode) { // we never set an item in the rootnode, we use m_dirLister->rootItem instead.
            bool hasNewNode = false;
            // A file became directory (well, it was overwritten)
            if (fit->first.isDir() != fit->second.isDir()) {
                //kDebug(7008) << "DIR/FILE STATUS CHANGE";
                const int r = node->rowNumber();
                removeFromNodeHash(node, oldUrl);
                KDirModelDirNode* dirNode = node->parent();
                delete dirNode->m_childNodes.takeAt(r); // i.e. "delete node"
                node = fit->second.isDir() ? new KDirModelDirNode(dirNode, fit->second)
                       : new KDirModelNode(dirNode, fit->second);
                dirNode->m_childNodes.insert(r, node); // same position!
                hasNewNode = true;
            } else {
                node->setItem(fit->second);
            }

            if (oldUrl != newUrl || hasNewNode) {
                // What if a renamed dir had children? -> kdirlister takes care of emitting for each item
                //kDebug(7008) << "Renaming" << oldUrl << "to" << newUrl << "in node hash";
                m_nodeHash.remove(cleanupUrl(oldUrl));
                m_nodeHash.insert(cleanupUrl(newUrl), node);
            }
            // Mimetype changed -> forget cached icon (e.g. from "cut", #164185 comment #13)
            if (fit->first.mimeTypePtr()->name() != fit->second.mimeTypePtr()->name()) {
                node->setPreview(QIcon());
            }

            const QModelIndex index = indexForNode(node);
            if (!topLeft.isValid() || index.row() < topLeft.row()) {
                topLeft = index;
            }
            if (!bottomRight.isValid() || index.row() > bottomRight.row()) {
                bottomRight = index;
            }
        }
    }
#ifndef NDEBUG // debugIndex only defined in debug mode
    kDebug(7008) << "dataChanged(" << debugIndex(topLeft) << " - " << debugIndex(bottomRight);
#endif
    bottomRight = bottomRight.sibling(bottomRight.row(), q->columnCount(QModelIndex())-1);
    emit q->dataChanged(topLeft, bottomRight);
}

// Called when a kioslave redirects (e.g. smb:/Workgroup -> smb://workgroup)
// and when renaming a directory.
void KDirModelPrivate::_k_slotRedirection(const KUrl& oldUrl, const KUrl& newUrl)
{
    KDirModelNode* node = nodeForUrl(oldUrl);
    if (!node)
        return;
    m_nodeHash.remove(cleanupUrl(oldUrl));
    m_nodeHash.insert(cleanupUrl(newUrl), node);

    // Ensure the node's URL is updated. In case of a listjob redirection
    // we won't get a refreshItem, and in case of renaming a directory
    // we'll get it too late (so the hash won't find the old url anymore).
    KFileItem item = node->item();
    if (!item.isNull()) { // null if root item, #180156
        item.setUrl(newUrl);
        node->setItem(item);
    }

    // The items inside the renamed directory have been handled before,
    // KDirLister took care of emitting refreshItem for each of them.
}

void KDirModelPrivate::_k_slotClear()
{
    const int numRows = m_rootNode->m_childNodes.count();
    if (numRows > 0) {
        q->beginRemoveRows( QModelIndex(), 0, numRows - 1 );
        q->endRemoveRows();
    }

    m_nodeHash.clear();
    //emit layoutAboutToBeChanged();
    clear();
    //emit layoutChanged();
}

void KDirModelPrivate::_k_slotJobUrlsChanged(const QStringList& urlList)
{
    m_allCurrentDestUrls = urlList;
}

void KDirModel::itemChanged( const QModelIndex& index )
{
    // This method is really a itemMimeTypeChanged(), it's mostly called by KMimeTypeResolver.
    // When the mimetype is determined, clear the old "preview" (could be
    // mimetype dependent like when cutting files, #164185)
    KDirModelNode* node = d->nodeForIndex(index);
    if (node)
        node->setPreview(QIcon());

#ifndef NDEBUG // debugIndex only defined in debug mode
    //kDebug(7008) << "dataChanged(" << debugIndex(index);
#endif
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
        case Qt::ToolTipRole:
            return item.text();
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
//                        slow
//                        QDir dir(path);
//                        count = dir.entryList(QDir::AllEntries|QDir::NoDotAndDotDot|QDir::System).count();
#ifdef Q_WS_WIN
                        QString s = path + QLatin1String( "\\*.*" );
                        s.replace('/', '\\');
                        count = 0;
                        WIN32_FIND_DATA findData;
                        HANDLE hFile = FindFirstFile( (LPWSTR)s.utf16(), &findData );
                        if( hFile != INVALID_HANDLE_VALUE ) {
                            do {
                                if (!( findData.cFileName[0] == '.' &&
                                       findData.cFileName[1] == '\0' ) &&
                                    !( findData.cFileName[0] == '.' &&
                                       findData.cFileName[1] == '.' &&
                                       findData.cFileName[2] == '\0' ) )
                                    ++count;
                            } while( FindNextFile( hFile, &findData ) != 0 );
                            FindClose( hFile );
                        }
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
        case HasJobRole:
            if (d->m_jobTransfersVisible && d->m_allCurrentDestUrls.isEmpty() == false) {
                KDirModelNode* node = d->nodeForIndex(index);
                const QString url = node->item().url().url();
                //return whether or not there are job dest urls visible in the view, so the delegate knows which ones to paint.
                return QVariant(d->m_allCurrentDestUrls.contains(url));
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
            // undo handling
            KIO::FileUndoManager::self()->recordJob( KIO::FileUndoManager::Rename, item.url(), newurl, job );
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
                node->setPreview(icon);
            } else if (value.type() == QVariant::Pixmap) {
                node->setPreview(qvariant_cast<QPixmap>(value));
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
    KDirModelNode* node = d->nodeForIndex(parent);
    if (!node || !d->isDir(node)) // #176555
        return 0;

    KDirModelDirNode* parentNode = static_cast<KDirModelDirNode *>(node);
    Q_ASSERT(parentNode);
    const int count = parentNode->m_childNodes.count();
#if 0
    QStringList filenames;
    for (int i = 0; i < count; ++i) {
        filenames << d->urlForNode(parentNode->m_childNodes.at(i)).fileName();
    }
    kDebug(7008) << "rowCount for " << d->urlForNode(parentNode) << ": " << count << filenames;
#endif
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

static bool lessThan(const KUrl &left, const KUrl &right)
{
    return left.url().compare(right.url()) < 0;
}

void KDirModel::requestSequenceIcon(const QModelIndex& index, int sequenceIndex)
{
    emit needSequenceIcon(index, sequenceIndex);
}

void KDirModel::setJobTransfersVisible(bool value)
{
    if(value) {
        d->m_jobTransfersVisible = true;
        connect(&JobUrlCache::instance(), SIGNAL(jobUrlsChanged(QStringList)), this, SLOT(_k_slotJobUrlsChanged(QStringList)), Qt::UniqueConnection);

        JobUrlCache::instance().requestJobUrlsChanged();
    } else {
        disconnect(this, SLOT(_k_slotJobUrlsChanged(QStringList)));
    }

}

bool KDirModel::jobTransfersVisible() const
{
    return d->m_jobTransfersVisible;
}

KUrl::List KDirModel::simplifiedUrlList(const KUrl::List &urls)
{
    if (!urls.count()) {
        return urls;
    }

    KUrl::List ret(urls);
    qSort(ret.begin(), ret.end(), lessThan);

    KUrl::List::iterator it = ret.begin();
    KUrl url = *it;
    ++it;
    while (it != ret.end()) {
        if (url.isParentOf(*it)) {
            it = ret.erase(it);
        } else {
            url = *it;
            ++it;
        }
    }

    return ret;
}

QStringList KDirModel::mimeTypes( ) const
{
    return KUrl::List::mimeDataTypes();
}

QMimeData * KDirModel::mimeData( const QModelIndexList & indexes ) const
{
    KUrl::List urls, mostLocalUrls;
    bool canUseMostLocalUrls = true;
    foreach (const QModelIndex &index, indexes) {
        const KFileItem& item = d->nodeForIndex(index)->item();
        urls << item.url();
        bool isLocal;
        mostLocalUrls << item.mostLocalUrl(isLocal);
        if (!isLocal)
            canUseMostLocalUrls = false;
    }
    QMimeData *data = new QMimeData();
    const bool different = canUseMostLocalUrls && (mostLocalUrls != urls);
    urls = simplifiedUrlList(urls);
    if (different) {
        mostLocalUrls = simplifiedUrlList(mostLocalUrls);
        urls.populateMimeData(mostLocalUrls, data);
    } else {
        urls.populateMimeData(data);
    }

    // for compatibility reasons (when dropping or pasting into kde3 applications)
    QString application_x_qiconlist;
    const int items = urls.count();
    for (int i = 0; i < items; i++) {
	const int offset = i*16;
	QString tmp("%1$@@$%2$@@$32$@@$32$@@$%3$@@$%4$@@$32$@@$16$@@$no data$@@$");
	application_x_qiconlist += tmp.arg(offset).arg(offset).arg(offset).arg(offset+40);
    }
    data->setData("application/x-qiconlist", application_x_qiconlist.toLatin1());

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
    return indexForUrl(item->url()); // O(n)
}

QModelIndex KDirModel::indexForItem( const KFileItem& item ) const
{
    // Note that we can only use the URL here, not the pointer.
    // KFileItems can be copied.
    return indexForUrl(item.url()); // O(n)
}

// url -> index. O(n)
QModelIndex KDirModel::indexForUrl(const KUrl& url) const
{
    KDirModelNode* node = d->nodeForUrl(url); // O(depth)
    if (!node) {
        kDebug(7007) << url << "not found";
        return QModelIndex();
    }
    return d->indexForNode(node); // O(n)
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
    if (orientation == Qt::Horizontal) {
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
                    if (!item.localPath().isEmpty()) {
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

    const KUrl parentUrl = parentItem.url();
    d->m_dirLister->openUrl(parentUrl, KDirLister::Keep);
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
    // emit expand for each parent and return last parent
    KDirModelNode* result = d->expandAllParentsUntil(url); // O(depth)
    //kDebug(7008) << url << result;

    if (!result) // doesn't seem related to our base url?
        return;
    if (!(result->item().isNull()) && result->item().url() == url) {
        // We have it already, nothing to do
        kDebug(7008) << "have it already item=" <<url /*result->item()*/;
        return;
    }

    d->m_urlsBeingFetched[result].append(url);

    if (result == d->m_rootNode) {
        kDebug(7008) << "Remembering to emit expand after listing the root url";
        // the root is fetched by default, so it must be currently being fetched
        return;
    }

    kDebug(7008) << "Remembering to emit expand after listing" << result->item().url();

    // start a new fetch to look for the next level down the URL
    const QModelIndex parentIndex = d->indexForNode(result); // O(n)
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
