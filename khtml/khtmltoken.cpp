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
// KDE HTML Widget - Tokenizers
// $Id$

//#define TOKEN_DEBUG


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "khtmltoken.h"
#include "khtmldecoder.h"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <strings.h>

#include "khtmltags.c"
#include "khtmlattrs.c"

// Include Java Script
#include <jsexec.h>
#include <kcharsets.h>
#include <kglobal.h>

// Token buffers are allocated in units of TOKEN_BUFFER_SIZE bytes.
#define TOKEN_BUFFER_SIZE ((128*1024)-1)

// String buffers are allocated in units of STRING_BUFFER_SIZE bytes.
#define STRING_BUFFER_SIZE ((128*1024)-1)

static const QChar commentStart [] = { '<','!','-','-' };
static const QChar scriptEnd [] = { '<','/','s','c','r','i','p','t','>' };
static const QChar styleEnd [] = { '<','/','s','t','y','l','e','>' };
static const QChar listingEnd [] = { '<','/','l','i','s','t','i','n','g','>' };


int getTagID(const char *tagStr, int len)
{
    const struct tags *tagPtr = findTag(tagStr, len);
    if (!tagPtr)
        return 0;
    
    return tagPtr->id;
}

//-----------------------------------------------------------------------------

HTMLTokenizer::HTMLTokenizer( )
{
    blocking.setAutoDelete( true );
    jsEnvironment = 0L;
    buffer = 0;
    scriptCode = 0;
    decoder = 0;
    charsets = KGlobal::charsets();

    reset();
}

void HTMLTokenizer::reset()
{
    while (!tokenBufferList.isEmpty())
    {
    	QChar *oldBuffer = (QChar *) tokenBufferList.take(0);
        delete [] oldBuffer;
    }

    last = 0;
    next = 0;
    curr = 0;
    tokenBufferSizeRemaining = 0; // No space allocated at all

    if ( buffer )
	delete [] buffer;
    buffer = 0;

    if ( scriptCode )
    	delete [] scriptCode;
    scriptCode = 0;

    if( decoder ) {
	delete decoder;
	decoder = 0;
    }

}

void HTMLTokenizer::begin()
{
    reset();
    blocking.clear();
    size = 1000;
    buffer = new QChar[ 1024 ];
    dest = buffer;
    tag = NoTag;
    pending = NonePending;
    discard = NoneDiscard;
    pre = false;
    prePos = 0;
    plaintext = 0;
    listing = false;
    script = false;
    style = false;
    skipLF = false;
    select = false;
    comment = false;
    textarea = false;
    startTag = false;
    tquote = NoQuote;
    searchCount = 0;
    title = false;
    charEntity = false;
    decoder = new KHTMLDecoder;

}

void HTMLTokenizer::addListing(HTMLString list)
{
    bool old_pre = pre;
    // This function adds the listing 'list' as
    // preformatted text-tokens to the token-collection
    // thereby converting TABs.
    pre = true;
    prePos = 0;

    while ( list.length() )
    {
	checkBuffer();
	
	if (skipLF && ( list[0] != QChar('\n') ))
	{
	    skipLF = false;
	}

	if (skipLF)
	{
            skipLF = false;
	    ++list;
	} 
	else if (( list[0] == QChar('\n') ) || ( list[0] == QChar('\r') ))
	{
	    if (discard == LFDiscard) 
	    {
	        // Ignore this LF
	    	discard = NoneDiscard; // We have discarded 1 LF
	    }
	    else
	    {
	        // Process this LF
	        if (pending)
	            addPending();
	        pending = LFPending;
	    }
	    /* Check for MS-DOS CRLF sequence */
	    if (list[0] == QChar('\r'))
	    {
		skipLF = true;
	    }
	    ++list;
	}
	else if (( list[0] == QChar(' ') ) || ( list[0] == QChar('\t')))
	{
	    if (pending)
	        addPending();
	    if (list[0] == QChar(' '))
	        pending = SpacePending;
	    else 
	        pending = TabPending;
	    ++list;
	}
	else
	{
	    discard = NoneDiscard;
	    if (pending)
	    	addPending();

	    prePos++;
	    *dest++ = list[0];
	    ++list;
	}

    }

    if ((pending == SpacePending) || (pending == TabPending))
    {
	addPending();
    }
    pending = NonePending;
    
    if ( dest > buffer )
    {
        *dest = 0;
        appendToken( buffer, dest-buffer );
        dest = buffer;
        prePos = 0;
    }

    // Add </listing> tag
    
    *dest = QChar(TAG_ESCAPE);
    dest++;
    *dest = QChar(ID_LISTING + ID_CLOSE_TAG);
    dest++;
    *dest = 0;
    appendToken( buffer, dest-buffer );
    dest = buffer;
    
    pre = old_pre;    
}

