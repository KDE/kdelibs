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
// KDE HTML Widget - Tokenizers
//

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "htmltoken.h"
#include "ampseq.h"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

// Include Java Script
#include <jsexec.h>

//-----------------------------------------------------------------------------

const char *BlockingToken::token()
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
    scriptString = "</script>";
    jsEnvironment = 0L;
    widget = _widget;
}

void HTMLTokenizer::begin()
{
    size = 1000;
    buffer = new char[ 1024 ];
    dest = buffer;
    tokenList.clear();
    tokenList.append( "" );		// dummy first token
    blocking.clear();
    tag = false;
    space = false;
    pre = false;
    script = false;
    comment = false;
    squote = false;
    dquote = false;
    tquote = false;
    scriptCount = 0;
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
    
    if ( str == 0L )
	return;
    
    const char *src = str;
    if ( tokenList.current() == 0L )
	tokenList.last();
    int pos = tokenList.at();

    // If we have <pre> and get a '\t' we need to know
    // in which row we are in order to calculate the next
    // tabulators position.
    int pre_pos = 0;

    while ( *src != 0 )
    {
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
	    if ( !strncmp( src, "-->", 3 ) )
	    {
		src += 3;
		comment = false;
	    }
	    else
		src++;
	}
	// We are inside of the <script> tag. Look for </script>,
	// otherwise print out every received character
	else if ( script )
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

	    if ( *src == '\\' )
	    {
		src++;
		scriptCode[ scriptCodeSize++ ] = *src++;
	    }
	    else if ( *src == '\"' && !squote)
	    {
		scriptCode[ scriptCodeSize++ ] = *src++;
		dquote = !dquote;
	    }
	    else if ( *src == '\'' && !dquote )
	    {
		scriptCode[ scriptCodeSize++ ] = *src++;
		squote = !squote;
	    }
	    // Did we find </script> ? => We have the complete code
	    else if ( scriptCount == 8 && *src == '>' )
	    {
		src++;
		scriptCode[ scriptCodeSize ] = 0;
		scriptCode[ scriptCodeSize + 1 ] = 0;
		script = false;
/*
		printf("================================================================\n");
		if ( jsEnvironment == 0L )
		    jsEnvironment = widget->getJSEnvironment();
		printf("================================================================\n");
		printf("%s\n",scriptCode );
		printf("================================================================\n");
		JSCode *code = jsEnvironment->parse( scriptCode );
		printf("================================================================\n");
		int ret = jsEnvironment->exec( code );
		printf("RETURN '%i'\n",ret );
		delete code;
		const char *javaOutString = jsEnvironment->readOutput();
		printf("================================================================\n");
		if ( srcPtr )
		    delete [] srcPtr;
		srcPtr = new char[ strlen( src ) + strlen( javaOutString ) + 1 ];
		strcpy( srcPtr, javaOutString );
		strcat( srcPtr, src );
		src = srcPtr;
		printf("================================================================\n");
*/
	    }
	    // Find out wether we see a </script> tag without looking at
	    // any other then the current character, since further characters
	    // may still be on their way thru the web!
	    else if ( scriptCount > 0 )
	    {
		if ( tolower(*src) == scriptString[ scriptCount ] )
		{
		    scriptBuffer[ scriptCount ] = *src;
		    scriptCount++;
		    src++;
		}
		// We were wrong => print all buffered characters and the current one;
		else
		{
		    scriptBuffer[ scriptCount ] = 0;
		    char *p = scriptBuffer;
		    while ( *p ) scriptCode[ scriptCodeSize++ ] = *p++;
		    scriptCode[ scriptCodeSize++ ] = *src++;
		    scriptCount = 0;
		}
		
	    }
	    // Is this perhaps the start of the </script> tag?
	    else if ( *src == '<' && !dquote && !squote )
	    {
		scriptCount = 1;
		scriptBuffer[ 0 ] = '<';
		src++;
	    }
	    else
		scriptCode[ scriptCodeSize++ ] = *src++;
	}
	else if ( *src == '&' )
	{
	    if ( pre )
		pre_pos++;	    
	    space = false;

	    // Is the string long enough?
	    if ( *(src+1) != '\0' && *(src+2) != '\0' )
	    {
		// Special character by number?
		if ( *(src + 1) == '#' )
		{
		    char *endptr;
		    int z = (int) strtol( src+2, &endptr, 10 );
		    *dest++ = z;
		    src = endptr;
		    // Skip a trailing ';' ?
		    if ( *src == ';' )
			src++;
		}
		// Special character ?
		else if ( isalpha( *(src + 1) ) )
		{
		    int tmpcnt;
		    
		    for ( tmpcnt = 0; tmpcnt < NUM_AMPSEQ; tmpcnt++ ) 
		    {
			if ( strncmp( AmpSequences[ tmpcnt ].tag, src+1,
			     strlen( AmpSequences[ tmpcnt ].tag ) ) == 0 )
			{
			    *dest++ = AmpSequences[ tmpcnt ].value;
			    src += strlen( AmpSequences[ tmpcnt ].tag ) + 1;
			    if ( *src == ';' )
				src++;
			    break;
			}
		    }

		    if ( tmpcnt == NUM_AMPSEQ )
			*dest++ = *src++;
		}
		else
		    *dest++ = *src++;
	    }
	    else
		*dest++ = *src++;
	}
	else if ( *src == '<' )
	{
	    if ( strncasecmp( src, "<!--", 4 ) == 0 && !script )
	    {
		src += 4;
		comment = true;
		continue;
	    }

	    space = false;
	    tquote = false;

	    if ( dest > buffer )
	    {
		*dest = 0;
		tokenList.append( buffer );
		dest = buffer;
	    }
	    *dest++ = TAG_ESCAPE;
	    *dest++ = '<';
	    tag = true;
	    src++;
	}
	else if ( *src == '>' && tag && !tquote )
	{
	    if ( strncasecmp( buffer+1, "<pre", 4 ) == 0 )
	    {
		pre_pos = 0;
		pre = true;
	    }
	    else if ( strncasecmp( buffer+1, "</pre", 5 ) == 0 )
		pre = false;
	    else if ( strncasecmp( buffer+1, "<script", 7 ) == 0 )
	    {
		script = true;
//		blocking.append( new BlockingToken( BlockingToken::Script,
//				tokenList.at() ) );

		scriptCode = new char[ 1024 ];
		scriptCodeSize = 0;
		scriptCodeMaxSize = 1024;
	    }
	    else if ( strncasecmp( buffer+1, "<frameset", 9 ) == 0 )
	    {
		blocking.append( new BlockingToken( BlockingToken::FrameSet,
				tokenList.at() ) );
	    }
	    else if ( strncasecmp( buffer+1, "<cell", 5 ) == 0 )
	    {
		blocking.append( new BlockingToken( BlockingToken::Cell,
				tokenList.at() ) );
	    }
	    else if ( strncasecmp( buffer+1, "<table", 6 ) == 0 )
	    {
		blocking.append( new BlockingToken( BlockingToken::Table,
				tokenList.at() ) );
	    }
	    else if ( !blocking.isEmpty() && 
		    strncasecmp( buffer+1, blocking.getLast()->token(),
			strlen( blocking.getLast()->token() ) ) == 0 )
	    {
		blocking.removeLast();
	    }

	    space = false;

	    *dest++ = '>';
	    *dest = 0;
	    tokenList.append( buffer );
	    dest = buffer;
	    tag = false;
	    src++;
	}
	else if ( !tag && pre && ( *src == ' ' || *src == '\t' ||
		*src == '\n' || *src == 13 ) )
	{
	    // For every line break in <pre> insert the tag '\n'.
	    if ( *src == '\n' )
	    {
		if ( dest > buffer )
		{
		    *dest = 0;
		    tokenList.append( buffer );
		    dest = buffer;
		}
		*dest++ = TAG_ESCAPE;
		*dest++ = '\n';
		*dest = 0;
		tokenList.append( buffer );
		dest = buffer;
		pre_pos = 0; 
	    }
	    else if ( *src == '\t' )
	    {
		int p = TAB_SIZE - ( pre_pos % TAB_SIZE );
		for ( int x = 0; x < p; x++ )
		    *dest++ = ' ';
	    }
	    else if ( *src == ' ' )
	    {
		pre_pos++;
		*dest++ = ' ';
		space = true;
	    }
	    src++;
	}
	else if ( *src == ' ' || *src == '\t' || *src == '\n' || *src == 13 )
	{
	    if ( !space )
	    {
		*dest++ = ' ';
		if ( !tag )
		{
		    *dest = 0;
		    tokenList.append( buffer );
		    dest = buffer;
		}
		space = true;
	    }
	    src++;
	}
	else
	{
	    space = false;
	    if ( pre )
		pre_pos++;

	    if ( tag && *src == '\"' )
		tquote = !tquote;
	    
	    *dest++ = *src++;
	}
    }

    if ( pos >= 0 )
	tokenList.at( pos );
    else
	tokenList.last();

    if ( srcPtr )
	delete [] srcPtr;
}

void HTMLTokenizer::end()
{
    int pos = tokenList.at();

    if ( dest > buffer )
    {
	*dest = 0;
	tokenList.append( buffer );
    }

    delete [] buffer;

    // if there are still blocking tokens then the HTML is illegal - remove
    // blocks anyway and hope for the best
    blocking.clear();

    if ( pos >= 0 )
	tokenList.at( pos );
    else
	tokenList.last();
}

const char* HTMLTokenizer::nextToken()
{
    const char *ret = tokenList.next();

    return ret;
}

bool HTMLTokenizer::hasMoreTokens()
{
    if ( !blocking.isEmpty() &&
	    blocking.getFirst()->getPosition() <= tokenList.at() )
	return false;

    return (tokenList.current() && tokenList.current() != tokenList.getLast());
}

HTMLTokenizer::~HTMLTokenizer()
{
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

