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
    // m_item is KFileItem() for the root item
    const KFileItem& item() const { return m_item; }
    void setItem(const KFileItem& item) { m_item = item; }
    KDirModelDirNode* parent() const { return m_parent; }
    // linear search
    int rowNumber() const; // O(n)
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
    QList<KDirModelNode *> m_childNodes; // owns the nodes
    QHash<QString, KDirModelNode *> m_childNodesByName; // key = filename

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
    void _k_slotDeleteItems(const KFileItemList&);
    void _k_slotRefreshItems(const QList<QPair<KFileItem, KFileItem> >&);
    void _k_slotClear();

    void clear() {
        delete m_rootNode;
        m_rootNode = new KDirModelDirNode(0, KFileItem());
    }
    // Find the row number and node for a given url.
    // This has to drill down from the root node.
    // Returns (0,0) if there is no node for this url.
    // If expandAndReturnLastParent is set, then we emit expand for each parent and then return the
    // last known parent if there is no node for this url (special case for expandToUrl)
    KDirModelNode* nodeForUrl(const KUrl& url, bool expandAndReturnLastParent = false) const;
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
KDirModelNode* KDirModelPrivate::nodeForUrl(const KUrl& _url, bool expandAndReturnLastParent) const // O(depth)
{
    KUrl url(_url);
    url.adjustPath(KUrl::RemoveTrailingSlash); // KDirLister does this too, so we remove the slash before comparing with the root node url.
    url.cleanPath(); // remove double slashes in the path
    url.setQuery(QString());
    url.setRef(QString());

    //kDebug(7008) << url;
    KUrl nodeUrl = urlForNode(m_rootNode);
    // For a URL without a path, like "applications:" or "settings://",
    // we want to resolve here "no path" to "/ assumed".
    // We don't do it before (e.g. in KDirLister) because we want to
    // give the ioslave a chance for a redirect (e.g. kio_ftp redirects "no path"
    // to the user's home dir)
    if (nodeUrl.path().isEmpty())
        nodeUrl.setPath("/");

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

        // E.g. pathStr is /a/b/c and nodePath is /a. We want to find the child "b" in dirNode.
        const QString relativePath = pathStr.mid(nodePath.length());
        Q_ASSERT(!relativePath.startsWith('/')); // check if multiple slashes have been removed
        const int nextSlash = relativePath.indexOf('/');
        const QString fileName = relativePath.left(nextSlash); // works even if nextSlash==-1
        KDirModelNode* node = dirNode->m_childNodesByName.value(fileName);
        if (!node) {
            //kDebug(7008) << "child equal or starting with" << url << "not found";
            if (expandAndReturnLastParent)
                return dirNode;
            else
                return 0;
        }

        if (expandAndReturnLastParent)
            emit q->expand(indexForNode(node));

        nodeUrl = urlForNode(node);
        nodeUrl.adjustPath(KUrl::RemoveTrailingSlash); // #172508
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
    connect( d->m_dirLister, SIGNAL(itemsDeleted(KFileItemList)),
             this, SLOT(_k_slotDeleteItems(KFileItemList)) );
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
    // TODO: add parent url to the newItems signal
    //
    // This way we can finally support properly trees where the urls are using different protocols.
    // Well, it's not that simple - nodeForUrl still needs to know where to drill down...

    KUrl firstItemUrl = items.first().url();
    firstItemUrl.setQuery(QString());
    firstItemUrl.setRef(QString());
    KUrl dir(firstItemUrl);
    dir.setPath(dir.directory());

    //kDebug(7008) << "dir=" << dir;

    KDirModelNode* result = nodeForUrl(dir); // O(depth)
    // If the directory containing the items wasn't found, then we have a big problem.
    // Are you calling KDirLister::openUrl(url,true,false)? Please use expandToUrl() instead.
    if (!result) {
        kError(7008) << "First item has URL" << firstItemUrl
                     << "-> parent directory would be" << dir
                     << "but that directory isn't in KDirModel!"
                     << "Root directory:" << urlForNode(m_rootNode);
        Q_ASSERT(result);
    }
    Q_ASSERT(isDir(result));
    KDirModelDirNode* dirNode = static_cast<KDirModelDirNode *>(result);

    const QModelIndex index = indexForNode(dirNode); // O(n)
    const int newItemsCount = items.count();
    const int newRowCount = dirNode->m_childNodes.count() + newItemsCount;
#if 0
#ifndef NDEBUG // debugIndex only defined in debug mode
    kDebug(7008) << items.count() << "in" << dir
             << "index=" << debugIndex(index) << "newRowCount=" << newRowCount;
#endif
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
        const KUrl url = it->url();
        dirNode->m_childNodesByName.insert(url.fileName(), node);
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
        delete dirNode->m_childNodes.takeAt(r);
        q->endRemoveRows();
        Q_ASSERT(dirNode->m_childNodesByName.contains(url.fileName()));
        dirNode->m_childNodesByName.remove(url.fileName());
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
            }                                                                            
            Q_ASSERT(node);
        }
        rowNumbers.setBit(node->rowNumber(), 1); // O(n)
        Q_ASSERT(dirNode->m_childNodesByName.contains(url.fileName()));
        dirNode->m_childNodesByName.remove(url.fileName());
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
        const QModelIndex index = q->indexForUrl(oldUrl); // O(n); maybe we could look up to the parent only once
        KDirModelNode* node = nodeForIndex(index);
        if (node != m_rootNode) { // we never set an item in the rootnode, we use m_dirLister->rootItem instead.
            node->setItem(fit->second);

            if (oldUrl.fileName() != newUrl.fileName()) {
                KDirModelDirNode* parentNode = node->parent();
                Q_ASSERT(parentNode);
                parentNode->m_childNodesByName.remove(oldUrl.fileName());
                parentNode->m_childNodesByName.insert(newUrl.fileName(), node);
            }
            if (!topLeft.isValid() || index.row() < topLeft.row()) {
                topLeft = index;
            }
            if (!bottomRight.isValid() || index.row() > bottomRight.row()) {
                bottomRight = index;
            }
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
    KDirModelNode* result = d->nodeForUrl(url, true /*emit expand for each parent and return last parent*/); // O(depth)
    kDebug(7008) << url << result;

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