void HTMLTokenizer::parseListing( HTMLString &src)
{
    // We are inside of the <script> or <style> tag. Look for the end tag
    // which is either </script> or </style>,
    // otherwise print out every received character

    while ( src.length() )
    {
	// do we need to enlarge the buffer?
	checkBuffer();

        // Allocate memory to store the script. We will write maximal
        // 10 characers.
        if ( scriptCodeSize + 10 > scriptCodeMaxSize )
        {
            QChar *newbuf = new QChar [ scriptCodeSize + 1024 ];
	    memcpy( newbuf, scriptCode, scriptCodeSize );
	    delete [] scriptCode;
	    scriptCode = newbuf;
	    scriptCodeMaxSize += 1024;
        }

	if ( ( src[0] == QChar('>') ) && ( searchFor[ searchCount ] == QChar('>')))
        {
	    ++src;
	    scriptCode[ scriptCodeSize ] = 0;
	    scriptCode[ scriptCodeSize + 1 ] = 0;
	    if (script) 
	    {
	        /* Parse scriptCode containing <script> info */
	        /* Not implemented */
	    }
	    else if (style)
	    {
	        /* Parse scriptCode containing <style> info */
	        /* Not implemented */
	    }
	    else
	    {
	        //
	        // Add scriptcode to the buffer
	        addListing(HTMLString(scriptCode, scriptCodeSize));
	    }
            script = style = listing = false;
	    delete [] scriptCode;
	    scriptCode = 0;
	    return; // Finished parsing script/style/listing	    
        }
        // Find out wether we see a </script> tag without looking at
        // any other then the current character, since further characters
        // may still be on their way thru the web!
        else if ( searchCount > 0 )
        {
	    QChar cmp = src[0];
	    if ( cmp.lower() == searchFor[ searchCount ] )
	    {
	        searchBuffer[ searchCount ] = src[0];
	        searchCount++;
	        ++src;
	    }
	    // We were wrong => print all buffered characters and the current one;
	    else
	    {
	        searchBuffer[ searchCount ] = 0;
	        QChar *p = searchBuffer;
	        while ( *p ) scriptCode[ scriptCodeSize++ ] = *p++;
	        scriptCode[ scriptCodeSize++ ] = src[0];
		++src;
	        searchCount = 0;
	    }
        }
        // Is this perhaps the start of the </script> or </style> tag?
        else if ( src[0] == QChar('<') )
        {
	    searchCount = 1;
	    searchBuffer[ 0 ] = QChar('<');
	    ++src;
        }
        else
        {
	    scriptCode[ scriptCodeSize++ ] = src[0];
	    ++src;
	}
    }
}

void HTMLTokenizer::parseScript(HTMLString &src)
{
    parseListing(src);
}
void HTMLTokenizer::parseStyle(HTMLString &src)
{
    parseListing(src);
}

void HTMLTokenizer::parseComment(HTMLString &src)
{
    while ( src.length() )
    {
	// do we need to enlarge the buffer?
	checkBuffer();

	// Look for '-->'
	if ( src[0] == QChar('-') ) 
	{
	    if (searchCount < 2)	// Watch out for '--->'
	        searchCount++;
	}
	else if ((searchCount == 2) && (src[0] == QChar('>')))
	{
	    // We got a '-->' sequence
	    comment = false;
	    ++src;
	    return; // Finished parsing comment!
	}
	else
	{
	    searchCount = 0;
	}
        ++src;
    }
}

