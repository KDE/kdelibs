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
#include <kglobalsettings.h>

#include <assert.h>

// most of these sizes are standard X font sizes, so all of our fonts
// display nicely.

const int defaultFontSizes[MAXFONTSIZES] = { 7, 8, 10, 12, 14, 18, 24, 28, 34, 40, 48, 56, 68, 82, 100 };

Settings::Settings()
{
    resetFontSizes();

    standardFamilies = new QString[6];

    charset = KGlobalSettings::generalFont().charSet();

    resetStandardFamilies();
}

Settings::~Settings()
{
    delete [] standardFamilies;
}

void Settings::setFontSizes(const QValueList<int> &_newFontSizes )
{
    QValueList<int> newFontSizes;
    newFontSizes = _newFontSizes;
    while ( newFontSizes.count() > m_fontSizes.count() )
      newFontSizes.remove( newFontSizes.fromLast() );

    QValueList<int>::ConstIterator it = newFontSizes.begin();
    QValueList<int>::ConstIterator end = newFontSizes.end();
    int i = 0;
    for (; it != end; it++ )
      m_fontSizes[ i++ ] = *it;
}

const QValueList<int> &Settings::fontSizes() const
{
    return m_fontSizes;
}

void Settings::resetFontSizes(void)
{
    m_fontSizes.clear();
    for ( int i = 0; i < MAXFONTSIZES; i++ )
      m_fontSizes << defaultFontSizes[ i ];
}

Settings::Settings( const Settings &s )
{
    m_fontSizes = s.m_fontSizes;
    standardFamilies = new QString[5];

    for(int i = 0; i < 5; i++)
	standardFamilies[i] = s.standardFamilies[i];

    charset       = s.charset;
}

const Settings &Settings::operator=( const Settings &s )
{
    m_fontSizes = s.m_fontSizes;

    for(int i = 0; i < 5; i++)
	standardFamilies[i] = s.standardFamilies[i];

    charset       = s.charset;

    return *this;
}

void Settings::resetStandardFamilies()
{
    standardFamilies[0] = KGlobalSettings::generalFont().family();
    standardFamilies[1] = "times";
    standardFamilies[2] = "helvetica";
    standardFamilies[3] = "times";
    standardFamilies[4] = "comic";
    standardFamilies[5] = KGlobalSettings::fixedFont().family();
}

void Settings::setDefaultFamily(const QString& family)
{
    standardFamilies[0] = family;
}

void Settings::setMonoSpaceFamily(const QString& family)
{
    standardFamilies[5] = family;
}
