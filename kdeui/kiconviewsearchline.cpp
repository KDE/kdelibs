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
 * is so close (it's actually based on) klistviewsearchline! Only few methods
 * would be reimplemented.
 */

#include "kiconviewsearchline.h"

#include <q3iconview.h>
#include <klocale.h>
#include <qtimer.h>
#include <kdebug.h>

#define DEFAULT_CASESENSITIVE false

typedef QList <Q3IconViewItem *> QIconViewItemList;

class KIconViewSearchLine::KIconViewSearchLinePrivate
{
public:
  KIconViewSearchLinePrivate() :
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
KIconViewSearchLine::KIconViewSearchLine( QWidget *parent,
					  Q3IconView *iconView ) :
  KLineEdit( parent )
{
  d = NULL;
  init( iconView );
}

KIconViewSearchLine::~KIconViewSearchLine()
{
  clear(); // empty hiddenItems, returning items back to iconView
  delete d;
}

bool KIconViewSearchLine::caseSensitive() const
{
  return d->caseSensitive;
}

Q3IconView *KIconViewSearchLine::iconView() const
{
  return d->iconView;
}

/******************************************************************************
 * Public Slots                                                               *
 *****************************************************************************/
void KIconViewSearchLine::updateSearch( const QString &s )
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

void KIconViewSearchLine::clear()
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
    kdDebug() << __FILE__ << ":" << __LINE__ <<
      "hiddenItems is not empty as it should be. " <<
      d->hiddenItems.count() << " items are still there.\n" << endl;

  d->search = "";
  d->queuedSearches = 0;
  KLineEdit::clear();
}

void KIconViewSearchLine::setCaseSensitive( bool cs )
{
  d->caseSensitive = cs;
}

void KIconViewSearchLine::setIconView( Q3IconView *iv )
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
bool KIconViewSearchLine::itemMatches( const Q3IconViewItem *item,
				       const QString &s ) const
{
  if ( s.isEmpty() )
    return true;

  if ( item == NULL )
    return false;

  return ( item->text().find( s, 0, caseSensitive() ) >= 0 );
}

void KIconViewSearchLine::init( Q3IconView *iconView )
{
  delete d;
  d = new KIconViewSearchLinePrivate;

  d->iconView = iconView;

  connect( this, SIGNAL( textChanged( const QString & ) ),
	   this, SLOT(   queueSearch( const QString & ) ) );

  if ( iconView != NULL )
    {
      connect( iconView, SIGNAL( destroyed() ),
	       this,     SLOT(   iconViewDeleted() ) );
      setEnabled( true );
    }
  else
    setEnabled( false );
}

void KIconViewSearchLine::hideItem( Q3IconViewItem *item )
{
  if ( ( item == NULL ) || ( d->iconView == NULL ) )
    return;

  d->hiddenItems.append( item );
  d->iconView->takeItem( item );
}

void KIconViewSearchLine::showItem( Q3IconViewItem *item )
{
  if ( d->iconView == NULL )
    {
      kdDebug() << __FILE__ << ":" << __LINE__ <<
	"showItem() could not be called while there's no iconView set." <<
	endl;
      return;
    }
  d->iconView->insertItem( item );
  d->hiddenItems.remove( item );
}

/******************************************************************************
 * Protected Slots                                                            *
 *****************************************************************************/
void KIconViewSearchLine::queueSearch( const QString &s )
{
  d->queuedSearches++;
  d->search = s;
  QTimer::singleShot( 200, this, SLOT( activateSearch() ) );
}

void KIconViewSearchLine::activateSearch()
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
void KIconViewSearchLine::iconViewDeleted()
{
  d->iconView = NULL;
  setEnabled( false );
}

#include "kiconviewsearchline.moc"
