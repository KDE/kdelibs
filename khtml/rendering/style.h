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
#ifndef STYLE_H
#define STYLE_H

#include <qcolor.h>
#include <qfont.h>
#include <qpixmap.h>
#include <dom_string.h>
#include <dom_misc.h>
#include <khtmllayout.h>

#include <stdio.h>

namespace DOM {

template <class DATA>
class DataRef
{
public:

    DataRef()
    {
	data=0;
    }

    ~DataRef()
    {
    	if(data) data->deref();
    }

    const DATA* operator->() const
    {    	
    	return get();
    }



    const DATA* get() const
    {
//    	printf("get\n");
	return data;
    }

    DATA* set()
    {    	
//    	printf("set\n");
    	if (!data->hasOneRef())
	{
//	    printf("copy\n");
	    data->deref();
	    data = new DATA(*data);
	    data->ref();
	}
	return data;
    }

    void createData()
    {
    	if (data)
    	    data->deref();
    	data = new DATA;
	data->ref();
    }

    DataRef<DATA>& operator=(const DataRef<DATA>& d)
    {
    	if (data)
    	    data->deref();
    	data = d.data;
	data->ref();
	return *this;
    }

private:
    DATA* data;
};


//------------------------------------------------
//------------------------------------------------

struct LengthBox
{
    Length left;
    Length right;
    Length top;
    Length bottom;
    Length& operator=(Length& len)
    {
    	left=len;
	right=len;
	top=len;
	bottom=len;
	return len;
    }
};



enum EPosition {
    STATIC, RELATIVE, ABSOLUTE, FIXED
};

enum EFloat {
    FNONE, FLEFT, FRIGHT
};

class StyleBoxData : public DomShared
{
public:

    EPosition position;

    EFloat floating;

    Length width;
    Length height;

    Length min_width;
    Length max_width;

    Length min_height;
    Length max_height;
    	
    Length left;
    Length right;
    Length top;
    Length bottom;

    LengthBox margin;
    LengthBox padding;

};


//------------------------------------------------


enum EBorderStyle {
    BNONE, BHIDDEN, DOTTED, DASHED, SOLID, DOUBLE,
    GROOVE, RIDGE, INSET, OUTSET
};


struct BorderValue
{
    Length width;
    EBorderStyle style;
    QColor color;

};


class StyleBorderData : public DomShared
{
public:
    BorderValue left;
    BorderValue right;
    BorderValue top;
    BorderValue bottom;

};

//------------------------------------------------

enum EOverflow {
    OVISIBLE, OHIDDEN, SCROLL, AUTO
};

enum EVisiblity {
    VISIBLE, HIDDEN, COLLAPSE
};

enum EVerticalAlign {
    BASELINE, SUB, SUPER, TOP, TEXT_TOP, MIDDLE,
    BOTTOM, TEXT_BOTTOM
};


class StyleVisualData : public DomShared
{
public:
    EOverflow overflow;

    LengthBox clip;

    EVisiblity visiblity;

    EVerticalAlign vertical_align;

};

//------------------------------------------------
enum EBackgroundRepeat {
    REPEAT, REPEAT_X, REPEAT_Y, NO_REPEAT
};

enum EBackgroundAttachment{
    BGSCROLL, BGFIXED
};


class StyleBackgroundData : public DomShared
{
public:
    QColor color;
    QPixmap image;

    EBackgroundRepeat repeat;
    EBackgroundAttachment attachment;

    Length x_position;
    Length y_position;
};

//------------------------------------------------

enum EWhiteSpace {
    NORMAL, PRE, NOWRAP
};

enum ETextAlign{
    LEFT, RIGHT, CENTER, JUSTIFY
};

enum EDirection{
    LTR, RTL
};

enum EClear{
    CNONE, CLEFT, CRIGHT, CBOTH
};


class StyleTextData : public DomShared
{
public:
    QFont font;
    QColor color;

    Length indent;
    ETextAlign align;
    EDirection direction;
    Length line_height;

