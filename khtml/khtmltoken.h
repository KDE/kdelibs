/*
    This file is part of the KDE libraries

    Copyright (C) 1997 Martin Jones (mjones@kde.org)
              (C) 1997 Torben Weis (weis@kde.org)
              (C) 1998 Waldo Bastian (bastian@kde.org)

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
//----------------------------------------------------------------------------
//
// KDE HTML Widget -- Tokenizers
// $Id:  $

#ifndef HTMLTOKEN_H
#define HTMLTOKEN_H

class StringTokenizer;
class HTMLTokenizer;

#include <qlist.h>
#include <qstrlist.h>
#include <qarray.h>

#include "khtmljscript.h"

// Every tag as deliverd by HTMLTokenizer starts with TAG_ESCAPE. This way
// you can devide between tags and words.
#define TAG_ESCAPE 13

// The count of spaces used for each tab.
#define TAB_SIZE 8

typedef char * TokenPtr;

//-----------------------------------------------------------------------------

class BlockingToken
{
public:
    enum TokenType { Table, FrameSet, Script, Cell };

    BlockingToken( TokenType tt, TokenPtr t )
	    {	ttype = tt; tok = t; }

    TokenPtr token()
	    {	return tok; }
    const char *tokenName();

protected:
    TokenType ttype;
    TokenPtr tok;
};

//-----------------------------------------------------------------------------

class HTMLTokenBuffer
{
public:
	TokenPtr first() 
	{ return (TokenPtr) data; }
protected:
	char data[1];
};


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

    void first();

protected:
    void reset();
	void addPending();
    void appendToken( const char *t, int len );
    void appendTokenBuffer( int min_size);
    void nextTokenBuffer(); // Move curr to next tokenBuffer
    
protected:
    // Internal buffers
    ///////////////////
    char *buffer;
    char *dest;

    // the size of buffer
    int size;

    // Token List
    /////////////
	QList<HTMLTokenBuffer> tokenBufferList;
    
	TokenPtr last;  // Last token appended

    TokenPtr next;  // Token written next
    int tokenBufferSizeRemaining; // The size remaining in the buffer written to

    TokenPtr curr;  // Token read next 
    unsigned int tokenBufferCurrIndex; // Index of HTMLTokenBuffer used by next read.
    
    // Tokenizer flags
    //////////////////
    // are we in a html tag
    bool tag;

    // are we in quotes within a html tag
    bool tquote;
    
	typedef enum 
	{ 
		NonePending = 0, 
		SpacePending, 
		LFPending, 
		TabPending 
	} HTMLPendingType;

    // To avoid multiple spaces
    HTMLPendingType pending;

	typedef enum 
	{ 
		NoneDiscard = 0, 
		SpaceDiscard, 
		LFDiscard, 
	} HTMLDiscardType;

    // Discard line breaks immediately after start-tags
    // Discard spaces after '=' within tags
    HTMLDiscardType discard;

	 // Discard the LF part of CRLF sequence
    bool skipLF;

    // Flag to say that we have the '<' but not the character following it.
    // Used to decide whether we will get a <TAG> or </TAG>
    // In case of a </TAG> we ignore pending LFs.
    // In case of a <TAG> we add any pending LF as a space.
    // If the character following is not '/', 'a..z', 'A..Z' or '!' 
    // the tag is inserted as text
	bool startTag;

	// Are we in a <title> ... </title> block
	bool title;
    
    // Are we in a <pre> ... </pre> block
    bool pre;
    
    // if 'pre == true' we track in which column we are
    int prePos;
    
    // Are we in a <script> ... </script> block
    bool script;

    // Are we in a <style> ... </style> block
    bool style;

    // Are we in a <select> ... </select> block
    bool select;

	 // Are we in a &... character entity description?
	 bool charEntity;

    // Area we in a <!-- comment --> block
    bool comment;

    // Are we in a <textarea> ... </textarea> block
    bool textarea;

    // Used to store the code of a srcipting sequence
    char *scriptCode;
    // Size of the script sequenze stored in @ref #scriptCode
    int scriptCodeSize;
    // Maximal size that can be stored in @ref #scriptCode
    int scriptCodeMaxSize;
    
    // Stores characters if we are scanning for a string like "</script>"
    char searchBuffer[ 10 ];
    // Counts where we are in the string we are scanning for 
    int searchCount;
    // The string we are searching for
    const char *searchFor;
    
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

inline void HTMLTokenizer::appendToken( const char *t, int len )
{
    if ( len < 1 )
        return;

    if (len > tokenBufferSizeRemaining)
    {
       // We need a new buffer
       appendTokenBuffer( len);
    }

    last = next; // Last points to the start of the token we are going to append
    tokenBufferSizeRemaining -= len+1; // One for the null-termination
    while (len--)
    {
        *next++ = *t++;
    }
    *next++ = '\0';
}

inline char* HTMLTokenizer::nextToken()
{
    if (!curr)
        return NULL;

    char *t = (char *) curr;
    curr += strlen(curr)+1;

    if ((curr != next) && (*curr == '\0'))
    {
    	// End of HTMLTokenBuffer, go to next buffer.
	    nextTokenBuffer();
    }

    return t;
}

inline bool HTMLTokenizer::hasMoreTokens()
{
    if ( !blocking.isEmpty() &&
	    blocking.getFirst()->token() == curr )
	{
       	return false;
    }

    return ( ( curr != 0 ) && (curr != next) );
}

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

