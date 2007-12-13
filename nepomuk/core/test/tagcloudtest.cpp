/* This file is part of the Nepomuk-KDE libraries
    Copyright (c) 2007 Sebastian Trueg <trueg@kde.org>

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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <resourcemanager.h>
#include "../ui/kmetadatatagcloud.h"

#include <krandom.h>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>

int main( int argc, char** argv )
{
    KAboutData about("tagcloudtest", 0, ki18n("tagcloudtest"), "1.0");
    KCmdLineArgs::init( argc, argv, &about );
    KApplication app;
    Nepomuk::ResourceManager::instance()->init();
    Nepomuk::TagCloud tcw;
    //  KTagCloudWidget tcw;
//   for( int i = 1; i <= 12; ++i )
//     tcw.addTag( QLocale::system().monthName(i), (int)(20.0*(double)KRandom::random()/(double)RAND_MAX) + 1 );
//   for( int i = 1; i <= 7; ++i )
//     tcw.addTag( QLocale::system().dayName(i), (int)(20.0*(double)KRandom::random()/(double)RAND_MAX) + 1 );

    tcw.show();

    return app.exec();
}
