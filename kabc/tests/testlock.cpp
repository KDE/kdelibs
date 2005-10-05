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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "testlock.h"

#include "stdaddressbook.h"

#include <kaboutdata.h>
#include <kapplication.h>
#include <kdebug.h>
#include <klocale.h>
#include <kcmdlineargs.h>
#include <kdirwatch.h>

#include <kmessagebox.h>
#include <kdialog.h>

#include <qwidget.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <q3listview.h>
#include <qdir.h>
//Added by qt3to4:
#include <QVBoxLayout>
#include <QHBoxLayout>

#include <iostream>

#include <sys/types.h>
#include <unistd.h>

using namespace KABC;

LockWidget::LockWidget( const QString &identifier )
{
  QVBoxLayout *topLayout = new QVBoxLayout( this );
  topLayout->setMargin( KDialog::marginHint() );
  topLayout->setSpacing( KDialog::spacingHint() );

  if ( identifier.isEmpty() ) {
    mLock = 0;
  } else {
    mLock = new Lock( identifier );

    int pid = getpid();

    QLabel *pidLabel = new QLabel( "Process ID: " + QString::number( pid ),
                                   this );
    topLayout->addWidget( pidLabel );

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
  }

  mLockView = new Q3ListView( this );
  topLayout->addWidget( mLockView );
  mLockView->addColumn( "Lock File" );
  mLockView->addColumn( "PID" );
  mLockView->addColumn( "Locking App" );

  updateLockView();

  QPushButton *quitButton = new QPushButton( "Quit", this );
  topLayout->addWidget( quitButton );
  connect( quitButton, SIGNAL( clicked() ), SLOT( close() ) );
  
  KDirWatch *watch = KDirWatch::self();
  connect( watch, SIGNAL( dirty( const QString & ) ),
           SLOT( updateLockView() ) );
  connect( watch, SIGNAL( created( const QString & ) ),
           SLOT( updateLockView() ) );
  connect( watch, SIGNAL( deleted( const QString & ) ),
           SLOT( updateLockView() ) );
  watch->addDir( Lock::locksDir() );
  watch->startScan();
}

LockWidget::~LockWidget()
{
  delete mLock;
}

void LockWidget::updateLockView()
{
  mLockView->clear();
  
  QDir dir( Lock::locksDir() );
  
  QStringList files = dir.entryList( "*.lock" );
  
  QStringList::ConstIterator it;
  for( it = files.begin(); it != files.end(); ++it ) {
    if ( *it == "." || *it == ".." ) continue;
    
    QString app;
    int pid;
    if ( !Lock::readLockFile( dir.filePath( *it ), pid, app ) ) {
      kdWarning() << "Unable to open lock file '" << *it << "'" << endl; 
    } else {
      new Q3ListViewItem( mLockView, *it, QString::number( pid ), app );
    }
  }
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
  { "a", 0, 0 },
  { "addressbook", "Standard address book", 0 },
  { "d", 0, 0 },
  { "diraddressbook", "Standard address book directory resource", 0 },
  { "+identifier", "Identifier of resource to be locked, e.g. filename", 0 },
  KCmdLineLastOption
};

int main(int argc,char **argv)
{
  KAboutData aboutData("testlock",I18N_NOOP("Test libkabc Lock"),"0.1");
  KCmdLineArgs::init(argc,argv,&aboutData);
  KCmdLineArgs::addCmdLineOptions( options );

  KApplication app;

  QString identifier;

  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
  if ( args->count() == 1 ) {
    identifier = args->arg( 0 );
  } else if ( args->count() != 0 ) {
    std::cerr << "Usage: testlock <identifier>" << std::endl;
    return 1;
  }

  if ( args->isSet( "addressbook" ) ) {
    if ( args->count() == 1 ) {
      std::cerr << "Ignoring resource identifier" << std::endl;
    }
    identifier = StdAddressBook::fileName(); 
  }

  if ( args->isSet( "diraddressbook" ) ) {
    if ( args->count() == 1 ) {
      std::cerr << "Ignoring resource identifier" << std::endl;
    }
    identifier = StdAddressBook::directoryName(); 
  }

  LockWidget mainWidget( identifier );

  kapp->setMainWidget( &mainWidget );
  mainWidget.show();

  return app.exec();  
}

#include "testlock.moc"
