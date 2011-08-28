/* -*- C++ -*-

   This file implements the SMIV class.

   $ Author: Mirko Boehm $
   $ Copyright: (C) 2005, Mirko Boehm $
   $ Contact: mirko@kde.org
         http://www.kde.org
         http://www.hackerbuero.org $
   $ License: LGPL with the following explicit clarification:
         This code may be linked against any version of the Qt toolkit
         from Trolltech, Norway. $

   $Id: SMIV.cpp 30 2005-08-16 16:16:04Z mirko $
*/

#include "SMIV.h"
#include "SMIVItem.h"

#include <QtCore/QDebug>
#include <QtGui/QFileDialog>
#include <QtGui/QApplication>

#include <State.h>
#include <ThreadWeaver.h>
#include <DebuggingAids.h>

using namespace ThreadWeaver;

SMIV::SMIV ( Weaver* w )
    : QWidget()
    , m_weaver ( w )
    , m_noOfJobs ( 0 )
    , m_quit ( false )
{
    ui.setupUi ( this );
    connect ( m_weaver,  SIGNAL ( finished() ),  SLOT ( slotJobsDone() ) );
    connect ( m_weaver,  SIGNAL ( jobDone ( ThreadWeaver::Job* ) ),
              SLOT( slotJobDone( ThreadWeaver::Job* ) ) );
    connect ( m_weaver,  SIGNAL ( suspended () ),  SLOT ( weaverSuspended() ) );
    ui.listView->setModel ( &model );
    ui.listView->setItemDelegate( &del );
}

SMIV::~SMIV ()
{
}

void SMIV::on_pbSelectFiles_clicked()
{
    ui.pbSelectFiles->setEnabled(false);
    // listWidget->clear();

    QStringList files = QFileDialog::getOpenFileNames
                        ( this, "Select Images to display",
                          QDir::homePath(),
                          "Images (*.png *.xpm *.jpg)");

    if ( ! files.isEmpty() )
    {
        m_noOfJobs = 3 * files.size(); // for progress display
        ui.progressBar->setEnabled (true);
        ui.progressBar->setRange (1, m_noOfJobs);
        ui.progressBar->reset();

        m_weaver->suspend();
        for (int index = 0; index < files.size(); ++index)
        {
            SMIVItem *item = new SMIVItem ( m_weaver, files.at(index ), this );
            connect ( item,  SIGNAL( thumbReady(SMIVItem* ) ),
                      SLOT ( slotThumbReady( SMIVItem* ) ) );
        }
        m_startTime.start();
        m_weaver->resume();

        ui.pbSelectFiles->setEnabled(false);
        ui.pbCancel->setEnabled(true);
        ui.pbSuspend->setEnabled(true);
        ui.pbSuspend->setText ( "Suspend" );
    } else {
        QApplication::beep();
        on_pbCancel_clicked();
    }
}

void SMIV::on_pbCancel_clicked()
{
    m_weaver->dequeue();
    m_weaver->requestAbort();
    ui.pbSelectFiles->setEnabled(true);
    ui.pbCancel->setEnabled(false);
    ui.pbSuspend->setEnabled(false);

    ui.progressBar->reset();
    ui.progressBar->setEnabled (false);
}

void SMIV::on_pbSuspend_clicked()
{
    if ( m_weaver->state().stateId() == Suspended )
    {
        ui.pbSuspend->setText ( "Suspend" );
        m_weaver->resume();
    } else {
        m_weaver->suspend();
        ui.pbSuspend->setEnabled ( false );
    }
}

void SMIV::on_pbQuit_clicked()
{
    // @TODO: suspend weaver and remove remaining jobs
    ui.pbSelectFiles->setEnabled(false);
    ui.pbCancel->setEnabled(false);
    ui.pbSuspend->setEnabled(false);
    ui.pbQuit->setEnabled(false);
    if ( m_weaver->isIdle() || m_weaver->state().stateId() == Suspended )
    {
        QApplication::instance()->quit();
    } else {
        m_quit = true;
        m_weaver->dequeue(); // on Weaver::finished() we exit
        m_weaver->resume();
    }
}

void SMIV::slotJobDone ( ThreadWeaver::Job* )
{
    ui.progressBar->setValue ( ui.progressBar->value() + 1 );
}

void SMIV::slotJobsDone ()
{
    if ( ! m_startTime.isNull() )
    {
        qDebug() << "SMIV::slotJobsDone: elapsed time: " << m_startTime.elapsed() << " msecs";
    }

    if ( m_quit )
    {
        QApplication::instance()->quit();
    } else {
        ui.pbSelectFiles->setEnabled(true);
        ui.pbCancel->setEnabled(false);
        ui.pbSuspend->setEnabled(false);

        ui.progressBar->reset();
        ui.progressBar->setEnabled (false);
    }
}

void SMIV::weaverSuspended()
{
    ui.pbSuspend->setText ( "Resume" );
    ui.pbSuspend->setEnabled ( true );
}


void SMIV::slotThumbReady ( SMIVItem *item )
{
    model.insert ( item );
    ui.listView->scrollTo ( model.index( model.rowCount()-1,  0 ),
                            QAbstractItemView::PositionAtBottom );
}

int main ( int argc,  char** argv )
{
    QApplication app ( argc,  argv );
    ThreadWeaver::setDebugLevel ( true, 1 );
    ThreadWeaver::Weaver weaver;
    weaver.setMaximumNumberOfThreads (8);
    SMIV smiv ( & weaver );
    smiv.show();
    return app.exec();
}

#include "SMIV.moc"
