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

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <strings.h>

#include <kcharsets.h>
#include <kapp.h>

// Include Java Script
#include <jsexec.h>

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
    scriptString = "</script>";
    jsEnvironment = 0L;
    widget = _widget;
    head = tail = curr = 0;
    buffer = 0;
}

void HTMLTokenizer::reset()
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

void HTMLTokenizer::begin()
{
    reset();
    blocking.clear();
    size = 1000;
    buffer = new char[ 1024 ];
    dest = buffer;
    tag = false;
    space = false;
    discardCR = false;
    pre = false;
    script = false;
    select = false;
    comment = false;
    textarea = false;
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

    KCharsets *charsets=KApplication::getKApplication()->getCharsets();
    
    if ( str == 0L )
	return;
    
    const char *src = str;

    // If we have <pre> and get a '\t' we need to know
    // in which row we are in order to calculate the next
    // tabulators position.
    int pre_pos = 0;

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
		    // parse only ascii characters unless in quotes
		    if (z<128 || tquote){
		        debugM("Adding character: '%c'\n",z);
			*dest++ = z;
		    }
		    else{
		      *dest=0;
		      // add currend token
		      appendToken(buffer,dest-buffer);
		      
		      // add token with the amp-sequence for further conversion
		      memcpy(buffer,src,endptr-src);
		      buffer[endptr-src]=0;
		      debugM("Adding token: '%s'\n",buffer);
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
		     if (!tag && !textarea){
		       // add currend token
		       debugM("Not quoted amp-seq: %0.20s\n",src);
		       *dest=0;
		       debugM("Adding current token: %s\n",buffer);
		       appendToken(buffer,dest-buffer);
		       dest=buffer;
		    
		       const char *endptr=src+1;
		       while(*endptr && isalpha(*endptr)) endptr++;
		       if (*endptr==';') endptr++;
		       // add token with the amp-sequence for further conversion
		       memcpy(buffer,src,endptr-src);
		       buffer[endptr-src]=0;
		       debugM("Adding token: '%s'\n",buffer);
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
		       debugM("Quted amp-seq: %0.20s\n",src);
		       const QString res=charsets->convertTag(src,len).copy();
		       debugM("Converted to: %s, len: %i\n",(const char *)res,len);
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
//	    discardCR = true;
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

	    if ( strncmp( buffer+2, "pre", 3 ) == 0 )
	    {
		pre_pos = 0;
		pre = true;
	    }
	    else if ( strncmp( buffer+2, "/pre", 4 ) == 0 )
	    {
		pre = false;
	    }
	    else if ( strncmp( buffer+2, "textarea", 8 ) == 0 )
	    {
		textarea = true;
		discardCR = true;
	    }
	    else if ( strncmp( buffer+2, "/textarea", 9 ) == 0 )
	    {
		textarea = false;
	    }
	    else if ( strncmp( buffer+2, "script", 6 ) == 0 )
	    {
		script = true;
//		blocking.append(new BlockingToken(BlockingToken::Script,tail));

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
				tail ) );
	    }
	    else if ( strncmp( buffer+2, "cell", 4 ) == 0 )
	    {
		blocking.append( new BlockingToken(BlockingToken::Cell,tail) );
	    }
	    else if ( strncmp( buffer+2, "table", 5 ) == 0 )
	    {
		blocking.append( new BlockingToken( BlockingToken::Table,
				tail ) );
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
		else if ( pre )
		{ // For every line break in <pre> insert the tag '\n'.
		    if ( !select )
		    {
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
			pre_pos = 0; 
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
	    /* Check for MS-DOS CRLF sequence */
	    if ((*src == '\r') && (*(src+1) == '\n'))
	    {
		src++;
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
	    else if ( pre )
	    {
		pre_pos++;
		*dest = ' ';
		dest++;
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
	    else if ( pre )
	    {
		int p = TAB_SIZE - ( pre_pos % TAB_SIZE );
		for ( int x = 0; x < p; x++ )
		{
		    *dest = ' ';
		    dest++;
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
	else if ( *src == '\"' || *src == '\'')
	{ // we treat " & ' the same in tags
	    if ( tag )
	    {
		src++;
		if ( *(dest-1) == '=' && !tquote )
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

		if ( pre )
		    pre_pos++;

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

		if ( pre )
		    pre_pos++;

		*dest = '=';
		dest++;
	    }
	}
	else
	{
	    space = false;
	    discardCR = false;

	    if ( pre )
		pre_pos++;

	    *dest = *src++;
	    dest++;
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

char* HTMLTokenizer::nextToken()
{
    char *t = curr->token();
    curr = curr->next();

    return t;
}

bool HTMLTokenizer::hasMoreTokens()
{
    if ( !blocking.isEmpty() &&
	    blocking.getFirst()->token() == curr )
	return false;

    return ( curr != 0 );
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
