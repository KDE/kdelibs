/*
    This file is part of the KDE libraries

    Copyright (C) 1999 Waldo Bastian (bastian@kde.org)

    Based on the Chimera CSS implementation of
                       John Kilburg <john@cs.unlv.edu>

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

#include <ctype.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

#include "khtmlstyle.h"
#include "htmlhashes.h"

#include <qstring.h>
#include <dom_stringimpl.h>
#include <khtmllayout.h>
using namespace DOM;
using namespace khtml;


CSSStyleSheet::CSSStyleSheet(const khtml::Settings *_settings)
{
    settings = _settings;
}

CSSStyleSheet::~CSSStyleSheet()
{
}

CSSStyle *
CSSStyleSheet::newStyle(CSSStyle *parentStyle)
{
    CSSStyle *newStyle = new CSSStyle();

    if (parentStyle)
    {
        newStyle->font = parentStyle->font;
        newStyle->text = parentStyle->text;
        newStyle->list = parentStyle->list;
    }
    else
    {
        newStyle->font.decoration = CSSStyleFont::decNormal;
        newStyle->font.color = settings->fontBaseColor;
        newStyle->font.family = settings->fontBaseFace;
        newStyle->font.fixed = false;
        newStyle->font.style = CSSStyleFont::stNormal;
        newStyle->font.weight = CSSStyleFont::Normal;
        newStyle->font.size = settings->fontBaseSize;
        newStyle->font.fp = 0;

        newStyle->text.transform = CSSStyleText::transNormal;
        newStyle->text.halign = CSSStyleText::halLeft;
        newStyle->text.indent = 0;
        newStyle->text.height = CSSStyleText::heightNormal;
        newStyle->text.valign = CSSStyleText::valBaseline;
        newStyle->text.valignOffset = 0;

        newStyle->list.type = CSSStyleList::liDisc;	
    }

    newStyle->box.marginF = 0;
    newStyle->box.marginP = UNDEFINED;
    newStyle->box.paddingF = 0;
    newStyle->box.paddingP = UNDEFINED;
    newStyle->box.borderColor = newStyle->font.color;
    newStyle->box.border = 2;
    newStyle->box.borderStyle = CSSStyleBox::borderNone;
    newStyle->bgtransparent = true;
    newStyle->width = UNDEFINED;
    newStyle->width_percent = UNDEFINED;
    newStyle->height = UNDEFINED;
    newStyle->height_percent = UNDEFINED;
    newStyle->bgcolor = Qt::lightGray;

    return newStyle;
}

void
CSSStyleSheet::getStyle(int /*tagID*/, HTMLStackElem */*tagStack*/,
                        CSSStyle */*styleElem*/,
                        const QChar */*klass*/, const QChar */*id*/)
{
}

void
CSSStyleSheet::addStyle(CSSStyle */*currentStyle*/, const QChar */*CSSString*/)
{
}


// ------------------------------------------------------------------------
const Font *khtml::getFont(CSSStyle *currentStyle)
{

    int fontsize = currentStyle->font.size;
    if ( fontsize < 0 )
	fontsize = 0;
    else if ( fontsize >= MAXFONTSIZES )
	fontsize = MAXFONTSIZES - 1;

    currentStyle->font.size = fontsize;

    khtml::Font f( currentStyle->font.family,
                fontsize,
                currentStyle->font.fixed ? pSettings->fixedFontSizes : pSettings->fontSizes,
                currentStyle->font.weight / 10,
                (currentStyle->font.style != CSSStyleFont::stNormal),
                pSettings->charset );
    f.setTextColor( currentStyle->font.color );
    f.setUnderline( currentStyle->font.decoration == CSSStyleFont::decUnderline );
    f.setStrikeOut( currentStyle->font.decoration == CSSStyleFont::decLineThrough );
    if (currentStyle->text.valign == CSSStyleText::valOffset)
    {
       f.setVOffset( currentStyle->text.valignOffset );
    }

    const khtml::Font *fp = khtml::pFontManager->getFont( f );

    currentStyle->font.fp = fp;

    return fp;
}

#include "qmap.h"

struct HTMLColors {
    QMap<QString,QColor> map;
    HTMLColors()
    {
	map["black"] = "#000000";
	map["green"] = "#008000";
	map["silver"] = "#c0c0c0";
	map["lime"] = "#00ff00";
	map["gray"] = "#808080";
	map["olive"] = "#808000";
	map["white"] = "#ffffff";
	map["yellow"] = "#ffff00";
	map["maroon"] = "#800000";
	map["navy"] = "#000080";
	map["red"] = "#ff0000";
	map["blue"] = "#0000ff";
	map["purple"] = "#800080";
	map["teal"] = "#008080";
	map["fuchsia"] = "#ff00ff";
	map["aqua"] = "#00ffff";
    };
} htmlColors;

void khtml::setNamedColor(QColor &color, const QString name)
{
    // also recognize "color=ffffff"
    if (name[0] != QChar('#') && name.length() == 6)
    {
	bool ok;
        name.toInt(&ok, 16);
	if(ok)
	{
	    QString col("#");
	    col += name;
	    color.setNamedColor(col);
	    return;
	}
    }
    
    if ( name.length() > 4 && name[0] == 'r'
	      && name[1] == 'g' && name[2] == 'b'
	      && name[3] == '(' && name[name.length()-1] == ')')
    {
	// CSS like rgb(r, g, b) style
	DOMString rgb = name.mid(4, name.length()-5);
	QList<Length> *l = rgb.implementation()->toLengthList();
	if(l->count() != 3)
	{
	    delete l;
	    return;
	}
	int r = l->at(0)->width(255);
	if(r < 0) r = 0;
	if(r > 255) r = 255;
	int g = l->at(1)->width(255);
	if(g < 0) g = 0;
	if(g > 255) g = 255;
	int b = l->at(2)->width(255);
	if(b < 0) b = 0;
	if(b > 255) b = 255;
	color.setRgb(r, g, b);
	delete l;
    }
    else
    {
    	const QColor& tc = htmlColors.map[name];
	if (tc.isValid())
    	    color = tc;	
	else
	    color.setNamedColor(name);
    }
}
