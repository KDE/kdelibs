/***************************************************************************
 * model.cpp
 * This file is part of the KDE project
 * copyright (C) 2006 by Sebastian Sauer (mail@dipe.org)
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
#include "action.h"
#include "actioncollection.h"
#include "manager.h"

#include <QEvent>

#include <kicon.h>
#include <kmenu.h>
//#include <kactioncollection.h>
//#include <kactionmenu.h>

using namespace Kross;

/******************************************************************************
 * ActionCollectionModel
 */

namespace Kross {

    /// \internal item representation.
    class ActionCollectionModelItem
    {
        public:
            enum Type { ActionType, CollectionType };
            const Type type;
            union {
                Action* action;
                ActionCollection* collection;
            };
            const QModelIndex parent;

            ActionCollectionModelItem(Action* a, const QModelIndex& p = QModelIndex()) : type(ActionType), action(a), parent(p) {}
            ActionCollectionModelItem(ActionCollection* c, const QModelIndex& p = QModelIndex()) : type(CollectionType), collection(c), parent(p) {}
    };

    /// \internal d-pointer class.
    class ActionCollectionModel::Private
    {
        public:
            ActionCollectionModelItem* item;
    };

}

ActionCollectionModel::ActionCollectionModel(QObject* parent, ActionCollection* collection)
    : QAbstractItemModel(parent)
    , d( new Private() )
{
    d->item = new ActionCollectionModelItem( collection ? collection : Kross::Manager::self().actionCollection() );
}

ActionCollectionModel::~ActionCollectionModel()
{
    delete d->item;
    delete d;
}

Action* ActionCollectionModel::action(const QModelIndex& index)
{
    ActionCollectionModelItem* item = index.isValid() ? static_cast<ActionCollectionModelItem*>(index.internalPointer()) : 0;
    return (item && item->type == ActionCollectionModelItem::ActionType) ? item->action : 0;
}

ActionCollection* ActionCollectionModel::collection(const QModelIndex& index)
{
    ActionCollectionModelItem* item = index.isValid() ? static_cast<ActionCollectionModelItem*>(index.internalPointer()) : 0;
    return (item && item->type == ActionCollectionModelItem::ActionType) ? item->collection : 0;
}

int ActionCollectionModel::columnCount(const QModelIndex&) const
{
    return 1;
}

int ActionCollectionModel::rowCount(const QModelIndex& index) const
{
    ActionCollectionModelItem* item = index.isValid() ? static_cast<ActionCollectionModelItem*>(index.internalPointer()) : d->item;
    if( item->type == ActionCollectionModelItem::CollectionType )
        return item->collection->actions().count() + item->collection->collections().count();
    return 0;
}

QModelIndex ActionCollectionModel::index(int row, int column, const QModelIndex& parent) const
{
    ActionCollectionModelItem* item = parent.isValid() ? static_cast<ActionCollectionModelItem*>(parent.internalPointer()) : d->item;
    const int count = item->collection->actions().count();
    if( row < count ) {
        Action* action = dynamic_cast< Action* >( item->collection->actions().value(row) );
        if( action )
            return createIndex(row, column, new ActionCollectionModelItem(action, parent));
    }
    else {
        QString name = item->collection->collections().value(row - count);
        ActionCollection* collection = item->collection->collection(name);
        if( collection )
            return createIndex(row, column, new ActionCollectionModelItem(collection, parent));
    }
    return QModelIndex();
}

QModelIndex ActionCollectionModel::parent(const QModelIndex& index) const
{
    if( index.isValid() ) {
        ActionCollectionModelItem* item = static_cast<ActionCollectionModelItem*>(index.internalPointer());
        return item->parent;
    }
    return QModelIndex();
}

Qt::ItemFlags ActionCollectionModel::flags(const QModelIndex &index) const
{
    if( ! index.isValid() )
        return Qt::ItemIsEnabled;
    //if(index.column() == 0 /*&& d->editable*/) return QAbstractItemModel::flags(index); // | Qt::ItemIsUserCheckable;
    return QAbstractItemModel::flags(index); // | Qt::ItemIsEditable;
}

QVariant ActionCollectionModel::data(const QModelIndex& index, int role) const
{
    if( index.isValid() ) {
        ActionCollectionModelItem* item = static_cast<ActionCollectionModelItem*>(index.internalPointer());
        switch( item->type ) {
            case ActionCollectionModelItem::ActionType: {
                switch( role ) {
                    case Qt::DecorationRole:
                        return item->action->icon();
                    case Qt::DisplayRole:
                        return item->action->text().replace("&","");
                    case Qt::ToolTipRole: // fall through
                    case Qt::WhatsThisRole:
                        return item->action->description();
                    //case Qt::CheckStateRole:
                    //    return item->action->isVisible();
                    default: break;
                }
            } break;
            case ActionCollectionModelItem::CollectionType: {
                switch( role ) {
                    case Qt::DisplayRole:
                        return item->collection->text();
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
#if 0
    if( ! index.isValid() /*|| ! d->editable*/ )
        return false;
    Action* action = static_cast< Action* >( index.internalPointer() );
    switch( role ) {
        case Qt::EditRole: {
            action->setText( value.toString() );
        } break;
        case Qt::CheckStateRole: {
            action->setVisible( ! action->isVisible() );
        } break;
        default:
            return false;
    }
    emit dataChanged(index, index);
    return true;
#else
    return false;
#endif
}

/******************************************************************************
 * ActionCollectionProxyModel
 */

ActionCollectionProxyModel::ActionCollectionProxyModel(QObject* parent, ActionCollection* collection)
    : QSortFilterProxyModel(parent)
{
    setSourceModel( new ActionCollectionModel(this, collection) );
    setFilterCaseSensitivity(Qt::CaseInsensitive);
}

ActionCollectionProxyModel::~ActionCollectionProxyModel()
{
}

void ActionCollectionProxyModel::setSourceModel(QAbstractItemModel* sourceModel)
{
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
            case ActionCollectionModelItem::CollectionType: // fall through
                if( ! item->collection->isEnabled() )
                    return false;
            default: break;
    }
    return true;
}
