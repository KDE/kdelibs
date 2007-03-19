/*
 *  Copyright (C) 2002 David Faure   <faure@kde.org>
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
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#include <kmimetype.h>
#include <kcmdlineargs.h>
//#include <kapplication.h>
#include <kdeversion.h>
#include <kcomponentdata.h>

#include <stdio.h>

static KCmdLineOptions options[] =
{
  { "+filename", "the filename to test", 0 },
  KCmdLineLastOption
};

int main(int argc, char *argv[])
{
    KCmdLineArgs::init( argc, argv, "kmimetypefinder", "MimeType Finder", "Gives the mimetype for a given file", KDE_VERSION_STRING );

    KCmdLineArgs::addCmdLineOptions( options );

    // KApplication app;
    KComponentData instance("kmimetypefinder");

    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    if( args->count() < 1 ) {
        printf( "No filename specified\n" );
        return 1;
    }
    const QString fileName = args->arg( 0 );
    int accuracy;
    KMimeType::Ptr mime = KMimeType::findByPath( fileName, 0, false, &accuracy );
    if ( mime && mime->name() != KMimeType::defaultMimeType() ) {
        printf("%s\n", mime->name().toLatin1().constData());
        printf("(accuracy %d)\n", accuracy);
    } else {
        return 1; // error
    }

    return 0;
}
