/**
 * css_renderstyledeclarationimpl.cpp
 *
 * Copyright (C)  2004  Zack Rusin <zack@kde.org>
 * Copyright (C)  2004  Apple Computer, Inc.
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301  USA
 */
#include "css_renderstyledeclarationimpl.h"

#include "rendering/render_style.h"
#include "rendering/render_object.h"

#include "cssproperties.h"

using namespace DOM;
using namespace khtml;


static CSSValueImpl *valueForLength(const Length &length, int max)
{
    if (length.isPercent()) {
        return new CSSPrimitiveValueImpl(length._length, CSSPrimitiveValue::CSS_PERCENTAGE);
    }
    else {
        return new CSSPrimitiveValueImpl(length.minWidth(max), CSSPrimitiveValue::CSS_PX);
    }
}

static DOMString stringForBorderStyle(EBorderStyle style)
{
    switch (style) {
    case khtml::BNATIVE:
        return "-khtml-native";
    case khtml::BNONE:
        return "none";
    case khtml::BHIDDEN:
        return "hidden";
    case khtml::INSET:
        return "inset";
    case khtml::GROOVE:
        return "groove";
    case khtml::RIDGE:
        return "ridge";
    case khtml::OUTSET:
        return "outset";
    case khtml::DOTTED:
        return "dotted";
    case khtml::DASHED:
        return "dashed";
    case khtml::SOLID:
        return "solid";
    case khtml::DOUBLE:
        return "double";
    }
    Q_ASSERT( 0 );
    return "";
}

static DOMString stringForTextAlign(ETextAlign align)
{
    switch (align) {
    case khtml::TAAUTO:
        return "auto";
    case khtml::LEFT:
        return "left";
    case khtml::RIGHT:
        return "right";
    case khtml::CENTER:
        return "center";
    case khtml::JUSTIFY:
        return "justify";
    case khtml::KHTML_LEFT:
        return "-khtml-left";
    case khtml::KHTML_RIGHT:
        return "-khtml-right";
    case khtml::KHTML_CENTER:
        return "-khtml-center";
    }
    Q_ASSERT( 0 );
    return "";
}

DOMString khtml::stringForListStyleType(EListStyleType type)
{
    switch (type) {
        case khtml::LDISC:
            return "disc";
        case khtml::LCIRCLE:
            return "circle";
        case khtml::LSQUARE:
            return "square";
        case khtml::LBOX:
            return "box";
        case khtml::LDIAMOND:
            return "-khtml-diamond";
        case khtml::LDECIMAL:
            return "decimal";
        case khtml::DECIMAL_LEADING_ZERO:
            return "decimal-leading-zero";
        case khtml::ARABIC_INDIC:
            return "-khtml-arabic-indic";
        case khtml::LAO:
            return "-khtml-lao";
        case khtml::PERSIAN:
            return "-khtml-persian";
        case khtml::URDU:
            return "-khtml-urdu";
        case khtml::THAI:
            return "-khtml-thai";
        case khtml::TIBETAN:
            return "-khtml-tibetan";
        case khtml::LOWER_ROMAN:
            return "lower-roman";
        case khtml::UPPER_ROMAN:
            return "upper-roman";
        case khtml::HEBREW:
            return "hebrew";
        case khtml::ARMENIAN:
            return "armenian";
        case khtml::GEORGIAN:
            return "georgian";
        case khtml::CJK_IDEOGRAPHIC:
            return "cjk-ideographic";
        case khtml::JAPANESE_FORMAL:
            return "-khtml-japanese-formal";
        case khtml::JAPANESE_INFORMAL:
            return "-khtml-japanese-informal";
        case khtml::SIMP_CHINESE_FORMAL:
            return "-khtml-simp-chinese-formal";
        case khtml::SIMP_CHINESE_INFORMAL:
            return "-khtml-simp-chinese-informal";
        case khtml::TRAD_CHINESE_FORMAL:
            return "-khtml-trad-chinese-formal";
        case khtml::TRAD_CHINESE_INFORMAL:
            return "-khtml-trad-chinese-informal";
        case khtml::LOWER_GREEK:
            return "lower-greek";
        case khtml::UPPER_GREEK:
            return "-khtml-upper-greek";
        case khtml::LOWER_ALPHA:
            return "lower-alpha";
        case khtml::UPPER_ALPHA:
            return "upper-alpha";
        case khtml::LOWER_LATIN:
            return "lower-latin";
        case khtml::UPPER_LATIN:
            return "upper-latin";
        case khtml::HIRAGANA:
            return "hiragana";
        case khtml::KATAKANA:
            return "katakana";
        case khtml::HIRAGANA_IROHA:
            return "hiragana-iroha";
        case khtml::KATAKANA_IROHA:
            return "katakana_iroha";
        case khtml::LNONE:
            return "none";
    }
    Q_ASSERT( 0 );
    return "";
}

