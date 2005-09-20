/*
 * This file is part of the DOM implementation for KDE.
 *
 * Copyright (C) 1999 Antti Koivisto (koivisto@kde.org)
 * Copyright (C) 1999-2003 Lars Knoll (knoll@kde.org)
 * Copyright (C) 2002-2003 Dirk Mueller (mueller@kde.org)
 * Copyright (C) 2002-2004 Apple Computer, Inc.
 * Copyright (C) 2005 Allan Sandfeld Jensen (kde@carewolf.com)
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
 * the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#include "xml/dom_stringimpl.h"
#include "css/cssstyleselector.h"
#include "css/css_valueimpl.h"
#include "render_style.h"

#include "kdebug.h"

using namespace khtml;
using namespace DOM;

/* CSS says Fixed for the default padding value, but we treat variable as 0 padding anyways, and like
 * this is works fine for table paddings aswell
 */
StyleSurroundData::StyleSurroundData()
    : margin( Fixed ), padding( Variable )
{
}

StyleSurroundData::StyleSurroundData(const StyleSurroundData& o )
    : Shared<StyleSurroundData>(),
      offset( o.offset ), margin( o.margin ), padding( o.padding ),
      border( o.border )
{
}

bool StyleSurroundData::operator==(const StyleSurroundData& o) const
{
    return offset==o.offset && margin==o.margin &&
	padding==o.padding && border==o.border;
}

StyleBoxData::StyleBoxData()
    : z_index( 0 ), z_auto( true )
{
    min_width = min_height = RenderStyle::initialMinSize();
    max_width = max_height = RenderStyle::initialMaxSize();
    box_sizing = RenderStyle::initialBoxSizing();
}

StyleBoxData::StyleBoxData(const StyleBoxData& o )
    : Shared<StyleBoxData>(),
      width( o.width ), height( o.height ),
      min_width( o.min_width ), max_width( o.max_width ),
      min_height ( o.min_height ), max_height( o.max_height ),
      box_sizing( o.box_sizing),
      z_index( o.z_index ), z_auto( o.z_auto )
{
}

bool StyleBoxData::operator==(const StyleBoxData& o) const
{
    return
	    width == o.width &&
	    height == o.height &&
	    min_width == o.min_width &&
	    max_width == o.max_width &&
	    min_height == o.min_height &&
	    max_height == o.max_height &&
            box_sizing == o.box_sizing &&
	    z_index == o.z_index &&
            z_auto == o.z_auto;
}

StyleVisualData::StyleVisualData()
     : textDecoration(RenderStyle::initialTextDecoration()),
      palette( QApplication::palette() )
{
}

StyleVisualData::~StyleVisualData() {
}

StyleVisualData::StyleVisualData(const StyleVisualData& o )
    : Shared<StyleVisualData>(),
      clip( o.clip ), textDecoration(o.textDecoration),
      palette( o.palette )
{
}

BackgroundLayer::BackgroundLayer()
:m_image(RenderStyle::initialBackgroundImage()),
 m_bgAttachment(RenderStyle::initialBackgroundAttachment()),
 m_bgRepeat(RenderStyle::initialBackgroundRepeat()),
 m_next(0)
{
    m_imageSet = m_attachmentSet = m_repeatSet = m_xPosSet = m_yPosSet = false;
}

BackgroundLayer::BackgroundLayer(const BackgroundLayer& o)
{
    m_next = o.m_next ? new BackgroundLayer(*o.m_next) : 0;
    m_image = o.m_image;
    m_xPosition = o.m_xPosition;
    m_yPosition = o.m_yPosition;
    m_bgAttachment = o.m_bgAttachment;
    m_bgRepeat = o.m_bgRepeat;
    m_imageSet = o.m_imageSet;
    m_attachmentSet = o.m_attachmentSet;
    m_repeatSet = o.m_repeatSet;
    m_xPosSet = o.m_xPosSet;
    m_yPosSet = o.m_yPosSet;
}

BackgroundLayer::~BackgroundLayer()
{
    delete m_next;
}

