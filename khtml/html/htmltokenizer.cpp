/*
    This file is part of the KDE libraries

    Copyright (C) 1997 Martin Jones (mjones@kde.org)
              (C) 1997 Torben Weis (weis@kde.org)
              (C) 1998 Waldo Bastian (bastian@kde.org)
              (C) 1999 Lars Knoll (knoll@kde.org)
              (C) 1999 Antti Koivisto (koivisto@kde.org)
              (C) 2000 Dirk Mueller (mueller@kde.org)

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
#include "misc/loader.h"
#include "khtmlview.h"
#include "khtml_part.h"
#include "htmlparser.h"
#include "htmltoken.h"
#include "html_documentimpl.h"
#include "dtd.h"
#include "htmlhashes.h"
#include <kcharsets.h>
#include <kglobal.h>
#include <assert.h>

#include <kdebug.h>

#include "kjs.h"

using namespace khtml;

static const QChar commentStart [] = { '<','!','-','-' };
static const QChar commentEnd [] = { '-','-','>' };
static const QChar scriptEnd [] = { '<','/','s','c','r','i','p','t','>' };
static const QChar styleEnd [] = { '<','/','s','t','y','l','e','>' };
static const QChar listingEnd [] = { '<','/','l','i','s','t','i','n','g','>' };
static const QChar textareaEnd [] = { '<','/','t','e','x','t','a','r','e','a','>' };

#define QT_ALLOC_QCHAR_VEC( N ) (QChar*) new char[ sizeof(QChar)*( N ) ]
#define QT_DELETE_QCHAR_VEC( P ) delete[] ((char*)( P ))

// ----------------------------------------------------------------------------

HTMLTokenizer::HTMLTokenizer(KHTMLParser *p, KHTMLView *_view)
{
    view = _view;
    buffer = 0;
    scriptCode = 0;
    charsets = KGlobal::charsets();
    parser = p;
    currToken = 0;
    cachedScript = 0;
    executingScript = false;

    reset();
}

void HTMLTokenizer::reset()
{
    assert(executingScript == false);
    if (cachedScript)
        cachedScript->deref(this);
    cachedScript = 0;

    if ( buffer )
        QT_DELETE_QCHAR_VEC(buffer);
    buffer = 0;
    size = 0;

    if ( scriptCode )
        QT_DELETE_QCHAR_VEC(scriptCode);
    scriptCode = 0;

    delete currToken;
    currToken = 0;
    //parser->reset();
}

void HTMLTokenizer::begin()
{
    executingScript = false;
    reset();
    currToken = 0;
    size = 4095;
    buffer = QT_ALLOC_QCHAR_VEC( 4096 );
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
    loadingExtScript = false;
    scriptSrc = "";
    pendingSrc = "";
    scriptOutput = "";
    noMoreData = false;
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
    // We are inside a <script>, <style>, <textarea> or comment. Look for the end tag
    // which is either </script>, </style> , </textarea> or -->
    // otherwise print out every received character
    if (charEntity) {
        QChar *scriptCodeDest = scriptCode+scriptCodeSize;
        parseEntity(src,scriptCodeDest);
        scriptCodeSize = scriptCodeDest-scriptCode;
    }

#ifdef TOKEN_DEBUG
    kdDebug( 6036 ) << "HTMLTokenizer::parseListing()" << endl;
#endif

    bool doScriptExec = false;

    while ( src.length() )
    {
        // do we need to enlarge the buffer?
        checkBuffer();

        // Allocate memory to store the script. We will write maximal
        // 10 characers.
        if ( scriptCodeSize + 10 > scriptCodeMaxSize )
        {
            int newsize = QMAX(scriptCodeMaxSize*2, scriptCodeMaxSize+1024);
            QChar *newbuf = QT_ALLOC_QCHAR_VEC( newsize );
            memcpy( newbuf, scriptCode, scriptCodeSize*sizeof(QChar) );
            QT_DELETE_QCHAR_VEC(scriptCode);
            scriptCode = newbuf;
            scriptCodeMaxSize = newsize;
        }

        char ch = src[0].latin1();
        if ( ( ch == '>' ) && ( searchFor[ searchCount ] == '>'))
        {
            ++src;
            scriptCode[ scriptCodeSize ] = 0;
            scriptCode[ scriptCodeSize + 1 ] = 0;
            if (comment) currToken->id = ID_COMMENT; /// ###
            if (script)
            {
                if (!scriptSrc.isEmpty()) {
                    // forget what we just got; load from src url instead
                    cachedScript = parser->doc()->docLoader()->requestScript(scriptSrc, parser->doc()->baseURL());
                }
                else {
#ifdef TOKEN_DEBUG
                    kdDebug( 6036 ) << "---START SCRIPT---" << endl;
                    kdDebug( 6036 ) << QString(scriptCode, scriptCodeSize) << endl;
                    kdDebug( 6036 ) << "---END SCRIPT---" << endl;
#endif
                    // Parse scriptCode containing <script> info
                    doScriptExec = true;
                }
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
            processToken();
            if(script)
                currToken->id = ID_SCRIPT + ID_CLOSE_TAG;
            else if(style)
                currToken->id = ID_STYLE + ID_CLOSE_TAG;
            else if (comment)
                currToken->id = ID_COMMENT + ID_CLOSE_TAG;
	    else if (textarea)
		currToken->id = ID_TEXTAREA + ID_CLOSE_TAG;
            else
                currToken->id = ID_LISTING + ID_CLOSE_TAG;
            processToken();
            if (cachedScript) {
                cachedScript->ref(this);
                if (cachedScript) { // will be 0 if script was already loaded and ref() executed it
                    loadingExtScript = true;
                    pendingSrc = QString(src.current(), src.length());
                    _src = "";
                    src = DOMStringIt();
                }
            }
            else if (doScriptExec && javascript) {
                executingScript = true;
                view->part()->executeScript(QString(scriptCode, scriptCodeSize));
                executingScript = false;
            }
            script = style = listing = comment = textarea = false;
            if(scriptCode)
                QT_DELETE_QCHAR_VEC(scriptCode);
            scriptCode = 0;

            addScriptOutput();

            return; // Finished parsing script/style/comment/listing
        }
        // Find out wether we see an end tag without looking at
        // any other then the current character, since further characters
        // may still be on their way thru the web!
        else if ( searchCount > 0 )
        {
            const QChar& cmp = src[0];
            // broken HTML: "--->"
            if (comment && searchCount == 2 && cmp.latin1() == '-' && searchBuffer[0].latin1() != '<')
            {
                scriptCode[ scriptCodeSize++ ] = cmp;
                ++src;
            }
            // broken HTML: "--!>"
            else if (comment && searchCount == 2 && cmp.latin1() == '!' && searchBuffer[0].latin1() != '<')
            {
                ++src;
            }
            // be tolerant: skip spaces before the ">", i.e "</script >"
            else if (!comment && cmp.isSpace() && searchFor[searchCount].latin1() == '>')
            {
                ++src;
            }
            else if ( cmp.lower() == searchFor[ searchCount ] )
            {
                searchBuffer[ searchCount++ ] = cmp;
                ++src;
            }
            // We were wrong => print all buffered characters and the current one;
            else
            {
                searchBuffer[ searchCount ] = 0;
		DOMStringIt pit(searchBuffer,searchCount);
		while (pit.length()) {
		    if (textarea && pit[0] == '&') {
			QChar *scriptCodeDest = scriptCode+scriptCodeSize;
			++pit;
			parseEntity(pit,scriptCodeDest,true);
			scriptCodeSize = scriptCodeDest-scriptCode;
		    }
		    else {
			scriptCode[ scriptCodeSize++ ] = pit[0];
			++pit;
		    }
		}
                searchCount = 0;
            }
        }
        // Is this perhaps the start of the </script> or </style> tag, or --> (end of comment)?
        else if ( ch == '<' || ch == '-' )
        {
            searchCount = 1;
            searchBuffer[ 0 ] = src[0];
            ++src;
        }
	else
        {
	    if (textarea && ch == '&') {
		QChar *scriptCodeDest = scriptCode+scriptCodeSize;
		++src;
		parseEntity(src,scriptCodeDest,true);
		scriptCodeSize = scriptCodeDest-scriptCode;
	    }
	    else {
		scriptCode[ scriptCodeSize++ ] = src[0];
		++src;
	    }
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
    parseListing(src); // ### disabled temporarily - skips body attrs. if placed before <HTML>
}

void HTMLTokenizer::parseProcessingInstruction(DOMStringIt &src)
{
    while ( src.length() )
    {
        char chbegin = src[0].latin1();
        // Look for '?>'
        if ( chbegin == '?' )
        {
            if (searchCount < 1)        // Watch out for '--->'
                searchCount++;
        }
        else if ((searchCount == 1) && (chbegin == '>'))
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

        // ascii is okay because we only do ascii comparisons
        char chbegin = src[0].latin1();

        if (skipLF && ( chbegin != '\n' ))
        {
            skipLF = false;
        }

        if (skipLF)
        {
            skipLF = false;
            ++src;
        }
        else if (( chbegin == '\n' ) || ( chbegin == '\r' ))
        {
            processToken();

            /* Check for MS-DOS CRLF sequence */
            if (chbegin == '\r')
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

