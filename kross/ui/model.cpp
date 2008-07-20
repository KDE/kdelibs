/***************************************************************************
 * model.cpp
 * This file is part of the KDE project
 * copyright (C) 2006-2007 by Sebastian Sauer (mail@dipe.org)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 * You should have received a copy of the GNU Library General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 ***************************************************************************/

#include "model.h"

#include <kross/core/action.h>
#include <kross/core/actioncollection.h>
#include <kross/core/manager.h>

#include <QtCore/QEvent>
#include <QtCore/QMimeData>

using namespace Kross;

/******************************************************************************
 * ActionCollectionModel
 */

namespace Kross {

    /// \internal item representation.
    class ActionCollectionModelItem : public QObject
    {
        public:
            enum Type { ActionType, CollectionType };
            const Type type;
            union {
                Action* action;
                ActionCollection* collection;
            };
            QVector<ActionCollectionModelItem*> children;
            const QModelIndex parent;

            QString name() const { return type == ActionType ? action->name() : collection->name(); }

            explicit ActionCollectionModelItem(Action* a, const QModelIndex& p = QModelIndex(), ActionCollectionModelItem* parentitem = 0)
                : QObject(parentitem), type(ActionType), action(a), parent(p)
            {
            }

            explicit ActionCollectionModelItem(ActionCollection* c, const QModelIndex& p = QModelIndex(), ActionCollectionModelItem* parentitem = 0)
                : QObject(parentitem), type(CollectionType), collection(c), parent(p)
            {
            }
    };

    /// \internal d-pointer class.
    class ActionCollectionModel::Private
    {
        public:
            ActionCollection* collection;
            ActionCollectionModelItem* item;
            Mode mode;

            template <class T>
            ActionCollectionModelItem* childItem( ActionCollectionModelItem* item,
                                                  const QModelIndex& parent,
                                                  int row,
                                                  int column,
                                                  T value );
    };

}

template <class T>
ActionCollectionModelItem* ActionCollectionModel::Private::childItem(
    ActionCollectionModelItem* item,
    const QModelIndex& parent,
    int row,
    int column,
    T value )
{
    Q_UNUSED(column);
    ActionCollectionModelItem* childItem = 0;
    if ( row < item->children.count() && item->children.at(row) != 0 ) {
        childItem = item->children.at(row);
    }
    else {
        childItem = new ActionCollectionModelItem(value,parent,item);
        item->children.resize(row+1);
        item->children[row] = childItem;
    }
    return childItem;
}

ActionCollectionModel::ActionCollectionModel(QObject* parent, ActionCollection* collection, Mode mode)
    : QAbstractItemModel(parent)
    , d( new Private() )
{
    d->collection = collection ? collection : Kross::Manager::self().actionCollection();
    //d->item = 0;
    d->item = new ActionCollectionModelItem( d->collection );
    d->mode = mode;
    //setSupportedDragActions(Qt::MoveAction);
    QObject::connect(d->collection, SIGNAL(updated()), this, SLOT(slotUpdated()));
}

ActionCollectionModel::~ActionCollectionModel()
{
    delete d->item;
    delete d;
}

void ActionCollectionModel::slotUpdated()
{
    emit layoutAboutToBeChanged();
    emit layoutChanged();
}

Action* ActionCollectionModel::action(const QModelIndex& index)
{
    ActionCollectionModelItem* item = index.isValid() ? static_cast<ActionCollectionModelItem*>(index.internalPointer()) : 0;
    return (item && item->type == ActionCollectionModelItem::ActionType) ? item->action : 0;
}

ActionCollection* ActionCollectionModel::collection(const QModelIndex& index)
{
    ActionCollectionModelItem* item = index.isValid() ? static_cast<ActionCollectionModelItem*>(index.internalPointer()) : 0;
    return (item && item->type == ActionCollectionModelItem::CollectionType) ? item->collection : 0;
}

