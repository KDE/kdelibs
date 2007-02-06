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
            Mode mode;
    };

}

ActionCollectionModel::ActionCollectionModel(QObject* parent, ActionCollection* collection, Mode mode)
    : QAbstractItemModel(parent)
    , d( new Private() )
{
    d->item = new ActionCollectionModelItem( collection ? collection : Kross::Manager::self().actionCollection() );
    d->mode = mode;
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
    if(index.column() == 0 && (d->mode & UserCheckable))
        return QAbstractItemModel::flags(index) | Qt::ItemIsUserCheckable;
    return QAbstractItemModel::flags(index); // | Qt::ItemIsEditable;
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
                            return item->action->icon();
                    } break;
                    case Qt::DisplayRole:
                        return item->action->text().replace("&","");
                    case Qt::ToolTipRole: // fall through
                    case Qt::WhatsThisRole: {
                        if( d->mode & ToolTips )
                            return item->action->description();
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
                    case Qt::DisplayRole:
                        return item->collection->text();
                    case Qt::ToolTipRole: // fall through
                    case Qt::WhatsThisRole: {
                        if( d->mode & ToolTips )
                            return item->collection->description();
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
    if( ! index.isValid() || ! (d->mode & UserCheckable) )
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
    emit dataChanged(index, index);
    return true;
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
