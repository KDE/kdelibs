/*
    This file is part of the KDE libraries

    Copyright (C) 1997 Martin Jones (mjones@kde.org)
              (C) 1997 Torben Weis (weis@kde.org)
              (C) 1998 Waldo Bastian (bastian@kde.org)
              (C) 1999 Lars Knoll (knoll@kde.org)
              (C) 1999 Antti Koivisto (koivisto@kde.org)
              (C) 2001 Dirk Mueller (mueller@kde.org)

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

//#define TOKEN_DEBUG 1
//#define TOKEN_DEBUG 2

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

//#include <string.h>
#include "html/htmltokenizer.h"
#include "html/html_documentimpl.h"
#include "html/htmlparser.h"
#include "html/dtd.h"

#include "misc/loader.h"
#include "misc/htmlhashes.h"

#include "khtmlview.h"
#include "khtml_part.h"
#include "xml/dom_docimpl.h"
#include "css/csshelper.h"
#include "ecma/kjs_proxy.h"
#include <kcharsets.h>
#include <kglobal.h>
#include <ctype.h>
#include <assert.h>
#include <qvariant.h>
#include <kdebug.h>
#include <stdlib.h>

#include "kentities.c"

using namespace khtml;

static const QChar commentStart [] = { '<','!','-','-', QChar::null };

static const char scriptEnd [] = "</script";
static const char xmpEnd [] = "</xmp";
static const char styleEnd [] =  "</style";
static const char textareaEnd [] = "</textarea";
static const char titleEnd [] = "</title";

#define KHTML_ALLOC_QCHAR_VEC( N ) (QChar*) malloc( sizeof(QChar)*( N ) )
#define KHTML_REALLOC_QCHAR_VEC(P, N ) (QChar*) P = realloc(p, sizeof(QChar)*( N ))
#define KHTML_DELETE_QCHAR_VEC( P ) free((char*)( P ))

// Full support for MS Windows extensions to Latin-1.
// Technically these extensions should only be activated for pages
// marked "windows-1252" or "cp1252", but
// in the standard Microsoft way, these extensions infect hundreds of thousands
// of web pages.  Note that people with non-latin-1 Microsoft extensions
// are SOL.
//
// See: http://www.microsoft.com/globaldev/reference/WinCP.asp
//      http://www.bbsinc.com/iso8859.html
//      http://www.obviously.com/
//
// There may be better equivalents
#if 0
#define fixUpChar(x)
#else
#define fixUpChar(x) \
            if (!(x).row() ) { \
                switch ((x).cell()) \
                { \
                /* ALL of these should be changed to Unicode SOON */ \
                case 0x80: (x) = 0x20ac; break; \
                case 0x82: (x) = ',';    break; \
                case 0x83: (x) = 0x0192; break; \
                case 0x84: (x) = '"';    break; \
                case 0x85: (x) = 0x2026; break; \
                case 0x86: (x) = 0x2020; break; \
                case 0x87: (x) = 0x2021; break; \
                case 0x88: (x) = 0x02C6; break; \
                case 0x89: (x) = 0x2030; break; \
                case 0x8A: (x) = 0x0160; break; \
                case 0x8b: (x) = '<';    break; \
                case 0x8C: (x) = 0x0152; break; \
\
                case 0x8E: (x) = 0x017D; break; \
\
\
                case 0x91: (x) = '\'';   break; \
                case 0x92: (x) = '\'';   break; \
                case 0x93: (x) = '"';    break; \
                case 0x94: (x) = '"';    break; \
                case 0x95: (x) = '*';    break; \
                case 0x96: (x) = '-';    break; \
                case 0x97: (x) = '-';    break; \
                case 0x98: (x) = '~';    break; \
                case 0x99: (x) = 0x2122; break; \
                case 0x9A: (x) = 0x0161; break; \
                case 0x9b: (x) = '>';    break; \
                case 0x9C: (x) = 0x0153; break; \
\
                case 0x9E: (x) = 0x017E; break; \
                case 0x9F: (x) = 0x0178; break; \
                /* This one should die */ \
                case 0xb7: (x) = '*';    break; \
                default: break; \
                } \
            } \
            else { \
                /* These should all die sooner rather than later */ \
                switch( (x).unicode() ) { \
                case 0x2013: (x) = '-'; break; \
                case 0x2014: (x) = '-'; break; \
                case 0x2018: (x) = '\''; break; \
                case 0x2019: (x) = '\''; break; \
                case 0x201c: (x) = '"'; break; \
                case 0x201d: (x) = '"'; break; \
                case 0x2022: (x) = '*'; break; \
                case 0x2122: (x) = 0x2122; break; \
                default: break; \
                } \
            }
#endif

// ----------------------------------------------------------------------------

HTMLTokenizer::HTMLTokenizer(DOM::DocumentPtr *_doc, KHTMLView *_view)
{
    view = _view;
    buffer = 0;
    scriptCode = 0;
    scriptCodeSize = scriptCodeMaxSize = scriptCodeResync = 0;
    charsets = KGlobal::charsets();
    parser = new KHTMLParser(_view, _doc);
    m_executingScript = 0;
    onHold = false;

    reset();
}

HTMLTokenizer::HTMLTokenizer(DOM::DocumentPtr *_doc, DOM::DocumentFragmentImpl *i)
{
    view = 0;
    buffer = 0;
    scriptCode = 0;
    scriptCodeSize = scriptCodeMaxSize = scriptCodeResync = 0;
    charsets = KGlobal::charsets();
    parser = new KHTMLParser( i, _doc );
    m_executingScript = 0;
    onHold = false;

    reset();
}

