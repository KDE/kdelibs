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

#ifndef __HTMLDATA_H__
#define __HTMLDATA_H__

#include <qcolor.h>
#include <qcursor.h>
#include <qstring.h>
#include <qfont.h>

// Also defined in khtmlfont.h
#ifndef MAXFONTSIZES
#define MAXFONTSIZES 7
#endif

class HTMLSettings
{
public:
    HTMLSettings();
    HTMLSettings( const HTMLSettings & );

    const HTMLSettings &operator=( const HTMLSettings & );

    void setFontSizes(const int *newFontSizes, const int *newFixedFontSizes);
    void getFontSizes(int *newFontSizes, int *newFixedFontSizes);
    void resetFontSizes(void);

    int     fontSizes[MAXFONTSIZES];
    int     fixedFontSizes[MAXFONTSIZES];
    QColor  fontBaseColor;
    QString fontBaseFace;

    QString fixedFontFace;

    QColor  linkColor;
    QColor  vLinkColor;

    QColor  bgColor;

    QFont::CharSet charset; 

    bool    underlineLinks;

    static const int fontBaseSize;
};

extern HTMLSettings* pSettings;

#endif

