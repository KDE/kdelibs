/*
    This file is part of the KDE libraries

    Copyright (C) 1997 Martin Jones (mjones@kde.org)
              (C) 1997 Torben Weis (weis@kde.org)
              (C) 1999,2001 Lars Knoll (knoll@kde.org)
              (C) 2000,2001 Dirk Mueller (mueller@kde.org)

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
//#define PARSER_DEBUG

#include "dom/dom_exception.h"

#include "html/html_baseimpl.h"
#include "html/html_blockimpl.h"
#include "html/html_documentimpl.h"
#include "html/html_elementimpl.h"
#include "html/html_formimpl.h"
#include "html/html_headimpl.h"
#include "html/html_imageimpl.h"
#include "html/html_inlineimpl.h"
#include "html/html_listimpl.h"
#include "html/html_miscimpl.h"
#include "html/html_tableimpl.h"
#include "html/html_objectimpl.h"
#include "xml/dom_textimpl.h"
#include "xml/dom_nodeimpl.h"
#include "misc/htmlhashes.h"
#include "html/htmltokenizer.h"
#include "khtmlview.h"
#include "khtml_part.h"
#include "css/cssproperties.h"
#include "css/cssvalues.h"

#include "rendering/render_object.h"

#include "html/htmlparser.h"
#include <kdebug.h>
#include <klocale.h>

using namespace DOM;
using namespace khtml;

//----------------------------------------------------------------------------

/**
 * @internal
 */
class HTMLStackElem
{
public:
    HTMLStackElem( int _id,
                   int _level,
                   DOM::NodeImpl *_node,
		   bool _inline,
                   HTMLStackElem * _next )
        :
        id(_id),
        level(_level),
	m_inline(_inline),
        node(_node),
        next(_next)
        { node->ref(); }

    ~HTMLStackElem()
        { node->deref(); }

    int       id;
    int       level;
    bool m_inline;
    NodeImpl *node;
    HTMLStackElem *next;
};

/**
 * @internal
 *
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
 *    tags: HTML HEAD BODY TBODY TR TD LI (did I forget any?)
 * 3. We wan't to add a block element inside to an inline element. Close all
 *    inline elements up to the next higher block element.
 * 4. If this doesn't help close elements, until we are allowed to add the
 *    element or ignore the tag.
 *
 */

KHTMLParser::KHTMLParser( KHTMLView *_parent, DocumentPtr *doc)
{
    //kdDebug( 6035 ) << "parser constructor" << endl;
#if SPEED_DEBUG > 0
    qt.start();
#endif

    HTMLWidget    = _parent;
    document      = doc;
    document->ref();

    blockStack = 0;
    current = 0;

    // ID_CLOSE_TAG == Num of tags
    forbiddenTag = new ushort[ID_CLOSE_TAG+1];

    reset();
}

KHTMLParser::KHTMLParser( DOM::DocumentFragmentImpl *i, DocumentPtr *doc )
{
    HTMLWidget = 0;
    document = doc;
    document->ref();

    forbiddenTag = new ushort[ID_CLOSE_TAG+1];

    blockStack = 0;
    current = 0;

    reset();

    current = i;
    current->ref();

    inBody = true;
}

KHTMLParser::~KHTMLParser()
{
#if SPEED_DEBUG > 0
    kdDebug( ) << "TIME: parsing time was = " << qt.elapsed() << endl;
#endif

    freeBlock();

    if (current) current->deref();

    document->deref();

    delete [] forbiddenTag;
    delete isindex;
}

void KHTMLParser::reset()
{
    setCurrent ( document->document() );

    freeBlock();

    // before parsing no tags are forbidden...
    memset(forbiddenTag, 0, (ID_CLOSE_TAG+1)*sizeof(ushort));

    inBody = false;
    haveFrameSet = false;
    haveContent = false;
    haveBody = false;
    haveTitle = false;
    haveMalformedTable = false;
    inSelect = false;
    m_inline = false;

    inPre = 0;
    form = 0;
    map = 0;
    head = 0;
    end = false;
    isindex = 0;

    discard_until = 0;
}

