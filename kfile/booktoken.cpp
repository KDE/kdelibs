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
// Modified HTMLTokenizer from khtmlw
//

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "booktoken.h"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <strings.h>

#include <kcharsets.h>
#include <kapp.h>

//-----------------------------------------------------------------------------

BookmarkTokenizer::BookmarkTokenizer()
{
    head = tail = curr = 0;
    buffer = 0;
}

void BookmarkTokenizer::reset()
{
    while ( head )
    {
	curr = head->next();
	delete head;
	head = curr;
    }

    head = tail = curr = 0;

    if ( buffer )
	delete [] buffer;
    buffer = 0;
}

void BookmarkTokenizer::begin()
{
    reset();
    size = 1000;
    buffer = new char[ 1024 ];
    dest = buffer;
    tag = false;
    space = false;
    discardCR = false;
    comment = false;
    tquote = false;
}

void BookmarkTokenizer::write( const char *str )
{
    KCharsets *charsets=KApplication::getKApplication()->getCharsets();
    
    if ( str == 0L )
	return;
    
    const char *src = str;

    while ( *src != 0 )
    {
	// do we need to enlarge the buffer?
	if ( dest - buffer > size )
	{
	    char *newbuf = new char [ size + 1024 ];
	    memcpy( newbuf, buffer, dest - buffer + 1 );
	    dest = newbuf + ( dest - buffer );
	    delete [] buffer;
	    buffer = newbuf;
	    size += 1024;
	}
	if ( comment )
	{
	    if ( !strncmp( src, "->", 2 ) )
	    {
		src += 2;
		comment = false;
	    }
	    else
		src++;
	}
	else if ( *src == '&' ) 
	{
	    space = false;

	    // Is the string long enough?
	    if ( *(src+1) != '\0' && *(src+2) != '\0' )
	    {
		// Special character by number?
		if ( *(src + 1) == '#' )
		{
		    char *endptr;
		    int z = (int) strtol( src+2, &endptr, 10 );
		    // parse only ascii characters unless in quotes
		    if (z<128 || tquote){
		        debug("Adding character: '%c'\n",z);
			*dest++ = z;
		    }
		    else{
		      *dest=0;
		      // add currend token
		      appendToken(buffer,dest-buffer);
		      
		      // add token with the amp-sequence for further conversion
		      memcpy(buffer,src,endptr-src);
		      buffer[endptr-src]=0;
		      debug("Adding token: '%s'\n",buffer);
		      appendToken(buffer,endptr-src);
		      
		      dest=buffer;
		    }
		    src = endptr;
		    // Skip a trailing ';' ?
		    if ( *src == ';' )
		        src++;
		}
		// Special character ?
		else if ( isalpha( *(src + 1) ) )
		{
		     if (!tag){
		       // add currend token
		       *dest=0;
		       debug("Adding current token: %s\n",buffer);
		       appendToken(buffer,dest-buffer);
		       dest=buffer;
		    
		       const char *endptr=src+1;
		       while(*endptr && isalpha(*endptr)) endptr++;
		       if (*endptr==';') endptr++;
		       // add token with the amp-sequence for further conversion
		       memcpy(buffer,src,endptr-src);
		       buffer[endptr-src]=0;
		       debug("Adding token: '%s'\n",buffer);
		       appendToken(buffer,endptr-src);
		       src=endptr;
		       *dest=0;
		     }
		     else{
		       // There is no need for font switching 
		       // when we are in tag quotes, so amp-sequences can be 
		       // translated here. I hope noone uses non iso-8859-1
		       // characters here.
		       int len=0;
		       const QString res=charsets->convertTag(src,len).copy();
		       debug("Converted to: %s, len: %i\n",(const char *)res,len);
		       if ( len > 0 )
		       {
			   memcpy(dest,(const char *)res,res.length());
			   dest+=res.length();
			   src+=len;
		       }
		       else
		       {
			   *dest++ = *src++;
		       }
		     }  
		}
		else
		    *dest++ = *src++;
	    }
	    else
		*dest++ = *src++;
	}
	else if ( *src == '<' && !tquote )
	{
	    src++;
	    if ( strncmp( src, "!-", 2 ) == 0 )
	    {
		src += 2;
		comment = true;
		continue;
	    }

	    space = true;      // skip leading spaces
	    discardCR = true;  // skip leading CR
	    tquote = false;

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
	}
	else if ( *src == '>' && tag && !tquote )
	{
	    space = false;
	    discardCR = false;

	    *dest = '>';
	    *(dest+1) = 0;

	    // make the tag lower case
	    char *ptr = buffer+2;
	    while ( *ptr && *ptr != ' ' )
	    {
		*ptr = tolower( *ptr );
		ptr++;
	    }

	    appendToken( buffer, dest-buffer+1 );
	    dest = buffer;

	    tag = false;
	    src++;
	}
	else if ( *src == '\n' )
	{
	    if ( !discardCR )
	    {
		if ( tag )
		{
		    if ( !space )
		    {
			*dest = ' ';
			dest++;
			space = true;
		    }
		}
		else if ( !space )
		{
		    *dest = 0;
		    appendToken( buffer, dest-buffer );
		    dest = buffer;

		    *dest = ' ';
		    *(dest+1) = 0;
		    appendToken( buffer, 1 );
		    dest = buffer;

		    space = true;
		}
	    }
	    src++;
	}
	else if ( *src == ' ' )
	{
	    if ( tag )
	    {
		if ( !space )
		{
		    *dest = ' ';
		    dest++;
		    space = true;
		}
	    }
	    else if ( !space )
	    {
		*dest = 0;
		appendToken( buffer, dest-buffer );
		dest = buffer;

		*dest = ' ';
		*(dest+1) = 0;
		appendToken( buffer, 1 );
		dest = buffer;

		space = true;
	    }
	    src++;
	}
	else if ( *src == '\t' )
	{
	    if ( tag )
	    {
		if ( !space )
		{
		    *dest = ' ';
		    dest++;
		    space = true;
		}
	    }
	    else if ( !space )
	    {
		*dest = 0;
		appendToken( buffer, dest-buffer );
		dest = buffer;

		*dest = ' ';
		*(dest+1) = 0;
		appendToken( buffer, 1 );
		dest = buffer;

		space = true;
	    }
	    src++;
	}
	else if ( *src == 13 )
	{
	    // discard
	    src++;
	}
	else if ( *src == '\"' || *src == '\'')
	{ // we treat " & ' the same in tags
	    if ( tag )
	    {
		src++;
		if ( *(dest-1) == '=' )
		{
		    tquote = true;
		    *dest = '\"';
		    dest++;
		    space = false;
		    discardCR = false;
		}
		else if ( tquote )
		{
		    tquote = false;
		    *dest = '\"';
		    dest++;
		    *dest = ' ';
		    dest++;
		    space = true;
		    discardCR = true;
		}
		else
		    continue;  // stray '\"'
	    }
	    else
	    {
		space = false;
		discardCR = false;

		*dest = *src++;
		dest++;
	    }
	}
	else if ( *src == '=' )
	{
	    src++;

	    if ( tag )
	    {
		if ( tquote )
		{
		    space = false;
		    discardCR = false;
		    *dest = '=';
		    dest++;
		}
		else
		{
		    // discard space before '='
		    if ( *(dest-1) == ' ' )
			dest--;

		    *dest = '=';
		    dest++;
		    space = true;
		    discardCR = true;
		}
	    }
	    else
	    {
		space = false;
		discardCR = false;

		*dest = '=';
		dest++;
	    }
	}
	else
	{
	    space = false;
	    discardCR = false;

	    *dest = *src++;
	    dest++;
	}
    }
}

void BookmarkTokenizer::end()
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
}

char* BookmarkTokenizer::nextToken()
{
    char *t = curr->token();
    curr = curr->next();

    return t;
}

bool BookmarkTokenizer::hasMoreTokens()
{
    return ( curr != 0 );
}

BookmarkTokenizer::~BookmarkTokenizer()
{
    reset();
}

//-----------------------------------------------------------------------------

