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
// $Id$

#ifndef HTMLTOKEN_H
#define HTMLTOKEN_H


//
// External Classes
//
///////////////////

class JSEnvironment;
class KHTMLDecoder;
class KCharsets;

//
// Internal Classes
//
///////////////////

class StringTokenizer;
class HTMLTokenizer;

#include <qlist.h>
#include <qstrlist.h>
#include <qarray.h>
#include <qstring.h>

#include "khtmlstring.h"

int getTagID(const char *tagStr, int len);

// Every tag as deliverd by HTMLTokenizer starts with TAG_ESCAPE. This way
// you can devide between tags and words.
#define TAG_ESCAPE 0xE000
// this is only used iternally in the tokenizer. All entities are mapped
// to the corresponding unicode values in the tokenizer.
#define TAG_ENTITY 0xEFFF

// The count of spaces used for each tab.
#define TAB_SIZE 8

typedef QChar * TokenPtr;
//-----------------------------------------------------------------------------


class BlockingToken
{
public:
    BlockingToken( int id, TokenPtr t )
           {   tok_id = id; tok = t; }
                
    TokenPtr token()
           {   return tok; }
    int tokenId()
           {   return tok_id; }
                                
protected:
    int tok_id;
    TokenPtr tok;
};
                                        
//-----------------------------------------------------------------------------

class HTMLTokenBuffer
{
public:
	TokenPtr first() 
	{ return (TokenPtr) data; }
protected:
	QChar data[1];
};

class Attribute
{
public:
    Attribute() { id = 0, s = HTMLString(); }
    void setValue(QChar *_s, int _l) { 
	s = HTMLString(_s, _l);
    }
    HTMLString value() const { return s; }

    ushort id;
    HTMLString s;
};

// makes code in the parser nicer
inline bool operator==( const Attribute &a, const int &i )
{ return a.id == i; }

inline bool operator==( const Attribute &a, const QString &s )
{ return a.value() == s; }

//---------------------------------------------------------------------------

class HTMLTokenizer
{
public:
    HTMLTokenizer();
    ~HTMLTokenizer();

    void begin();
    void setPlainText();
    void write( const char * );
    void end();

    HTMLString nextToken();
    bool hasMoreTokens();

    const Attribute *nextOption();

    void first();

protected:
    void reset();
    void addPending();
    void appendToken( const QChar *t, int len );
    void appendTokenBuffer( int min_size);
    void nextTokenBuffer(); // Move curr to next tokenBuffer

    void addListing(HTMLString list);

    void parseComment(HTMLString &str);
    void parseText(HTMLString &str);
    void parseStyle(HTMLString &str);
    void parseScript(HTMLString &str);
    void parseListing(HTMLString &str);
    void parseTag(HTMLString &str);
    void parseEntity(HTMLString &str, bool start = false);

    // check if we have enough space in the buffer.
    // if not enlarge it
    void checkBuffer(int len = 1);
protected:
    // Internal buffers
    ///////////////////
    QChar *buffer;
    QChar *dest;

    // QChar -> char conversion cache
    char chr;

    // the size of buffer
    int size;

    // Token List
    /////////////
    QList<HTMLTokenBuffer> tokenBufferList;

    // decodes the input stream to unicode
    KHTMLDecoder *decoder;
    
    TokenPtr last;  // Last token appended

    TokenPtr next;  // Token written next
    int tokenBufferSizeRemaining; // The size remaining in the buffer written to

    TokenPtr curr;  // Token read next 
    unsigned int tokenBufferCurrIndex; // Index of HTMLTokenBuffer used by next read.

    QChar *nextOptionPtr;

    // Tokenizer flags
    //////////////////
    // are we in quotes within a html tag
    typedef enum
    {
        NoQuote = 0,
        SingleQuote,
        DoubleQuote
    } HTMLQuote;
        
    HTMLQuote tquote;
    
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
	LFDiscard
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


    typedef enum {
	NoTag = 0,
	TagName,
	SearchAttribute,
	AttributeName,
	SearchEqual,
	SearchValue,
	QuotedValue,
	Value,
	SearchEnd
    } HTMLTagParse;
    // Flag to say, we are just parsing a tag, meaning, we are in the middle
    // of <tab... 
    HTMLTagParse tag;

    // Flag to say that we are just parsing an attribute
    bool parseAttr;

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

    // Are we in a <listing> ... </listing> block
    bool listing;

    // Are we in lain textmode ?
    bool plaintext;
    
    // Are we in a &... character entity description?
    bool charEntity;

    // Area we in a <!-- comment --> block
    bool comment;

    // Are we in a <textarea> ... </textarea> block
    bool textarea;

    // Used to store the code of a srcipting sequence
    QChar *scriptCode;
    // Size of the script sequenze stored in @ref #scriptCode
    int scriptCodeSize;
    // Maximal size that can be stored in @ref #scriptCode
    int scriptCodeMaxSize;
    
    // Stores characters if we are scanning for a string like "</script>"
    QChar searchBuffer[ 10 ];
    // Counts where we are in the string we are scanning for 
    int searchCount;
    // The string we are searching for
    const QChar *searchFor;
    
    /**
     * This pointer is 0L until used. The @ref KHTMLWidget has an instance of
     * this class for us. We ask for it when we see some JavaScript stuff for
     * the first time.
     */
    JSEnvironment* jsEnvironment;
    
    // These are tokens for which we are awaiting ending tokens
    QList<BlockingToken> blocking;

    Attribute currAttr;

    QChar entityBuffer[10];
    uint entityPos;

    KCharsets *charsets;
};

//-----------------------------------------------------------------------------

class StringTokenizer
{
public:
    StringTokenizer();
    ~StringTokenizer();

    // FIXME 2nd argument is 0 terminated!!!
    void tokenize( HTMLString, const QChar * );
    HTMLString nextToken();
    bool hasMoreTokens() { return ( pos != 0 ); }

protected:
    QChar *pos;
    QChar *end;
    QChar *buffer;
    int  bufLen;
};

#endif // HTMLTOKEN

