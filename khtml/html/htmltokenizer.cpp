/*
    This file is part of the KDE libraries

    Copyright (C) 1997 Martin Jones (mjones@kde.org)
              (C) 1997 Torben Weis (weis@kde.org)
              (C) 1998 Waldo Bastian (bastian@kde.org)
              (C) 1999 Lars Knoll (knoll@kde.org)
	      (C) 1999 Antti Koivisto (koivisto@kde.org)

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
//#define TOKEN_PRINT

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "htmltokenizer.h"
#include "htmltoken.h"
#include "khtmlview.h"
#include "khtml_part.h"
#include "dtd.h"
#include "htmlhashes.h"
#include <kcharsets.h>
#include <kglobal.h>

#include <kdebug.h>

#include "kjs.h"

using namespace khtml;

static const QChar commentStart [] = { '<','!','-','-' };
static const QChar commentEnd [] = { '-','-','>' };
static const QChar scriptEnd [] = { '<','/','s','c','r','i','p','t','>' };
static const QChar styleEnd [] = { '<','/','s','t','y','l','e','>' };
static const QChar listingEnd [] = { '<','/','l','i','s','t','i','n','g','>' };

// ----------------------------------------------------------------------------

HTMLTokenizer::HTMLTokenizer(KHTMLParser *p, KHTMLView *_view)
{
    view = _view;
    buffer = 0;
    scriptCode = 0;
    charsets = KGlobal::charsets();
    parser = p;
    currToken = 0;

    reset();
}

void HTMLTokenizer::reset()
{
    if ( buffer )
	delete [] buffer;
    buffer = 0;
    size = 0;

    if ( scriptCode )
    	delete [] scriptCode;
    scriptCode = 0;

    if(currToken) delete currToken;
    //parser->reset();
}

void HTMLTokenizer::begin()
{
    reset();
    currToken = 0;
    size = 1023;
    buffer = new QChar[ 1024 ];
    dest = buffer;
    tag = NoTag;
    pending = NonePending;
    discard = NoneDiscard;
    pre = false;
    prePos = 0;
    plaintext = 0;
    listing = false;
    processingInstruction = false;
    script = false;
    style = false;
    skipLF = false;
    select = false;
    comment = false;
    textarea = false;
    startTag = false;
    tquote = NoQuote;
    searchCount = 0;
    charEntity = false;
}

void HTMLTokenizer::addListing(DOMStringIt list)
{
    bool old_pre = pre;
    // This function adds the listing 'list' as
    // preformatted text-tokens to the token-collection
    // thereby converting TABs.
    if(!style) pre = true;
    prePos = 0;

    while ( list.length() )
    {
	checkBuffer();
	
	if (skipLF && ( list[0] != '\n' ))
	{
	    skipLF = false;
	}

	if (skipLF)
	{
            skipLF = false;
	    ++list;
	}
	else if (( list[0] == '\n' ) || ( list[0] == '\r' ))
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
	    if (list[0] == '\r')
	    {
		skipLF = true;
	    }
	    ++list;
	}
	else if (( list[0] == ' ' ) || ( list[0] == '\t'))
	{
	    if (pending)
	        addPending();
	    if (list[0] == ' ')
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

    currToken->text = DOMString( buffer, dest-buffer);
    processToken();
    prePos = 0;

    pre = old_pre;
}

void HTMLTokenizer::parseListing( DOMStringIt &src)
{
    // Wee are instide a <script>, <style> or comment. Look for the end tag
    // which is either </script>, </style> or -->
    // otherwise print out every received character

    kdDebug( 6036 ) << "HTMLTokenizer::parseListing()" << endl;

    while ( src.length() )
    {
	// do we need to enlarge the buffer?
	checkBuffer();

        // Allocate memory to store the script. We will write maximal
        // 10 characers.
        if ( scriptCodeSize + 10 > scriptCodeMaxSize )
        {
            QChar *newbuf = new QChar [ scriptCodeMaxSize + 1024 ];
	    memcpy( newbuf, scriptCode, scriptCodeSize*sizeof(QChar) );
	    delete [] scriptCode;
	    scriptCode = newbuf;
	    scriptCodeMaxSize += 1024;
        }

	if ( ( src[0] == '>' ) && ( searchFor[ searchCount ] == '>'))
        {
	    ++src;
	    scriptCode[ scriptCodeSize ] = 0;
	    scriptCode[ scriptCodeSize + 1 ] = 0;
	    if (comment) currToken->id = ID_COMMENT; /// ###
	    if (script)
	    {
	        /* Parse scriptCode containing <script> info */
		// ### use KHTMLView::executeScript...
	    	KJSProxy *jscript = view->part()->jScript();
		kdDebug( 6036 ) << "scriptcode is: " << QString(scriptCode, scriptCodeSize) << endl;
	  	if(jscript) jscript->evaluate(scriptCode, scriptCodeSize);
	    }
	    else if (style)
	    {
		// just add it. The style element will get a DOM::TextImpl passed, which it will
		// convert into a StyleSheet.
	        addListing(DOMStringIt(scriptCode, scriptCodeSize));
	    }
	    else
	    {
	        //
	        // Add scriptcode to the buffer
	        addListing(DOMStringIt(scriptCode, scriptCodeSize));
	    }
	    delete [] scriptCode;
	    scriptCode = 0;
	    processToken();
	    if(script)
		currToken->id = ID_SCRIPT + ID_CLOSE_TAG;
	    else if(style)
		currToken->id = ID_STYLE + ID_CLOSE_TAG;
	    else if (comment)
		currToken->id = ID_COMMENT + ID_CLOSE_TAG;
	    else
		currToken->id = ID_LISTING + ID_CLOSE_TAG;
	    processToken();
            script = style = listing = comment = false;
	    return; // Finished parsing script/style/listing	
        }
        // Find out wether we see an end tag without looking at
        // any other then the current character, since further characters
        // may still be on their way thru the web!
        else if ( searchCount > 0 )
        {
	    QChar cmp = src[0];
	    if (comment && searchCount == 2 && src[0] == '-') { // Watch out for '--->'
		scriptCode[ scriptCodeSize++ ] = '-';
		++src;
	    }
	    else if ( cmp.lower() == searchFor[ searchCount ] )
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
        // Is this perhaps the start of the </script> or </style> tag, or --> (end of comment)?
        else if ( src[0] == '<' || src[0] == '-' )
        {
	    searchCount = 1;
	    searchBuffer[ 0 ] = src[0];
	    ++src;
        }
        else
        {
	    scriptCode[ scriptCodeSize++ ] = src[0];
	    ++src;
	}
    }
}

