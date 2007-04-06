/* This file is part of the KDE libraries
   Copyright (c) 2004 Gustavo Sverzut Barbieri <gsbarbieri@users.sourceforge.net>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

/**
 * \todo
 *     Maybe we should have a common interface for SearchLines, this file
 * is so close (it's actually based on) k3listviewsearchline! Only few methods
 * would be reimplemented.
 */

#include "k3iconviewsearchline.h"

#include <Qt3Support/Q3IconDrag>
#include <klocale.h>
#include <QtCore/QTimer>
#include <kdebug.h>

#define DEFAULT_CASESENSITIVE false

typedef QList <Q3IconViewItem *> QIconViewItemList;

class K3IconViewSearchLine::K3IconViewSearchLinePrivate
{
public:
  K3IconViewSearchLinePrivate() :
    iconView( 0 ),
    caseSensitive( DEFAULT_CASESENSITIVE ),
    activeSearch( false ),
    queuedSearches( 0 ) {}

  Q3IconView *iconView;
  bool caseSensitive;
  bool activeSearch;
  QString search;
  int queuedSearches;
  QIconViewItemList hiddenItems;
};

/******************************************************************************
 * Public Methods                                                             *
 *****************************************************************************/
K3IconViewSearchLine::K3IconViewSearchLine( QWidget *parent,
					  Q3IconView *iconView ) :
  KLineEdit( parent )
{
  d = NULL;
  init( iconView );
}

K3IconViewSearchLine::~K3IconViewSearchLine()
{
  clear(); // empty hiddenItems, returning items back to iconView
  delete d;
}

bool K3IconViewSearchLine::caseSensitive() const
{
  return d->caseSensitive;
}

Q3IconView *K3IconViewSearchLine::iconView() const
{
  return d->iconView;
}

/******************************************************************************
 * Public Slots                                                               *
 *****************************************************************************/
void K3IconViewSearchLine::updateSearch( const QString &s )
{
  Q3IconView *iv = d->iconView;
  if( ! iv )
    return; // disabled

  QString search = d->search = s.isNull() ? text() : s;

  QIconViewItemList *hi = &(d->hiddenItems);

  Q3IconViewItem *currentItem = iv->currentItem();

  Q3IconViewItem *item = NULL;

  // Remove Non-Matching items, add them them to hidden list
  Q3IconViewItem *i = iv->firstItem();
  while ( i != NULL )
    {
      item = i;
      i = i->nextItem(); // Point to next, otherwise will loose it.
      if ( ! itemMatches( item, search ) )
	{
	  hideItem( item );

	  if ( item == currentItem )
	    currentItem = NULL; // It's not in iconView anymore.
	}
    }

    // Add Matching items, remove from hidden list
    QIconViewItemList::iterator it = hi->begin();
    while ( it != hi->end() )
      {
	item = *it;
	++it;
	if ( itemMatches( item, search ) )
	  showItem( item );
      }

    iv->sort();

    if ( currentItem != NULL )
      iv->ensureItemVisible( currentItem );
}

void K3IconViewSearchLine::clear()
{
  // Clear hidden list, give items back to QIconView, if it still exists
  Q3IconViewItem *item = NULL;
  QIconViewItemList::iterator it = d->hiddenItems.begin();
  while ( it != d->hiddenItems.end() )
    {
      item = *it;
      ++it;
      if ( item != NULL )
	{
	  if ( d->iconView != NULL )
	    showItem( item );
	  else
	    delete item;
	}
    }
  if ( ! d->hiddenItems.isEmpty() )
    kDebug() << __FILE__ << ":" << __LINE__ <<
      "hiddenItems is not empty as it should be. " <<
      d->hiddenItems.count() << " items are still there.\n" << endl;

  d->search = "";
  d->queuedSearches = 0;
  KLineEdit::clear();
}

void K3IconViewSearchLine::setCaseSensitive( bool cs )
{
  d->caseSensitive = cs;
}

void K3IconViewSearchLine::setIconView( Q3IconView *iv )
{
  if ( d->iconView != NULL )
    disconnect( d->iconView, SIGNAL( destroyed() ),
		this,        SLOT(   iconViewDeleted() ) );

  d->iconView = iv;

  if ( iv != NULL )
    {
      connect( d->iconView, SIGNAL( destroyed() ),
	       this,        SLOT(   iconViewDeleted() ) );
      setEnabled( true );
    }
  else
    setEnabled( false );
}

/******************************************************************************
 * Protected Methods                                                          *
 *****************************************************************************/
bool K3IconViewSearchLine::itemMatches( const Q3IconViewItem *item,
				       const QString &s ) const
{
  if ( s.isEmpty() )
    return true;

  if ( item == NULL )
    return false;

  return ( item->text().indexOf( s, 0,
	       caseSensitive()?Qt::CaseSensitive:Qt::CaseInsensitive ) >= 0 );
}

void K3IconViewSearchLine::init( Q3IconView *iconView )
{
  delete d;
  d = new K3IconViewSearchLinePrivate;

  d->iconView = iconView;

  connect( this, SIGNAL( textChanged( const QString & ) ),
	   this, SLOT(   queueSearch( const QString & ) ) );

  if ( iconView != NULL )
  {
      connect( iconView, SIGNAL( destroyed() ),
              this,     SLOT(   iconViewDeleted() ) );
      setEnabled( true );
  } else {
      setEnabled( false );
  }

  setClearButtonShown(true);
}

void K3IconViewSearchLine::hideItem( Q3IconViewItem *item )
{
  if ( ( item == NULL ) || ( d->iconView == NULL ) )
    return;

  d->hiddenItems.append( item );
  d->iconView->takeItem( item );
}

void K3IconViewSearchLine::showItem( Q3IconViewItem *item )
{
  if ( d->iconView == NULL )
    {
      kDebug() << __FILE__ << ":" << __LINE__ <<
	"showItem() could not be called while there's no iconView set." <<
	endl;
      return;
    }
  d->iconView->insertItem( item );
  d->hiddenItems.removeAll( item );
}

/******************************************************************************
 * Protected Slots                                                            *
 *****************************************************************************/
void K3IconViewSearchLine::queueSearch( const QString &s )
{
  d->queuedSearches++;
  d->search = s;
  QTimer::singleShot( 200, this, SLOT( activateSearch() ) );
}

void K3IconViewSearchLine::activateSearch()
{
  d->queuedSearches--;

  if ( d->queuedSearches <= 0 )
    {
      updateSearch( d->search );
      d->queuedSearches = 0;
    }
}

/******************************************************************************
 * Private Slots                                                              *
 *****************************************************************************/
void K3IconViewSearchLine::iconViewDeleted()
{
  d->iconView = NULL;
  setEnabled( false );
}

#include "k3iconviewsearchline.moc"
