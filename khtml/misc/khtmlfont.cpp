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
// KDE HTML Widget -- Fonts
// $Id$

#include "khtmlfont.h"
using namespace khtml;

#include <string.h>
#include <qstring.h>
#include <kapp.h>
#include <klocale.h>
#include "kglobal.h"

template class QList<khtml::Font>;

FontManager* khtml::pFontManager = 0;

Font::Font( QString _family, int _size, const int fontSizes[7],
	    int _weight, bool _italic, QFont::CharSet _charset)
    : font( _family, fontSizes[ _size ], _weight, _italic )
{
    textCol = Qt::black;
    fsize = _size;
    // ### FIXME: We have to get unicode support back.
    // This doesn't work, because it triggers a XListFonts every time hasUnicode() is called.
    // Either fix kcharsets, or... ???
    //if(KGlobal::charsets()->hasUnicode(_family))
    //_charset = QFont::Unicode;
    //    else 
    if (_charset) setCharset(_charset);
    else setCharset(KGlobal::charsets()->charsetForLocale());
    pointsize = fontSizes[ _size ];
    dirty = true;
    VOffset = 0;
}

FontManager::FontManager()
{
    list.setAutoDelete( TRUE );
}

const Font *FontManager::getFont( const Font &f )
{
    Font *cf;

    for ( cf = list.first(); cf; cf = list.next() )
    {
	if ( *cf == f )
	    return cf;
    }

    cf = new Font( f );

    list.append( cf );

    return cf;
}

