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
using namespace khtml;

#include <kglobal.h>

// most of these sizes are standard X font sizes, so all of our fonts
// display nicely.

const int defaultFontSizes[MAXFONTSIZES] = { 7, 8, 10, 12, 14, 18, 24, 28, 34, 40, 48, 56, 68, 82, 100 };

Settings::Settings()
{
    memcpy( m_fontSizes, defaultFontSizes, sizeof(m_fontSizes) );
    standardFamilies = new QString[6];

    charset	  = QFont::Latin1;

    resetStandardFamilies();
}

Settings::~Settings()
{
    delete [] standardFamilies;
}

void Settings::setFontSizes(const int *newFontSizes, int numFontSizes)
{
    if(numFontSizes > MAXFONTSIZES) numFontSizes = MAXFONTSIZES;
    memcpy( m_fontSizes, newFontSizes, sizeof(int)*numFontSizes );
}

const int *Settings::fontSizes() const
{
    return m_fontSizes;
}

void Settings::resetFontSizes(void)
{
    memcpy( m_fontSizes, defaultFontSizes, sizeof(m_fontSizes) );
}

Settings::Settings( const Settings &s )
{
    memcpy( m_fontSizes, s.m_fontSizes, sizeof(m_fontSizes) );
    standardFamilies = new QString[5];

    for(int i = 0; i < 5; i++)
	standardFamilies[i] = s.standardFamilies[i];

    charset       = s.charset;
}

const Settings &Settings::operator=( const Settings &s )
{
    memcpy( m_fontSizes, s.m_fontSizes, sizeof(m_fontSizes) );

    for(int i = 0; i < 5; i++)
	standardFamilies[i] = s.standardFamilies[i];

    charset       = s.charset;

    return *this;
}

void Settings::resetStandardFamilies()
{
    standardFamilies[0] = "helvetica";
    standardFamilies[1] = "times";
    standardFamilies[2] = "helvetica";
    standardFamilies[3] = "times";
    standardFamilies[4] = "comic";
    standardFamilies[5] = "fixed";
}

void Settings::setDefaultFamily(const QString& family)
{
    standardFamilies[0] = family;
}

void Settings::setMonoSpaceFamily(const QString& family)
{
    standardFamilies[5] = family;
}