void HTMLTokenizer::parseScript(DOMStringIt &src)
{
    parseListing(src);
}
void HTMLTokenizer::parseStyle(DOMStringIt &src)
{
    parseListing(src);
}

void HTMLTokenizer::parseComment(DOMStringIt &src)
{
    parseListing(src);
/*    while ( src.length() )
    {
	// Look for '-->'
	if ( src[0] == '-' )
	{
	    if (searchCount < 2)	// Watch out for '--->'
	        searchCount++;
	}
	else if ((searchCount == 2) && (src[0] == '>'))
	{
	    // We got a '-->' sequence
	    comment = false;
	    ++src;
	    discard=LFDiscard;
	    return; // Finished parsing comment!
	}
	else
	{
	    searchCount = 0;
	}
        ++src;
    }*/
}

void HTMLTokenizer::parseProcessingInstruction(DOMStringIt &src)
{
    while ( src.length() )
    {
	// Look for '?>'
	if ( src[0] == '?' )
	{
	    if (searchCount < 1)	// Watch out for '--->'
	        searchCount++;
	}
	else if ((searchCount == 1) && (src[0] == '>'))
	{
	    // We got a '?>' sequence
	    processingInstruction = false;
	    ++src;
	    discard=LFDiscard;
	    return; // Finished parsing comment!
	}
	else
	{
	    searchCount = 0;
	}
        ++src;
    }
}

