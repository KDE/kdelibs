/* This file is part of the KDE libraries
    Copyright (C) 1997 Martin Jones (mjones@kde.org)
              (C) 1997 Torben Weis (weis@kde.org)

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
//-----------------------------------------------------------------------------
//
// This is a modified HTMLTokenizer from khtmlw.
//

#ifndef BOOKTOKEN_H
#define BOOKTOKEN_H

#include <qlist.h>
#include <qstrlist.h>
#include <qarray.h>

// Every tag as deliverd by HTMLTokenizer starts with TAG_ESCAPE. This way
// you can devide between tags and words.
#define TAG_ESCAPE 13

// The count of spaces used for each tab.
#define TAB_SIZE 8



//-----------------------------------------------------------------------------

class BMToken
{
public:
    BMToken( const char *t, int len )
    {
	tok = new char [ len + 1 ];
	memcpy( tok, t, len+1 );
	nextToken = 0;
    }

    ~BMToken()
    {
	delete [] tok;
    }

    char *token()
	{ return tok; }

    BMToken *next()
	{ return nextToken; }
    void setNext( BMToken *n )
	{ nextToken = n; }

private:
    char *tok;
    BMToken *nextToken;
};

//-----------------------------------------------------------------------------

class BookmarkTokenizer
{
public:
    BookmarkTokenizer();
    ~BookmarkTokenizer();

    void begin();
    void write( const QString& );
    void end();

    char* nextToken();
    bool hasMoreTokens();

    void first()
	{ curr = head; }

protected:
    void reset();
    void appendToken( const char *t, int len )
    {
	if ( len < 1 )
	    return;

	BMToken *tok = new BMToken( t, len );

	if ( head )
	{
	    tail->setNext( tok );
	    tail = tok;
	}
	else
	{
	    head = tail = tok;
	}

	if ( !curr )
	    curr = tok;
    }
    
protected:
    char *buffer;
    char *dest;

    BMToken *head;
    BMToken *tail;

    BMToken *curr;
    
    // the size of buffer
    int size;
    
    // are we in a html tag
    bool tag;

    // are we in quotes within a html tag
    bool tquote;
    
    // To avoid multiple spaces
    bool space;

    // Discard line breaks immediately after tags
    bool discardCR;
    
    // Area we in a <!-- comment --> block
    bool comment;
};

//-----------------------------------------------------------------------------

#endif

