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

/*
 * WARNING:
 * --------
 *
 * The order of the values in the enums have to agree with the order specified
 * in cssvalues.in, otherwise some optimizations in the parser will fail,
 * and produce invaliud results.
 */

#include <qcolor.h>
#include <qfont.h>
#include <qlist.h>
#include <qpalette.h>
#include <qapplication.h>
#include "dom/dom_misc.h"

#include <khtmllayout.h>

#define SET_VAR(group,variable,value) \
    if (group->variable != value) \
        group.access()->variable = value;

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
    	return data;
    }

    const DATA* get() const
    {
    	return data;
    }


    DATA* access()
    {
    	if (!data->hasOneRef())
	{
	    data->deref();
	    data = new DATA(*data);
	    data->ref();
	}
	return data;
    }

    void init()
    {
    	if (data)
    	    data->deref();
    	data = new DATA;
	data->ref();
    }

    DataRef<DATA>& operator=(const DataRef<DATA>& d)
    {
    	if (data==d.data)
	    return *this;
    	if (data)
    	    data->deref();
    	data = d.data;

	data->ref();

	return *this;
    }

private:
    DATA* data;
};


class SharedData
{
public:
    SharedData() { _ref=0; /*counter++;*/ }
    virtual ~SharedData() { /*counter--;*/ }

    void ref() { _ref++;  }
    void deref() { if(_ref) _ref--; if(_ref<=0) delete this; }
    bool hasOneRef() { //kdDebug(300) << "ref=" << _ref << endl;
    	return _ref==1; }

//    static int counter;
protected:
    unsigned int _ref;
};


//------------------------------------------------

//------------------------------------------------
// Box model attributes. Not inherited.

struct LengthBox
{
    LengthBox()
    {
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

    bool operator==(const LengthBox& o) const
    {
    	return left==o.left && right==o.right && top==o.top && bottom==o.bottom;
    }


    bool nonZero() const { return left.value!=0 || right.value!=0 || top.value!=0 || bottom.value!=0; }
};



enum EPosition {
    STATIC, RELATIVE, ABSOLUTE, FIXED
};

enum EFloat {
    FNONE, FLEFT, FRIGHT
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
    QColor color;
    unsigned short width : 12;
    EBorderStyle style : 4;

    bool nonZero() const { return width!=0 && style!=BNONE; }

    bool operator==(const BorderValue& o) const
    {
    	return width==o.width && style==o.style && color==o.color;
    }

};

class BorderData : public SharedData
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

    bool operator==(const BorderData& o) const
    {
    	return left==o.left && right==o.right && top==o.top && bottom==o.bottom;
    }

};

class StyleSurroundData : public SharedData
{
public:
    StyleSurroundData();

    StyleSurroundData(const StyleSurroundData& o );
    bool operator==(const StyleSurroundData& o) const;

    LengthBox offset;
    LengthBox margin;
    LengthBox padding;
    BorderData border;
    BorderValue outline;
};


//------------------------------------------------
// Box attributes. Not inherited.


const int ZAUTO=0;

class StyleBoxData : public SharedData
{
public:
    StyleBoxData();

    StyleBoxData(const StyleBoxData& o );


    // copy and assignment
//    StyleBoxData(const StyleBoxData &other);
//    const StyleBoxData &operator = (const StyleBoxData &other);

    void setDefaultValues()
    {
    	z_index = ZAUTO;
    }

    bool operator==(const StyleBoxData& o) const;

    Length width;
    Length height;

    Length min_width;
    Length max_width;

    Length min_height;
    Length max_height;

    Length vertical_align;

    int z_index;
};


//------------------------------------------------
// Random visual rendering model attributes. Not inherited.

enum EOverflow {
    OVISIBLE, OHIDDEN, SCROLL, AUTO
};

enum EVerticalAlign {
    BASELINE, MIDDLE, SUB, SUPER, TEXT_TOP,
    TEXT_BOTTOM, TOP, BOTTOM, BASELINE_MIDDLE, LENGTH
};

enum EClear{
    CNONE = 0, CLEFT = 1, CRIGHT = 2, CBOTH = 3
};

enum ETableLayout {
    TAUTO, TFIXED
};

class StyleVisualData : public SharedData
{
public:
    StyleVisualData();

    virtual ~StyleVisualData();