void HTMLTokenizer::reset()
{
    assert(m_executingScript == 0);
    assert(onHold == false);

    while (!cachedScript.isEmpty())
        cachedScript.dequeue()->deref(this);

    if ( buffer )
        KHTML_DELETE_QCHAR_VEC(buffer);
    buffer = dest = 0;
    size = 0;

    if ( scriptCode )
        KHTML_DELETE_QCHAR_VEC(scriptCode);
    scriptCode = 0;
    scriptCodeSize = scriptCodeMaxSize = scriptCodeResync = 0;

    currToken.reset();
}

void HTMLTokenizer::begin()
{
    m_executingScript = 0;
    onHold = false;
    reset();
    size = 254;
    buffer = KHTML_ALLOC_QCHAR_VEC( 255 );
    dest = buffer;
    tag = NoTag;
    pending = NonePending;
    discard = NoneDiscard;
    pre = false;
    prePos = 0;
    plaintext = false;
    xmp = false;
    processingInstruction = false;
    script = false;
    escaped = false;
    style = false;
    skipLF = false;
    select = false;
    comment = false;
    server = false;
    textarea = false;
    title = false;
    startTag = false;
    tquote = NoQuote;
    searchCount = 0;
    Entity = NoEntity;
    scriptSrc = "";
    pendingSrc = "";
    noMoreData = false;
    brokenComments = false;
    brokenServer = false;
    lineno = 0;
    scriptStartLineno = 0;
    tagStartLineno = 0;
}

void HTMLTokenizer::processListing(DOMStringIt list)
{
    bool old_pre = pre;

    // This function adds the listing 'list' as
    // preformatted text-tokens to the token-collection
    // thereby converting TABs.
    if(!style) pre = true;
    prePos = 0;

    while ( list.length() )
    {
        checkBuffer(3*TAB_SIZE);

        if (skipLF && ( *list != '\n' ))
        {
            skipLF = false;
        }

        if (skipLF)
        {
            skipLF = false;
            ++list;
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
            ++list;
        }
        else if (( *list == ' ' ) || ( *list == '\t'))
        {
            if (pending)
                addPending();
            if (*list == ' ')
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
            *dest++ = *list;
            ++list;
        }

    }

    if ((pending == SpacePending) || (pending == TabPending))
        addPending();
    else
        pending = NonePending;

    prePos = 0;
    pre = old_pre;
}

void HTMLTokenizer::parseSpecial(DOMStringIt &src)
{
    assert( textarea || title || !Entity );
    assert( !tag );
    assert( xmp+textarea+title+style+script == 1 );
    if (script)
        scriptStartLineno = lineno+src.lineCount();

    if ( comment ) parseComment( src );

    while ( src.length() ) {
        checkScriptBuffer();
        unsigned char ch = src->latin1();
        if ( !scriptCodeResync && !brokenComments && !textarea && !xmp && !title && ch == '-' && scriptCodeSize >= 3 && !src.escaped() && QConstString( scriptCode+scriptCodeSize-3, 3 ).string() == "<!-" ) {
            comment = true;
            parseComment( src );
            continue;
        }
        if ( scriptCodeResync && !tquote && ( ch == '>' ) ) {
            ++src;
            scriptCodeSize = scriptCodeResync-1;
            scriptCodeResync = 0;
            scriptCode[ scriptCodeSize ] = scriptCode[ scriptCodeSize + 1 ] = 0;
            if ( script )
                scriptHandler();
            else {
                processListing(DOMStringIt(scriptCode, scriptCodeSize));
                processToken();
                if ( style )         { currToken.id = ID_STYLE + ID_CLOSE_TAG; }
                else if ( textarea ) { currToken.id = ID_TEXTAREA + ID_CLOSE_TAG; }
                else if ( title ) { currToken.id = ID_TITLE + ID_CLOSE_TAG; }
                else if ( xmp )  { currToken.id = ID_XMP + ID_CLOSE_TAG; }
                processToken();
                style = script = style = textarea = title = xmp = false;
                tquote = NoQuote;
                scriptCodeSize = scriptCodeResync = 0;
            }
            return;
        }
        // possible end of tagname, lets check.
        if ( !scriptCodeResync && !escaped && !src.escaped() && ( ch == '>' || ch == '/' || ch <= ' ' ) && ch &&
             scriptCodeSize >= searchStopperLen &&
             !QConstString( scriptCode+scriptCodeSize-searchStopperLen, searchStopperLen ).string().find( searchStopper, 0, false )) {
            scriptCodeResync = scriptCodeSize-searchStopperLen+1;
            tquote = NoQuote;
            continue;
        }
        if ( scriptCodeResync && !escaped ) {
            if(ch == '\"')
                tquote = (tquote == NoQuote) ? DoubleQuote : ((tquote == SingleQuote) ? SingleQuote : NoQuote);
            else if(ch == '\'')
                tquote = (tquote == NoQuote) ? SingleQuote : (tquote == DoubleQuote) ? DoubleQuote : NoQuote;
            else if (tquote != NoQuote && (ch == '\r' || ch == '\n'))
                tquote = NoQuote;
        }
        escaped = ( !escaped && ch == '\\' );
        if (!scriptCodeResync && (textarea||title) && !src.escaped() && ch == '&') {
            QChar *scriptCodeDest = scriptCode+scriptCodeSize;
            ++src;
            parseEntity(src,scriptCodeDest,true);
            scriptCodeSize = scriptCodeDest-scriptCode;
        }
        else {
            scriptCode[ scriptCodeSize++ ] = *src;
            ++src;
        }
    }
}

