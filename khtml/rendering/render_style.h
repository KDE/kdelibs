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
#ifndef RENDERSTYLE_H
#define RENDERSTYLE_H

#include <qcolor.h>
#include <qfont.h>
#include <dom/dom_string.h>
#include <dom/dom_misc.h>
#include <khtmllayout.h>

#include <stdio.h>

namespace khtml {

    class CachedImage;

template <class DATA>
class DataRef
{
public:

    DataRef()
    {
	data=0;
    }

    virtual ~DataRef()
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
//    	if (data==0)
//	    createData();
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
// Box model attributes. Not inherited.

struct LengthBox
{
    LengthBox()
    {
	left = Length(0, Fixed);
	right = Length(0, Fixed);
	top = Length(0, Fixed);
	bottom = Length(0, Fixed);
    }

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

    bool nonZero() const { return left.value!=0 || right.value!=0 || top.value!=0 || bottom.value!=0; }
};



enum EPosition {
    STATIC, RELATIVE, ABSOLUTE, FIXED
};

// CENTER is not part of CSS, but we need it to get <table align=center> and the <center>
// element right
enum EFloat {
    FNONE, FLEFT, FRIGHT, FCENTER
};


//------------------------------------------------
// Border attributes. Not inherited.


enum EBorderStyle {
    BNONE, BHIDDEN, DOTTED, DASHED, SOLID, DOUBLE,
    GROOVE, RIDGE, INSET, OUTSET
};


class BorderValue
{
public:
    BorderValue()
    {
	width = 3; // medium is default value
	style = BNONE;
    }
    unsigned short width;
    EBorderStyle style;
    QColor color;

    bool nonZero() const { return width!=0 && style!=BNONE; }

};

class BorderData : public DOM::DomShared
{
public:
    BorderValue left;
    BorderValue right;
    BorderValue top;
    BorderValue bottom;

    bool hasBorder() const
    {
    	return left.nonZero() || right.nonZero() || top.nonZero() || bottom.nonZero();	
    }

};

class StyleSurroundData : public DOM::DomShared
{
public:
    StyleSurroundData()
    {
    }
    ~StyleSurroundData()
    {
    }

    LengthBox offset;
    LengthBox margin;
    LengthBox padding;
    BorderData border;
};


//------------------------------------------------
// Box attributes. Not inherited.


class StyleBoxData : public DOM::DomShared
{
public:
    StyleBoxData()
    {
    }
    ~StyleBoxData()
    {
    }

    // copy and assignment
//    StyleBoxData(const StyleBoxData &other);
//    const StyleBoxData &operator = (const StyleBoxData &other);

    void setDefaultValues()
    {
	position = STATIC;
	floating = FNONE;
    }

    EPosition position;
    EFloat floating;

    Length width;
    Length height;

    Length min_width;
    Length max_width;

    Length min_height;
    Length max_height;
};


//------------------------------------------------
// Random visual rendering model attributes. Not inherited.

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

enum EClear{
    CNONE = 0, CLEFT = 1, CRIGHT = 2, CBOTH = 3
};

enum ETableLayout {
    TAUTO, TFIXED
};

class StyleVisualData : public DOM::DomShared
{
public:
    StyleVisualData()
    {
	vertical_align = BASELINE;
	colspan = 1;
	clear = CNONE;
	overflow = OVISIBLE;
	visiblity = VISIBLE; // ### default is inherit!!!
	layout = TAUTO;
    }
    EOverflow overflow;
    EVisiblity visiblity;
    EVerticalAlign vertical_align;
    EClear clear;
    ETableLayout layout;

    LengthBox clip;

    short colspan; // for html, not a css2 attribute

    short counter_increment; //ok, so these are not visual mode spesific
    short counter_reset;     //can't go to list, since these are not inherited
};

//------------------------------------------------
enum EBackgroundRepeat {
    REPEAT, REPEAT_X, REPEAT_Y, NO_REPEAT
};



class StyleBackgroundData : public DOM::DomShared
{
public:
    StyleBackgroundData()
    {
	image = 0;
    }

    QColor color;
    CachedImage *image;

    EBackgroundRepeat repeat;
    bool attachment;

