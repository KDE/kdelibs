/*
 *  Copyright (C) 2002 David Faure   <faure@kde.org>
 *  Copyright (C) 2003 Waldo Bastian <bastian@kde.org>
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
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#include "kruntest.h"

#include <kapplication.h>
#include <kdebug.h>
#include <kservice.h>
#include <kde_file.h>
#include <kcmdlineargs.h>
#include <QtGui/QPushButton>
#include <QtGui/QLayout>

#include <stdlib.h>
#include <unistd.h>

const int MAXKRUNS = 100;

testKRun * myArray[MAXKRUNS];

void testKRun::foundMimeType( const QString& _type )
{
  kDebug() << "testKRun::foundMimeType " << _type;
  kDebug() << "testKRun::foundMimeType URL=" << url().url();
  setFinished( true );
  timer().setSingleShot( true );
  timer().start( 0 );
  return;
}

Receiver::Receiver()
{
    QVBoxLayout *lay = new QVBoxLayout(this);
    QPushButton * h = new QPushButton( "Press here to terminate", this );
    lay->addWidget( h );
    start = new QPushButton( "Launch KRuns", this );
    lay->addWidget( start );
    stop = new QPushButton( "Stop those KRuns", this );
    stop->setEnabled(false);
    lay->addWidget( stop );

    QPushButton* launchOne = new QPushButton( "Launch one http KRun", this );
    lay->addWidget(launchOne);

    connect(h, SIGNAL(clicked()), qApp, SLOT(quit()));
    connect(start, SIGNAL(clicked()), this, SLOT(slotStart()));
    connect(stop, SIGNAL(clicked()), this, SLOT(slotStop()));
    connect(launchOne, SIGNAL(clicked()), this, SLOT(slotLaunchOne()));

    adjustSize();
    show();
}

void Receiver::slotStop()
{
  for (int i = 0 ; i < MAXKRUNS ; i++ )
  {
    kDebug() << " deleting krun " << i;
    delete myArray[i];
  }
  start->setEnabled(true);
  stop->setEnabled(false);
}

void Receiver::slotStart()
{
  for (int i = 0 ; i < MAXKRUNS ; i++ )
  {
    kDebug() << "creating testKRun " << i;
    myArray[i] = new testKRun( KUrl("file:/tmp"), window(), 0,
                               true /*isLocalFile*/, false /* showProgressInfo */ );
    myArray[i]->setAutoDelete(false);
  }
  start->setEnabled(false);
  stop->setEnabled(true);
}

void Receiver::slotLaunchOne()
{
    new testKRun(KUrl("http://www.kde.org"), window());
}

int main(int argc, char **argv)
{
    KCmdLineArgs::init(argc,argv, "kruntest", 0, ki18n("kruntest"), 0);
    KApplication app;

    Receiver receiver;
    return app.exec();
}

#include "kruntest.moc"
