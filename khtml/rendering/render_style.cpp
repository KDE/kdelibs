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

#include "render_style.h"

using namespace khtml;

const QColor RenderStyle::undefinedColor;

StyleSurroundData::StyleSurroundData()
{
    margin.left = Length(0,Fixed);
    margin.right = Length(0,Fixed);
    margin.top = Length(0,Fixed);
    margin.bottom = Length(0,Fixed);
    padding.left = Length(0,Fixed);
    padding.right = Length(0,Fixed);
    padding.top = Length(0,Fixed);
    padding.bottom = Length(0,Fixed);    
}

StyleSurroundData::StyleSurroundData(const StyleSurroundData& o ) 
        : SharedData()
{ 
    offset = o.offset;
    margin = o.margin;
    padding = o.padding;
    border = o.border;	
}

bool StyleSurroundData::operator==(const StyleSurroundData& o) const
{
    return offset==o.offset && margin==o.margin &&
	padding==o.padding && border==o.border;
}

StyleBoxData::StyleBoxData()
{
}

StyleBoxData::StyleBoxData(const StyleBoxData& o ) 
        : SharedData()
{
    width = o.width;
    height = o.height;
    min_width = o.min_width;
    max_width = o.max_width;
    min_height = o.min_height;
    max_height = o.max_height;	
    z_index = o.z_index;
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



void
RenderStyle::setBitDefaults()
{
    _border_collapse = true;
    _empty_cells = SHOW;
    _caption_side = CAPTOP;
    _list_style_type = DISC;
    _list_style_position = OUTSIDE;
    _visiblity = VISIBLE;
    _text_align = JUSTIFY;
    _direction = LTR;
    _text_decoration = TDNONE;
    _white_space = NORMAL;

    _vertical_align = BASELINE;
    _clear = CNONE;
    _overflow = OVISIBLE;
    _table_layout = TAUTO;
    _position = STATIC;
    _floating = FNONE;
    _bg_repeat = REPEAT;
    _bg_attachment = SCROLL;

    _visuallyOrdered = false;
    _direction = LTR;

    _htmlHacks=false;
    _flowAroundFloats=false;
}



RenderStyle::RenderStyle()
{
    counter++;
    if (!_default)
	_default = new RenderStyle(true);

    box = _default->box;    	
    visual = _default->visual;
    background = _default->background;
    surround = _default->surround;

    inherited = _default->inherited;

    setBitDefaults();

    _display = INLINE;
}

RenderStyle::RenderStyle(bool)
{
    box.createData();
    box.set()->setDefaultValues();    	
    visual.createData();
    background.createData();
    surround.createData();		

    inherited.createData();
    inherited.set()->setDefaultValues();		
}

RenderStyle::RenderStyle(const RenderStyle& other)
{
    counter++;
    box = other.box;
    visual = other.visual;
    background = other.background;
    surround = other.surround;

    inherited = other.inherited;

    _border_collapse = other._border_collapse;
    _empty_cells = other._empty_cells;
    _caption_side = other._caption_side;
    _list_style_type = other._list_style_type;
    _list_style_position = other._list_style_position;
    _visiblity = other._visiblity;
    _text_align = other._text_align;
    _direction = other._direction;
    _text_decoration = other._text_decoration;
    _white_space = other._white_space;

    _vertical_align = other._vertical_align;
    _clear = other._clear;
    _overflow = other._overflow;
    _table_layout = other._table_layout;
    _position = other._position;
    _floating = other._floating;
    _bg_repeat = other._bg_repeat;
    _bg_attachment = other._bg_attachment;
    _visuallyOrdered = other._visuallyOrdered;

    _display = other._display;

    _htmlHacks = other._htmlHacks;
    _flowAroundFloats = other._flowAroundFloats;
}

RenderStyle::RenderStyle(const RenderStyle* inheritParent)
{
    counter++;
    box = _default->box;
    visual = _default->visual;
    surround = _default->surround;
    background = _default->background;

    inherited = inheritParent->inherited;

    setBitDefaults();

    _border_collapse = inheritParent->_border_collapse;
    _empty_cells = inheritParent->_empty_cells;
    _caption_side = inheritParent->_caption_side;
    _list_style_type = inheritParent->_list_style_type;
    _list_style_position = inheritParent->_list_style_position;
    _visiblity = inheritParent->_visiblity;
    _text_align = inheritParent->_text_align;
    _direction = inheritParent->_direction;
    _text_decoration = inheritParent->_text_decoration;
    _white_space = inheritParent->_white_space;
    _visuallyOrdered = inheritParent->_visuallyOrdered;

    _htmlHacks = inheritParent->_htmlHacks;

    _display = INLINE;
}

RenderStyle::~RenderStyle()
{
    counter--;
}

bool RenderStyle::operator==(const RenderStyle& other) const
{
    return
        *box.get() == *other.box.get() &&
        *visual.get() == *other.visual.get() &&
    	*background.get() == *other.background.get() &&
        *surround.get() == *other.surround.get() &&
	
    	*inherited.get() == *other.inherited.get() &&
        _display == other._display;
}

void RenderStyle::mergeData(RenderStyle* other)
{
    if ( box.get()!=other->box.get() &&
    	*box.get() == *other->box.get()) {
    	box=other->box;
//	kdDebug( 6040 ) << "STYLE box merge " << endl;
	}
    if (visual.get()!=other->visual.get() &&
    	*visual.get() == *other->visual.get()) {
    	visual=other->visual;
//	kdDebug( 6040 ) << "STYLE visual merge " << endl;
	}
    if (background.get()!=other->background.get() &&
     	*background.get() == *other->background.get()) {
    	background=other->background;
//	kdDebug( 6040 ) << "STYLE bg merge " << endl;
	}
    if (surround.get()!=other->surround.get() &&
    	*surround.get() == *other->surround.get()) {
    	surround=other->surround;
//	kdDebug( 6040 ) << "STYLE surround merge " << endl;
	}
    if (inherited.get()!=other->inherited.get() &&
    	*inherited.get() == *other->inherited.get()) {
    	inherited=other->inherited;
//	kdDebug( 6040 ) << "STYLE text merge " << endl;
	}

}

RenderStyle*
RenderStyle::inheritFrom(RenderStyle* from)
{
    if(!from) return this;
    inherited = from->inherited;

    return this;
}

RenderStyle* RenderStyle::_default = 0;
int RenderStyle::counter = 0;
int SharedData::counter = 0;