void KHTMLParser::parseToken(Token *t)
{
    if (t->tid > 2*ID_CLOSE_TAG)
    {
      kdDebug( 6035 ) << "Unknown tag!! tagID = " << t->tid << endl;
      return;
    }
    if(discard_until) {
        if(t->tid == discard_until)
            discard_until = 0;

        // do not skip </iframe>
        if ( discard_until || current->id() + ID_CLOSE_TAG != t->tid )
            return;
    }

#ifdef PARSER_DEBUG
    kdDebug( 6035 ) << "\n\n==> parser: processing token " << getTagName(t->tid) << "(" << t->tid << ")"
                    << " current = " << getTagName(current->id()) << "(" << current->id() << ")" << endl;
    kdDebug(6035) << "inline=" << m_inline << " inBody=" << inBody << " haveFrameSet=" << haveFrameSet << " haveContent=" << haveContent << endl;
#endif

    // holy shit. apparently some sites use </br> instead of <br>
    // be compatible with IE and NS
    if(t->tid == ID_BR+ID_CLOSE_TAG && document->document()->parseMode() != DocumentImpl::Strict)
        t->tid -= ID_CLOSE_TAG;

    if(t->tid > ID_CLOSE_TAG)
    {
        processCloseTag(t);
        return;
    }

    // ignore spaces, if we're not inside a paragraph or other inline code
    if( t->tid == ID_TEXT && t->text ) {
        if(inBody && !skipMode() &&
           current->id() != ID_STYLE && current->id() != ID_TITLE &&
           current->id() != ID_SCRIPT &&
           t->text->l > 2) haveContent = true;
#ifdef PARSER_DEBUG
        kdDebug(6035) << "length="<< t->text->l << " text='" << QConstString(t->text->s, t->text->l).string() << "'" << endl;
#endif
    }

    NodeImpl *n = getElement(t);
    // just to be sure, and to catch currently unimplemented stuff
    if(!n)
        return;

    // set attributes
    if(n->isElementNode() && t->tid != ID_ISINDEX)
    {
        ElementImpl *e = static_cast<ElementImpl *>(n);
        e->setAttributeMap(t->attrs);

        // take care of optional close tags
        if(endTag[e->id()] == DOM::OPTIONAL)
            popBlock(t->tid);
    }

    // if this tag is forbidden inside the current context, pop
    // blocks until we are allowed to add it...
    while(forbiddenTag[t->tid]) {
#ifdef PARSER_DEBUG
        kdDebug( 6035 ) << "t->id: " << t->tid << " is forbidden :-( " << endl;
#endif
        popOneBlock();
    }

    // sometimes flat doesn't make sense
    switch(t->tid) {
    case ID_SELECT:
    case ID_OPTION:
        t->flat = false;
    }

    // the tokenizer needs the feedback for space discarding
    if ( tagPriority[t->tid] == 0 )
	t->flat = true;

    if ( !insertNode(n, t->flat) ) {
        // we couldn't insert the node...
#ifdef PARSER_DEBUG
        kdDebug( 6035 ) << "insertNode failed current=" << current->id() << ", new=" << n->id() << "!" << endl;
#endif
        if (map == n)
        {
#ifdef PARSER_DEBUG
            kdDebug( 6035 ) << "  --> resetting map!" << endl;
#endif
            map = 0;
        }
        if (form == n)
        {
#ifdef PARSER_DEBUG
            kdDebug( 6035 ) << "   --> resetting form!" << endl;
#endif
            form = 0;
        }
        delete n;
    }
}