void HTMLTokenizer::scriptHandler()
{
    QString currentScriptSrc = scriptSrc;
    scriptSrc = QString::null;

    processListing(DOMStringIt(scriptCode, scriptCodeSize));
    QString exScript( buffer, dest-buffer );

    processToken();
    currToken.id = ID_SCRIPT + ID_CLOSE_TAG;
    processToken();

    QString prependingSrc;

    if ( !parser->skipMode() ) {
        CachedScript* cs = 0;

        // forget what we just got, load from src url instead
        if ( !currentScriptSrc.isEmpty() &&
             (cs = parser->doc()->docLoader()->requestScript(currentScriptSrc, scriptSrcCharset) ))
            cachedScript.enqueue(cs);

        if (cs) {
            pendingSrc.prepend( QString(src.current(), src.length() ) );
            setSrc(QString::null);
            scriptCodeSize = scriptCodeResync = 0;
            cs->ref(this);

        }
        else if (currentScriptSrc.isEmpty() && view && javascript ) {
            if ( !m_executingScript )
                pendingSrc.prepend( QString( src.current(), src.length() ) ); // deep copy - again
            else
                prependingSrc = QString( src.current(), src.length() ); // deep copy

            setSrc(QString::null);
            scriptCodeSize = scriptCodeResync = 0;
            scriptExecution( exScript, QString(), tagStartLineno /*scriptStartLineno*/ );
        }
    }

    script = false;
    scriptCodeSize = scriptCodeResync = 0;

    if ( !m_executingScript && cachedScript.isEmpty() ) {
        // kdDebug( 6036 ) << "adding pending Output to parsed string" << endl;
        QString newStr = QString(src.current(), src.length());
        newStr += pendingSrc;
        setSrc(newStr);
        pendingSrc = "";
    }
    else if ( !prependingSrc.isEmpty() )
        write( prependingSrc, false );
}

void HTMLTokenizer::scriptExecution( const QString& str, QString scriptURL,
                                     int baseLine)
{
    bool oldscript = script;
    m_executingScript++;
    script = false;
    QString url;
    if (scriptURL.isNull())
      url = static_cast<DocumentImpl*>(view->part()->document().handle())->URL();
    else
      url = scriptURL;

    view->part()->executeScript(url,baseLine,Node(),str);
    m_executingScript--;
    script = oldscript;
}

void HTMLTokenizer::parseComment(DOMStringIt &src)
{
    checkScriptBuffer(src.length());
    while ( src.length() ) {
        scriptCode[ scriptCodeSize++ ] = *src;
#if defined(TOKEN_DEBUG) && TOKEN_DEBUG > 1
        qDebug("comment is now: *%s*",
               QConstString((QChar*)src.current(), QMIN(16, src.length())).string().latin1());
#endif
        if (src->unicode() == '>' &&
            ( ( brokenComments && !( script || style ) ) ||
              ( scriptCodeSize > 2 && scriptCode[scriptCodeSize-3] == '-' &&
                scriptCode[scriptCodeSize-2] == '-' ) ) ) {
            ++src;
            if ( !( script || xmp || textarea || style) ) {
#ifdef COMMENTS_IN_DOM
                checkScriptBuffer();
                scriptCode[ scriptCodeSize ] = 0;
                scriptCode[ scriptCodeSize + 1 ] = 0;
                currToken.id = ID_COMMENT;
                processListing(DOMStringIt(scriptCode, scriptCodeSize - 2));
                processToken();
                currToken.id = ID_COMMENT + ID_CLOSE_TAG;
                processToken();
#endif
                scriptCodeSize = 0;
            }
            comment = false;
            return; // Finished parsing comment
        }
        ++src;
    }
}

void HTMLTokenizer::parseServer(DOMStringIt &src)
{
    checkScriptBuffer(src.length());
    while ( src.length() ) {
        scriptCode[ scriptCodeSize++ ] = *src;
        if (src->unicode() == '>' &&
            scriptCodeSize > 1 && scriptCode[scriptCodeSize-2] == '%') {
            ++src;
            server = false;
            scriptCodeSize = 0;
            return; // Finished parsing server include
        }
        ++src;
    }
}

void HTMLTokenizer::parseProcessingInstruction(DOMStringIt &src)
{
    char oldchar = 0;
    while ( src.length() )
    {
        unsigned char chbegin = src->latin1();
        if(chbegin == '\'') {
            tquote = tquote == SingleQuote ? NoQuote : SingleQuote;
        }
        else if(chbegin == '\"') {
            tquote = tquote == DoubleQuote ? NoQuote : DoubleQuote;
        }
        // Look for '?>'
        // some crappy sites omit the "?" before it, so
        // we look for an unquoted '>' instead. (IE compatible)
        else if ( chbegin == '>' && ( !tquote || oldchar == '?' ) )
        {
            // We got a '?>' sequence
            processingInstruction = false;
            ++src;
            discard=LFDiscard;
            return; // Finished parsing comment!
        }
        ++src;
        oldchar = chbegin;
    }
}

void HTMLTokenizer::parseText(DOMStringIt &src)
{
    while ( src.length() )
    {
        // do we need to enlarge the buffer?
        checkBuffer();

        // ascii is okay because we only do ascii comparisons
        unsigned char chbegin = src->latin1();

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
            if (chbegin == '\r')
                skipLF = true;

            *dest++ = '\n';
            ++src;
        }
        else {
            *dest++ = *src;
            ++src;
        }
    }
}


