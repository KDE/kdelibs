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
#include "html_blockimpl.h"
#include "html_documentimpl.h"
#include "css/cssstyleselector.h"

#include "css/cssproperties.h"
#include "css/cssvalues.h"
#include "misc/htmlhashes.h"

#include <kdebug.h>

using namespace khtml;
using namespace DOM;

NodeImpl::Id HTMLDivElementImpl::id() const
{
    return ID_DIV;
}

void HTMLDivElementImpl::parseAttribute(AttributeImpl *attr)
{
    switch(attr->id())
    {
    case ATTR_ALIGN:
    {
        DOMString v = attr->value();
        if ( strcasecmp( attr->value(), "center" ) == 0 )
            v = "-konq-center";
        addCSSProperty(CSS_PROP_TEXT_ALIGN, v);
        break;
    }
    default:
        HTMLElementImpl::parseAttribute(attr);
    }
}

// -------------------------------------------------------------------------

NodeImpl::Id HTMLHRElementImpl::id() const
{
    return ID_HR;
}

void HTMLHRElementImpl::parseAttribute(AttributeImpl *attr)
{
    switch( attr->id() )
    {
    case ATTR_ALIGN:
        if ( strcasecmp( attr->value(), "left") != 0) // _not_ equal
            addCSSProperty(CSS_PROP_MARGIN_LEFT, CSS_VAL_AUTO);
        else
            addCSSProperty(CSS_PROP_MARGIN_LEFT, "1px");
        if( strcasecmp( attr->value(), "right") != 0)
            addCSSProperty(CSS_PROP_MARGIN_RIGHT, CSS_VAL_AUTO);
        else
            addCSSProperty(CSS_PROP_MARGIN_RIGHT, "1px");
        break;
    case ATTR_WIDTH:
    {
        if(!attr->val()) break;
        // cheap hack to cause linebreaks
        // khtmltests/html/strange_hr.html
        bool ok;
        int v = attr->val()->toInt(&ok);
        if(ok && !v)
            addCSSLength(CSS_PROP_WIDTH, "1");
        else
            addCSSLength(CSS_PROP_WIDTH, attr->value());
    }
    break;
    default:
        HTMLElementImpl::parseAttribute(attr);
    }
}

// ### make sure we undo what we did during detach
void HTMLHRElementImpl::attach()
{
    if (attributes(true /* readonly */)) {
        // there are some attributes, lets check
        DOMString color = getAttribute(ATTR_COLOR);
        DOMStringImpl* si = getAttribute(ATTR_SIZE).implementation();
        int _s =  si ? si->toInt() : -1;
        DOMString n("1");
        if (!color.isNull()) {
            addCSSProperty(CSS_PROP_BORDER_TOP_STYLE, CSS_VAL_SOLID);
            addCSSProperty(CSS_PROP_BORDER_RIGHT_STYLE, CSS_VAL_SOLID);
            addCSSProperty(CSS_PROP_BORDER_BOTTOM_STYLE, CSS_VAL_SOLID);
            addCSSProperty(CSS_PROP_BORDER_LEFT_STYLE, CSS_VAL_SOLID);
            addCSSProperty(CSS_PROP_BORDER_TOP_WIDTH, DOMString("0"));
            addCSSLength(CSS_PROP_BORDER_BOTTOM_WIDTH, DOMString(si));
            addCSSProperty(CSS_PROP_BORDER_COLOR, color);
        }
        else {
            if (_s > 1 && getAttribute(ATTR_NOSHADE).isNull()) {
                addCSSProperty(CSS_PROP_BORDER_BOTTOM_WIDTH, n);
                addCSSProperty(CSS_PROP_BORDER_TOP_WIDTH, n);
                addCSSProperty(CSS_PROP_BORDER_LEFT_WIDTH, n);
                addCSSProperty(CSS_PROP_BORDER_RIGHT_WIDTH, n);
                addCSSLength(CSS_PROP_HEIGHT, DOMString(QString::number(_s-2)));
            }
            else if (_s >= 0) {
                addCSSProperty(CSS_PROP_BORDER_TOP_WIDTH, DOMString(QString::number(_s)));
                addCSSProperty(CSS_PROP_BORDER_BOTTOM_WIDTH, DOMString("0"));
            }
        }
        if (_s == 0)
            addCSSProperty(CSS_PROP_MARGIN_BOTTOM, n);
    }

    HTMLElementImpl::attach();
}

// -------------------------------------------------------------------------

long HTMLPreElementImpl::width() const
{
    // ###
    return 0;
}

void HTMLPreElementImpl::setWidth( long /*w*/ )
{
    // ###
}


