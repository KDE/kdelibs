/*
    This file is part of the KDE libraries

    Copyright (C) 1997 Martin Jones (mjones@kde.org)
              (C) 1997 Torben Weis (weis@kde.org)
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

#define PARSER_DEBUG

#ifdef GrayScale
#undef GrayScale
#endif

#ifdef Color
#undef Color
#endif

#include "khtmlparser.h"

#include "dom_nodeimpl.h"
#include "dom_exception.h"
#include "html_baseimpl.h"
#include "html_blockimpl.h"
#include "html_documentimpl.h"
#include "html_elementimpl.h"
#include "html_formimpl.h"
#include "html_headimpl.h"
#include "html_imageimpl.h"
#include "html_inlineimpl.h"
#include "html_listimpl.h"
#include "html_miscimpl.h"
#include "html_tableimpl.h"
#include "html_objectimpl.h"
#include "dom_textimpl.h"

#include "khtmltags.h"
#include "khtmltoken.h"
#include "khtmlfont.h"
#include "khtmlattrs.h"
#include "khtmlstyle.h"
#include "khtmldata.h"
#include "khtml.h"

#include <stdio.h>

//----------------------------------------------------------------------------

// ### FIXME: move this list to dtd.cpp
//
// priority of tags. Closing tags of higher priority close tags of lower
// priority.
// Update this list, whenever you change khtmltags.*
//
// 0 elements with forbidden close tag and text. They don't get pushed
//   to the stack.
// 1 inline elements
// 2 regular block level elements
// 3 lists (OL UL DIR MENU)
// 4 TD TH
// 5 TR
// 6 tbody thead tfoot caption
// 7 table
// 8 body frameset
// 9 html
const unsigned short tagPriority[] = {
    0, // 0
    1, // ID_A == 1
    1, // ID_ABBR
    1, // ID_ACRONYM
    2, // ID_ADDRESS
    1, // ID_APPLET
    0, // ID_AREA
    1, // ID_B
    0, // ID_BASE
    0, // ID_BASEFONT
    1, // ID_BDO
    1, // ID_BIG
    2, // ID_BLOCKQUOTE
    8, // ID_BODY
    0, // ID_BR
    1, // ID_BUTTON
    1, // ID_CAPTION
    2, // ID_CENTER
    1, // ID_CITE
    1, // ID_CODE
    0, // ID_COL
    1, // ID_COLGROUP
    2, // ID_DD
    1, // ID_DEL
    1, // ID_DFN
    3, // ID_DIR
    2, // ID_DIV
    3, // ID_DL
    2, // ID_DT
    1, // ID_EM
    2, // ID_FIELDSET
    1, // ID_FONT
    2, // ID_FORM
    0, // ID_FRAME
    8, // ID_FRAMESET
    2, // ID_H1
    2, // ID_H2
    2, // ID_H3
    2, // ID_H4
    2, // ID_H5
    2, // ID_H6
    8, // ID_HEAD
    0, // ID_HR
    9, // ID_HTML
    1, // ID_I
    1, // ID_IFRAME
    0, // ID_IMG
    0, // ID_INPUT
    1, // ID_INS
    0, // ID_ISINDEX
    1, // ID_KBD
    1, // ID_LABEL
    1, // ID_LEGEND
    1, // ID_LI
    0, // ID_LINK
    1, // ID_LISTING
    1, // ID_MAP
    3, // ID_MENU
    0, // ID_META
    8, // ID_NOFRAMES
    2, // ID_NOSCRIPT
    1, // ID_OBJECT
    3, // ID_OL
    1, // ID_OPTGROUP
    1, // ID_OPTION
    2, // ID_P
    0, // ID_PARAM
    1, // ID_PLAIN
    1, // ID_PRE
    1, // ID_Q
    1, // ID_S
    1, // ID_SAMP
    1, // ID_SCRIPT
    1, // ID_SELECT
    1, // ID_SMALL
    1, // ID_SPAN
    1, // ID_STRIKE
    1, // ID_STRONG
    1, // ID_STYLE
    1, // ID_SUB
    1, // ID_SUP
    7, // ID_TABLE
    6, // ID_TBODY
    4, // ID_TD
    1, // ID_TEXTAREA
    6, // ID_TFOOT
    4, // ID_TH
    6, // ID_THEAD
    1, // ID_TITLE
    5, // ID_TR
    1, // ID_TT
    1, // ID_U
    3, // ID_UL
    1, // ID_VAR
    0, // ID_TEXT
};

/*
 * The parser parses tokenized input into the document, building up the
 * document tree. If the document is wellformed, parsing it is
 * straightforward.
 * Unfortunately, people can't write wellformed HTML documents, so the parser
 * has to be tolerant about errors.
 *
 * We have to take care of the following error conditions:
 * 1. The element being added is explicitly forbidden inside some outer tag.
 *    In this case we should close all tags up to the one, which forbids
 *    the element, and add it afterwards.
 * 2. We are not allowed to add the element directly. It could be, that
 *    the person writing the document forgot some tag inbetween (or that the
 *    tag inbetween is optional...) This could be the case with the following
 *    tags: HTML HEAD BODY TBODY TR TD LI (did I forget any????)
 * 3. We wan't to add a block element inside to an inline element. Close all
 *    inline elements up to the next higher block element.
 * 4. If this doesn't help close elements, until we are allowed to add the
 *    element or ignore the tag.
 *
 */
