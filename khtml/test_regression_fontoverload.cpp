/**
 * This file is part of the KDE project
 *
 * Copyright (C) 2003 Stephan Kulow (coolo@kde.org)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 */

#define QT_NO_XFTFREETYPE 1
#include <private/qfontengine_p.h>
#include <qfontdatabase.h>
#include <qfont.h>
#include "khtml_settings.h"
#include <qwidget.h>

QFontEngine *
QFontDatabase::findFont( QFont::Script script, const QFontPrivate *fp,
			 const QFontDef &request, int force_encoding_id )
{
    QFontEngine *fe = new QFontEngineBox( request.pixelSize );
    QFontCache::Key key( request, script, fp->screen );
    QFontCache::instance->insertEngine( key, fe );
    return fe;
}

bool QFontDatabase::isBitmapScalable( const QString &family,
				      const QString &style) const
{
    return true;
}

bool  QFontDatabase::isSmoothlyScalable( const QString &family,
                                         const QString &style) const
{
    return true;
}

const QString &KHTMLSettings::availableFamilies()
{
    if ( !avFamilies ) {
        avFamilies = new QString;
        *avFamilies = ",Adobe Courier,Adobe Helvetica,Adobe New Century Schoolbook,Adobe Times,Adobe Utopia,Century Schoolbook L,Charter,Clean,Console,Courier,Courier 10 Pitch,Cursor,DEC Terminal,Dingbats,ETL Fixed,Fixed,Goha Tibeb Zemen,Gothic,Helvetica,Luxi Mono,Luxi Sans,Luxi Serif,Mincho,New Century Schoolbook,Newspaper,Nil,Proof,Schumacher Clean,Song Ti,Sony Fixed,Standard Symbols L,Symbol,Terminal,Times,Utopia,";
    }

  return *avFamilies;
}

int QPaintDevice::x11AppDpiY( int )
{
    return 100;
}

int QPaintDevice::x11AppDpiX( int )
{
    return 100;
}
