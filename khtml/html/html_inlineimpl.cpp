/**
 * This file is part of the DOM implementation for KDE.
 *
 * Copyright (C) 1999-2003 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 *           (C) 2000 Simon Hausmann <hausmann@kde.org>
 *           (C) 2001-2003 Dirk Mueller (mueller@kde.org)
 *           (C) 2002 Apple Computer, Inc.
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
 */
// -------------------------------------------------------------------------

#include "html/html_inlineimpl.h"
#include "html/html_imageimpl.h"
#include "html/html_documentimpl.h"

#include "misc/htmlhashes.h"
#include "khtmlview.h"
#include "khtml_part.h"
#include "css/csshelper.h"
#include "css/cssproperties.h"
#include "css/cssvalues.h"
#include "css/cssstyleselector.h"
#include "xml/dom2_eventsimpl.h"
#include "rendering/render_br.h"
#include "rendering/render_image.h"

#include <kdebug.h>

using namespace khtml;
using namespace DOM;

NodeImpl::Id HTMLAnchorElementImpl::id() const
{
    return ID_A;
}

void HTMLAnchorElementImpl::defaultEventHandler(EventImpl *evt)
{
    bool keydown = evt->id() == EventImpl::KEYDOWN_EVENT;

    // React on clicks and on keypresses.
    // Don't make this KEYUP_EVENT again, it makes khtml follow links
    // it shouldn't, when pressing Enter in the combo.
    if ( ( (evt->id() == EventImpl::CLICK_EVENT && !static_cast<MouseEventImpl*>(evt)->isDoubleClick()) ||
         ( keydown && m_focused)) && m_hasAnchor) {

        MouseEventImpl *e = 0;
        if ( evt->id() == EventImpl::CLICK_EVENT )
            e = static_cast<MouseEventImpl*>( evt );

        TextEventImpl *k = 0;
        if (keydown)
            k = static_cast<TextEventImpl *>( evt );

        QString utarget;
        QString url;
        if ( e && e->button() == 2 ) {
	    HTMLElementImpl::defaultEventHandler(evt);
	    return;
        }

        if ( k ) {
            if (k->virtKeyVal() != TextEventImpl::DOM_VK_ENTER) {
                if (k->qKeyEvent)
                    k->qKeyEvent->ignore();
                HTMLElementImpl::defaultEventHandler(evt);
                return;
            }
            if (k->qKeyEvent) k->qKeyEvent->accept();
        }

        url = khtml::parseURL(getAttribute(ATTR_HREF)).string();

        utarget = getAttribute(ATTR_TARGET).string();

        if ( e && e->button() == 1 )
            utarget = "_blank";

        if ( evt->target()->id() == ID_IMG ) {
            HTMLImageElementImpl* img = static_cast<HTMLImageElementImpl*>( evt->target() );
            if ( img && img->isServerMap() )
            {
                khtml::RenderImage *r = static_cast<khtml::RenderImage *>(img->renderer());
                if(r && e)
                {
                    KHTMLView* v = getDocument()->view();
                    int x = e->clientX();
                    int y = e->clientY();
                    int absx = 0;
                    int absy = 0;
                    if ( v ) {
                        x += v->contentsX();
                        y += v->contentsY();
                    }
                    r->absolutePosition(absx, absy);
                    url += QString("?%1,%2").arg( x - absx ).arg( y - absy );
                }
                else {
                    evt->setDefaultHandled();
		    HTMLElementImpl::defaultEventHandler(evt);
		    return;
                }
            }
        }
        if ( !evt->defaultPrevented() ) {
            int state = 0;
            int button = 0;

            if ( e ) {
                if ( e->ctrlKey() )
                    state |= Qt::ControlButton;
                if ( e->shiftKey() )
                    state |= Qt::ShiftButton;
                if ( e->altKey() )
                    state |= Qt::AltButton;
                if ( e->metaKey() )
                    state |= Qt::MetaButton;

                if ( e->button() == 0 )
                    button = Qt::LeftButton;
                else if ( e->button() == 1 )
                    button = Qt::MidButton;
                else if ( e->button() == 2 )
                    button = Qt::RightButton;
            }
	    else if ( k )
	    {
	      if ( k->checkModifier(Qt::ShiftButton) )
                state |= Qt::ShiftButton;
	      if ( k->checkModifier(Qt::AltButton) )
                state |= Qt::AltButton;
	      if ( k->checkModifier(Qt::ControlButton) )
                state |= Qt::ControlButton;
	    }

	    // ### also check if focused node is editable if not in designmode,
	    // and prevent link loading then (LS)
	    if (getDocument()->view() && !getDocument()->designMode())
		getDocument()->view()->part()->
		    urlSelected( url, button, state, utarget );
        }
        evt->setDefaultHandled();
    }
    HTMLElementImpl::defaultEventHandler(evt);
}