KHTMLParser::KHTMLParser( KHTMLWidget *_parent,
			  HTMLDocumentImpl *doc)
{
    //printf("parser constructor\n");

    HTMLWidget    = _parent;
    document      = doc;

    blockStack = 0;

    // ### default settings should be created only once per app...
    settings = new HTMLSettings;

    // Style stuff
    styleSheet = new CSSStyleSheet(settings);

#ifdef CSS_TEST
    styleSheet->test();
#endif

    currentStyle = styleSheet->newStyle(NULL);

    // ID_CLOSE_TAG == Num of tags
    forbiddenTag = new ushort[ID_CLOSE_TAG+1];

    reset();
}

KHTMLParser::~KHTMLParser()
{
    delete styleSheet;
    freeBlock();

    delete settings;

    delete [] forbiddenTag;

    if(currentStyle) delete currentStyle;
}

void KHTMLParser::reset()
{
    current = document;
    listLevel = -1; // no List...

    freeBlock();

    // before parsing no tags are forbidden...
    memset(forbiddenTag, 0, (ID_CLOSE_TAG+1)*sizeof(ushort));

    block = 0;

    inBody = false;
}

void KHTMLParser::parseToken(Token *t)
{
    if (t->id > 2*ID_CLOSE_TAG)
    {
      printf("Unknown tag!! tagID = %d\n", t->id);
      return;
    }
    if(t->id > ID_CLOSE_TAG)
    {
	processCloseTag(t);
	return;
    }
    NodeImpl *n = getElement(t);

    // just to be sure, and to catch currently unimplemented stuff
    if(!n) return;

    // set attributes
    if(n->isElementNode())
    {
	ElementImpl *e = static_cast<ElementImpl *>(n);
	e->setAttribute(t->attrs);

	// take care of optional close tags
	if(e->endTag() == DOM::OPTIONAL)
	    popBlock(t->id);
    }

    // if this tag is forbidden inside the current context, pop
    // blocks until we are allowed to add it...
    while(forbiddenTag[t->id]) popOneBlock();

    try
    {
	insertNode(n);
    }
    catch(DOMException)
    {
	// we couldn't insert the node...
	printf("insertNode failed!\n");
	delete n;
    }
}

