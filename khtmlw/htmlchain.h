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

#ifndef __HTMLCHAIN_H__
#define __HTMLCHAIN_H__

class HTMLObject;

/*
 */
class HTMLChain
{
public:
    HTMLChain() : chain(30), curr(0), tail(0) {  }

    void first() { curr = 0; }
    void last()  { curr = tail; }
    void next()  { if (curr < tail) curr++; }
    void prev()  { if (curr > 0) curr--; }

    HTMLObject *current() { return chain[curr]; }

    void push( HTMLObject *obj ) { 
    	if (tail >= (int) chain.size()) 
    	{
    		chain.resize(chain.size()*2);
    	}
    	chain[tail] = obj;
    	tail++;
    }
    void pop() { if (tail > 0) tail--; }

protected:
	QArray<HTMLObject *> chain;
	int curr;
	int tail;
};

#endif