void HTMLAnchorElementImpl::click()
{
    QMouseEvent me(QEvent::MouseButtonRelease, QPoint(0,0),Qt::LeftButton, 0);
    dispatchMouseEvent(&me,EventImpl::CLICK_EVENT, 1);
}

void HTMLAnchorElementImpl::parseAttribute(AttributeImpl *attr)
{
    switch(attr->id())
    {
    case ATTR_HREF:
        m_hasAnchor = attr->val() != 0;
        break;
    case ATTR_TARGET:
        m_hasTarget = attr->val() != 0;
        break;
    case ATTR_NAME:
    case ATTR_TITLE:
    case ATTR_REL:
	break;
    case ATTR_ACCESSKEY:
        break;
    default:
        HTMLElementImpl::parseAttribute(attr);
    }
}

// -------------------------------------------------------------------------

NodeImpl::Id HTMLBRElementImpl::id() const
{
    return ID_BR;
}

void HTMLBRElementImpl::parseAttribute(AttributeImpl *attr)
{
    switch(attr->id())
    {
    case ATTR_CLEAR:
    {
        DOMString str = attr->value().lower();
        if( str.isEmpty() ) str = "none";
        else if( strcmp (str,"all")==0 ) str = "both";
        addCSSProperty(CSS_PROP_CLEAR, str);
        break;
    }
    default:
        HTMLElementImpl::parseAttribute(attr);
    }
}

void HTMLBRElementImpl::attach()
{
    assert(!attached());
    assert(!m_render);
    assert(parentNode());

    if (parentNode()->renderer()) {
        RenderStyle* style = getDocument()->styleSelector()->styleForElement( this );
        style->ref();
        if( style->display() != NONE ) {
          m_render = new (getDocument()->renderArena()) RenderBR(this);
          m_render->setStyle(style);
          parentNode()->renderer()->addChild(m_render, nextRenderer());
        }
        style->deref();
    }
    NodeImpl::attach();
}

// -------------------------------------------------------------------------

NodeImpl::Id HTMLFontElementImpl::id() const
{
    return ID_FONT;
}

void HTMLFontElementImpl::parseAttribute(AttributeImpl *attr)
{
    switch(attr->id())
    {
    case ATTR_SIZE:
    {
        DOMStringImpl* v = attr->val();
        if(v) {
            const QChar* s = v->s;
            int num = v->toInt();
            int len = v->l;
            while( len && s->isSpace() )
              len--,s++;
            if ( len && *s == '+' )
                num += 3;
            int size;
            switch (num)
            {
            case -2:
            case  1: size = CSS_VAL_XX_SMALL;  break;
            case -1:
            case  2: size = CSS_VAL_SMALL;    break;
            case  0: // treat 0 the same as 3, because people
                     // expect it to be between -1 and +1
            case  3: size = CSS_VAL_MEDIUM;   break;
            case  4: size = CSS_VAL_LARGE;    break;
            case  5: size = CSS_VAL_X_LARGE;  break;
            case  6: size = CSS_VAL_XX_LARGE; break;
            default:
                if (num > 6)
                    size = CSS_VAL__KHTML_XXX_LARGE;
                else
                    size = CSS_VAL_XX_SMALL;
            }
            addCSSProperty(CSS_PROP_FONT_SIZE, size);
        }
        break;
    }
    case ATTR_COLOR:
        addHTMLColor(CSS_PROP_COLOR, attr->value());
        // HTML4 compatibility hack
        addHTMLColor(CSS_PROP__KHTML_TEXT_DECORATION_COLOR, attr->value());
        break;
    case ATTR_FACE:
        addCSSProperty(CSS_PROP_FONT_FAMILY, attr->value());
        break;
    default:
        HTMLElementImpl::parseAttribute(attr);
    }
}