int ActionCollectionModel::columnCount(const QModelIndex&) const
{
    return 1;
}

int ActionCollectionModel::rowCount(const QModelIndex& index) const
{
    ActionCollectionModelItem* item = index.isValid() ? static_cast<ActionCollectionModelItem*>(index.internalPointer()) : d->item;
    Q_ASSERT( item );
    if( item->type == ActionCollectionModelItem::CollectionType )
        return item->collection->actions().count() + item->collection->collections().count();
    return 0;
}

QModelIndex ActionCollectionModel::index(int row, int column, const QModelIndex& parent) const
{
    ActionCollectionModelItem* item = parent.isValid() ? static_cast<ActionCollectionModelItem*>(parent.internalPointer()) : d->item;
    Q_ASSERT( item && item->type == ActionCollectionModelItem::CollectionType );
    const int count = item->collection->actions().count();
    if( row < count ) {
        Action* action = dynamic_cast< Action* >( item->collection->actions().value(row) );
        if( action )
        {
            return createIndex(row, column,d->childItem(item,parent,row,column,action));
        }
    }
    else {
        QString name = item->collection->collections().value(row - count);
        ActionCollection* collection = item->collection->collection(name);
        if( collection )
        {
            return createIndex(row, column,d->childItem(item,parent,row,column,collection));
        }
    }
    return QModelIndex();
}

QModelIndex ActionCollectionModel::parent(const QModelIndex& index) const
{
    if( ! index.isValid() )
        return QModelIndex();
    return static_cast<ActionCollectionModelItem*>(index.internalPointer())->parent;
}

Qt::ItemFlags ActionCollectionModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags flags = QAbstractItemModel::flags(index);
    if( ! index.isValid() )
        return Qt::ItemIsDropEnabled | flags;

    flags |= Qt::ItemIsSelectable;
    //flags |= Qt::ItemIsEditable;
    flags |= Qt::ItemIsDragEnabled;
    flags |= Qt::ItemIsDropEnabled;

    if( (index.column() == 0) && (d->mode & UserCheckable) )
        flags |= Qt::ItemIsUserCheckable;
    return flags;
}

QVariant ActionCollectionModel::data(const QModelIndex& index, int role) const
{
    if( index.isValid() ) {
        ActionCollectionModelItem* item = static_cast<ActionCollectionModelItem*>(index.internalPointer());
        switch( item->type ) {
            case ActionCollectionModelItem::ActionType: {
                switch( role ) {
                    case Qt::DecorationRole: {
                        if( d->mode & Icons )
                            if( ! item->action->iconName().isEmpty() )
                                return item->action->icon();
                    } break;
                    case Qt::DisplayRole:
                        return item->action->text().remove('&');
                    case Qt::ToolTipRole: // fall through
                    case Qt::WhatsThisRole: {
                        if( d->mode & ToolTips ) {
                            const QString file = QFileInfo( item->action->file() ).fileName();
                            return QString("<qt><b>%1</b><br>%2</qt>")
                                .arg( file.isEmpty() ? item->action->name() : file )
                                .arg( item->action->description() );
                        }
                    } break;
                    case Qt::CheckStateRole: {
                        if( d->mode & UserCheckable )
                            return item->action->isEnabled();
                    } break;
                    default: break;
                }
            } break;
            case ActionCollectionModelItem::CollectionType: {
                switch( role ) {
                    case Qt::DecorationRole: {
                        if( d->mode & Icons )
                            if( ! item->collection->iconName().isEmpty() )
                                return item->collection->icon();
                    } break;
                    case Qt::DisplayRole:
                        return item->collection->text();
                    case Qt::ToolTipRole: // fall through
                    case Qt::WhatsThisRole: {
                        if( d->mode & ToolTips )
                            return QString("<qt><b>%1</b><br>%2</qt>").arg(item->collection->text()).arg(item->collection->description());
                    } break;
                    case Qt::CheckStateRole: {
                        if( d->mode & UserCheckable )
                            return item->collection->isEnabled();
                    } break;
                    default: break;
                }
            } break;
            default: break;
        }
    }
    return QVariant();
}

