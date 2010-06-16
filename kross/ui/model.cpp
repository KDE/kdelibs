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

#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>

#include <QtCore/QEvent>
#include <QtCore/QMimeData>

using namespace Kross;

/******************************************************************************
 * ActionCollectionModel
 */

namespace Kross {

    /// \internal d-pointer class.
    class ActionCollectionModel::Private
    {
        public:
            ActionCollection* collection;
            Mode mode;
    };

}

ActionCollectionModel::ActionCollectionModel(QObject* parent, ActionCollection* collection, Mode mode)
    : QAbstractItemModel(parent)
    , d( new Private() )
{
    //krossdebug( QString( "ActionCollectionModel::ActionCollectionModel:") );
    d->collection = collection ? collection : Kross::Manager::self().actionCollection();
    d->mode = mode;
    //setSupportedDragActions(Qt::MoveAction);

    //ActionCollection propagates signals to parent
    QObject::connect( d->collection, SIGNAL( dataChanged( Action* ) ), this, SLOT( slotDataChanged( Action* ) ) );
    QObject::connect( d->collection, SIGNAL( dataChanged( ActionCollection* ) ), this, SLOT( slotDataChanged( ActionCollection* ) ) );

    QObject::connect( d->collection, SIGNAL( collectionToBeInserted( ActionCollection*, ActionCollection* ) ), this, SLOT( slotCollectionToBeInserted( ActionCollection*, ActionCollection* ) ) );
    QObject::connect( d->collection, SIGNAL( collectionInserted( ActionCollection*, ActionCollection* ) ), this, SLOT( slotCollectionInserted( ActionCollection*, ActionCollection* ) ) );
    QObject::connect( d->collection, SIGNAL( collectionToBeRemoved( ActionCollection*, ActionCollection* ) ), this, SLOT( slotCollectionToBeRemoved( ActionCollection*, ActionCollection* ) ) );
    QObject::connect( d->collection, SIGNAL( collectionRemoved( ActionCollection*, ActionCollection* ) ), this, SLOT( slotCollectionRemoved( ActionCollection*, ActionCollection* ) ) );

    QObject::connect( d->collection, SIGNAL( actionToBeInserted( Action*, ActionCollection* ) ), this, SLOT( slotActionToBeInserted( Action*, ActionCollection* ) ) );
    QObject::connect( d->collection, SIGNAL( actionInserted( Action*, ActionCollection* ) ), this, SLOT( slotActionInserted( Action*, ActionCollection* ) ) );
    QObject::connect( d->collection, SIGNAL( actionToBeRemoved( Action*, ActionCollection* ) ), this, SLOT( slotActionToBeRemoved( Action*, ActionCollection* ) ) );
    QObject::connect( d->collection, SIGNAL( actionRemoved( Action*, ActionCollection* ) ), this, SLOT( slotActionRemoved( Action*, ActionCollection* ) ) );
}

ActionCollectionModel::~ActionCollectionModel()
{
    delete d;
}

ActionCollection *ActionCollectionModel::rootCollection() const
{
    return d->collection;
}

int ActionCollectionModel::rowNumber( ActionCollection *collection ) const
{
    Q_ASSERT( collection != 0 );
    ActionCollection *par = collection->parentCollection();
    Q_ASSERT( par != 0 );
    int row = par->collections().indexOf( collection->objectName() ) + par->actions().count();
    return row;
}

QModelIndex ActionCollectionModel::indexForCollection( ActionCollection *collection ) const
{
    if ( collection == d->collection ) {
        return QModelIndex();
    }
    return createIndex( rowNumber( collection ), 0, collection->parentCollection() );
}

QModelIndex ActionCollectionModel::indexForAction( Action *act ) const
{
    ActionCollection *coll = static_cast<ActionCollection*>( act->parent() );
    return createIndex( coll->actions().indexOf( act ), 0, coll );
}

void ActionCollectionModel::slotCollectionToBeInserted( ActionCollection* child, ActionCollection* parent )
{
    //krossdebug( QString( "ActionCollectionModel::slotCollectionToBeInserted: %1 %2" ).arg( child->name() ).arg( parent->name( ) )  );
    Q_ASSERT( parent );
    Q_UNUSED( child )
    int row = parent->actions().count() + parent->collections().count(); // we assume child is appended!!
    QModelIndex parIdx = indexForCollection( parent );
    beginInsertRows( parIdx, row, row );
}

void ActionCollectionModel::slotCollectionInserted( ActionCollection*, ActionCollection* )
{
    //krossdebug( QString( "ActionCollectionModel::slotCollectionInserted: %1 %2" ).arg( child->name( ) ).arg( parent->name( ) )  );
    endInsertRows();
}

void ActionCollectionModel::slotCollectionToBeRemoved( ActionCollection* child, ActionCollection* parent )
{
    //krossdebug( QString( "ActionCollectionModel::slotCollectionToBeRemoved: %1 %2" ).arg( child->name() ).arg( parent->name() ) );
    int row = rowNumber( child );
    QModelIndex parIdx = indexForCollection( parent );
    beginRemoveRows( parIdx, row, row );
}

