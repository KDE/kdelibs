/*
    This file is part of KDE.

    Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>

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

#include "exampleprefs_base.h"

#include <kaboutdata.h>
#include <kapplication.h>
#include <kdebug.h>
#include <klocale.h>
#include <kcmdlineargs.h>
#include <kglobal.h>
#include <kconfig.h>
#include <kstandarddirs.h>

int main( int argc, char **argv )
{
  KAboutData aboutData( "example", I18N_NOOP("cfgc example"), "0.1" );
  aboutData.addAuthor( "Cornelius Schumacher", 0, "schumacher@kde.org" );

  KCmdLineArgs::init( argc, argv, &aboutData );

  KApplication app;

  ExamplePrefsBase *prefs = new ExamplePrefsBase("Trans1", "Folder2");
  
  prefs->readConfig();

  prefs->setAnotherOption(17);

  kWarning() << "Another Option  = " << prefs->anotherOption() << endl;
  kWarning() << "Another Option2 = " << prefs->anotherOption2() << endl;
  kWarning() << "MyPaths         = " << prefs->myPaths() << endl;
  kWarning() << "MyPaths2        = " << prefs->myPaths2() << endl;
}
