/*
    This file is part of the KDE libraries

    Copyright (C) 1997 Martin Jones (mjones@kde.org)
              (C) 1997 Torben Weis (weis@kde.org)
              (C) 1998 Waldo Bastian (bastian@kde.org)
              (C) 1999 Lars Knoll (knoll@kde.org)

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
// KDE HTML Widget -- HTML Parser
// $Id$

#ifndef HTMLPARSER_H
#define HTMLPARSER_H

#include "dom/dom_string.h"

class KHTMLView;
class HTMLStackElem;

namespace DOM {
    class HTMLDocumentImpl;
    class HTMLElementImpl;
    class NodeImpl;
    class HTMLFormElementImpl;
    class HTMLMapElementImpl;
}

namespace khtml {
    class Token;
};

class KHTMLParser;
typedef void (KHTMLParser::*blockFunc)(HTMLStackElem *stackElem);

/**
 * The parser for html. It receives a stream of tokens from the HTMLTokenizer, and
 * builds up the Document structure form it.
 */
class KHTMLParser
{
public:
    KHTMLParser( KHTMLView *w, DOM::HTMLDocumentImpl *i );
    virtual ~KHTMLParser();

    /**
     * parses one token delivered by the tokenizer
     * Caution: the parser deletes the token after use!
     */
    void parseToken(khtml::Token *_t);

    /**
     * resets the parser
     */
    void reset();

    bool parsingBody() { return inBody; }

    DOM::HTMLDocumentImpl *doc() { return document; }

protected:

    KHTMLView *HTMLWidget;
    DOM::HTMLDocumentImpl *document;

    /*
     * generate an element from the token
     */
    DOM::NodeImpl *getElement(khtml::Token *);

    void processCloseTag(khtml::Token *);

    void insertNode(DOM::NodeImpl *n);

    /*
     * The currently active element (the one new elements will be added to)
     */
    DOM::NodeImpl *current;

    HTMLStackElem *blockStack;

    void pushBlock( int _id, int _level,
		    blockFunc _exitFunc = 0,
		    int _miscData1 = 0);
    					
    void popBlock( int _id );
    void popOneBlock();
    void popInlineBlocks();

    // used for passing the data in the parser
    blockFunc exitFunc;
    int exitFuncData;

    void freeBlock( void);

    ushort *forbiddenTag;

    /*
     * currently active form
     */
    DOM::HTMLFormElementImpl *form;

    /*
     * current map
     */
    DOM::HTMLMapElementImpl *map;

    bool inBody;
    bool haveFrameSet;
    bool _inline;
    bool end;
    /*
     * tells the parser to discard all tags, until it reaches the one specified
     */
    int discard_until;

    bool headLoaded;
};

#endif // HTMLPARSER_H