void KHTMLParser::insertNode(NodeImpl *n)
{
    int id = n->id();

    // let's be stupid and just try to insert it.
    // this should work if the document is wellformed
    try
    {
#ifdef PARSER_DEBUG
	NodeImpl *tmp = current;
#endif
	NodeImpl *newNode = current->addChild(n);
#ifdef PARSER_DEBUG
	printf("added %s to %s, new current=%s\n",
	       n->nodeName().string().ascii(),
	       tmp->nodeName().string().ascii(),
	       newNode->nodeName().string().ascii());
#endif
	// don't push elements without end tag on the stack
	if(tagPriority[id] != 0)
	{
	    pushBlock(id, tagPriority[id], exitFunc, exitFuncData);
	    current = newNode;
	    if(!block && current->blocking())
		block = current;
	}
	else
	    n->calcMinMaxWidth();

	n->attach(HTMLWidget);

	if(n->isElementNode())
	    static_cast<HTMLElementImpl *>(n)->setStyle(currentStyle);

    }
    catch(DOMException exception)
    {
#ifdef PARSER_DEBUG
	printf("\nADDING NODE FAILED!!!!\ncurrent = %s, new = %s\n\n",
	       current->nodeName().string().ascii(),
	       n->nodeName().string().ascii());
#endif

	// error handling...
	HTMLElementImpl *e;
	bool ignore = false;

	if(!inBody && n->id() == ID_TEXT)
	{
	    TextImpl *t = static_cast<TextImpl *>(n);
	    // ignore spaces in head...
	    if(t->string()->l == 1 && ( *(t->string()->s) == QChar(' ') ||
					*(t->string()->s) == QChar(0xA0)) )
		return;
	}

	switch(current->id())
	{
	case ID_HTML:
	    switch(id)
	    {
		// ### check, there's only one HTML and one BODY tag!
	    case ID_SCRIPT:
	    case ID_STYLE:
	    case ID_META:
	    case ID_LINK:
	    case ID_OBJECT:
	    case ID_TITLE:
	    case ID_ISINDEX:
	    case ID_BASE:
		e = new HTMLHeadElementImpl(document);
		insertNode(e);
		break;
	    default:
		e = new HTMLBodyElementImpl(document, HTMLWidget);
		insertNode(e);
		break;
	    }
	    break;
	case ID_HEAD:
	    // we can get here only if the element is not allowed in head.
	    // This means the body starts here...
	    popBlock(ID_HEAD);
	    break;
	case ID_BODY:
	    ignore = true;
	    break;
	case ID_TABLE:
	    switch(id)
	    {
	    case ID_COL:
	    case ID_COLGROUP:
		ignore = true;
		break;
	    default:
		e = new HTMLTableSectionElementImpl(document, ID_TBODY);
		insertNode(e);
		break;
	    }
	    break;
	case ID_THEAD:
	case ID_TFOOT:
	case ID_TBODY:
	    e = new HTMLTableRowElementImpl(document);
	    insertNode(e);
	    break;
	case ID_TR:
	    e = new HTMLTableCellElementImpl(document, ID_TD);
	    insertNode(e);
	    break;
	case ID_UL:
	case ID_OL:
	case ID_DIR:
	case ID_MENU:
	    e = new HTMLLIElementImpl(document);
	    insertNode(e);
	    break;
	case ID_P:
	    if(!n->isInline())
		popBlock(ID_P);
	    break;
	default:
	    if(current->isDocumentNode())
	    {
		if(current->firstChild() != 0)
		{
		    // already have a HTML element...
		    ignore = true;
		}
		else
		{
		    e = new HTMLHtmlElementImpl(document);
		    insertNode(e);
		}
	    }
	    else if(current->isInline())
		popInlineBlocks();
	    else
		ignore = true;
	}	
	// if we couldn't handle the error, just rethrow the exception...
	if(ignore)
	{
	    printf("Exception handler failed in HTMLPArser::insertNode()\n");
	    throw exception;
	}
	
	insertNode(n);
    }
}


