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
// KDE HTML Widget
//

#ifndef HTMLTOKEN_H
#define HTMLTOKEN_H

class StringTokenizer;
class HTMLTokenizer;

#include <qlist.h>
#include <qstrlist.h>
#include <qarray.h>

#include "jscript.h"

// Every tag as deliverd by HTMLTokenizer starts with TAG_ESCAPE. This way
// you can devide between tags and words.
#define TAG_ESCAPE 13

// The count of spaces used for each tab.
#define TAB_SIZE 8



//-----------------------------------------------------------------------------

class Token
{
public:
    Token( const char *t, int len )
    {
	tok = new char [ len + 1 ];
	strcpy( tok, t );
	nextToken = 0;
    }

    ~Token()
    {
	delete [] tok;
    }

    char *token()
	{ return tok; }

    Token *next()
	{ return nextToken; }
    void setNext( Token *n )
	{ nextToken = n; }

private:
    char *tok;
    Token *nextToken;
};

//-----------------------------------------------------------------------------

class BlockingToken
{
public:
    enum TokenType { Table, FrameSet, Script, Cell };

    BlockingToken( TokenType tt, Token *t )
	    {	ttype = tt; tok = t; }

    Token *token()
	    {	return tok; }
    const char *tokenName();

protected:
    TokenType ttype;
    Token *tok;
};

//-----------------------------------------------------------------------------

class HTMLTokenizer
{
public:
    HTMLTokenizer( KHTMLWidget *_widget = 0L );
    ~HTMLTokenizer();

    void begin();
    void write( const char * );
    void end();

    char* nextToken();
    bool hasMoreTokens();

    void first()
	{ curr = head; }

protected:
    void reset();
    void appendToken( const char *t, int len )
    {
	Token *tok = new Token( t, len );

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
//    QStrList tokenList;
    char *buffer;
    char *dest;

    Token *head;
    Token *tail;

    Token *curr;
    
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
    
    // Are we in a <pre> ... </pre> block
    bool pre;
    
    // Are we in a <script> ... </script> block
    bool script;
    
    // Are we in a <-- comment -->
    bool comment;

    // Used to store the code of a srcipting sequence
    char *scriptCode;
    // Size of the script sequenze stored in @ref #scriptCode
    int scriptCodeSize;
    // Maximal size that can be stored in @ref #scriptCode
    int scriptCodeMaxSize;
    
    // Used to store the string "</script>" for comparison
    const char *scriptString;
    // Stores characters if we are scanning for "</script>"
    char scriptBuffer[ 10 ];
    // Counts where we are in the string "</script>"
    int scriptCount;
    
    // Is TRUE if we are in a <script> tag and insideof '...' quotes
    bool squote;
    // Is TRUE if we are in a <script> tag and insideof "..." quotes
    bool dquote;

    KHTMLWidget *widget;
    
    /**
     * This pointer is 0L until used. The @ref KHTMLWidget has an instance of
     * this class for us. We ask for it when we see some JavaScript stuff for
     * the first time.
     */
    JSEnvironment* jsEnvironment;
    
    // These are tokens for which we are awaiting ending tokens
    QList<BlockingToken> blocking;
};

//-----------------------------------------------------------------------------

class StringTokenizer
{
public:
    StringTokenizer();
    ~StringTokenizer();

    void tokenize( const char *, const char * );
    const char* nextToken();
    bool hasMoreTokens() { return ( pos != 0 ); }

protected:
    char *pos;
    char *end;
    char *buffer;
    int  bufLen;
};

#endif // HTMLTOKEN