    StyleVisualData(const StyleVisualData& o );


    LengthBox clip;

    short colspan; // for html, not a css2 attribute

    short counter_increment; //ok, so these are not visual mode spesific
    short counter_reset;     //can't go to inherited, since these are not inherited

    QPalette palette;      //widget styling with IE attributes

};

//------------------------------------------------
enum EBackgroundRepeat {
    REPEAT, REPEAT_X, REPEAT_Y, NO_REPEAT
};



class StyleBackgroundData : public SharedData
{
public:
    StyleBackgroundData()
    {
	image = 0;
    }

    virtual ~StyleBackgroundData()
    {
    }

    StyleBackgroundData(const StyleBackgroundData& o ) : SharedData()
    {
    	color = o.color;
	image = o.image;
	x_position = o.x_position;
	y_position = o.y_position;
    }

    bool operator==(const StyleBackgroundData& o) const
    {
    	return
	    color == o.color &&
	    image == o.image &&
	    x_position == o.x_position &&
	    y_position == o.y_position;
    }

    QColor color;
    CachedImage *image;

    Length x_position;
    Length y_position;
};


//------------------------------------------------
// Inherited attributes.
//
// the inherited-decoration and inherited-shadow attributes
// are inherited from the
// first parent which is block level
//
// this applies to decoration_color too

enum EWhiteSpace {
    NORMAL, PRE, NOWRAP
};

enum ETextAlign {
    LEFT, RIGHT, CENTER, JUSTIFY, KONQ_CENTER
};

enum ETextTransform {
    CAPITALIZE, UPPERCASE, LOWERCASE, TTNONE
};

enum EDirection {
    LTR, RTL
};

enum ETextDecoration {
    TDNONE = 0x0 , UNDERLINE = 0x1, OVERLINE = 0x2, LINE_THROUGH= 0x4, BLINK = 0x8
};

class StyleInheritedData : public SharedData
{
public:
    void setDefaultValues()
    {
	letter_spacing = 0;
	word_spacing = 0;
	line_height = Length(-100, Percent);
	indent = Length(0, Fixed);
	border_spacing = 0;
	style_image = 0;
	cursor_image = 0;
    }

    StyleInheritedData() { setDefaultValues(); }
    virtual ~StyleInheritedData() { }

    StyleInheritedData(const StyleInheritedData& o ) : SharedData()
    {
	indent = o.indent;
	line_height = o.line_height;
	letter_spacing = o.letter_spacing;
	border_spacing = o.border_spacing;
	style_image = o.style_image;
	cursor_image = o.cursor_image;
	font = o.font;
	color = o.color;
	decoration_color = o.decoration_color;
	letter_spacing = 0;
	word_spacing = 0;
    }

    bool operator==(const StyleInheritedData& o) const
    {
        return indent == o.indent &&
               line_height == o.line_height &&
               letter_spacing == o.letter_spacing &&
               word_spacing == o.word_spacing &&
               border_spacing == o.border_spacing &&
               style_image == o.style_image &&
	 cursor_image == o.cursor_image &&
               font == o.font &&
               color == o.color &&
               decoration_color == o.decoration_color;

        // doesn't work because structs are not packed
    	//return memcmp(this, &o, sizeof(*this))==0;
    }

    Length indent;
    Length line_height;

    int letter_spacing : 8;
    int word_spacing : 8;

    short border_spacing;

    CachedImage *style_image;
    CachedImage *cursor_image;

    QFont font;
    QColor color;
    QColor decoration_color;

};


enum EEmptyCell {
    SHOW, HIDE
};

enum ECaptionSide
{
    CAPTOP, CAPBOTTOM, CAPLEFT, CAPRIGHT
};


enum EListStyleType {
     DISC, CIRCLE, SQUARE, LDECIMAL, DECIMAL_LEADING_ZERO,
     LOWER_ROMAN, UPPER_ROMAN, LOWER_GREEK,
     LOWER_ALPHA, LOWER_LATIN, UPPER_ALPHA, UPPER_LATIN,
     HEBREW, ARMENIAN, GEORGIAN, CJK_IDEOGRAPHIC,
     HIRAGANA, KATAKANA, HIRAGANA_IROHA, KATAKANA_IROHA, LNONE
};

enum EListStylePosition { OUTSIDE, INSIDE };