bool KHTMLParser::insertNode(NodeImpl *n, bool flat)
{
    int id = n->id();

    // let's be stupid and just try to insert it.
    // this should work if the document is wellformed
#ifdef PARSER_DEBUG
    NodeImpl *tmp = current;
#endif
    NodeImpl *newNode = current->addChild(n);
    if ( newNode ) {
#ifdef PARSER_DEBUG
        kdDebug( 6035 ) << "added " << n->nodeName().string() << " to " << tmp->nodeName().string() << ", new current=" << newNode->nodeName().string() << endl;
#endif

	// don't push elements without end tag on the stack
        if(tagPriority[id] != 0 && !flat) {
#if SPEED_DEBUG < 2
            if(!n->attached() && HTMLWidget )
                n->attach();
#endif
	    if(n->isInline()) m_inline = true;
            pushBlock(id, tagPriority[id]);
            setCurrent( newNode );
        } else {
#if SPEED_DEBUG < 2
            if(!n->attached() && HTMLWidget)
                n->attach();
            if (n->maintainsState()) {
                document->document()->registerMaintainsState(n);
                QString state(document->document()->nextState());
                if (!state.isNull()) n->restoreState(state);
            }
            n->closeRenderer();
#endif
	    if(n->isInline()) m_inline = true;
        }

#if SPEED_DEBUG < 1
        if(tagPriority[id] == 0 && n->renderer())
            n->renderer()->calcMinMaxWidth();
#endif
        return true;
    } else {
#ifdef PARSER_DEBUG
        kdDebug( 6035 ) << "ADDING NODE FAILED!!!! current = " << current->nodeName().string() << ", new = " << n->nodeName().string() << endl;
#endif
        // error handling...
        HTMLElementImpl *e;
        bool handled = false;

	// never create anonymous objects just to hold a space.
	if ( id == ID_TEXT &&
	     static_cast<TextImpl *>(n)->string()->l == 1 &&
	     static_cast<TextImpl *>(n)->string()->s[0] == " " )
	    return false;

        // switch according to the element to insert
        switch(id)
        {
        case ID_COMMENT:
            break;
        case ID_HEAD:
            // ### alllow not having <HTML> in at all, as per HTML spec
            if (!current->isDocumentNode() && current->id() != ID_HTML )
                return false;
            break;
        case ID_META:
        case ID_LINK:
        case ID_ISINDEX:
        case ID_BASE:
            if( !head )
                createHead();
            if( head ) {
                if ( head->addChild(n) ) {
#if SPEED_DEBUG < 2
                    if(!n->attached() && HTMLWidget)
                        n->attach();
#endif
		}

                return true;
            }

            break;
        case ID_HTML:
            if (!current->isDocumentNode() ) {
		if ( doc()->firstChild()->id() == ID_HTML) {
		    // we have another <HTML> element.... apply attributes to existing one
		    // make sure we don't overwrite already existing attributes
		    NamedAttrMapImpl *map = static_cast<ElementImpl*>(n)->attributes(true);
		    NamedAttrMapImpl *bmap = static_cast<ElementImpl*>(doc()->firstChild())->attributes(false);
		    bool changed = false;
		    for (unsigned long l = 0; map && l < map->length(); ++l) {
			NodeImpl::Id attrId = map->idAt(l);
			DOMStringImpl *attrValue = map->valueAt(l);
			changed = !bmap->getValue(attrId);
			bmap->setValue(attrId,attrValue);
		    }
		    if ( changed )
			doc()->recalcStyle( NodeImpl::Inherit );
		}
                return false;
	    }
            break;
        case ID_TITLE:
        case ID_STYLE:
            if ( !head )
                createHead();
            if ( head ) {
                DOM::NodeImpl *newNode = head->addChild(n);
                if ( newNode ) {
                    pushBlock(id, tagPriority[id]);
                    setCurrent ( newNode );
#if SPEED_DEBUG < 2
		    if(!n->attached() && HTMLWidget)
                        n->attach();
#endif
                } else {
#ifdef PARSER_DEBUG
                    kdDebug( 6035 ) << "adding style before to body failed!!!!" << endl;
#endif
                    discard_until = ID_STYLE + ID_CLOSE_TAG;
                    return false;
                }
                return true;
            } else if(inBody) {
                discard_until = id + ID_CLOSE_TAG;
                return false;
            }
            break;
        case ID_SCRIPT:
            // if we failed to insert it, go into skip mode
            discard_until = id + ID_CLOSE_TAG;
            break;
        case ID_BODY:
            if(inBody && doc()->body()) {
                // we have another <BODY> element.... apply attributes to existing one
                // make sure we don't overwrite already existing attributes
                // some sites use <body bgcolor=rightcolor>...<body bgcolor=wrongcolor>
                NamedAttrMapImpl *map = static_cast<ElementImpl*>(n)->attributes(true);
                NamedAttrMapImpl *bmap = doc()->body()->attributes(false);
                bool changed = false;
                for (unsigned long l = 0; map && l < map->length(); ++l) {
                    NodeImpl::Id attrId = map->idAt(l);
                    DOMStringImpl *attrValue = map->valueAt(l);
                    if ( !bmap->getValue(attrId) ) {
                        bmap->setValue(attrId,attrValue);
                        changed = true;
                    }
                }
                if ( changed )
                    doc()->recalcStyle( NodeImpl::Inherit );
            } else if ( current->isDocumentNode() )
                break;
            return false;
            break;

            // the following is a hack to move non rendered elements
            // outside of tables.
            // needed for broken constructs like <table><form ...><tr>....
        case ID_INPUT:
        {
            ElementImpl *e = static_cast<ElementImpl *>(n);
            DOMString type = e->getAttribute(ATTR_TYPE);

            if ( strcasecmp( type, "hidden" ) != 0 )
                break;
            // Fall through!
        }
        case ID_TEXT:
            // ignore text inside the following elements.
            switch(current->id())
            {
            case ID_SELECT:
                return false;
            default:
                ;
                // fall through!!
            };
            break;
         case ID_DL:
             popBlock( ID_DT );
             if ( current->id() == ID_DL ) {
                 e = new HTMLGenericElementImpl( document, ID_DD );
                 insertNode( e );
                 handled = true;
             }
             break;
        case ID_DD:
            popBlock( ID_DD );
            /* nobreak */
        case ID_DT:
            if ( checkChild( current->id(), ID_DL ) ) {
                e = new HTMLDListElementImpl( document );
                insertNode( e );
                handled = true;
            }
            break;
        case ID_AREA:
        {
            if(map)
            {
                map->addChild(n);
#if SPEED_DEBUG < 2
                if(!n->attached() && HTMLWidget)
		    n->attach();
#endif
                handled = true;
                return true;
            }
            return false;
        }
        case ID_THEAD:
        case ID_TBODY:
        case ID_TFOOT:
        case ID_TR:
        case ID_TD:
        case ID_TH:
            // lets try to close the stray table content
            if ( haveMalformedTable ) {
                while (haveMalformedTable)
                    popOneBlock();
                handled = true;
                break;
            }
        default:
            break;
        }

        // switch on the currently active element
        switch(current->id())
        {
        case ID_HTML:
            switch(id)
            {
            case ID_SCRIPT:
            case ID_STYLE:
            case ID_META:
            case ID_LINK:
            case ID_OBJECT:
            case ID_EMBED:
            case ID_TITLE:
            case ID_ISINDEX:
            case ID_BASE:
                if(!head) {
                    head = new HTMLHeadElementImpl(document);
                    e = head;
                    insertNode(e);
                    handled = true;
                }
                break;
            default:
                if ( haveFrameSet ) break;
                e = new HTMLBodyElementImpl(document);
                startBody();
                insertNode(e);
                handled = true;
                break;
            }
            break;
        case ID_HEAD:
            // we can get here only if the element is not allowed in head.
            if (id == ID_HTML)
                return false;
            else {
                // This means the body starts here...
                if ( haveFrameSet ) break;
                popBlock(ID_HEAD);
                e = new HTMLBodyElementImpl(document);
                startBody();
                insertNode(e);
                handled = true;
            }
            break;
        case ID_BODY:
            break;
        case ID_TABLE:
        case ID_THEAD:
        case ID_TFOOT:
        case ID_TBODY:
        case ID_TR:
            switch(id)
            {
            case ID_TABLE:
                popBlock(ID_TABLE); // end the table
                handled = checkChild( current->id(), id);
                break;
            case ID_TEXT:
            {
                TextImpl *t = static_cast<TextImpl *>(n);
                DOMStringImpl *i = t->string();
                unsigned int pos = 0;
                while(pos < i->l && ( *(i->s+pos) == QChar(' ') ||
                                      *(i->s+pos) == QChar(0xa0))) pos++;
                if(pos == i->l)
                    break;
            }
            default:
            {
                NodeImpl *node = current;
                NodeImpl *parent = node->parentNode();

                NodeImpl *parentparent = parent->parentNode();

                if(( node->id() == ID_TR &&
                     ( parent->id() == ID_THEAD ||
                       parent->id() == ID_TBODY ||
                       parent->id() == ID_TFOOT ) && parentparent->id() == ID_TABLE ) ||
                   ( !checkChild( ID_TR, id ) && ( node->id() == ID_THEAD || node->id() == ID_TBODY || node->id() == ID_TFOOT ) &&
                     parent->id() == ID_TABLE ) )
                {
                    node = ( node->id() == ID_TR ) ? parentparent : parent;
                    NodeImpl *parent = node->parentNode();
                    int exceptioncode = 0;
                    parent->insertBefore(n, node, exceptioncode);
                    if (exceptioncode) {
#ifdef PARSER_DEBUG
                        kdDebug(6035) << "adding content before table failed.." << endl;
#endif
                        break;
                    }
                    if ( n->isElementNode() && tagPriority[id] != 0 &&
                         !flat && endTag[id] != DOM::FORBIDDEN ) {

                        pushBlock(id, tagPriority[id]);
                        setCurrent ( n );
                        haveMalformedTable = true;
                    }
                    return true;
                }

                if ( current->id() == ID_TR )
                    e = new HTMLTableCellElementImpl(document, ID_TD);
                else if ( current->id() == ID_TABLE )
                    e = new HTMLTableSectionElementImpl( document, ID_TBODY, true /* implicit */ );
                else
                    e = new HTMLTableRowElementImpl( document );

                insertNode(e);
                handled = true;
                break;
            } // end default
            } // end switch
            break;
        case ID_OBJECT:
            discard_until = id + ID_CLOSE_TAG;
            return false;
        case ID_UL:
        case ID_OL:
        case ID_DIR:
        case ID_MENU:
            e = new HTMLLIElementImpl(document);
            e->addCSSProperty(CSS_PROP_LIST_STYLE_TYPE, CSS_VAL_NONE);
            insertNode(e);
            handled = true;
            break;
        case ID_DL:
            e = new HTMLGenericElementImpl( document, ID_DT );
            insertNode( e );
            handled = true;
            break;
        case ID_SELECT:
            if( n->isInline() )
                return false;
            break;
        case ID_P:
        case ID_H1:
        case ID_H2:
        case ID_H3:
        case ID_H4:
        case ID_H5:
        case ID_H6:
            if(!n->isInline())
            {
                popBlock(current->id());
                handled = true;
            }
            break;
        case ID_OPTION:
            if (id == ID_OPTGROUP)
            {
                popBlock(ID_OPTION);
                handled = true;
            }
            else if(id == ID_SELECT)
            {
                // IE treats a nested select as </select>. Let's do the same
                popBlock( ID_SELECT );
                break;
            }
            break;
            // head elements in the body should be ignored.

        case ID_ADDRESS:
        case ID_COLGROUP:
        case ID_FONT:
        case ID_CAPTION:
            popBlock(current->id());
            handled = true;
            break;
        default:
            if(current->isDocumentNode())
            {
                if(current->firstChild() == 0) {
                    e = new HTMLHtmlElementImpl(document);
                    insertNode(e);
                    handled = true;
                }
            }
            else if(current->isInline())
            {
                popInlineBlocks();
                handled = true;
            }
        }

        // if we couldn't handle the error, just rethrow the exception...
        if(!handled)
        {
            //kdDebug( 6035 ) << "Exception handler failed in HTMLPArser::insertNode()" << endl;
            return false;
        }

        return insertNode(n);
    }
}


