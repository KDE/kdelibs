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
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 */

#include <ktrader.h>
#include <kmimetype.h>
#include <kcmdlineargs.h>
#include <kapplication.h>

#include <stdio.h>

static KCmdLineOptions options[] =
{
  { "+filename", "the filename to test", 0 },
  { 0, 0, 0 }
};

int main(int argc, char *argv[])
{
  KCmdLineArgs::init( argc, argv, "kmimefromext", "A mimetype testing tool, gives the mimetype for a given filename", "0.0" );

  KCmdLineArgs::addCmdLineOptions( options );

  KApplication app;

  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
  QString fileName = args->arg( 0 );

  // The "true" here means only the filename will be looked at.
  // "Mime-magic" will not interfer. The file doesn't exist.
  // TODO: a cmd line parameter for controlling this bool ;)
  KMimeType::Ptr mime = KMimeType::findByPath( fileName, 0, true );
  if ( mime && mime->name() != KMimeType::defaultMimeType() )
    printf( "%s\n", mime->name().latin1());
  else
    return 1; // error
     
  return 0;
}
