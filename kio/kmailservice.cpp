/* This file is part of the KDE libraries
   Copyright (C) 2000 Simon Hausmann <hausmann@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <kapplication.h>
#include <klocale.h>
#include <kcmdlineargs.h>

static const KCmdLineOptions options[] =
{
    { "+url", 0, 0 },
    { 0, 0, 0 }
};

int main( int argc, char **argv )
{
    KLocale::setMainCatalogue("kdelibs");
    KCmdLineArgs::init( argc, argv, "kmailservice", I18N_NOOP("mail service"), "unknown" );
    KCmdLineArgs::addCmdLineOptions( options );

    KApplication a;

    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

    if ( args->count() != 1 )
        return 1;

    a.invokeMailer(KURL(args->arg(0)));

    return 0;
}