BackgroundLayer& BackgroundLayer::operator=(const BackgroundLayer& o) {
    if (m_next != o.m_next) {
        delete m_next;
        m_next = o.m_next ? new BackgroundLayer(*o.m_next) : 0;
    }

    m_image = o.m_image;
    m_xPosition = o.m_xPosition;
    m_yPosition = o.m_yPosition;
    m_bgAttachment = o.m_bgAttachment;
    m_bgRepeat = o.m_bgRepeat;

    m_imageSet = o.m_imageSet;
    m_attachmentSet = o.m_attachmentSet;
    m_repeatSet = o.m_repeatSet;
    m_xPosSet = o.m_xPosSet;
    m_yPosSet = o.m_yPosSet;

    return *this;
}

bool BackgroundLayer::operator==(const BackgroundLayer& o) const {
    return m_image == o.m_image && m_xPosition == o.m_xPosition && m_yPosition == o.m_yPosition &&
           m_bgAttachment == o.m_bgAttachment && m_bgRepeat == o.m_bgRepeat &&
           m_imageSet == o.m_imageSet && m_attachmentSet == o.m_attachmentSet && m_repeatSet == o.m_repeatSet &&
           m_xPosSet == o.m_xPosSet && m_yPosSet == o.m_yPosSet &&
           ((m_next && o.m_next) ? *m_next == *o.m_next : m_next == o.m_next);
}

void BackgroundLayer::fillUnsetProperties()
{
    BackgroundLayer* curr;
    for (curr = this; curr && curr->isBackgroundImageSet(); curr = curr->next());
    if (curr && curr != this) {
        // We need to fill in the remaining values with the pattern specified.
        for (BackgroundLayer* pattern = this; curr; curr = curr->next()) {
            curr->m_image = pattern->m_image;
            pattern = pattern->next();
            if (pattern == curr || !pattern)
                pattern = this;
        }
    }

    for (curr = this; curr && curr->isBackgroundXPositionSet(); curr = curr->next());
    if (curr && curr != this) {
        // We need to fill in the remaining values with the pattern specified.
        for (BackgroundLayer* pattern = this; curr; curr = curr->next()) {
            curr->m_xPosition = pattern->m_xPosition;
            pattern = pattern->next();
            if (pattern == curr || !pattern)
                pattern = this;
        }
    }

    for (curr = this; curr && curr->isBackgroundYPositionSet(); curr = curr->next());
    if (curr && curr != this) {
        // We need to fill in the remaining values with the pattern specified.
        for (BackgroundLayer* pattern = this; curr; curr = curr->next()) {
            curr->m_yPosition = pattern->m_yPosition;
            pattern = pattern->next();
            if (pattern == curr || !pattern)
                pattern = this;
        }
    }

    for (curr = this; curr && curr->isBackgroundAttachmentSet(); curr = curr->next());
    if (curr && curr != this) {
        // We need to fill in the remaining values with the pattern specified.
        for (BackgroundLayer* pattern = this; curr; curr = curr->next()) {
            curr->m_bgAttachment = pattern->m_bgAttachment;
            pattern = pattern->next();
            if (pattern == curr || !pattern)
                pattern = this;
        }
    }

    for (curr = this; curr && curr->isBackgroundRepeatSet(); curr = curr->next());
    if (curr && curr != this) {
        // We need to fill in the remaining values with the pattern specified.
        for (BackgroundLayer* pattern = this; curr; curr = curr->next()) {
            curr->m_bgRepeat = pattern->m_bgRepeat;
            pattern = pattern->next();
            if (pattern == curr || !pattern)
                pattern = this;
        }
    }
}

void BackgroundLayer::cullEmptyLayers()
{
    BackgroundLayer *next;
    for (BackgroundLayer *p = this; p; p = next) {
        next = p->m_next;
        if (next && !next->isBackgroundImageSet() &&
            !next->isBackgroundXPositionSet() && !next->isBackgroundYPositionSet() &&
            !next->isBackgroundAttachmentSet() && !next->isBackgroundRepeatSet()) {
            delete next;
            p->m_next = 0;
            break;
        }
    }
}

StyleBackgroundData::StyleBackgroundData()
{}

StyleBackgroundData::StyleBackgroundData(const StyleBackgroundData& o)
    : Shared<StyleBackgroundData>(), m_background(o.m_background), m_outline(o.m_outline)
{}

bool StyleBackgroundData::operator==(const StyleBackgroundData& o) const
{
    return m_background == o.m_background && m_color == o.m_color && m_outline == o.m_outline;
}