    Length x_position;
    Length y_position;
};

//------------------------------------------------
// Text attributes. These are inherited.
//
// the text-decoration and text-shadow attributes
// are inherited from the
// first parent which is block level
//
// this applies to decoration_color too

enum EWhiteSpace {
    NORMAL, PRE, NOWRAP
};

enum ETextAlign{
    LEFT, RIGHT, CENTER, JUSTIFY
};

enum EDirection{
    LTR, RTL
};

enum ETextDecoration{
    TDNONE = 0x0 , UNDERLINE = 0x1, OVERLINE = 0x2, LINE_THROUGH= 0x4, BLINK = 0x8
};

class StyleTextData : public DOM::DomShared
{
public:
    void setDefaultValues()
    {
	letter_spacing = 0;
	word_spacing = 0;
	align = JUSTIFY;
	direction = LTR;
	text_decoration = TDNONE;
	line_height = Length(100, Percent);
	indent = Length(0, Fixed);
	fontSize = 3;
    }

    QFont font;
    short fontSize; 	    // to help implementing smaller, larger
    QColor color;
    QColor decoration_color;

    Length indent;
    Length line_height;

    unsigned int letter_spacing : 8;
    unsigned int word_spacing : 8;

    ETextAlign align : 2;
    EDirection direction : 1;
    EWhiteSpace white_space : 2;
    int text_decoration : 4;
};


//------------------------------------------------
// Table attributes. These are inherited.

enum EEmptyCell {
    SHOW, HIDE
};

enum ECaptionSide
{
    CAPTOP, CAPBOTTOM, CAPLEFT, CAPRIGHT
};

class StyleTableData : public DOM::DomShared
{
public:
    StyleTableData()
    {
    	border_collapse = true;
    	border_spacing = 0;
    }

    // true == collapse, false = separate
    bool border_collapse;
    short border_spacing;

    EEmptyCell empty_cells;

    ECaptionSide caption_side;

};


//------------------------------------------------
// List attributes. These are inherited.

enum EListStyleType {
     DISC, CIRCLE, SQUARE, LDECIMAL, DECIMAL_LEADING_ZERO,
     LOWER_ROMAN, UPPER_ROMAN, LOWER_GREEK,
     LOWER_ALPHA, LOWER_LATIN, UPPER_ALPHA, UPPER_LATIN,
     HEBREW, ARMENIAN, GEORGIAN, CJK_IDEOGRAPHIC,
     HIRAGANA, KATAKANA, HIRAGANA_IROHA, KATAKANA_IROHA, LNONE
};

enum EListStylePosition { OUTSIDE, INSIDE };


class StyleListData : public DOM::DomShared
{
public:
    StyleListData()
    {
	style_image = 0;
	style_type = DISC;
	style_position = OUTSIDE;
    }

    EListStyleType style_type;
    CachedImage *style_image;
    EListStylePosition style_position;
};

//------------------------------------------------

enum EDisplay {
    INLINE, BLOCK, LIST_ITEM, RUN_IN, COMPACT,
    MARKER, TABLE, INLINE_TABLE, TABLE_ROW_GROUP,
    TABLE_HEADER_GROUP, TABLE_FOOTER_GROUP, TABLE_ROW,
    TABLE_COLUMN_GROUP, TABLE_COLUMN, TABLE_CELL,
    TABLE_CAPTION, NONE
};

class RenderStyle
{
protected:

    EDisplay _display;

    static RenderStyle* _default;

// non-inherited attributes
    DataRef<StyleBoxData> box;
    DataRef<StyleVisualData> visual;
    DataRef<StyleBackgroundData> background;
    DataRef<StyleSurroundData> surround;

// inherited attributes
    DataRef<StyleListData> list;
    DataRef<StyleTableData> table;
    DataRef<StyleTextData> text;

    static const QColor undefinedColor;

public:
    RenderStyle();
    RenderStyle(const RenderStyle&);
    RenderStyle(const RenderStyle* inheritParent);
    ~RenderStyle();

    /**
     * Intantiates new style object following the
     * css2 inheritance rules.
     */
    RenderStyle* inheritFrom(RenderStyle* inherit);

    bool        isFloating() { return (box->floating == FLEFT || box->floating == FRIGHT); }
    bool        hasMargin() { return surround->margin.nonZero(); }
    bool        hasPadding() { return surround->padding.nonZero(); }
    bool        hasBorder() { return surround->border.hasBorder(); }
    bool        hasOffset() { return surround->offset.nonZero(); }

// attribute getter methods

    EDisplay 	display() { return _display; }

    Length  	left() {  return surround->offset.left; }
    Length  	right() {  return surround->offset.right; }
    Length  	top() {  return surround->offset.top; }
    Length  	bottom() {  return surround->offset.bottom; }

    EPosition 	position() { return box->position; }
    EFloat  	floating() { return box->floating; }

    Length  	width() { return box->width; }
    Length  	height() { return box->height; }
    Length  	minWidth() { return box->min_width; }
    Length  	maxWidth() { return box->max_width; }
    Length  	minHeight() { return box->min_height; }
    Length  	maxHeight() { return box->max_height; }

