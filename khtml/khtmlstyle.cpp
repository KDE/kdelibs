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
// $Id:  $

#include "khtmlstyle.h"

static const int UNDEFINED= -1;

CSSStyleSheet::CSSStyleSheet(const HTMLSettings *_settings)
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
        newStyle->font.style = CSSStyleFont::stNormal;
        newStyle->font.weight = CSSStyleFont::Normal;
        newStyle->font.size = settings->fontBaseSize;
        newStyle->font.fp = 0;

        newStyle->text.transform = CSSStyleText::transNormal;
        newStyle->text.halign = CSSStyleText::halLeft;
        newStyle->text.indent = 0;
        newStyle->text.height = CSSStyleText::heightNormal;

        newStyle->list.type = CSSStyleList::liDisc;
    }

    newStyle->text.valign = CSSStyleText::valBaseline;
    newStyle->box.marginF = 0;
    newStyle->box.marginP = UNDEFINED;
    newStyle->box.paddingF = 0;
    newStyle->box.paddingP = UNDEFINED;
    newStyle->box.borderColor = newStyle->font.color;
    newStyle->box.border = 2;
    newStyle->box.borderStyle = CSSStyleBox::borderNone;
    newStyle->bgtransparant = true;
    newStyle->width = UNDEFINED;
    newStyle->width_percent = UNDEFINED;
    newStyle->height = UNDEFINED;
    newStyle->height_percent = UNDEFINED;
    
    return newStyle;
}

void
CSSStyleSheet::getStyle(int tagID, HTMLStackElem *tagStack, 
                        CSSStyle *styleElem, 
                        const char *klass, const char *id)
{
}

void
CSSStyleSheet::addStyle(CSSStyle *currentStyle, const char *CSSString)
{
}