NodeImpl *KHTMLParser::getElement(Token* t)
{
    NodeImpl *n = 0;

    switch(t->tid)
    {
    case ID_HTML:
        n = new HTMLHtmlElementImpl(document);
        break;
    case ID_HEAD:
        if(!head && current->id() == ID_HTML) {
            head = new HTMLHeadElementImpl(document);
            n = head;
        }
        break;
    case ID_BODY:
        // body no longer allowed if we have a frameset
        if(haveFrameSet) break;
        popBlock(ID_HEAD);
        n = new HTMLBodyElementImpl(document);
        haveBody =  true;
        startBody();
        break;

// head elements
    case ID_BASE:
        n = new HTMLBaseElementImpl(document);
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
        // only one non-empty <title> allowed
        if (haveTitle) {
            discard_until = ID_TITLE+ID_CLOSE_TAG;
            break;
        }
        n = new HTMLTitleElementImpl(document);
        // we'll set haveTitle when closing the tag
        break;

// frames
    case ID_FRAME:
        n = new HTMLFrameElementImpl(document);
        break;
    case ID_FRAMESET:
        popBlock(ID_HEAD);
        if ( inBody && !haveFrameSet && !haveContent && !haveBody) {
            popBlock( ID_BODY );
            // ### actually for IE document.body returns the now hidden "body" element
            // we can't implement that behavior now because it could cause too many
            // regressions and the headaches are not worth the work as long as there is
            // no site actually relying on that detail (Dirk)
            if (static_cast<HTMLDocumentImpl*>(document->document())->body())
                static_cast<HTMLDocumentImpl*>(document->document())->body()
                    ->addCSSProperty(CSS_PROP_DISPLAY, CSS_VAL_NONE);
            inBody = false;
        }
        if ( (haveBody || haveContent || haveFrameSet) && current->id() == ID_HTML)
            break;
        n = new HTMLFrameSetElementImpl(document);
        haveFrameSet = true;
        startBody();
        break;
        // a bit a special case, since the frame is inlined...
    case ID_IFRAME:
        n = new HTMLIFrameElementImpl(document);
        if (!t->flat) discard_until = ID_IFRAME+ID_CLOSE_TAG;
        break;

// form elements
    case ID_FORM:
        // thou shall not nest <form> - NS/IE quirk
        if (form) break;
        n = form = new HTMLFormElementImpl(document, false);
        break;
    case ID_BUTTON:
        n = new HTMLButtonElementImpl(document, form);
        break;
    case ID_FIELDSET:
        n = new HTMLFieldSetElementImpl(document, form);
        break;
    case ID_INPUT:
        n = new HTMLInputElementImpl(document, form);
        break;
    case ID_ISINDEX:
        n = handleIsindex(t);
        if( !inBody ) {
            isindex = n;
            n = 0;
        } else
            t->flat = true;
        break;
    case ID_KEYGEN:
        n = new HTMLKeygenElementImpl(document, form);
        break;
    case ID_LABEL:
        n = new HTMLLabelElementImpl(document);
        break;
    case ID_LEGEND:
        n = new HTMLLegendElementImpl(document, form);
        break;
    case ID_OPTGROUP:
        n = new HTMLOptGroupElementImpl(document, form);
        break;
    case ID_OPTION:
        n = new HTMLOptionElementImpl(document, form);
        break;
    case ID_SELECT:
        inSelect = true;
        n = new HTMLSelectElementImpl(document, form);
        break;
    case ID_TEXTAREA:
        n = new HTMLTextAreaElementImpl(document, form);
        break;

// lists
    case ID_DL:
        n = new HTMLDListElementImpl(document);
        break;
    case ID_DD:
        n = new HTMLGenericElementImpl(document, t->tid);
        popBlock(ID_DT);
        popBlock(ID_DD);
        break;
    case ID_DT:
        n = new HTMLGenericElementImpl(document, t->tid);
        popBlock(ID_DD);
        popBlock(ID_DT);
        break;
    case ID_UL:
    {
        n = new HTMLUListElementImpl(document);
        break;
    }
    case ID_OL:
    {
        n = new HTMLOListElementImpl(document);
        break;
    }
    case ID_DIR:
        n = new HTMLDirectoryElementImpl(document);
        break;
    case ID_MENU:
        n = new HTMLMenuElementImpl(document);
        break;
    case ID_LI:
    {
        popBlock(ID_LI);
        HTMLElementImpl *e = new HTMLLIElementImpl(document);
        n = e;
        break;
    }
// formatting elements (block)
    case ID_BLOCKQUOTE:
    case ID_LAYER:
    case ID_ILAYER:
        n = new HTMLGenericElementImpl(document, t->tid);
        break;
    case ID_P:
    case ID_DIV:
        n = new HTMLDivElementImpl(document, t->tid);
        break;
    case ID_H1:
    case ID_H2:
    case ID_H3:
    case ID_H4:
    case ID_H5:
    case ID_H6:
        n = new HTMLGenericElementImpl(document, t->tid);
        break;
    case ID_HR:
        n = new HTMLHRElementImpl(document);
        break;
    case ID_PRE:
        ++inPre;
    case ID_XMP:
    case ID_PLAINTEXT:
        n = new HTMLPreElementImpl(document, t->tid);
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
        n = new HTMLGenericElementImpl(document, t->tid);
        break;

// anchor
    case ID_A:
        if (blockStack && blockStack->id == ID_A)
            popBlock(ID_A);

        n = new HTMLAnchorElementImpl(document);
        break;

// images
    case ID_IMG:
        n = new HTMLImageElementImpl(document, form);
        break;
    case ID_MAP:
        map = new HTMLMapElementImpl(document);
        n = map;
        break;
    case ID_AREA:
        n = new HTMLAreaElementImpl(document);
        break;

// objects, applets and scripts
    case ID_APPLET:
        n = new HTMLAppletElementImpl(document);
        break;
    case ID_EMBED:
        n = new HTMLEmbedElementImpl(document);
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
        n = new HTMLTableColElementImpl(document, t->tid);
        break;
    case ID_TR:
        popBlock(ID_TR);
        n = new HTMLTableRowElementImpl(document);
        break;
    case ID_TD:
    case ID_TH:
        popBlock(ID_TH);
        popBlock(ID_TD);
        n = new HTMLTableCellElementImpl(document, t->tid);
        break;
    case ID_TBODY:
    case ID_THEAD:
    case ID_TFOOT:
        popBlock( ID_THEAD );
        popBlock( ID_TBODY );
        popBlock( ID_TFOOT );
        n = new HTMLTableSectionElementImpl(document, t->tid, false);
        break;

// inline elements
    case ID_BR:
        n = new HTMLBRElementImpl(document);
        break;
    case ID_Q:
        n = new HTMLGenericElementImpl(document, t->tid);
        break;

// elements with no special representation in the DOM

// block:
    case ID_ADDRESS:
    case ID_CENTER:
        n = new HTMLGenericElementImpl(document, t->tid);
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
    case ID_SPAN:
    case ID_NOBR:
    case ID_WBR:
    case ID_BDO:
        n = new HTMLGenericElementImpl(document, t->tid);
        break;

        // these are special, and normally not rendered
    case ID_NOEMBED:
        if (!t->flat) {
            n = new HTMLGenericElementImpl(document, t->tid);
            discard_until = ID_NOEMBED + ID_CLOSE_TAG;
        }
        return n;
    case ID_NOFRAMES:
        if (!t->flat) {
            n = new HTMLGenericElementImpl(document, t->tid);
            discard_until = ID_NOFRAMES + ID_CLOSE_TAG;
        }
        return n;
    case ID_NOSCRIPT:
        if (!t->flat) {
            n = new HTMLGenericElementImpl(document, t->tid);
            if(HTMLWidget && HTMLWidget->part()->jScriptEnabled())
                discard_until = ID_NOSCRIPT + ID_CLOSE_TAG;
        }
        return n;
    case ID_NOLAYER:
//        discard_until = ID_NOLAYER + ID_CLOSE_TAG;
        return 0;
        break;
    case ID_MARQUEE:
        n = new HTMLGenericElementImpl(document, t->tid);
        break;
// text
    case ID_TEXT:
//        kdDebug(6035) << "ID_TEXT: \"" << DOMString(t->text).string() << "\"" << endl;
        n = new TextImpl(document, t->text);
        break;
    case ID_COMMENT:
#ifdef COMMENTS_IN_DOM
        n = new CommentImpl(document, t->text);
#endif
        break;
    default:
        kdDebug( 6035 ) << "Unknown tag " << t->tid << "!" << endl;
    }
    return n;
}

