/*
 * This file is part of the DOM implementation for KDE.
 *
 * Copyright (C) 2000-2003 Lars Knoll (knoll@kde.org)
 *           (C) 2000 Antti Koivisto (koivisto@kde.org)
 *           (C) 2000-2003 Dirk Mueller (mueller@kde.org)
 *           (C) 2002 Apple Computer, Inc.
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
#include <qfontmetrics.h>
#include <qptrlist.h>
#include <qpalette.h>
#include <qapplication.h>

#include "dom/dom_misc.h"
#include "misc/khtmllayout.h"
#include "misc/shared.h"
#include "rendering/font.h"

#include <assert.h>

#define SET_VAR(group,variable,value) \
    if (!(group->variable == value)) \
        group.access()->variable = value;

#ifndef ENABLE_DUMP
#ifndef NDEBUG
#define ENABLE_DUMP 1
#endif
#endif

namespace DOM {
    class DOMStringImpl;
}

namespace khtml {

    class CachedImage;
    class CachedObject;

template <class DATA>
class DataRef
{
public:

    DataRef()
    {
	data=0;
    }
    DataRef( const DataRef<DATA> &d )
    {
    	data = d.data;
	data->ref();
    }

    ~DataRef()
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

    bool operator == ( const DataRef<DATA> &o ) const {
	return (*data == *(o.data) );
    }
    bool operator != ( const DataRef<DATA> &o ) const {
	return (*data != *(o.data) );
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
    }
    LengthBox( LengthType t )
	: left( t ), right ( t ), top( t ), bottom( t ) {}

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


    bool nonZero() const { return left.value() || right.value() || top.value() || bottom.value(); }
};



enum EPosition {
    STATIC, RELATIVE, ABSOLUTE, FIXED
};

enum EFloat {
    FNONE = 0, FLEFT, FRIGHT
};


//------------------------------------------------
// Border attributes. Not inherited.


enum EBorderStyle {
    BNONE, BHIDDEN, DOTTED, DASHED, DOUBLE, SOLID,
    OUTSET, INSET, GROOVE, RIDGE
};

class BorderValue
{
public:
    BorderValue() : width( 3 ), style( BNONE ) {}

    QColor color;
    unsigned short width : 12;
    EBorderStyle style : 4;

    bool operator==(const BorderValue& o) const
    {
    	return width==o.width && style==o.style && color==o.color;
    }

};

class BorderData : public Shared<BorderData>
{
public:
    BorderValue left;
    BorderValue right;
    BorderValue top;
    BorderValue bottom;

    bool hasBorder() const
    {
    	return left.width || right.width || top.width || bottom.width;
    }

    bool operator==(const BorderData& o) const
    {
    	return left==o.left && right==o.right && top==o.top && bottom==o.bottom;
    }

};

class StyleSurroundData : public Shared<StyleSurroundData>
{
public:
    StyleSurroundData();

    StyleSurroundData(const StyleSurroundData& o );
    bool operator==(const StyleSurroundData& o) const;
    bool operator!=(const StyleSurroundData& o) const {
        return !(*this == o);
    }

    LengthBox offset;
    LengthBox margin;
    LengthBox padding;
    BorderData border;
};


//------------------------------------------------
// Box attributes. Not inherited.

class StyleBoxData : public Shared<StyleBoxData>
{
public:
    StyleBoxData();

    StyleBoxData(const StyleBoxData& o );


    // copy and assignment
//    StyleBoxData(const StyleBoxData &other);
//    const StyleBoxData &operator = (const StyleBoxData &other);

    bool operator==(const StyleBoxData& o) const;
    bool operator!=(const StyleBoxData& o) const {
        return !(*this == o);
    }

    Length width;
    Length height;

    Length min_width;
    Length max_width;

    Length min_height;
    Length max_height;

    Length vertical_align;

    signed int z_index :31;
    bool z_auto        : 1;
};

//------------------------------------------------
// Random visual rendering model attributes. Not inherited.

enum EOverflow {
    OVISIBLE, OHIDDEN, OSCROLL, OAUTO
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

enum EUnicodeBidi {
    UBNormal, Embed, Override
};

class StyleVisualData : public Shared<StyleVisualData>
{
public:
    StyleVisualData();

    ~StyleVisualData();

    StyleVisualData(const StyleVisualData& o );

    bool operator==( const StyleVisualData &o ) const {
	return ( clip == o.clip &&
		 counter_increment == o.counter_increment &&
		 counter_reset == o.counter_reset &&
		 palette == o.palette );
    }
    bool operator!=( const StyleVisualData &o ) const {
        return !(*this == o);
    }

    LengthBox clip;
    unsigned textDecoration : 4; // Text decorations defined *only* by this element.

    short counter_increment; //ok, so these are not visual mode spesific
    short counter_reset;     //can't go to inherited, since these are not inherited

    QPalette palette;      //widget styling with IE attributes

};

//------------------------------------------------
enum EBackgroundRepeat {
    REPEAT, REPEAT_X, REPEAT_Y, NO_REPEAT
};



class StyleBackgroundData : public Shared<StyleBackgroundData>
{
public:
    StyleBackgroundData();
    ~StyleBackgroundData() {}
    StyleBackgroundData(const StyleBackgroundData& o );

    bool operator==(const StyleBackgroundData& o) const;
    bool operator!=(const StyleBackgroundData &o) const {
	return !(*this == o);
    }

    QColor color;
    CachedImage *image;

    Length x_position;
    Length y_position;
    BorderValue outline;
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
  NORMAL, PRE, NOWRAP, KHTML_NOWRAP
};

enum ETextAlign {
    TAAUTO, LEFT, RIGHT, CENTER, JUSTIFY, KONQ_CENTER
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

class StyleInheritedData : public Shared<StyleInheritedData>
{
    StyleInheritedData& operator=(const StyleInheritedData&);
public:
    StyleInheritedData();
    ~StyleInheritedData();
    StyleInheritedData(const StyleInheritedData& o );

    bool operator==(const StyleInheritedData& o) const;
    bool operator != ( const StyleInheritedData &o ) const {
	return !(*this == o);
    }

    Length indent;
    // could be packed in a short but doesn't
    // make a difference currently because of padding
    Length line_height;

    CachedImage *style_image;

    khtml::Font font;
    QColor color;
    QColor decoration_color;

    short border_hspacing;
    short border_vspacing;
};


enum EEmptyCell {
    SHOW, HIDE
};

enum ECaptionSide
{
    CAPTOP, CAPBOTTOM
};


enum EListStyleType {
     DISC, CIRCLE, SQUARE, LDECIMAL, DECIMAL_LEADING_ZERO,
     LOWER_ROMAN, UPPER_ROMAN, LOWER_GREEK,
     LOWER_ALPHA, LOWER_LATIN, UPPER_ALPHA, UPPER_LATIN,
     HEBREW, ARMENIAN, GEORGIAN, CJK_IDEOGRAPHIC,
     HIRAGANA, KATAKANA, HIRAGANA_IROHA, KATAKANA_IROHA, LNONE
};

enum EListStylePosition { OUTSIDE, INSIDE };

enum EVisibility { VISIBLE, HIDDEN, COLLAPSE };

enum ECursor {
    CURSOR_AUTO, CURSOR_CROSS, CURSOR_DEFAULT, CURSOR_POINTER, CURSOR_PROGRESS,  CURSOR_MOVE,
    CURSOR_E_RESIZE, CURSOR_NE_RESIZE, CURSOR_NW_RESIZE, CURSOR_N_RESIZE, CURSOR_SE_RESIZE, CURSOR_SW_RESIZE,
    CURSOR_S_RESIZE, CURSOR_W_RESIZE, CURSOR_TEXT, CURSOR_WAIT, CURSOR_HELP
};

enum EUserInput {
    UI_ENABLED, UI_DISABLED, UI_NONE
};

enum ContentType {
    CONTENT_NONE, CONTENT_OBJECT, CONTENT_TEXT,
    CONTENT_ATTR
};

struct ContentData {
    ContentData() : _contentType( CONTENT_NONE ), _nextContent(0) {}
    ~ContentData();
    void clearContent();

    ContentType _contentType;

    DOM::DOMStringImpl* contentText()
    { if (_contentType == CONTENT_TEXT) return _content.text; return 0; }
    CachedObject* contentObject()
    { if (_contentType == CONTENT_OBJECT) return _content.object; return 0; }

    union {
        CachedObject* object;
        DOM::DOMStringImpl* text;
        // counters...
    } _content ;

    ContentData* _nextContent;
};

//------------------------------------------------

enum EDisplay {
    INLINE, BLOCK, LIST_ITEM, RUN_IN,
    COMPACT, INLINE_BLOCK, TABLE, INLINE_TABLE,
    TABLE_ROW_GROUP, TABLE_HEADER_GROUP, TABLE_FOOTER_GROUP, TABLE_ROW,
    TABLE_COLUMN_GROUP, TABLE_COLUMN, TABLE_CELL,
    TABLE_CAPTION, NONE
};

class RenderStyle : public Shared<RenderStyle>
{
    friend class CSSStyleSelector;
public:
    static void cleanup();

    // static pseudo styles. Dynamic ones are produced on the fly.
    enum PseudoId { NOPSEUDO, FIRST_LINE, FIRST_LETTER, BEFORE, AFTER, SELECTION };

protected:

// !START SYNC!: Keep this in sync with the copy constructor in render_style.cpp

    // inherit
    struct InheritedFlags {
    // 32 bit inherited, don't add to the struct, or the operator will break.
	bool operator==( const InheritedFlags &other ) const
        {    return _iflags ==other._iflags;    }
	bool operator!=( const InheritedFlags &other ) const
        {    return _iflags != other._iflags;   }

        union {
            struct {
                EEmptyCell _empty_cells : 1 ;
                ECaptionSide _caption_side : 1;
                EListStyleType _list_style_type : 5 ;
                EListStylePosition _list_style_position :1;

                EVisibility _visibility : 2;
                ETextAlign _text_align : 3;
                ETextTransform _text_transform : 2;
                unsigned _text_decorations : 4;
                ECursor _cursor_style : 5;

                EDirection _direction : 1;
                bool _border_collapse : 1 ;
                EWhiteSpace _white_space : 2;
                // non CSS2 inherited
                bool _visuallyOrdered : 1;
                bool _htmlHacks :1;
                EUserInput _user_input : 2;
            } f;
            Q_UINT32 _iflags;
        };
    } inherited_flags;

// don't inherit
    struct NonInheritedFlags {
    // 32 bit non-inherited, don't add to the struct, or the operator will break.
	bool operator==( const NonInheritedFlags &other ) const
        {   return _niflags == other._niflags;    }
	bool operator!=( const NonInheritedFlags &other ) const
        {   return _niflags != other._niflags;    }

        union {
            struct {
                EDisplay _display : 5;
                EBackgroundRepeat _bg_repeat : 2;
                bool _bg_attachment : 1;
                EOverflow _overflow : 4 ;
                EVerticalAlign _vertical_align : 4;
                EClear _clear : 2;
                EPosition _position : 2;
                EFloat _floating : 2;
                ETableLayout _table_layout : 1;
                bool _flowAroundFloats :1;

                PseudoId _styleType : 3;
		bool _affectedByHover : 1;
		bool _affectedByActive : 1;
                bool _hasClip : 1;
                EUnicodeBidi _unicodeBidi : 2;
            } f;
            Q_UINT32 _niflags;
        };
    } noninherited_flags;

// non-inherited attributes
    DataRef<StyleBoxData> box;
    DataRef<StyleVisualData> visual;
    DataRef<StyleBackgroundData> background;
    DataRef<StyleSurroundData> surround;

// inherited attributes
    DataRef<StyleInheritedData> inherited;

// list of associated pseudo styles
    RenderStyle* pseudoStyle;

    // added this here, so we can get rid of the vptr in this class.
    // makes up for the same size.
    ContentData *content;
// !END SYNC!

// static default style
    static RenderStyle* _default;

private:
    RenderStyle(const RenderStyle*) {}

protected:
    void setBitDefaults()
    {
        inherited_flags.f._empty_cells = initialEmptyCells();
        inherited_flags.f._caption_side = initialCaptionSide();
	inherited_flags.f._list_style_type = initialListStyleType();
	inherited_flags.f._list_style_position = initialListStylePosition();
	inherited_flags.f._visibility = initialVisibility();
	inherited_flags.f._text_align = initialTextAlign();
	inherited_flags.f._text_transform = initialTextTransform();
	inherited_flags.f._text_decorations = initialTextDecoration();
	inherited_flags.f._cursor_style = initialCursor();
	inherited_flags.f._direction = initialDirection();
	inherited_flags.f._border_collapse = initialBorderCollapse();
	inherited_flags.f._white_space = initialWhiteSpace();
	inherited_flags.f._visuallyOrdered = false;
	inherited_flags.f._htmlHacks=false;
	inherited_flags.f._user_input = UI_NONE;

        noninherited_flags.f._display = initialDisplay();
	noninherited_flags.f._bg_repeat = initialBackgroundRepeat();
	noninherited_flags.f._bg_attachment = initialBackgroundAttachment();
	noninherited_flags.f._overflow = initialOverflow();
	noninherited_flags.f._vertical_align = initialVerticalAlign();
	noninherited_flags.f._clear = initialClear();
	noninherited_flags.f._position = initialPosition();
	noninherited_flags.f._floating = initialFloating();
	noninherited_flags.f._table_layout = initialTableLayout();
	noninherited_flags.f._flowAroundFloats= initialFlowAroundFloats();
	noninherited_flags.f._styleType = NOPSEUDO;
	noninherited_flags.f._affectedByHover = false;
	noninherited_flags.f._affectedByActive = false;
	noninherited_flags.f._hasClip = false;
	noninherited_flags.f._unicodeBidi = initialUnicodeBidi();
    }

public:

    RenderStyle();
    // used to create the default style.
    RenderStyle(bool);
    RenderStyle(const RenderStyle&);

    ~RenderStyle();

    void inheritFrom(const RenderStyle* inheritParent);

    PseudoId styleType() { return  noninherited_flags.f._styleType; }

    RenderStyle* getPseudoStyle(PseudoId pi);
    RenderStyle* addPseudoStyle(PseudoId pi);
    void removePseudoStyle(PseudoId pi);

    bool affectedByHoverRules() const { return  noninherited_flags.f._affectedByHover; }
    bool affectedByActiveRules() const { return  noninherited_flags.f._affectedByActive; }

    void setAffectedByHoverRules(bool b) {  noninherited_flags.f._affectedByHover = b; }
    void setAffectedByActiveRules(bool b) {  noninherited_flags.f._affectedByActive = b; }

    bool operator==(const RenderStyle& other) const;
    bool        isFloating() const { return !(noninherited_flags.f._floating == FNONE); }
    bool        hasBorder() const { return surround->border.hasBorder(); }

    bool visuallyOrdered() const { return inherited_flags.f._visuallyOrdered; }
    void setVisuallyOrdered(bool b) {  inherited_flags.f._visuallyOrdered = b; }

// attribute getter methods

    EDisplay 	display() const { return noninherited_flags.f._display; }

    Length  	left() const {  return surround->offset.left; }
    Length  	right() const {  return surround->offset.right; }
    Length  	top() const {  return surround->offset.top; }
    Length  	bottom() const {  return surround->offset.bottom; }

    EPosition 	position() const { return  noninherited_flags.f._position; }
    EFloat  	floating() const { return  noninherited_flags.f._floating; }

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
    { if(background->outline.style == BNONE) return 0; return background->outline.width; }
    EBorderStyle    outlineStyle() const {  return background->outline.style; }
    const QColor &  	    outlineColor() const {  return background->outline.color; }

    EOverflow overflow() const { return  noninherited_flags.f._overflow; }
    bool hidesOverflow() const { return overflow() != OVISIBLE; }
    bool scrollsOverflow() const { return overflow() == OSCROLL || overflow() == OAUTO; }

    EVisibility visibility() const { return inherited_flags.f._visibility; }
    EVerticalAlign verticalAlign() const { return  noninherited_flags.f._vertical_align; }
    Length verticalAlignLength() const { return box->vertical_align; }

    Length clipLeft() const { return visual->clip.left; }
    Length clipRight() const { return visual->clip.right; }
    Length clipTop() const { return visual->clip.top; }
    Length clipBottom() const { return visual->clip.bottom; }
    bool hasClip() const { return noninherited_flags.f._hasClip; }

    EUnicodeBidi unicodeBidi() const { return noninherited_flags.f._unicodeBidi; }

    EClear clear() const { return  noninherited_flags.f._clear; }
    ETableLayout tableLayout() const { return  noninherited_flags.f._table_layout; }

    const QFont & font() const { return inherited->font.f; }
    // use with care. call font->update() after modifications
    const Font &htmlFont() { return inherited->font; }
    const QFontMetrics & fontMetrics() const { return inherited->font.fm; }

    const QColor & color() const { return inherited->color; }
    Length textIndent() const { return inherited->indent; }
    ETextAlign textAlign() const { return inherited_flags.f._text_align; }
    ETextTransform textTransform() const { return inherited_flags.f._text_transform; }
    int textDecorationsInEffect() const { return inherited_flags.f._text_decorations; }
    int textDecoration() const { return visual->textDecoration; }
    const QColor &textDecorationColor() const { return inherited->decoration_color; }
    int wordSpacing() const { return inherited->font.wordSpacing; }
    int letterSpacing() const { return inherited->font.letterSpacing; }

    EDirection direction() const { return inherited_flags.f._direction; }
    Length lineHeight() const { return inherited->line_height; }

    EWhiteSpace whiteSpace() const { return inherited_flags.f._white_space; }


    const QColor & backgroundColor() const { return background->color; }
    CachedImage *backgroundImage() const { return background->image; }
    EBackgroundRepeat backgroundRepeat() const { return  noninherited_flags.f._bg_repeat; }
    bool backgroundAttachment() const { return  noninherited_flags.f._bg_attachment; }
    Length backgroundXPosition() const { return background->x_position; }
    Length backgroundYPosition() const { return background->y_position; }

    // returns true for collapsing borders, false for separate borders
    bool borderCollapse() const { return inherited_flags.f._border_collapse; }
    short borderHorizontalSpacing() const { return inherited->border_hspacing; }
    short borderVerticalSpacing() const { return inherited->border_vspacing; }
    EEmptyCell emptyCells() const { return inherited_flags.f._empty_cells; }
    ECaptionSide captionSide() const { return inherited_flags.f._caption_side; }

    short counterIncrement() const { return visual->counter_increment; }
    short counterReset() const { return visual->counter_reset; }

    EListStyleType listStyleType() const { return inherited_flags.f._list_style_type; }
    CachedImage *listStyleImage() const { return inherited->style_image; }
    EListStylePosition listStylePosition() const { return inherited_flags.f._list_style_position; }

    Length marginTop() const { return surround->margin.top; }
    Length marginBottom() const {  return surround->margin.bottom; }
    Length marginLeft() const {  return surround->margin.left; }
    Length marginRight() const {  return surround->margin.right; }

    Length paddingTop() const {  return surround->padding.top; }
    Length paddingBottom() const {  return surround->padding.bottom; }
    Length paddingLeft() const { return surround->padding.left; }
    Length paddingRight() const {  return surround->padding.right; }

    ECursor cursor() const { return inherited_flags.f._cursor_style; }

    EUserInput userInput() const { return inherited_flags.f._user_input; }

// attribute setter methods

    void setDisplay(EDisplay v) {  noninherited_flags.f._display = v; }
    void setPosition(EPosition v) {  noninherited_flags.f._position = v; }
    void setFloating(EFloat v) {  noninherited_flags.f._floating = v; }

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

    void resetBorderTop() { SET_VAR(surround, border.top, BorderValue()) }
    void resetBorderRight() { SET_VAR(surround, border.right, BorderValue()) }
    void resetBorderBottom() { SET_VAR(surround, border.bottom, BorderValue()) }
    void resetBorderLeft() { SET_VAR(surround, border.left, BorderValue()) }
    void resetOutline() { SET_VAR(background, outline, BorderValue()) }

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
    void setOutlineWidth(unsigned short v) {  SET_VAR(background,outline.width,v) }
    void setOutlineStyle(EBorderStyle v)   {  SET_VAR(background,outline.style,v) }
    void setOutlineColor(const QColor & v) {  SET_VAR(background,outline.color,v) }

    void setOverflow(EOverflow v) {  noninherited_flags.f._overflow = v; }
    void setVisibility(EVisibility v) { inherited_flags.f._visibility = v; }
    void setVerticalAlign(EVerticalAlign v) { noninherited_flags.f._vertical_align = v; }
    void setVerticalAlignLength(Length l) { SET_VAR(box, vertical_align, l ) }

    void setClipLeft(Length v) { SET_VAR(visual,clip.left,v) }
    void setClipRight(Length v) { SET_VAR(visual,clip.right,v) }
    void setClipTop(Length v) { SET_VAR(visual,clip.top,v) }
    void setClipBottom(Length v) { SET_VAR(visual,clip.bottom,v) }
    void setClip( Length top, Length right, Length bottom, Length left );
    void setHasClip( bool b ) { noninherited_flags.f._hasClip = b; }

    void setUnicodeBidi( EUnicodeBidi b ) { noninherited_flags.f._unicodeBidi = b; }

    void setClear(EClear v) {  noninherited_flags.f._clear = v; }
    void setTableLayout(ETableLayout v) {  noninherited_flags.f._table_layout = v; }
    bool setFontDef(const khtml::FontDef & v) {
        // bah, this doesn't compare pointers. broken! (Dirk)
        if (!(inherited->font.fontDef == v)) {
            inherited.access()->font = Font( v );
            return true;
        }
        return false;
    }

    void setColor(const QColor & v) { SET_VAR(inherited,color,v) }
    void setTextIndent(Length v) { SET_VAR(inherited,indent,v) }
    void setTextAlign(ETextAlign v) { inherited_flags.f._text_align = v; }
    void setTextTransform(ETextTransform v) { inherited_flags.f._text_transform = v; }
    void addToTextDecorationsInEffect(int v) { inherited_flags.f._text_decorations |= v; }
    void setTextDecorationsInEffect(int v) { inherited_flags.f._text_decorations = v; }
    void setTextDecoration(unsigned v) { SET_VAR(visual, textDecoration, v); }
    void setTextDecorationColor(const QColor &v) { SET_VAR(inherited,decoration_color,v) }
    void setDirection(EDirection v) { inherited_flags.f._direction = v; }
    void setLineHeight(Length v) { SET_VAR(inherited,line_height,v) }

    void setWhiteSpace(EWhiteSpace v) { inherited_flags.f._white_space = v; }

    void setWordSpacing(int v) { SET_VAR(inherited,font.wordSpacing,v) }
    void setLetterSpacing(int v) { SET_VAR(inherited,font.letterSpacing,v) }

    void setBackgroundColor(const QColor & v) {  SET_VAR(background,color,v) }
    void setBackgroundImage(CachedImage *v) {  SET_VAR(background,image,v) }
    void setBackgroundRepeat(EBackgroundRepeat v) {  noninherited_flags.f._bg_repeat = v; }
    void setBackgroundAttachment(bool scroll) {  noninherited_flags.f._bg_attachment = scroll; }
    void setBackgroundXPosition(Length v) {  SET_VAR(background,x_position,v) }
    void setBackgroundYPosition(Length v) {  SET_VAR(background,y_position,v) }

    void setBorderCollapse(bool collapse) { inherited_flags.f._border_collapse = collapse; }
    void setBorderHorizontalSpacing(short v) { SET_VAR(inherited,border_hspacing,v) }
    void setBorderVerticalSpacing(short v) { SET_VAR(inherited,border_vspacing,v) }

    void setEmptyCells(EEmptyCell v) { inherited_flags.f._empty_cells = v; }
    void setCaptionSide(ECaptionSide v) { inherited_flags.f._caption_side = v; }


    void setCounterIncrement(short v) {  SET_VAR(visual,counter_increment,v) }
    void setCounterReset(short v) {  SET_VAR(visual,counter_reset,v) }

    void setListStyleType(EListStyleType v) { inherited_flags.f._list_style_type = v; }
    void setListStyleImage(CachedImage *v) {  SET_VAR(inherited,style_image,v)}
    void setListStylePosition(EListStylePosition v) { inherited_flags.f._list_style_position = v; }

    void resetMargin() { SET_VAR(surround, margin, LengthBox(Fixed)) }
    void setMarginTop(Length v)     {  SET_VAR(surround,margin.top,v) }
    void setMarginBottom(Length v)  {  SET_VAR(surround,margin.bottom,v) }
    void setMarginLeft(Length v)    {  SET_VAR(surround,margin.left,v) }
    void setMarginRight(Length v)   {  SET_VAR(surround,margin.right,v) }

    void resetPadding() { SET_VAR(surround, padding, LengthBox(Variable)) }
    void setPaddingTop(Length v)    {  SET_VAR(surround,padding.top,v) }
    void setPaddingBottom(Length v) {  SET_VAR(surround,padding.bottom,v) }
    void setPaddingLeft(Length v)   {  SET_VAR(surround,padding.left,v) }
    void setPaddingRight(Length v)  {  SET_VAR(surround,padding.right,v) }

    void setCursor( ECursor c ) { inherited_flags.f._cursor_style = c; }

    void setUserInput(EUserInput ui) { inherited_flags.f._user_input = ui; }

    bool htmlHacks() const { return inherited_flags.f._htmlHacks; }
    void setHtmlHacks(bool b=true) { inherited_flags.f._htmlHacks = b; }

    bool flowAroundFloats() const { return  noninherited_flags.f._flowAroundFloats; }
    void setFlowAroundFloats(bool b=true) {  noninherited_flags.f._flowAroundFloats = b; }

    int zIndex() const { return box->z_auto? 0 : box->z_index; }
    void setZIndex(int v) { SET_VAR(box,z_auto,false ); SET_VAR(box, z_index, v); }
    bool hasAutoZIndex() const { return box->z_auto; }
    void setHasAutoZIndex() { SET_VAR(box, z_auto, true ); }

    QPalette palette() const { return visual->palette; }
    void setPaletteColor(QPalette::ColorGroup g, QColorGroup::ColorRole r, const QColor& c);
    void resetPalette() // Called when the desktop color scheme changes.
    {
        const_cast<StyleVisualData *>(visual.get())->palette = QApplication::palette();
    }


    ContentData* contentData() const { return content; }
    bool contentDataEquivalent(RenderStyle* otherStyle);
    void setContent(DOM::DOMStringImpl* s, bool add);
    void setContent(CachedObject* o, bool add);

    bool inheritedNotEqual( RenderStyle *other ) const;

    enum Diff { Equal, NonVisible = Equal, Visible, Position, Layout, CbLayout };
    Diff diff( const RenderStyle *other ) const;

#ifdef ENABLE_DUMP
    QString createDiff( const RenderStyle &parent ) const;
#endif

    // Initial values for all the properties
    static bool initialBackgroundAttachment() { return true; }
    static EBackgroundRepeat initialBackgroundRepeat() { return REPEAT; }
    static bool initialBorderCollapse() { return false; }
    static EBorderStyle initialBorderStyle() { return BNONE; }
    static ECaptionSide initialCaptionSide() { return CAPTOP; }
    static EClear initialClear() { return CNONE; }
    static EDirection initialDirection() { return LTR; }
    static EDisplay initialDisplay() { return INLINE; }
    static EEmptyCell initialEmptyCells() { return SHOW; }
    static EFloat initialFloating() { return FNONE; }
    static EListStylePosition initialListStylePosition() { return OUTSIDE; }
    static EListStyleType initialListStyleType() { return DISC; }
    static EOverflow initialOverflow() { return OVISIBLE; }
    static EPosition initialPosition() { return STATIC; }
    static ETableLayout initialTableLayout() { return TAUTO; }
    static EUnicodeBidi initialUnicodeBidi() { return UBNormal; }
    static ETextTransform initialTextTransform() { return TTNONE; }
    static EVisibility initialVisibility() { return VISIBLE; }
    static EWhiteSpace initialWhiteSpace() { return NORMAL; }
    static Length initialBackgroundXPosition() { return Length(); }
    static Length initialBackgroundYPosition() { return Length(); }
    static short initialBorderHorizontalSpacing() { return 0; }
    static short initialBorderVerticalSpacing() { return 0; }
    static ECursor initialCursor() { return CURSOR_AUTO; }
    static QColor initialColor() { return Qt::black; }
    static CachedImage* initialBackgroundImage() { return 0; }
    static CachedImage* initialListStyleImage() { return 0; }
    static unsigned short initialBorderWidth() { return 3; }
    static int initialLetterWordSpacing() { return 0; }
    static Length initialSize() { return Length(); }
    static Length initialMinSize() { return Length(0, Fixed); }
    static Length initialMaxSize() { return Length(UNDEFINED, Fixed); }
    static Length initialOffset() { return Length(); }
    static Length initialMargin() { return Length(Fixed); }
    static Length initialPadding() { return Length(Variable); }
    static Length initialTextIndent() { return Length(Fixed); }
    static EVerticalAlign initialVerticalAlign() { return BASELINE; }
    static int initialWidows() { return 2; }
    static int initialOrphans() { return 2; }
    static Length initialLineHeight() { return Length(-100, Percent); }
    static ETextAlign initialTextAlign() { return TAAUTO; }
    static ETextDecoration initialTextDecoration() { return TDNONE; }
    static bool initialFlowAroundFloats() { return false; }
    static int initialOutlineOffset() { return 0; }
    static float initialOpacity() { return 1.0f; }
};


} // namespace

#endif

