/**
 * css_renderstyledeclarationimpl.cpp
 *
 * Copyright (C)  2004  Zack Rusin <zack@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 * 02111-1307  USA
 */
#include "css_renderstyledeclarationimpl.h"

#include "rendering/render_style.h"
#include "rendering/render_object.h"

#include "cssproperties.h"

using namespace DOM;
using namespace khtml;


RenderStyleDeclarationImpl::RenderStyleDeclarationImpl( DOM::NodeImpl *node )
    : CSSStyleDeclarationImpl( 0 )
{
    m_node = node;
    m_renderer = m_node->renderer();
}

RenderStyleDeclarationImpl::~RenderStyleDeclarationImpl()
{
}

DOM::DOMString RenderStyleDeclarationImpl::cssText() const
{
    return DOMString();
}

void RenderStyleDeclarationImpl::setCssText( DOM::DOMString )
{
    // ### report that this sucka is read only
}

CSSValueImpl *RenderStyleDeclarationImpl::getPropertyCSSValue( int propertyID ) const
{
    CSSPrimitiveValueImpl *val = 0;

    switch( propertyID )
    {
    case CSS_PROP_BACKGROUND_COLOR:
        val = new CSSPrimitiveValueImpl( m_renderer->style()->backgroundColor().rgb() );
        break;
    case CSS_PROP_BACKGROUND_IMAGE:
        break;
    case CSS_PROP_BACKGROUND_REPEAT:
        break;
    case CSS_PROP_BACKGROUND_ATTACHMENT:
        break;
    case CSS_PROP_BACKGROUND_POSITION:
        break;
    case CSS_PROP_BACKGROUND_POSITION_X:
        break;
    case CSS_PROP_BACKGROUND_POSITION_Y:
        break;
    case CSS_PROP_BORDER_COLLAPSE:
        break;
    case CSS_PROP_BORDER_SPACING:
        break;
    case CSS_PROP__KHTML_BORDER_HORIZONTAL_SPACING:
        break;
    case CSS_PROP__KHTML_BORDER_VERTICAL_SPACING:
        break;
    case CSS_PROP_BORDER_TOP_COLOR:
        break;
    case CSS_PROP_BORDER_RIGHT_COLOR:
        break;
    case CSS_PROP_BORDER_BOTTOM_COLOR:
        break;
    case CSS_PROP_BORDER_LEFT_COLOR:
        break;
    case CSS_PROP_BORDER_TOP_STYLE:
        break;
    case CSS_PROP_BORDER_RIGHT_STYLE:
        break;
    case CSS_PROP_BORDER_BOTTOM_STYLE:
        break;
    case CSS_PROP_BORDER_LEFT_STYLE:
        break;
    case CSS_PROP_BORDER_TOP_WIDTH:
        val = new CSSPrimitiveValueImpl( m_renderer->borderTop(), CSSPrimitiveValue::CSS_PX );
        break;
    case CSS_PROP_BORDER_RIGHT_WIDTH:
        val = new CSSPrimitiveValueImpl( m_renderer->borderRight(), CSSPrimitiveValue::CSS_PX );
        break;
    case CSS_PROP_BORDER_BOTTOM_WIDTH:
        val = new CSSPrimitiveValueImpl( m_renderer->borderBottom(), CSSPrimitiveValue::CSS_PX );
        break;
    case CSS_PROP_BORDER_LEFT_WIDTH:
        val = new CSSPrimitiveValueImpl( m_renderer->borderLeft(), CSSPrimitiveValue::CSS_PX );
        break;
    case CSS_PROP_BOTTOM:
        break;
    case CSS_PROP_CAPTION_SIDE:
        break;
    case CSS_PROP_CLEAR:
        break;
    case CSS_PROP_CLIP:
        break;
    case CSS_PROP_COLOR:
        break;
    case CSS_PROP_CONTENT:
        break;
    case CSS_PROP_COUNTER_INCREMENT:
        break;
    case CSS_PROP_COUNTER_RESET:
        break;
    case CSS_PROP_CURSOR:
        break;
    case CSS_PROP_DIRECTION:
        break;
    case CSS_PROP_DISPLAY:
        break;
    case CSS_PROP_EMPTY_CELLS:
        break;
    case CSS_PROP_FLOAT:
        break;
    case CSS_PROP_FONT_FAMILY:
        break;
    case CSS_PROP_FONT_SIZE:
        break;
    case CSS_PROP_FONT_STYLE:
        break;
    case CSS_PROP_FONT_VARIANT:
        break;
    case CSS_PROP_FONT_WEIGHT:
        break;
    case CSS_PROP_HEIGHT:
        val = new CSSPrimitiveValueImpl( m_renderer->height(),
                                         CSSPrimitiveValue::CSS_PX );
        break;
    case CSS_PROP_LEFT:
        break;
    case CSS_PROP_LETTER_SPACING:
        break;
    case CSS_PROP_LINE_HEIGHT:
        break;
    case CSS_PROP_LIST_STYLE_IMAGE:
        break;
    case CSS_PROP_LIST_STYLE_POSITION:
        break;
    case CSS_PROP_LIST_STYLE_TYPE:
        break;
    case CSS_PROP_MARGIN_TOP:
        break;
    case CSS_PROP_MARGIN_RIGHT:
        break;
    case CSS_PROP_MARGIN_BOTTOM:
        break;
    case CSS_PROP_MARGIN_LEFT:
        break;
    case CSS_PROP_MAX_HEIGHT:
        val = new CSSPrimitiveValueImpl( m_renderer->availableHeight(),
                                         CSSPrimitiveValue::CSS_PX );
        break;
    case CSS_PROP_MAX_WIDTH:
        val = new CSSPrimitiveValueImpl( m_renderer->maxWidth(),
                                         CSSPrimitiveValue::CSS_PX );
        break;
    case CSS_PROP_MIN_HEIGHT:
        val = new CSSPrimitiveValueImpl( m_renderer->contentHeight(),
                                         CSSPrimitiveValue::CSS_PX );
        break;
    case CSS_PROP_MIN_WIDTH:
        val = new CSSPrimitiveValueImpl( m_renderer->minWidth(),
                                         CSSPrimitiveValue::CSS_PX );
        break;
    case CSS_PROP_ORPHANS:
        break;
    case CSS_PROP_OUTLINE_COLOR:
        break;
    case CSS_PROP_OUTLINE_STYLE:
        break;
    case CSS_PROP_OUTLINE_WIDTH:
        break;
    case CSS_PROP_OVERFLOW:
        break;
    case CSS_PROP_PADDING_TOP:
        val = new CSSPrimitiveValueImpl( m_renderer->paddingTop(),
                                         CSSPrimitiveValue::CSS_PX );
        break;
    case CSS_PROP_PADDING_RIGHT:
        val = new CSSPrimitiveValueImpl( m_renderer->paddingRight(),
                                         CSSPrimitiveValue::CSS_PX );
        break;
    case CSS_PROP_PADDING_BOTTOM:
        val = new CSSPrimitiveValueImpl( m_renderer->paddingBottom(),
                                         CSSPrimitiveValue::CSS_PX );
        break;
    case CSS_PROP_PADDING_LEFT:
        val = new CSSPrimitiveValueImpl( m_renderer->paddingLeft(),
                                         CSSPrimitiveValue::CSS_PX );
        break;
    case CSS_PROP_PAGE_BREAK_AFTER:
        break;
    case CSS_PROP_PAGE_BREAK_BEFORE:
        break;
    case CSS_PROP_PAGE_BREAK_INSIDE:
        break;
    case CSS_PROP_POSITION:
        break;
    case CSS_PROP_QUOTES:
        break;
    case CSS_PROP_RIGHT:
        break;
    case CSS_PROP_SIZE:
        break;
    case CSS_PROP_TABLE_LAYOUT:
        break;
    case CSS_PROP_TEXT_ALIGN:
        break;
    case CSS_PROP_TEXT_DECORATION:
        break;
    case CSS_PROP_TEXT_INDENT:
        break;
    case CSS_PROP_TEXT_TRANSFORM:
        break;
    case CSS_PROP_TOP:
        break;
    case CSS_PROP_UNICODE_BIDI:
        break;
    case CSS_PROP_VERTICAL_ALIGN:
        break;
    case CSS_PROP_VISIBILITY:
        break;
    case CSS_PROP_WHITE_SPACE:
        break;
    case CSS_PROP_WIDOWS:
        break;
    case CSS_PROP_WIDTH:
        val = new CSSPrimitiveValueImpl( m_renderer->width(),
                                         CSSPrimitiveValue::CSS_PX );
        break;
    case CSS_PROP_WORD_SPACING:
        break;
    case CSS_PROP_Z_INDEX:
        break;
    case CSS_PROP_BACKGROUND:
        break;
    case CSS_PROP_BORDER:
        break;
    case CSS_PROP_BORDER_COLOR:
        break;
    case CSS_PROP_BORDER_STYLE:
        break;
    case CSS_PROP_BORDER_TOP:
        val = new CSSPrimitiveValueImpl( m_renderer->borderTop(),
                                         CSSPrimitiveValue::CSS_PX );
        break;
    case CSS_PROP_BORDER_RIGHT:
        val = new CSSPrimitiveValueImpl( m_renderer->borderRight(),
                                         CSSPrimitiveValue::CSS_PX );
        break;
    case CSS_PROP_BORDER_BOTTOM:
        val = new CSSPrimitiveValueImpl( m_renderer->borderBottom(),
                                         CSSPrimitiveValue::CSS_PX );
        break;
    case CSS_PROP_BORDER_LEFT:
        val = new CSSPrimitiveValueImpl( m_renderer->borderLeft(),
                                         CSSPrimitiveValue::CSS_PX );
        break;
    case CSS_PROP_BORDER_WIDTH:
        break;
    case CSS_PROP_FONT:
        break;
    case CSS_PROP_LIST_STYLE:
        break;
    case CSS_PROP_MARGIN:
        break;
    case CSS_PROP_OUTLINE:
        break;
    case CSS_PROP_PADDING:
        break;
    case CSS_PROP_SCROLLBAR_BASE_COLOR:
        break;
    case CSS_PROP_SCROLLBAR_FACE_COLOR:
        break;
    case CSS_PROP_SCROLLBAR_SHADOW_COLOR:
        break;
    case CSS_PROP_SCROLLBAR_HIGHLIGHT_COLOR:
        break;
    case CSS_PROP_SCROLLBAR_3DLIGHT_COLOR:
        break;
    case CSS_PROP_SCROLLBAR_DARKSHADOW_COLOR:
        break;
    case CSS_PROP_SCROLLBAR_TRACK_COLOR:
        break;
    case CSS_PROP_SCROLLBAR_ARROW_COLOR:
        break;
    case CSS_PROP__KHTML_FLOW_MODE:
        break;
    case CSS_PROP__KHTML_USER_INPUT:
        break;
    case CSS_PROP__KHTML_TEXT_DECORATION_COLOR:
        break;
    default:
        Q_ASSERT( 0 );
        break;
    }
    return val;
}