void HTMLTokenizer::parseText(HTMLString &src)
{
    while ( src.length() )
    {
	// do we need to enlarge the buffer?
	checkBuffer();

	if (skipLF && ( src[0] != QChar('\n') ))
	{
	    skipLF = false;
	}

	if (skipLF)
	{
            skipLF = false;
	    ++src;
	} 
	else if (( src[0] == QChar('\n') ) || ( src[0] == QChar('\r') ))
	{
	    if ( dest > buffer )
	    {
		*dest = 0;
		appendToken( buffer, dest-buffer );
	    }
	    dest = buffer;

	    /* Check for MS-DOS CRLF sequence */
	    if (src[0] == QChar('\r'))
	    {
		skipLF = true;
	    }
	    ++src;
	}
        else
        {
	    *dest++ = src[0];
	    ++src;
        }
    }
}

void HTMLTokenizer::parseEntity(HTMLString &src, bool start)
{
    if( start ) 
    {
	entityPos = 0;
	charEntity = true;
    }

    while( src.length() )
    {
	if(entityPos > 8) {
	    checkBuffer(10);
	    // entity too long, ignore and insert as is
	    *dest++ = QChar('&');
	    memcpy(dest, entityBuffer, entityPos*sizeof(QChar));
	    dest += entityPos;
	    if ( pre )
		prePos += entityPos+1;
	    charEntity = false;
	    return;
	}
	if( (src[0].lower() >= QChar('a') && src[0].lower() <= QChar('z')) || 
	    (src[0] >= QChar('0') && src[0] <= QChar('9')) ||
	     src[0] == QChar('#') ) 
	{
	    entityBuffer[entityPos] = src[0];
	    entityPos++;
	    ++src;
	}
	else // end of entity... try to decode it 
	{
	    QConstString cStr(entityBuffer, entityPos);
	    QChar res = charsets->fromEntity(cStr.string());
	    
	    if (tag && src[0] != QChar(';') ) {
		// Don't translate entities in tags with a missing ';'
		res = QChar::null;
	    }
    
	    if ( res != QChar::null ) {
		checkBuffer();
		// Just insert it
		*dest++ = res;
		if (pre)
		    prePos++;
		if (src[0] == QChar(';'))
		    ++src;
	    } else {
		printf("unknown entity!\n");

		checkBuffer(10);
		// ignore the sequence, add it to the buffer as plaintext
		*dest++ = QChar('&');
		memcpy(dest, entityBuffer, entityPos*sizeof(QChar));
		dest += entityPos;
		charEntity = false;
		if (pre)
		    prePos += entityPos+1;
	    }
	    charEntity = false;
	    return;
	}
    }
}