enum EVisiblity { VISIBLE, HIDDEN, COLLAPSE };

enum ECursor {
    CURSOR_AUTO, CURSOR_CROSS, CURSOR_DEFAULT, CURSOR_POINTER, CURSOR_MOVE,
    CURSOR_E_RESIZE, CURSOR_NE_RESIZE, CURSOR_NW_RESIZE, CURSOR_N_RESIZE, CURSOR_SE_RESIZE, CURSOR_SW_RESIZE,
    CURSOR_S_RESIZE, CURSOR_W_RESIZE, CURSOR_TEXT, CURSOR_WAIT, CURSOR_HELP
};

enum EFontVariant {
    FVNORMAL, SMALL_CAPS
};

//------------------------------------------------

enum EDisplay {
    INLINE, BLOCK, LIST_ITEM, RUN_IN, COMPACT, MARKER, KONQ_RULER,
    TABLE, INLINE_TABLE, TABLE_ROW_GROUP,
    TABLE_HEADER_GROUP, TABLE_FOOTER_GROUP, TABLE_ROW,
    TABLE_COLUMN_GROUP, TABLE_COLUMN, TABLE_CELL,
    TABLE_CAPTION, NONE
};

class RenderStyle : public DOM::DomShared
{
public:
    static void cleanup();

    // static pseudo styles. Dynamic ones are produced on the fly.
    enum PseudoId { NOPSEUDO, FIRST_LINE, FIRST_LETTER };

protected:
    void setBitDefaults();

// !START SYNC!: Keep this in sync with the copy constructor in render_style.cpp

    EDisplay _display : 5;

// inherit
    bool _border_collapse : 1 ;
    EEmptyCell _empty_cells : 2 ;
    ECaptionSide _caption_side : 2;
    EListStyleType _list_style_type : 5 ;
    EListStylePosition _list_style_position :1;
    EVisiblity _visiblity : 2;
    ETextAlign _text_align : 3;
    ETextTransform _text_transform : 4;
    EDirection _direction : 1;
    EWhiteSpace _white_space : 2;
    int _text_decoration : 4;
    bool _visuallyOrdered : 1;
    ECursor _cursor_style : 4;
    EFontVariant _font_variant : 1;

    bool _htmlHacks :1;

// don't inherit

    EOverflow _overflow : 4 ;
    EVerticalAlign _vertical_align : 4;
    EClear _clear : 2;
    ETableLayout _table_layout : 1;
    EBackgroundRepeat _bg_repeat : 2;
    bool _bg_attachment : 1;
    EPosition _position : 2;
    EFloat _floating : 2;

    bool _flowAroundFloats :1;

    PseudoId _styleType : 2;
    bool _hasHover : 1;
    bool _hasFocus : 1;
    bool _hasActive : 1;

// non-inherited attributes
    DataRef<StyleBoxData> box;
    DataRef<StyleVisualData> visual;
    DataRef<StyleBackgroundData> background;
    DataRef<StyleSurroundData> surround;

// inherited attributes
    DataRef<StyleInheritedData> inherited;

// list of associated pseudo styles
    RenderStyle* pseudoStyle;

// !END SYNC!

// static default style
    static RenderStyle* _default;

private:
    RenderStyle(const RenderStyle*) {}

public:

    RenderStyle();
    // used to create the default style.
    RenderStyle(bool);
    RenderStyle(const RenderStyle&);

    ~RenderStyle();

    void inheritFrom(const RenderStyle* inheritParent);

    PseudoId styleType() { return _styleType; }

    RenderStyle* getPseudoStyle(PseudoId pi);
    RenderStyle* addPseudoStyle(PseudoId pi);
    bool hasPseudoStyle() const { return pseudoStyle; }
    void removePseudoStyle(PseudoId pi);

    bool hasHover() const { return _hasHover; }
    bool hasFocus() const { return _hasFocus; }
    bool hasActive() const { return _hasActive; }

    void setHasHover() { _hasHover = true; }
    void setHasFocus() { _hasFocus = true; }
    void setHasActive() { _hasActive = true; }

    bool operator==(const RenderStyle& other) const;

    bool        isFloating() const { return (_floating == FLEFT || _floating == FRIGHT); }
    bool        hasMargin() const { return surround->margin.nonZero(); }
    bool        hasPadding() const { return surround->padding.nonZero(); }
    bool        hasBorder() const { return surround->border.hasBorder(); }
    bool        hasOffset() const { return surround->offset.nonZero(); }