    unsigned short  borderLeftWidth()
    { if( surround->border.left.style == BNONE) return 0; return surround->border.left.width; }
    EBorderStyle    borderLeftStyle() { return surround->border.left.style; }
    const QColor &  borderLeftColor() { return surround->border.left.color; }
    unsigned short  borderRightWidth()
    { if (surround->border.right.style == BNONE) return 0; return surround->border.right.width; }
    EBorderStyle    borderRightStyle() {  return surround->border.right.style; }
    const QColor &  	    borderRightColor() {  return surround->border.right.color; }
    unsigned short  borderTopWidth()
    { if(surround->border.top.style == BNONE) return 0; return surround->border.top.width; }
    EBorderStyle    borderTopStyle() {return surround->border.top.style; }
    const QColor &  borderTopColor() {  return surround->border.top.color; }
    unsigned short  borderBottomWidth()
    { if(surround->border.bottom.style == BNONE) return 0; return surround->border.bottom.width; }
    EBorderStyle    borderBottomStyle() {  return surround->border.bottom.style; }
    const QColor &  	    borderBottomColor() {  return surround->border.bottom.color; }

    EOverflow overflow() { return visual->overflow; }
    EVisiblity visiblity() { return visual->visiblity; }
    EVerticalAlign verticalAlign() { return visual->vertical_align; }

    Length clipLeft() { return visual->clip.left; }
    Length clipRight() { return visual->clip.right; }
    Length clipTop() { return visual->clip.top; }
    Length clipBottom() { return visual->clip.bottom; }
    EClear clear() { return visual->clear; }
    ETableLayout tableLayout() { return visual->layout; }

    short colSpan() { return visual->colspan; }

    const QFont & font() { return text->font; }
    short fontSize() { return text->fontSize; }

    const QColor & color() { return text->color; }
    Length textIndent() { return text->indent; }
    ETextAlign textAlign() { return text->align; }
    int textDecoration() { return text->text_decoration; }
    const QColor &textDecorationColor() { return text->decoration_color; }

    EDirection direction() { return text->direction; }
    Length lineHeight() { return text->line_height; }

    EWhiteSpace whiteSpace() { return text->white_space; }


    const QColor & backgroundColor() { return background->color; }
    CachedImage *backgroundImage() { return background->image; }
    EBackgroundRepeat backgroundRepeat() { return background->repeat; }
    // backgroundAttachment returns true for scrolling (regular) attachment, false for fixed
    bool backgroundAttachment() { return background->attachment; }
    Length backgroundXPosition() { return background->x_position; }
    Length backgroundYPosition() { return background->y_position; }

    // returns true for collapsing borders, false for separate borders
    bool borderCollapse() { return table->border_collapse; }
    short borderSpacing() { return table->border_spacing; }
    EEmptyCell emptyCells() { return table->empty_cells; }
    ECaptionSide captionSide() { return table->caption_side; }

    short counterIncrement() { return visual->counter_increment; }
    short counterReset() { return visual->counter_reset; }

    EListStyleType listStyleType() { return list->style_type; }
    CachedImage *listStyleImage() { return list->style_image; }
    EListStylePosition listStylePosition() { return list->style_position; }

    Length marginTop() { return surround->margin.top; }
    Length marginBottom() {  return surround->margin.bottom; }
    Length marginLeft() {  return surround->margin.left; }
    Length marginRight() {  return surround->margin.right; }

    Length paddingTop() {  return surround->padding.top; }
    Length paddingBottom() {  return surround->padding.bottom; }
    Length paddingLeft() { return surround->padding.left; }
    Length paddingRight() {  return surround->padding.right; }

// attribute setter methods

    void setDisplay(EDisplay v) { _display = v; }
    void setPosition(EPosition v) { box.set()->position = v; }
    void setFloating(EFloat v) { box.set()->floating = v; }

    void setRight(Length v) {  surround.set()->offset.right = v; }
    void setLeft(Length v) {  surround.set()->offset.left = v; }
    void setTop(Length v) {  surround.set()->offset.top = v; }
    void setBottom(Length v) {  surround.set()->offset.bottom = v; }

    void setWidth(Length v) { box.set()->width = v; }
    void setHeight(Length v) { box.set()->height = v; }

    void setMinWidth(Length v) { box.set()->min_width = v; }
    void setMaxWidth(Length v) { box.set()->max_width = v; }
    void setMinHeight(Length v) { box.set()->min_height = v; }
    void setMaxHeight(Length v) { box.set()->max_height = v; }

