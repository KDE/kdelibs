/**
 * This file is part of the DOM implementation for KDE.
 *
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * $Id$
 */
// -------------------------------------------------------------------------
//#define DEBUG
//#define DEBUG_LAYOUT
//#define PAR_DEBUG
//#define EVENT_DEBUG
//#define UNSUPPORTED_ATTR

#include "html/dtd.h"
#include "html/html_elementimpl.h"
#include "html/html_documentimpl.h"
#include "html/htmltokenizer.h"

#include "misc/htmlhashes.h"

#include "khtmlview.h"
#include "khtml_part.h"

#include "rendering/render_object.h"
#include "rendering/render_replaced.h"
#include "css/css_valueimpl.h"
#include "css/css_stylesheetimpl.h"
#include "css/cssproperties.h"
#include "css/cssvalues.h"
#include "xml/dom_textimpl.h"
#include "xml/dom2_eventsimpl.h"

#include <kapplication.h>
#include <kdebug.h>

using namespace DOM;
using namespace khtml;

HTMLElementImpl::~HTMLElementImpl()
{
}

void HTMLElementImpl::parseAttribute(AttributeImpl *attr)
{
    DOMString indexstring;
    switch( attr->id() )
    {
    case ATTR_ALIGN:
        if (attr->val()) {
            if ( strcasecmp(attr->value(), "middle" ) == 0 )
                addCSSProperty( CSS_PROP_TEXT_ALIGN, "center" );
            else
                addCSSProperty(CSS_PROP_TEXT_ALIGN, attr->value());
        }
        else
            removeCSSProperty(CSS_PROP_TEXT_ALIGN);
        break;
// the core attributes...
    case ATTR_ID:
        // unique id
        setHasID();
        setChanged(); // in case of a CSS selector on id
        break;
    case ATTR_CLASS:
    case ATTR_NAME:
        setChanged(); // in case of a CSS selector on class/name
        break;
    case ATTR_STYLE:
        // ### we need to remove old style info in case there was any!
        // ### the inline sheet ay contain more than 1 property!
        // stylesheet info
        setHasStyle();
        if(!m_styleDecls) createDecl();
        m_styleDecls->setProperty(attr->value());
        setChanged();
        break;
    case ATTR_TABINDEX:
        indexstring=getAttribute(ATTR_TABINDEX);
        if (indexstring.length())
            setTabIndex(indexstring.toInt());
        break;
// i18n attributes
    case ATTR_LANG:
        break;
    case ATTR_DIR:
        addCSSProperty(CSS_PROP_DIRECTION, attr->value());
        addCSSProperty(CSS_PROP_UNICODE_BIDI, CSS_VAL_EMBED);
        break;
// standard events
    case ATTR_ONCLICK:
	setHTMLEventListener(EventImpl::KHTML_ECMA_CLICK_EVENT,
	    getDocument()->createHTMLEventListener(attr->value().string()));
        break;
    case ATTR_ONDBLCLICK:
	setHTMLEventListener(EventImpl::KHTML_ECMA_DBLCLICK_EVENT,
	    getDocument()->createHTMLEventListener(attr->value().string()));
        break;
    case ATTR_ONMOUSEDOWN:
        setHTMLEventListener(EventImpl::MOUSEDOWN_EVENT,
	    getDocument()->createHTMLEventListener(attr->value().string()));
        break;
    case ATTR_ONMOUSEMOVE:
        setHTMLEventListener(EventImpl::MOUSEMOVE_EVENT,
	    getDocument()->createHTMLEventListener(attr->value().string()));
        break;
    case ATTR_ONMOUSEOUT:
        setHTMLEventListener(EventImpl::MOUSEOUT_EVENT,
	    getDocument()->createHTMLEventListener(attr->value().string()));
        break;
    case ATTR_ONMOUSEOVER:
        setHTMLEventListener(EventImpl::MOUSEOVER_EVENT,
	    getDocument()->createHTMLEventListener(attr->value().string()));
        break;
    case ATTR_ONMOUSEUP:
        setHTMLEventListener(EventImpl::MOUSEUP_EVENT,
	    getDocument()->createHTMLEventListener(attr->value().string()));
        break;
    case ATTR_ONFOCUS:
        setHTMLEventListener(EventImpl::DOMFOCUSIN_EVENT,
	    getDocument()->createHTMLEventListener(attr->value().string()));
        break;
    case ATTR_ONKEYDOWN:
        setHTMLEventListener(EventImpl::KHTML_KEYDOWN_EVENT,
	    getDocument()->createHTMLEventListener(attr->value().string()));
	break;
    case ATTR_ONKEYPRESS:
        setHTMLEventListener(EventImpl::KHTML_KEYPRESS_EVENT,
	    getDocument()->createHTMLEventListener(attr->value().string()));
	break;
    case ATTR_ONKEYUP:
        setHTMLEventListener(EventImpl::KHTML_KEYUP_EVENT,
	    getDocument()->createHTMLEventListener(attr->value().string()));
        break;
// other misc attributes
    default:
#ifdef UNSUPPORTED_ATTR
	kdDebug(6030) << "UATTR: <" << this->nodeName().string() << "> ["
		      << attr->name().string() << "]=[" << attr->value().string() << "]" << endl;
#endif
        break;
    }
}

