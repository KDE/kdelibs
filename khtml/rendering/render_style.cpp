/*
 * This file is part of the DOM implementation for KDE.
 *
 * Copyright (C) 1999 Antti Koivisto (koivisto@kde.org)
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

#include "xml/dom_stringimpl.h"

#include "render_style.h"

#include "kdebug.h"

using namespace khtml;

StyleSurroundData::StyleSurroundData()
    : margin( Fixed ), padding( Fixed )
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
    : z_index( ZAUTO )
{
}

StyleBoxData::StyleBoxData(const StyleBoxData& o )
    : Shared<StyleBoxData>(),
      width( o.width ), height( o.height ),
      min_width( o.min_width ), max_width( o.max_width ), 
      min_height ( o.min_height ), max_height( o.max_height ),
      z_index( o.z_index )
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
	    z_index == o.z_index;
}

StyleVisualData::StyleVisualData()
    : colspan( 1 ), counter_increment( 0 ), counter_reset( 0 ),
      palette( QApplication::palette() )
{
}

StyleVisualData::~StyleVisualData() {
}

StyleVisualData::StyleVisualData(const StyleVisualData& o ) 
    : Shared<StyleVisualData>(),
      clip( o.clip ), colspan( o.colspan ),
      counter_increment( o.counter_increment ), counter_reset( o.counter_reset ),
      palette( o.palette )
{
}



StyleBackgroundData::StyleBackgroundData()
    : image( 0 )
{
}

StyleBackgroundData::StyleBackgroundData(const StyleBackgroundData& o ) 
    : Shared<StyleBackgroundData>(),
      color( o.color ), image( o.image ),
      x_position( o.x_position ), y_position( o.y_position ),
      outline( o.outline )
{
}

bool StyleBackgroundData::operator==(const StyleBackgroundData& o) const
{
    return
	color == o.color &&
	image == o.image &&
	x_position == o.x_position &&
	y_position == o.y_position &&
	outline == o.outline;
}



StyleInheritedData::StyleInheritedData()
    : indent( Fixed ), line_height( -100, Percent ), style_image( 0 ),
      cursor_image( 0 ), font(), color( Qt::black ), decoration_color( Qt::black ), border_spacing( 0 )
{
}

StyleInheritedData::~StyleInheritedData() 
{ 
}

StyleInheritedData::StyleInheritedData(const StyleInheritedData& o )
    : Shared<StyleInheritedData>(),
      indent( o.indent ), line_height( o.line_height ), style_image( o.style_image ),
      cursor_image( o.cursor_image ), font( o.font ), 
      color( o.color ), decoration_color( o.decoration_color ),
      border_spacing( o.border_spacing )
{
}

bool StyleInheritedData::operator==(const StyleInheritedData& o) const
{
    return 
	indent == o.indent &&
	line_height == o.line_height &&
	border_spacing == o.border_spacing &&
	style_image == o.style_image &&
	cursor_image == o.cursor_image &&
	font == o.font &&
	color == o.color &&
	decoration_color == o.decoration_color;

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

    inherited = _default->inherited;

    setBitDefaults();

    pseudoStyle = 0;
    content = 0;
}

RenderStyle::RenderStyle(bool)
{
    setBitDefaults();

    box.init();
    visual.init();
    background.init();
    surround.init();

    inherited.init();

    pseudoStyle = 0;
    content = 0;
}

RenderStyle::RenderStyle(const RenderStyle& o)
    : Shared<RenderStyle>(),
      inherited_flags( o.inherited_flags ), noninherited_flags( o.noninherited_flags ),
      box( o.box ), visual( o.visual ), background( o.background ), surround( o.surround ),
      inherited( o.inherited ), pseudoStyle( 0 ), content( o.content )
{
}

void RenderStyle::inheritFrom(const RenderStyle* inheritParent)
{
    inherited = inheritParent->inherited;
    inherited_flags = inheritParent->inherited_flags;
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
    if ( content )
	delete content;
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
            inherited == o.inherited);
}

RenderStyle* RenderStyle::getPseudoStyle(PseudoId pid)
{
    RenderStyle *ps = 0;
    if (noninherited_flags._styleType==NOPSEUDO) {
	ps = pseudoStyle;
    while (ps) {
        if (ps->noninherited_flags._styleType==pid)
		break;

        ps = ps->pseudoStyle;
    }
    }
    return ps;
}

RenderStyle* RenderStyle::addPseudoStyle(PseudoId pid)
{
    RenderStyle *ps = getPseudoStyle(pid);

    if (!ps)
    {
            ps = new RenderStyle(*this); // use the real copy constructor to get an identical copy
        ps->ref();
        ps->noninherited_flags._styleType = pid;
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
        if (ps->noninherited_flags._styleType==pid) {
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
	    inherited != other->inherited
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
    Diff d = Equal;

    // we anyway assume they are the same
// 	EDisplay _display : 5;

    // NonVisible:
// 	ECursor _cursor_style : 4;

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
	 *inherited.get() != *other->inherited.get()
	)
	d = CbLayout;

    if ( d == CbLayout )
	return d;

    // changes causing Layout changes:

// only for tables:
// 	_border_collapse
// 	EEmptyCell _empty_cells : 2 ;
// 	ECaptionSide _caption_side : 2;
//     ETableLayout _table_layout : 1;
//     EPosition _position : 2;
//     EFloat _floating : 2;
    if ( ((int)noninherited_flags._display) >= TABLE ) {
	// Stupid gcc gives a compile error on
	// a != other->b if a and b are bitflags. Using
	// !(a== other->b) instead.
	if ( !(inherited_flags._border_collapse == other->inherited_flags._border_collapse) ||
	     !(inherited_flags._empty_cells == other->inherited_flags._empty_cells) ||
	     !(inherited_flags._caption_side == other->inherited_flags._caption_side) ||
	     !(noninherited_flags._table_layout == other->noninherited_flags._table_layout) ||
	     !(noninherited_flags._position == other->noninherited_flags._position) ||
	     !(noninherited_flags._floating == other->noninherited_flags._floating) )

	    d = CbLayout;
    }

// only for lists:
// 	EListStyleType _list_style_type : 5 ;
// 	EListStylePosition _list_style_position :1;
    if (noninherited_flags._display == LIST_ITEM ) {
	if ( !(inherited_flags._list_style_type == other->inherited_flags._list_style_type) ||
	     !(inherited_flags._list_style_position == other->inherited_flags._list_style_position) )
	    d = Layout;
    }

    if ( d == Layout )
	return d;

// ### These could be better optimised
// 	ETextAlign _text_align : 3;
// 	ETextTransform _text_transform : 4;
// 	EDirection _direction : 1;
// 	EWhiteSpace _white_space : 2;
// 	EFontVariant _font_variant : 1;
//     EClear _clear : 2;
    if ( !(inherited_flags._text_align == other->inherited_flags._text_align) ||
	 !(inherited_flags._text_transform == other->inherited_flags._text_transform) ||
	 !(inherited_flags._direction == other->inherited_flags._direction) ||
	 !(inherited_flags._white_space == other->inherited_flags._white_space) ||
	 !(inherited_flags._font_variant == other->inherited_flags._font_variant) ||
	 !(noninherited_flags._clear == other->noninherited_flags._clear)
	)
	d = Layout;

// only for inline:
//     EVerticalAlign _vertical_align : 4;

    if ( !(noninherited_flags._display == INLINE) ) {
	if ( !(noninherited_flags._vertical_align == other->noninherited_flags._vertical_align))
	    d = Layout;
    }

    if ( d == Layout )
	return d;


    // Visible:
// 	EVisibility _visibility : 2;
//     EOverflow _overflow : 4 ;
//     EBackgroundRepeat _bg_repeat : 2;
//     bool _bg_attachment : 1;
// 	int _text_decoration : 4;
//     DataRef<StyleBackgroundData> background;
    if ( !(inherited_flags._visibility == other->inherited_flags._visibility) ||
	 !(noninherited_flags._overflow == other->noninherited_flags._overflow) ||
	 !(noninherited_flags._bg_repeat == other->noninherited_flags._bg_repeat) ||
	 !(noninherited_flags._bg_attachment == other->noninherited_flags._bg_attachment) ||
	 !(noninherited_flags._jsClipMode == other->noninherited_flags._jsClipMode) ||
	 !(inherited_flags._text_decoration == other->inherited_flags._text_decoration) ||
	 *background.get() != *other->background.get()
	)
	d = Visible;

    // Position:

    return d;
}


RenderStyle* RenderStyle::_default = 0;
//int RenderStyle::counter = 0;
//int SharedData::counter = 0;

void RenderStyle::cleanup()
{
    delete _default;
    _default = 0;
//    counter = 0;
//    SharedData::counter = 0;
}

void RenderStyle::setContent(CachedObject* o)
{
    if ( !content ) 
	content = new ContentData;
    else
	content->clearContent();
//    o->ref();
    content->_content.object = o;
    content->_contentType = CONTENT_OBJECT;
}


void RenderStyle::setPaletteColor(QPalette::ColorGroup g, QColorGroup::ColorRole r, const QColor& c)
{
    visual.access()->palette.setColor(g,r,c);
}

void RenderStyle::setClip( Length top, Length right, Length bottom, Length left )
{
    StyleVisualData *data = visual.access();
    data->clip.top = top;
    data->clip.right = right;
    data->clip.bottom = bottom;
    data->clip.left = left;
}

void RenderStyle::setContent(DOM::DOMStringImpl* s)
{
    if ( !content ) 
	content = new ContentData;
    else
	content->clearContent();

    content->_content.text = s;
    content->_content.text->ref();
    content->_contentType = CONTENT_TEXT;
}

ContentData::~ContentData()
{
    clearContent();
}

void ContentData::clearContent()
{
    switch (_contentType)
    {
        case CONTENT_OBJECT:
//            _content.object->deref();
            _content.object = 0;
            break;
        case CONTENT_TEXT:
            _content.text->deref();
            _content.text = 0;
        default:
            ;
    }

}
