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

#include "kdir.h"
#include "kfiledetaillist.h"
#include "qkeycode.h"
#include <kapp.h>
#include <klocale.h>
KFileDetailList::KFileDetailList(bool s, QDir::SortSpec sorting, 
				 QWidget *parent, const char *name)
    : KTabListBox(parent, name, 7), KFileInfoContents(s,sorting)
{
    QWidget::setFocusPolicy(QWidget::StrongFocus);
    setSeparator('\t');
    setColumn(0, "", file_pixmap->width() + 10, KTabListBox::PixmapColumn);
    QFontMetrics fm = fontMetrics();
    QString text = i18n("Name");
    setColumn(1, text, QMAX(fm.width(text + "_"), 150));
    text = i18n("Size");
    setColumn(2, text, QMAX(fm.width(text + "_"), 50));
    text = i18n("Permissions");
    setColumn(3, text, QMAX(fm.width(text + "_"), 80));
    text = i18n("Date");
    int _width = QMAX(fm.width(text + "_"), 50);
    setColumn(4, text, QMAX(_width, fm.width(KFileInfo::dateTime(0) + "_")));
    text = i18n("Owner");
    setColumn(5, text, QMAX(fm.width(text + "_"), 70));
    text = i18n("Group");
    setColumn(6, text, QMAX(fm.width(text + "_"), 70));
    
    dict().insert("file", file_pixmap);
    dict().insert("l_file", locked_file);
    dict().insert("folder", folder_pixmap);
    dict().insert("l_folder", locked_folder);

    setSortMode(Switching);
    //setSorting(QDir::Name);
    
    connect(this,SIGNAL(selected(int,int )), SLOT(selected(int)));
    connect(this, SIGNAL(highlighted(int,int)), SLOT(highlighted(int)));

    connect(this, SIGNAL(headerClicked(int)), SLOT(reorderFiles(int)));
}

void KFileDetailList::repaint(bool)
{
    KTabListBox::repaint();
}

void KFileDetailList::keyPressEvent( QKeyEvent *e)
{
    int oldRow = currentItem();
    int edge;
    int numVisible = lastRowVisible() - topCell();
    
    switch( e->key() ) {
    case Key_Up:
	if( oldRow > 0 ) {
	    KTabListBox::setCurrentItem(oldRow-1);
	    edge = topItem();
	    if ( currentItem() < edge )
		setTopItem( edge - 1 );
	}
	break;
    case Key_Down:
	if( oldRow < numRows()-1 ) {
	    KTabListBox::setCurrentItem(oldRow+1);
	    edge = lastRowVisible();
	    if ( currentItem() >= edge )
		setTopItem( topCell() + 1 );
	}
	break;          
    case Key_Enter:
    case Key_Return:
	select( currentItem() );
	break;
    case Key_PageUp:
	if ( oldRow > 0 ) {
          if ( oldRow >= numVisible ) {
              KTabListBox::setCurrentItem(oldRow - numVisible);
              edge = topCell() - numVisible;
              if ( currentItem() - edge >= numVisible )
                  edge++;
              if ( edge >= 0)
                  setTopItem(edge);
              else
                  setTopItem(0);
          } else {
              KTabListBox::setCurrentItem(0);
              setTopItem(0);
          }
        } 
	break;
    case Key_PageDown:
	if ( oldRow < numRows()-1 ) {
          if ( oldRow < numRows()-numVisible-1 ) {
              KTabListBox::setCurrentItem(oldRow + numVisible);
              edge = lastRowVisible();
              if ( currentItem()+numVisible-1 >=  edge )
                  setTopItem( topCell() + numVisible );
              else
                  setTopItem( numRows() - numVisible );
          } else {
              KTabListBox::setCurrentItem(numRows()-1);
              setTopItem( numRows() - numVisible );
          }

        } 
	break;
    default:       
	e->ignore(); 
	return;     
    }    
}

void KFileDetailList::focusInEvent ( QFocusEvent *)
{
    if (currentItem() < 0)
	KTabListBox::setCurrentItem(topItem());
}

KFileDetailList::~KFileDetailList()
{
}

void KFileDetailList::setAutoUpdate(bool f)
{
    KTabListBox::setAutoUpdate(f);
}

void KFileDetailList::highlightItem(unsigned int i)
{
    KTabListBox::setCurrentItem(i);
    unsigned j = (KTabListBox::lastRowVisible()-KTabListBox::topItem())/2;
    j = (j>i) ? 0 : (i-j);
    KTabListBox::setTopItem( j );
}

void KFileDetailList::clearView()
{
    KTabListBox::clear();
}


void KFileDetailList::reorderFiles(int inColumn)
{
    QDir::SortSpec new_sort;
    
    // QDir::SortSpec oldFlags = sorting() & (~QDir::SortByMask);

    switch ( inColumn ) {
    case 1:
	new_sort = QDir::Name;
	break;
    case 2:  
	new_sort = QDir::Size;
	break;
    case 4:  
	new_sort = QDir::Time;
	break;
    default: 
	return;
    }
    
    setSorting(new_sort);
}

bool KFileDetailList::insertItem(const KFileInfo *i, int index)
{
    const char *type;
    if (i->isDir())
	if (i->isReadable())
	    type = "folder";
	else
	    type = "l_folder";
    else
	if (i->isReadable())
	    type = "file";
	else
	    type = "l_file";
    
    QString item = QString("%1\t%2\t%3\t%4\t%5\t%6\t%7").
	arg(type).arg(i->fileName()).arg(i->size()).arg(i->access()).
	arg(i->date()).arg(i->owner()).arg(i->group());

    KTabListBox::insertItem(item, index);

    // TODO: find out, if a repaint is really necessary
    return true;
}

void KFileDetailList::selected(int row)
{
    select(row);
}

void KFileDetailList::highlighted(int row)
{
    highlight(row);
}


#include "kfiledetaillist.moc"

