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


#if 0
StyleBoxData::StyleBoxData(const StyleBoxData &other)
    : DomShared()
{
    position = other.position;
    floating = other.floating;

    width = other.width;
    height = other.height;

    min_width = other.min_width;
    max_width = other.max_width;

    min_height = other.min_height;
    max_height = other.max_height;

    offset = 0;
    margin = 0;
    padding = 0;
    border = 0;
    if(other.offset)
	offset = new LengthBox(*other.offset);
    if(other.margin)
	margin = new LengthBox(*other.margin);
    if(other.padding)
	padding = new LengthBox(*other.padding);
    if(other.border)
	border = new BorderData(*other.border);
}

const StyleBoxData &StyleBoxData::operator = (const StyleBoxData &other)
{
    position = other.position;
    floating = other.floating;

    width = other.width;
    height = other.height;

    min_width = other.min_width;
    max_width = other.max_width;

    min_height = other.min_height;
    max_height = other.max_height;

    delete offset;
    delete margin;
    delete padding;
    delete border;
    offset = 0;
    margin = 0;
    padding = 0;
    border = 0;
    if(other.offset)
	offset = new LengthBox(*other.offset);
    if(other.margin)
	margin = new LengthBox(*other.margin);
    if(other.padding)
	padding = new LengthBox(*other.padding);
    if(other.border)
	border = new BorderData(*other.border);

    return *this;
}

#endif


RenderStyle::RenderStyle()
{
    if (!_default)
    {
    	box.createData();
    	box.set()->setDefaultValues();
    	text.createData();
    	text.set()->setDefaultValues();
    	visual.createData();
     	background.createData();
    	list.createData();
    	table.createData();
    	surround.createData();
	_default = this;
    }
    else
    {
    	box = _default->box;
    	text = _default->text;
    	visual = _default->visual;
    	background = _default->background;
    	list = _default->list;
    	table = _default->table;
    	surround = _default->surround;
    }

    _display = INLINE;
}


RenderStyle::RenderStyle(const RenderStyle& other)
{
    box = other.box;
    text = other.text;
    visual = other.visual;
    background = other.background;
    list = other.list;
    table = other.table;
    surround = other.surround;
    text = other.text;
    
    _display = INLINE;
}

RenderStyle::RenderStyle(const RenderStyle* inheritParent)
{
    box = _default->box;
    text = _default->text;
    visual = _default->visual;
    surround = _default->surround;
    background = _default->background;

    text = inheritParent->text;
    list = inheritParent->list;
    table = inheritParent->table;

    _display = INLINE;
}

RenderStyle::~RenderStyle()
{
}

RenderStyle*
RenderStyle::inheritFrom(RenderStyle* from)
{
    if(!from) return this;
    text = from->text;
    list = from->list;
    table = from->table;

#if 0
    // text-decoration and text-shadow need special treatment
    // since they inherit from the last block element
    RenderStyle *parentBlock = from;
    while(parentBlock && parentBlock->display() != BLOCK)
	parentBlock = parentBlock->parentStyle;
    if(parentBlock)
    {
	setTextDecoration(parentBlock->textDecoration());
	setTextDecorationColor(parentBlock->textDecorationColor());
    }
#endif


    return this;
}

RenderStyle* RenderStyle::_default = 0;
