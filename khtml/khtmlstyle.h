/* 
    This file is part of the KDE libraries

    Copyright (C) 1999 Waldo Bastian (bastian@kde.org)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
//----------------------------------------------------------------------------
//
// KDE HTML Widget -- Cascading Style Sheets / 1
// $Id$

#ifndef HTMLSTYLE_H
#define HTMLSTYLE_H

#include <qcolor.h>
#include <qlist.h>
#include <qstring.h>
#include <khtmldata.h>
#include <khtmlfont.h>
//
// External Classes
//
///////////////////

// khtmlparser.h
class KHTMLParser;

//
// Internal Classes
//
///////////////////

class HTMLObject;
class HTMLStackElem;
class CSSStyle;
class CSSStyleFont;
class CSSStyleText;
class CSSStyleBox;
class CSSStyleList;

typedef void (KHTMLParser::*blockFunc)(HTMLStackElem *stackElem);

class HTMLStackElem
{
public:
    HTMLStackElem( int _id, 
		   int _level, 
		   CSSStyle * _style,
		   blockFunc _exitFunc, 
		   HTMLObject *_obj,
		   int _miscData1,
		   HTMLStackElem * _next
	) 
	:	id(_id), 
		level(_level),
		style(_style),
		exitFunc(_exitFunc), 
		obj(_obj),
		miscData1(_miscData1), 
		next(_next) 
    { }

    int       id;
    int       level;

    CSSStyle *style;
   	 
    blockFunc exitFunc;
    HTMLObject *obj;
   
    int       miscData1;

    HTMLStackElem *next;
};

class CSSStyleFont
{
public:
    QColor  color;
    QString family;
    enum stEnum { stNormal = 0, stItalic, stOblique };
    int     style;
    enum varEnum { varNormal = 0, varSmallCaps };
    int	    variant;
    enum decEnum { decNormal = 0, decUnderline, decOverline, decLineThrough, 
		   decBlink };
    int     decoration;
    enum weightEnum { Normal=400, Bold=700 };
    int     weight; // 100 - 900, ( <= 500 : Normal, > 500 : Bold )
    int     size;
    const HTMLFont *fp;
};

class CSSStyleText
{
public:
    enum valEnum { valBaseline = 0, valOffset, valTop, valTextTop, valMiddle,
		valBottom, valTextBottom };
    int     valign; 
    int     valignOffset; // sub, super and percentage set valOffset and
                          // should specify with valigOffset the number of 
	                  // pixels the element should be raised above the
	                  // bottomline.
    enum transEnum { transNormal = 0, transCapital, transUpper, transLower};
    int     transform;
    enum halEnum { halLeft = 0, halRight, halCenter, halJustify };
    int     halign;
    int     indent;
    enum heightEnum { heightNormal = 0, heightScaled, heightFixed };
    int     height;    
    int     heightValue; // the scaling factor or fixed height
};

class CSSStyleBox
{
public:
    int     marginF;
    int     marginP;
    int     paddingF;
    int     paddingP;
    int     border;
    QColor  borderColor;
    enum borderEnum { borderNone=0, borderDotted, borderDashed, 
           borderSolid, borderDouble, borderGroove, 
           borderRidge, borderInset, borderOutset };
    int     borderStyle;
};

class CSSStyleList
{
public:
    enum liEnum { liDisc, liCircle, liSquare, liDecimal, liLowRoman, liUpRoman,
    	   liLowAlpha, liUpAlpha, liNone };
    int     type;
    QString url;
};


class CSSStyle
{
public:
    CSSStyleFont font;
    CSSStyleText text;
    CSSStyleBox  box;
    CSSStyleList list;
    bool         bgtransparant;
    QColor       bgcolor;
    int          width;
    int          width_percent;
    int          height;
    int          height_percent;
    enum floatEnum { floatNone=0, floatLeft, floatRight };
    int          floating;
    enum clearEnum { clearNone=0, clearLeft, clearRight, clearAll };
    int          clear;
};


class CSSProperty;
typedef QList<CSSProperty> CSSPropList;
class CSSSelector;
typedef QList<CSSSelector> CSSSelecList;


class CSSStyleSheet
{
public:
    CSSStyleSheet(const HTMLSettings *);
    ~CSSStyleSheet();

    void test(void);
    /*
     * newStyle
     *
     * Creates a new style element based on parentStyle
     *
     */
    CSSStyle *newStyle(CSSStyle *parentStyle);
    
    /*
     * getStyle
     *
     * Fills styleElem in with info from the style sheet
     *
     */
    void getStyle(int tagID, HTMLStackElem *tagStack, 
                  CSSStyle *styleElem, const char *klass, const char *id);

    void addStyle(CSSStyle *currentStyle, const char *CSSString);   

protected:
    const HTMLSettings *settings;    
    
    const char *parseSpace(const char *curP, const char *endP);
    const char *parseToChar(const char *curP, const char *endP, 
                            int c, bool chkws);
    const char *parseAt(const char *curP, const char *endP);
    CSSSelector *parseSelector2(const char *curP, const char *endP);
    CSSSelector *parseSelector1(const char *curP, const char *endP);
    CSSSelecList *parseSelector(const char *curP, const char *endP);
    CSSProperty *parseProperty(const char *curP, const char *endP);
    CSSPropList *parseProperties(const char *curP, const char *endP);
    const char *parseRule(const char *curP, const char *endP);
    void parseSheet(const char *src, int len);
};

#endif // HTMLSTYLE_H