void HTMLElementImpl::recalcStyle( StyleChange ch )
{
    ElementImpl::recalcStyle( ch );

    if (m_render /*&& changed*/)
        m_render->updateFromElement();
}

void HTMLElementImpl::addCSSProperty(int id, const DOMString &value)
{
    if(!m_styleDecls) createDecl();
    m_styleDecls->setProperty(id, value, false, true);
    setChanged();
}

void HTMLElementImpl::addCSSProperty(int id, int value)
{
    if(!m_styleDecls) createDecl();
    m_styleDecls->setProperty(id, value, false, true);
    setChanged();
}

void HTMLElementImpl::addCSSLength(int id, const DOMString &value, bool numOnly, bool multiLength)
{
    if(!m_styleDecls) createDecl();

    // strip attribute garbage..
    DOMStringImpl* v = value.implementation();
    if ( v ) {
        unsigned int l = 0;

        while ( l < v->l && v->s[l].unicode() <= ' ') l++;

        for ( ;l < v->l; l++ ) {
            char cc = v->s[l].latin1();
            if ( cc > '9' || ( cc < '0' && ( numOnly || (cc != '%' && cc != '.' &&
							 !( multiLength && cc == '*') ) ) ) )
                break;
        }
        if ( l != v->l ) {
            m_styleDecls->setLengthProperty( id, DOMString( v->s, l ), false, true, multiLength );
            setChanged();
            return;
        }
    }

    m_styleDecls->setLengthProperty(id, value, false, true, multiLength);
    setChanged();
}

void HTMLElementImpl::removeCSSProperty(int id)
{
    if(!m_styleDecls)
        return;
    m_styleDecls->setParent(getDocument()->elementSheet());
    m_styleDecls->removeProperty(id);
    setChanged();
}

DOMString HTMLElementImpl::innerHTML() const
{
    return toHTML();
}

DOMString HTMLElementImpl::innerText() const
{
    DOMString text;

    const NodeImpl *n = this;
    // find the next text/image after the anchor, to get a position
    while(n) {
        if(n->firstChild())
            n = n->firstChild();
        else if(n->nextSibling())
            n = n->nextSibling();
        else {
            NodeImpl *next = 0;
            while(!next) {
                n = n->parentNode();
                if(!n || n == (NodeImpl *)this ) goto end;
                next = n->nextSibling();
            }
            n = next;
        }
        if(n->isTextNode() ) {
            text += static_cast<const TextImpl *>(n)->data();
        }
    }
 end:
    return text;
}

bool HTMLElementImpl::setInnerHTML( const DOMString &html )
{
    // the following is in accordance with the definition as used by IE
    if( endTag[id()] == FORBIDDEN )
        return false;
    // IE disallows innerHTML on inline elements. I don't see why we should have this restriction, as our
    // dhtml engine can cope with it. Lars
    //if ( isInline() ) return false;
    switch( id() ) {
        case ID_COL:
        case ID_COLGROUP:
        case ID_FRAMESET:
        case ID_HEAD:
        case ID_HTML:
        case ID_STYLE:
        case ID_TABLE:
        case ID_TBODY:
        case ID_TFOOT:
        case ID_THEAD:
        case ID_TITLE:
        case ID_TR:
            return false;
        default:
            break;
    }
    if ( !getDocument()->isHTMLDocument() )
        return false;

    DocumentFragmentImpl *fragment = new DocumentFragmentImpl( docPtr() );
    HTMLTokenizer *tok = new HTMLTokenizer( docPtr(), fragment );
    tok->begin();
    tok->write( html.string(), true );
    tok->end();
    delete tok;

    removeChildren();
    int ec = 0;
    appendChild( fragment, ec );
    delete fragment;
    return !ec;
}

