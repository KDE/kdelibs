/*
* Tests the KActiveLabel Widget class
*
* Copyright (c) 2006 by Tobias Koenig <tokoe@kde.org>
*
* License: GPL, version 2
*/

#include <QLayout>

#include <kaboutdata.h>
#include <kapplication.h>
#include <kcmdlineargs.h>

#include <kactivelabel.h>

#include "kactivelabeltest.h"

KActiveLabelTest::KActiveLabelTest(QWidget *parent)
    : QWidget(parent)
{
  setCaption( "KActiveLabel test application" );

  QVBoxLayout *layout = new QVBoxLayout( this );

  KActiveLabel *label = new KActiveLabel( this );
  label->setText( "That's a small test text ;)" );

  layout->addWidget( label );
}

int main( int argc, char ** argv )
{
  KAboutData about( "KActiveLabelTest", "KActiveLabelTest", "version");
  KCmdLineArgs::init(argc, argv, &about);

  KApplication app;

  KActiveLabelTest *toplevel = new KActiveLabelTest( 0 );
  toplevel->show();

  app.setMainWidget( toplevel );
  app.exec();
}

#include "kactivelabeltest.moc"