    bool visuallyOrdered() const { return _visuallyOrdered; }
    void setVisuallyOrdered(bool b) { _visuallyOrdered = b; }

// attribute getter methods

    EDisplay 	display() const { return _display; }

    Length  	left() const {  return surround->offset.left; }
    Length  	right() const {  return surround->offset.right; }
    Length  	top() const {  return surround->offset.top; }
    Length  	bottom() const {  return surround->offset.bottom; }

    EPosition 	position() const { return _position; }
    EFloat  	floating() const { return _floating; }

    Length  	width() const { return box->width; }
    Length  	height() const { return box->height; }
    Length  	minWidth() const { return box->min_width; }
    Length  	maxWidth() const { return box->max_width; }
    Length  	minHeight() const { return box->min_height; }
    Length  	maxHeight() const { return box->max_height; }

    unsigned short  borderLeftWidth() const
    { if( surround->border.left.style == BNONE) return 0; return surround->border.left.width; }
    EBorderStyle    borderLeftStyle() const { return surround->border.left.style; }
    const QColor &  borderLeftColor() const { return surround->border.left.color; }
    unsigned short  borderRightWidth() const
    { if (surround->border.right.style == BNONE) return 0; return surround->border.right.width; }
    EBorderStyle    borderRightStyle() const {  return surround->border.right.style; }
    const QColor &  	    borderRightColor() const {  return surround->border.right.color; }
    unsigned short  borderTopWidth() const
    { if(surround->border.top.style == BNONE) return 0; return surround->border.top.width; }
    EBorderStyle    borderTopStyle() const {return surround->border.top.style; }
    const QColor &  borderTopColor() const {  return surround->border.top.color; }
    unsigned short  borderBottomWidth() const
    { if(surround->border.bottom.style == BNONE) return 0; return surround->border.bottom.width; }
    EBorderStyle    borderBottomStyle() const {  return surround->border.bottom.style; }
    const QColor &  	    borderBottomColor() const {  return surround->border.bottom.color; }

    unsigned short  outlineWidth() const
    { if(surround->outline.style == BNONE) return 0; return surround->outline.width; }
    EBorderStyle    outlineStyle() const {  return surround->outline.style; }
    const QColor &  	    outlineColor() const {  return surround->outline.color; }

    EOverflow overflow() const { return _overflow; }
    EVisiblity visiblity() const { return _visiblity; }
    EVerticalAlign verticalAlign() const { return _vertical_align; }
    Length verticalAlignLength() const { return box->vertical_align; }

    Length clipLeft() const { return visual->clip.left; }
    Length clipRight() const { return visual->clip.right; }
    Length clipTop() const { return visual->clip.top; }
    Length clipBottom() const { return visual->clip.bottom; }
    EClear clear() const { return _clear; }
    ETableLayout inheritedLayout() const { return _table_layout; }

    short colSpan() const { return visual->colspan; }

    const QFont & font() const { return inherited->font; }

    const QColor & color() const { return inherited->color; }
    Length textIndent() const { return inherited->indent; }
    ETextAlign textAlign() const { return _text_align; }
    ETextTransform textTransform() const { return _text_transform; }
    int textDecoration() const { return _text_decoration; }
    const QColor &textDecorationColor() const { return inherited->decoration_color; }
    int wordSpacing() const { return inherited->word_spacing; }
    int letterSpacing() const { return inherited->letter_spacing; }

    EDirection direction() const { return _direction; }
    Length lineHeight() const { return inherited->line_height; }

    EWhiteSpace whiteSpace() const { return _white_space; }


    const QColor & backgroundColor() const { return background->color; }
    CachedImage *backgroundImage() const { return background->image; }
    EBackgroundRepeat backgroundRepeat() const { return _bg_repeat; }
    // backgroundAttachment returns true for scrolling (regular) attachment, false for fixed
    bool backgroundAttachment() const { return _bg_attachment; }
    Length backgroundXPosition() const { return background->x_position; }
    Length backgroundYPosition() const { return background->y_position; }