void HTMLTokenizer::parseEntity(DOMStringIt &src, QChar *&dest, bool start)
{
    if( start )
    {
        cBufferPos = 0;
        Entity = SearchEntity;
    }

    while( src.length() )
    {
        ushort cc = src->unicode();
        switch(Entity) {
        case NoEntity:
            return;

            break;
        case SearchEntity:
            if(cc == '#') {
                cBuffer[cBufferPos++] = cc;
                ++src;
                Entity = NumericSearch;
            }
            else
                Entity = EntityName;

            break;

        case NumericSearch:
            if(cc == 'x' || cc == 'X') {
                cBuffer[cBufferPos++] = cc;
                ++src;
                Entity = Hexadecimal;
            }
            else if(cc >= '0' && cc <= '9')
                Entity = Decimal;
            else
                Entity = SearchSemicolon;

            break;

        case Hexadecimal:
        {
            int uc = EntityChar.unicode();
            int ll = kMin(src.length(), 9-cBufferPos);
            while(ll--) {
                QChar csrc(src->lower());
                cc = csrc.cell();

                if(csrc.row() || !((cc >= '0' && cc <= '9') || (cc >= 'a' && cc <= 'f'))) {
                    Entity = SearchSemicolon;
                    break;
                }
                uc = uc*16 + (cc - ( cc < 'a' ? '0' : 'a' - 10));
                cBuffer[cBufferPos++] = cc;
                ++src;
            }
            EntityChar = QChar(uc);
            if(cBufferPos == 9) Entity = SearchSemicolon;
            break;
        }
        case Decimal:
        {
            int uc = EntityChar.unicode();
            int ll = kMin(src.length(), 9-cBufferPos);
            while(ll--) {
                cc = src->cell();

                if(src->row() || !(cc >= '0' && cc <= '9')) {
                    Entity = SearchSemicolon;
                    break;
                }

                uc = uc * 10 + (cc - '0');
                cBuffer[cBufferPos++] = cc;
                ++src;
            }
            EntityChar = QChar(uc);
            if(cBufferPos == 9)  Entity = SearchSemicolon;
            break;
        }
        case EntityName:
        {
            int ll = kMin(src.length(), 9-cBufferPos);
            while(ll--) {
                QChar csrc = *src;
                cc = csrc.cell();

                if(csrc.row() || !((cc >= 'a' && cc <= 'z') ||
                                   (cc >= '0' && cc <= '9') || (cc >= 'A' && cc <= 'Z'))) {
                    Entity = SearchSemicolon;
                    break;
                }

                cBuffer[cBufferPos++] = cc;
                ++src;
            }
            if(cBufferPos == 9) Entity = SearchSemicolon;
            if(Entity == SearchSemicolon) {
                if(cBufferPos > 1) {
                    const entity *e = findEntity(cBuffer, cBufferPos);
                    if(e)
                        EntityChar = e->code;

                    // be IE compatible
                    if(tag && EntityChar.unicode() > 255 && *src != ';')
                        EntityChar = QChar::null;
                }
            }
            else
                break;
        }
        case SearchSemicolon:

            //kdDebug( 6036 ) << "ENTITY " << EntityChar.unicode() << ", " << res << endl;

            fixUpChar(EntityChar);

            if ( EntityChar != QChar::null ) {
                checkBuffer();
                // Just insert it
                if (*src == ';')
                    ++src;

                src.push( EntityChar );
            } else {
#ifdef TOKEN_DEBUG
                kdDebug( 6036 ) << "unknown entity!" << endl;
#endif
                checkBuffer(10);
                // ignore the sequence, add it to the buffer as plaintext
                *dest++ = '&';
                for(unsigned int i = 0; i < cBufferPos; i++)
                    dest[i] = cBuffer[i];
                dest += cBufferPos;
                Entity = NoEntity;
                if (pre)
                    prePos += cBufferPos+1;
            }

            Entity = NoEntity;
            EntityChar = QChar::null;
            return;
        };
    }
}