bool HTMLElementImpl::setInnerText( const DOMString &text )
{
    // following the IE specs.
    if( endTag[id()] == FORBIDDEN )
        return false;
    // IE disallows innerHTML on inline elements. I don't see why we should have this restriction, as our
    // dhtml engine can cope with it. Lars
    //if ( isInline() ) return false;
    switch( id() ) {
        case ID_COL:
        case ID_COLGROUP:
        case ID_FRAMESET:
        case ID_HEAD:
        case ID_HTML:
        case ID_TABLE:
        case ID_TBODY:
        case ID_TFOOT:
        case ID_THEAD:
        case ID_TR:
            return false;
        default:
            break;
    }

    removeChildren();

    TextImpl *t = new TextImpl( docPtr(), text.implementation() );
    int ec = 0;
    appendChild( t, ec );
    if ( !ec )
        return true;
    return false;
}

DOMString HTMLElementImpl::namespaceURI() const
{
    // For HTML documents, we treat HTML elements as having no namespace. But for XML documents
    // the elements have the namespace defined in the XHTML spec
    if (getDocument()->isHTMLDocument())
        return DOMString();
    else
        return XHTML_NAMESPACE;
}

void HTMLElementImpl::addHTMLAlignment( DOMString alignment )
{
    //qDebug("alignment is %s", alignment.string().latin1() );
    // vertical alignment with respect to the current baseline of the text
    // right or left means floating images
    int propfloat = -1;
    int propvalign = -1;
    if ( strcasecmp( alignment, "absmiddle" ) == 0 ) {
        propvalign = CSS_VAL_MIDDLE;
    } else if ( strcasecmp( alignment, "absbottom" ) == 0 ) {
        propvalign = CSS_VAL_BOTTOM;
    } else if ( strcasecmp( alignment, "left" ) == 0 ) {
	propfloat = CSS_VAL_LEFT;
	propvalign = CSS_VAL_TOP;
    } else if ( strcasecmp( alignment, "right" ) == 0 ) {
	propfloat = CSS_VAL_RIGHT;
	propvalign = CSS_VAL_TOP;
    } else if ( strcasecmp( alignment, "top" ) == 0 ) {
	propvalign = CSS_VAL_TOP;
    } else if ( strcasecmp( alignment, "middle" ) == 0 ) {
	propvalign = CSS_VAL__KONQ_BASELINE_MIDDLE;
    } else if ( strcasecmp( alignment, "center" ) == 0 ) {
	propvalign = CSS_VAL_MIDDLE;
    } else if ( strcasecmp( alignment, "bottom" ) == 0 ) {
	propvalign = CSS_VAL_BASELINE;
    } else if ( strcasecmp ( alignment, "texttop") == 0 ) {
	propvalign = CSS_VAL_TEXT_TOP;
    }

    if ( propfloat != -1 )
	addCSSProperty( CSS_PROP_FLOAT, propfloat );
    if ( propvalign != -1 )
	addCSSProperty( CSS_PROP_VERTICAL_ALIGN, propvalign );
}

bool HTMLElementImpl::isURLAllowed(const QString& url) const
{
    KHTMLView *w = getDocument()->view();

    KURL newURL(getDocument()->completeURL(url));
    newURL.setRef(QString::null);

    // Prohibit non-file URLs if we are asked to.
    if (!w || w->part()->onlyLocalReferences() && newURL.protocol() != "file")
        return false;

    // do we allow this suburl ?
    if ( !kapp || !kapp->kapp->authorizeURLAction("redirect", w->part()->url(), newURL) )
        return false;

    // We allow one level of self-reference because some sites depend on that.
    // But we don't allow more than one.
    bool foundSelfReference = false;
    for (KHTMLPart *part = w->part(); part; part = part->parentPart()) {
        KURL partURL = part->url();
        partURL.setRef(QString::null);
        if (partURL == newURL) {
            if (foundSelfReference)
                return false;
            foundSelfReference = true;
        }
    }

    return true;
}



// -------------------------------------------------------------------------
HTMLGenericElementImpl::HTMLGenericElementImpl(DocumentPtr *doc, ushort i)
    : HTMLElementImpl(doc)
{
    _id = i;
}

HTMLGenericElementImpl::~HTMLGenericElementImpl()
{
}

