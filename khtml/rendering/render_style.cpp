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

StyleVisualData::StyleVisualData()
{
    colspan = 1;
    palette = QApplication::palette();
}

StyleVisualData::~StyleVisualData() {
}

StyleVisualData::StyleVisualData(const StyleVisualData& o ) : SharedData()
{
    clip = o.clip;
    colspan = o.colspan;
    counter_increment = o.counter_increment;
    counter_reset = o.counter_reset;
    palette = o.palette;
}


void
RenderStyle::setBitDefaults()
{
    inherited_flags._border_collapse = true;
    inherited_flags._empty_cells = SHOW;
    inherited_flags._caption_side = CAPTOP;
    inherited_flags._list_style_type = DISC;
    inherited_flags._list_style_position = OUTSIDE;
    inherited_flags._visiblity = VISIBLE;
    inherited_flags._text_align = JUSTIFY;
    inherited_flags._text_transform = TTNONE;
    inherited_flags._direction = LTR;
    inherited_flags._white_space = NORMAL;
    inherited_flags._text_decoration = TDNONE;
    inherited_flags._cursor_style = CURSOR_AUTO;
    inherited_flags._font_variant = FVNORMAL;

    _visuallyOrdered = false;
    _htmlHacks=false;

    _display = INLINE;
    
    _overflow = OVISIBLE;
    _vertical_align = BASELINE;
    _clear = CNONE;
    _table_layout = TAUTO;
    _bg_repeat = REPEAT;
    _bg_attachment = SCROLL;
    _position = STATIC;
    _floating = FNONE;

    _noLineBreak=false;
    _flowAroundFloats=false;

    _styleType = NOPSEUDO;
    _hasHover = false;
    _hasFocus = false;
    _hasActive = false;
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

}

RenderStyle::RenderStyle(bool)
{
    setBitDefaults();

    box.init();
    box.access()->setDefaultValues();
    visual.init();
    background.init();
    surround.init();

    inherited.init();
    inherited.access()->setDefaultValues();

    pseudoStyle = 0;
}

RenderStyle::RenderStyle(const RenderStyle& other)
    : DOM::DomShared() // shut up, compiler
{

    inherited_flags = other.inherited_flags;

    _visuallyOrdered = other._visuallyOrdered;
    _htmlHacks = other._htmlHacks;

    _display = other._display;

    
    _overflow = other._overflow;
    _vertical_align = other._vertical_align;
    _clear = other._clear;
    _table_layout = other._table_layout;
    _bg_repeat = other._bg_repeat;
    _bg_attachment = other._bg_attachment;
    _position = other._position;
    _floating = other._floating;

    _noLineBreak = other._noLineBreak;
    _flowAroundFloats = other._flowAroundFloats;

    _styleType=NOPSEUDO;
    _hasHover = other._hasHover;
    _hasFocus = other._hasFocus;
    _hasActive = other._hasActive;

    box = other.box;
    visual = other.visual;
    background = other.background;
    surround = other.surround;

    inherited = other.inherited;

    pseudoStyle=0;
}

void RenderStyle::inheritFrom(const RenderStyle* inheritParent)
{

    inherited = inheritParent->inherited;
    inherited_flags = inheritParent->inherited_flags;

    _visuallyOrdered = inheritParent->_visuallyOrdered;
    _htmlHacks = inheritParent->_htmlHacks;
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
}

bool RenderStyle::operator==(const RenderStyle& o) const
{
// compare everything except the pseudoStyle pointer
    return (*box.get() == *o.box.get() &&
            *visual.get() == *o.visual.get() &&
            *background.get() == *o.background.get() &&
            *surround.get() == *o.surround.get() &&
            *inherited.get() == *o.inherited.get() &&
	    inherited_flags == o.inherited_flags &&
            _display == o._display &&
            _visuallyOrdered == o._visuallyOrdered &&
            _htmlHacks == o._htmlHacks &&
            _overflow == o._overflow &&
            _vertical_align == o._vertical_align &&
            _clear == o._clear &&
            _table_layout && o._table_layout &&
            _bg_repeat == o._bg_repeat &&
            _bg_attachment == o._bg_attachment &&
            _position == o._position &&
            _floating == o._floating &&
            _flowAroundFloats == o._flowAroundFloats &&
            _styleType == o._styleType);
}

RenderStyle* RenderStyle::getPseudoStyle(PseudoId pid)
{

    if (!(_styleType==NOPSEUDO))
        return 0;

    RenderStyle *ps = pseudoStyle;

    while (ps) {
        if (ps->_styleType==pid)
            return ps;

        ps = ps->pseudoStyle;
    }

    return 0;
}

RenderStyle* RenderStyle::addPseudoStyle(PseudoId pid)
{
    RenderStyle *ps = getPseudoStyle(pid);

    if (!ps)
    {
        if (pid==BEFORE || pid==AFTER)
            ps = new RenderPseudoElementStyle();
        else
            ps = new RenderStyle(*this); // use the real copy constructor to get an identical copy
        ps->ref();
        ps->_styleType = pid;
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
        if (ps->_styleType==pid) {
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

	    _visuallyOrdered != other->_visuallyOrdered ||
	    _htmlHacks != other->_htmlHacks ||
	    
	    *inherited.get() != *other->inherited.get() 
	    );
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

RenderPseudoElementStyle::RenderPseudoElementStyle() : RenderStyle()
{
    _contentType = CONTENT_NONE;
}

RenderPseudoElementStyle::RenderPseudoElementStyle(bool b) : RenderStyle(b)
{
    _contentType = CONTENT_NONE;
}
RenderPseudoElementStyle::RenderPseudoElementStyle(const RenderStyle& r) : RenderStyle(r)
{
    _contentType = CONTENT_NONE;
}

RenderPseudoElementStyle::~RenderPseudoElementStyle() { clearContent(); }


void RenderPseudoElementStyle::setContent(CachedObject* o)
{
    clearContent();
//    o->ref();
    _content.object = o;
    _contentType = CONTENT_OBJECT;
}

void RenderPseudoElementStyle::setContent(DOM::DOMStringImpl* s)
{
    clearContent();
    _content.text = s;
    _content.text->ref();
    _contentType = CONTENT_TEXT;
}

DOM::DOMStringImpl* RenderPseudoElementStyle::contentText()
{
    if (_contentType==CONTENT_TEXT)
        return _content.text;
    else
        return 0;
}

CachedObject* RenderPseudoElementStyle::contentObject()
{
    if (_contentType==CONTENT_OBJECT)
        return _content.object;
    else
        return 0;
}


void RenderPseudoElementStyle::clearContent()
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
