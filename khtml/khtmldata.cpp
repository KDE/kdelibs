/* 
    This file is part of the KDE libraries

    Copyright (C) 1997 Martin Jones (mjones@kde.org)
              (C) 1997 Torben Weis (weis@kde.org)

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
// KDE HTML Widget -- Settings
// $Id$


#include "khtmldata.h"
#include <kglobal.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// most of these sizes are standard X font sizes, so all of our fonts
// display nicely.

const int defaultFontSizes[MAXFONTSIZES] = { 6, 8, 10, 12, 14, 18, 24 };

HTMLSettings::HTMLSettings()
{
    memcpy( fontSizes, defaultFontSizes, sizeof(fontSizes) );
    memcpy( fixedFontSizes, defaultFontSizes, sizeof(fixedFontSizes) );
    fontBaseColor = Qt::black;
    fontBaseFace  = KGlobal::generalFont().family();

    fixedFontFace = KGlobal::fixedFont().family();

    linkColor     = Qt::blue;
    vLinkColor    = Qt::darkMagenta;

    bgColor       = Qt::lightGray;

    charset	  = QFont::Latin1;
    
    underlineLinks = true;
}

void HTMLSettings::setFontSizes(const int *newFontSizes, const int *newFixedFontSizes)
{
    memcpy( fontSizes, newFontSizes, sizeof(fontSizes) );
    if (newFixedFontSizes)
    {
       memcpy( fixedFontSizes, newFixedFontSizes, sizeof(fixedFontSizes) );
    }
    else
    {
       memcpy( fixedFontSizes, newFontSizes, sizeof(fixedFontSizes) );
    }    
}                                    

void HTMLSettings::getFontSizes(int *oldFontSizes, int *oldFixedFontSizes)
{
    memcpy( oldFontSizes, fontSizes, sizeof(fontSizes) );
    if (oldFixedFontSizes)
    {
       memcpy( oldFontSizes, fontSizes, sizeof(fontSizes) );
    }
}                                    

void HTMLSettings::resetFontSizes()
{
    memcpy( fontSizes, defaultFontSizes, sizeof(fontSizes) );
    memcpy( fixedFontSizes, defaultFontSizes, sizeof(fixedFontSizes) );
}                                    

HTMLSettings::HTMLSettings( const HTMLSettings &s )
{
    memcpy( fontSizes, s.fontSizes, sizeof(fontSizes) );
    memcpy( fixedFontSizes, s.fixedFontSizes, sizeof(fixedFontSizes) );
    fontBaseColor = s.fontBaseColor;
    fontBaseFace  = s.fontBaseFace;

    fixedFontFace = s.fixedFontFace;

    linkColor     = s.linkColor;
    vLinkColor    = s.vLinkColor;

    bgColor       = s.bgColor;
    
    charset       = s.charset;

    underlineLinks = s.underlineLinks;
}

const HTMLSettings &HTMLSettings::operator=( const HTMLSettings &s )
{
    memcpy( fontSizes, s.fontSizes, sizeof(fontSizes) );
    memcpy( fixedFontSizes, s.fixedFontSizes, sizeof(fixedFontSizes) );
    fontBaseColor = s.fontBaseColor;
    fontBaseFace  = s.fontBaseFace;

    fixedFontFace = s.fixedFontFace;

    linkColor     = s.linkColor;
    vLinkColor    = s.vLinkColor;

    bgColor       = s.bgColor;
    
    charset       = s.charset;

    underlineLinks = s.underlineLinks;

    return *this;
}

const int HTMLSettings::fontBaseSize = 3;
