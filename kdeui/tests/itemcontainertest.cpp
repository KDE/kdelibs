/*
* Tests the item container widgets KIconView, KListView, KListBox
*
* Copyright (c) 2000 by Michael Reiher <michael.reiher@gmx.de>
*
* License: GPL, version 2
* Version: $Id:
*
*/

#include <qlayout.h>
#include <qvbox.h>
#include <qhbox.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qlabel.h>

#include <kapplication.h>
#include <kglobal.h>
#include <kconfig.h>
#include <kiconview.h>
#include <klistview.h>
#include <klistbox.h>

#include "itemcontainertest.h"

static const char * item_xpm[] = {
"22 22 3 1",
" 	c None",
".	c #000000",
"+	c #FF0000",
"        ......        ",
"     ....++++....     ",
"    ..++++..++++..    ",
"   ..++++++++++++..   ",
"  ..++++++..++++++..  ",
" ..++++++++++++++++.. ",
" .++++++++..++++++++. ",
" .++++++++++++++++++. ",
"..++++++++..++++++++..",
".++++++++++++++++++++.",
".+.+.+.+.+..+.+.+.+.+.",
".+.+.+.+.+..+.+.+.+.+.",
".++++++++++++++++++++.",
"..++++++++..++++++++..",
" .++++++++++++++++++. ",
" .++++++++..++++++++. ",
" ..++++++++++++++++.. ",
"  ..++++++..++++++..  ",
"   ..++++++++++++..   ",
"    ..++++..++++..    ",
"     ....++++....     ",
"        ......        "};


KApplication *app;

TopLevel::TopLevel(QWidget *parent, const char *name)
    : QWidget(parent, name)
{
    setCaption("Item container test application");

    QHBoxLayout* hBox = new QHBoxLayout( this );
    QVBoxLayout* vBox = new QVBoxLayout( hBox );
    hBox->addSpacing( 5 );

    //Selection mode selection
    m_pbgMode = new QButtonGroup( 1, Qt::Horizontal, "Selection Mode", this);
    m_pbgMode->insert(new QRadioButton("NoSlection", m_pbgMode), TopLevel::NoSelection );
    m_pbgMode->insert(new QRadioButton("Single", m_pbgMode), TopLevel::Single );
    m_pbgMode->insert(new QRadioButton("Multi", m_pbgMode), TopLevel::Multi );
    m_pbgMode->insert(new QRadioButton("Extended", m_pbgMode), TopLevel::Extended );
    m_pbgMode->setExclusive( true );
    vBox->addWidget( m_pbgMode );

    connect( m_pbgMode, SIGNAL( clicked( int ) ),
	     this, SLOT( slotSwitchMode( int ) ) );

    //Signal labels
    QGroupBox* gbWiget = new QGroupBox( 1, Qt::Horizontal, "Widget", this);
    m_plblWidget = new QLabel( gbWiget );
    vBox->addWidget( gbWiget );
    QGroupBox* gbSignal = new QGroupBox( 1, Qt::Horizontal, "emitted Signal", this);
    m_plblSignal = new QLabel( gbSignal );
    vBox->addWidget( gbSignal );
    QGroupBox* gbItem = new QGroupBox( 1, Qt::Horizontal, "on Item", this);
    m_plblItem = new QLabel( gbItem );
    vBox->addWidget( gbItem );

    QButtonGroup* bgListView = new QButtonGroup( 1, Qt::Horizontal, "KListView", this);
    QCheckBox* cbListView = new QCheckBox("Single Column", bgListView);
    vBox->addWidget( bgListView );
    connect( cbListView, SIGNAL( toggled( bool ) ),
	     this, SLOT( slotToggleSingleColumn( bool ) ) );

    KGlobal::config()->reparseConfiguration();

    //Create IconView
    QGroupBox* gbIconView = new QGroupBox( 1, Qt::Horizontal, "KIconView", this);
    m_pIconView = new KIconView( gbIconView );
    hBox->addWidget( gbIconView );
    hBox->addSpacing( 5 );
    connect( m_pIconView, SIGNAL( executed( QIconViewItem* ) ),
	     this, SLOT( slotIconViewExec( QIconViewItem* ) ) );

    //Create ListView
    QGroupBox* gbListView = new QGroupBox( 1, Qt::Horizontal, "KListView", this);
    m_pListView = new KListView( gbListView );
    m_pListView->addColumn("Item");
    m_pListView->addColumn("Text");
    hBox->addWidget( gbListView );
    hBox->addSpacing( 5 );
    connect( m_pListView, SIGNAL( executed( QListViewItem* ) ),
	     this, SLOT( slotListViewExec( QListViewItem* ) ) );

    //Create ListBox
    QGroupBox* gbListBox = new QGroupBox( 1, Qt::Horizontal, "KListBox", this);
    m_pListBox = new KListBox( gbListBox );
    hBox->addWidget( gbListBox );
    connect( m_pListBox, SIGNAL( executed( QListBoxItem* ) ),
	     this, SLOT( slotListBoxExec( QListBoxItem* ) ) );

    //Initialize buttons
    cbListView->setChecked( !m_pListView->allColumnsShowFocus() );
    m_pbgMode->setButton( TopLevel::Extended );
    slotSwitchMode( TopLevel::Extended );

    //Fill container widgets
    for( int i = 0; i < 10; i++ ) {
      new QIconViewItem( m_pIconView, QString("Item%1").arg(i), QPixmap(item_xpm) );

      QListViewItem* lv = new QListViewItem( m_pListView, QString("Item%1").arg(i), QString("Text%1").arg(i) );
      lv->setPixmap( 0, QPixmap(item_xpm));
      lv->setPixmap( 1, QPixmap(item_xpm));
      
      new QListBoxPixmap( m_pListBox, QPixmap(item_xpm), QString("Item%1").arg(i));
    }

    connect( m_pIconView, SIGNAL( clicked( QIconViewItem* ) ),
	     this, SLOT( slotClicked( QIconViewItem* ) ) );
    connect( m_pIconView, SIGNAL( doubleClicked( QIconViewItem* ) ),
	     this, SLOT( slotDoubleClicked( QIconViewItem* ) ) );
}

