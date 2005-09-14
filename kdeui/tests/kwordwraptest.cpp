/*
 *  Copyright (C) 2003 David Faure   <faure@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation;
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 */

#include <kcmdlineargs.h>
#include <kapplication.h>
#include <kdebug.h>
#include <qwidget.h>
#include "kwordwrap.h"

int main(int argc, char *argv[])
{
    KCmdLineArgs::init( argc, argv, "test", "Test" ,"test app" ,"1.0" );
    KApplication app;

    QFont font( "helvetica", 12 ); // let's hope we all have the same...
    QFontMetrics fm( font );
    QRect r( 0, 0, 100, -1 );
    QString str = "test wadabada [/foo/bar/waba]";
    KWordWrap* ww = KWordWrap::formatText( fm, r, 0, str );
    kdDebug() << str << " => " << ww->truncatedString() << endl;
    delete ww;

    str = "</p></p></p></p>";
    for ( ; r.width() > 0 ; r.setWidth( r.width()-10 ) )
    {
        ww = KWordWrap::formatText( fm, r, 0, str );
        kdDebug() << str << " => " << ww->truncatedString() << endl;
        delete ww;
    }
}