void HTMLTokenizer::parseEntity(DOMStringIt &src, QChar *&dest, bool start)
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
        else
        // end of entity... try to decode it
        {
            QChar res;
            if(entityPos > 1)
            {
                QConstString cStr(entityBuffer, entityPos);
                res = charsets->fromEntity(cStr.string());

                //kdDebug( 6036 ) << "ENTITY " << res.unicode() << ", " << res << endl;

                if (tag && src[0] != ';' ) {
                    // Don't translate entities in tags with a missing ';'
                    res = QChar::null;
                }

                // Partial support for MS Windows Latin-1 extensions
                // full list http://www.bbsinc.com/iso8859.html
                // There may be better equivalents
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
#ifdef TOKEN_DEBUG
                kdDebug( 6036 ) << "unknown entity!" << endl;
#endif

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
        parseEntity(src,dest);

    while ( src.length() )
    {
        checkBuffer();
        char curchar = src[0].latin1();

        // decide if quoted or not....
        if ( curchar == '\"' || curchar == '\'' )
        { // we treat " & ' the same in tags
            if ( tquote == NoQuote )
            {
                // according to HTML4 DTD, we can simplify
                // strings like "  my \nstring " to "my string"
                discard = SpaceDiscard; // ignore leading spaces
                pending = NonePending;
                if (curchar == '\'')
                    tquote = SingleQuote;
                else
                    tquote = DoubleQuote;
            }
            else if ( (( tquote == SingleQuote )&&( curchar == '\'')) ||
                      (( tquote == DoubleQuote )&&( curchar == '\"')) )
            {
                tquote = IgnoreQuote;
                discard = NoneDiscard;
                pending = NonePending; // remove space at the end of value

            }
            else if (tquote == IgnoreQuote)
            {
                // we remove additional quotes directly following the
                // end of the quoted section. Helps with bad html as
                // <tag attr="value"" nextattr="..." ...>
            }
            else
            {
                *dest++ = src[0];
            }
            ++src;
        }
        else if ( discard != NoneDiscard &&
                  ( curchar == ' ' || curchar == '\t' ||
                    curchar == '\n' || curchar == '\r' ) )
        {
            pending = SpacePending;
            ++src;
        }
        else
        {
            if (tquote == IgnoreQuote)
                tquote = NoQuote;

            switch(tag) {
            case NoTag:
            {
                return;
            }
            case TagName:
            {
                if( tquote )
                {
#ifdef TOKEN_DEBUG
                    kdDebug( 6036 ) << "bad HTML in parseTag: TagName" << endl;
#endif
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
#ifdef TOKEN_DEBUG
                            kdDebug( 6036 ) << "Found comment" << endl;
#endif
                            // Found '<!--' sequence
                            ++src;
                            dest = buffer; // ignore the previous part of this tag
                            comment = true;
                            searchCount = 0;
                            searchFor = commentEnd;
                            scriptCode = QT_ALLOC_QCHAR_VEC( 1024 );
                            scriptCodeSize = 0;
                            scriptCodeMaxSize = 1024;
                            tag = NoTag;
                            parseComment(src);

                            return; // Finished parsing tag!
                        }
                        *dest = src[0].lower();
                        dest++;
                        ++src;
                        break;
                    }
                    else
                    {
                        searchCount = 0; // Stop looking for '<!--' sequence
                    }
                }

                curchar = src[0].latin1();
                if( ((curchar >= 'a') && (curchar <= 'z')) ||
                    ((curchar >= 'A') && (curchar <= 'Z')) ||
                    ((curchar >= '0') && (curchar <= '9')) ||
                    curchar == '/' )
                {
                    *dest = src[0].lower();
                    dest++;
                    ++src;
                }
                else
                {
                    int len = dest - buffer;
                    bool beginTag;
                    QChar *ptr = buffer;
                    if ((len > 0) && (*ptr == '/'))
                    {
                        // End Tag
                        beginTag = false;
                        ptr++;
                        len--;
                    }
                    else
                    {
                        // Start Tag
                        beginTag = true;
                        // Ignore CR/LF's after a start tag
                        discard = LFDiscard;
                    }
                    // limited xhtml support. Accept empty xml tags like <br/>
                    if((len > 1) && (*(dest-1) == '/')) len--;

                    QConstString tmp(ptr, len);
                    uint tagID = khtml::getTagID(tmp.string().ascii(), len);
                    if (!tagID) {
#ifdef TOKEN_DEBUG
                        kdDebug( 6036 ) << "Unknown tag: \"" << tmp.string() << "\"" << endl;
#endif
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
#ifdef TOKEN_DEBUG
                    kdDebug( 6036 ) << "broken HTML in parseTag: SearchAttribute " << endl;
#endif
                    tquote=NoQuote;
                    ++src;
                    break;
                }
                curchar = src[0].latin1();
                if( curchar == '>' )
                {
                    tag = SearchEnd; // we reached the end
                    break;
                }
                if( !curchar ) // we ignore everything that isn't ascii
                {
                    ++src;
                    break;
                }
                if( ((curchar >= 'a') && (curchar <= 'z')) ||
                    ((curchar >= 'A') && (curchar <= 'Z')) ||
                    ((curchar >= '0') && (curchar <= '9')) ||
                    curchar == '-' )
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
                if( (((curchar >= 'a') && (curchar <= 'z')) ||
                     ((curchar >= 'A') && (curchar <= 'Z')) ||
                     ((curchar >= '0') && (curchar <= '9')) ||
                     curchar == '-') && !tquote )
                {
                    *dest = src[0].lower();
                    dest++;
                    ++src;
                }
                else
                {
                    // beginning of name
                    QChar *ptr = buffer;
                    attrName = QString(ptr, dest-buffer);
                    uint a = khtml::getAttrID(attrName.ascii(), dest-buffer);
                    dest = buffer;
                    *dest++ = a;

                    if (!a) {
#ifdef TOKEN_DEBUG
                       kdDebug( 6036 ) << "Unknown attribute: \"" << attrName << "\"" << endl;
#endif
                    } else
                    {
#ifdef TOKEN_DEBUG
                       kdDebug( 6036 ) << "Known attribute: \"" << attrName << "\"" << endl;
#endif
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
#ifdef TOKEN_DEBUG
                      kdDebug( 6036 ) << "bad HTML in parseTag: SearchEqual" << endl;
#endif
                      // this is moslty due to a missing '"' somewhere before..
                      // so let's start searching for a new tag
                      tquote = NoQuote;

                      Attribute a;
                      a.id = *buffer;
                      if(a.id==0) a.setName( attrName );
                      a.setValue(0, 0);
                      currToken->attrs.add(a);

                      dest = buffer;
                      tag = SearchAttribute;
                      discard = SpaceDiscard;
                      pending = NonePending;
                }
                else if( curchar == '=' )
                {
#ifdef TOKEN_DEBUG
                    kdDebug(6036) << "found equal" << endl;
#endif
                    tag = SearchValue;
                    pending = NonePending; // ignore spaces before '='
                    discard = SpaceDiscard; // discard spaces after '='
                    ++src;
                }
                else if( curchar == '>' )
                    tag = SearchEnd;
                else // other chars indicate a new attribte
                {
                    Attribute a;
                    a.id = *buffer;
                    if(a.id==0) a.setName( attrName );
                    a.setValue(0, 0);
                    currToken->attrs.add(a);

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
                break;
            }
            case QuotedValue:
            {
                // ### attributes like '&{blaa....};' are supposed to be treated as jscript.
                if ( curchar == '&' )
                {
                    ++src;

                    discard = NoneDiscard;
                    if (pending)
                        addPending();

                    charEntity = true;
                    parseEntity(src, dest, true);
                    break;
                }
                else if ( !tquote )
                {
                    // end of attribute
                    Attribute a;
                    a.id = *buffer;
                    if(a.id==0) a.setName( attrName );
                    while(*(dest-1) == ' ' && dest>buffer+1) dest--; // remove trailing spaces
                    a.setValue(buffer+1, dest-buffer-1);
#ifdef TOKEN_DEBUG
                    kdDebug() << "adding value: *" << QConstString(buffer+1, dest-buffer-1).string() << "*" << endl;
#endif

                    currToken->attrs.add(a);

                    dest = buffer;
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
            {
                if ( curchar == '&' )
                {
                    ++src;

                    discard = NoneDiscard;
                    if (pending)
                        addPending();

                    charEntity = true;
                    parseEntity(src, dest, true);
                    break;
                }
                else if( tquote )
                {
                  // additional quote. discard it, and define as end of
                  // attribute
#ifdef TOKEN_DEBUG
                    kdDebug( 6036 ) << "bad HTML in parseTag: Value" << endl;
#endif
                    ++src;
                    tquote = NoQuote;
                }
                // if discard==NoneDiscard at this point, it means
                // that we passed an empty "" pair. bit hacky, but...
                // helps with <tag attr=""otherattr="something">
                if ( pending || src[0].latin1() == '>' || discard==NoneDiscard)
                {
                    // no quotes. Every space means end of value
                    Attribute a;
                    a.id = *buffer;
                    if(a.id==0) a.setName( attrName );
                    a.setValue(buffer+1, dest-buffer-1);
#ifdef TOKEN_DEBUG
                    kdDebug() << "adding value: *" << QConstString(buffer+1, dest-buffer-1).string() << "*" << endl;
#endif
                    currToken->attrs.add(a);

                    dest = buffer;
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
                if ( curchar != '>')
                {
                    ++src; // discard everything, until we found the end
                    break;
                }

                searchCount = 0; // Stop looking for '<!--' sequence
                tag = NoTag;
                tquote = NoQuote;
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
                    if(a.id==0) a.setName( attrName );
                    a.setValue(buffer+1, dest-buffer-1);
                    currToken->attrs.add(a);
                    dest = buffer;
                }
                uint tagID = currToken->id;
#ifdef TOKEN_DEBUG
                kdDebug( 6036 ) << "appending Tag: " << tagID << endl;
#endif
                bool beginTag = (tagID < ID_CLOSE_TAG);
                if( beginTag && tagID != ID_IMG && tagID != ID_INPUT ) {
                    // Ignore Space/LF's after a start tag
                    discard = LFDiscard;
                } else if (!beginTag) {
                    // Don't ignore CR/LF's after a close tag
                    discard = NoneDiscard;
                    tagID -= ID_CLOSE_TAG;
                }

                if ( tagID == ID_SCRIPT  && beginTag ) {
                    int attrIndex = currToken->attrs.find(ATTR_SRC);
                    scriptSrc = (attrIndex == -1 ? (QString)"" : currToken->attrs[attrIndex]->value().string());
                    attrIndex = currToken->attrs.find(ATTR_LANGUAGE);
                    javascript = true;
                    if( attrIndex != -1 ) {
                        QString lang = currToken->attrs[attrIndex]->value().string();
                        lang = lang.lower();
                        if( !lang.contains("javascript") &&
                            !lang.contains("ecmascript") &&
                            !lang.contains("jscript") )
                           javascript = false;
                    } else {
                        attrIndex = currToken->attrs.find(ATTR_TYPE);
                        if( attrIndex != -1 ) {
                            QString lang = currToken->attrs[attrIndex]->value().string();
                            lang = lang.lower();
                            if( !lang.contains("javascript") &&
                                !lang.contains("ecmascript") &&
                                !lang.contains("jscript") )
                                javascript = false;
                        }
                    }
                }

                processToken();

                if(pre)
                {
                    // we have to take care to close the pre block in
                    // case we encounter an unallowed element....
                    if(!DOM::checkChild(ID_PRE, tagID)) {
                        //kdDebug(0) << " not allowed in <pre> " << (int)tagID << endl;
                        pre = false;
                    }
                }

                if ( tagID == ID_PRE )
                {
                    prePos = 0;
                    pre = beginTag;
                }
                else if ( tagID == ID_TEXTAREA )
                {
                    if(beginTag) {
                        listing = true;
			textarea = true;
                        searchCount = 0;
                        searchFor = textareaEnd;
                        scriptCode = QT_ALLOC_QCHAR_VEC( 1024 );
                        scriptCodeSize = 0;
                        scriptCodeMaxSize = 1024;
                        parseListing(src);
                    }
                }
                else if ( tagID == ID_SCRIPT )
                {
                    if (beginTag)
                    {
#ifdef TOKEN_DEBUG
                        kdDebug( 6036 ) << "start of script, token->id = " << currToken->id << endl;
#endif
                        script = true;
                        searchCount = 0;
                        searchFor = scriptEnd;
                        scriptCode = QT_ALLOC_QCHAR_VEC( 1024 );
                        scriptCodeSize = 0;
                        scriptCodeMaxSize = 1024;
                        parseScript(src);
#ifdef TOKEN_DEBUG
                        kdDebug( 6036 ) << "end of script" << endl;
#endif
                    }
                }
                else if ( tagID == ID_STYLE )
                {
                    if (beginTag)
                    {
                        style = true;
                        searchCount = 0;
                        searchFor = styleEnd;
                        scriptCode = QT_ALLOC_QCHAR_VEC( 1024 );
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
                        scriptCode = QT_ALLOC_QCHAR_VEC( 1024 );
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
#ifdef TOKEN_DEBUG
                kdDebug( 6036 ) << "error in parseTag! " << __LINE__ << endl;
#endif
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
            *dest++ = QChar(' ');
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
                *dest = QChar(' ');
                dest++;
            }
            prePos += p;
            break;

        default:
#ifdef TOKEN_DEBUG
            kdDebug( 6036 ) << "Assertion failed: pending = " << (int) pending << endl;
#endif
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

    // reentrant...
    // we just insert the code at the tokenizers current position. Parsing will continue once
    // we return from the script stuff
    // (this won't happen if we're in the middle of loading an external script)
    if(executingScript) {
#ifdef TOKEN_DEBUG
        kdDebug( 6036 ) << "adding to scriptOutput" << endl;
#endif
        scriptOutput += str;
        return;
    }

    if (loadingExtScript) {
        // don't parse; we will do this later
        pendingSrc += str;
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
        parseEntity(src, dest);

    while ( src.length() )
    {
        // do we need to enlarge the buffer?
        checkBuffer();

        // doesn't hurt because we only do ASCII comparisons
        // use chbegin here instead of comparing equality with "src[0]"
        // because this is slow (two function calls)
        char chbegin = src[0].latin1();

        if (skipLF && (chbegin != '\n'))
            skipLF = false;

        if (skipLF)
        {
            skipLF = false;
            ++src;
        }
        else if ( startTag )
        {
            startTag = false;

            switch(chbegin) {
            case '/':
            {
                // Start of an End-Tag
                if(pending == LFPending)
                    pending = NonePending; // Ignore leading Spaces/LFs

                break;
            }

            case '!':
            {
                // <!-- comment -->
                searchCount = 1; // Look for '<!--' sequence to start comment

                break;
            }

            case '?':
            {
                // xml processing instruction
                processingInstruction = true;
                parseProcessingInstruction(src);
                continue;

                break;
            }

            default:
            {

                if( ((chbegin >= 'a') && (chbegin <= 'z')) || ((chbegin >= 'A') && (chbegin <= 'Z')))
                {
                    // Start of a Start-Tag
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
            }
            }; // end case

            if(pending) addPending();

            processToken();

            tag = TagName;
            parseTag(src);
        }
        else if ( chbegin == '&' )
        {
            ++src;

            discard = NoneDiscard;
            if (pending)
                addPending();

            charEntity = true;
            parseEntity(src, dest, true);
        }
        else if ( chbegin == '<')
        {
            ++src;
            startTag = true;
            discard = NoneDiscard;
        }
        else if (( chbegin == '\n' ) || ( chbegin == '\r' ))
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
            if (chbegin == '\r')
            {
                skipLF = true;
            }
            ++src;
        }
        else if (( chbegin == ' ' ) || ( chbegin == '\t' ))
        {
            if ( pre || textarea)
            {
                if (pending)
                    addPending();
                if (chbegin == ' ')
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
            unsigned char row = src[0].row();
            if ( row > 0x05 && row < 0x10 || row > 0xfd )
                    currToken->complexText = true;
            *dest++ = src[0];
            ++src;
        }
    }

    _src = QString();
    if (noMoreData && !cachedScript)
        end(); // this actually causes us to be deleted
}

void HTMLTokenizer::end()
{
    if ( buffer == 0 ) {
        emit finishedParsing();
        return;
    }

    if(currToken) processToken();

    if(buffer)
        QT_DELETE_QCHAR_VEC(buffer);

    if(scriptCode)
        QT_DELETE_QCHAR_VEC(scriptCode);

    scriptCode = 0;
    buffer = 0;
    emit finishedParsing();
}

void HTMLTokenizer::finish()
{
    // do this as long as we don't find matching comment ends
    while(comment && scriptCode && scriptCodeSize > 0)
    {
        // we've found an unmatched comment start
        scriptCode[ scriptCodeSize ] = 0;
        scriptCode[ scriptCodeSize + 1 ] = 0;
        int pos = QConstString(scriptCode, scriptCodeSize).string().find('>');
        QString food;
        food.setUnicode(scriptCode+pos+1, scriptCodeSize-pos-1); // deep copy
        QT_DELETE_QCHAR_VEC(scriptCode);
        scriptCode = 0;
        script = style = listing = comment = textarea = false;
        scriptCodeSize = 0;
        write(food);
    }
    // this indicates we will not recieve any more data... but if we are waiting on
    // an external script to load, we can't finish parsing until that is done
    noMoreData = true;
    if (!loadingExtScript && !executingScript)
        end(); // this actually causes us to be deleted
}

void HTMLTokenizer::processToken()
{
    if ( dest > buffer )
    {
#ifdef TOKEN_DEBUG
        if(currToken->id && currToken->id != ID_COMMENT)
            kdDebug( 6036 ) << "Error in processToken!!!" << endl;
#endif
        if ( currToken->complexText ) {
            // ### we do too much QString copying here, but better here than in RenderText...
            // anyway have to find a better solution in the long run (lars)
            QString s = QConstString(buffer, dest-buffer).string();
            s.compose();
            currToken->text = DOMString( s );
        } else
            currToken->text = DOMString( buffer, dest - buffer );
        if (currToken->id != ID_COMMENT)
            currToken->id = ID_TEXT;
    }
    else if(!currToken->id)
        return;

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
    // pass the token over to the parser, the parser DOES NOT delete the token
    parser->parseToken(currToken);

    // ### FIXME: make this faster
    delete currToken;
    currToken = new Token;
}


HTMLTokenizer::~HTMLTokenizer()
{
    reset();
}


void HTMLTokenizer::enlargeBuffer()
{
    QChar *newbuf = QT_ALLOC_QCHAR_VEC( size*2 );
    memcpy( newbuf, buffer, (dest - buffer + 1)*sizeof(QChar) );
    dest = newbuf + ( dest - buffer );
    QT_DELETE_QCHAR_VEC(buffer);
    buffer = newbuf;
    size *= 2;
}

void HTMLTokenizer::notifyFinished(CachedObject *finishedObj)
{
    if (finishedObj == cachedScript) {
#ifdef TOKEN_DEBUG
        kdDebug( 6036 ) << "Finished loading an external script" << endl;
#endif
        loadingExtScript = false;
        DOMString scriptSource = cachedScript->script();
#ifdef TOKEN_DEBUG
        kdDebug( 6036 ) << "External script is:" << endl << scriptSource.string() << endl;
#endif
        cachedScript->deref(this);
        cachedScript = 0;
        executingScript = true;
        view->part()->executeScript(scriptSource.string());
        executingScript = false;

        // 'script' is true when we are called synchronously from
        // parseScript(). In that case parseScript() will take care
        // of 'scriptOutput'.
        if (!script)
        {
           QString rest = scriptOutput+pendingSrc;
           scriptOutput = pendingSrc = "";
           write(rest);
        }
    }
}

void HTMLTokenizer::addScriptOutput()
{
    if ( !scriptOutput.isEmpty() ) {
//      kdDebug( 6036 ) << "adding scriptOutput to parsed string" << endl;
        QString newStr = scriptOutput;
        newStr += QString(src.current(), src.length());
        _src = newStr;
        src = DOMStringIt(_src);
        scriptOutput = "";
    }
}
#include "htmltokenizer.moc"