    EWhiteSpace white_space;
    EClear clear;

};

//------------------------------------------------

enum ETableLayout {
    TAUTO, TFIXED
};

enum EBorderCollapse {
    BCOLLAPSE, SEPARATE
};

enum EEmptyCell {
    SHOW, HIDE
};

class StyleTableData : public DomShared
{
public:
    ETableLayout layout;

    EBorderCollapse border_collapse;
    Length border_spacing;

    EEmptyCell empty_cells;

};

class StyleListData : public DomShared
{
public:


};

//------------------------------------------------
//------------------------------------------------

enum EDisplay {
    INLINE, BLOCK, LIST_ITEM, RUN_IN, COMPACT,
    MARKER, TABLE, INLINE_TABLE, TABLE_ROW_GROUP,
    TABLE_HEADER_GROUP, TABLE_FOOTER_GROUP, TABLE_ROW,
    TABLE_COLUMN_GROUP, TABLE_COLUMN, TABLE_CELL,
    TABLE_CAPTION, NONE
};

class Style
{
protected:

    EDisplay _display;

    DataRef<StyleBoxData> box;
    DataRef<StyleBorderData> border;
    DataRef<StyleTextData> text;
    DataRef<StyleVisualData> visual;
    DataRef<StyleBackgroundData> background;
    DataRef<StyleListData> list;
    DataRef<StyleTableData> table;

public:
    Style();
    Style(const Style&);
    ~Style();

    EDisplay 	display() { return _display; }

    Length  	left() { return box->left; }
    Length  	right() { return box->right; }
    Length  	top() { return box->top; }
    Length  	bottom() { return box->bottom; }

    EPosition 	position() { return box->position; }
    EFloat  	floating() { return box->floating; }

    Length  	width() { return box->width; }
    Length  	height() { return box->height; }
    Length  	minWidth() { return box->min_width; }
    Length  	maxWidth() { return box->max_width; }

    Length  	    borderLeftWidth() { return border->left.width; }
    EBorderStyle    borderLeftStyle() { return border->left.style; }
    QColor  	    borderLeftColor() { return border->left.color; }
    Length  	    borderRightWidth() { return border->right.width; }
    EBorderStyle    borderRightStyle() { return border->right.style; }
    QColor  	    borderRightColor() { return border->right.color; }
    Length  	    borderTopWidth() { return border->top.width; }
    EBorderStyle    borderTopStyle() { return border->top.style; }
    QColor  	    borderTopColor() { return border->top.color; }
    Length  	    borderBottomWidth() { return border->bottom.width; }
    EBorderStyle    borderBottomStyle() { return border->bottom.style; }
    QColor  	    borderBottomColor() { return border->bottom.color; }

    EOverflow overflow() { return visual->overflow; }
    EVisiblity visiblity() { return visual->visiblity; }
    EVerticalAlign verticalAlign() { return visual->vertical_align; }

    Length clipLeft() { return visual->clip.left; }
    Length clipRight() { return visual->clip.right; }
    Length clipTop() { return visual->clip.top; }
    Length clipBottom() { return visual->clip.bottom; }

    QFont font() { return text->font; }
    QColor color() { return text->color; }
    Length textIndent() { return text->indent; }
    ETextAlign textAlign() { return text->align; }

    EDirection direction() { return text->direction; }
    Length lineHeight() { return text->line_height; }

    EWhiteSpace whiteSpace() { return text->white_space; }
    EClear clear() { return text->clear; }

    QColor backgroundColor() { return background->color; }
    QPixmap backgroundImage() { return background->image; }
    EBackgroundRepeat backgroundRepeat() { return background->repeat; }
    EBackgroundAttachment backgroundAttachment() { return background->attachment; }
    Length backgroundXPosition() { return background->x_position; }
    Length backgroundYPosition() { return background->y_position; }

    ETableLayout tableLayout() { return table->layout; }
    EBorderCollapse borderCollapse() { return table->border_collapse; }
    Length borderSpacing() { return table->border_spacing; }
    EEmptyCell emptyCells() { return table->empty_cells; }


    void setDisplay(EDisplay v) { _display = v; }
    void setPosition(EPosition v) { box.set()->position = v; }
    void setFloating(EFloat v) { box.set()->floating = v; }