DOMString RenderStyleDeclarationImpl::getPropertyValue( int propertyID ) const
{
    CSSProperty var = property( propertyID );
    DOMString str = var.cssText();
    return str;
}

bool RenderStyleDeclarationImpl::getPropertyPriority( int ) const
{
    return false;
}

DOM::DOMString RenderStyleDeclarationImpl::removeProperty( int, bool )
{
    // ### emit error since we're read-only
    return DOMString();
}

bool RenderStyleDeclarationImpl::setProperty ( int, const DOM::DOMString&, bool,
                                               bool )
{
    // ### emit error since we're read-only
    return false;
}

void RenderStyleDeclarationImpl::setProperty ( int, int, bool,
                                               bool )
{
    // ### emit error since we're read-only
}

void RenderStyleDeclarationImpl::setLengthProperty( int, const DOM::DOMString&, bool,
                                                    bool, bool )
{
    // ### emit error since we're read-only
}

void RenderStyleDeclarationImpl::setProperty( const DOMString& )
{
    // ### emit error since we're read-only
}

DOM::DOMString RenderStyleDeclarationImpl::item( unsigned long ) const
{
    // ###
    return DOMString();
}


CSSProperty RenderStyleDeclarationImpl::property( int id ) const
{
    CSSProperty prop;
    prop.m_id = id;
    prop.m_bImportant = false;
    prop.nonCSSHint = false;

    CSSValueImpl* v = getPropertyCSSValue( id );
    if ( !v )
        v = new CSSPrimitiveValueImpl;
    prop.setValue( v );
    return prop;
}