bool ActionCollectionModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    Q_UNUSED(value);
    if( ! index.isValid() /*|| ! (d->mode & UserCheckable)*/ )
        return false;
    ActionCollectionModelItem* item = static_cast<ActionCollectionModelItem*>(index.internalPointer());
    switch( item->type ) {
        case ActionCollectionModelItem::ActionType: {
            switch( role ) {
                //case Qt::EditRole: item->action->setText( value.toString() ); break;
                case Qt::CheckStateRole: item->action->setEnabled( ! item->action->isEnabled() ); break;
                default: return false;
            }
        } break;
        case ActionCollectionModelItem::CollectionType: {
            switch( role ) {
                //case Qt::EditRole: item->collection->setText( value.toString() ); break;
                case Qt::CheckStateRole: item->collection->setEnabled( ! item->collection->isEnabled() ); break;
                default: return false;
            }
        } break;
        default: return false;
    }
    //emit dataChanged(index, index);
    return true;
}

bool ActionCollectionModel::insertRows(int row, int count, const QModelIndex& parent)
{
    krossdebug( QString("ActionCollectionModel::insertRows: row=%1 count=%2").arg(row).arg(count) );
    if( ! parent.isValid() )
        return false;

    ActionCollectionModelItem* parentitem = static_cast<ActionCollectionModelItem*>(parent.internalPointer());
    switch( parentitem->type ) {
        case ActionCollectionModelItem::ActionType: {
            krossdebug( QString("ActionCollectionModel::insertRows: parentindex is Action with name=%1").arg(parentitem->action->name()) );
        } break;
        case ActionCollectionModelItem::CollectionType: {
            krossdebug( QString("ActionCollectionModel::insertRows: parentindex is ActionCollection with name=%1").arg(parentitem->collection->name()) );
        } break;
        default: break;
    }

    return QAbstractItemModel::insertRows(row, count, parent);
}

bool ActionCollectionModel::removeRows(int row, int count, const QModelIndex& parent)
{
    krossdebug( QString("ActionCollectionModel::removeRows: row=%1 count=%2").arg(row).arg(count) );
    return QAbstractItemModel::removeRows(row, count, parent);
}

bool ActionCollectionModel::insertColumns(int column, int count, const QModelIndex& parent)
{
    krossdebug( QString("ActionCollectionModel::insertColumns: column=%1 count=%2").arg(column).arg(count) );
    return QAbstractItemModel::insertColumns(column, count, parent);
}

bool ActionCollectionModel::removeColumns(int column, int count, const QModelIndex& parent)
{
    krossdebug( QString("ActionCollectionModel::removeColumns: column=%1 count=%2").arg(column).arg(count) );
    return QAbstractItemModel::removeColumns(column, count, parent);
}

QStringList ActionCollectionModel::mimeTypes() const
{
    //krossdebug( QString("ActionCollectionModel::mimeTypes") );
    return QStringList() << "application/vnd.text.list";
}

QString fullPath(const QModelIndex& index)
{
    if( ! index.isValid() ) return QString();
    ActionCollectionModelItem* item = static_cast<ActionCollectionModelItem*>(index.internalPointer());
    QString n = item->name();
    if( item->type == ActionCollectionModelItem::CollectionType ) n += '/';
    QString p = fullPath( item->parent ); //recursive
    return p.isNull() ? n : ( p.endsWith('/') ? p + n : p + '/' + n );
}

QMimeData* ActionCollectionModel::mimeData(const QModelIndexList& indexes) const
{
    //krossdebug( QString("ActionCollectionModel::mimeData") );
    QMimeData* mimeData = new QMimeData();
    QByteArray encodedData;

    QDataStream stream(&encodedData, QIODevice::WriteOnly);
    foreach(const QModelIndex &index, indexes) {
        //if( ! index.isValid() ) continue;
        //QString text = data(index, Qt::DisplayRole).toString();
        QString path = fullPath(index);
        if( ! path.isNull() )
            stream << path;
    }

    mimeData->setData("application/vnd.text.list", encodedData);
    return mimeData;
}

