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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "khtmltoken.h"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <strings.h>

#include "khtmltags.c"

#include <kcharsets.h>
#include <kapp.h>

// Include Java Script
#include <jsexec.h>

// Token buffers are allocated in units of TOKEN_BUFFER_SIZE bytes.
#define TOKEN_BUFFER_SIZE ((32*1024)-1)

// String buffers are allocated in units of STRING_BUFFER_SIZE bytes.
#define STRING_BUFFER_SIZE ((32*1024)-1)

static const char *commentStart = "<!--";
static const char *scriptEnd = "</script>";
static const char *styleEnd = "</style>";
static const char *listingEnd = "</listing>";


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
    reset();
}

void HTMLTokenizer::reset()
{
    while (!tokenBufferList.isEmpty())
    {
    	char *oldBuffer = (char *) tokenBufferList.take(0);
        delete [] oldBuffer;
    }

    while (!stringBufferList.isEmpty())
    {
    	char *oldBuffer = (char *) stringBufferList.take(0);
        delete [] oldBuffer;
    }

    last = 0;
    next = 0;
    curr = 0;
    tokenBufferSizeRemaining = 0; // No space allocated at all

    nextString = 0;
    stringBufferSizeRemaining = 0; // No space allocated at all

    if ( buffer )
	delete [] buffer;
    buffer = 0;

    if ( scriptCode )
    	delete [] scriptCode;
    scriptCode = 0;
}

void HTMLTokenizer::begin()
{
    reset();
    blocking.clear();
    size = 1000;
    buffer = new char[ 1024 ];
    dest = buffer;
    tag = false;
    pending = NonePending;
    discard = NoneDiscard;
    pre = false;
    prePos = 0;
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
}

