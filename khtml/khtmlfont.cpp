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
#include <string.h>
#include <qstring.h>
#include <kapp.h>
#include <klocale.h>
#include "kglobal.h"

HTMLFontManager* pFontManager = 0;

HTMLFont::HTMLFont( const char *_family, int _size, const int fontSizes[7], 
                    int _weight, bool _italic, const char *_charset)
    : font( _family, fontSizes[ _size ], _weight, _italic )
{
    textCol = Qt::black;
    fsize = _size;
    if (_charset) setCharset(_charset);
    else setCharset(KGlobal::locale()->charset());
    pointsize = fontSizes[ _size ];
	
}

HTMLFontManager::HTMLFontManager()
{
    list.setAutoDelete( TRUE );
}

const HTMLFont *HTMLFontManager::getFont( const HTMLFont &f )
{
    HTMLFont *cf;

    for ( cf = list.first(); cf; cf = list.next() )
    {
	if ( *cf == f )
	    return cf;
    }

    cf = new HTMLFont( f );

    list.append( cf );

    return cf;
}