RenderStyleDeclarationImpl::RenderStyleDeclarationImpl( DOM::NodeImpl *node )
    : CSSStyleDeclarationImpl(0), m_node(node)
{
    kDebug() << "Render Style Declaration created" << endl;
}

RenderStyleDeclarationImpl::~RenderStyleDeclarationImpl()
{
    kDebug() << "Render Style Declaration destroyed" << endl;
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
    NodeImpl *node = m_node.get();
    if (!node)
        return 0;

    // Make sure our layout is up to date before we allow a query on these attributes.
    DocumentImpl* docimpl = node->getDocument();
    if (docimpl) {
        docimpl->updateLayout();
    }

    RenderObject *renderer = m_node->renderer();
    if (!renderer)
        return 0;
    RenderStyle *style = renderer->style();
    if (!style)
        return 0;

    switch(propertyID)
    {
    case CSS_PROP_BACKGROUND_COLOR:
        return new CSSPrimitiveValueImpl(style->backgroundColor().rgb());
    case CSS_PROP_BACKGROUND_IMAGE:
        if (style->backgroundImage())
            return new CSSPrimitiveValueImpl(style->backgroundImage()->url(),
                                             CSSPrimitiveValue::CSS_URI);
        return 0;
    case CSS_PROP_BACKGROUND_REPEAT:
        switch (style->backgroundRepeat()) {
        case khtml::REPEAT:
            return new CSSPrimitiveValueImpl("repeat", CSSPrimitiveValue::CSS_STRING);
        case khtml::REPEAT_X:
            return new CSSPrimitiveValueImpl("repeat-x", CSSPrimitiveValue::CSS_STRING);
        case khtml::REPEAT_Y:
            return new CSSPrimitiveValueImpl("repeat-y", CSSPrimitiveValue::CSS_STRING);
        case khtml::NO_REPEAT:
            return new CSSPrimitiveValueImpl("no-repeat", CSSPrimitiveValue::CSS_STRING);
        default:
            Q_ASSERT( 0 );
        }
    case CSS_PROP_BACKGROUND_ATTACHMENT:
        if (style->backgroundAttachment())
            return new CSSPrimitiveValueImpl("scroll", CSSPrimitiveValue::CSS_STRING);
        else
            return new CSSPrimitiveValueImpl("fixed", CSSPrimitiveValue::CSS_STRING);
    case CSS_PROP_BACKGROUND_POSITION:
    {
        DOMString string;
        Length length(style->backgroundXPosition());
        if (length.isPercent())
            string = QString::number(length._length) + "%";
        else
            string = QString::number(length.minWidth(renderer->contentWidth()));
        string += " ";
        length = style->backgroundYPosition();
        if (length.isPercent())
            string += QString::number(length._length) + "%";
        else
            string += QString::number(length.minWidth(renderer->contentWidth()));
        return new CSSPrimitiveValueImpl(string, CSSPrimitiveValue::CSS_STRING);
    }
    case CSS_PROP_BACKGROUND_POSITION_X:
        return valueForLength(style->backgroundXPosition(), renderer->contentWidth());
    case CSS_PROP_BACKGROUND_POSITION_Y:
        return valueForLength(style->backgroundYPosition(), renderer->contentHeight());
    case CSS_PROP_BORDER_COLLAPSE:
        if (style->borderCollapse())
            return new CSSPrimitiveValueImpl("collapse", CSSPrimitiveValue::CSS_STRING);
        else
            return new CSSPrimitiveValueImpl("separate", CSSPrimitiveValue::CSS_STRING);
    case CSS_PROP_BORDER_SPACING:
    {
        QString string(QString::number(style->borderHorizontalSpacing()) +
                       "px " +
                       QString::number(style->borderVerticalSpacing()) +
                       "px");
        return new CSSPrimitiveValueImpl(DOMString(string), CSSPrimitiveValue::CSS_STRING);
    }
    case CSS_PROP__KHTML_BORDER_HORIZONTAL_SPACING:
        return new CSSPrimitiveValueImpl(style->borderHorizontalSpacing(),
                                         CSSPrimitiveValue::CSS_PX);
    case CSS_PROP__KHTML_BORDER_VERTICAL_SPACING:
        return new CSSPrimitiveValueImpl(style->borderVerticalSpacing(),
                                         CSSPrimitiveValue::CSS_PX);
    case CSS_PROP_BORDER_TOP_COLOR:
        return new CSSPrimitiveValueImpl(style->borderLeftColor().rgb());
    case CSS_PROP_BORDER_RIGHT_COLOR:
        return new CSSPrimitiveValueImpl(style->borderRightColor().rgb());
    case CSS_PROP_BORDER_BOTTOM_COLOR:
        return new CSSPrimitiveValueImpl(style->borderBottomColor().rgb());
    case CSS_PROP_BORDER_LEFT_COLOR:
        return new CSSPrimitiveValueImpl(style->borderLeftColor().rgb());
    case CSS_PROP_BORDER_TOP_STYLE:
        return new CSSPrimitiveValueImpl(stringForBorderStyle(style->borderTopStyle()),
                                         CSSPrimitiveValue::CSS_STRING);
    case CSS_PROP_BORDER_RIGHT_STYLE:
        return new CSSPrimitiveValueImpl(stringForBorderStyle(style->borderRightStyle()),
                                         CSSPrimitiveValue::CSS_STRING);
    case CSS_PROP_BORDER_BOTTOM_STYLE:
        return new CSSPrimitiveValueImpl(stringForBorderStyle(style->borderBottomStyle()),
                                         CSSPrimitiveValue::CSS_STRING);
    case CSS_PROP_BORDER_LEFT_STYLE:
        return new CSSPrimitiveValueImpl(stringForBorderStyle(style->borderLeftStyle()),
                                         CSSPrimitiveValue::CSS_STRING);
    case CSS_PROP_BORDER_TOP_WIDTH:
        return new CSSPrimitiveValueImpl( style->borderTopWidth(), CSSPrimitiveValue::CSS_PX );
    case CSS_PROP_BORDER_RIGHT_WIDTH:
        return new CSSPrimitiveValueImpl( style->borderRightWidth(), CSSPrimitiveValue::CSS_PX );
    case CSS_PROP_BORDER_BOTTOM_WIDTH:
        return new CSSPrimitiveValueImpl( style->borderBottomWidth(), CSSPrimitiveValue::CSS_PX );
    case CSS_PROP_BORDER_LEFT_WIDTH:
        return new CSSPrimitiveValueImpl( style->borderLeftWidth(), CSSPrimitiveValue::CSS_PX );
    case CSS_PROP_BOTTOM:
        break;
    case CSS_PROP_CAPTION_SIDE:
        break;
    case CSS_PROP_CLEAR:
        break;
    case CSS_PROP_CLIP:
        break;
    case CSS_PROP_COLOR:
        return new CSSPrimitiveValueImpl(style->color().rgb());
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
        switch (style->display()) {
        case khtml::INLINE:
            return new CSSPrimitiveValueImpl("inline", CSSPrimitiveValue::CSS_STRING);
        case khtml::BLOCK:
            return new CSSPrimitiveValueImpl("block", CSSPrimitiveValue::CSS_STRING);
        case khtml::LIST_ITEM:
            return new CSSPrimitiveValueImpl("list-item", CSSPrimitiveValue::CSS_STRING);
        case khtml::RUN_IN:
            return new CSSPrimitiveValueImpl("run-in", CSSPrimitiveValue::CSS_STRING);
        case khtml::COMPACT:
            return new CSSPrimitiveValueImpl("compact", CSSPrimitiveValue::CSS_STRING);
        case khtml::INLINE_BLOCK:
            return new CSSPrimitiveValueImpl("inline-block", CSSPrimitiveValue::CSS_STRING);
        case khtml::TABLE:
            return new CSSPrimitiveValueImpl("table", CSSPrimitiveValue::CSS_STRING);
        case khtml::INLINE_TABLE:
            return new CSSPrimitiveValueImpl("inline-table", CSSPrimitiveValue::CSS_STRING);
        case khtml::TABLE_ROW_GROUP:
            return new CSSPrimitiveValueImpl("table-row-group", CSSPrimitiveValue::CSS_STRING);
        case khtml::TABLE_HEADER_GROUP:
            return new CSSPrimitiveValueImpl("table-header-group", CSSPrimitiveValue::CSS_STRING);
        case khtml::TABLE_FOOTER_GROUP:
            return new CSSPrimitiveValueImpl("table-footer-group", CSSPrimitiveValue::CSS_STRING);
        case khtml::TABLE_ROW:
            return new CSSPrimitiveValueImpl("table-row", CSSPrimitiveValue::CSS_STRING);
        case khtml::TABLE_COLUMN_GROUP:
            return new CSSPrimitiveValueImpl("table-column-group", CSSPrimitiveValue::CSS_STRING);
        case khtml::TABLE_COLUMN:
            return new CSSPrimitiveValueImpl("table-column", CSSPrimitiveValue::CSS_STRING);
        case khtml::TABLE_CELL:
            return new CSSPrimitiveValueImpl("table-cell", CSSPrimitiveValue::CSS_STRING);
        case khtml::TABLE_CAPTION:
            return new CSSPrimitiveValueImpl("table-caption", CSSPrimitiveValue::CSS_STRING);
        case khtml::NONE:
            return new CSSPrimitiveValueImpl("none", CSSPrimitiveValue::CSS_STRING);
        default:
            Q_ASSERT( 0 );
        }
    case CSS_PROP_EMPTY_CELLS:
        switch (style->emptyCells()) {
            case khtml::SHOW:
                return new CSSPrimitiveValueImpl("show", CSSPrimitiveValue::CSS_STRING);
            case khtml::HIDE:
                return new CSSPrimitiveValueImpl("hide", CSSPrimitiveValue::CSS_STRING);
        }
        break;
    case CSS_PROP_FLOAT:
    {
        switch (style->floating()) {
        case khtml::FNONE:
            return new CSSPrimitiveValueImpl("none", CSSPrimitiveValue::CSS_STRING);
        case khtml::FLEFT:
            return new CSSPrimitiveValueImpl("left", CSSPrimitiveValue::CSS_STRING);
        case khtml::FRIGHT:
            return new CSSPrimitiveValueImpl("right", CSSPrimitiveValue::CSS_STRING);
        case khtml::FLEFT_ALIGN:
            return new CSSPrimitiveValueImpl("-khtml-left", CSSPrimitiveValue::CSS_STRING);
        case khtml::FRIGHT_ALIGN:
            return new CSSPrimitiveValueImpl("-khtml-right", CSSPrimitiveValue::CSS_STRING);
        }
    }
    case CSS_PROP_FONT_FAMILY:
    {
        FontDef def = style->htmlFont().getFontDef();
        return new CSSPrimitiveValueImpl(DOMString(def.family), CSSPrimitiveValue::CSS_STRING);
    }
    case CSS_PROP_FONT_SIZE:
    {
        FontDef def = style->htmlFont().getFontDef();
        return new CSSPrimitiveValueImpl(def.size, CSSPrimitiveValue::CSS_PX);
    }
    case CSS_PROP_FONT_STYLE:
    {
        // FIXME: handle oblique
        FontDef def = style->htmlFont().getFontDef();
        if (def.italic)
            return new CSSPrimitiveValueImpl("italic", CSSPrimitiveValue::CSS_STRING);
        else
            return new CSSPrimitiveValueImpl("normal", CSSPrimitiveValue::CSS_STRING);
    }
    case CSS_PROP_FONT_VARIANT:
    {
        FontDef def = style->htmlFont().getFontDef();
        if (def.smallCaps)
            return new CSSPrimitiveValueImpl("small-caps", CSSPrimitiveValue::CSS_STRING);
        else
            return new CSSPrimitiveValueImpl("normal", CSSPrimitiveValue::CSS_STRING);
    }
    case CSS_PROP_FONT_WEIGHT:
    {
        // FIXME: this does not reflect the full range of weights
        // that can be expressed with CSS
        FontDef def = style->htmlFont().getFontDef();
        if (def.weight == QFont::Bold)
            return new CSSPrimitiveValueImpl("bold", CSSPrimitiveValue::CSS_STRING);
        else
            return new CSSPrimitiveValueImpl("normal", CSSPrimitiveValue::CSS_STRING);
    }
    case CSS_PROP_HEIGHT:
        return new CSSPrimitiveValueImpl(renderer->contentHeight(), CSSPrimitiveValue::CSS_PX);
    case CSS_PROP_LEFT:
        break;
    case CSS_PROP_LETTER_SPACING:
        if (style->letterSpacing() == 0)
            return new CSSPrimitiveValueImpl("normal", CSSPrimitiveValue::CSS_STRING);
        return new CSSPrimitiveValueImpl(style->letterSpacing(), CSSPrimitiveValue::CSS_PX);
    case CSS_PROP_LINE_HEIGHT:
    {
        Length length(style->lineHeight());
        if (length.isPercent()) {
            //XXX: merge from webcore the computedStyle/specifiedStyle distinction in rendering/font.h
            float computedSize = style->htmlFont().getFontDef().size;
            return new CSSPrimitiveValueImpl((int)(length._length * computedSize) / 100, CSSPrimitiveValue::CSS_PX);
        }
        else {
            return new CSSPrimitiveValueImpl(length._length, CSSPrimitiveValue::CSS_PX);
        }
    }
    case CSS_PROP_LIST_STYLE_IMAGE:
        break;
    case CSS_PROP_LIST_STYLE_POSITION:
        switch (style->listStylePosition()) {
        case OUTSIDE:
            return new CSSPrimitiveValueImpl("outside", CSSPrimitiveValue::CSS_STRING);
        case INSIDE:
            return new CSSPrimitiveValueImpl("inside", CSSPrimitiveValue::CSS_STRING);
        }
        Q_ASSERT( 0 );
        break;
    case CSS_PROP_LIST_STYLE_TYPE:
        return new CSSPrimitiveValueImpl(stringForListStyleType(style->listStyleType()), CSSPrimitiveValue::CSS_STRING);
    case CSS_PROP_MARGIN_TOP:
        return valueForLength(style->marginTop(), renderer->contentHeight());
    case CSS_PROP_MARGIN_RIGHT:
        return valueForLength(style->marginRight(), renderer->contentWidth());
    case CSS_PROP_MARGIN_BOTTOM:
        return valueForLength(style->marginBottom(), renderer->contentHeight());
    case CSS_PROP_MARGIN_LEFT:
        return valueForLength(style->marginLeft(), renderer->contentWidth());
    case CSS_PROP_MAX_HEIGHT:
        return new CSSPrimitiveValueImpl( renderer->availableHeight(),
                                          CSSPrimitiveValue::CSS_PX );
        break;
    case CSS_PROP_MAX_WIDTH:
        return new CSSPrimitiveValueImpl( renderer->maxWidth(),
                                          CSSPrimitiveValue::CSS_PX );
        break;
    case CSS_PROP_MIN_HEIGHT:
        return new CSSPrimitiveValueImpl( renderer->contentHeight(),
                                          CSSPrimitiveValue::CSS_PX );
        break;
    case CSS_PROP_MIN_WIDTH:
        return new CSSPrimitiveValueImpl( renderer->minWidth(),
                                          CSSPrimitiveValue::CSS_PX );
        break;
    case CSS_PROP_OPACITY:
        break;
    case CSS_PROP_ORPHANS:
        break;
    case CSS_PROP_OUTLINE_COLOR:
        break;
    case CSS_PROP_OUTLINE_OFFSET:
        break;
    case CSS_PROP_OUTLINE_STYLE:
        break;
    case CSS_PROP_OUTLINE_WIDTH:
        break;
    case CSS_PROP_OVERFLOW:
    {
        switch (style->overflow()) {
        case khtml::OVISIBLE:
            return new CSSPrimitiveValueImpl("visible", CSSPrimitiveValue::CSS_STRING);
        case khtml::OHIDDEN:
            return new CSSPrimitiveValueImpl("hidden", CSSPrimitiveValue::CSS_STRING);
        case khtml::OSCROLL:
            return new CSSPrimitiveValueImpl("scroll", CSSPrimitiveValue::CSS_STRING);
        case khtml::OAUTO:
            return new CSSPrimitiveValueImpl("auto", CSSPrimitiveValue::CSS_STRING);
        case khtml::OMARQUEE:
            return new CSSPrimitiveValueImpl("marquee", CSSPrimitiveValue::CSS_STRING);
        }
    }
    case CSS_PROP_PADDING_TOP:
        return valueForLength(style->paddingTop(), renderer->contentHeight());
    case CSS_PROP_PADDING_RIGHT:
        return valueForLength(style->paddingRight(), renderer->contentWidth());
    case CSS_PROP_PADDING_BOTTOM:
        return valueForLength(style->paddingBottom(), renderer->contentHeight());
    case CSS_PROP_PADDING_LEFT:
        return valueForLength(style->paddingLeft(), renderer->contentWidth());
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
        return new CSSPrimitiveValueImpl(stringForTextAlign(style->textAlign()), CSSPrimitiveValue::CSS_STRING);
    case CSS_PROP_TEXT_DECORATION:
    {
        QString string;
        if (style->textDecoration() & khtml::UNDERLINE)
            string += "underline";
        if (style->textDecoration() & khtml::OVERLINE) {
            if (string.length() > 0)
                string += " ";
            string += "overline";
        }
        if (style->textDecoration() & khtml::LINE_THROUGH) {
            if (string.length() > 0)
                string += " ";
            string += "line-through";
        }
        if (style->textDecoration() & khtml::BLINK) {
            if (string.length() > 0)
                string += " ";
            string += "blink";
        }
        if (string.length() == 0)
            string = "none";
        return new CSSPrimitiveValueImpl(DOMString(string), CSSPrimitiveValue::CSS_STRING);
    }
    case CSS_PROP_TEXT_INDENT:
        return valueForLength(style->textIndent(), renderer->contentWidth());
    case CSS_PROP_TEXT_TRANSFORM:
        break;
    case CSS_PROP_TOP:
        break;
    case CSS_PROP_UNICODE_BIDI:
        break;
    case CSS_PROP_VERTICAL_ALIGN:
    {
        switch (style->verticalAlign()) {
        case khtml::BASELINE:
            return new CSSPrimitiveValueImpl("baseline", CSSPrimitiveValue::CSS_STRING);
        case khtml::MIDDLE:
            return new CSSPrimitiveValueImpl("middle", CSSPrimitiveValue::CSS_STRING);
        case khtml::SUB:
            return new CSSPrimitiveValueImpl("sub", CSSPrimitiveValue::CSS_STRING);
        case khtml::SUPER:
            return new CSSPrimitiveValueImpl("super", CSSPrimitiveValue::CSS_STRING);
        case khtml::TEXT_TOP:
            return new CSSPrimitiveValueImpl("text-top", CSSPrimitiveValue::CSS_STRING);
        case khtml::TEXT_BOTTOM:
            return new CSSPrimitiveValueImpl("text-bottom", CSSPrimitiveValue::CSS_STRING);
        case khtml::TOP:
            return new CSSPrimitiveValueImpl("top", CSSPrimitiveValue::CSS_STRING);
        case khtml::BOTTOM:
            return new CSSPrimitiveValueImpl("bottom", CSSPrimitiveValue::CSS_STRING);
        case khtml::BASELINE_MIDDLE:
            return new CSSPrimitiveValueImpl("baseline-middle", CSSPrimitiveValue::CSS_STRING);
        case khtml::LENGTH:
            return valueForLength(style->verticalAlignLength(), renderer->contentWidth());
        }
    }
    case CSS_PROP_VISIBILITY:
        switch (style->visibility()) {
            case khtml::VISIBLE:
                return new CSSPrimitiveValueImpl("visible", CSSPrimitiveValue::CSS_STRING);
            case khtml::HIDDEN:
                return new CSSPrimitiveValueImpl("hidden", CSSPrimitiveValue::CSS_STRING);
            case khtml::COLLAPSE:
                return new CSSPrimitiveValueImpl("collapse", CSSPrimitiveValue::CSS_STRING);
        }
        break;
    case CSS_PROP_WHITE_SPACE:
    {
        switch (style->whiteSpace()) {
            case khtml::NORMAL:
                return new CSSPrimitiveValueImpl("normal", CSSPrimitiveValue::CSS_STRING);
            case khtml::NOWRAP:
                return new CSSPrimitiveValueImpl("nowrap", CSSPrimitiveValue::CSS_STRING);
            case khtml::PRE:
                return new CSSPrimitiveValueImpl("pre", CSSPrimitiveValue::CSS_STRING);
            case khtml::PRE_WRAP:
                return new CSSPrimitiveValueImpl("pre-wrap", CSSPrimitiveValue::CSS_STRING);
            case khtml::PRE_LINE:
                return new CSSPrimitiveValueImpl("pre-line", CSSPrimitiveValue::CSS_STRING);
            case khtml::KHTML_NOWRAP:
                return new CSSPrimitiveValueImpl("-khtml-nowrap", CSSPrimitiveValue::CSS_STRING);
        }
        break;
    }
    case CSS_PROP_WIDOWS:
        break;
    case CSS_PROP_WIDTH:
        return new CSSPrimitiveValueImpl( renderer->contentWidth(),
                                         CSSPrimitiveValue::CSS_PX );
    case CSS_PROP_WORD_SPACING:
        return new CSSPrimitiveValueImpl(style->wordSpacing(), CSSPrimitiveValue::CSS_PX);
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
        return new CSSPrimitiveValueImpl( renderer->borderTop(),
                                         CSSPrimitiveValue::CSS_PX );
        break;
    case CSS_PROP_BORDER_RIGHT:
        return new CSSPrimitiveValueImpl( renderer->borderRight(),
                                         CSSPrimitiveValue::CSS_PX );
        break;
    case CSS_PROP_BORDER_BOTTOM:
        return new CSSPrimitiveValueImpl( renderer->borderBottom(),
                                         CSSPrimitiveValue::CSS_PX );
        break;
    case CSS_PROP_BORDER_LEFT:
        return new CSSPrimitiveValueImpl( renderer->borderLeft(),
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
    default:
        Q_ASSERT( 0 );
        break;
    }
    return 0;
}

DOMString RenderStyleDeclarationImpl::getPropertyValue( int propertyID ) const
{
    CSSValueImpl* value = getPropertyCSSValue(propertyID);
    if (value)
        return value->cssText();
    return "";
}

bool RenderStyleDeclarationImpl::getPropertyPriority( int ) const
{
    // This class does not support the notion of priority, since the object
    // is a computed value.
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
