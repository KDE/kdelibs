/*
    This file is part of the KDE libraries

    Copyright (C) 1997 Martin Jones (mjones@kde.org)
              (C) 1997 Torben Weis (weis@kde.org)
              (C) 1998 Waldo Bastian (bastian@kde.org)
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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
//----------------------------------------------------------------------------
//
// KDE HTML Widget -- Tokenizers

#ifndef HTMLTOKENIZER_H
#define HTMLTOKENIZER_H

#include <qstring.h>
#include <qobject.h>
#include <q3ptrqueue.h>

#include "misc/loader_client.h"
#include "misc/htmltags.h"
#include "misc/stringit.h"
#include "xml/dom_stringimpl.h"
#include "xml/xml_tokenizer.h"
#include "xml/dom_elementimpl.h"
#include "xml/dom_docimpl.h"

class KCharsets;
class KHTMLView;

namespace DOM {
    class DocumentPtr;
    class DocumentFragmentImpl;
}

namespace khtml {
    class CachedScript;
    class KHTMLParser;

    /**
     * @internal
     * represents one HTML tag. Consists of a numerical id, and the list
     * of attributes. Can also represent text. In this case the id = 0 and
     * text contains the text.
     */
    class Token
    {
    public:
        Token() {
            tid = 0;
            attrs = 0;
            text = 0;
            flat = false;
            //qDebug("new token, creating %08lx", attrs);
        }
        ~Token() {
            if(attrs) attrs->deref();
            if(text) text->deref();
        }
        void addAttribute(DocumentImpl* doc, QChar* buffer, const QString& attrName, const DOMString& v)
        {
            DOMStringImpl *value = 0;
            NodeImpl::Id tid = 0;
            if(buffer->unicode()) {
		tid = buffer->unicode();
		value = v.implementation();
            }
            else if ( !attrName.isEmpty() && attrName != "/" ) {
		tid = doc->getId(NodeImpl::AttributeId, DOMString(attrName).implementation(), false, true);
		value = v.implementation();
            }

            if (value && tid) {
                if(!attrs) {
                    attrs = new DOM::NamedAttrMapImpl(0);
                    attrs->ref();
                }
                if (!attrs->getValue(tid))
		    attrs->setValue(tid,value);
            }
        }
        void reset()
        {
            if(attrs) {
                attrs->deref();
                attrs = 0;
            }
            tid = 0;
            if(text) {
                text->deref();
                text = 0;
            }
            flat = false;
        }
        DOM::NamedAttrMapImpl* attrs;
        DOMStringImpl* text;
        ushort tid;
        bool flat;
    };

// The count of spaces used for each tab.
#define TAB_SIZE 8

//-----------------------------------------------------------------------------

class HTMLTokenizer : public Tokenizer, public CachedObjectClient
{
    friend class KHTMLParser;
public:
    HTMLTokenizer(DOM::DocumentPtr *, KHTMLView * = 0);
    HTMLTokenizer(DOM::DocumentPtr *, DOM::DocumentFragmentImpl *frag);
    virtual ~HTMLTokenizer();

    void begin();
    void write( const khtml::TokenizerString &str, bool appendData );
    void end();
    void finish();
    void timerEvent( QTimerEvent *e );
    virtual void setOnHold(bool _onHold);
    void abort() { m_abort = true; }
    virtual void setAutoClose(bool b=true);    
    virtual bool isWaitingForScripts() const;
    virtual bool isExecutingScript() const;
      
protected:
    void reset();
    void addPending();
    void processToken();
    void processListing(khtml::TokenizerString list);

    void parseComment(khtml::TokenizerString &str);
    void parseServer(khtml::TokenizerString &str);
    void parseText(khtml::TokenizerString &str);
    void parseListing(khtml::TokenizerString &str);
    void parseSpecial(khtml::TokenizerString &str);
    void parseTag(khtml::TokenizerString &str);
    void parseEntity(khtml::TokenizerString &str, QChar *&dest, bool start = false);
    void parseProcessingInstruction(khtml::TokenizerString &str);
    void scriptHandler();
    void scriptExecution(const QString& script, const QString& scriptURL = QString(), int baseLine = 0);
    void setSrc(const TokenizerString& source);

    // check if we have enough space in the buffer.
    // if not enlarge it
    inline void checkBuffer(int len = 10)
    {
        if ( (dest - buffer) > size-len )
            enlargeBuffer(len);
    }
    inline void checkScriptBuffer(int len = 10)
    {
        if ( scriptCodeSize + len >= scriptCodeMaxSize )
            enlargeScriptBuffer(len);
    }