StyleMarqueeData::StyleMarqueeData()
{
    increment = RenderStyle::initialMarqueeIncrement();
    speed = RenderStyle::initialMarqueeSpeed();
    direction = RenderStyle::initialMarqueeDirection();
    behavior = RenderStyle::initialMarqueeBehavior();
    loops = RenderStyle::initialMarqueeLoopCount();
}

StyleMarqueeData::StyleMarqueeData(const StyleMarqueeData& o)
:Shared<StyleMarqueeData>(), increment(o.increment), speed(o.speed), loops(o.loops),
 behavior(o.behavior), direction(o.direction)
{}

bool StyleMarqueeData::operator==(const StyleMarqueeData& o) const
{
    return (increment == o.increment && speed == o.speed && direction == o.direction &&
            behavior == o.behavior && loops == o.loops);
}

StyleCSS3NonInheritedData::StyleCSS3NonInheritedData()
:Shared<StyleCSS3NonInheritedData>()
, opacity(RenderStyle::initialOpacity())
{
}

StyleCSS3NonInheritedData::StyleCSS3NonInheritedData(const StyleCSS3NonInheritedData& o)
:Shared<StyleCSS3NonInheritedData>(),
 opacity(o.opacity),
#ifdef APPLE_CHANGES
 flexibleBox(o.flexibleBox),
#endif
 marquee(o.marquee)
{
}

bool StyleCSS3NonInheritedData::operator==(const StyleCSS3NonInheritedData& o) const
{
    return
     opacity == o.opacity &&
#ifdef APPLE_CHANGES
     flexibleBox == o.flexibleBox &&
#endif
     marquee == o.marquee;
}

StyleCSS3InheritedData::StyleCSS3InheritedData()
:Shared<StyleCSS3InheritedData>(), textShadow(0)
#ifdef APPLE_CHANGES
, userModify(READ_ONLY), textSizeAdjust(RenderStyle::initialTextSizeAdjust())
#endif
{

}

StyleCSS3InheritedData::StyleCSS3InheritedData(const StyleCSS3InheritedData& o)
:Shared<StyleCSS3InheritedData>()
{
    textShadow = o.textShadow ? new ShadowData(*o.textShadow) : 0;
#ifdef APPLE_CHANGES
    userModify = o.userModify;
    textSizeAdjust = o.textSizeAdjust;
#endif
}

StyleCSS3InheritedData::~StyleCSS3InheritedData()
{
    delete textShadow;
}

bool StyleCSS3InheritedData::operator==(const StyleCSS3InheritedData& o) const
{
    return shadowDataEquivalent(o)
#ifdef APPLE_CHANGES
            && (userModify == o.userModify) && (textSizeAdjust == o.textSizeAdjust)
#endif
    ;
}

bool StyleCSS3InheritedData::shadowDataEquivalent(const StyleCSS3InheritedData& o) const
{
    if (!textShadow && o.textShadow || textShadow && !o.textShadow)
        return false;
    if (textShadow && o.textShadow && (*textShadow != *o.textShadow))
        return false;
    return true;
}

StyleInheritedData::StyleInheritedData()
    : indent( RenderStyle::initialTextIndent() ), line_height( RenderStyle::initialLineHeight() ),
      style_image( RenderStyle::initialListStyleImage() ),
      font(), color( RenderStyle::initialColor() ),
      border_hspacing( RenderStyle::initialBorderHorizontalSpacing() ),
      border_vspacing( RenderStyle::initialBorderVerticalSpacing() ),
      widows( RenderStyle::initialWidows() ), orphans( RenderStyle::initialOrphans() ),
      quotes(0)
{
}

StyleInheritedData::~StyleInheritedData()
{
    if (quotes) quotes->deref();
}

StyleInheritedData::StyleInheritedData(const StyleInheritedData& o )
    : Shared<StyleInheritedData>(),
      indent( o.indent ), line_height( o.line_height ), style_image( o.style_image ),
      font( o.font ), color( o.color ), decoration_color( o.decoration_color ),
      border_hspacing( o.border_hspacing ),
      border_vspacing( o.border_vspacing ),
      widows(o.widows), orphans(o.orphans)
{
    quotes = o.quotes;
    if (quotes) quotes->ref();
}