void TopLevel::slotSwitchMode( int id ) 
{
  m_pIconView->clearSelection();
  m_pListView->clearSelection();
  m_pListBox->clearSelection();

  switch( id ) {
  case TopLevel::NoSelection:
    m_pIconView->setSelectionMode( KIconView::NoSelection );
    m_pListView->setSelectionMode( QListView::NoSelection );
    m_pListBox->setSelectionMode( KListBox::NoSelection );
    break;
  case TopLevel::Single:
    m_pIconView->setSelectionMode( KIconView::Single );
    m_pListView->setSelectionMode( QListView::Single );
    m_pListBox->setSelectionMode( KListBox::Single );
    break;
  case TopLevel::Multi:
    m_pIconView->setSelectionMode( KIconView::Multi );
    m_pListView->setSelectionMode( QListView::Multi );
    m_pListBox->setSelectionMode( KListBox::Multi );
    break;
  case TopLevel::Extended:
    m_pIconView->setSelectionMode( KIconView::Extended );
    m_pListView->setSelectionMode( QListView::Extended );
    m_pListBox->setSelectionMode( KListBox::Extended );
    break;
  default:
    Q_ASSERT(0);
  }
}

void TopLevel::slotIconViewExec( QIconViewItem* item )
{
  m_plblWidget->setText("KIconView");
  m_plblSignal->setText("executed");
  if( item ) 
    m_plblItem->setText( item->text() );
  else
    m_plblItem->setText("Viewport");
}

void TopLevel::slotListViewExec( QListViewItem* item )
{
  m_plblWidget->setText("KListView");
  m_plblSignal->setText("executed");
  if( item ) 
    m_plblItem->setText( item->text(0) );
  else
    m_plblItem->setText("Viewport");
}

void TopLevel::slotListBoxExec( QListBoxItem* item )
{
  m_plblWidget->setText("KListBox");
  m_plblSignal->setText("executed");
  if( item ) 
    m_plblItem->setText( item->text() );
  else
    m_plblItem->setText("Viewport");
}

void TopLevel::slotToggleSingleColumn( bool b )
{
  m_pListView->setAllColumnsShowFocus( !b );
}

int main( int argc, char ** argv )
{
    app = new KApplication ( argc, argv, "ItemContainerTest" );

    TopLevel *toplevel = new TopLevel(0, "itemcontainertest");

    toplevel->show();
    toplevel->resize( 600, 300 );
    app->setMainWidget(toplevel);
    app->exec();
}

#include "itemcontainertest.moc"