void HTMLTokenizer::parseTag(DOMStringIt &src)
{
    assert(!Entity );

    while ( src.length() )
    {
        checkBuffer();
#if defined(TOKEN_DEBUG) && TOKEN_DEBUG > 1
        uint l = 0;
        while(l < src.length() && (*(src.current()+l)).latin1() != '>')
            l++;
        qDebug("src is now: *%s*, tquote: %d",
               QConstString((QChar*)src.current(), l).string().latin1(), tquote);
#endif
        switch(tag) {
        case NoTag:
        {
            return;
        }
        case TagName:
        {
#if defined(TOKEN_DEBUG) &&  TOKEN_DEBUG > 1
            qDebug("TagName");
#endif
            if (searchCount > 0)
            {
                if (*src == commentStart[searchCount])
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
                        tag = NoTag;
                        parseComment(src);

                        return; // Finished parsing tag!
                    }
                    // cuts of high part, is okay
                    cBuffer[cBufferPos++] = src->cell();
                    ++src;
                    break;
                }
                else
                    searchCount = 0; // Stop looking for '<!--' sequence
            }

            bool finish = false;
            unsigned int ll = kMin(src.length(), CBUFLEN-cBufferPos);
            while(ll--) {
                ushort curchar = *src;
                if(curchar <= ' ' || curchar == '>' ) {
                    finish = true;
                    break;
                }
                // this is a nasty performance trick. will work for the A-Z
                // characters, but not for others. if it contains one,
                // we fail anyway
                char cc = curchar;
                cBuffer[cBufferPos++] = cc | 0x20;
                ++src;
            }

            // Disadvantage: we add the possible rest of the tag
            // as attribute names. ### judge if this causes problems
            if(finish || CBUFLEN == cBufferPos) {
                bool beginTag;
                char* ptr = cBuffer;
                unsigned int len = cBufferPos;
                cBuffer[cBufferPos] = '\0';
                if ((cBufferPos > 0) && (*ptr == '/'))
                {
                    // End Tag
                    beginTag = false;
                    ptr++;
                    len--;
                }
                else
                    // Start Tag
                    beginTag = true;
                // Accept empty xml tags like <br/>
                if(len > 1 && ptr[len-1] == '/' ) {
                    ptr[--len] = '\0';
                    // if its like <br/> and not like <input/ value=foo>, take it as flat
                    if (*src == '>')
                        currToken.flat = true;
                }

                uint tagID = khtml::getTagID(ptr, len);
                if (!tagID) {
#ifdef TOKEN_DEBUG
                    QCString tmp(ptr, len+1);
                    kdDebug( 6036 ) << "Unknown tag: \"" << tmp.data() << "\"" << endl;
#endif
                    dest = buffer;
                }
                else
                {
#ifdef TOKEN_DEBUG
                    QCString tmp(ptr, len+1);
                    kdDebug( 6036 ) << "found tag id=" << tagID << ": " << tmp.data() << endl;
#endif
                    currToken.id = beginTag ? tagID : tagID + ID_CLOSE_TAG;
                    dest = buffer;
                }
                tag = SearchAttribute;
                cBufferPos = 0;
            }
            break;
        }
        case SearchAttribute:
        {
#if defined(TOKEN_DEBUG) && TOKEN_DEBUG > 1
                qDebug("SearchAttribute");
#endif
            bool atespace = false;
            ushort curchar;
            while(src.length()) {
                curchar = *src;
                if(curchar > ' ') {
                    if(curchar == '>')
                        tag = SearchEnd;
                    else if(atespace && (curchar == '\'' || curchar == '"'))
                    {
                        tag = SearchValue;
                        *dest++ = 0;
                        attrName = QString::null;
                    }
                    else
                        tag = AttributeName;

                    cBufferPos = 0;
                    break;
                }
                atespace = true;
                ++src;
            }
            break;
        }
        case AttributeName:
        {
#if defined(TOKEN_DEBUG) && TOKEN_DEBUG > 1
                qDebug("AttributeName");
#endif
            ushort curchar;
            int ll = kMin(src.length(), CBUFLEN-cBufferPos);

            while(ll--) {
                curchar = *src;
                if(curchar <= '>') {
                    if(curchar <= ' ' || curchar == '=' || curchar == '>') {
                        unsigned int a;
                        cBuffer[cBufferPos] = '\0';
                        a = khtml::getAttrID(cBuffer, cBufferPos);
                        if ( !a )
                            attrName = QString::fromLatin1(QCString(cBuffer, cBufferPos+1).data());

                        dest = buffer;
                        *dest++ = a;
#ifdef TOKEN_DEBUG
                        if (!a || (cBufferPos && *cBuffer == '!'))
                            kdDebug( 6036 ) << "Unknown attribute: *" << QCString(cBuffer, cBufferPos+1).data() << "*" << endl;
                        else
                            kdDebug( 6036 ) << "Known attribute: " << QCString(cBuffer, cBufferPos+1).data() << endl;
#endif
                        // did we just get />
                        if (!a && cBufferPos == 1 && *cBuffer == '/' && curchar == '>')
                            currToken.flat = true;

                        tag = SearchEqual;
                        break;
                    }
                }
                cBuffer[cBufferPos++] = (char) curchar | 0x20;
                ++src;
            }
            if ( cBufferPos == CBUFLEN ) {
                cBuffer[cBufferPos] = '\0';
                attrName = QString::fromLatin1(QCString(cBuffer, cBufferPos+1).data());
                dest = buffer;
                *dest++ = 0;
                tag = SearchEqual;
            }
            break;
        }
        case SearchEqual:
        {
#if defined(TOKEN_DEBUG) && TOKEN_DEBUG > 1
                qDebug("SearchEqual");
#endif
            ushort curchar;
            bool atespace = false;
            while(src.length()) {
                curchar = src->unicode();
                if(curchar > ' ') {
                    if(curchar == '=') {
#ifdef TOKEN_DEBUG
                        kdDebug(6036) << "found equal" << endl;
#endif
                        tag = SearchValue;
                        ++src;
                    }
                    else if(atespace && (curchar == '\'' || curchar == '"'))
                    {
                        tag = SearchValue;
                        *dest++ = 0;
                        attrName = QString::null;
                    }
                    else {
                        DOMString v("");
                        currToken.addAttribute(parser->docPtr()->document(), buffer, attrName, v);
                        dest = buffer;
                        tag = SearchAttribute;
                    }
                    break;
                }
                atespace = true;
                ++src;
            }
            break;
        }
        case SearchValue:
        {
            ushort curchar;
            while(src.length()) {
                curchar = src->unicode();
                if(curchar > ' ') {
                    if(( curchar == '\'' || curchar == '\"' )) {
                        tquote = curchar == '\"' ? DoubleQuote : SingleQuote;
                        tag = QuotedValue;
                        ++src;
                    } else
                        tag = Value;

                    break;
                }
                ++src;
            }
            break;
        }
        case QuotedValue:
        {
#if defined(TOKEN_DEBUG) && TOKEN_DEBUG > 1
                qDebug("QuotedValue");
#endif
            ushort curchar;
            while(src.length()) {
                checkBuffer();

                curchar = src->unicode();
                if(curchar <= '\'' && !src.escaped()) {
                    // ### attributes like '&{blaa....};' are supposed to be treated as jscript.
                    if ( curchar == '&' )
                    {
                        ++src;
                        parseEntity(src, dest, true);
                        break;
                    }
                    else if ( (tquote == SingleQuote && curchar == '\'') ||
                              (tquote == DoubleQuote && curchar == '\"') )
                    {
                        // some <input type=hidden> rely on trailing spaces. argh
                        while(dest > buffer+1 && (*(dest-1) == '\n' || *(dest-1) == '\r'))
                            dest--; // remove trailing newlines
                        DOMString v(buffer+1, dest-buffer-1);
                        currToken.addAttribute(parser->docPtr()->document(), buffer, attrName, v);

                        dest = buffer;
                        tag = SearchAttribute;
                        tquote = NoQuote;
                        ++src;
                        break;
                    }
                }
                *dest++ = *src;
                ++src;
            }
            break;
        }
        case Value:
        {
#if defined(TOKEN_DEBUG) && TOKEN_DEBUG > 1
            qDebug("Value");
#endif
            ushort curchar;
            while(src.length()) {
                checkBuffer();
                curchar = src->unicode();
                if(curchar <= '>' && !src.escaped()) {
                    // parse Entities
                    if ( curchar == '&' )
                    {
                        ++src;
                        parseEntity(src, dest, true);
                        break;
                    }
                    // no quotes. Every space means end of value
                    // '/' does not delimit in IE!
                    if ( curchar <= ' ' || curchar == '>' )
                    {
                        DOMString v(buffer+1, dest-buffer-1);
                        currToken.addAttribute(parser->docPtr()->document(), buffer, attrName, v);
                        dest = buffer;
                        tag = SearchAttribute;
                        break;
                    }
                }

                *dest++ = *src;
                ++src;
            }
            break;
        }
        case SearchEnd:
        {
#if defined(TOKEN_DEBUG) && TOKEN_DEBUG > 1
                qDebug("SearchEnd");
#endif
            while(src.length()) {
                if(*src == '>')
                    break;

                if (*src == '/')
                    currToken.flat = true;

                ++src;
            }
            if(!src.length() && *src != '>') break;

            searchCount = 0; // Stop looking for '<!--' sequence
            tag = NoTag;
            tquote = NoQuote;
            ++src;

            if ( !currToken.id ) //stop if tag is unknown
                return;

            uint tagID = currToken.id;
#if defined(TOKEN_DEBUG) && TOKEN_DEBUG > 0
            kdDebug( 6036 ) << "appending Tag: " << tagID << endl;
#endif
            bool beginTag = !currToken.flat && (tagID < ID_CLOSE_TAG);

            if(tagID >= ID_CLOSE_TAG)
                tagID -= ID_CLOSE_TAG;
            else if ( beginTag && tagID == ID_SCRIPT ) {
                AttributeImpl* a = 0;
                scriptSrc = scriptSrcCharset = "";
                if ( currToken.attrs && /* potentially have a ATTR_SRC ? */
                     parser->doc()->view()->part()->jScriptEnabled() && /* jscript allowed at all? */
                     view /* are we a regular tokenizer or just for innerHTML ? */
                    ) {
                    if ( ( a = currToken.attrs->getAttributeItem( ATTR_SRC ) ) )
                        scriptSrc = parser->doc()->completeURL(khtml::parseURL( a->value() ).string() );
                    if ( ( a = currToken.attrs->getAttributeItem( ATTR_CHARSET ) ) )
                        scriptSrcCharset = a->value().string().stripWhiteSpace();
                    if ( scriptSrcCharset.isEmpty() )
                        scriptSrcCharset = parser->doc()->view()->part()->encoding();
                    if (!(a = currToken.attrs->getAttributeItem( ATTR_LANGUAGE )))
                        a = currToken.attrs->getAttributeItem(ATTR_TYPE);
                }
                javascript = true;
                if( a ) {
                    QString lang = a->value().string();
                    lang = lang.lower();
                    if( !lang.contains("javascript") &&
                        !lang.contains("ecmascript") &&
                        !lang.contains("livescript") &&
                        !lang.contains("jscript") )
                        javascript = false;
                }
            }

            processToken();

            // lets see if we're still in parsing mood for spaces
            pre = parser->preMode();

            switch( tagID ) {
            case ID_PRE:
                prePos = 0;
                break;
            case ID_SCRIPT:
                if (beginTag) {
                    searchStopper = scriptEnd;
                    searchStopperLen = 8;
                    script = true;
                    parseSpecial(src);
                }
                break;
            case ID_STYLE:
                if (beginTag) {
                    searchStopper = styleEnd;
                    searchStopperLen = 7;
                    style = true;
                    parseSpecial(src);
                }
                break;
            case ID_TEXTAREA:
                if(beginTag) {
                    searchStopper = textareaEnd;
                    searchStopperLen = 10;
                    textarea = true;
                    discard = AllDiscard;
                    parseSpecial(src);
                }
                break;
            case ID_TITLE:
                if (beginTag) {
                    searchStopper = titleEnd;
                    searchStopperLen = 7;
                    title = true;
                    parseSpecial(src);
                }
                break;
            case ID_XMP:
                if (beginTag) {
                    searchStopper = xmpEnd;
                    searchStopperLen = 5;
                    xmp = true;
                    parseSpecial(src);
                }
                break;
            case ID_SELECT:
                select = beginTag;
                break;
            case ID_PLAINTEXT:
                plaintext = beginTag;
                break;
            }
            return; // Finished parsing tag!
        }
        } // end switch
    }
    return;
}