    void setBorderWidth(unsigned short v) {	
	BorderData& d = surround.set()->border;	
	d.left.width = v;
    	d.right.width = v;
    	d.top.width = v;	
    	d.bottom.width = v;
    }	
    void setBorderColor(const QColor & v) {
    	BorderData& d = surround.set()->border;	
    	d.left.color = v;
    	d.right.color = v;
    	d.top.color = v;	
    	d.bottom.color = v;
    }	
    void setBorderStyle(EBorderStyle v) {	
	BorderData& d = surround.set()->border;
    	d.left.style = v;
    	d.right.style = v;
    	d.top.style = v;	
    	d.bottom.style = v;
    }	
    void setBorderLeftWidth(unsigned short v) {  surround.set()->border.left.width = v; }
    void setBorderLeftStyle(EBorderStyle v) {  surround.set()->border.left.style = v; }
    void setBorderLeftColor(const QColor & v) {  surround.set()->border.left.color = v; }
    void setBorderRightWidth(unsigned short v) {  surround.set()->border.right.width = v; }
    void setBorderRightStyle(EBorderStyle v) {  surround.set()->border.right.style = v; }
    void setBorderRightColor(const QColor & v) {  surround.set()->border.right.color = v; }
    void setBorderTopWidth(unsigned short v) {  surround.set()->border.top.width = v; }
    void setBorderTopStyle(EBorderStyle v) {  surround.set()->border.top.style = v; }
    void setBorderTopColor(const QColor & v) {  surround.set()->border.top.color = v; }
    void setBorderBottomWidth(unsigned short v) { surround.set()->border.bottom.width = v; }
    void setBorderBottomStyle(EBorderStyle v) {  surround.set()->border.bottom.style = v; }
    void setBorderBottomColor(const QColor & v) {  surround.set()->border.bottom.color = v; }

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

    void setClear(EClear v) { visual.set()->clear = v; }
    void setTableLayout(ETableLayout v) { visual.set()->layout = v; }
    void colSpan(short v) { visual.set()->colspan = v; }

    void setFont(const QFont & v) { text.set()->font = v; }
    void setFontSize(short v) { text.set()->fontSize = v; }

    void setColor(const QColor & v) { text.set()->color = v; }
    void setTextIndent(Length v) { text.set()->indent = v; }
    void setTextAlign(ETextAlign v) { text.set()->align = v; }
    void setTextDecoration(int v) { text.set()->text_decoration = v; }
    void setTextDecorationColor(const QColor &c) { text.set()->decoration_color = c; }
    void setDirection(EDirection v) { text.set()->direction = v; }
    void setLineHeight(Length v) { text.set()->line_height = v; }

    void setWhiteSpace(EWhiteSpace v) { text.set()->white_space = v; }


    void setBackgroundColor(const QColor & v) {  background.set()->color = v; }
    void setBackgroundImage(CachedImage *v) {  background.set()->image = v; }
    void setBackgroundRepeat(EBackgroundRepeat v) {  background.set()->repeat = v; }
    void setBackgroundAttachment(bool scroll) {  background.set()->attachment = scroll; }
    void setBackgroundXPosition(Length v) {  background.set()->x_position = v; }
    void setBackgroundYPosition(Length v) {  background.set()->y_position = v; }

    void setBorderCollapse(bool collapse) { table.set()->border_collapse = collapse; }
    void setBorderSpacing(short v) { table.set()->border_spacing = v; }
    void setEmptyCells(EEmptyCell v) { table.set()->empty_cells = v; }
    void setCaptionSide(ECaptionSide v) { table.set()->caption_side = v; }


    void setCounterIncrement(short v) {  visual.set()->counter_increment = v; }
    void setCounterReset(short v) {  visual.set()->counter_reset = v; }

    void setListStyleType(EListStyleType v) {  list.set()->style_type = v; }
    void setListStyleImage(CachedImage *v) {  list.set()->style_image = v; }
    void setListStylePosition(EListStylePosition v) {  list.set()->style_position = v; }

    void setMarginTop(Length v) { surround.set()->margin.top = v; }
    void setMarginBottom(Length v) {  surround.set()->margin.bottom = v; }
    void setMarginLeft(Length v) {  surround.set()->margin.left = v; }
    void setMarginRight(Length v) { surround.set()->margin.right = v; }

    void setPaddingTop(Length v) {  surround.set()->padding.top = v; }
    void setPaddingBottom(Length v) {  surround.set()->padding.bottom = v; }
    void setPaddingLeft(Length v) {  surround.set()->padding.left = v; }
    void setPaddingRight(Length v) {  surround.set()->padding.right = v; }
};

} // namespace

#endif

