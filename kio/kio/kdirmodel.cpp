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
    KDirModelNode( KDirModelNode* parent = 0 ) :
        m_item(0),
        m_childNodes(),
        m_parent(parent)
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
    int rowNumber() const { if (!m_parent) return 0;
        return m_parent->m_childNodes.indexOf(const_cast<KDirModelNode*>(this)); }

    KFileItem* m_item;

    // TODO maybe move this field to a DirNode subclass to save 4 bytes per file?
    QList<KDirModelNode *> m_childNodes;
private:
    KDirModelNode* const m_parent;
};

class KDirModelPrivate
{
public:
    KDirModelPrivate( KDirModel* model )
        : q(model), m_dirLister(0), m_rootNode(new KDirModelNode) {
    }
    ~KDirModelPrivate() {
        delete m_rootNode;
    }
    void clear() {
        delete m_rootNode;
        m_rootNode = new KDirModelNode;
    }
    //KDirModelNode* nodeForFileItem(KFileItem* item) const;
    KDirModelNode* nodeForUrl(const KUrl& _url) const;
    KDirModelNode* nodeForIndex(const QModelIndex& index) const;
    QModelIndex indexForNode(KDirModelNode* node) const;

    KDirModel* q;
    KDirLister* m_dirLister;
    KDirModelNode* m_rootNode;
};

// If we want to support arbitrary trees like "home:/ as a child of system:/" then,
// we need to get the parent KFileItem in slotNewItems, and then we can use a QHash<KFileItem*,KDirModelNode*> cache.
// For now we'll assume "child url = parent url + filename"

KDirModelNode* KDirModelPrivate::nodeForUrl(const KUrl& _url) const
{
    KUrl url(_url);
    url.adjustPath(KUrl::RemoveTrailingSlash);
    //kDebug() << k_funcinfo << url << endl;
    if (url == m_dirLister->url())
        return m_rootNode;

    const QString urlStr = url.url();
    KDirModelNode* node = m_rootNode;
    KUrl nodeUrl = m_dirLister->url();
    while ( nodeUrl != url ) {
        Q_ASSERT( urlStr.startsWith(nodeUrl.url()) );
        bool foundChild = false;
        QList<KDirModelNode *>::const_iterator it = node->m_childNodes.begin();
        const QList<KDirModelNode *>::const_iterator end = node->m_childNodes.end();
        for ( ; it != end ; ++it ) {
            const KUrl u = (*it)->item()->url();
            if ( u == url ) {
                //kDebug() << "Found! " << u << endl;
                return *it;
            }
            if ( urlStr.startsWith(u.url()) ) {
                node = *it;
                foundChild = true;
                Q_ASSERT( node->item()->isDir() );
                break;
            }
        }
        if (!foundChild) {
            //kDebug() << "child equal or starting with " << url << " not found" << endl;
            return false;
        }
        nodeUrl = node->item()->url();
        //kDebug() << " " << nodeUrl << endl;
    }
    return 0;
}

QModelIndex KDirModelPrivate::indexForNode(KDirModelNode* node) const
{
    if (node == m_rootNode)
        return QModelIndex();

    KDirModelNode* const parentNode = node->parent();
    const int row = parentNode->m_childNodes.indexOf(node);
    return q->createIndex(row, 0, node);
}

KDirModelNode* KDirModelPrivate::nodeForIndex(const QModelIndex& index) const
{
    return index.isValid()
        ? static_cast<KDirModelNode*>(index.internalPointer())
        : m_rootNode;
}


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
 * Invalid parent index means root of the tree, m_m_rootNode
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

    KDirModelNode* dirNode = d->nodeForUrl(dir);

    Q_ASSERT(dirNode);
    KFileItemList::const_iterator it = items.begin();
    const KFileItemList::const_iterator end = items.end();
    for ( ; it != end ; ++it ) {
        KDirModelNode* node = new KDirModelNode( dirNode );
        node->m_item = *it;
        dirNode->m_childNodes.append( node );
    }

    const QModelIndex index = d->indexForNode(dirNode);
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

    KDirModelNode* node = d->nodeForUrl(item->url());
    Q_ASSERT(node);
    if (!node)
        return;

    const QModelIndex parentIndex = d->indexForNode(node->parent());
    const int rowNumber = node->rowNumber();
    //kDebug() << k_funcinfo << debugIndex(parentIndex) << " " << rowNumber << endl;

    KDirModelNode* dirNode = d->nodeForIndex(parentIndex);
    Q_ASSERT(dirNode);
    dirNode->m_childNodes.removeAt(rowNumber);

    beginRemoveRows( parentIndex, rowNumber, rowNumber );
    endRemoveRows();
}

void KDirModel::slotRefreshItems( const KFileItemList& items )
{
    QModelIndex topLeft, bottomRight;

    // Solution 1: we could emit dataChanged for one row (if items.size()==1) or all rows
    // Solution 2: more fine-grained, actually figure out the beginning and end rows.
    for ( KFileItemList::const_iterator fit = items.begin(), fend = items.end() ; fit != fend ; ++fit ) {
        const QModelIndex index = d->indexForNode(d->nodeForUrl( (*fit)->url() ));
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
    d->clear();
    emit layoutChanged();
}

int KDirModel::columnCount( const QModelIndex & ) const
{
    return ColumnCount;
}

QVariant KDirModel::data( const QModelIndex & index, int role ) const
{
    if (index.isValid()) {
        KFileItem* item = static_cast<KDirModelNode*>(index.internalPointer())->item();
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
    return QAbstractItemModel::sort(column, order);
}

bool KDirModel::setData( const QModelIndex & index, const QVariant & value, int role )
{
    return QAbstractItemModel::setData(index, value, role);
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
