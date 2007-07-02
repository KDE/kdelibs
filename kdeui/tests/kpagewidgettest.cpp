/*
    This file is part of the KDE Libraries

    Copyright (C) 2006 Tobias Koenig (tokoe@kde.org)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB. If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <QtGui/QGridLayout>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>

#include <kaboutdata.h>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kicon.h>

#include "kpagewidgetmodel.h"

#include "kpagewidgettest.h"

KPageWidgetTest::KPageWidgetTest( QWidget *parent )
  : QWidget( parent )
{
  QGridLayout *layout = new QGridLayout( this );

  mWidget = new KPageWidget( this );
  layout->addWidget( mWidget, 0, 0, 7, 1 );

  connect( mWidget, SIGNAL( currentPageChanged( KPageWidgetItem*, KPageWidgetItem* ) ),
           this, SLOT( currentPageChanged( KPageWidgetItem*, KPageWidgetItem* ) ) );
  connect( mWidget, SIGNAL( pageToggled( KPageWidgetItem*, bool ) ),
           this, SLOT( pageToggled( KPageWidgetItem*, bool ) ) );

  int rowCount = 0;
  QPushButton *button = new QPushButton( "Auto", this );
  layout->addWidget( button, rowCount, 1 );
  connect( button, SIGNAL( clicked() ), this, SLOT( setAutoFace() ) );
  rowCount++;

  button = new QPushButton( "Plain", this );
  layout->addWidget( button, rowCount, 1 );
  connect( button, SIGNAL( clicked() ), this, SLOT( setPlainFace() ) );
  rowCount++;

  button = new QPushButton( "List", this );
  layout->addWidget( button, rowCount, 1 );
  connect( button, SIGNAL( clicked() ), this, SLOT( setListFace() ) );
  rowCount++;

  button = new QPushButton( "Tree", this );
  layout->addWidget( button, rowCount, 1 );
  connect( button, SIGNAL( clicked() ), this, SLOT( setTreeFace() ) );
  rowCount++;

  button = new QPushButton( "Tabbed", this );
  layout->addWidget( button, rowCount, 1 );
  connect( button, SIGNAL( clicked() ), this, SLOT( setTabbedFace() ) );
  rowCount++;

  button = new QPushButton( "Add Page", this );
  layout->addWidget( button, rowCount, 1 );
  connect( button, SIGNAL( clicked() ), this, SLOT( addPage() ) );
  rowCount++;

  button = new QPushButton( "Add Sub Page", this );
  layout->addWidget( button, rowCount, 1 );
  connect( button, SIGNAL( clicked() ), this, SLOT( addSubPage() ) );
  rowCount++;

  button = new QPushButton( "Insert Page", this );
  layout->addWidget( button, rowCount, 1 );
  connect( button, SIGNAL( clicked() ), this, SLOT( insertPage() ) );
  rowCount++;

  button = new QPushButton( "Delete Page", this );
  layout->addWidget( button, rowCount, 1 );
  connect( button, SIGNAL( clicked() ), this, SLOT( deletePage() ) );
  rowCount++;

  KPageWidgetItem *item = mWidget->addPage( new QPushButton( "folder" ), "folder" );
  item->setIcon( KIcon( "folder" ) );
  item = mWidget->addSubPage( item, new QPushButton( "subfolder" ), "subfolder" );
  item->setIcon( KIcon( "folder" ) );
  item = mWidget->addPage( new QLabel( "second folder" ), "second folder" );
  item->setIcon( KIcon( "folder" ) );
}

KPageWidgetTest::~KPageWidgetTest()
{
}

void KPageWidgetTest::setAutoFace()
{
  mWidget->setFaceType( KPageWidget::Auto );
}

void KPageWidgetTest::setPlainFace()
{
  mWidget->setFaceType( KPageWidget::Plain );
}

void KPageWidgetTest::setListFace()
{
  mWidget->setFaceType( KPageWidget::List );
}

void KPageWidgetTest::setTreeFace()
{
  mWidget->setFaceType( KPageWidget::Tree );
}

void KPageWidgetTest::setTabbedFace()
{
  mWidget->setFaceType( KPageWidget::Tabbed );
}

void KPageWidgetTest::addPage()
{
  static int counter = 0;

  const QString title = QString( "dynamic folder %1" ).arg( QString::number( counter ) );
  KPageWidgetItem *item = mWidget->addPage( new QPushButton( title ) , title );
  item->setIcon( KIcon( "folder" ) );
  item->setHeader( QString( "Header Test No. %1" ).arg( QString::number( counter ) ) );
  item->setCheckable( true );

  counter++;
}

void KPageWidgetTest::addSubPage()
{
  static int counter = 0;

  KPageWidgetItem *item = mWidget->currentPage();
  if ( !item )
    return;

  const QString title = QString( "subfolder %1" ).arg( QString::number( counter ) );
  item = mWidget->addSubPage( item, new QLabel( title ) , title );
  item->setIcon( KIcon( "folder" ) );

  counter++;
}

void KPageWidgetTest::insertPage()
{
  static int counter = 0;

  KPageWidgetItem *item = mWidget->currentPage();
  if ( !item )
    return;

  const QString title = QString( "before folder %1" ).arg( QString::number( counter ) );
  item = mWidget->insertPage( item, new QLabel( title ) , title );
  item->setIcon( KIcon( "folder" ) );

  counter++;
}

void KPageWidgetTest::deletePage()
{
  KPageWidgetItem *item = mWidget->currentPage();
  if ( item )
    mWidget->removePage( item );
}

void KPageWidgetTest::currentPageChanged( KPageWidgetItem *current, KPageWidgetItem *before )
{
  if ( current )
    qDebug( "Current item: %s", qPrintable( current->name() ) );
  else
    qDebug( "No current item" );

  if ( before )
    qDebug( "Item before: %s", qPrintable( before->name() ) );
  else
    qDebug( "No item before" );
}

void KPageWidgetTest::pageToggled( KPageWidgetItem *item, bool checked )
{
  qDebug( "Item %s changed check state to: %s", qPrintable( item->name() ), checked ? "checked" : "unchecked" );
}

int main( int argc, char **argv )
{
  KAboutData about("KPageWidgetTest", 0, ki18n("KPageWidgetTest"), "version");
  KCmdLineArgs::init( argc, argv, &about );

  KApplication app;

  KPageWidgetTest testWidget( 0 );
  testWidget.show();

  return app.exec();
}

#include "kpagewidgettest.moc"