    // returns true for collapsing borders, false for separate borders
    bool borderCollapse() const { return _border_collapse; }
    short borderSpacing() const { return inherited->border_spacing; }
    EEmptyCell emptyCells() const { return _empty_cells; }
    ECaptionSide captionSide() const { return _caption_side; }

    short counterIncrement() const { return visual->counter_increment; }
    short counterReset() const { return visual->counter_reset; }

    EListStyleType listStyleType() const { return _list_style_type; }
    CachedImage *listStyleImage() const { return inherited->style_image; }
    EListStylePosition listStylePosition() const { return _list_style_position; }

    Length marginTop() const { return surround->margin.top; }
    Length marginBottom() const {  return surround->margin.bottom; }
    Length marginLeft() const {  return surround->margin.left; }
    Length marginRight() const {  return surround->margin.right; }

    Length paddingTop() const {  return surround->padding.top; }
    Length paddingBottom() const {  return surround->padding.bottom; }
    Length paddingLeft() const { return surround->padding.left; }
    Length paddingRight() const {  return surround->padding.right; }

    ECursor cursor() const { return _cursor_style; }
    EFontVariant fontVariant() { return _font_variant; }

    CachedImage *cursorImage() const { return inherited->cursor_image; }


// attribute setter methods

    void setDisplay(EDisplay v) { _display = v; }
    void setPosition(EPosition v) { _position = v; }
    void setFloating(EFloat v) { _floating = v; }

    void setLeft(Length v)  {  SET_VAR(surround,offset.left,v) }
    void setRight(Length v) {  SET_VAR(surround,offset.right,v) }
    void setTop(Length v)   {  SET_VAR(surround,offset.top,v) }
    void setBottom(Length v){  SET_VAR(surround,offset.bottom,v) }

    void setWidth(Length v)  { SET_VAR(box,width,v) }
    void setHeight(Length v) { SET_VAR(box,height,v) }

    void setMinWidth(Length v)  { SET_VAR(box,min_width,v) }
    void setMaxWidth(Length v)  { SET_VAR(box,max_width,v) }
    void setMinHeight(Length v) { SET_VAR(box,min_height,v) }
    void setMaxHeight(Length v) { SET_VAR(box,max_height,v) }

    void setBorderLeftWidth(unsigned short v)   {  SET_VAR(surround,border.left.width,v) }
    void setBorderLeftStyle(EBorderStyle v)     {  SET_VAR(surround,border.left.style,v) }
    void setBorderLeftColor(const QColor & v)   {  SET_VAR(surround,border.left.color,v) }
    void setBorderRightWidth(unsigned short v)  {  SET_VAR(surround,border.right.width,v) }
    void setBorderRightStyle(EBorderStyle v)    {  SET_VAR(surround,border.right.style,v) }
    void setBorderRightColor(const QColor & v)  {  SET_VAR(surround,border.right.color,v) }
    void setBorderTopWidth(unsigned short v)    {  SET_VAR(surround,border.top.width,v) }
    void setBorderTopStyle(EBorderStyle v)      {  SET_VAR(surround,border.top.style,v) }
    void setBorderTopColor(const QColor & v)    {  SET_VAR(surround,border.top.color,v) }
    void setBorderBottomWidth(unsigned short v) {  SET_VAR(surround,border.bottom.width,v) }
    void setBorderBottomStyle(EBorderStyle v)   {  SET_VAR(surround,border.bottom.style,v) }
    void setBorderBottomColor(const QColor & v) {  SET_VAR(surround,border.bottom.color,v) }
    void setOutlineWidth(unsigned short v) {  SET_VAR(surround,outline.width,v) }
    void setOutlineStyle(EBorderStyle v)   {  SET_VAR(surround,outline.style,v) }
    void setOutlineColor(const QColor & v) {  SET_VAR(surround,outline.color,v) }

    void setOverflow(EOverflow v) { _overflow = v; }
    void setVisiblity(EVisiblity v) { _visiblity = v; }
    void setVerticalAlign(EVerticalAlign v) { _vertical_align = v; }
    void setVerticalAlignLength(Length l) { SET_VAR(box, vertical_align, l ) }

    void setClipLeft(Length v) { SET_VAR(visual,clip.left,v) }
    void setClipRight(Length v) { SET_VAR(visual,clip.right,v) }
    void setClipTop(Length v) { SET_VAR(visual,clip.top,v) }
    void setClipBottom(Length v) { SET_VAR(visual,clip.bottom,v) }