NodeImpl *KHTMLParser::getElement(Token *t)
{
    exitFunc = 0;
    exitFuncData = 0;

    NodeImpl *n = 0;

    switch(t->id)
    {
    case ID_HTML:
	n = new HTMLHtmlElementImpl(document);
	break;
    case ID_HEAD:
	n = new HTMLHeadElementImpl(document);
	break;
    case ID_BODY:
	n = new HTMLBodyElementImpl(document, HTMLWidget);
	break;

// head elements
    case ID_BASE:
	n = new HTMLBaseElementImpl(document);
	break;
    case ID_ISINDEX:
	n = new HTMLIsIndexElementImpl(document);
	break;
    case ID_LINK:
	n = new HTMLLinkElementImpl(document);
	break;
    case ID_META:
	n = new HTMLMetaElementImpl(document);
	break;
    case ID_STYLE:
	n = new HTMLStyleElementImpl(document);
	break;
    case ID_TITLE:
	n = new HTMLTitleElementImpl(document);
	break;

// frames
    case ID_FRAME:
	n = new HTMLFrameElementImpl(document);
	break;
    case ID_FRAMESET:
	n = new HTMLFrameSetElementImpl(document);
	break;
	// a bit a special case, since the frame is inlined...
    case ID_IFRAME:
	n = new HTMLIFrameElementImpl(document);
	break;

// form elements
    case ID_BUTTON:
	n = new HTMLButtonElementImpl(document);
	break;
    case ID_FIELDSET:
	n = new HTMLFieldSetElementImpl(document);
	break;
    case ID_FORM:
	n = new HTMLFormElementImpl(document);
	break;
    case ID_INPUT:
	n = new HTMLInputElementImpl(document);
	break;
    case ID_LABEL:
	n = new HTMLLabelElementImpl(document);
	break;
    case ID_LEGEND:
	n = new HTMLLegendElementImpl(document);
	break;
    case ID_OPTGROUP:
	n = new HTMLOptGroupElementImpl(document);
	break;
    case ID_OPTION:
	n = new HTMLOptionElementImpl(document);
	break;
    case ID_SELECT:
	n = new HTMLSelectElementImpl(document);
	break;
    case ID_TEXTAREA:
	n = new HTMLTextAreaElementImpl(document);
	break;

// lists
    case ID_DL:
	n = new HTMLDListElementImpl(document);
	break;
    case ID_DD:
	n = new HTMLGenericBlockElementImpl(document, t->id);
	popBlock(ID_DT);
	break;
    case ID_DT:
	// ###
	n = new HTMLInlineElementImpl(document, t->id);
	popBlock(ID_DD);
	break;
    case ID_UL:
    {
	HTMLUListElementImpl *l = new HTMLUListElementImpl(document);
	n = l;
	exitFunc = &KHTMLParser::blockEndList;
	exitFuncData = listLevel;
	// list levels > 255 indicate an OL as outer list
	if(listLevel > 255)
	    listLevel = 0;
	else
	    listLevel++;
	if(listLevel > 2) listLevel -= 3;
	l->setType((ListType)listLevel);
	break;
    }
    case ID_OL:
    {
	HTMLOListElementImpl *l = new HTMLOListElementImpl(document);
	n = l;
	exitFunc = &KHTMLParser::blockEndList;
	exitFuncData = listLevel;
	// list levels > 256 indicate an OL as outer list
	if(listLevel < 256)
	    listLevel = 256;
	else
	    listLevel++;
	if(listLevel > 260) listLevel -= 5;
	// numbered lists start at pos 3 of ListType...
	l->setType((ListType)(listLevel-256+3));
	break;
    }
    case ID_DIR:
	n = new HTMLDirectoryElementImpl(document);
	break;
    case ID_MENU:
	n = new HTMLMenuElementImpl(document);
	break;
    case ID_LI:
	n = new HTMLLIElementImpl(document);
	break;

// formatting elements (block)
    case ID_BLOCKQUOTE:
	n = new HTMLBlockquoteElementImpl(document);
	break;
    case ID_DIV:
	n = new HTMLDivElementImpl(document);
	break;
    case ID_H1:
    case ID_H2:
    case ID_H3:
    case ID_H4:
    case ID_H5:
    case ID_H6:
	n = new HTMLHeadingElementImpl(document, t->id);
	break;
    case ID_HR:
	n = new HTMLHRElementImpl(document);
	break;
    case ID_P:
	n = new HTMLParagraphElementImpl(document);
	break;
    case ID_PRE:
	n = new HTMLPreElementImpl(document);
	break;

// font stuff
    case ID_BASEFONT:
	n = new HTMLBaseFontElementImpl(document);
	break;
    case ID_FONT:
	n = new HTMLFontElementImpl(document);
	break;

// ins/del
    case ID_DEL:
    case ID_INS:
	n = new HTMLModElementImpl(document, t->id);
	break;

// anchor
    case ID_A:
	n = new HTMLAnchorElementImpl(document);
	break;

// images
    case ID_IMG:
	n = new HTMLImageElementImpl(document);
	break;
    case ID_MAP:
	n = new HTMLMapElementImpl(document);
	break;
    case ID_AREA:
	n = new HTMLAreaElementImpl(document);
	break;

// objects, applets and scripts
    case ID_APPLET:
	n = new HTMLAppletElementImpl(document);
	break;
    case ID_OBJECT:
	n = new HTMLObjectElementImpl(document);
	break;
    case ID_PARAM:
	n = new HTMLParamElementImpl(document);
	break;
    case ID_SCRIPT:
	n = new HTMLScriptElementImpl(document);
	break;

// tables
    case ID_TABLE:
	n = new HTMLTableElementImpl(document);
	break;
    case ID_CAPTION:
	n = new HTMLTableCaptionElementImpl(document);
	break;
    case ID_COLGROUP:
    case ID_COL:
	n = new HTMLTableColElementImpl(document, t->id);
	break;
    case ID_TR:
	n = new HTMLTableRowElementImpl(document);
	break;
    case ID_TD:
    case ID_TH:
	n = new HTMLTableCellElementImpl(document, t->id);
	break;
    case ID_THEAD:
    case ID_TBODY:
    case ID_TFOOT:
	n = new HTMLTableSectionElementImpl(document, t->id);
	break;

// inline elements
    case ID_BR:
	n = new HTMLBRElementImpl(document);
	break;
    case ID_Q:
	n = new HTMLQuoteElementImpl(document);
	break;

// elements with no special representation in the DOM

// block:
    case ID_ADDRESS:
    case ID_CENTER:
	n = new HTMLGenericBlockElementImpl(document, t->id);
	break;
// inline
	// %fontstyle
    case ID_TT:
    case ID_U:
    case ID_B:
    case ID_I:
    case ID_S:
    case ID_STRIKE:
    case ID_BIG:
    case ID_SMALL:

	// %phrase
    case ID_EM:
    case ID_STRONG:
    case ID_DFN:
    case ID_CODE:
    case ID_SAMP:
    case ID_KBD:
    case ID_VAR:
    case ID_CITE:
    case ID_ABBR:
    case ID_ACRONYM:

	// %special
    case ID_SUB:
    case ID_SUP:
	n = new HTMLInlineElementImpl(document, t->id);
	break;

    case ID_BDO:
    case ID_SPAN:
	break;

	// these are special, and normally not rendered
    case ID_NOFRAMES:
    case ID_NOSCRIPT:
	// Waldo's plaintext stuff
    case ID_PLAIN:
	return 0;
	break;
// text
    case ID_TEXT:
	n = new TextImpl(document, t->text);
	break;

    default:
	printf("Unknown tag %d!\n", t->id);
    }
    return n;
}

