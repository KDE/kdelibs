// -*- C++ -*-

//
//  KIconLoader demo
//
//  Copyright (C) 1997 Christoph Neerfeld
//  email:  Christoph.Neerfeld@mail.bonn.netsurf.de
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//

#include <stdlib.h>

#include <qapplication.h>
#include <qpushbutton.h>

#include <kapp.h>
#include <drag.h>

#include <kiconloader.h>
#include "kiconloadertestmain.h"
#include "kconfig.h"

int main( int argc, char **argv )
{
debug ( "[kiconloader demo] started-------------------------" );

  KApplication a( argc, argv, "kiconloaderdemo" );

  QString temp;
  KConfig *config = a.getConfig();
  config->setGroup("KDE Setup");
  if( !config->hasKey("IconPath") )
     {
       temp = (QString) getenv("KDEDIR");
       temp += "/lib/pics:";
       temp += (QString) getenv("KDEDIR");
       temp += "/lib/pics/toolbar";
       config->writeEntry("IconPath", temp);
     }
  MyMain demo;
  a.setMainWidget( (QWidget *) &demo );
  a.setRootDropZone( new KDNDDropZone( (QWidget *) &demo, DndNotDnd ) );
  demo.show();
  return a.exec();
}

#include "kiconloadertestmain.moc"