    void setClear(EClear v) { _clear = v; }
    void setTableLayout(ETableLayout v) { _table_layout = v; }
    void ssetColSpan(short v) { SET_VAR(visual,colspan,v) }

    void setFont(const QFont & v) { SET_VAR(inherited,font,v) }

    void setColor(const QColor & v) { SET_VAR(inherited,color,v) }
    void setTextIndent(Length v) { SET_VAR(inherited,indent,v) }
    void setTextAlign(ETextAlign v) { _text_align = v; }
    void setTextTransform(ETextTransform v) { _text_transform = v; }
    void setTextDecoration(int v) { _text_decoration = v; }
    void setTextDecorationColor(const QColor &v) { SET_VAR(inherited,decoration_color,v) }
    void setDirection(EDirection v) { _direction = v; }
    void setLineHeight(Length v) { SET_VAR(inherited,line_height,v) }

    void setWhiteSpace(EWhiteSpace v) { _white_space = v; }

    void setWordSpacing(int v) { SET_VAR(inherited,word_spacing,v) }
    void setLetterSpacing(int v) { SET_VAR(inherited,letter_spacing,v) }

    void setBackgroundColor(const QColor & v) {  SET_VAR(background,color,v) }
    void setBackgroundImage(CachedImage *v) {  SET_VAR(background,image,v) }
    void setBackgroundRepeat(EBackgroundRepeat v) {  _bg_repeat = v; }
    void setBackgroundAttachment(bool scroll) {  _bg_attachment = scroll; }
    void setBackgroundXPosition(Length v) {  SET_VAR(background,x_position,v) }
    void setBackgroundYPosition(Length v) {  SET_VAR(background,y_position,v) }

    void setBorderCollapse(bool collapse) { _border_collapse = collapse; }
    void setBorderSpacing(short v) { SET_VAR(inherited,border_spacing,v) }
    void setEmptyCells(EEmptyCell v) { _empty_cells = v; }
    void setCaptionSide(ECaptionSide v) { _caption_side = v; }


    void setCounterIncrement(short v) {  SET_VAR(visual,counter_increment,v) }
    void setCounterReset(short v) {  SET_VAR(visual,counter_reset,v) }

    void setListStyleType(EListStyleType v) {  _list_style_type = v; }
    void setListStyleImage(CachedImage *v) {  SET_VAR(inherited,style_image,v)}
    void setListStylePosition(EListStylePosition v) {  _list_style_position = v; }

    void setMarginTop(Length v)     {  SET_VAR(surround,margin.top,v) }
    void setMarginBottom(Length v)  {  SET_VAR(surround,margin.bottom,v) }
    void setMarginLeft(Length v)    {  SET_VAR(surround,margin.left,v) }
    void setMarginRight(Length v)   {  SET_VAR(surround,margin.right,v) }

    void setPaddingTop(Length v)    {  SET_VAR(surround,padding.top,v) }
    void setPaddingBottom(Length v) {  SET_VAR(surround,padding.bottom,v) }
    void setPaddingLeft(Length v)   {  SET_VAR(surround,padding.left,v) }
    void setPaddingRight(Length v)  {  SET_VAR(surround,padding.right,v) }

    void setCursor( ECursor c ) { _cursor_style = c; }
    void setFontVariant( EFontVariant f ) { _font_variant = f; }
    void setCursorImage( CachedImage *v ) { SET_VAR(inherited,cursor_image,v) }

    bool htmlHacks() const { return _htmlHacks; }
    void setHtmlHacks(bool b=true) { _htmlHacks = b; }

    bool flowAroundFloats() const { return _flowAroundFloats; }
    void setFlowAroundFloats(bool b=true) { _flowAroundFloats = b; }

    int zIndex() const { return box->z_index; }
    void setZIndex(int v) { SET_VAR(box,z_index,v) }

    QPalette palette() const { return visual->palette; }
    void setPaletteColor(QPalette::ColorGroup g, QColorGroup::ColorRole r, const QColor& c)
    {
        visual.access()->palette.setColor(g,r,c);
    }
    void resetPalette() // Called when the desktop color scheme changes.
    {
        const_cast<StyleVisualData *>(visual.get())->palette = QApplication::palette();
    }


};

} // namespace

#endif