void HTMLTokenizer::parseTag(HTMLString &src)
{
    if (charEntity)
        parseEntity(src);

    while ( src.length() )
    {
	checkBuffer();

	// decide if quoted or not....
	if ( src[0] == QChar('\"') || src[0] == QChar('\'') )
	{ // we treat " & ' the same in tags
	    if ( !tquote )
	    {
		// according to HTML4 DTD, we can simplify
		// strings like "  my \nstring " to "my string"
		discard = SpaceDiscard; // ignore leading spaces
		pending = NonePending;
		if (src[0] == QChar('\''))
		    tquote = SingleQuote;
		else
		    tquote = DoubleQuote;
	    }
	    else if ( (( tquote == SingleQuote )&&( src[0] == QChar('\''))) ||
		      (( tquote == DoubleQuote )&&( src[0] == QChar('\"'))) )
	    {
		tquote = NoQuote;
		pending = NonePending; // remove space at the end of value
	    }
	    else
	    {
		*dest++ = src[0];
	    }
	    ++src;
	} 
	else if ( discard != NoneDiscard &&
		  ( src[0] == QChar(' ') || src[0] == QChar('\t') ||
		    src[0] == QChar('\n') || src[0] == QChar('\r') ) )
	{
	    pending = SpacePending;
	    ++src;
	}
	else
	{
	    switch(tag) {
	    case NoTag:
	    {
		return;
	    }
	    case TagName:
	    {
		if( tquote ) 
		{
		    printf("bad HTML in parseTag: TagName\n");
		    searchCount = 0;
		    ++src;
		    break;
		}
		if (searchCount > 0)
		{
		    if (src[0] == commentStart[searchCount])
		    {
			searchCount++;
			if (searchCount == 4)
			{
			    // Found '<!--' sequence
			    comment = true;
			    dest = buffer; // ignore the previous part of this tag
			    tag = NoTag;
			    searchCount = 0;
			    parseComment(src);
			    return; // Finished parsing tag!
			}
		    }
		    else
		    {
			searchCount = 0; // Stop looking for '<!--' sequence
		    }  
		}
		if( ((src[0].lower() >= QChar('a')) && (src[0].lower() <= QChar('z'))) ||
		    ((src[0] >= QChar('0')) && (src[0] <= QChar('9'))) ||
		      src[0] == QChar('/') ) 
		{
		    *dest = src[0].lower();
		    dest++;
		    ++src;
		} 
		else 
		{
		    int len;
		    QChar *ptr = buffer+1;
		    if (*ptr == QChar('/')) 
		    { 
			// End Tag
			startTag = false;
			ptr++;
			len = dest - buffer - 2;
		    } 
		    else 
		    {
			// Start Tag
			startTag = true;
			// Ignore CR/LF's after a start tag
			discard = LFDiscard;
			len = dest - buffer - 1;
		    }
		    
		    QConstString tmp(ptr, len); 
		    const struct tags *tagPtr = findTag(tmp.string().ascii(), len);
		    if (!tagPtr) {
			printf("Unknown tag: \"%s\"\n", tmp.string().ascii());
			dest = buffer;
			tag = SearchEnd; // ignore the tag
		    }
		    else
		    {
			dest = buffer + 1;
			uint tagID = tagPtr->id;
#ifdef TOKEN_DEBUG
			printf("found tag id=%d\n", tagID);
#endif
			if (startTag)
			    *dest = QChar(tagID);
			else
			    *dest = QChar(tagID + ID_CLOSE_TAG);
			
			dest++;
			*dest++ = TAG_ESCAPE;
			tag = SearchAttribute;
		    }
		}
		break;
	    }
	    case SearchAttribute:
	    {
		if( tquote )
		{
		    printf("broken HTML in parseTag: SearchAttribute \n");
		    ++src;
		    break;
		}		
		if( src[0] == QChar('>') )
		{
		    tag = SearchEnd; // we reached the end
		    break;
		}
		if( src[0].row() ) // we ignore everything that isn't ascii
		{
		    ++src;
		    break;
		}
		if( ((src[0].lower() >= QChar('a')) && (src[0].lower() <= QChar('z'))) ||
		    ((src[0] >= QChar('0')) && (src[0] <= QChar('9'))) ||
		      src[0] == QChar('-') )
		{
		    tag = AttributeName;
		    discard = NoneDiscard;
		    break;
		}
		++src; // ignore
		break;
	    }
	    case AttributeName:
	    {
		if( (((src[0].lower() >= QChar('a')) && (src[0].lower() <= QChar('z'))) ||
		    ((src[0] >= QChar('0')) && (src[0] <= QChar('9'))) ||
		    src[0] == QChar('-')) && !tquote ) 
		{
		    *dest = src[0].lower();
		    dest++;
		    ++src;
		} 
		else 
		{
		    // find beginning
		    int len = 0;
		    QChar *ptr = dest-1;
		    while(*ptr != QChar(TAG_ESCAPE) ) ptr--, len++;
		    
		    QConstString tmp(ptr+1, len); 
		    const struct attrs *a = findAttr(tmp.string().ascii(),len);
		    if (!a) {
			printf("Unknown attribute: \"%s\"\n", 
			       tmp.string().ascii());
			dest = ptr + 1; // unknown attribute, ignore
			tag = SearchAttribute; // go to next attribute
		    } 
		    else
		    {
#ifdef TOKEN_DEBUG
			printf("Known attribute: \"%s\"\n", 
			       tmp.string().ascii());
#endif
			dest = ptr + 1;
			*dest++ = a->id;
			
			tag = SearchEqual;
		    }		
		}
		break;
	    }
	    case SearchEqual:
	    {
		if(tquote)
		{
		    printf("bad HTML in parseTag: SearchEqual\n");
		    ++src;
		    break;
		}
		if( src[0] == QChar('=') )
		{
		    tag = SearchValue;
		    pending = NonePending; // ignore spaces before '='
		    discard = SpaceDiscard; // discard spaces after '='
		    ++src;
		}
		else if( src[0] == QChar('>') )
		    tag = SearchEnd;
		else // other chars indicate a new attribte
		{
		    *dest++ = QChar(TAG_ESCAPE);
		    tag = SearchAttribute;
		    discard = SpaceDiscard;
		    pending = NonePending;
		}
		break;
	    }
	    case SearchValue:
	    {
		if(tquote)
		{
		    tag = QuotedValue;
		}
		else
		{
		    tag = Value;
		}
		pending = NonePending;
		discard = SpaceDiscard;
		break;

	    case QuotedValue:
	    {
		if ( src[0] == QChar('&') ) 
		{
		    ++src;
		    
		    discard = NoneDiscard; 
		    if (pending)
			addPending();
		    
		    charEntity = true;
		    parseEntity(src, true);
                    break;
		}
		else if ( !tquote )
		{
		    // end of attribute
		    *dest++ = QChar(TAG_ESCAPE);
		    tag = SearchAttribute;
		    discard = SpaceDiscard;
		    pending = NonePending;
		    break;
		}
		if( pending ) addPending();
		
		discard = NoneDiscard;
		*dest++ = src[0];
		++src;
		break;
	    }
	    case Value:
		if( tquote )
		{
		    printf("bad HTML in parseTag: Value\n");
		    ++src;
		    break;
		}
		else if ( pending || src[0] == QChar('>') )
		{
		    // no quotes. Every space means end of value
		    *dest++ = QChar(TAG_ESCAPE);
		    tag = SearchAttribute;
		    discard = SpaceDiscard;
		    pending = NonePending;
		    break;
		}
		*dest++ = src[0];
		++src;
		break;
	    }
	    case SearchEnd:
	    {
		if ( tquote || src[0] != QChar('>'))
		{
		    ++src; // discard everything, until we found the end
		    break;
		}
		
		searchCount = 0; // Stop looking for '<!--' sequence
		tag = NoTag;
		pending = NonePending; // Ignore pending spaces
		++src;

		if ( dest == buffer ) //stop if tag is unknown
		{
		  discard = NoneDiscard;
		  *dest = QChar::null;
		  return;
		}

		if( *(dest-1) == QChar(TAG_ESCAPE) ) dest--; 

		uint tagID = (buffer+1)->unicode();
#ifdef TOKEN_DEBUG
		printf("appending Tag: %d, len = %d\n", tagID, dest-buffer);
#endif
		if(startTag) 
                {
		   // Ignore CR/LF's after a start tag
		   discard = LFDiscard;
                }
                else
                {
		   // Don't ignore CR/LF's after a close tag
		   discard = NoneDiscard;
                   tagID -= ID_CLOSE_TAG;
		}

		*dest = QChar::null;
		appendToken( buffer, dest-buffer );
		dest = buffer;
		
		if ( tagID == ID_PRE )
		{
		    prePos = 0;
		    pre = startTag;
		}
		else if ( tagID == ID_TEXTAREA )
		{
		    textarea = startTag;
		}
		else if ( tagID == ID_TITLE )
		{
		    title = startTag;
		}
		else if ( tagID == ID_SCRIPT )
		{
		    if (startTag)
		    {
			script = true;
			searchCount = 0;
			searchFor = scriptEnd;		
			scriptCode = new QChar[ 1024 ];
			scriptCodeSize = 0;
			scriptCodeMaxSize = 1024;
			parseScript(src);
		    }
		}
		else if ( tagID == ID_STYLE )
		{
		    if (startTag)
		    {
			style = true;
			searchCount = 0;		
			searchFor = styleEnd;		
			scriptCode = new QChar[ 1024 ];
			scriptCodeSize = 0;
			scriptCodeMaxSize = 1024;
			parseStyle(src);
		    }
		}
		else if ( tagID == ID_LISTING )
		{
		    if (startTag)
		    {
			listing = true;
			searchCount = 0;		
			searchFor = listingEnd;		
			scriptCode = new QChar[ 1024 ];
			scriptCodeSize = 0;
			scriptCodeMaxSize = 1024;
			parseListing(src);
		    }
		}
		else if ( tagID == ID_SELECT )
		{
		    select = startTag;
		}
		else if (( tagID == ID_FRAMESET ) ||
			 ( tagID == ID_TABLE ))
		{
		    if (startTag)
		    {
			blocking.append( new BlockingToken(tagID, last) );
#ifdef TOKEN_DEBUG
			printf("appending blockingToken\n");
#endif
		    }
		    else
		    {
			if ( !blocking.isEmpty() &&  
			     (blocking.getLast()->tokenId() == (int)tagID) )
			{
#ifdef TOKEN_DEBUG
			    printf("removing blockingToken\n");
#endif
			    blocking.removeLast();
			}
#ifdef TOKEN_DEBUG
			else 
			    printf("unmatched blockingTocken\n");
#endif
		    }
		}
		return; // Finished parsing tag!
	    }
	    default:
	    {
		printf("error in parseTag! %d\n", __LINE__);
		return;
	    }
	    
	    } // end switch
	}
    }
    return;
}

