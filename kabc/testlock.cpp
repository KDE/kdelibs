#include "testlock.h"

#include <kaboutdata.h>
#include <kapplication.h>
#include <kdebug.h>
#include <klocale.h>
#include <kcmdlineargs.h>
/*
    This file is part of libkabc.

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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <kmessagebox.h>
#include <kdialog.h>

#include <qwidget.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>

#include <iostream>

using namespace KABC;

LockWidget::LockWidget( const QString &identifier )
{
  mLock = new Lock( identifier );

  QVBoxLayout *topLayout = new QVBoxLayout( this );
  topLayout->setMargin( KDialog::marginHint() );
  topLayout->setSpacing( KDialog::spacingHint() );

  QHBoxLayout *identifierLayout = new QHBoxLayout( topLayout );
  
  QLabel *resourceLabel = new QLabel( "Identifier:", this );
  identifierLayout->addWidget( resourceLabel );
  
  QLabel *resourceIdentifier = new QLabel( identifier, this );
  identifierLayout->addWidget( resourceIdentifier );

  mStatus = new QLabel( "Status: Unlocked", this );
  topLayout->addWidget( mStatus );
    
  QPushButton *button = new QPushButton( "Lock", this );
  topLayout->addWidget( button );
  connect( button, SIGNAL( clicked() ), SLOT( lock() ) );

  button = new QPushButton( "Unlock", this );
  topLayout->addWidget( button );
  connect( button, SIGNAL( clicked() ), SLOT( unlock() ) );

  button = new QPushButton( "Quit", this );
  topLayout->addWidget( button );
  connect( button, SIGNAL( clicked() ), SLOT( close() ) );
}

LockWidget::~LockWidget()
{
  delete mLock;
}

void LockWidget::lock()
{
  if ( !mLock->lock() ) {
    KMessageBox::sorry( this, mLock->error() );
  } else {
    mStatus->setText( "Status: Locked" );
  }
}

void LockWidget::unlock()
{
  if ( !mLock->unlock() ) {
    KMessageBox::sorry( this, mLock->error() );
  } else {
    mStatus->setText( "Status: Unlocked" );
  }
}


static const KCmdLineOptions options[] =
{
  {"+identifier","Identifier of resource to be locked, e.g. filename", 0 },
  {0,0,0}
};

int main(int argc,char **argv)
{
  KAboutData aboutData("testlock",I18N_NOOP("Test libkabc Lock"),"0.1");
  KCmdLineArgs::init(argc,argv,&aboutData);
  KCmdLineArgs::addCmdLineOptions( options );

  KApplication app;

  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
  if ( args->count() != 1 ) {
    cerr << "Usage: testlock <identifier>" << endl;
    return 1;
  }

  QString identifier = args->arg( 0 );

  LockWidget mainWidget( identifier );

  kapp->setMainWidget( &mainWidget );
  mainWidget.show();

  return app.exec();  
}

#include "testlock.moc"
