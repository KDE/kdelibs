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

#include "kpagewidget.h"

#include "kpagewidgetmodel.h"

class KPageWidget::Private
{
  public:
    Private( KPageWidget *_parent )
      : parent( _parent )
    {
    }

    KPageWidgetModel *model;

    void slotCurrentPageChanged( const QModelIndex&, const QModelIndex& );

    KPageWidget *parent;
};

void KPageWidget::Private::slotCurrentPageChanged( const QModelIndex &current, const QModelIndex &before )
{
  KPageWidgetItem *currentItem = 0;
  if ( current.isValid() )
    currentItem = model->item( current );

  KPageWidgetItem *beforeItem = 0;
  if ( before.isValid() )
    beforeItem = model->item( before );

  emit parent->currentPageChanged( currentItem, beforeItem );
}

KPageWidget::KPageWidget( QWidget *parent )
  : KPageView( parent ), d( new Private( this ) )
{
  d->model = new KPageWidgetModel( this );

  connect( this, SIGNAL( currentPageChanged( const QModelIndex&, const QModelIndex& ) ),
           this, SLOT( slotCurrentPageChanged( const QModelIndex&, const QModelIndex& ) ) );

  connect( d->model, SIGNAL( toggled( KPageWidgetItem*, bool ) ),
           this, SIGNAL( pageToggled( KPageWidgetItem*, bool ) ) );

  setModel( d->model );
}

KPageWidget::~KPageWidget()
{
  delete d;
}

KPageWidgetItem* KPageWidget::addPage( QWidget *widget, const QString &name )
{
  return d->model->addPage( widget, name );
}

void KPageWidget::addPage( KPageWidgetItem *item )
{
  d->model->addPage( item );
}

KPageWidgetItem* KPageWidget::insertPage( KPageWidgetItem *before, QWidget *widget, const QString &name )
{
  return d->model->insertPage( before, widget, name );
}

void KPageWidget::insertPage( KPageWidgetItem *before, KPageWidgetItem *item )
{
  d->model->insertPage( before, item );
}

KPageWidgetItem* KPageWidget::addSubPage( KPageWidgetItem *parent, QWidget *widget, const QString &name )
{
  return d->model->addSubPage( parent, widget, name );
}

void KPageWidget::addSubPage( KPageWidgetItem *parent, KPageWidgetItem *item )
{
  d->model->addSubPage( parent, item );
}

void KPageWidget::removePage( KPageWidgetItem *item )
{
  d->model->removePage( item );
}

void KPageWidget::setCurrentPage( KPageWidgetItem *item )
{
  const QModelIndex index = d->model->index( item );
  if ( !index.isValid() )
    return;

  KPageView::setCurrentPage( index );
}

KPageWidgetItem* KPageWidget::currentPage() const
{
  const QModelIndex index = KPageView::currentPage();

  if ( !index.isValid() )
    return 0;

  return d->model->item( index );
}

#include "kpagewidget.moc"
