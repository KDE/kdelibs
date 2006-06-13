/*
* Tests the KActiveLabel Widget class
*
* Copyright 2006 by Tobias Koenig <tokoe@kde.org>
*
* Licensed under the GNU General Public License version 2
*/

#include <QLayout>

#include <kaboutdata.h>
#include <kapplication.h>
#include <kcmdlineargs.h>

#include "kactivelabel.h"

#include "kactivelabeltest.h"

KActiveLabelTest::KActiveLabelTest(QWidget *parent)
    : QWidget(parent)
{
  setWindowTitle( "KActiveLabel test application" );

  QVBoxLayout *layout = new QVBoxLayout( this );

  KActiveLabel *label = new KActiveLabel( this );
  label->setHtml( "That's a small test text ;) <a href=\"whatsthis:click me if you can\">Click me</a>" );

  layout->addWidget( label );
}

int main( int argc, char ** argv )
{
  KAboutData about( "KActiveLabelTest", "KActiveLabelTest", "version");
  KCmdLineArgs::init(argc, argv, &about);

  KApplication app;

  KActiveLabelTest *toplevel = new KActiveLabelTest( 0 );
  toplevel->show();

  app.exec();
}

#include "kactivelabeltest.moc"