void ActionCollectionModel::slotCollectionRemoved( ActionCollection*, ActionCollection* )
{
    //krossdebug( QString( "ActionCollectionModel::slotCollectionRemoved: %1 %2" ).arg( child->name() ).arg( parent->name() ) );
    endRemoveRows();
}

void ActionCollectionModel::slotActionToBeInserted( Action* child, ActionCollection* parent )
{
    //krossdebug( QString( "ActionCollectionModel::slotActionInserted: %1 %2" ).arg( child->name() ).arg( parent->name() ) );
    Q_ASSERT( parent );
    Q_UNUSED( child )
    int row = parent->actions().count(); // assume child is appended to actions!!
    QModelIndex parIdx = indexForCollection( parent );
    beginInsertRows( parIdx, row, row );
}

void ActionCollectionModel::slotActionInserted( Action*, ActionCollection* )
{
    //krossdebug( QString( "ActionCollectionModel::slotActionInserted: %1 %2" ).arg( child->name() ).arg( parent->name() ) );
    endInsertRows();
}

void ActionCollectionModel::slotActionToBeRemoved( Action* child, ActionCollection* parent )
{
    //krossdebug( QString( "ActionCollectionModel::slotActionToBeRemoved: %1 %2" ).arg( child->name() ).arg( parent->name() ) );
    Q_ASSERT( parent );
    int row = parent->actions().indexOf( child );
    QModelIndex parIdx = indexForCollection( parent );
    beginRemoveRows( parIdx, row, row );
}

void ActionCollectionModel::slotActionRemoved( Action*, ActionCollection* )
{
    //krossdebug( QString( "ActionCollectionModel::slotActionRemoved: %1 %2" ).arg( child->name() ).arg( parent->name() ) );
    endRemoveRows();
}

//NOTE: not used anymore, remove?
void ActionCollectionModel::slotUpdated()
{
    //emit layoutAboutToBeChanged();
    //emit layoutChanged();
}

void ActionCollectionModel::slotDataChanged( ActionCollection* coll )
{
    //krossdebug( QString( "ActionCollectionModel::slotDataChanged: %1" ).arg( coll->name() ) );
    QModelIndex idx = indexForCollection( coll );
    emit dataChanged( idx, idx ); // NOTE: change if more than one column
}

void ActionCollectionModel::slotDataChanged( Action* act )
{
    //krossdebug( QString( "ActionCollectionModel::slotDataChanged: %1" ).arg( act->name() ) );
    QModelIndex idx = indexForAction( act );
    emit dataChanged( idx, idx ); // NOTE: change if more than one column
}

Action* ActionCollectionModel::action(const QModelIndex& index)
{
    ActionCollection *par = static_cast<ActionCollection*>( index.internalPointer() );
    if ( par == 0 || index.row() >= par->actions().count() ) {
        return 0;
    }
    return par->actions().value( index.row() );
}

ActionCollection* ActionCollectionModel::collection(const QModelIndex& index)
{
    ActionCollection *par = static_cast<ActionCollection*>( index.internalPointer() );
    if ( par == 0 ) {
        return 0;
    }
    int row = index.row() - par->actions().count();
    if ( row < 0 ) {
        return 0; // this is probably an action
    }
    return par->collection( par->collections().value( row) );
}

int ActionCollectionModel::columnCount(const QModelIndex&) const
{
    return 1;
}

int ActionCollectionModel::rowCount(const QModelIndex& index) const
{
    if ( action( index) ) {
        return 0;
    }
    ActionCollection* par = index.isValid() ? collection( index ) : d->collection;
    Q_ASSERT_X( par, "ActionCollectionModel::rowCount", "index is not an action nor a collection" );
    if (!par) {
        kWarning()<<"index is not an action nor a collection"<<index;
        return 0;
    }
    int rows = par->actions().count() + par->collections().count();
    return rows;
}

QModelIndex ActionCollectionModel::index(int row, int column, const QModelIndex& parent) const
{
    if ( ! hasIndex( row, column, parent ) ) {
        return QModelIndex();
    }
    ActionCollection* par = parent.isValid() ? collection( parent ) : d->collection;
    if ( par == 0 ) {
        // safety: may happen if parent index is an action (ModelTest tests this)
        return QModelIndex();
    }
    return createIndex( row, column, par );
}

