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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// most of these sizes are standard X font sizes, so all of our fonts
// display nicely.

const int defaultFontSizes[MAXFONTSIZES] = { 7, 8, 10, 12, 14, 18, 24 };

Settings::Settings()
{
    memcpy( m_fontSizes, defaultFontSizes, sizeof(m_fontSizes) );
    fontBaseFace  = KGlobal::generalFont().family();
    fixedFontFace = KGlobal::fixedFont().family();

    charset	  = QFont::Latin1;
}

void Settings::setFontSizes(const int *newFontSizes)
{
    memcpy( m_fontSizes, newFontSizes, sizeof(m_fontSizes) );
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

    fontBaseFace  = s.fontBaseFace;
    fixedFontFace = s.fixedFontFace;

    charset       = s.charset;
}

const Settings &Settings::operator=( const Settings &s )
{
    memcpy( m_fontSizes, s.m_fontSizes, sizeof(m_fontSizes) );

    fontBaseFace  = s.fontBaseFace;
    fixedFontFace = s.fixedFontFace;

    charset       = s.charset;

    return *this;
}
