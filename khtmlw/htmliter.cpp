/* This file is part of the KDE libraries
    Copyright (C) 1998 Martin Jones (mjones@kde.org)

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

#include "htmlobj.h"
#include "htmlclue.h"
#include "htmltable.h"
#include "htmliter.h"

//---------------------------------------------------------------------------

HTMLListIterator::HTMLListIterator( HTMLClue *c )
{
    clue = c;
    curr = clue->children();
    iter = 0;

    if ( curr )
	iter = curr->getIterator();
}

HTMLObject *HTMLListIterator::current()
{
    // if we have a child iterator then returns its current object
    if ( iter )
	return iter->current();

    return curr;
}

bool HTMLListIterator::next()
{
    if ( !curr )
	return false;

    if ( iter )
    {
	// tell the child iterator to move to the next object
	if ( iter->next() == false )
	{
	    delete iter;
	    iter = 0;
	}
	else
	{
	    return true;
	}
    }

    // move to the next object, skipping over empty child iterators.
    do
    {
	curr = curr->next();
	if ( curr )
	    iter = curr->getIterator();
    }
    while ( curr && iter && iter->current() == 0 );

    return ( curr != 0 );
}

//---------------------------------------------------------------------------

HTMLTableIterator::HTMLTableIterator( HTMLTable *t )
{
    row = 0;
    col = 0;
    curr = 0;
    table = t;
    iter = 0;

    // move to the next object, skipping over empty child iterators.
    do
    {
	curr = nextCell();
	if ( curr )
	    iter = curr->getIterator();
    }
    while ( curr && iter && iter->current() == 0 );
}

HTMLObject *HTMLTableIterator::current()
{
    if ( iter )
	return iter->current();

    return curr;
}

bool HTMLTableIterator::next()
{
    if ( !curr )
	return false;

    if ( iter )
    {
	if ( iter->next() == false )
	{
	    delete iter;
	    iter = 0;
	}
	else
	{
	    return true;
	}
    }

    do
    {
	curr = nextCell();
	if ( curr )
	    iter = curr->getIterator();
    }
    while ( curr && iter && iter->current() == 0 );

    return ( curr != 0 );
}

HTMLObject *HTMLTableIterator::nextCell()
{
    // If this is not the first time that nextCell has been called then
    // increment the column.
    if ( row != 0 || col != 0 || curr != 0 )
    {
	col++;
    }

    // find the next non-null cell
    while ( row < table->rows() )
    {
	for ( ; col < table->cols(); col++ )
	{
	    if ( ( curr = table->cell( row, col ) ) == 0 )
		continue;
	    if ( col < table->cols() - 1 && curr == table->cell( row, col+1 ) )
		continue;
	    if ( row < table->rows() - 1 && curr == table->cell( row+1, col ) )
		continue;
	    if ( curr )
		return curr;
	}
	row++;
	col = 0;
    }

    return 0;
}