bool StyleInheritedData::operator==(const StyleInheritedData& o) const
{
    return
	indent == o.indent &&
	line_height == o.line_height &&
        border_hspacing == o.border_hspacing &&
        border_vspacing == o.border_vspacing &&
	style_image == o.style_image &&
	font == o.font &&
	color == o.color &&
	decoration_color == o.decoration_color &&
        border_hspacing == o.border_hspacing &&
        border_vspacing == o.border_vspacing &&
        quotes == o.quotes &&
        widows == o.widows &&
        orphans == o.orphans ;

    // doesn't work because structs are not packed
    //return memcmp(this, &o, sizeof(*this))==0;
}

RenderStyle::RenderStyle()
{
//    counter++;
    if (!_default)
	_default = new RenderStyle(true);

    box = _default->box;
    visual = _default->visual;
    background = _default->background;
    surround = _default->surround;
    css3NonInheritedData = _default->css3NonInheritedData;
    css3InheritedData = _default->css3InheritedData;

    inherited = _default->inherited;

    setBitDefaults();

    pseudoStyle = 0;
    content = 0;
    counter_reset = 0;
    counter_increment = 0;
}

RenderStyle::RenderStyle(bool)
{
    setBitDefaults();

    box.init();
    visual.init();
    background.init();
    surround.init();
    css3NonInheritedData.init();
#ifdef APPLE_CHANGES	// ### yet to be merged
    css3NonInheritedData.access()->flexibleBox.init();
#endif
    css3NonInheritedData.access()->marquee.init();
    css3InheritedData.init();
    inherited.init();

    pseudoStyle = 0;
    content = 0;
    counter_reset = 0;
    counter_increment = 0;
}

RenderStyle::RenderStyle(const RenderStyle& o)
    : Shared<RenderStyle>(),
      inherited_flags( o.inherited_flags ), noninherited_flags( o.noninherited_flags ),
      box( o.box ), visual( o.visual ), background( o.background ), surround( o.surround ),
      css3NonInheritedData( o.css3NonInheritedData ), css3InheritedData( o.css3InheritedData ),
      inherited( o.inherited ), pseudoStyle( 0 ), content( o.content ),
      counter_reset(o.counter_reset), counter_increment(o.counter_increment)
{
    if (counter_reset) counter_reset->ref();
    if (counter_increment) counter_increment->ref();
}

void RenderStyle::inheritFrom(const RenderStyle* inheritParent)
{
    css3InheritedData = inheritParent->css3InheritedData;
    inherited = inheritParent->inherited;
    inherited_flags = inheritParent->inherited_flags;

    // Simulate ":after,:before { white-space: pre-line }"
    if (styleType() == AFTER || styleType() == BEFORE)
        setWhiteSpace(PRE_LINE);
}

RenderStyle::~RenderStyle()
{
    RenderStyle *ps = pseudoStyle;
    RenderStyle *prev = 0;

    while (ps) {
        prev = ps;
        ps = ps->pseudoStyle;
	// to prevent a double deletion.
	// this works only because the styles below aren't really shared
	// Dirk said we need another construct as soon as these are shared
        prev->pseudoStyle = 0;
        prev->deref();
    }
    delete content;
    if (counter_reset) counter_reset->deref();
    if (counter_increment) counter_increment->deref();
}

bool RenderStyle::operator==(const RenderStyle& o) const
{
// compare everything except the pseudoStyle pointer
    return (inherited_flags == o.inherited_flags &&
            noninherited_flags == o.noninherited_flags &&
	    box == o.box &&
            visual == o.visual &&
            background == o.background &&
            surround == o.surround &&
            css3NonInheritedData == o.css3NonInheritedData &&
            css3InheritedData == o.css3InheritedData &&
            inherited == o.inherited);
}

RenderStyle* RenderStyle::getPseudoStyle(PseudoId pid)
{
    RenderStyle *ps = 0;
    if (noninherited_flags.f._styleType==NOPSEUDO)
        for (ps = pseudoStyle; ps; ps = ps->pseudoStyle)
            if (ps->noninherited_flags.f._styleType==pid)
		break;
    return ps;
}

RenderStyle* RenderStyle::addPseudoStyle(PseudoId pid)
{
    RenderStyle *ps = getPseudoStyle(pid);

    if (!ps)
    {
        switch (pid) {
          case FIRST_LETTER:             // pseudo-elements (FIRST_LINE has a special handling)
          case BEFORE:
          case AFTER:
            ps = new RenderStyle();
            break;
          default:
            ps = new RenderStyle(*this); // use the real copy constructor to get an identical copy
        }
        ps->ref();
        ps->noninherited_flags.f._styleType = pid;
        ps->pseudoStyle = pseudoStyle;

        pseudoStyle = ps;
    }

    return ps;
}