QModelIndex ActionCollectionModel::parent(const QModelIndex& index) const
{
    if( ! index.isValid() ) {
        return QModelIndex();
    }
    ActionCollection *par = static_cast<ActionCollection*>( index.internalPointer() );
    Q_ASSERT( par != 0 );
    if ( par == d->collection ) {
        return QModelIndex();
    }
    return createIndex( rowNumber( par ), 0, par->parentCollection() );
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
        Action *act = action( index );
        if ( act ) {
            switch( role ) {
                case Qt::DecorationRole: {
                    if( d->mode & Icons )
                        if( ! act->iconName().isEmpty() )
                            return act->icon();
                } break;
                case Qt::DisplayRole:
                    return KGlobal::locale()->removeAcceleratorMarker( act->text() );
                case Qt::ToolTipRole: // fall through
                case Qt::WhatsThisRole: {
                    if( d->mode & ToolTips ) {
                        const QString file = QFileInfo( act->file() ).fileName();
                        return QString( "<qt><b>%1</b><br>%2</qt>" )
                            .arg( file.isEmpty() ? act->name() : file )
                            .arg( act->description() );
                    }
                } break;
                case Qt::CheckStateRole: {
                    if( d->mode & UserCheckable )
                        return act->isEnabled() ? Qt::Checked : Qt::Unchecked;
                } break;
                default: break;
            }
            return QVariant();
        }
        ActionCollection *coll = collection( index );
        if ( coll ) {
            switch( role ) {
                case Qt::DecorationRole: {
                    if( d->mode & Icons )
                        if( ! coll->iconName().isEmpty() )
                            return coll->icon();
                } break;
                case Qt::DisplayRole:
                    return coll->text();
                case Qt::ToolTipRole: // fall through
                case Qt::WhatsThisRole: {
                    if( d->mode & ToolTips )
                        return QString( "<qt><b>%1</b><br>%2</qt>" ).arg( coll->text() ).arg( coll->description() );
                } break;
                case Qt::CheckStateRole: {
                    if( d->mode & UserCheckable )
                        return coll->isEnabled() ? Qt::Checked : Qt::Unchecked;
                } break;
                default: break;
            }
            return QVariant();
        }
    }
    return QVariant();
}

bool ActionCollectionModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    Q_UNUSED(value);
    if( ! index.isValid() /*|| ! (d->mode & UserCheckable)*/ )
        return false;
    
    Action *act = action( index );
    if ( act ) {
        switch( role ) {
            //case Qt::EditRole: act->setText( value.toString() ); break;
            case Qt::CheckStateRole: act->setEnabled( ! act->isEnabled() ); break;
            default: return false;
        }
        return false;
    }
    ActionCollection *coll = collection( index );
    if ( coll ) {
        switch( role ) {
            //case Qt::EditRole: item->coll->setText( value.toString() ); break;
            case Qt::CheckStateRole: coll->setEnabled( ! coll->isEnabled() ); break;
            default: return false;
        }
        return false;
    }
    //emit dataChanged(index, index);
    return true;
}

bool ActionCollectionModel::insertRows(int row, int count, const QModelIndex& parent)
{
    krossdebug( QString("ActionCollectionModel::insertRows: row=%1 count=%2").arg(row).arg(count) );
    if( ! parent.isValid() )
        return false;

    ActionCollection* coll = collection( parent );
    if ( coll ) {
        krossdebug( QString( "ActionCollectionModel::insertRows: parentindex is ActionCollection with name=%1" ).arg( coll->name() ) );
    } else {
        Action *act = action( parent );
        if ( act ) {
            krossdebug( QString( "ActionCollectionModel::insertRows: parentindex is Action with name=%1" ).arg( act->name() ) );
        }
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
    QString n;
    Action *a = ActionCollectionModel::action( index );
    if ( a ) {
        n = a->name();
    } else {
        ActionCollection *c = ActionCollectionModel::collection( index );
        if ( c ) {
            n = c->name() + '/';
            if ( ! n.endsWith('/' ) )
                n += '/';
        }
    }
    ActionCollection* par = static_cast<ActionCollection*>( index.internalPointer() );
    for ( ActionCollection *p = par; p != 0; p = par->parentCollection() ) {
        QString s = p->name();
        if ( ! s.endsWith( '/' ) ) {
            s += '/';
        }
        n = s + n;
    }
    return n;
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

    QModelIndex targetindex = index( row, column, parent );
    ActionCollection *coll = collection( targetindex );
    if ( coll ) {
        krossdebug( QString( "ActionCollectionModel::dropMimeData: parentindex is ActionCollection with name=%1" ).arg( coll->name() ) );
    } else {
        Action *act = this->action( targetindex );
        if ( act ) {
            krossdebug( QString( "ActionCollectionModel::dropMimeData: parentindex is Action with name=%1" ).arg( act->name() ) );
        }
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
    setDynamicSortFilter(true);
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
    //krossdebug( QString( "ActionCollectionProxyModel::filterAcceptsRow: row=%1 parentrow=%2" ).arg( source_row ).arg( source_parent.row() ) );
    QModelIndex index = sourceModel()->index(source_row, 0, source_parent);
    if( ! index.isValid() )
        return false;

    Action *action = ActionCollectionModel::action( index );
    if ( action ) {
        return action->isEnabled() && QSortFilterProxyModel::filterAcceptsRow( source_row, source_parent );
    }
    ActionCollection *collection = ActionCollectionModel::collection( index );
    if( collection ) {
        return collection->isEnabled();
    }
    return true;
}

#include "model.moc"