void HTMLTokenizer::addPending()
{
    if ( select && !(comment || script))
    {
        *dest++ = ' ';
    }
    else if ( textarea )
    {
        switch(pending) {
        case LFPending:  *dest++ = '\n'; prePos = 0; break;
        case SpacePending: *dest++ = ' '; ++prePos; break;
        case TabPending: *dest++ = '\t'; prePos += TAB_SIZE - (prePos % TAB_SIZE); break;
        case NonePending:
            assert(0);
        }
    }
    else if ( pre )
    {
        int p;

        switch (pending)
        {
        case SpacePending:
            // Insert a breaking space
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
                *dest++ = QChar(' ');
            prePos += p;
            break;

        case NonePending:
            assert(0);
            break;
        }
    }
    else
    {
        *dest++ = ' ';
    }

    pending = NonePending;
}

void HTMLTokenizer::write( const QString &str, bool appendData )
{
#ifdef TOKEN_DEBUG
    kdDebug( 6036 ) << this << " Tokenizer::write(\"" << str << "\"," << appendData << ")" << endl;
#endif

    if ( !buffer )
        return;

    if ( ( m_executingScript && appendData ) ||
         ( !m_executingScript && cachedScript.count() ) ) {
        // don't parse; we will do this later
        pendingSrc += str;
        return;
    }

    if ( onHold ) {
        QString rest = QString( src.current(), src.length() );
        rest += str;
        setSrc(rest);
        return;
    }
    else
        setSrc(str);

//     if (Entity)
//         parseEntity(src, dest);

    while ( src.length() )
    {
        // do we need to enlarge the buffer?
        checkBuffer();

        ushort cc = src->unicode();

        if (skipLF && (cc != '\n'))
            skipLF = false;

        if (skipLF) {
            skipLF = false;
            ++src;
        }
        else if ( Entity )
            parseEntity( src, dest );
        else if ( plaintext )
            parseText( src );
        else if (script)
            parseSpecial(src);
        else if (style)
            parseSpecial(src);
        else if (xmp)
            parseSpecial(src);
        else if (textarea)
            parseSpecial(src);
        else if (title)
            parseSpecial(src);
        else if (comment)
            parseComment(src);
        else if (server)
            parseServer(src);
        else if (processingInstruction)
            parseProcessingInstruction(src);
        else if (tag)
            parseTag(src);
        else if ( startTag )
        {
            startTag = false;

            switch(cc) {
            case '/':
                break;
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
                tquote = NoQuote;
                parseProcessingInstruction(src);
                continue;

                break;
            }
            case '%':
                if (!brokenServer) {
                    // <% server stuff, handle as comment %>
                    server = true;
                    tquote = NoQuote;
                    parseServer(src);
                    continue;
                }
                // else fall through
            default:
            {
                if( ((cc >= 'a') && (cc <= 'z')) || ((cc >= 'A') && (cc <= 'Z')))
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
                    continue;
                }
            }
            }; // end case

            if ( pending ) {
                // pre context always gets its spaces/linefeeds
                if ( pre )
                    addPending();
                // only add in existing inline context or if
                // we just started one, i.e. we're about to insert real text
                else if ( !parser->selectMode() &&
                          ( !parser->noSpaces() || dest > buffer )) {
                    addPending();
		    discard = AllDiscard;
                }
                // just forget it
                else
                    pending = NonePending;
            }

            processToken();

            cBufferPos = 0;
            tag = TagName;
            parseTag(src);
        }
        else if ( cc == '&' && !src.escaped())
        {
            ++src;
            if ( pending )
                addPending();
            parseEntity(src, dest, true);
        }
        else if ( cc == '<' && !src.escaped())
        {
            tagStartLineno = lineno+src.lineCount();
            ++src;
            startTag = true;
        }
        else if (( cc == '\n' ) || ( cc == '\r' ))
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
            if (cc == '\r')
            {
                skipLF = true;
            }
            ++src;
        }
        else if (( cc == ' ' ) || ( cc == '\t' ))
        {
            if ( pre || textarea)
            {
                if (discard == SpaceDiscard || discard == AllDiscard)
                {
                    // Ignore this LF
                    discard = NoneDiscard; // We have discarded 1 LF
                }
                else {
                    if (pending)
                        addPending();
                    if (cc == ' ')
                        pending = SpacePending;
                    else
                        pending = TabPending;
                }
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
            *dest = *src;
            fixUpChar( *dest );
            ++dest;
            ++src;
        }
    }
    _src = QString();

    if (noMoreData && cachedScript.isEmpty() && !m_executingScript )
        end(); // this actually causes us to be deleted
}