void KHTMLParser::processCloseTag(Token *t)
{
#ifdef PARSER_DEBUG
    printf("added the following childs to %s\n", current->nodeName().string().ascii());
    NodeImpl *child = current->firstChild();
    while(child != 0)
    {
	printf("    %s\n", child->nodeName().string().ascii());
	child = child->nextSibling();
    }
#endif
    popBlock(t->id-ID_CLOSE_TAG);
#ifdef PARSER_DEBUG
    printf("closeTag --> current = %s\n", current->nodeName().string().ascii());
#endif
}


void KHTMLParser::pushBlock(int _id, int _level,
                            blockFunc _exitFunc,
                            int _miscData1)
{
    HTMLStackElem *Elem = new HTMLStackElem(_id, _level, currentStyle,
					    current, _exitFunc, _miscData1,
    					    blockStack);

    currentStyle = styleSheet->newStyle(currentStyle);
    blockStack = Elem;
    addForbidden(_id, forbiddenTag);
}    					

void KHTMLParser::popBlock( int _id )
{
    HTMLStackElem *Elem = blockStack;
    int maxLevel = 0;

    while( Elem && (Elem->id != _id))
    {
    	if (maxLevel < Elem->level)
    	{
    	    maxLevel = Elem->level;
    	}
    	Elem = Elem->next;
    }
    if (!Elem || maxLevel > Elem->level)
	return;

    Elem = blockStack;

    while (Elem)
    {
    	if (Elem->id == _id)
	{
	    popOneBlock();
    	    Elem = 0;
	}
    	else
	{
	    popOneBlock();
	    Elem = blockStack;
	}
    }
}

void KHTMLParser::popOneBlock()
{
    HTMLStackElem *Elem = blockStack;		

#ifdef PARSER_DEBUG
    printf("popping block: %d\n", Elem->id);
#endif

    if(Elem->node != current)
    {
	current->close();

	if(!block)
	{
	    // ### find a more efficient way...
	    int absX, absY;
	    current->getAbsolutePosition(absX, absY);
	    int height = absY + current->getDescent() + 2*5; // 5==BORDER
	    int docHeight = HTMLWidget->contentsHeight();
	    if(height > docHeight)
		HTMLWidget->resizeContents(HTMLWidget->contentsWidth(), height);
	}
	else if(block == current)
	{
	    block = 0;
	}
    }
    if (Elem->exitFunc != 0)
	(this->*(Elem->exitFunc))( Elem );

    removeForbidden(Elem->id, forbiddenTag);

    if (Elem && Elem->style)
    {
	delete currentStyle;
	currentStyle = Elem->style;
    }
    blockStack = Elem->next;
    current = Elem->node;

    delete Elem;
}

void KHTMLParser::popInlineBlocks()
{
    while(current->isInline())
	popOneBlock();
}

void KHTMLParser::freeBlock()
{
    while (blockStack)
	popOneBlock();
    blockStack = 0;
}

void KHTMLParser::blockEndList( HTMLStackElem *Elem)
{
    listLevel = Elem->miscData1;
}


