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

#include <khtmllayout.h>

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
//    	kdDebug(300) << "get" << endl;
	return data;
    }

    DATA* set()
    {    	
//    	kdDebug(300) << "set" << endl;
//    	if (data==0)
//	    createData();
    	if (!data->hasOneRef())
	{
//	    kdDebug(300) << "copy" << endl;
	    data->deref();
	    data = new DATA(*data);
	    data->ref();
	}
	return data;
    }

    void createData()
    {
//    	kdDebug(300) << "create" << endl;
    	if (data)
    	    data->deref();
    	data = new DATA;
	data->ref();
    }

    DataRef<DATA>& operator=(const DataRef<DATA>& d)
    {
    	//kdDebug(300) << "op=" << endl;
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
    SharedData() { _ref=0; counter++; }
    virtual ~SharedData() { counter--; }

    void ref() { _ref++;  }
    void deref() { if(_ref) _ref--; if(_ref<=0) delete this; }
    bool hasOneRef() { //kdDebug(300) << "ref=" << _ref << endl;
    	return _ref==1; }

    static int counter;
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

// CENTER is not part of CSS, but we need it to get <inherited align=center> and the <center>
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
    QColor color;
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
    StyleSurroundData()
    {
    }
    virtual ~StyleSurroundData()
    {
    }

    StyleSurroundData(const StyleSurroundData& o ) : SharedData()
    {
    	offset = o.offset;
	margin = o.margin;
	padding = o.padding;
	border = o.border;	
    }

    LengthBox offset;
    LengthBox margin;
    LengthBox padding;
    BorderData border;

    bool operator==(const StyleSurroundData& o) const
    {
    	return offset==o.offset && margin==o.margin &&
	    padding==o.padding && border==o.border;
    }
};


//------------------------------------------------
// Box attributes. Not inherited.


const short ZAUTO=-11111;

class StyleBoxData : public SharedData
{
public:
    StyleBoxData()
    {
    }
    virtual ~StyleBoxData()
    {
    }

    StyleBoxData(const StyleBoxData& o ) : SharedData()
    {
    	width = o.width;
	height = o.height;
	min_width = o.min_width;
	max_width = o.max_width;
	min_height = o.min_height;
	max_height = o.max_height;	
	z_index = o.z_index;
    }


    // copy and assignment
//    StyleBoxData(const StyleBoxData &other);
//    const StyleBoxData &operator = (const StyleBoxData &other);

    void setDefaultValues()
    {
    	z_index = ZAUTO;
    }

    bool operator==(const StyleBoxData& o) const
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

    Length width;
    Length height;

    Length min_width;
    Length max_width;

    Length min_height;
    Length max_height;
    
    short z_index;
};


//------------------------------------------------
// Random visual rendering model attributes. Not inherited.

enum EOverflow {
    OVISIBLE, OHIDDEN, SCROLL, AUTO
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

class StyleVisualData : public SharedData
{
public:
    StyleVisualData()
    {
	colspan = 1;
    }

    virtual ~StyleVisualData() {
    }

    StyleVisualData(const StyleVisualData& o ) : SharedData()
    {
    	clip = o.clip;
	colspan = o.colspan;
	counter_increment = o.counter_increment;
	counter_reset = o.counter_reset;
    }

    bool operator==(const StyleVisualData& o) const
    {
    	return
	    clip == o.clip &&
	    colspan == o.colspan &&
	    counter_increment == o.counter_increment &&
	    counter_reset == o.counter_reset;
    }

    LengthBox clip;

    short colspan; // for html, not a css2 attribute

    short counter_increment; //ok, so these are not visual mode spesific
    short counter_reset;     //can't go to inherited, since these are not inherited

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

enum ETextAlign{
    LEFT, RIGHT, CENTER, JUSTIFY
};

enum EDirection{
    LTR, RTL
};

enum ETextDecoration{
    TDNONE = 0x0 , UNDERLINE = 0x1, OVERLINE = 0x2, LINE_THROUGH= 0x4, BLINK = 0x8
};

class StyleInheritedData : public SharedData
{
public:
    void setDefaultValues()
    {
	letter_spacing = 0;
	word_spacing = 0;	
	line_height = Length(100, Percent);
	indent = Length(0, Fixed);
	border_spacing = 0;
	style_image = 0;
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
	font = o.font;
	color = o.color;
	decoration_color = o.decoration_color;
//////////////// patched by S.G. Hwang /////////////////////
	letter_spacing = 0;
	word_spacing = 0;	
    }

