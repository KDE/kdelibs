// -*- c++ -*-
/* This file is part of the KDE libraries
    Copyright (C) 1997 Stephan Kulow <coolo@kde.org>

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

#include <kdir.h>
#include <qpixmap.h>
#include "kfilesimpleview.h"
#include "qkeycode.h"
#include <qpainter.h>
#include <kapp.h>
#include "debug.h"

KFileSimpleView::KFileSimpleView(bool s, QDir::SortSpec sorting,
                                 QWidget *parent, const char *name)
    : QTableView(parent, name), KFileInfoContents(s,sorting), cellWidths(0)
{
    QWidget::setFocusPolicy(QWidget::StrongFocus);
    width_max = 100;
    width_length = 0;
    width_array = new uint[width_max];
    
    setLineWidth( 2 );
    setFrameStyle( Panel | Sunken );
    cellWidths = new int[1];
    cellWidths[0] = -1;
    rowsVisible = 1;

    setCellHeight( fontMetrics().lineSpacing() + 5);
    setCellWidth(0);
    setTableFlags(Tbl_autoHScrollBar |
		  Tbl_smoothHScrolling);
    curCol = curRow = 0;
    QTableView::setNumCols(0);
    QTableView::setNumRows(1);
    
    setBackgroundMode( PaletteBase );
    touched = false;
}

KFileSimpleView::~KFileSimpleView()
{
    delete [] cellWidths;
    delete [] width_array;
}

void KFileSimpleView::setNumCols(int count)
{
    debugC("setNumCols %d", count);
    delete [] cellWidths;
    cellWidths = new int[count];
    for (int i = 0; i < count; i++)
	cellWidths[i] = -1;
    QTableView::setNumCols(count);
}

void KFileSimpleView::setAutoUpdate(bool f)
{
    QTableView::setAutoUpdate(f);
}

void KFileSimpleView::highlightItem(unsigned int i)
{
    debugC("highlightItem %d", i);

    int col =  i / rowsVisible;
    highlightItem( i - rowsVisible * col, col );
}

void KFileSimpleView::highlightItem(int row, int col)
{ 
    debugC("highlightItem %d %d", row, col);

    if (col * rowsVisible + row  >= count())
	return;

    int cx;
    if (!colXPos ( col , &cx ))
	cx = 0;
    
    int edge = leftCell();              // find left edge
    if ( col < edge || cx < 0) {
	edge = leftCell() - curCol + col;
	if (edge < 0)
	    edge = 0;
	setLeftCell( edge );
    }

    edge = lastColVisible();
    if ( col >= edge )
	setLeftCell( leftCell() + col - edge + 1);

    edge = topCell();
    if ( row < edge )
	setTopCell( edge - 1 );

    edge = lastRowVisible();
    if ( row >= edge )
	setTopCell( topCell() + 1 );

    if (curCol != static_cast<int>(col) || 
	curRow != static_cast<int>(row)) 
    {
	int oldRow = curRow;
	int oldCol = curCol;
	curCol = col; curRow = row;
	updateCell( oldRow, oldCol);
	updateCell( row, col );
    }
}

void KFileSimpleView::clearView()
{
    setNumCols(1);
    pixmaps.clear();
    curCol = curRow = hasFocus() ? 0 : -1;
    width_length = 0;
}

void KFileSimpleView::paintCell( QPainter *p, int row, int col)
{
    uint index = col * rowsVisible + row;

    int w = cellWidth( col );                   // width of cell in pixels
    int h = cellHeight( row );                  // height of cell in pixels
    int x2 = w - 1;
    int y2 = h - 1;

    if ( (row == curRow) && (col == curCol) ) { // if we are on current cell,
        if ( hasFocus() ) {
              p->fillRect(0, 0, x2, y2, kapp->selectColor);
	      p->setPen( kapp->selectTextColor );
        }
        else { // no focus => draw only a dashed line around current item
             p->setPen( DotLine );               // used dashed line to
             p->drawRect( 0, 0, x2, y2 );        // draw rect along cell edges
             p->setPen( SolidLine );             // restore to normal
             p->setPen( kapp->windowTextColor );
        }
    } else // not on current cell, use the normal color
          p->setPen( kapp->windowTextColor );

    if (index < count()) {
	p->drawPixmap(0, 1, *pixmaps.at(index));
	p->drawText(3 + pixmaps.at(index)->width(), 15, text(index));
    }
}


void KFileSimpleView::keyPressEvent( QKeyEvent* e )
{
    int newRow = curRow;                        // store previous current cell
    int newCol = curCol;
    int oldRow = curRow;
    int oldCol = curCol;
    int lastItem = 0;
    int jump     = 0;

    switch( e->key() ) {                        // Look at the key code
    case Key_Left:
	if( newCol > 0 )
	    newCol--;
        else newRow = 0;
	break;
    case Key_Right:                         // Correspondingly...
	if( newCol < numCols()-1 )
	    newCol++;
        else newRow = lastRowVisible();
	if (newCol * rowsVisible + oldRow >= count())
	    newRow = count() - rowsVisible * newCol - 1;
	break;
    case Key_Up:
	if( newRow >= 0 ) {
	    newRow--;
	    if (newRow == -1)
		if (newCol == 0)
		    newRow = 0;
		else {
		    newRow = rowsVisible - 1;
		    newCol--;
		}
	}
	break;
    case Key_Down:
	if( newRow <= numRows()-1 ) {
	    newRow++;
	    if (newRow >= numRows()) {
		newRow = 0;
 	 	newCol++;
	    }
	}
	break;
    case Key_Home:
        newRow = 0;
        newCol = 0; 
        break;
    case Key_End:
        newRow = count() % numRows() - 1; // calc last item in last col
        newCol = numCols() - 1;
        break;
    case Key_PageUp:
        jump = lastColVisible() - leftCell(); // num of cols we want to jump
        if ( curCol - jump < 0 ) {
          newRow = 0;
          newCol = 0;
        } else newCol = curCol - jump;
        break;
    case Key_PageDown:
//        jump = lastColVisible() - curCol + 1; // num of cols we want to jump
          jump = lastColVisible() - leftCell();
        lastItem = count() % numRows() - 1;   // last item in last col

        if ( curCol + jump >= numCols() ) { // too far, just go to last col
          newCol = numCols() - 1;
          newRow = lastItem; 
        } else {
          newCol += jump; 
          if ( newCol == numCols()-1 && curRow > lastItem ) newRow = lastItem;
          else newRow = curRow;
        }
        break;
    case Key_Enter:
    case Key_Return:
	select( curCol * rowsVisible + curRow );
	return;
	break;
    default: 
	{
	    if ((e->ascii() >= 'a' && e->ascii() <= 'z') || 
		(e->ascii() >= 'A' && e->ascii() <= 'Z')) {
		char tmp[2] = " ";
		tmp[0] = e->ascii();
		QString res = findCompletion(tmp);
		if (!res.isNull())
		    debugC("found %s",res.data());
	    } else
		e->ignore();
	}
	return;     
    }
   
    highlightItem( newRow, newCol );
    
    if ( curRow != oldRow || curCol != oldCol )
	highlight( curRow + curCol * rowsVisible );
   
}


bool KFileSimpleView::insertItem(const KFileInfo *i, int index)
{
    if (numCols() * rowsVisible < count())
        setNumCols(numCols() + 1);
    
    if (i->isDir()) {
	if (i->isReadable())
	    pixmaps.insert(index, folder_pixmap);
	else
	    pixmaps.insert(index, locked_folder);	
    } else {
	if (i->isReadable())
	    pixmaps.insert(index, file_pixmap);
	else
	    pixmaps.insert(index, locked_file);
    }
    
    int curCol;
    if (rowsVisible > 0)
	curCol = index / rowsVisible;
    else
	curCol = 0;

    for (int j = curCol; j < numCols(); j++)
      cellWidths[ j ] = -1; // reset values

    uint size = fontMetrics().width( i->fileName() );
    insertArray(size, index);
    
    return colIsVisible(curCol) || curCol < leftCell() ;
}

int KFileSimpleView::cellWidth ( int col )
{
    if (cellWidths[col] == -1) {
	// debugC("not cached %d", col);
	int offset = col * rowsVisible;
	int width = 100;
	for (uint j = 0; offset + j < width_length && j < rowsVisible; j++) {
	    int w = width_array[offset + j];
	    if (width < w)
		width = w;
	}
	cellWidths[col] = width + file_pixmap->width() + 9; 
    }
    // debugC("cellWidth %d %d", col, cellWidths[col]);
    return cellWidths[col];
}

void KFileSimpleView::resizeEvent ( QResizeEvent *e )
{
    QTableView::resizeEvent(e);
    rowsVisible = lastRowVisible();
    
    int cols;
    if (rowsVisible > 0) {
	setNumRows(rowsVisible);
	cols = count() / rowsVisible + 1;
    } else {
	if (rowsVisible < 0)
	    rowsVisible = 0;
	cols = 0;
    }
    setNumCols(cols);
    QTableView::repaint(true);
}

void KFileSimpleView::mousePressEvent( QMouseEvent* e )
{
    int oldRow = curRow;                        // store previous current cell
    int oldCol = curCol;
    QPoint clickedPos = e->pos();               // extract pointer position
    curRow = findRow( clickedPos.y() );         // map to row; set current cell
    
    if (curRow > static_cast<int>(rowsVisible))
	curRow = rowsVisible;
    
    curCol = findCol( clickedPos.x() );         // map to col; set current cell

    uint index = curCol * rowsVisible + curRow;

    if ( index  >= count()) {
	curCol = oldCol;
	curRow = oldRow;
	return;
    }
    
    if ( (curRow != oldRow)                     // if current cell has moved,
         || (curCol != oldCol) ) {
        updateCell( oldRow, oldCol );           // erase previous marking
        updateCell( curRow, curCol );           // show new current cell
    }
    
    if ( useSingle() && isDir(index)) 
	select( index );
    else
	highlight( index );
}

void KFileSimpleView::mouseDoubleClickEvent ( QMouseEvent *e )
{
    int oldRow = curRow;                        
    int oldCol = curCol;
    QPoint clickedPos = e->pos();               
    curRow = findRow( clickedPos.y() );         
    
    if (curRow > static_cast<int>(rowsVisible))
	curRow = rowsVisible;
    
    curCol = findCol( clickedPos.x() );         
    
    uint index = curCol * rowsVisible + curRow;

    if ( index >= count()) {
	curCol = oldCol;
	curRow = oldRow;
	return;
    }
    
    if ( (curRow != oldRow)                     
         || (curCol != oldCol) ) {
        updateCell( oldRow, oldCol );
        updateCell( curRow, curCol );
    }
    select( index );
}
    
void KFileSimpleView::focusInEvent ( QFocusEvent * )
{
    if (curRow < 0 || curCol < 0)
	curRow = curCol = 0;
    updateCell( curRow, curCol );
}  

void KFileSimpleView::focusOutEvent ( QFocusEvent * ) 
{
    updateCell( curRow, curCol );  
}

void KFileSimpleView::insertArray(uint item, uint pos)
{
    //  debug("insert %s %d", item->fileName(), pos);
    if (width_length == width_max) {
	width_max *= 2;
	uint *newArray = new uint[width_max];
	int found = 0;
	for (uint j = 0; j < width_length; j++) {
	    if (j == pos) {
		found = 1;
		newArray[j] = item;
	    }
	    newArray[j+found] = width_array[j];
	}
	if (!found)
	    newArray[pos] = item;
	
	delete [] width_array;
	width_array = newArray;
	width_length++;
	return;
    }
    
    // faster repositioning (very fast :)
    memmove(width_array + pos+1,
	    width_array + pos,
	    (width_max - 1 - pos) * sizeof(uint));
    
    width_array[pos] = item;
    width_length++;
}

#include "kfilesimpleview.moc"