    void setRight(Length v) { box.set()->right = v; }
    void setLeft(Length v) { box.set()->left = v; }
    void setTop(Length v) { box.set()->top = v; }
    void setBottom(Length v) { box.set()->bottom = v; }

    void setWidth(Length v) { box.set()->width = v; }
    void setHeight(Length v) { box.set()->height = v; }
    void setMinWidth(Length v) { box.set()->min_width = v; }
    void setMaxWidth(Length v) { box.set()->max_width = v; }
    void setMinHeight(Length v) { box.set()->min_height = v; }
    void setMaxHeight(Length v) { box.set()->max_height = v; }

    void setBorderWidth(Length v) {
    	StyleBorderData* d = border.set();
	d->left.width = v;
    	d->right.width = v;
    	d->top.width = v;	
    	d->bottom.width = v;
    }	
    void setBorderColor(QColor v) {
    	StyleBorderData* d = border.set();
    	d->left.color = v;
    	d->right.color = v;
    	d->top.color = v;	
    	d->bottom.color = v;
    }	
    void setBorderStyle(EBorderStyle v) {
    	StyleBorderData* d = border.set();
    	d->left.style = v;
    	d->right.style = v;
    	d->top.style = v;	
    	d->bottom.style = v;
    }	
    void setBorderLeftWidth(Length v) { border.set()->left.width = v; }
    void setBorderLeftStyle(EBorderStyle v) { border.set()->left.style = v; }
    void setBorderLeftColor(QColor v) { border.set()->left.color = v; }
    void setBorderRightWidth(Length v) { border.set()->right.width = v; }
    void setBorderRightStyle(EBorderStyle v) { border.set()->right.style = v; }
    void setBorderRightColor(QColor v) { border.set()->right.color = v; }
    void setBorderTopWidth(Length v) { border.set()->top.width = v; }
    void setBorderTopStyle(EBorderStyle v) { border.set()->top.style = v; }
    void setBorderTopColor(QColor v) { border.set()->top.color = v; }
    void setBorderBottomWidth(Length v) { border.set()->bottom.width = v; }
    void setBorderBottomStyle(EBorderStyle v) { border.set()->bottom.style = v; }
    void setBorderBottomColor(QColor v) { border.set()->bottom.color = v; }

    void setOverflow(EOverflow v) { visual.set()->overflow = v; }
    void setVisiblity(EVisiblity v) { visual.set()->visiblity = v; }
    void setVerticalAlign(EVerticalAlign v) { visual.set()->vertical_align = v; }

    void setClip(Length v) {
    	StyleVisualData* d = visual.set();
	d->clip.left = v;
	d->clip.right = v;
	d->clip.top = v;
	d->clip.bottom = v;
    }	
    void setClipLeft(Length v) { visual.set()->clip.left = v; }
    void setClipRight(Length v) { visual.set()->clip.right = v; }
    void setClipTop(Length v) { visual.set()->clip.top = v; }
    void setClipBottom(Length v) { visual.set()->clip.bottom = v; }

    void setFont(QFont v) { text.set()->font = v; }
    void setColor(QColor v) { text.set()->color = v; }
    void setTextIndent(Length v) { text.set()->indent = v; }
    void setTextAlign(ETextAlign v) { text.set()->align = v; }
    void setDirection(EDirection v) { text.set()->direction = v; }
    void setLineHeight(Length v) { text.set()->line_height = v; }

    void setWhiteSpace(EWhiteSpace v) { text.set()->white_space = v; }
    void setClear(EClear v) { text.set()->clear = v; }

    void setBackgroundColor(QColor v) {  background.set()->color = v; }
    void setBackgroundImage(QPixmap v) {  background.set()->image = v; }
    void setBackgroundRepeat(EBackgroundRepeat v) {  background.set()->repeat = v; }
    void setBackgroundAttachment(EBackgroundAttachment v) {  background.set()->attachment = v; }
    void setBackgroundXPosition(Length v) {  background.set()->x_position = v; }
    void setBackgroundYPosition(Length v) {  background.set()->y_position = v; }


};

} // namespace

#endif