void HTMLTokenizer::addPending()
{
    if ( tag || select)     
    {
    	*dest++ = QChar(' ');
    }
    else if ( textarea )
    {
	if (pending == LFPending)
	    *dest++ = QChar('\n');
	else
	    *dest++ = QChar(' ');    	
    }
    else if ( pre )
    {
    	int p;

	switch (pending)
	{
	case SpacePending:
	    // Insert a non-breaking space
	    *dest++ = QChar(0xa0); 
	    prePos++;
	    break;

	case LFPending:
	    if ( dest > buffer )
	    {
		*dest = 0;
		appendToken( buffer, dest-buffer );
	    }
	    dest = buffer;
	    *dest++ = QChar(TAG_ESCAPE);
	    *dest = QChar(ID_NEWLINE);
	    dest++;
	    *dest = 0;
	    appendToken( buffer, 2 );
	    dest = buffer;
	    prePos = 0; 
	    break;
	  	
	case TabPending:
	    p = TAB_SIZE - ( prePos % TAB_SIZE );
	    for ( int x = 0; x < p; x++ )
	    {
		*dest = QChar(' ');
		dest++;
	    }
	    prePos += p;
	    break;
	  	
	default:
	    printf("Assertion failed: pending = %d\n", (int) pending);
	    break;
	}
    }
    else
    {
    	*dest++ = QChar(' ');
    }

    pending = NonePending;
}