void HTMLTokenizer::end()
{
    if ( buffer == 0 ) {
        emit finishedParsing();
        return;
    }

    // parseTag is using the buffer for different matters
    if ( !tag )
        processToken();

    if(buffer)
        KHTML_DELETE_QCHAR_VEC(buffer);

    if(scriptCode)
        KHTML_DELETE_QCHAR_VEC(scriptCode);

    scriptCode = 0;
    scriptCodeSize = scriptCodeMaxSize = scriptCodeResync = 0;
    buffer = 0;
    emit finishedParsing();
}

void HTMLTokenizer::finish()
{
    // do this as long as we don't find matching comment ends
    while((comment || server) && scriptCode && scriptCodeSize)
    {
        // we've found an unmatched comment start
        if (comment)
            brokenComments = true;
        else
            brokenServer = true;
        checkScriptBuffer();
        scriptCode[ scriptCodeSize ] = 0;
        scriptCode[ scriptCodeSize + 1 ] = 0;
        int pos;
        QString food;
        if (script || style) {
            food.setUnicode(scriptCode, scriptCodeSize);
        }
        else if (server) {
            food = "<";
            food += QString(scriptCode, scriptCodeSize);
        }
        else {
            pos = QConstString(scriptCode, scriptCodeSize).string().find('>');
            food.setUnicode(scriptCode+pos+1, scriptCodeSize-pos-1); // deep copy
        }
        KHTML_DELETE_QCHAR_VEC(scriptCode);
        scriptCode = 0;
        scriptCodeSize = scriptCodeMaxSize = scriptCodeResync = 0;
        comment = server = false;
        if ( !food.isEmpty() )
            write(food, true);
    }
    // this indicates we will not recieve any more data... but if we are waiting on
    // an external script to load, we can't finish parsing until that is done
    noMoreData = true;
    if (cachedScript.isEmpty() && !m_executingScript && !onHold)
        end(); // this actually causes us to be deleted
}