void KHTMLParser::processCloseTag(Token *t)
{
    // support for really broken html. Can't believe I'm supporting such crap (lars)
    switch(t->tid)
    {
    case ID_HTML+ID_CLOSE_TAG:
    case ID_BODY+ID_CLOSE_TAG:
    case ID_DT+ID_CLOSE_TAG:
    case ID_DD+ID_CLOSE_TAG:
        // we never trust those close tags, since stupid webpages close
        // them prematurely
        return;
    case ID_FORM+ID_CLOSE_TAG:
        form = 0;
        // this one is to get the right style on the body element
        break;
    case ID_MAP+ID_CLOSE_TAG:
        map = 0;
        break;
    case ID_SELECT+ID_CLOSE_TAG:
        inSelect = false;
        break;
    case ID_TITLE+ID_CLOSE_TAG:
        // Set haveTitle only if <title> isn't empty
        if ( current->firstChild() )
            haveTitle = true;
        break;
    default:
        break;
    }

#ifdef PARSER_DEBUG
    kdDebug( 6035 ) << "added the following children to " << current->nodeName().string() << endl;
    NodeImpl *child = current->firstChild();
    while(child != 0)
    {
        kdDebug( 6035 ) << "    " << child->nodeName().string() << endl;
        child = child->nextSibling();
    }
#endif
    popBlock(t->tid-ID_CLOSE_TAG);
#ifdef PARSER_DEBUG
    kdDebug( 6035 ) << "closeTag --> current = " << current->nodeName().string() << endl;
#endif
}