void HTMLTokenizer::addListing(const char *list)
{
    bool old_pre = pre;
    // This function adds the listing 'list' as
    // preformatted text-tokens to the token-collection
    // thereby converting TABs.
    pre = true;
    prePos = 0;

    while ( *list != 0 )
    {
	// do we need to enlarge the buffer?
	if ( (dest - buffer) > size )
	{
	    char *newbuf = new char [ size + 1024 + 20 ];
	    memcpy( newbuf, buffer, dest - buffer + 1 );
	    dest = newbuf + ( dest - buffer );
	    delete [] buffer;
	    buffer = newbuf;
	    size += 1024;
	}
	
	if (skipLF && (*list != '\n'))
	{
	    skipLF = false;
	}

	if (skipLF)
	{
	    list++;
	} 
	else if (( *list == '\n' ) || ( *list == '\r' ))
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
	    if (*list == '\r')
	    {
		skipLF = true;
	    }
	    list++;
	}
	else if (( *list == ' ' ) || ( *list == '\t'))
	{
	    if (pending)
	        addPending();
	    if (*list == ' ')
	        pending = SpacePending;
	    else 
	        pending = TabPending;
	    list++;
	}
	else
	{
	    discard = NoneDiscard;
	    if (pending)
	    	addPending();

	    prePos++;
	    *dest++ = *list++;
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
    
    *dest = TAG_ESCAPE;
    dest++;
    *((unsigned char *)dest) = ID_LISTING + ID_CLOSE_TAG;
    dest++;
    *dest = 0;
    appendToken( buffer, dest-buffer );
    dest = buffer;
    
    pre = old_pre;    
}

void HTMLTokenizer::parseListing( const char * &src)
{
    // We are inside of the <script> or <style> tag. Look for the end tag
    // which is either </script> or </style>,
    // otherwise print out every received character

    while ( *src != 0 )
    {
	// do we need to enlarge the buffer?
	if ( (dest - buffer) > size )
	{
	    char *newbuf = new char [ size + 1024 + 20 ];
	    memcpy( newbuf, buffer, dest - buffer + 1 );
	    dest = newbuf + ( dest - buffer );
	    delete [] buffer;
	    buffer = newbuf;
	    size += 1024;
	}
        // Allocate memory to store the script. We will write maximal
        // 10 characers.
        if ( scriptCodeSize + 11 > scriptCodeMaxSize )
        {
            char *newbuf = new char [ scriptCodeSize + 1024 ];
	    memcpy( newbuf, scriptCode, scriptCodeSize );
	    delete [] scriptCode;
	    scriptCode = newbuf;
	    scriptCodeMaxSize += 1024;
        }

        if ( ( *src == '>' ) && ( searchFor[ searchCount ] == '>'))
        {
	    src++;
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
	        addListing(scriptCode);
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
	    if ( tolower(*src) == searchFor[ searchCount ] )
	    {
	        searchBuffer[ searchCount ] = *src;
	        searchCount++;
	        src++;
	    }
	    // We were wrong => print all buffered characters and the current one;
	    else
	    {
	        searchBuffer[ searchCount ] = 0;
	        char *p = searchBuffer;
	        while ( *p ) scriptCode[ scriptCodeSize++ ] = *p++;
	        scriptCode[ scriptCodeSize++ ] = *src++;
	        searchCount = 0;
	    }
        }
        // Is this perhaps the start of the </script> or </style> tag?
        else if ( *src == '<' )
        {
	    searchCount = 1;
	    searchBuffer[ 0 ] = '<';
	    src++;
        }
        else
        {
	    scriptCode[ scriptCodeSize++ ] = *src++;
	}
    }
}

void HTMLTokenizer::parseScript( const char * &src)
{
    parseListing(src);
}
void HTMLTokenizer::parseStyle( const char * &src)
{
    parseListing(src);
}

void HTMLTokenizer::parseComment( const char * &src)
{
    while ( *src != 0 )
    {
	// do we need to enlarge the buffer?
	if ( (dest - buffer) > size )
	{
	    char *newbuf = new char [ size + 1024 + 20 ];
	    memcpy( newbuf, buffer, dest - buffer + 1 );
	    dest = newbuf + ( dest - buffer );
	    delete [] buffer;
	    buffer = newbuf;
	    size += 1024;
	}

	// Look for '-->'
	if (*src == '-') 
	{
	    if (searchCount < 2)	// Watch out for '--->'
	        searchCount++;
	}
	else if ((searchCount == 2) && (*src == '>'))
	{
	    // We got a '-->' sequence
	    comment = false;
	    src++;
	    return; // Finished parsing comment!
	}
	else
	{
	    searchCount = 0;
	}
        src++;
    }
}

void HTMLTokenizer::parseEntity( const char * &src)
{
    KCharsets *charsets=KApplication::getKApplication()->getCharsets();

    while ( *src != 0 )
    {
	// do we need to enlarge the buffer?
	if ( (dest - buffer) > size )
	{
	    char *newbuf = new char [ size + 1024 + 20 ];
	    memcpy( newbuf, buffer, dest - buffer + 1 );
	    dest = newbuf + ( dest - buffer );
	    delete [] buffer;
	    buffer = newbuf;
	    size += 1024;
	}

        unsigned long entityValue = 0;
	QString res;
	int bytesConverted = 0; // 0 bytes --> all converted

	searchBuffer[ searchCount+1] = *src;
	searchBuffer[ searchCount+2] = '\0';
	    
	// Check for '&#000' or '&#x0000' sequence
	if (searchBuffer[2] == '#')
	{
	    if ((searchCount > 1) && 
	        (!isdigit(*src)) &&
	        (searchBuffer[3] != 'x'))    
	    {	
	        // '&#000'
	        searchBuffer[ searchCount+1] = '\0';
	        entityValue = strtoul( &(searchBuffer[3]), 
	    	    			NULL, 10 );
	        charEntity = false;
	    } 
	    if ((searchCount > 1) && 
	        (!isalnum(*src)) &&
	        (searchBuffer[3] == 'x'))    
	    {
	        // '&#x0000'	
	        searchBuffer[ searchCount+1] = '\0';
	        entityValue = strtoul( &(searchBuffer[4]), 
	    	    			NULL, 16 );
	        charEntity = false;
	    }
	}
	else
	{
	    // Check for &abc12 sequence
	    if (!isalnum(*src))
	    {
	        charEntity = false;
	        searchBuffer[ searchCount+1] = '\0';
	        res = charsets->convertTag(searchBuffer+1, bytesConverted).copy();
	        if (bytesConverted <= 0)
	        {
	            bytesConverted = 0;
	            res = 0;
		}
	    }
	}
	    
	if (searchCount > 8)
	{
	    // This sequence is too long.. we ignore it
	    charEntity = false;
            memcpy(dest,searchBuffer+1, searchCount);
	    dest += searchCount;
	    // *dest++ = *src++;
	    if ( pre )
	        prePos += searchCount;	    
	    searchCount = 0;
	    return;
	}
	else if (charEntity)
	{
	    // Keep searching for end of character entity 	
	    searchCount++;
	    src++;
	}
	else
	{
	    // We have a complete sequence
	    if (res && (res.length() == 1))
	    {
	        entityValue = *((unsigned char *)res.data());
	    }

	    if (tag && 
	        ( (*src != ';') || (bytesConverted != searchCount) )
	       )
	    {
	        // Don't translate entities in tags with a missing ';'
	        entityValue = 0;
	        res = 0;
	    }
	    
	    if (
	        (
	         (entityValue < 128) &&
	         (entityValue > 0)
	        ) 
	         ||
	         (entityValue == 160)
	       )
	    {
	        // Just insert plain ascii
	        *dest++ = (char) entityValue;
	        if (pre)
	            prePos++;
		if (*src == ';')
		    src++;
	    }	    	
	    else if (!entityValue && !res)
	    {
	        // ignore the sequence, add it to the buffer as plaintext
	        *dest++ = '&';
	        memcpy(dest,searchBuffer+2, searchCount-1);
	        dest += searchCount-1;
	        if (pre)
	            prePos += searchCount;
		bytesConverted = 0;
	    }
	    else if (!tag && !textarea && !select && !title) 
	    {
	        // add current token first
	        if (dest > buffer)
	        {
		    *dest=0;
		    appendToken(buffer,dest-buffer);
		    dest = buffer;
		}
		    
		// add token with the amp-sequence for further conversion
		((unsigned char *)searchBuffer)[1] = ID_ENTITY; 
		if (bytesConverted > 0)
		    appendToken(searchBuffer, bytesConverted+1);
		else
		    appendToken(searchBuffer, searchCount+1);
		dest = buffer;
		// Assume a width of 1
		if (pre)
		    prePos++;
		if (*src == ';')
		    src++;
	    }
	    else if (res)
	    {
	        // insert the characters, assuming iso-8859-1
	        memcpy(dest, res.data(), res.length());
	        dest += res.length();
	        if (pre)
	            prePos += res.length();
		if (*src == ';')
		    src++;
	    }
	    else if (entityValue > 0) 
	    {
	        // insert the character, assuming iso-8859-1
	        *dest++ = (char) entityValue;
	        if (pre)
	            prePos++;
		if (*src == ';')
		    src++;
	    }
	    if (bytesConverted > 0)
	    {
	        memcpy(dest, searchBuffer+1+bytesConverted, searchCount-bytesConverted);
	        dest += searchCount-bytesConverted;
                if ( pre )
                    prePos += searchCount-bytesConverted;
	    }
	    searchCount = 0;
	    return;
	 }
    }
}

void HTMLTokenizer::parseTag( const char * &src)
{
    // TODO:
    // &-entities can occur in attributes! We should take care of that!
    // Best thing is probably moving the entity-stuff to a seperate function
    // as well.
    //
    // Waba
    if (charEntity)
        parseEntity(src);

    while ( *src != 0 )
    {
	// do we need to enlarge the buffer?
	if ( (dest - buffer) > size )
	{
	    char *newbuf = new char [ size + 1024 + 20 ];
	    memcpy( newbuf, buffer, dest - buffer + 1 );
	    dest = newbuf + ( dest - buffer );
	    delete [] buffer;
	    buffer = newbuf;
	    size += 1024;
	}

	if (skipLF && (*src != '\n'))
	{
	    skipLF = false;
	}
	if (skipLF)
	{
	    src++;
	} 
	else if ( *src == '>' && !tquote )
	{
	    int tagID;
	    bool startTag;
	    const char *tagStr;

            searchCount = 0; // Stop looking for '<!--' sequence
	    tag = false;
	    pending = NonePending; // Ignore pending spaces
	    src++;

	    *dest = '\0';

	    // make the tag lower case
	    char *ptr = buffer+2;
	    if (*ptr == '/')
	    { 
	    	// End Tag
	    	startTag = false;
	    	ptr++;
	    	discard = NoneDiscard;
	    }
	    else
	    {
	    	// Start Tag
	    	startTag = true;
	    	// Ignore CR/LF's after a start tag
	    	discard = LFDiscard;
	    }

	    tagStr = ptr;
	    while (  
	             ((*ptr >= 'a') && (*ptr <= 'z')) ||
	             ((*ptr >= 'A') && (*ptr <= 'Z')) ||
	             ((*ptr >= '0') && (*ptr <= '9'))
	          )
	    {
		*ptr = tolower( *ptr );
		ptr++;
	    }
	    *ptr = '\0';
	    // tagStr : Tag
	    // ptr : first argument

	    const struct tags *tagPtr = findTag(tagStr, ptr-tagStr);
            if (!tagPtr)
            {
printf("Unknown tag: \"%s\"\n", tagStr);
               dest = buffer;
               return; // Unknown tag, ignore
            }
            
            if (dest <= ptr)
            {
                dest = ptr+1;
                *dest = '\0';
            }

            tagID = tagPtr->id;
            if (startTag)
                *((unsigned char *)ptr) = tagID;
            else
                *((unsigned char *)ptr) = tagID + ID_CLOSE_TAG;
            
            ptr--;
            *ptr = TAG_ESCAPE;  
	    appendToken( ptr, dest-ptr );
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
		    scriptCode = new char[ 1024 ];
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
		    scriptCode = new char[ 1024 ];
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
		    scriptCode = new char[ 1024 ];
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
#ifdef NEW_LAYOUT
           	     ( tagID == ID_CELL ) ||
                     ( tagID == ID_TABLE ))
#else
           	     ( tagID == ID_CELL ))
#endif
	    {
	        if (startTag)
	        {
		    blocking.append( new BlockingToken(tagID, last) );
		}
		else
		{
	            if ( !blocking.isEmpty() &&  
	                 (blocking.getLast()->tokenId() == tagID) )
                    {
		        blocking.removeLast();
	            }
	        }
	    }
	    return; // Finished parsing tag!
	}
	else if (( *src == '\n' ) || ( *src == '\r' ))
	{
            searchCount = 0; // Stop looking for '<!--' sequence
	    if (discard == NoneDiscard)
	        pending = SpacePending; // Treat LFs inside tags as spaces

	    /* Check for MS-DOS CRLF sequence */
	    if (*src == '\r')
	    {
		skipLF = true;
	    }
	    src++;
	}
	else if (( *src == ' ' ) || ( *src == '\t'))
	{
            searchCount = 0; // Stop looking for '<!--' sequence
            if (discard == NoneDiscard)
                pending = SpacePending;
	    src++;
	}
	else if ( *src == '\"' || *src == '\'')
	{ // we treat " & ' the same in tags
    	    discard = NoneDiscard;
	    if ( *(dest-1) == '=' && !tquote )
	    {
                // according to HTML4 DTD, we can simplify
		// strings like "  my \nstring " to "my string"
		discard = SpaceDiscard; // ignore leading spaces
		pending = NonePending;
		if (*src == '\'')
		    tquote = SingleQuote;
		else
		    tquote = DoubleQuote;
	    }
	    else if ( (( tquote == SingleQuote ) && ( *src == '\'')) ||
                      (( tquote == DoubleQuote ) && ( *src == '\"')))
	    {
                tquote = NoQuote;
		pending = SpacePending; // Add space automatically
	    }
	    else if (tquote)
	    {
	        *dest++ = *src;
	    }
	    else
	    {
	        // Ignore stray "\'"
	    }
            src++;
	}
	else if ( *src == '=' )
	{
	    src++;
	    discard = NoneDiscard;
            searchCount = 0; // Stop looking for '<!--' sequence
            *dest++ = '=';
	    if ( !tquote )
	    {
	        pending = NonePending; // Discard spaces before '='
	        discard = SpaceDiscard; // Ignore following spaces
	    }
	}
	else if ( *src == '&' ) 
	{
            src++;
	    
	    discard = NoneDiscard; 
	    if (pending)
	    	addPending();
	    
	    charEntity = true;
            searchBuffer[0] = TAG_ESCAPE;
            searchBuffer[1] = '&';
            searchCount = 1;
            parseEntity(src);
	}
	else
	{
	    discard = NoneDiscard;
	    if (pending)
	    {
	        if (tquote)
	           *dest++ = ' ';
	        else
	           *dest++ = TAG_ESCAPE; // Field delimiter
                pending = NonePending;
	    }

	    if (searchCount > 0)
	    {
	        if (*src == commentStart[searchCount])
	        {
	            searchCount++;
	    	    if (searchCount == 4)
	    	    {
	    	        // Found '<!--' sequence
	    	        comment = true;
		        dest = buffer; // ignore the previous part of this tag
		        tag = false;
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
	    *dest++ = *src++;
	}
    }
}

void HTMLTokenizer::addPending()
{
    if ( tag || select)     
    {
    	*dest++ = ' ';
    }
    else if ( textarea )
    {
	if (pending == LFPending)
	    *dest++ = '\n';
	else
	    *dest++ = ' ';    	
    }
    else if ( pre )
    {
    	int p;

	switch (pending)
	{
	  case SpacePending:
		// Insert a non-breaking space
		*(unsigned char *)dest++ = 0xa0; 
	  	prePos++;
	  	break;

	  case LFPending:
		if ( dest > buffer )
		{
		    *dest = 0;
		    appendToken( buffer, dest-buffer );
		}
		dest = buffer;
		*dest++ = TAG_ESCAPE;
		*((unsigned char *)dest) = ID_NEWLINE;
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
		    *dest = ' ';
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
    	*dest++ = ' ';
    }

    pending = NonePending;
}

void HTMLTokenizer::write( const char *str )
{
    // If this pointer is not 0L then we allocated some memory to store HTML
    // code in. This may happen while parsing the <script> tag, since the output
    // of the java code is treated as HTML code. This means we have to modify
    // the HTML code on the fly by inserting new HTML stuff.
    // If this pointer is not null, one has to free the memory before leaving
    // this function.
    char *srcPtr = 0L;

    
    if ( str == 0L || buffer == 0L )
	return;
    
    const char *src = str;

    if (comment)
        parseComment(src);
    else if (script)
        parseScript(src);
    else if (style)
        parseStyle(src);
    else if (listing)
        parseListing(src);
    else if (tag)
        parseTag(src);
    else if (charEntity)
        parseEntity(src);

    while ( *src != 0 )
    {
	// do we need to enlarge the buffer?
	if ( (dest - buffer) > size )
	{
	    char *newbuf = new char [ size + 1024 + 20 ];
	    memcpy( newbuf, buffer, dest - buffer + 1 );
	    dest = newbuf + ( dest - buffer );
	    delete [] buffer;
	    buffer = newbuf;
	    size += 1024;
	}

	if (skipLF && (*src != '\n'))
	{
	    skipLF = false;
	}
	if (skipLF)
	{
	    src++;
	} 
	else if ( startTag)
	{
	    startTag = false;
	    if (*src == '/') 
	    {
	       // Start of an End-Tag 
	       if (pending == LFPending)
	           pending = NonePending; // Ignore leading LFs
	    }
	    else if ( ((*src >= 'a') && (*src <='z')) || 
	    	      ((*src >= 'A') && (*src <='Z')) )	    	    
	    {
	       // Start of a Start-Tag
	    }
	    else if ( *src == '!')
	    {
	       // <!-- comment -->
	    }
	    else
	    {
	       // Invalid tag
	       // Add as is
	       if (pending)
	           addPending();
	       *dest = '<';
	       dest++;
	       *dest++ = *src++;
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
	    *dest = '<';
	    dest++;
	    tag = true;
	    searchCount = 1; // Look for '<!--' sequence to start comment
	    // No 'src++' add '*src' in a second pass with 'startTag=false'
	    parseTag(src);
	}
	else if ( *src == '&' ) 
	{
            src++;
	    
	    discard = NoneDiscard; 
	    if (pending)
	    	addPending();
	    
	    charEntity = true;
            searchBuffer[0] = TAG_ESCAPE;
            searchBuffer[1] = '&';
            searchCount = 1;
            parseEntity(src);
	}
	else if ( *src == '<')
	{
	    src++;
	    startTag = true;
	    discard = NoneDiscard;
	}
	else if (( *src == '\n' ) || ( *src == '\r' ))
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
	    if (*src == '\r')
	    {
		skipLF = true;
	    }
	    src++;
	}
	else if (( *src == ' ' ) || ( *src == '\t'))
	{
	    if ( pre || textarea)
	    {
	    	if (pending)
	    	    addPending();
	    	if (*src == ' ')
	    	    pending = SpacePending;
	    	else 
	    	    pending = TabPending;
	    } 
	    else
	    {
	    	pending = SpacePending;
	    }
	    src++;
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
	    
	    *dest++ = *src++;
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

void HTMLTokenizer::appendTokenBuffer( int min_size)
{
    int newBufSize = TOKEN_BUFFER_SIZE; 

    // If we were using a buffer, mark it's end
    if (next)
    {
       // Mark current buffer end
       *next = '\0';
    }

    if (min_size > newBufSize)
    {
        // Wow! This surely is a big token...
        newBufSize += min_size; 
    }
    HTMLTokenBuffer *newBuffer = (HTMLTokenBuffer *) new char [ newBufSize + 1];
    tokenBufferList.append( newBuffer);
    next = newBuffer->first();
    tokenBufferSizeRemaining = newBufSize;
    if (!curr)
    {
       curr = tokenBufferList.at(0)->first();
       tokenBufferCurrIndex = 0;
    }
}                                                                                         

void HTMLTokenizer::appendStringBuffer( int min_size)
{
    int newBufSize = STRING_BUFFER_SIZE; 

    if (min_size > newBufSize)
    {
        // Wow! This surely is a big string...
        newBufSize += min_size; 
    }
    HTMLTokenBuffer *newBuffer = (HTMLTokenBuffer *) new char [ newBufSize + 1];
    stringBufferList.append( newBuffer);
    nextString = newBuffer->first();
    stringBufferSizeRemaining = newBufSize;
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
        printf("ERROR in HTMLTokenize::nextToken()\n");
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

//-----------------------------------------------------------------------------

StringTokenizer::StringTokenizer()
{
    buffer = 0;
    pos    = 0;
    end    = 0;
    bufLen = 0;
}

void StringTokenizer::tokenize( const char *str, const char *_separators )
{
    if ( *str == '\0' )
    {
	pos = 0;
	return;
    }

    int strLength = strlen( str ) + 1;

    if ( bufLen < strLength )
    {
	delete [] buffer;
	buffer = new char[ strLength ];
	bufLen = strLength;
    }

    const char *src = str;
    end = buffer;
    bool quoted = false;
    
    for ( ; *src != '\0'; src++ )
    {
	char *x = strchr( _separators, *src );
	if ( *src == '\"' )
	    quoted = !quoted;
	else if ( x && !quoted )
	    *end++ = 0;
	else
	    *end++ = *src;
    }

    *end = 0;

    if ( end - buffer <= 1 )
	pos = 0;	// no tokens
    else
	pos = buffer;
}

const char* StringTokenizer::nextToken()
{
    if ( pos == 0 )
	return 0;

    char *ret = pos;
    pos += strlen( ret ) + 1;
    if ( pos >= end )
	pos = 0;

    return ret;
}

StringTokenizer::~StringTokenizer()
{
    if ( buffer != 0 )
	delete [] buffer;
}