    bool operator==(const StyleInheritedData& o) const
    {
    	return memcmp(this, &o, sizeof(*this))==0;
    }

    Length indent;
    Length line_height;

//////////////// patched by S.G. Hwang /////////////////////
    //unsigned int letter_spacing : 8;
    //unsigned int word_spacing : 8;
    int letter_spacing : 8;
    int word_spacing : 8;

    short border_spacing;

    CachedImage *style_image;

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

    EDisplay _display : 5;



// inherit
    bool _border_collapse : 1 ;
    EEmptyCell _empty_cells : 2 ;
    ECaptionSide _caption_side : 2;
    EListStyleType _list_style_type : 5 ;
    EListStylePosition _list_style_position :1;
    EVisiblity _visiblity : 2;
    ETextAlign _text_align : 2;
    EDirection _direction : 1;
    EWhiteSpace _white_space : 2;
    int _text_decoration : 4;
    bool _visuallyOrdered : 1;

    bool _htmlHacks :1;

// don't inherit

    EOverflow _overflow : 4 ;
    EVerticalAlign _vertical_align : 8;
    EClear _clear : 2;
    ETableLayout _table_layout : 1;
    EBackgroundRepeat _bg_repeat : 2;
    bool _bg_attachment : 1;
    EPosition _position : 2;
    EFloat _floating : 2;

    bool _flowAroundFloats :1;

    static RenderStyle* _default;

// non-inherited attributes
    DataRef<StyleBoxData> box;
    DataRef<StyleVisualData> visual;
    DataRef<StyleBackgroundData> background;
    DataRef<StyleSurroundData> surround;

// inherited attributes
    DataRef<StyleInheritedData> inherited;

    static const QColor undefinedColor;

    void setBitDefaults();

public:
    RenderStyle();
    // used to create the default style.
    RenderStyle(bool _default);
    RenderStyle(const RenderStyle&);
    RenderStyle(const RenderStyle* inheritParent);
    ~RenderStyle();

    bool operator==(const RenderStyle& other) const;
    void mergeData(RenderStyle* other);

    static int counter;

    /**
     * Intantiates new style object following the
     * css2 inheritance rules.
     */
    RenderStyle* inheritFrom(RenderStyle* inherit);

    bool        isFloating() { return (_floating == FLEFT || _floating == FRIGHT); }
    bool        hasMargin() { return surround->margin.nonZero(); }
    bool        hasPadding() { return surround->padding.nonZero(); }
    bool        hasBorder() { return surround->border.hasBorder(); }
    bool        hasOffset() { return surround->offset.nonZero(); }

    bool visuallyOrdered() { return _visuallyOrdered; }
    void setVisuallyOrdered(bool b) { _visuallyOrdered = b; }

// attribute getter methods

    EDisplay 	display() { return _display; }

    Length  	left() {  return surround->offset.left; }
    Length  	right() {  return surround->offset.right; }
    Length  	top() {  return surround->offset.top; }
    Length  	bottom() {  return surround->offset.bottom; }

    EPosition 	position() { return _position; }
    EFloat  	floating() { return _floating; }

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

    EOverflow overflow() { return _overflow; }
    EVisiblity visiblity() { return _visiblity; }
    EVerticalAlign verticalAlign() { return _vertical_align; }

    Length clipLeft() { return visual->clip.left; }
    Length clipRight() { return visual->clip.right; }
    Length clipTop() { return visual->clip.top; }
    Length clipBottom() { return visual->clip.bottom; }
    EClear clear() { return _clear; }
    ETableLayout inheritedLayout() { return _table_layout; }

    short colSpan() { return visual->colspan; }

    const QFont & font() { return inherited->font; }

    const QColor & color() { return inherited->color; }
    Length textIndent() { return inherited->indent; }
    ETextAlign textAlign() { return _text_align; }
    int textDecoration() { return _text_decoration; }
    const QColor &textDecorationColor() { return inherited->decoration_color; }
///////////  patched by S.G. Hwang ////////////////////////////////////
    int wordSpacing() { return inherited->word_spacing; }
    int letterSpacing() { return inherited->letter_spacing; }
///////////  end of patched ///////////////////////////////////////////

    EDirection direction() { return _direction; }
    Length lineHeight() { return inherited->line_height; }

    EWhiteSpace whiteSpace() { return _white_space; }