void HTMLTokenizer::setPlainText()
{
    if (!plaintext)
    {
       // Do this only once!
       plaintext = true;    
       dest = buffer;
       *dest++ = QChar(TAG_ESCAPE);
       *dest = QChar(ID_PLAIN);
       dest++;
       *dest = 0;
       appendToken( buffer, 2 );
       dest = buffer;
    }
}

void HTMLTokenizer::write( const char *str)
{
    // If this pointer is not 0L then we allocated some memory to store HTML
    // code in. This may happen while parsing the <script> tag, since the output
    // of the java code is treated as HTML code. This means we have to modify
    // the HTML code on the fly by inserting new HTML stuff.
    // If this pointer is not null, one has to free the memory before leaving
    // this function.
    QChar *srcPtr = 0L;

    if ( str == 0L || buffer == 0L )
	return;

    QString _src = decoder->decode(str);
    HTMLString src = HTMLString(_src);

    if (plaintext)
        parseText(src);
    else if (comment)
        parseComment(src);
    else if (script)
        parseScript(src);
    else if (style)
        parseStyle(src);
    else if (listing)
        parseListing(src);
    else if (tag)
    {
        parseTag(src);
	startTag = false;
	searchCount = 0;
    }
    else if (charEntity)
        parseEntity(src);

    while ( src.length() )
    {
	// do we need to enlarge the buffer?
	checkBuffer();

	if (skipLF && (src[0] != QChar('\n')))
	{
	    skipLF = false;
	}
	if (skipLF)
	{
            skipLF = false;
	    ++src;
	} 
	else if ( startTag )
	{
	    startTag = false;
	    if (src[0] == QChar('/')) 
	    {
		// Start of an End-Tag 
		if (pending == LFPending)
		    pending = NonePending; // Ignore leading LFs
	    }
	    else if ( (src[0].lower() >= QChar('a')) && (src[0].lower() <= QChar('z'))) 
	    {
		// Start of a Start-Tag
	    }
	    else if ( src[0] == QChar('!'))
	    {
		// <!-- comment -->
	    }
	    else
	    {
		// Invalid tag
		// Add as is
		if (pending)
		    addPending();
		*dest = QChar('<');
		dest++;
		*dest++ = src[0];
		++src;
		continue;	        
	    }

            if (pending)
	        addPending();

	    if ( dest > buffer )
	    {
		*dest = 0;
		appendToken( buffer, dest-buffer );
		dest = buffer;
	    }
	    *dest = TAG_ESCAPE;
	    dest++;
	    searchCount = 1; // Look for '<!--' sequence to start comment
	    tag = TagName;
	    parseTag(src);
	    startTag = false;
	    searchCount = 0;
	}
	else if ( src[0] == QChar('&') ) 
	{
            ++src;
	    
	    discard = NoneDiscard; 
	    if (pending)
	    	addPending();
	    
	    charEntity = true;
            parseEntity(src, true);
	}
	else if ( src[0] == QChar('<'))
	{
	    ++src;
	    startTag = true;
	    discard = NoneDiscard;
	}
	else if (( src[0] == QChar('\n') ) || ( src[0] == QChar('\r') ))
	{
	    if ( pre || textarea)
	    {
	    	if (discard == LFDiscard) 
	    	{
		    // Ignore this LF
	    	    discard = NoneDiscard; // We have discarded 1 LF
	    	}
	    	else
	    	{
	    	    // Process this LF
	    	    if (pending)
	    	        addPending();
		    pending = LFPending;
		}
	    } 
	    else
	    {
	    	if (discard == LFDiscard) 
	    	{
		    // Ignore this LF
	    	    discard = NoneDiscard; // We have discarded 1 LF
	    	}
	    	else
	    	{
	    	    // Process this LF
	    	    if (pending == NonePending)
			pending = LFPending;
		}
	    }
	    /* Check for MS-DOS CRLF sequence */
	    if (src[0] == QChar('\r'))
	    {
		skipLF = true;
	    }
	    ++src;
	}
	else if (( src[0] == QChar(' ') ) || ( src[0] == QChar('\t') ))
	{
	    if ( pre || textarea)
	    {
	    	if (pending)
	    	    addPending();
	    	if (src[0] == QChar(' '))
	    	    pending = SpacePending;
	    	else 
	    	    pending = TabPending;
	    } 
	    else
	    {
	    	pending = SpacePending;
	    }
	    ++src;
	}
	else
	{
	    discard = NoneDiscard;
	    if (pending)
	    	addPending();

	    if ( pre )
	    {
		prePos++;
	    }
	    
	    *dest++ = src[0];
	    ++src;
	}
    }

    if ( srcPtr )
	delete [] srcPtr;
}