void RenderStyle::removePseudoStyle(PseudoId pid)
{
    RenderStyle *ps = pseudoStyle;
    RenderStyle *prev = this;

    while (ps) {
        if (ps->noninherited_flags.f._styleType==pid) {
            prev->pseudoStyle = ps->pseudoStyle;
            ps->deref();
            return;
        }
        prev = ps;
        ps = ps->pseudoStyle;
    }
}


bool RenderStyle::inheritedNotEqual( RenderStyle *other ) const
{
    return
	(
	    inherited_flags != other->inherited_flags ||
            inherited != other->inherited ||
            css3InheritedData != other->css3InheritedData
	    );
}

/*
  compares two styles. The result gives an idea of the action that
  needs to be taken when replacing the old style with a new one.

  CbLayout: The containing block of the object needs a relayout.
  Layout: the RenderObject needs a relayout after the style change
  Visible: The change is visible, but no relayout is needed
  NonVisible: The object does need neither repaint nor relayout after
       the change.

  ### TODO:
  A lot can be optimised here based on the display type, lots of
  optimisations are unimplemented, and currently result in the
  worst case result causing a relayout of the containing block.
*/
RenderStyle::Diff RenderStyle::diff( const RenderStyle *other ) const
{
    // we anyway assume they are the same
// 	EDisplay _display : 5;

    // NonVisible:
// 	ECursor _cursor_style : 4;
//	EUserInput _user_input : 2;	as long as :enabled is not impl'd

// ### this needs work to know more exactly if we need a relayout
//     or just a repaint

// non-inherited attributes
//     DataRef<StyleBoxData> box;
//     DataRef<StyleVisualData> visual;
//     DataRef<StyleSurroundData> surround;

// inherited attributes
//     DataRef<StyleInheritedData> inherited;

    if ( *box.get() != *other->box.get() ||
         *visual.get() != *other->visual.get() ||
         *surround.get() != *other->surround.get() ||
         !(inherited->indent == other->inherited->indent) ||
         !(inherited->line_height == other->inherited->line_height) ||
         !(inherited->style_image == other->inherited->style_image) ||
         !(inherited->font == other->inherited->font) ||
         !(inherited->border_hspacing == other->inherited->border_hspacing) ||
         !(inherited->border_vspacing == other->inherited->border_vspacing) ||
         !(inherited_flags.f._visuallyOrdered == other->inherited_flags.f._visuallyOrdered) ||
         !(inherited_flags.f._htmlHacks == other->inherited_flags.f._htmlHacks) )
        return CbLayout;

    // changes causing Layout changes:

// only for tables:
// 	_border_collapse
// 	EEmptyCell _empty_cells : 2 ;
// 	ECaptionSide _caption_side : 2;
//     ETableLayout _table_layout : 1;
//     EPosition _position : 2;
//     EFloat _floating : 2;
    if ( ((int)noninherited_flags.f._display) >= TABLE ) {
        if ( !(inherited_flags.f._empty_cells == other->inherited_flags.f._empty_cells) ||
             !(inherited_flags.f._caption_side == other->inherited_flags.f._caption_side) ||
             !(inherited_flags.f._border_collapse == other->inherited_flags.f._border_collapse) ||
             !(noninherited_flags.f._table_layout == other->noninherited_flags.f._table_layout) ||
             !(noninherited_flags.f._position == other->noninherited_flags.f._position) ||
             !(noninherited_flags.f._floating == other->noninherited_flags.f._floating) ||
             !(noninherited_flags.f._flowAroundFloats == other->noninherited_flags.f._flowAroundFloats) ||
             !(noninherited_flags.f._unicodeBidi == other->noninherited_flags.f._unicodeBidi) )
            return CbLayout;
    }

// only for lists:
// 	EListStyleType _list_style_type : 5 ;
// 	EListStylePosition _list_style_position :1;
    if (noninherited_flags.f._display == LIST_ITEM ) {
        if ( !(inherited_flags.f._list_style_type == other->inherited_flags.f._list_style_type) ||
             !(inherited_flags.f._list_style_position == other->inherited_flags.f._list_style_position) )
            return Layout;
    }

// ### These could be better optimised
// 	ETextAlign _text_align : 3;
// 	ETextTransform _text_transform : 4;
// 	EDirection _direction : 1;
// 	EWhiteSpace _white_space : 2;
//     EClear _clear : 2;
    if ( !(inherited_flags.f._text_align == other->inherited_flags.f._text_align) ||
	 !(inherited_flags.f._text_transform == other->inherited_flags.f._text_transform) ||
	 !(inherited_flags.f._direction == other->inherited_flags.f._direction) ||
	 !(inherited_flags.f._white_space == other->inherited_flags.f._white_space) ||
	 !(noninherited_flags.f._clear == other->noninherited_flags.f._clear)
	)
        return Layout;

// only for inline:
//     EVerticalAlign _vertical_align : 4;

    if ( !(noninherited_flags.f._display == INLINE) &&
         !(noninherited_flags.f._vertical_align == other->noninherited_flags.f._vertical_align) )
	    return Layout;

    // Visible:
// 	EVisibility _visibility : 2;
//     EOverflow _overflow : 4 ;
// 	int _text_decorations : 4;
//     DataRef<StyleBackgroundData> background;
    if (inherited->color != other->inherited->color ||
        inherited->decoration_color != other->inherited->decoration_color ||
        !(inherited_flags.f._visibility == other->inherited_flags.f._visibility) ||
        !(noninherited_flags.f._overflow == other->noninherited_flags.f._overflow) ||
        !(inherited_flags.f._text_decorations == other->inherited_flags.f._text_decorations) ||
        !(noninherited_flags.f._hasClip == other->noninherited_flags.f._hasClip) ||
        visual->textDecoration != other->visual->textDecoration ||
        *background.get() != *other->background.get() ||
        css3NonInheritedData->opacity != other->css3NonInheritedData->opacity ||
        !css3InheritedData->shadowDataEquivalent(*other->css3InheritedData.get())
       )
        return Visible;

    return Equal;
}