void HTMLTokenizer::processToken()
{
    KJSProxy *jsProxy = view ? view->part()->jScript() : 0L;
    if (jsProxy)
        jsProxy->setEventHandlerLineno(tagStartLineno);
    if ( dest > buffer )
    {
#ifdef TOKEN_DEBUG
        if(currToken.id) {
            qDebug( "unexpected token id: %d, str: *%s*", currToken.id,QConstString( buffer,dest-buffer ).string().latin1() );
            assert(0);
        }

#endif
        currToken.text = new DOMStringImpl( buffer, dest - buffer );
        currToken.text->ref();
        currToken.id = ID_TEXT;
    }
    else if(!currToken.id) {
        currToken.reset();
        if (jsProxy)
            jsProxy->setEventHandlerLineno(lineno+src.lineCount());
        return;
    }

    dest = buffer;

#ifdef TOKEN_DEBUG
    QString name = getTagName(currToken.id).string();
    QString text;
    if(currToken.text)
        text = QConstString(currToken.text->s, currToken.text->l).string();

    kdDebug( 6036 ) << "Token --> " << name << "   id = " << currToken.id << endl;
    if (currToken.flat)
        kdDebug( 6036 ) << "Token is FLAT!" << endl;
    if(!text.isNull())
        kdDebug( 6036 ) << "text: \"" << text << "\"" << endl;
    unsigned long l = currToken.attrs ? currToken.attrs->length() : 0;
    if(l) {
        kdDebug( 6036 ) << "Attributes: " << l << endl;
        for (unsigned long i = 0; i < l; ++i) {
            AttributeImpl* c = currToken.attrs->attributeItem(i);
            kdDebug( 6036 ) << "    " << c->id() << " " << parser->doc()->getDocument()->attrName(c->id()).string()
                            << "=\"" << c->value().string() << "\"" << endl;
        }
    }
    kdDebug( 6036 ) << endl;
#endif
    // pass the token over to the parser, the parser DOES NOT delete the token
    parser->parseToken(&currToken);

    if ( currToken.flat && currToken.id != ID_TEXT && !parser->noSpaces() )
	discard = NoneDiscard;

    currToken.reset();
    if (jsProxy)
        jsProxy->setEventHandlerLineno(0);
}


HTMLTokenizer::~HTMLTokenizer()
{
    reset();
    delete parser;
}


void HTMLTokenizer::enlargeBuffer(int len)
{
    int newsize = kMax(size*2, size+len);
    int oldoffs = (dest - buffer);

    buffer = (QChar*)realloc(buffer, newsize*sizeof(QChar));
    dest = buffer + oldoffs;
    size = newsize;
}

void HTMLTokenizer::enlargeScriptBuffer(int len)
{
    int newsize = kMax(scriptCodeMaxSize*2, scriptCodeMaxSize+len);
    scriptCode = (QChar*)realloc(scriptCode, newsize*sizeof(QChar));
    scriptCodeMaxSize = newsize;
}

void HTMLTokenizer::notifyFinished(CachedObject */*finishedObj*/)
{
    assert(!cachedScript.isEmpty());
    bool done = false;
    while (!done && cachedScript.head()->isLoaded()) {
#ifdef TOKEN_DEBUG
        kdDebug( 6036 ) << "Finished loading an external script" << endl;
#endif
        CachedScript* cs = cachedScript.dequeue();
        done = cachedScript.isEmpty();
        DOMString scriptSource = cs->script();
#ifdef TOKEN_DEBUG
        kdDebug( 6036 ) << "External script is:" << endl << scriptSource.string() << endl;
#endif
        setSrc(QString::null);

        // make sure we forget about the script before we execute the new one
        // infinite recursion might happen otherwise
        QString cachedScriptUrl( cs->url().string() );
        cs->deref(this);

	scriptExecution( scriptSource.string(), cachedScriptUrl );

        // 'script' is true when we are called synchronously from
        // parseScript(). In that case parseScript() will take care
        // of 'scriptOutput'.
        if ( !script ) {
            QString rest = pendingSrc;
            pendingSrc = "";
            write(rest, false);
            // we might be deleted at this point, do not
            // access any members.
        }
    }
}

void HTMLTokenizer::setSrc(QString source)
{
    lineno += src.lineCount();
    _src = source;
    src = DOMStringIt(_src);
}

void HTMLTokenizer::setOnHold(bool _onHold)
{
    if (onHold == _onHold) return;
    onHold = _onHold;
    if (onHold)
        setSrc(QString(src.current(), src.length())); // ### deep copy
}