void HTMLTokenizer::end()
{
    if ( buffer == 0 )
	return;

    if ( dest > buffer )
    {
	*dest = 0;
	appendToken( buffer, dest-buffer );
    }

    delete [] buffer;
    buffer = 0;

    // if there are still blocking tokens then the HTML is illegal - remove
    // blocks anyway and hope for the best
    blocking.clear();
}

void HTMLTokenizer::appendToken( const QChar *t, int len )
{
    if ( len < 1 )
        return;

    if (len >= tokenBufferSizeRemaining)
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
    *next++ = QChar('\0');
}


void HTMLTokenizer::appendTokenBuffer( int min_size)
{
    int newBufSize = TOKEN_BUFFER_SIZE; 

    // If we were using a buffer, mark it's end
    if (next)
    {
	// Mark current buffer end
	*next = QChar::null;
    }

    if (min_size > newBufSize)
    {
        // Wow! This surely is a big token...
        newBufSize += min_size; 
    }
    HTMLTokenBuffer *newBuffer = (HTMLTokenBuffer *) new QChar [ newBufSize + 1];
    tokenBufferList.append( newBuffer);
    next = newBuffer->first();
    tokenBufferSizeRemaining = newBufSize;
    if (!curr)
    {
	curr = tokenBufferList.at(0)->first();
	tokenBufferCurrIndex = 0;
    }
}                                                                                         

void HTMLTokenizer::nextTokenBuffer()
{
    tokenBufferCurrIndex++;
    if (tokenBufferCurrIndex < tokenBufferList.count())
    {
        curr = tokenBufferList.at(tokenBufferCurrIndex)->first();
    }
    else 
    {
        // Should never occur.
        printf("ERROR in HTMLTokenize::nextTokenBuffer()\n");
    }
}

