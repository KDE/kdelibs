/*
    This file is part of the KDE Libraries

    Copyright (C) 2006 Tobias Koenig (tokoe@kde.org)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB. If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <QPointer>
#include <QWidget>

#include "kpagewidgetmodel.h"

class KPageWidgetItem::Private
{
  public:
    Private()
      : checkable( false )
    {
    }

    ~Private()
    {
      delete widget;
      widget = 0;
    }

    QString name;
    QString header;
    QIcon icon;
    QPointer<QWidget> widget;
    bool checkable;
};

KPageWidgetItem::KPageWidgetItem( QWidget *widget, const QString &name )
  : d( new Private )
{
  d->widget = widget;
  d->name = name;
}

KPageWidgetItem::~KPageWidgetItem()
{
  delete d;
}

QWidget* KPageWidgetItem::widget() const
{
  return d->widget;
}

QString KPageWidgetItem::name() const
{
  return d->name;
}

void KPageWidgetItem::setHeader( const QString &header )
{
  d->header = header;
}

QString KPageWidgetItem::header() const
{
  return d->header;
}

void KPageWidgetItem::setIcon( const QIcon &icon )
{
  d->icon = icon;
}

QIcon KPageWidgetItem::icon() const
{
  return d->icon;
}

void KPageWidgetItem::setIsCheckable( bool checkable )
{
  d->checkable = checkable;
}

bool KPageWidgetItem::isCheckable() const
{
  return d->checkable;
}


class PageItem
{
  public:
    PageItem( KPageWidgetItem *pageItem, PageItem *parent = 0);
    ~PageItem();

    void appendChild( PageItem *child );
    void insertChild( int row, PageItem *child );
    void removeChild( int row );

    PageItem *child( int row );
    int childCount() const;
    int columnCount() const;
    int row() const;
    PageItem *parent();

    KPageWidgetItem* pageWidgetItem() const;

    PageItem *findChild( const KPageWidgetItem *item );

    void dump( int indent = 0 );

  private:
    KPageWidgetItem *mPageWidgetItem;

    QList<PageItem*> mChildItems;
    PageItem *mParentItem;
};

PageItem::PageItem( KPageWidgetItem *pageWidgetItem, PageItem *parent )
  : mPageWidgetItem( pageWidgetItem ), mParentItem( parent )
{
}

PageItem::~PageItem()
{
  delete mPageWidgetItem;
  mPageWidgetItem = 0;

  qDeleteAll(mChildItems);
}

void PageItem::appendChild( PageItem *item )
{
  mChildItems.append( item );
}

void PageItem::insertChild( int row, PageItem *item )
{
  mChildItems.insert( row, item );
}

void PageItem::removeChild( int row )
{
  mChildItems.removeAt( row );
}

PageItem *PageItem::child( int row )
{
  return mChildItems.value( row );
}

int PageItem::childCount() const
{
  return mChildItems.count();
}

int PageItem::columnCount() const
{
  return 1;
}

PageItem *PageItem::parent()
{
  return mParentItem;
}

int PageItem::row() const
{
  if ( mParentItem )
    return mParentItem->mChildItems.indexOf( const_cast<PageItem*>(this) );

  return 0;
}

KPageWidgetItem* PageItem::pageWidgetItem() const
{
  return mPageWidgetItem;
}

PageItem *PageItem::findChild( const KPageWidgetItem *item )
{
  if ( mPageWidgetItem == item )
    return this;

  for ( int i = 0; i < mChildItems.count(); ++i ) {
    PageItem *pageItem = mChildItems[ i ]->findChild( item );
    if ( pageItem )
      return pageItem;
  }

  return 0;
}

void PageItem::dump( int indent )
{
  QString prefix;
  for ( int i = 0; i < indent; ++i )
    prefix.append( " " );

  const QString name = ( mPageWidgetItem ? mPageWidgetItem->name() : "root" );
  qDebug( "%s (%p)", qPrintable( QString( "%1%2" ).arg( prefix, name ) ), this );
  for ( int i = 0; i < mChildItems.count(); ++i )
    mChildItems[ i ]->dump( indent + 2 );
}


class KPageWidgetModel::Private
{
  public:
    Private()
    {
      rootItem = new PageItem( 0, 0 );
    }

    ~Private()
    {
      delete rootItem;
      rootItem = 0;
    }

    PageItem *rootItem;
};

KPageWidgetModel::KPageWidgetModel( QObject *parent )
  : KPageModel( parent ),
    d( new Private )
{
}

KPageWidgetModel::~KPageWidgetModel()
{
  delete d;
}

int KPageWidgetModel::columnCount( const QModelIndex& ) const
{
  return 1;
}

QVariant KPageWidgetModel::data( const QModelIndex &index, int role ) const
{
  if ( !index.isValid() )
    return QVariant();

  PageItem *item = static_cast<PageItem*>( index.internalPointer() );

  if ( role == Qt::DisplayRole )
    return QVariant( item->pageWidgetItem()->name() );
  else if ( role == Qt::DecorationRole )
    return QVariant( item->pageWidgetItem()->icon() );
  else if ( role == HeaderRole )
    return QVariant( item->pageWidgetItem()->header() );
  else if ( role == WidgetRole )
    return QVariant::fromValue( item->pageWidgetItem()->widget() );
  else if ( role == Qt::CheckStateRole ) {
    if ( item->pageWidgetItem()->isCheckable() ) {
      return Qt::Checked; // TODO
    } else
      return QVariant();
  } else
    return QVariant();
}

Qt::ItemFlags KPageWidgetModel::flags( const QModelIndex &index ) const
{
  if ( !index.isValid() )
    return 0;

  Qt::ItemFlags flags = Qt::ItemIsSelectable | Qt::ItemIsEnabled;

  PageItem *item = static_cast<PageItem*>( index.internalPointer() );
  if ( item->pageWidgetItem()->isCheckable() )
    flags |= Qt::ItemIsUserCheckable;

  return flags;
}

QModelIndex KPageWidgetModel::index( int row, int column, const QModelIndex &parent ) const
{
  PageItem *parentItem;

  if ( parent.isValid() )
    parentItem = static_cast<PageItem*>( parent.internalPointer() );
  else
    parentItem = d->rootItem;

  PageItem *childItem = parentItem->child( row );
  if ( childItem )
    return createIndex( row, column, childItem );
  else
    return QModelIndex();
}

QModelIndex KPageWidgetModel::parent( const QModelIndex &index ) const
{
  if ( !index.isValid() )
    return QModelIndex();

  PageItem *item = static_cast<PageItem*>( index.internalPointer() );
  PageItem *parentItem = item->parent();

  if ( parentItem == d->rootItem )
    return QModelIndex();
  else
    return createIndex( parentItem->row(), 0, parentItem );
}

int KPageWidgetModel::rowCount( const QModelIndex &parent ) const
{
  PageItem *parentItem;

  if ( !parent.isValid() )
    parentItem = d->rootItem;
  else
    parentItem = static_cast<PageItem*>( parent.internalPointer() );

  return parentItem->childCount();
}

KPageWidgetItem* KPageWidgetModel::addPage( QWidget *widget, const QString &name )
{
  KPageWidgetItem *item = new KPageWidgetItem( widget, name );

  addPage( item );

  return item;
}

void KPageWidgetModel::addPage( KPageWidgetItem *item )
{
  PageItem *pageItem = new PageItem( item, d->rootItem );
  d->rootItem->appendChild( pageItem );

  emit layoutChanged();
}

KPageWidgetItem* KPageWidgetModel::insertPage( KPageWidgetItem *before, QWidget *widget, const QString &name )
{
  KPageWidgetItem *item = new KPageWidgetItem( widget, name );

  insertPage( before, item );

  return item;
}

void KPageWidgetModel::insertPage( KPageWidgetItem *before, KPageWidgetItem *item )
{
  PageItem *beforePageItem = d->rootItem->findChild( before );
  if ( !beforePageItem ) {
    qDebug( "Invalid KPageWidgetItem passed!" );
    return;
  }

  PageItem *parent = beforePageItem->parent();

  PageItem *newPageItem = new PageItem( item, parent );
  parent->insertChild( beforePageItem->row(), newPageItem );

  emit layoutChanged();
}

KPageWidgetItem* KPageWidgetModel::addSubPage( KPageWidgetItem *parent, QWidget *widget, const QString &name )
{
  KPageWidgetItem *item = new KPageWidgetItem( widget, name );

  addSubPage( parent, item );

  return item;
}

void KPageWidgetModel::addSubPage( KPageWidgetItem *parent, KPageWidgetItem *item )
{
  PageItem *parentPageItem = d->rootItem->findChild( parent );
  if ( !parentPageItem ) {
    qDebug( "Invalid KPageWidgetItem passed!" );
    return;
  }

  PageItem *newPageItem = new PageItem( item, parentPageItem );
  parentPageItem->appendChild( newPageItem );

  emit layoutChanged();
}

void KPageWidgetModel::removePage( KPageWidgetItem *item )
{
  if ( !item )
    return;

  PageItem *pageItem = d->rootItem->findChild( item );
  if ( !pageItem ) {
    qDebug( "Invalid KPageWidgetItem passed!" );
    return;
  }

  PageItem *parentPageItem = pageItem->parent();
  int row = parentPageItem->row();

  QModelIndex index;
  if ( parentPageItem != d->rootItem )
    index = createIndex( row, 0, parentPageItem );

  beginRemoveRows( index, pageItem->row(), 1 );

  parentPageItem->removeChild( pageItem->row() );
  delete pageItem;

  endRemoveRows();

  emit layoutChanged();
}

KPageWidgetItem* KPageWidgetModel::item( const QModelIndex &index )
{
  if ( !index.isValid() )
    return 0;

  PageItem *item = static_cast<PageItem*>( index.internalPointer() );
  if ( !item )
    return 0;

  return item->pageWidgetItem();
}

QModelIndex KPageWidgetModel::index( const KPageWidgetItem *item ) const
{
  if ( !item )
    return QModelIndex();

  const PageItem *pageItem = d->rootItem->findChild( item );
  if ( !pageItem ) {
    return QModelIndex();
  }

  return createIndex( pageItem->row(), 0, (void*)pageItem );
}
