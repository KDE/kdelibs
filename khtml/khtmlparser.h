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

#include "dom_string.h"
#include "khtmlstyle.h"

class KHTMLWidget;
class Token;
class HTMLStackElem;
class CSSStyle;
class HTMLSettings;

namespace DOM {
    class HTMLDocumentImpl;
    class NodeImpl;
}
using namespace DOM;


class KHTMLParser
{
public:
    KHTMLParser( KHTMLWidget *w, HTMLDocumentImpl *i );
    virtual ~KHTMLParser();

    /**
     * parses one token delivered by the tokenizer
     * Caution: the parser deletes the token after use!
     */
    void parseToken(Token *_t);

    /**
     * resets the parser
     */
    void reset();

protected:
    KHTMLWidget *HTMLWidget;
    HTMLDocumentImpl *document;

    /*
     * generate an element from the token
     */
    NodeImpl *getElement(Token *);

    void processCloseTag(Token *);

    void insertNode(NodeImpl *n);

    /*
     * The currently active element (the one new elements will be added to)
     */
    NodeImpl *current;

    /*
     * A node blocking the incremental rendering
     */
    NodeImpl *block;

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

    // block exit functions:
    void blockEndList( HTMLStackElem *Elem);

    /*
     * Current style sheet
     */
    CSSStyleSheet *styleSheet;

    /*
     * Current style
     */
    CSSStyle *currentStyle;

    /*
     * default font sizes etc...
     */
    HTMLSettings *settings;

    ushort *forbiddenTag;

    /*
     * degree of list nesting.
     */
    int listLevel;

    bool inBody;
    bool _inline;
};

#endif // HTMLPARSER_H

