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

#include <qfont.h>
#include <qvaluelist.h>
class QString;

#define MAXFONTSIZES 15

namespace khtml
{

/**
 * @internal
 */
class Settings
{
public:
    Settings();
    Settings( const Settings & );
    ~Settings();

    const Settings &operator=( const Settings & );

    void setFontSizes(const QValueList<int> &newFontSizes );
    const QValueList<int> &fontSizes() const;
    void resetFontSizes();

    void resetStandardFamilies();
    void setDefaultFamily(const QString& family);
    void setMonoSpaceFamily(const QString& family);

    QFont::CharSet charset;

    const QString *families() const { return standardFamilies; }

private:
    QValueList<int>     m_fontSizes;
    QString *standardFamilies;

};

};

#endif