    void enlargeBuffer(int len);
    void enlargeScriptBuffer(int len);

    // from CachedObjectClient
    void notifyFinished(khtml::CachedObject *finishedObj);

protected:
    // Internal buffers
    ///////////////////
    QChar *buffer;
    QChar *dest;

    khtml::Token currToken;

    // the size of buffer
    int size;

    // Tokenizer flags
    //////////////////
    // are we in quotes within a html tag
    enum
    {
        NoQuote = 0,
        SingleQuote,
        DoubleQuote
    } tquote;

    enum
    {
        NonePending = 0,
        SpacePending,
        LFPending,
        TabPending
    } pending;

    enum
    {
        NoneDiscard = 0,
        SpaceDiscard,	// Discard spaces after '=' within tags
        LFDiscard,	// Discard line breaks immediately after start-tags
        AllDiscard	// discard all spaces, LF's etc until next non white char
    } discard;

    // Discard the LF part of CRLF sequence
    bool skipLF;

    // Flag to say that we have the '<' but not the character following it.
    bool startTag;

    // Flag to say, we are just parsing a tag, meaning, we are in the middle
    // of <tag...
    enum {
        NoTag = 0,
        TagName,
        SearchAttribute,
        AttributeName,
        SearchEqual,
        SearchValue,
        QuotedValue,
        Value,
        SearchEnd
    } tag;

    // Are we in a &... character entity description?
    enum {
        NoEntity = 0,
        SearchEntity,
        NumericSearch,
        Hexadecimal,
        Decimal,
        EntityName,
        SearchSemicolon
    } Entity;

    // are we in a <script> ... </script> block
    bool script;

    QChar EntityChar;

    // Are we in a <pre> ... </pre> block
    bool pre;

    // if 'pre == true' we track in which column we are
    int prePos;

    // Are we in a <style> ... </style> block
    bool style;

    // Are we in a <select> ... </select> block
    bool select;

    // Are we in a <xmp> ... </xmp> block
    bool xmp;

    // Are we in a <title> ... </title> block
    bool title;

    // Are we in plain textmode ?
    bool plaintext;

    // XML processing instructions. Ignored at the moment
    bool processingInstruction;

    // Area we in a <!-- comment --> block
    bool comment;

    // Are we in a <textarea> ... </textarea> block
    bool textarea;

    // was the previous character escaped ?
    bool escaped;

    // are we in a server includes statement?
    bool server;

    bool brokenServer;

    bool brokenScript;

    // name of an unknown attribute
    QString attrName;

    // Used to store the code of a srcipting sequence
    QChar *scriptCode;
    // Size of the script sequenze stored in scriptCode
    int scriptCodeSize;
    // Maximal size that can be stored in scriptCode
    int scriptCodeMaxSize;
    // resync point of script code size
    int scriptCodeResync;

    // Stores characters if we are scanning for a string like "</script>"
    QChar searchBuffer[ 10 ];
    // Counts where we are in the string we are scanning for
    int searchCount;
    // The string we are searching for
    const QChar *searchFor;
    // the stopper string
    const char* searchStopper;
    // the stopper len
    int searchStopperLen;
    // if no more data is coming, just parse what we have (including ext scripts that
    // may be still downloading) and finish
    bool noMoreData;
    // URL to get source code of script from
    QString scriptSrc;
    QString scriptSrcCharset;
    bool javascript;
    // the HTML code we will parse after the external script we are waiting for has loaded
    TokenizerString pendingSrc;
    // true if we are executing a script while parsing a document. This causes the parsing of
    // the output of the script to be postponed until after the script has finished executing
    int m_executingScript;
    Q3PtrQueue<khtml::CachedScript> cachedScript;
    // you can pause the tokenizer if you need to display a dialog or something
    bool onHold;
    // you can ask the tokenizer to abort the current write() call, e.g. to redirect somewhere else
    bool m_abort;

    // if we found one broken comment, there are most likely others as well
    // store a flag to get rid of the O(n^2) behavior in such a case.
    bool brokenComments;
    // current line number
    int lineno;
    // line number at which the current <script> started
    int scriptStartLineno;
    int tagStartLineno;
    // autoClose mode is used when the tokenizer was created by a script document.writing
    // on an already loaded document
    int m_autoCloseTimer;

#define CBUFLEN 20
    char cBuffer[CBUFLEN+2];
    unsigned int cBufferPos;

    khtml::TokenizerString src;

    KCharsets *charsets;
    KHTMLParser *parser;

    KHTMLView *view;
};

} // namespace

#endif // HTMLTOKENIZER

