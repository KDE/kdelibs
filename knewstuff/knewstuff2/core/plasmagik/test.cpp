/***************************************************************************
*   Copyright (C) 2006 by Riccardo Iaconelli  <ruphy@fsfe.org>            *
*                                                                         *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU General Public License     *
*   along with this program; if not, write to the                         *
*   Free Software Foundation, Inc.,                                       *
*   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
***************************************************************************/
#include <kapplication.h>
#include <kaboutdata.h>
#include <klocale.h>
#include <kcmdlineargs.h>
#include <kguiitem.h>

#include "plasmagik.h"

static KCmdLineOptions options[] =
    {
        { "s", 0, 0 },
        { "set-arbitrary-names", I18N_NOOP( "Fill in the fildes to speed up testing. Just for development purpose." ), 0 },
        KCmdLineLastOption
    };

// class test : public Plasmagik
// {
//
// };


int main ( int argc, char *argv[] )
{
    KAboutData aboutData ( "plasmagik", I18N_NOOP( "Plasmagik" ),
                           "3", I18N_NOOP( "The official \"magic\" packager for plasmoids" ), KAboutData::License_GPL,
                           "(c) 2006, Riccardo Iaconelli", 0, "http://plasma.kde.org/", "ruphy@fsfe.org" );
    aboutData.addAuthor( "Riccardo Iaconelli", 0, "ruphy@fsfe.org" );

    KCmdLineArgs::init( argc, argv, &aboutData );
    KCmdLineArgs::addCmdLineOptions( options );
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    KApplication app;

//     Plasmagik *o = new Plasmagik("test");
    Plasmagik o("test");
    //     QStringList lang;
    //     lang << "C++" << "Java";
    //  QString languages = "|C++|Python|Java|Javascript";
    //  o.setProgrammingLanguages(lang);
//     o.setProgrammingLanguages( "|C++|Python|Java|Javascript" );
//     o.setPackageType( Plasmagik::KrossPackage, false );
//     o.setPackageType( Plasmagik::Artwork, true );
    //     o.setPackageType(Plasmagik::KrossPackage);
    //     o.setPackageType(Plasmagik::QtScriptPackage);
    //     o.setPackageType(Plasmagik::Artwork);
    //     o.setPackageType(Plasmagik::Custom);
    //     o.setPackageType(Plasmagik::Artwork);
    //     o.setPackageType(Plasmagik::KrossPackage);
    //     kDebug() << Plasmagik::KrossPackage << endl;
    //     o.setPackageType(Plasmagik::Artwork);
//     if ( args->isSet( "s" ) ) o.fillFields();
//     return o.exec();
    o.exec();
    return 0;
}