void HTMLTokenizer::parseText(DOMStringIt &src)
{
    while ( src.length() )
    {
	// do we need to enlarge the buffer?
	checkBuffer();

	if (skipLF && ( src[0] != '\n' ))
	{
	    skipLF = false;
	}

	if (skipLF)
	{
            skipLF = false;
	    ++src;
	}
	else if (( src[0] == '\n' ) || ( src[0] == '\r' ))
	{
	    processToken();

	    /* Check for MS-DOS CRLF sequence */
	    if (src[0] == '\r')
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

void HTMLTokenizer::parseEntity(DOMStringIt &src, bool start)
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
	    *dest++ = '&';
	    memcpy(dest, entityBuffer, entityPos*sizeof(QChar));
	    dest += entityPos;
	    if ( pre )
		prePos += entityPos+1;
	    charEntity = false;
	    return;
	}
	if( (src[0].lower() >= 'a' && src[0].lower() <= 'z') ||
	    (src[0] >= '0' && src[0] <= '9') ||
	     src[0] == '#' )
	{
	    entityBuffer[entityPos] = src[0];
	    entityPos++;
	    ++src;
	}
	else // end of entity... try to decode it
	{
	    QConstString cStr(entityBuffer, entityPos);
	    QChar res = charsets->fromEntity(cStr.string());
	
	    //kdDebug( 6036 ) << "ENTITY " << res.unicode() << ", " << res << endl;
	
	    if (tag && src[0] != ';' ) {
		// Don't translate entities in tags with a missing ';'
		res = QChar::null;
	    }
	
	    // Partial support for MS Windows Latin-1 extensions
	    // full list http://www.bbsinc.com/iso8859.html
	    // There may be better equivelants
	    if ( res != QChar::null ) {
	    	switch (res.unicode())
		{
		    case 0x91: res = '\''; break;
		    case 0x92: res = '\''; break;
		    case 0x93: res = '"'; break;
		    case 0x94: res = '"'; break;
		    case 0x95: res = 0xb7; break;
		    case 0x96: res = '-'; break;
	    	    case 0x97: res = '-'; break;
		    case 0x98: res = '~'; break;
		    default: break;
	    	}
	    }

	    if ( res != QChar::null ) {
		checkBuffer();
		// Just insert it
		*dest++ = res;
		if (pre)
		    prePos++;
		if (src[0] == ';')
		    ++src;
	    } else {
		kdDebug( 6036 ) << "unknown entity!" << endl;

		checkBuffer(10);
		// ignore the sequence, add it to the buffer as plaintext
		*dest++ = '&';
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

void HTMLTokenizer::parseTag(DOMStringIt &src)
{
    if (charEntity)
        parseEntity(src);

    while ( src.length() )
    {
	checkBuffer();

	const QChar &curChar = src[0];
	
	// decide if quoted or not....
	if ( curChar == '\"' || curChar == '\'' )
	{ // we treat " & ' the same in tags
	    if ( !tquote )
	    {
		// according to HTML4 DTD, we can simplify
		// strings like "  my \nstring " to "my string"
		discard = SpaceDiscard; // ignore leading spaces
		pending = NonePending;
		if (curChar == '\'')
		    tquote = SingleQuote;
		else
		    tquote = DoubleQuote;
	    }
	    else if ( (( tquote == SingleQuote )&&( curChar == '\'')) ||
		      (( tquote == DoubleQuote )&&( curChar == '\"')) )
	    {
		tquote = NoQuote;
                discard = NoneDiscard;
		pending = NonePending; // remove space at the end of value

		// we remove additional quotes directly following the
		// end of the quoted section. Helps with bad html as
		// <tag attr="value"" nextattr="..." ...>
		while(src.length() > 1 &&
		      (src[1] == '\'' || src[1] == '\"'))
		    ++src;
	    }
	    else
	    {
		*dest++ = curChar;
	    }
	    ++src;
	}
	else if ( discard != NoneDiscard &&
		  ( curChar == ' ' || curChar == '\t' ||
		    curChar == '\n' || curChar == '\r' ) )
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
		    kdDebug( 6036 ) << "bad HTML in parseTag: TagName" << endl;
		    searchCount = 0;
		    ++src;
		    break;
		}
		if (searchCount > 0)
		{
		    if (curChar == commentStart[searchCount])
		    {
			searchCount++;
			if (searchCount == 4)
			{
#ifdef TOKEN_DEBUG
			    kdDebug( 6036 ) << "Found comment" << endl;
#endif
			    // Found '<!--' sequence
			    ++src;
			    dest = buffer; // ignore the previous part of this tag
			    comment = true;
			    searchCount = 0;
			    searchFor = commentEnd;
			    scriptCode = new QChar[ 1024 ];
			    scriptCodeSize = 0;
			    scriptCodeMaxSize = 1024;
			    parseComment(src);
			
			    return; // Finished parsing tag!
			}
			*dest = curChar.lower();
			dest++;
			++src;
			break;
		    }
		    else
		    {
			searchCount = 0; // Stop looking for '<!--' sequence
		    }
		}
		if( ((curChar.lower() >= 'a') && (curChar.lower() <= 'z')) ||
		    ((curChar >= '0') && (curChar <= '9')) ||
		    curChar == '/' )
		{
		    *dest = curChar.lower();
		    dest++;
		    ++src;
		}
		else
		{
		    int len;
                    bool beginTag;
		    QChar *ptr = buffer;
		    if (*ptr == '/')
		    {
			// End Tag
			beginTag = false;
			ptr++;
			len = dest - buffer - 1;
		    }
		    else
		    {
			// Start Tag
			beginTag = true;
			// Ignore CR/LF's after a start tag
			discard = LFDiscard;
			len = dest - buffer;
		    }
		    // limited xhtml support. Accept empty xml tags like <br/>
		    if(*(dest-1) == '/' && len > 1) len--;
		
		    QConstString tmp(ptr, len);
		    uint tagID = khtml::getTagID(tmp.string().ascii(), len);
		    if (!tagID) {
			kdDebug( 6036 ) << "Unknown tag: \"" << tmp.string() << "\"" << endl;
			dest = buffer;
			tag = SearchEnd; // ignore the tag
		    }
		    else
		    {
#ifdef TOKEN_DEBUG
			kdDebug( 6036 ) << "found tag id=" << tagID << endl;
#endif
			if (beginTag)
			    currToken->id = tagID;
			else
			    currToken->id = tagID + ID_CLOSE_TAG;
			
			dest = buffer;
			tag = SearchAttribute;
		    }
		}
		break;
	    }
	    case SearchAttribute:
	    {
		if( tquote )
		{
		    kdDebug( 6036 ) << "broken HTML in parseTag: SearchAttribute " << endl;
		    tquote=NoQuote;
		    ++src;
		    break;
		}		
		if( curChar == '>' )
		{
		    tag = SearchEnd; // we reached the end
		    break;
		}
		if( curChar.row() ) // we ignore everything that isn't ascii
		{
		    ++src;
		    break;
		}
		if( ((curChar.lower() >= 'a') && (curChar.lower() <= 'z')) ||
		    ((curChar >= '0') && (curChar <= '9')) ||
		    curChar == '-' )
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
		if( (((curChar.lower() >= 'a') && (curChar.lower() <= 'z')) ||
		     ((curChar >= '0') && (curChar <= '9')) ||
		     curChar == '-') && !tquote )
		{
		    *dest = curChar.lower();
		    dest++;
		    ++src;
		}
		else
		{
		    // beginning of name
		    QChar *ptr = buffer;
		    QConstString tmp(ptr, dest-buffer);
		    uint a = khtml::getAttrID(tmp.string().ascii(),
						     dest-buffer);
		    dest = buffer;
		    if (!a) {
			// unknown attribute, ignore
			kdDebug( 6036 ) << "Unknown attribute: \"" << tmp.string() << "\"" << endl;
                        *dest++ = 0x0; /* ignore */
		    }
		    else
		    {
#ifdef TOKEN_DEBUG
			kdDebug( 6036 ) << "Known attribute: \"" << tmp.string() << "\"" << endl;
#endif
			*dest++ = a;
		    }		
		    tag = SearchEqual;
		    discard = SpaceDiscard; // discard spaces before '='
		}
		break;
	    }
	    case SearchEqual:
	    {
		if( tquote )
		{
		      kdDebug( 6036 ) << "bad HTML in parseTag: SearchEqual" << endl;
		      // this is moslty due to a missing '"' somewhere before..
		      // so let's start searching for a new tag
		      tquote = NoQuote;
		
		      Attribute a;
		      a.id = *buffer;
		      if(a.id)
		      {
			  a.setValue(0, 0);
			  currToken->attrs.add(a);
		      }
		      dest = buffer;
		      tag = SearchAttribute;
		      discard = SpaceDiscard;
		      pending = NonePending;
		}
		else if( curChar == '=' )
 		{
		    tag = SearchValue;
		    pending = NonePending; // ignore spaces before '='
		    discard = SpaceDiscard; // discard spaces after '='
		    ++src;
		}
		else if( curChar == '>' )
		    tag = SearchEnd;
		else // other chars indicate a new attribte
		{
		    Attribute a;
		    a.id = *buffer;
		    if(a.id)
		    {
			a.setValue(0, 0);
			currToken->attrs.add(a);
		    }
		    dest = buffer;
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
	    }
	    case QuotedValue:
	    {
		// ### attributes like '&{blaa....};' are supposed to be treated as jscript.
		if ( curChar == '&' )
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
		    Attribute a;
		    a.id = *buffer;
		    // remove trailing spaces
		    while(*(dest-1) == ' ' && dest>buffer+1) dest--;
		    if(a.id)		    {
			a.setValue(buffer+1, dest-buffer-1);
			currToken->attrs.add(a);
		    }
		    dest = buffer;
		    tag = SearchAttribute;
		    discard = SpaceDiscard;
		    pending = NonePending;
		    break;
		}
		if( pending ) addPending();
		
		discard = NoneDiscard;
		*dest++ = curChar;
		++src;
		break;
	    }
	    case Value:
	    {
		if( tquote )
		{
		  // additional quote. discard it, and define as end of
		  // attribute
		    kdDebug( 6036 ) << "bad HTML in parseTag: Value" << endl;
		    ++src;
		    tquote = NoQuote;
		}
		if ( pending || curChar == '>' )
		{
		    // no quotes. Every space means end of value
		    Attribute a;
		    a.id = *buffer;
		    if(a.id)
		    {
			a.setValue(buffer+1, dest-buffer-1);
			currToken->attrs.add(a);
		    }
		    dest = buffer;
		    tag = SearchAttribute;
		    discard = SpaceDiscard;
		    pending = NonePending;
		    break;
		}
		*dest++ = curChar;
		++src;
		break;
	    }
	    case SearchEnd:
	    {
		if ( tquote || curChar != '>')
		{
		    ++src; // discard everything, until we found the end
		    break;
		}
		
		searchCount = 0; // Stop looking for '<!--' sequence
		tag = NoTag;
		pending = NonePending; // Ignore pending spaces
		++src;

		if ( currToken->id == 0 ) //stop if tag is unknown
		{
		    discard = NoneDiscard;
		    *dest = QChar::null;
		    return;
		}

		if(dest>buffer)
		{
		    // add the last attribute
		    Attribute a;
		    a.id = *buffer;
		    if(a.id)		    {
			a.setValue(buffer+1, dest-buffer-1);
			currToken->attrs.add(a);
		    }
		    dest = buffer;
		}
		uint tagID = currToken->id;
#ifdef TOKEN_DEBUG
		kdDebug( 6036 ) << "appending Tag: " << tagID << endl;
#endif
		bool beginTag = (tagID < ID_CLOSE_TAG);
		if(beginTag)
		{
		    // Ignore Space/LF's after a start tag
		    discard = AllDiscard;
		}
		else
		{
		    // Don't ignore CR/LF's after a close tag
		    discard = NoneDiscard;
		    tagID -= ID_CLOSE_TAG;
		}

		processToken();
		
		if(pre)
		{
		    // we have to take care to close the pre block in
		    // case we encounter an unallowed element....
		    if(!DOM::checkChild(ID_PRE, tagID))
		       pre = false;
		}

		if ( tagID == ID_PRE )
		{
		    prePos = 0;
		    pre = beginTag;
		}
		else if ( tagID == ID_TEXTAREA )
		{
		    textarea = beginTag;
		}
		else if ( tagID == ID_SCRIPT )
		{
		    if (beginTag)
		    {
			kdDebug( 6036 ) << "start of script, token->id = " << currToken->id << endl; 
			script = true;
			searchCount = 0;
			searchFor = scriptEnd;		
			scriptCode = new QChar[ 1024 ];
			scriptCodeSize = 0;
			scriptCodeMaxSize = 1024;
			parseScript(src);
			kdDebug( 6036 ) << "end of script" << endl;
		    }
		}
		else if ( tagID == ID_STYLE )
		{
		    if (beginTag)
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
		    if (beginTag)
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
		    select = beginTag;
		}
		return; // Finished parsing tag!
	    }
	    default:
	    {
		kdDebug( 6036 ) << "error in parseTag! " << __LINE__ << endl;
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
	    *dest++ = QChar(0xa0);
	    prePos++;
	    break;

	case LFPending:
	    *dest = '\n';
	    dest++;
	    prePos = 0;
	    break;
	  	
	case TabPending:
	    p = TAB_SIZE - ( prePos % TAB_SIZE );
	    for ( int x = 0; x < p; x++ )
	    {
		*dest = QChar(0xa0);
		dest++;
	    }
	    prePos += p;
	    break;
	
	default:
	    kdDebug( 6036 ) << "Assertion failed: pending = " << (int) pending << endl;
	    break;
	}
    }
    else
    {
    	*dest++ = ' ';
    }

    pending = NonePending;
}

void HTMLTokenizer::setPlainText()
{
    if (!plaintext)
    {
       // Do this only once!
       plaintext = true;
       currToken->id = ID_PLAIN;
       processToken();
       dest = buffer;
    }
}

void HTMLTokenizer::write( const QString &str )
{
    // we have to make this function reentrant. This is needed, because some
    // script code could call document.write(), which would add something here.
#ifdef TOKEN_DEBUG
    kdDebug( 6036 ) << "Tokenizer::write(\"" << str << "\")" << endl;
#endif

    if ( str.isEmpty() || buffer == 0L )
	return;

    if(!_src.isEmpty())
    {
	// reentrant...
	// we just insert the code at the tokenizers current position. Parsing will continue once
	// we return from the script stuff
	QString newStr = str;
	newStr += QString(src.current(), src.length());

	_src = newStr;
	src = DOMStringIt(_src);
	return;
    }	

    _src = str;
    src = DOMStringIt(_src);
    if(!currToken) currToken = new Token;

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
    else if (processingInstruction)
        parseProcessingInstruction(src);
    else if (tag)
    {
        parseTag(src);
    }
    else if (charEntity)
        parseEntity(src);

    while ( src.length() )
    {
	// do we need to enlarge the buffer?
	checkBuffer();

	if (skipLF && (src[0] != '\n'))
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
	    if (src[0] == '/')
	    {
		// Start of an End-Tag
		pending = NonePending; // Ignore leading Spaces/LFs
	    }
	    else if ( (src[0].lower() >= 'a') && (src[0].lower() <= 'z'))
	    {
		// Start of a Start-Tag
	    }
	    else if ( src[0] == '!')
	    {
		// <!-- comment -->
		searchCount = 1; // Look for '<!--' sequence to start comment
	    }
	    else if( src[0] == '?' )
	    {
		// xml processing instruction
		processingInstruction = true;
		parseProcessingInstruction(src);
		continue;
	    }
	    else
	    {
		// Invalid tag
		// Add as is
		if (pending)
		    addPending();
		*dest = '<';
		dest++;
		*dest++ = src[0];
		++src;
		continue;	
	    }

	    if(pending) addPending();

	    processToken();

	    tag = TagName;
	    parseTag(src);
	}
	else if ( src[0] == '&' )
	{
            ++src;
	
	    discard = NoneDiscard;
	    if (pending)
	    	addPending();
	
	    charEntity = true;
            parseEntity(src, true);
	}
	else if ( src[0] == '<')
	{
	    ++src;
	    startTag = true;
	    discard = NoneDiscard;
	}
	else if (( src[0] == '\n' ) || ( src[0] == '\r' ))
	{
	    if ( pre || textarea)
	    {
	    	if (discard == LFDiscard || discard == AllDiscard)
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
	    	else if(discard == AllDiscard)
		{
		}
		else
	    	{
	    	    // Process this LF
	    	    if (pending == NonePending)
			pending = LFPending;
		}
	    }
	    /* Check for MS-DOS CRLF sequence */
	    if (src[0] == '\r')
	    {
		skipLF = true;
	    }
	    ++src;
	}
	else if (( src[0] == ' ' ) || ( src[0] == '\t' ))
	{
	    if ( pre || textarea)
	    {
	    	if (pending)
	    	    addPending();
	    	if (src[0] == ' ')
	    	    pending = SpacePending;
	    	else
	    	    pending = TabPending;
	    }
	    else
	    {
		if(discard == SpaceDiscard)
		    discard = NoneDiscard;
		else if(discard == AllDiscard)
		{ }
		else
		    pending = SpacePending;
	    }
	    ++src;
	}
	else
	{
	    if (pending)
	    	addPending();

	    discard = NoneDiscard;
	    if ( pre )
	    {
		prePos++;
	    }
	
	    *dest++ = src[0];
	    ++src;
	}
    }

    _src = QString();
}

void HTMLTokenizer::end()
{
    if ( buffer == 0 )
	return;

    processToken();

    delete [] buffer;
    buffer = 0;
}

void HTMLTokenizer::processToken()
{
    if ( dest > buffer )
    {
	if(currToken->id && currToken->id != ID_COMMENT)
	    kdDebug( 6036 ) << "Error in processToken!!!" << endl;
/*
	if(!pre && dest - buffer == 1 && *buffer == ' ')
	{
	    dest = buffer;
	    return;
	}
*/
        currToken->text = DOMString( buffer, dest-buffer);
        if (currToken->id != ID_COMMENT)
	    currToken->id = ID_TEXT;
    }
    else if(!currToken->id)
    {
//      kdDebug( 6036 ) << "Empty token!" << endl;
	return;
    }
    dest = buffer;

#ifdef TOKEN_PRINT
    QString name = getTagName(currToken->id).string();
    QString text = currToken->text.string();

    kdDebug( 6036 ) << "Token --> " << name << "   id = " << currToken->id << endl;
    if(currToken->text != 0)
	kdDebug( 6036 ) << "text: \"" << text << "\"" << endl;
#else
#ifdef TOKEN_DEBUG
    QString name = getTagName(currToken->id).string();
    QString text = currToken->text.string();

    kdDebug( 6036 ) << "Token --> " << name << "   id = " << currToken->id << endl;
    if(currToken->text != 0)
	kdDebug( 6036 ) << "text: \"" << text << "\"" << endl;
    int l = currToken->attrs.length();
    if(l>0)
    {
	int i = 0;
	kdDebug( 6036 ) << "Attributes: " << l << endl;
	while(i<l)
	{
	    name = currToken->attrs.name(i).string();
	    text = currToken->attrs.value(i).string();
	    kdDebug( 6036 ) << "    " << currToken->attrs.id(i) << " " << name << "=" << text << endl;
	    i++;
	}
    }
    kdDebug( 6036 ) << endl;
#endif
#endif
    // pass the token over to the parser, the parser deletes the token
    parser->parseToken(currToken);

    currToken = new Token;
}


HTMLTokenizer::~HTMLTokenizer()
{
    reset();
}


void HTMLTokenizer::checkBuffer(int len)
{
	// do we need to enlarge the buffer?
	if ( (dest - buffer) > size-len )
	{
	    QChar *newbuf = new QChar [ size + 1024 ];
	    memcpy( newbuf, buffer, (dest - buffer + 1)*sizeof(QChar) );
	    dest = newbuf + ( dest - buffer );
	    delete [] buffer;
	    buffer = newbuf;
	    size += 1024;
	}
}

//-----------------------------------------------------------------------------
#if 0
int ustrlen( const QChar *c )
{
    int l = 0;
    while( *c++ != QChar::null ) l++;
    return l;
}

QChar *ustrchr( const QChar *c, const QChar s )
{
    while( *c != QChar::null )
    {
	if( *c == s ) return (QChar *)c;
	c++;
    }
    return 0L;
}
#endif