    const QColor & backgroundColor() { return background->color; }
    CachedImage *backgroundImage() { return background->image; }
    EBackgroundRepeat backgroundRepeat() { return _bg_repeat; }
    // backgroundAttachment returns true for scrolling (regular) attachment, false for fixed
    bool backgroundAttachment() { return _bg_attachment; }
    Length backgroundXPosition() { return background->x_position; }
    Length backgroundYPosition() { return background->y_position; }

    // returns true for collapsing borders, false for separate borders
    bool borderCollapse() { return _border_collapse; }
    short borderSpacing() { return inherited->border_spacing; }
    EEmptyCell emptyCells() { return _empty_cells; }
    ECaptionSide captionSide() { return _caption_side; }

    short counterIncrement() { return visual->counter_increment; }
    short counterReset() { return visual->counter_reset; }

    EListStyleType listStyleType() { return _list_style_type; }
    CachedImage *listStyleImage() { return inherited->style_image; }
    EListStylePosition listStylePosition() { return _list_style_position; }

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
    void setPosition(EPosition v) { _position = v; }
    void setFloating(EFloat v) { _floating = v; }

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

    void setOverflow(EOverflow v) { _overflow = v; }
    void setVisiblity(EVisiblity v) { _visiblity = v; }
    void setVerticalAlign(EVerticalAlign v) { _vertical_align = v; }

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

    void setClear(EClear v) { _clear = v; }
    void setTableLayout(ETableLayout v) { _table_layout = v; }
    void colSpan(short v) { visual.set()->colspan = v; }

    void setFont(const QFont & v) { inherited.set()->font = v; }

    void setColor(const QColor & v) { inherited.set()->color = v; }
    void setTextIndent(Length v) { inherited.set()->indent = v; }
    void setTextAlign(ETextAlign v) { _text_align = v; }
    void setTextDecoration(int v) { _text_decoration = v; }
    void setTextDecorationColor(const QColor &c) { inherited.set()->decoration_color = c; }
    void setDirection(EDirection v) { _direction = v; }
    void setLineHeight(Length v) { inherited.set()->line_height = v; }

    void setWhiteSpace(EWhiteSpace v) { _white_space = v; }
//////////////////// patched by S.G Hwnag //////////////////////////////
    void setWordSpacing(int v) { inherited.set()->word_spacing = v; }
    void setLetterSpacing(int v) { inherited.set()->letter_spacing = v; }
//////////////////// end of patch //////////////////////////////////////


    void setBackgroundColor(const QColor & v) {  background.set()->color = v; }
    void setBackgroundImage(CachedImage *v) {  background.set()->image = v; }
    void setBackgroundRepeat(EBackgroundRepeat v) {  _bg_repeat = v; }
    void setBackgroundAttachment(bool scroll) {  _bg_attachment = scroll; }
    void setBackgroundXPosition(Length v) {  background.set()->x_position = v; }
    void setBackgroundYPosition(Length v) {  background.set()->y_position = v; }

    void setBorderCollapse(bool collapse) { _border_collapse = collapse; }
    void setBorderSpacing(short v) { inherited.set()->border_spacing = v; }
    void setEmptyCells(EEmptyCell v) { _empty_cells = v; }
    void setCaptionSide(ECaptionSide v) { _caption_side = v; }


    void setCounterIncrement(short v) {  visual.set()->counter_increment = v; }
    void setCounterReset(short v) {  visual.set()->counter_reset = v; }

    void setListStyleType(EListStyleType v) {  _list_style_type = v; }
    void setListStyleImage(CachedImage *v) {  inherited.set()->style_image = v; }
    void setListStylePosition(EListStylePosition v) {  _list_style_position = v; }

    void setMarginTop(Length v) { surround.set()->margin.top = v; }
    void setMarginBottom(Length v) {  surround.set()->margin.bottom = v; }
    void setMarginLeft(Length v) {  surround.set()->margin.left = v; }
    void setMarginRight(Length v) { surround.set()->margin.right = v; }

    void setPaddingTop(Length v) {  surround.set()->padding.top = v; }
    void setPaddingBottom(Length v) {  surround.set()->padding.bottom = v; }
    void setPaddingLeft(Length v) {  surround.set()->padding.left = v; }
    void setPaddingRight(Length v) {  surround.set()->padding.right = v; }

    bool htmlHacks() const { return _htmlHacks; }
    void setHtmlHacks(bool b=true) { _htmlHacks = b; }

    bool flowAroundFloats() const { return _flowAroundFloats; }
    void setFlowAroundFloats(bool b=true) { _flowAroundFloats = b; }
    
    short zIndex() const { return box->z_index; }
    void setZIndex(short v) { box.set()->z_index = v; }
};

} // namespace

#endif