RenderStyle* RenderStyle::_default = 0;

void RenderStyle::cleanup()
{
    delete _default;
    _default = 0;
}

void RenderStyle::setPaletteColor(QPalette::ColorGroup g, QColorGroup::ColorRole r, const QColor& c)
{
    visual.access()->palette.setColor(g,r,c);
}

void RenderStyle::adjustBackgroundLayers()
{
    if (backgroundLayers()->next()) {
        // First we cull out layers that have no properties set.
        accessBackgroundLayers()->cullEmptyLayers();

        // Next we repeat patterns into layers that don't have some properties set.
        accessBackgroundLayers()->fillUnsetProperties();
    }
}

void RenderStyle::setClip( Length top, Length right, Length bottom, Length left )
{
    StyleVisualData *data = visual.access();
    data->clip.top = top;
    data->clip.right = right;
    data->clip.bottom = bottom;
    data->clip.left = left;
}

void RenderStyle::setQuotes(DOM::QuotesValueImpl* q)
{
    DOM::QuotesValueImpl *t = inherited->quotes;
    inherited.access()->quotes = q;
    if (q) q->ref();
    if (t) t->deref();
}

QString RenderStyle::openQuote(int level) const
{
    if (inherited->quotes)
        return inherited->quotes->openQuote(level);
    else
        return "\""; // 0 is default quotes
}

QString RenderStyle::closeQuote(int level) const
{
    if (inherited->quotes)
        return inherited->quotes->closeQuote(level);
    else
        return "\""; // 0 is default quotes
}

bool RenderStyle::contentDataEquivalent(RenderStyle* otherStyle)
{
    ContentData* c1 = content;
    ContentData* c2 = otherStyle->content;

    while (c1 && c2) {
        if (c1->_contentType != c2->_contentType)
            return false;
        if (c1->_contentType == CONTENT_TEXT) {
            DOM::DOMString c1Str(c1->_content.text);
            DOM::DOMString c2Str(c2->_content.text);
            if (c1Str != c2Str)
                return false;
        }
        else if (c1->_contentType == CONTENT_OBJECT) {
            if (c1->_content.object != c2->_content.object)
                return false;
        }
        else if (c1->_contentType == CONTENT_COUNTER) {
            if (c1->_content.counter != c2->_content.counter)
                return false;
        }

        c1 = c1->_nextContent;
        c2 = c2->_nextContent;
    }

    return !c1 && !c2;
}

