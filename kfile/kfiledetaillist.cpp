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

    QListView::setSorting(-1);

    //    connect(this,SIGNAL(selectionChanged(QListViewItem*)),
    //	    SLOT(selected(QListViewItem*)));
    //    connect(this, SIGNAL(highlighted(int,int)), SLOT(highlighted(int)));
    connect( this, SIGNAL(returnPressed(QListViewItem *)), 
	     SLOT(selected(QListViewItem *)) );
    connect( this, SIGNAL(doubleClicked(QListViewItem *)),
	     SLOT(selected(QListViewItem *)) );
    connect( this, SIGNAL(clicked(QListViewItem *)),
	     SLOT(singleClicked(QListViewItem *)) );
    connect(header(), SIGNAL(sectionClicked(int)), SLOT(reorderFiles(int)));
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
  QListView::clear();
}


void KFileDetailList::reorderFiles(int inColumn)
{
    QDir::SortSpec new_sort;

    // QDir::SortSpec oldFlags = sorting() & (~QDir::SortByMask);

    switch ( inColumn ) {
    case 0:
	new_sort = QDir::Name;
	break;
    case 1:
	new_sort = QDir::Size;
	break;
    case 3:
	new_sort = QDir::Time;
	break;
    default:
	return;
    }

    KFileInfoContents::setSorting(new_sort);
}

bool KFileDetailList::insertItem(const KFileInfo *i, int /* index */)
{
  // WABA: Why is index not used?
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

  QListViewItem *item = new QListViewItem(this,
					  i->fileName(),
					  QString::number(i->size()),
					  i->access(), i->date(),
					  i->owner(), i->group());
  item->setPixmap(0, type);
// WABA: I was told the following call is redundant
//  QListView::insertItem(item);

  // TODO: find out, if a repaint is really necessary
  return true;
}

void KFileDetailList::selected(QListViewItem *s)
{
  QListViewItem *item = firstChild();
  int i;
  for (i = 0; i < childCount() && item; i++) {
    if (item == s)
      break;
    item = item->nextSibling();
  }
  select(i);
}

void KFileDetailList::highlighted(int row)
{
    highlight(row);
}


void KFileDetailList::singleClicked( QListViewItem *item )
{
  if ( useSingle() )
     selected( item );
}
    
    
#include "kfiledetaillist.moc"

