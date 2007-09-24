/*
 *  Copyright (C) 2003 David Faure   <faure@kde.org>
 *
    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#include <kcmdlineargs.h>
#include <kapplication.h>
#include <kdebug.h>
#include <QtGui/QWidget>
#include "kwordwrap.h"

int main(int argc, char *argv[])
{
    KCmdLineArgs::init( argc, argv, "test", 0, ki18n("Test"), "1.0", ki18n("test app"));
    KApplication app;

    QFont font( "helvetica", 12 ); // let's hope we all have the same...
    QFontMetrics fm( font );
    QRect r( 0, 0, 100, -1 );
    QString str = "test wadabada [/foo/bar/waba]";
    KWordWrap* ww = KWordWrap::formatText( fm, r, 0, str );
    kDebug() << str << " => " << ww->truncatedString();
    delete ww;

    str = "</p></p></p></p>";
    for ( ; r.width() > 0 ; r.setWidth( r.width()-10 ) )
    {
        ww = KWordWrap::formatText( fm, r, 0, str );
        kDebug() << str << " => " << ww->truncatedString();
        delete ww;
    }
}