bool ActionCollectionModel::dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent)
{
    krossdebug( QString("ActionCollectionModel::dropMimeData: row=%1 col=%2").arg(row).arg(column) );
    if( action == Qt::IgnoreAction ) return true;
    if( ! data->hasFormat("application/vnd.text.list") ) return false;
    if( column > 0 ) return false;

    krossdebug( QString("ActionCollectionModel::dropMimeData: ENCODED DATA:") );
    QByteArray encodedData = data->data("application/vnd.text.list");
    QDataStream stream(&encodedData, QIODevice::ReadOnly);
    QStringList newItems;
    int rows = 0;
    while( ! stream.atEnd() ) {
        QString text;
        stream >> text;
        newItems << text;
        krossdebug( QString("  %1 \"%2\"").arg(rows).arg(text) );
        ++rows;
    }

    //FIXME: return false for now since insertRows/removeRows need to be implemented before!
    //return false;

    /*
    int beginRow;
    if( row != -1 )
        beginRow = row;
    else if( parent.isValid() )
        beginRow = parent.row();
    else
        beginRow = rowCount( QModelIndex() );
    krossdebug( QString("ActionCollectionModel::dropMimeData: beginRow=%1").arg(beginRow) );
    */

    ActionCollectionModelItem* targetparentitem = parent.isValid() ? static_cast<ActionCollectionModelItem*>(parent.internalPointer()) : d->item;
    switch( targetparentitem->type ) {
        case ActionCollectionModelItem::ActionType: {
            krossdebug( QString("ActionCollectionModel::dropMimeData: parentindex is Action with name=%1").arg(targetparentitem->action->name()) );
        } break;
        case ActionCollectionModelItem::CollectionType: {
            krossdebug( QString("ActionCollectionModel::dropMimeData: parentindex is ActionCollection with name=%1").arg(targetparentitem->collection->name()) );
        } break;
        default: break;
    }



    return false;
    //return QAbstractItemModel::dropMimeData(data, action, row, column, parent);
}

Qt::DropActions ActionCollectionModel::supportedDropActions() const
{
    return Qt::CopyAction | Qt::MoveAction | Qt::TargetMoveAction;
    //return Qt::CopyAction | Qt::MoveAction | Qt::TargetMoveAction | Qt::LinkAction;
}

/******************************************************************************
 * ActionCollectionProxyModel
 */

ActionCollectionProxyModel::ActionCollectionProxyModel(QObject* parent, ActionCollectionModel* model)
    : QSortFilterProxyModel(parent)
{
    setSourceModel( model ? model : new ActionCollectionModel(this) );
    setFilterCaseSensitivity(Qt::CaseInsensitive);
}

ActionCollectionProxyModel::~ActionCollectionProxyModel()
{
}

void ActionCollectionProxyModel::setSourceModel(QAbstractItemModel* sourceModel)
{
    Q_ASSERT( dynamic_cast< ActionCollectionModel* >(sourceModel) );
    QSortFilterProxyModel::setSourceModel(sourceModel);
}

bool ActionCollectionProxyModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
    QModelIndex index = sourceModel()->index(source_row, 0, source_parent);
    if( ! index.isValid() )
        return false;
    ActionCollectionModelItem* item = static_cast<ActionCollectionModelItem*>(index.internalPointer());
    switch( item->type ) {
            case ActionCollectionModelItem::ActionType: {
                if( ! item->action->isEnabled() )
                    return false;
                return QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
            } break;
            case ActionCollectionModelItem::CollectionType: {
                if( ! item->collection->isEnabled() )
                    return false;
            } break;
            default: break;
    }
    return true;
}

#include "model.moc"