void RenderStyle::setContent(CachedObject* o, bool add)
{
    if (!o)
        return; // The object is null. Nothing to do. Just bail.

    ContentData* lastContent = content;
    while (lastContent && lastContent->_nextContent)
        lastContent = lastContent->_nextContent;

    bool reuseContent = !add;
    ContentData* newContentData = 0;
    if (reuseContent && content) {
        content->clearContent();
        newContentData = content;
    }
    else
        newContentData = new ContentData;

    if (lastContent && !reuseContent)
        lastContent->_nextContent = newContentData;
    else
        content = newContentData;

    //    o->ref();
    newContentData->_content.object = o;
    newContentData->_contentType = CONTENT_OBJECT;
}

void RenderStyle::setContent(DOM::DOMStringImpl* s, bool add)
{
    if (!s)
        return; // The string is null. Nothing to do. Just bail.

    ContentData* lastContent = content;
    while (lastContent && lastContent->_nextContent)
        lastContent = lastContent->_nextContent;

    bool reuseContent = !add;
    if (add && lastContent) {
        if (lastContent->_contentType == CONTENT_TEXT) {
            // We can augment the existing string and share this ContentData node.
            DOMStringImpl* oldStr = lastContent->_content.text;
            DOMStringImpl* newStr = oldStr->copy();
            newStr->ref();
            oldStr->deref();
            newStr->append(s);
            lastContent->_content.text = newStr;
            return;
        }
    }

    ContentData* newContentData = 0;
    if (reuseContent && content) {
        content->clearContent();
        newContentData = content;
    }
    else
        newContentData = new ContentData;

    if (lastContent && !reuseContent)
        lastContent->_nextContent = newContentData;
    else
        content = newContentData;

    newContentData->_content.text = s;
    newContentData->_content.text->ref();
    newContentData->_contentType = CONTENT_TEXT;

}

void RenderStyle::setContent(DOM::CounterImpl* c, bool add)
{
    if (!c)
        return;

    ContentData* lastContent = content;
    while (lastContent && lastContent->_nextContent)
        lastContent = lastContent->_nextContent;

    bool reuseContent = !add;
    ContentData* newContentData = 0;
    if (reuseContent && content) {
        content->clearContent();
        newContentData = content;
    }
    else
        newContentData = new ContentData;

    if (lastContent && !reuseContent)
        lastContent->_nextContent = newContentData;
    else
        content = newContentData;

    c->ref();
    newContentData->_content.counter = c;
    newContentData->_contentType = CONTENT_COUNTER;
}

void RenderStyle::setContent(EQuoteContent q, bool add)
{
    if (q == NO_QUOTE)
        return;

    ContentData* lastContent = content;
    while (lastContent && lastContent->_nextContent)
        lastContent = lastContent->_nextContent;

    bool reuseContent = !add;
    ContentData* newContentData = 0;
    if (reuseContent && content) {
        content->clearContent();
        newContentData = content;
    }
    else
        newContentData = new ContentData;

    if (lastContent && !reuseContent)
        lastContent->_nextContent = newContentData;
    else
        content = newContentData;

    newContentData->_content.quote = q;
    newContentData->_contentType = CONTENT_QUOTE;
}

ContentData::~ContentData()
{
    clearContent();
}

void ContentData::clearContent()
{
    delete _nextContent;
    _nextContent = 0;

    switch (_contentType)
    {
        case CONTENT_OBJECT:
            _content.object = 0;
            break;
        case CONTENT_TEXT:
            _content.text->deref();
            _content.text = 0;
            break;
        case CONTENT_COUNTER:
            _content.counter->deref();
            _content.counter = 0;
            break;
        default:
            ;
    }
}

void RenderStyle::setTextShadow(ShadowData* val, bool add)
{
    StyleCSS3InheritedData* css3Data = css3InheritedData.access();
    if (!add) {
        delete css3Data->textShadow;
        css3Data->textShadow = val;
        return;
    }

    ShadowData* last = css3Data->textShadow;
    while (last->next) last = last->next;
    last->next = val;
}

ShadowData::ShadowData(const ShadowData& o)
:x(o.x), y(o.y), blur(o.blur), color(o.color)
{
    next = o.next ? new ShadowData(*o.next) : 0;
}

