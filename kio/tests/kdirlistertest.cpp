/* This file is part of KDE desktop environment

   Copyright 2001 Michael Brade <brade@informatik.uni-muenchen.de>

   This file is licensed under LGPL version 2.
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
  QPushButton* setURL = new QPushButton( "Set URL", this );
  QPushButton* test = new QPushButton( "Many", this );

  layout->addWidget( start );
  layout->addWidget( setURL );
  layout->addWidget( test );
  resize( layout->sizeHint() );

  connect( start, SIGNAL( clicked() ), SLOT( start() ) );
  connect( setURL, SIGNAL( clicked() ), SLOT( setURL() ) );
  connect( test, SIGNAL( clicked() ), SLOT( test() ) );

  debug->connect( lister, SIGNAL( started( const QString & ) ),
                           SLOT( started( const QString & ) ) );
  debug->connect( lister, SIGNAL( completed() ),
                           SLOT( completed() ) );
  debug->connect( lister, SIGNAL( completed( const KURL & ) ),
                           SLOT( completed( const KURL & ) ) );
  debug->connect( lister, SIGNAL( canceled() ),
                           SLOT( canceled() ) );
  debug->connect( lister, SIGNAL( canceled( const KURL & ) ),
                           SLOT( canceled( const KURL & ) ) );
  debug->connect( lister, SIGNAL( redirection( const KURL & ) ),
                           SLOT( redirection( const KURL & ) ) );
  debug->connect( lister, SIGNAL( redirection( const KURL &, const KURL & ) ),
                           SLOT( redirection( const KURL &, const KURL & ) ) );
  debug->connect( lister, SIGNAL( clear() ),
                           SLOT( clear() ) );
  debug->connect( lister, SIGNAL( newItems( const KFileItemList & ) ),
                           SLOT( newItems( const KFileItemList & ) ) );
  debug->connect( lister, SIGNAL( itemsFilteredByMime( const KFileItemList & ) ),
                           SLOT( itemsFilteredByMime( const KFileItemList & ) ) );
  debug->connect( lister, SIGNAL( deleteItem( KFileItem * ) ),
                           SLOT( deleteItem( KFileItem * ) ) );
  debug->connect( lister, SIGNAL( refreshItems( const KFileItemList & ) ),
                           SLOT( refreshItems( const KFileItemList & ) ) );
  debug->connect( lister, SIGNAL( closeView() ),
                           SLOT( closeView() ) );
}

KDirListerTest::~KDirListerTest()
{
  delete lister;
}

void KDirListerTest::start()
{
  KURL home( getenv( "HOME" ) );
  KURL root( "file:/" );

  lister->openURL( home, true, false );
  lister->openURL( root, true, true );
  lister->stop();
//  lister->stop( root );
}

void KDirListerTest::setURL()
{
  lister->setURL( KURL( "file:/" ) );
  lister->setURLDirty( true );
  lister->listDirectory();
}

void KDirListerTest::test()
{
  KURL home( getenv( "HOME" ) );
  KURL root( "file:/" );
  lister->openURL( home, true, false );
  lister->openURL( root, true, true );
  lister->openURL( KURL("file:/etc"), true, true );
//  lister->openURL( root, true, true );
  lister->openURL( KURL("file:/dev"), true, true );
  lister->openURL( KURL("file:/tmp"), true, true );
  lister->openURL( KURL("file:/usr/include"), true, true );
  lister->openURL( KURL("file:/usr/"), true, true );
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