void KHTMLParser::pushBlock(int _id, int _level)
{
    HTMLStackElem *Elem = new HTMLStackElem(_id, _level, current, m_inline, blockStack);

    blockStack = Elem;
    addForbidden(_id, forbiddenTag);
}

void KHTMLParser::popBlock( int _id )
{
    HTMLStackElem *Elem = blockStack;
    int maxLevel = 0;

#ifdef PARSER_DEBUG
    kdDebug( 6035 ) << "popBlock(" << getTagName(_id) << ")" << endl;
    while(Elem) {
        kdDebug( 6035) << "   > " << getTagName(Elem->id) << endl;
        Elem = Elem->next;
    }
    Elem = blockStack;
#endif

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

    // we should never get here, but some bad html might cause it.
    if(!Elem) return;
#ifdef PARSER_DEBUG
    kdDebug( 6035 ) << "popping block: " << getTagName(Elem->id) << "(" << Elem->id << ")" << endl;
#endif

#if SPEED_DEBUG < 1
    if((Elem->node != current)) {
        if (current->maintainsState()) {
            document->document()->registerMaintainsState(current);
            QString state(document->document()->nextState());
            if (!state.isNull()) current->restoreState(state);
        }
        current->closeRenderer();
    }
#endif

    removeForbidden(Elem->id, forbiddenTag);

    blockStack = Elem->next;
    // we only set inline to false, if the element we close is a block level element.
    // This helps getting cases as <p><b>bla</b> <b>bla</b> right.
    if (current->id() == ID_PRE)
        --inPre;

    m_inline = Elem->m_inline;

    if (current->id() == ID_FORM && form && haveMalformedTable)
        form->setMalformed(true);

    setCurrent( Elem->node );

    if (haveMalformedTable && (!current ||
                               current->id() == ID_TR ||
                               current->id() == ID_THEAD ||
                               current->id() == ID_TBODY ||
                               current->id() == ID_TFOOT ||
                               current->id() == ID_TABLE ) )
        haveMalformedTable = false;

    delete Elem;
}