void HTMLTokenizer::first()
{ 
    tokenBufferCurrIndex = 0;
    curr = 0;
    if (tokenBufferList.count())
    {
        HTMLTokenBuffer *tokenBufferCurr = tokenBufferList.at(tokenBufferCurrIndex);
        if (tokenBufferCurr)
        {
            curr = tokenBufferCurr->first();
        }
    }
}



HTMLTokenizer::~HTMLTokenizer()
{
    reset();
}

HTMLString HTMLTokenizer::nextToken()
{
    if (!curr)
    {
        nextOptionPtr = 0;
        return HTMLString();
    }
    
    QChar *t = (QChar *) curr;
    int len = ustrlen(curr);
    curr += len+1;

    if ((curr != next) && (*curr == QChar::null))
    {
    	// End of HTMLTokenBuffer, go to next buffer.
	    nextTokenBuffer();
    }

    if(*(t+2) == QChar::null)
	nextOptionPtr = 0;
    else
	nextOptionPtr = t+3; // Skip: TAG_ESCAPE / ID_xxx / TAG_ESCAPE

    return HTMLString(t, len);
}

const Attribute *HTMLTokenizer::nextOption()
{
    QChar *t = nextOptionPtr;
    
    if (!t)
        return 0;
        
    if (*t == QChar::null)
    {
        nextOptionPtr = 0;
        return 0;
    }
    
    while( *nextOptionPtr != QChar(TAG_ESCAPE) && 
	   *nextOptionPtr != QChar::null ) 
	nextOptionPtr++;

    uint len = nextOptionPtr - t - 1;
    if ( *nextOptionPtr == QChar::null )
	nextOptionPtr = 0;
    else 
	*nextOptionPtr++ = QChar('\0');

    currAttr.id = t->unicode();
    currAttr.setValue(t+1, len);
#ifdef TOKEN_DEBUG
    printf("attribute: %d, %s, len=%d\n", currAttr.id, 
	   currAttr.value().string().latin1(), len);
#endif

    return &currAttr;
}

bool HTMLTokenizer::hasMoreTokens()
{
    if ( !blocking.isEmpty() &&
	    blocking.getFirst()->token() == curr )
	{
       	return false;
    }

    return ( ( curr != 0 ) && (curr != next) );
}

inline void HTMLTokenizer::checkBuffer(int len)
{
	// do we need to enlarge the buffer?
	if ( (dest - buffer) > size-len )
	{
	    QChar *newbuf = new QChar [ size + 1024 + 20 ];
	    memcpy( newbuf, buffer, dest - buffer + 1 );
	    dest = newbuf + ( dest - buffer );
	    delete [] buffer;
	    buffer = newbuf;
	    size += 1024;
	}
}

//-----------------------------------------------------------------------------

StringTokenizer::StringTokenizer()
{
    buffer = 0;
    pos    = 0;
    end    = 0;
    bufLen = 0;
}

void StringTokenizer::tokenize( HTMLString str, const QChar *_separators )
{
    if ( str.unicode() == 0 )
    {
	pos = 0;
	return;
    }

    int strLength = str.length();

    if ( bufLen < strLength )
    {
	delete [] buffer;
	buffer = new QChar[ strLength ];
	bufLen = strLength;
    }

    end = buffer;
    bool quoted = false;
    
    for ( ; str.length() - 1; ++str )
    {
	QChar *x = ustrchr( _separators, str[0] );
	if ( str[0] == QChar('\"') )
	    quoted = !quoted;
	else if ( x && !quoted )
	    *end++ = 0;
	else
	    *end++ = str[0];
    }

    *end = 0;

    if ( end - buffer <= 1 )
	pos = 0;	// no tokens
    else
	pos = buffer;
}

HTMLString StringTokenizer::nextToken()
{
    if ( pos == 0 )
	return HTMLString();

    QChar *ret = pos;
    int len = ustrlen(ret);
    pos += len + 1;
    if ( pos >= end )
	pos = 0;

    return HTMLString(ret, len);
}

StringTokenizer::~StringTokenizer()
{
    if ( bufLen > 0 )
	delete [] buffer;
}

