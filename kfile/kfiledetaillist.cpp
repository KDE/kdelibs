// -*- c++ -*-
/* This file is part of the KDE libraries
    Copyright (C) 1998 Richard Moore <rich@kde.org>
                  1998 Stephan Kulow <coolo@kde.org>
                  1998 Daniel Grana <grana@ie.iwi.unibe.ch>

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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include "kfiledetaillist.h"
#include "kdir.h"
#include "qkeycode.h"
#include <kapp.h>
#include <klocale.h>
#include <qdir.h>
#include <qheader.h>

KFileDetailList::KFileDetailList(bool s, QDir::SortSpec sorting,
				 QWidget *parent, const char *name)
    : QListView(parent, name), KFileInfoContents(s,sorting)
{
    QWidget::setFocusPolicy(QWidget::StrongFocus);
    QFontMetrics fm = fontMetrics();
    QString text = i18n("Name");
    addColumn(text);
    text = i18n("Size");
    addColumn(text);
    text = i18n("Permissions");
    addColumn(text);
    text = i18n("Date");
    addColumn(text);
    text = i18n("Owner");
    addColumn(text);
    text = i18n("Group");
    addColumn(text);

    QListView::setSorting( 0 );

    // enable header-clicking (sorting) only for name, size and time columns
    QHeader *h = header();
    h->setClickEnabled( false, -1 );
    h->setClickEnabled( true, 0 );
    h->setClickEnabled( true, 1 );
    h->setClickEnabled( true, 3 );

    KFileInfoContents::setSortMode( KFileInfoContents::Switching );

    connect( this, SIGNAL(returnPressed(QListViewItem *)),
	     SLOT(selected(QListViewItem *)) );
    connect( this, SIGNAL(doubleClicked(QListViewItem *)),
	     SLOT(selected(QListViewItem *)) );
    connect( this, SIGNAL(clicked(QListViewItem *)),
	     SLOT(singleClicked(QListViewItem *)) );
    connect( this, SIGNAL(selectionChanged(QListViewItem *)),
	     this, SLOT(highlighted(QListViewItem *)) );
    connect( h, SIGNAL(sectionClicked(int)), SLOT(reorderFiles(int)));
}

void KFileDetailList::repaint(bool)
{
    QListView::repaint();
}

KFileDetailList::~KFileDetailList()
{
}

void KFileDetailList::setAutoUpdate(bool )
{
  // KTabListBox::setAutoUpdate(f);
}

void KFileDetailList::highlightItem(unsigned int i)
{
  QListViewItem *item = firstChild();
  while (i && item) { item = item->nextSibling(); i--; }
  QListView::setCurrentItem(item);
  ensureItemVisible(currentItem());
}

void KFileDetailList::clearView()
{
  mapper.clear();
  QListView::clear();
}


void KFileDetailList::reorderFiles(int inColumn)
{
    QDir::SortSpec new_sort;

    QDir::SortSpec oldFlags = (QDir::SortSpec) (sorting() & QDir::SortByMask);

    switch ( inColumn ) {
    case 0:
        new_sort = (QDir::SortSpec) (oldFlags | QDir::Name 
				     & ~(QDir::Size | QDir::Time));
	break;
    case 1:
        new_sort = (QDir::SortSpec) (oldFlags | QDir::Size
				     & ~(QDir::Name | QDir::Time));
	break;
    case 3:
        new_sort = (QDir::SortSpec) (oldFlags | QDir::Time
				     & ~(QDir::Name | QDir::Size));
	break;
    default:
	return;
    }

    QListView::setSorting( inColumn );
    KFileInfoContents::setSortMode( KFileInfoContents::Switching );
    KFileInfoContents::setSorting(new_sort);
}

bool KFileDetailList::insertItem(const KFileInfo *i, int index )
{
  QPixmap type;

  if (i->isDir())
    if (i->isReadable()) {
      type = *folder_pixmap;
    } else {
      type = *locked_folder;
    } else {
      if (i->isReadable()) {
	type = *file_pixmap;
      } else {
	type = *locked_file;
      }
    }

  KFileListViewItem *item = new KFileListViewItem(this,
						  i->fileName(),
						  QString::number(i->size()),
						  i->access(), i->date(),
						  i->owner(), i->group());
  item->setPixmap(0, type);
  mapper.insert( (KFileInfo*) i, item );
  updateItems( index ); // maybe delay this call thru a timer?

  // TODO: find out, if a repaint is really necessary
  return true;
}


// ugly (but necessary) hack: set the key for sorting for the items over and
// over again, because KFileInfoContents tells us: "insert this item at this
// position" - but QListView can only arrange items by a sorting key.
// So if KFileInfoContents wants us to insert an item at position 3, which is
// already occupied, well, all items above must be raised about one position
void KFileDetailList::updateItems( int index )
{
  int cnt = KFileInfoContents::count();
  for ( int i = index; i < cnt; i++ ) {
    KFileInfo *fi = (KFileInfo *) at( i );
    ItemMapper::Iterator it = mapper.find( fi );
    if ( it != mapper.end() ) {
      KFileListViewItem *item = (KFileListViewItem *) it.data();
      if ( item )
	item->setIndex( i );
    }
  }
}


void KFileDetailList::selected( QListViewItem *item )
{
  KFileInfo *fi = kfileInfoItem( item );
  if ( fi && fi->isDir() ) {
    select(fi);
    setContentsPos(0, 0); // scroll to top left
  }
}

void KFileDetailList::highlighted( QListViewItem *item )
{
  KFileInfo *fi = kfileInfoItem( item );
  if ( fi )
    highlight( fi );
}

void KFileDetailList::singleClicked( QListViewItem *item )
{
  if ( useSingle() )
     selected( item );
}


// helper function - maps from QListViewItem to KFileInfo
KFileInfo * KFileDetailList::kfileInfoItem( const QListViewItem *lvi )
{
  if ( !lvi )
    return 0L;

  int i;
  QListViewItem *item = firstChild();
  for (i = 0; i < childCount() && item; i++) {
    if (item == lvi)
      break;
    item = item->nextSibling();
  }

  return (KFileInfo *) KFileInfoContents::at( i );
}



// helper wrapper for correct sorting
KFileListViewItem::KFileListViewItem( QListView *parent, QString label1,
				      QString label2, QString label3,
				      QString label4, QString label5,
				      QString label6 )
  : QListViewItem( parent, label1, label2, label3, label4, label5, label6 )
{
  myKey = "";
}

void KFileListViewItem::setIndex( int i )
{
  // ARGHHH, hack alert: QListView sorts alphabetically, so a key
  // 11 would be smaller than 2
  // So we just fill a string with n+1 characters (A for now)
  // If anyone comes up with a better idea PLEASE FIX THIS!

  myKey.fill( 65, i+1 );
}


QString KFileListViewItem::key( int, bool ) const
{
  return myKey;
}


#include "kfiledetaillist.moc"
