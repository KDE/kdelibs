/* This file is part of the KDE desktop environment

   Copyright (C) 2001, 2002 Michael Brade <brade@kde.org>

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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <qlayout.h>
#include <qpushbutton.h>

#include <kapplication.h>
#include <kdirlister.h>
#include <kdebug.h>

#include "kdirlistertest.h"

#include <cstdlib>


KDirListerTest::KDirListerTest( QWidget *parent, const char *name )
  : QWidget( parent, name )
{
  lister = new KDirLister( false /* true */ );
  debug = new PrintSignals;

  QVBoxLayout* layout = new QVBoxLayout( this );

  QPushButton* start = new QPushButton( "Start listing", this );
  QPushButton* test = new QPushButton( "Many", this );

  layout->addWidget( start );
  layout->addWidget( test );
  resize( layout->sizeHint() );

  connect( start, SIGNAL( clicked() ), SLOT( start() ) );
  connect( test, SIGNAL( clicked() ), SLOT( test() ) );

  connect( lister, SIGNAL( started( const KURL & ) ),
           debug,  SLOT( started( const KURL & ) ) );
  connect( lister, SIGNAL( completed() ),
           debug,  SLOT( completed() ) );
  connect( lister, SIGNAL( completed( const KURL & ) ),
           debug,  SLOT( completed( const KURL & ) ) );
  connect( lister, SIGNAL( canceled() ),
           debug,  SLOT( canceled() ) );
  connect( lister, SIGNAL( canceled( const KURL & ) ),
           debug,  SLOT( canceled( const KURL & ) ) );
  connect( lister, SIGNAL( redirection( const KURL & ) ),
           debug,  SLOT( redirection( const KURL & ) ) );
  connect( lister, SIGNAL( redirection( const KURL &, const KURL & ) ),
           debug,  SLOT( redirection( const KURL &, const KURL & ) ) );
  connect( lister, SIGNAL( clear() ),
           debug,  SLOT( clear() ) );
  connect( lister, SIGNAL( newItems( const KFileItemList & ) ),
           debug,  SLOT( newItems( const KFileItemList & ) ) );
  connect( lister, SIGNAL( itemsFilteredByMime( const KFileItemList & ) ),
           debug,  SLOT( itemsFilteredByMime( const KFileItemList & ) ) );
  connect( lister, SIGNAL( deleteItem( KFileItem * ) ),
           debug,  SLOT( deleteItem( KFileItem * ) ) );
  connect( lister, SIGNAL( refreshItems( const KFileItemList & ) ),
           debug,  SLOT( refreshItems( const KFileItemList & ) ) );
  connect( lister, SIGNAL( closeView() ),
           debug,  SLOT( closeView() ) );
  connect( lister, SIGNAL( infoMessage( const QString& ) ),
           debug,  SLOT( infoMessage( const QString& ) ) );
  connect( lister, SIGNAL( percent( int ) ),
           debug,  SLOT( percent( int ) ) );
  connect( lister, SIGNAL( totalSize( KIO::filesize_t ) ),
           debug,  SLOT( totalSize( KIO::filesize_t ) ) );
  connect( lister, SIGNAL( processedSize( KIO::filesize_t ) ),
           debug,  SLOT( processedSize( KIO::filesize_t ) ) );
  connect( lister, SIGNAL( speed( int ) ),
           debug,  SLOT( speed( int ) ) );
}

KDirListerTest::~KDirListerTest()
{
  delete lister;
}

void KDirListerTest::start()
{
  KURL home( getenv( "HOME" ) );
  KURL root( "file:/" );

  lister->openURL( home, false, false );
//  lister->openURL( root, true, true );
//  lister->stop();
//  lister->stop( root );
}

void KDirListerTest::test()
{
  KURL home( getenv( "HOME" ) );
  KURL root( "file:/" );
/*  lister->openURL( home, true, false );
  lister->openURL( root, true, true );
  lister->openURL( KURL("file:/etc"), true, true );
  lister->openURL( root, true, true );
  lister->openURL( KURL("file:/dev"), true, true );
  lister->openURL( KURL("file:/tmp"), true, true );
  lister->openURL( KURL("file:/usr/include"), true, true );
  lister->updateDirectory( KURL("file:/usr/include") );
  lister->updateDirectory( KURL("file:/usr/include") );
  lister->openURL( KURL("file:/usr/"), true, true );
*/
  lister->openURL( KURL("file:/dev"), true, true );
}

int main ( int argc, char *argv[] )
{
  KApplication app( argc, argv, "kdirlistertest" );

  KDirListerTest *test = new KDirListerTest( 0 );
  test->show();
  app.setMainWidget( test );
  return app.exec();
}

#include "kdirlistertest.moc"
