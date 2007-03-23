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
#include <qpushbutton.h>
#include <QtGui/QLayout>

#include <stdlib.h>
#include <unistd.h>

const int MAXKRUNS = 100;

testKRun * myArray[MAXKRUNS];

void testKRun::foundMimeType( const QString& _type )
{
  kDebug() << "testKRun::foundMimeType " << _type << endl;
  kDebug() << "testKRun::foundMimeType URL=" << m_strURL.url() << endl;
  m_bFinished = true;
  m_timer.setSingleShot( true );
  m_timer.start( 0 );
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
    QObject::connect( h, SIGNAL(clicked()), kapp, SLOT(quit()) );
    QObject::connect( start, SIGNAL(clicked()), this, SLOT(slotStart()) );
    QObject::connect( stop, SIGNAL(clicked()), this, SLOT(slotStop()) );

    adjustSize();
    show();
}

void Receiver::slotStop()
{
  for (int i = 0 ; i < MAXKRUNS ; i++ )
  {
    kDebug() << " deleting krun " << i << endl;
    delete myArray[i];
  }
  start->setEnabled(true);
  stop->setEnabled(false);
}

void Receiver::slotStart()
{
  for (int i = 0 ; i < MAXKRUNS ; i++ )
  {
    kDebug() << "creating testKRun " << i << endl;
    myArray[i] = new testKRun( KUrl("file:/tmp"), window(),0, true, false /* no autodelete */ );
  }
  start->setEnabled(false);
  stop->setEnabled(true);
}

int main(int argc, char **argv)
{
    KCmdLineArgs::init(argc,argv,"kruntest", 0, 0, 0, 0);
    KApplication app;

    Receiver receiver;
    return app.exec();
}

#include "kruntest.moc"
