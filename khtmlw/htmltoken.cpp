/* This file is part of the KDE libraries
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
//-----------------------------------------------------------------------------
//
// KDE HTML Widget - Tokenizers
//

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "htmltoken.h"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <strings.h>

#include <kcharsets.h>
#include <kapp.h>

// Include Java Script
#include <jsexec.h>

// Token buffers are allocated in units of TOKEN_BUFFER_SIZE bytes.
#define TOKEN_BUFFER_SIZE (32*1024) - 1

static const char *commentStart = "<!--";
static const char *scriptEnd = "</script>";
static const char *styleEnd = "</style>";

enum quoteEnum { NO_QUOTE=0, SINGLE_QUOTE, DOUBLE_QUOTE };

//-----------------------------------------------------------------------------

const char *BlockingToken::tokenName()
{
    switch ( ttype )
    {
	case Table:
		return "</table";
		break;

	case FrameSet:
		return "</frameset";
		break;

	case Script:
		return "</script";
		break;

	case Cell:
		return "</cell";
		break;
    }

    return "";
}

//-----------------------------------------------------------------------------

HTMLTokenizer::HTMLTokenizer( KHTMLWidget *_widget )
{
    blocking.setAutoDelete( true );
    jsEnvironment = 0L;
    widget = _widget;
    last = next = curr = 0;
    buffer = 0;
    scriptCode = 0;
}

void HTMLTokenizer::reset()
{
    while (!tokenBufferList.isEmpty())
    {
    	char *oldBuffer = (char *) tokenBufferList.take(0);
        delete [] oldBuffer;
    }

    last = next = curr = 0;
    tokenBufferSizeRemaining = 0; // No space allocated at all

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
    script = false;
    style = false;
    skipLF = false;
    select = false;
    comment = false;
    textarea = false;
    startTag = false;
    tquote = NO_QUOTE;
    searchCount = 0;
    title = false;
    charEntity = false;
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
		*dest = TAG_ESCAPE;
		*(dest+1) = '\n';
		*(dest+2) = 0;
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

    KCharsets *charsets=KApplication::getKApplication()->getCharsets();
    
    if ( str == 0L || buffer == 0L )
	return;
    
    const char *src = str;

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
	else if ( comment )
	{
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
	    }
	    else
	    {
	    	searchCount = 0;
	    }
            src++;
	}
	// We are inside of the <script> or <style> tag. Look for the end tag
	// which is either </script> or </style>,
	// otherwise print out every received character
	else if ( script || style )
	{
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
		    script = false;
		    /* Parse scriptCode containing <script> info */
		    /* Not implemented */
		}
		else
		{
		    style = false;
		    /* Parse scriptCode containing <style> info */
		    /* Not implemented */
		}
		delete [] scriptCode;
		scriptCode = 0;
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
		scriptCode[ scriptCodeSize++ ] = *src++;
	}
	else if (charEntity)
	{
            unsigned long entityValue = 0;
	    QString res = 0;

	    searchBuffer[ searchCount+1] = *src;
	    searchBuffer[ searchCount+2] = '\0';
	    
	    // Check for '&#000' sequence
	    if (searchBuffer[2] == '#')
	    {
		if ((searchCount > 1) && 
		    (!isdigit(*src)) &&
		    (searchBuffer[3] != 'x')) 
	        {	
	            // &#123
	    	    searchBuffer[ searchCount+1] = '\0';
	    	    entityValue = strtoul( &(searchBuffer[3]), 
	    	    				NULL, 10 );
	    	    charEntity = false;
	        }
		if ((searchCount > 1) && 
		    (!isalnum(*src)) &&
		    (searchBuffer[3] == 'x')) 
	        {	
	            // &#x12AB
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
	            int len;
		    charEntity = false;
		    // check trailing char to be ";", but only if in a tag (David)
                    if ((searchBuffer[searchCount+1] == ';') || (!tag)) {
	              searchBuffer[ searchCount+1] = '\0';
	              res = charsets->convertTag(searchBuffer+1, len).copy();
	              if (len <= 0)
	              {
		    	res = 0;
	              }
	           }
	        }
	    }
        // Mapping for MS-Windows Latin-1 extension.
        // These mappings do not address all the extended
        // character sets as defined by MS-Windows Latin-1
        // extension.  Rather it only deals with those that
        // are heavily used on web pages by MS-Windows based
        // tools.  Some of the entities do not have corresponding
        // enteries under ISO-8859-1, hence are not mapped. Also
        // note that some of the mappings are close but not prefect
        // matches! (Dawit A)
	    switch (entityValue)
	    {
	    case 139:
		entityValue = 60;
		break;
	    case 145:
		entityValue = 96;
		break;
	    case 146:
		entityValue = 39;
		break;
		// for the next 4 values mapping to a name doesn't work...
		// perhaps it's just my computer not having these chars...
	    case 147:
	        //strcpy(searchBuffer+2, "ldquo");
		//searchCount = 6;
		//break;
	    case 148:
	        //strcpy(searchBuffer+2, "rdquo");
		//searchCount = 6;
                entityValue = 34;
		break;
	    case 150:
	        //strcpy(searchBuffer+2, "ndash");
		//searchCount = 6;
		//break;
	    case 151:
	        //strcpy(searchBuffer+2, "mdash");
		//searchCount = 6;
                entityValue = 45;
		break;
	    case 152:
		entityValue = 126;
		break;
	    case 155:
		entityValue = 62;
		break;
	    case 133:
	        strcpy(searchBuffer+2, "hellip");
		searchCount = 7;
		break;
	    case 149:
	        strcpy(searchBuffer+2, "bull");
		searchCount = 5;
		break;
	    case 153:
	        strcpy(searchBuffer+2, "trade");
		searchCount = 6;
		break;
	    default:;
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
		    memcpy(dest,searchBuffer+1, searchCount);
		    dest += searchCount;
		    if (pre)
		    	prePos += searchCount;
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
		searchCount = 0;
	    }
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
	    else if ( *src == '?')
	    {
	       // <? meta stuff ?>
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
	}
	else if ( *src == '<' && !tag)
	{
	    src++;
	    startTag = true;
	    discard = NoneDiscard;
	}
	else if ( *src == '>' && tag && !tquote )
	{
            searchCount = 0; // Stop looking for '<!--' sequence

	    *dest = '>';
	    *(dest+1) = 0;

	    // make the tag lower case
	    char *ptr = buffer+2;
	    if (*ptr == '/')
	    { 
	    	// End Tag
	    	discard = NoneDiscard;
	    }
	    else
	    {
	    	// Start Tag
	    	// Ignore CR/LF's after a start tag
	    	discard = LFDiscard;
	    }
	    while ( *ptr && *ptr != ' ' )
	    {
		*ptr = tolower( *ptr );
		ptr++;
	    }

	    appendToken( buffer, dest-buffer+1 );
	    dest = buffer;

	    tag = false;
	    pending = NonePending; // Ignore pending spaces
	    src++;

	    if ( strncmp( buffer+2, "pre", 3 ) == 0 )
	    {
		prePos = 0;
		pre = true;
	    }
	    else if ( strncmp( buffer+2, "/pre", 4 ) == 0 )
	    {
		pre = false;
	    }
	    else if ( strncmp( buffer+2, "textarea", 8 ) == 0 )
	    {
		textarea = true;
	    }
	    else if ( strncmp( buffer+2, "/textarea", 9 ) == 0 )
	    {
		textarea = false;
	    }
	    else if ( strncmp( buffer+2, "title", 5 ) == 0 )
	    {
		title = true;
	    }
	    else if ( strncmp( buffer+2, "/title", 6 ) == 0 )
	    {
		title = false;
	    }
	    else if ( strncmp( buffer+2, "script", 6 ) == 0 )
	    {
		script = true;
                searchCount = 0;
                searchFor = scriptEnd;		
		scriptCode = new char[ 1024 ];
		scriptCodeSize = 0;
		scriptCodeMaxSize = 1024;
	    }
	    else if ( strncmp( buffer+2, "style", 5 ) == 0 )
	    {
		style = true;
                searchCount = 0;		
                searchFor = styleEnd;		
		scriptCode = new char[ 1024 ];
		scriptCodeSize = 0;
		scriptCodeMaxSize = 1024;
	    }
	    else if ( strncmp( buffer+2, "select", 6 ) == 0 )
	    {
		select = true;
	    }
	    else if ( strncmp( buffer+2, "/select", 7 ) == 0 )
	    {
		select = false;
	    }
	    else if ( strncmp( buffer+2, "frameset", 8 ) == 0 )
	    {
		blocking.append( new BlockingToken( BlockingToken::FrameSet,
				last ) );
	    }
	    else if ( strncmp( buffer+2, "cell", 4 ) == 0 )
	    {
		blocking.append( new BlockingToken(BlockingToken::Cell, last) );
	    }
	    else if ( strncmp( buffer+2, "table", 5 ) == 0 )
	    {
		blocking.append( new BlockingToken( BlockingToken::Table,
				last ) );
	    }
	    else if ( !blocking.isEmpty() && 
		    strncasecmp( buffer+1, blocking.getLast()->tokenName(),
			strlen( blocking.getLast()->tokenName() ) ) == 0 )
	    {
		blocking.removeLast();
	    }
	}
	else if (( *src == '\n' ) || ( *src == '\r' ))
	{
	    if ( tquote)
	    {
		if (discard == NoneDiscard)
		    pending = SpacePending;
	    }
	    else if ( tag )
	    {
                searchCount = 0; // Stop looking for '<!--' sequence
		if (discard == NoneDiscard)
		    pending = SpacePending; // Treat LFs inside tags as spaces
	    }
	    else if ( pre || textarea)
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
	    if ( tquote)
	    {
		if (discard == NoneDiscard)
		    pending = SpacePending;
	    }
	    else if ( tag )
	    {
                searchCount = 0; // Stop looking for '<!--' sequence
		if (discard == NoneDiscard)
		    pending = SpacePending;
	    }
	    else if ( pre || textarea)
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
	else if ( *src == '\"' || *src == '\'')
	{ // we treat " & ' the same in tags
    	    discard = NoneDiscard;
	    if ( tag )
	    {
	        searchCount = 0; // Stop looking for '<!--' sequence
                if ( ((tquote == SINGLE_QUOTE) && (*src == '\"')) ||
                    ((tquote == DOUBLE_QUOTE) && (*src == '\'')))
		{
                   // just add it
                   *dest++ = *src;
                }
                else if ( *(dest-1) == '=' && !tquote )
                {
		    // according to HTML4 DTD, we can simplify
		    // strings like "  my \nstring " to "my string"
		    discard = SpaceDiscard; // ignore leading spaces
		    pending = NonePending;
                   if (*src == '\"')
                        tquote = DOUBLE_QUOTE;
                   else
                       tquote = SINGLE_QUOTE;           
                   *dest++ = *src;
		}
		else if ( tquote )
		{
                   tquote = NO_QUOTE;
                   *dest++ = *src;
		    pending = SpacePending; // Add space automatically
		}
		else
		{
		    // Ignore stray "\'"
		}
                src++;
	    }
	    else
	    {
	    	if (pending)
	    	    addPending();

		if ( pre )
		    prePos++;

		*dest++ = *src++;
	    }
	}
	else if ( *src == '=' )
	{
	    src++;
	    discard = NoneDiscard;
	    if ( tag )
	    {
	        searchCount = 0; // Stop looking for '<!--' sequence
                *dest++ = '=';
		if ( !tquote )
		{
		    pending = NonePending; // Discard spaces before '='
		    discard = SpaceDiscard; // Ignore following spaces
		}
	    }
	    else
	    {
	    	if (pending)
	    	    addPending();

		if ( pre )
		    prePos++;

		*dest++ = '=';
	    }
	}
	else
	{
	    discard = NoneDiscard;
	    if (pending)
	    	addPending();

	    if (tag)
	    {
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
		        continue;
	    	    }
	    	}
	    	else
	    	{
	            searchCount = 0; // Stop looking for '<!--' sequence
                }  
              }
	    } 
	    else if ( pre )
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
    int quoted = NO_QUOTE;
    
    for ( ; *src != '\0'; src++ )
    {
	char *x = strchr( _separators, *src );
        if (( *src == '\"' ) && !quoted)
           quoted = DOUBLE_QUOTE;
        else if (( *src == '\'') && !quoted)
           quoted = SINGLE_QUOTE;
        else if ( (( *src == '\"') && (quoted == DOUBLE_QUOTE)) ||
                 (( *src == '\'') && (quoted == SINGLE_QUOTE)))
           quoted = NO_QUOTE;
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
