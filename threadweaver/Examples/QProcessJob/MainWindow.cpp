/* -*- C++ -*-

This file implements the mainwindows for the QProcessJob example.

$ Author: Mirko Boehm $
$ Copyright: (C) 2006, 2007 Mirko Boehm $
$ Contact: mirko@kde.org
http://www.kde.org
http://www.hackerbuero.org $

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
#include "MainWindow.h"
#include "QProcessJob.h"

#include <ThreadWeaver.h>

using namespace ThreadWeaver;

MainWindow::MainWindow ( QWidget * parent )
    : QMainWindow ( parent )
{
    ui.setupUi ( this );
    connect ( Weaver::instance(),  SIGNAL ( jobDone ( ThreadWeaver::Job* ) ),
              SLOT ( update ( ThreadWeaver::Job* ) ) );
}

void MainWindow::on_pushButtonStart_clicked ()
{
    ui.pushButtonStart->setEnabled( false );
    Weaver::instance()->enqueue ( new QProcessJob ( this ) );
}

void MainWindow::on_pushButtonQuit_clicked ()
{
    ui.pushButtonStart->setEnabled( false );
    ui.pushButtonQuit->setEnabled( false );
    Weaver::instance()->finish();
    QApplication::instance()->quit();
}

void MainWindow::update( ThreadWeaver::Job *j )
{
    qDebug ( "MainWindow::update: job finished." );
    QProcessJob* job = dynamic_cast<QProcessJob*> ( j );

    if ( job )
    {
        ui.labelTime->setText ( job->result() );
    } else {
        qDebug ( "I do not know anything about this kind of jobs." );
    }
    ui.pushButtonStart->setEnabled( true );
    ui.pushButtonQuit->setEnabled( true );
}

#include "MainWindow.moc"