bool ShadowData::operator==(const ShadowData& o) const
{
    if ((next && !o.next) || (!next && o.next) ||
        (next && o.next && *next != *o.next))
        return false;

    return x == o.x && y == o.y && blur == o.blur && color == o.color;
}

bool RenderStyle::counterDataEquivalent(RenderStyle* otherStyle)
{
    // ### Should we compare content?
    return counter_reset == otherStyle->counter_reset &&
           counter_increment == otherStyle->counter_increment;
}

static bool hasCounter(const DOM::DOMString& c, CSSValueListImpl *l)
{
    int len = l->length();
    for(int i=0; i<len; i++) {
        CounterActImpl* ca = static_cast<CounterActImpl*>(l->item(i));
        Q_ASSERT(ca != 0);
        if (ca->m_counter == c) return true;
    }
    return false;
}

bool RenderStyle::hasCounterReset(const DOM::DOMString& c) const
{
    if (counter_reset)
        return hasCounter(c, counter_reset);
    else
        return false;
}

bool RenderStyle::hasCounterIncrement(const DOM::DOMString& c) const
{
    if (counter_increment)
        return hasCounter(c, counter_increment);
    else
        return false;
}

static short readCounter(const DOM::DOMString& c, CSSValueListImpl *l)
{
    int len = l->length();
    for(int i=0; i<len; i++) {
        CounterActImpl* ca = static_cast<CounterActImpl*>(l->item(i));
        Q_ASSERT(ca != 0);
        if (ca->m_counter == c) return ca->m_value;
    }
    return 0;
}

short RenderStyle::counterReset(const DOM::DOMString& c) const
{
    if (counter_reset)
        return readCounter(c, counter_reset);
    else
        return 0;
}

short RenderStyle::counterIncrement(const DOM::DOMString& c) const
{
    if (counter_increment)
        return readCounter(c, counter_increment);
    else
        return 0;
}

void RenderStyle::setCounterReset(CSSValueListImpl *l)
{
    CSSValueListImpl *t = counter_reset;
    counter_reset = l;
    if (l) l->ref();
    if (t) t->deref();
}

void RenderStyle::setCounterIncrement(CSSValueListImpl *l)
{
    CSSValueListImpl *t = counter_increment;
    counter_increment = l;
    if (l) l->ref();
    if (t) t->deref();
}

#ifdef ENABLE_DUMP

static QString describeFont( const QFont &f)
{
    QString res = "'" + f.family() + "' ";

    if ( f.pointSize() > 0)
        res += QString::number( f.pointSize() ) + "pt";
    else
        res += QString::number( f.pixelSize() ) + "px";

    if ( f.bold() )
        res += " bold";
    if ( f.italic() )
        res += " italic";
    if ( f.underline() )
        res += " underline";
    if ( f.overline() )
        res += " overline";
    if ( f.strikeOut() )
        res += " strikeout";
    return res;
}

QString RenderStyle::createDiff( const RenderStyle &parent ) const
{
    QString res;
      if ( color().isValid() && parent.color() != color() )
        res += " [color=" + color().name() + "]";
    if ( backgroundColor().isValid() && parent.backgroundColor() != backgroundColor() )
        res += " [bgcolor=" + backgroundColor().name() + "]";
    if ( parent.font() != font() )
        res += " [font=" + describeFont( font() ) + "]";

    return res;
}
#endif

RenderPageStyle::RenderPageStyle() : next(0), m_pageType(ANY_PAGE)
{
}

RenderPageStyle::~RenderPageStyle()
{
    delete next;
}

RenderPageStyle* RenderPageStyle::getPageStyle(PageType type)
{
    RenderPageStyle *ps = 0;
    for (ps = this; ps; ps = ps->next)
        if (ps->m_pageType==type)
            break;
    return ps;
}

RenderPageStyle* RenderPageStyle::addPageStyle(PageType type)
{
    RenderPageStyle *ps = getPageStyle(type);

    if (!ps)
    {
        ps = new RenderPageStyle(*this); // use the real copy constructor to get an identical copy
        ps->m_pageType = type;

        ps->next = next;
        next = ps;
    }

    return ps;
}

void RenderPageStyle::removePageStyle(PageType type)
{
    RenderPageStyle *ps = next;
    RenderPageStyle *prev = this;

    while (ps) {
        if (ps->m_pageType==type) {
            prev->next = ps->next;
            delete ps;
            return;
        }
        prev = ps;
        ps = ps->next;
    }
}