void KHTMLParser::popInlineBlocks()
{
    while(blockStack && current->isInline() && current->id() != ID_FONT)
        popOneBlock();
}

void KHTMLParser::freeBlock()
{
    while (blockStack)
        popOneBlock();
    blockStack = 0;
}

void KHTMLParser::createHead()
{
    if(head || !doc()->firstChild())
        return;

    head = new HTMLHeadElementImpl(document);
    HTMLElementImpl *body = doc()->body();
    int exceptioncode = 0;
    doc()->firstChild()->insertBefore(head, body, exceptioncode);
    if ( exceptioncode ) {
#ifdef PARSER_DEBUG
        kdDebug( 6035 ) << "creation of head failed!!!!" << endl;
#endif
        delete head;
        head = 0;
    }
}

NodeImpl *KHTMLParser::handleIsindex( Token *t )
{
    NodeImpl *n;
    HTMLFormElementImpl *myform = form;
    if ( !myform ) {
        myform = new HTMLFormElementImpl(document, true);
        n = myform;
    } else
        n = new HTMLDivElementImpl( document, ID_DIV );
    NodeImpl *child = new HTMLHRElementImpl( document );
    n->addChild( child );
    DOMStringImpl* a = t->attrs ? t->attrs->getValue(ATTR_PROMPT) : 0;
    DOMString text = i18n("This is a searchable index. Enter search keywords: ");
    if (a)
        text = a;
    child = new TextImpl(document, text.implementation());
    n->addChild( child );
    child = new HTMLIsIndexElementImpl(document, myform);
    static_cast<ElementImpl *>(child)->setAttribute(ATTR_TYPE, "khtml_isindex");
    n->addChild( child );
    child = new HTMLHRElementImpl( document );
    n->addChild( child );

    return n;
}

void KHTMLParser::startBody()
{
    if(inBody) return;

    inBody = true;

    if( isindex ) {
        insertNode( isindex, true /* don't decend into this node */ );
        isindex = 0;
    }
}
