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

#ifndef __HTMLITER_H__
#define __HTMLITER_H__

class HTMLObject;
class HTMLClue;
class HTMLTable;

/*
 * This is the base class for iterators of the HTMLObject heirarchy.
 * Currently the iterators only iterarate through the displayed objects,
 * i.e. container objects such as HTMLClue & HTMLTable are not moved to
 * by next(), rather their children are traversed.
 */
class HTMLIterator
{
public:
    HTMLIterator() {}
    virtual ~HTMLIterator() {}

    /*
     * return a pointer to the current object
     */
    virtual HTMLObject *current() = 0;

    /*
     * move to the next object.
     * returns true if we are not already at the end of the list.
     */
    virtual bool next() = 0;
};

/*
 * Iterator for all HTMLClue derived container objects
 */
class HTMLListIterator : public HTMLIterator
{
public:
    HTMLListIterator( HTMLClue *c );
    virtual ~HTMLListIterator()
	{ if ( iter ) delete iter; }

    virtual bool next();
    virtual HTMLObject *current();

protected:
    HTMLClue *clue;
    HTMLObject *curr;
    HTMLIterator *iter;
};

/*
 * Iterator for HTMLTable container object
 */
class HTMLTableIterator : public HTMLIterator
{
public:
    HTMLTableIterator( HTMLTable *t );
    virtual ~HTMLTableIterator()
	{ if ( iter ) delete iter; }

    virtual bool next();
    virtual HTMLObject *current();

protected:
    HTMLObject *nextCell();

protected:
    int row;
    int col;
    HTMLTable *table;
    HTMLObject *curr;
    HTMLIterator *iter;
};


#endif

