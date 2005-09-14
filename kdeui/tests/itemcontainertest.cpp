/*
* Tests the item container widgets KIconView, KListView, KListBox
*
* Copyright (c) 2000 by Michael Reiher <michael.reiher@gmx.de>
*
* License: GPL, version 2
*/

#include <qlayout.h>
#include <q3vbox.h>
#include <q3hbox.h>
#include <q3buttongroup.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qlabel.h>
//Added by qt3to4:
#include <QPixmap>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include <kapplication.h>
#include <kcmdlineargs.h>
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
    m_pbgMode = new Q3ButtonGroup( 1, Qt::Horizontal, "Selection Mode", this);
    m_pbgMode->insert(new QRadioButton("NoSlection", m_pbgMode), TopLevel::NoSelection );
    m_pbgMode->insert(new QRadioButton("Single", m_pbgMode), TopLevel::Single );
    m_pbgMode->insert(new QRadioButton("Multi", m_pbgMode), TopLevel::Multi );
    m_pbgMode->insert(new QRadioButton("Extended", m_pbgMode), TopLevel::Extended );
    m_pbgMode->setExclusive( true );
    vBox->addWidget( m_pbgMode );

    connect( m_pbgMode, SIGNAL( clicked( int ) ),
	     this, SLOT( slotSwitchMode( int ) ) );

    //Signal labels
    Q3GroupBox* gbWiget = new Q3GroupBox( 1, Qt::Horizontal, "Widget", this);
    m_plblWidget = new QLabel( gbWiget );
    vBox->addWidget( gbWiget );
    Q3GroupBox* gbSignal = new Q3GroupBox( 1, Qt::Horizontal, "emitted Signal", this);
    m_plblSignal = new QLabel( gbSignal );
    vBox->addWidget( gbSignal );
    Q3GroupBox* gbItem = new Q3GroupBox( 1, Qt::Horizontal, "on Item", this);
    m_plblItem = new QLabel( gbItem );
    vBox->addWidget( gbItem );

    Q3ButtonGroup* bgListView = new Q3ButtonGroup( 1, Qt::Horizontal, "KListView", this);
    QCheckBox* cbListView = new QCheckBox("Single Column", bgListView);
    vBox->addWidget( bgListView );
    connect( cbListView, SIGNAL( toggled( bool ) ),
	     this, SLOT( slotToggleSingleColumn( bool ) ) );

    KGlobal::config()->reparseConfiguration();

    //Create IconView
    Q3GroupBox* gbIconView = new Q3GroupBox( 1, Qt::Horizontal, "KIconView", this);
    m_pIconView = new KIconView( gbIconView );
    hBox->addWidget( gbIconView );
    hBox->addSpacing( 5 );
    connect( m_pIconView, SIGNAL( executed( Q3IconViewItem* ) ),
	     this, SLOT( slotIconViewExec( Q3IconViewItem* ) ) );

    //Create ListView
    Q3GroupBox* gbListView = new Q3GroupBox( 1, Qt::Horizontal, "KListView", this);
    m_pListView = new KListView( gbListView );
    m_pListView->addColumn("Item");
    m_pListView->addColumn("Text");
    hBox->addWidget( gbListView );
    hBox->addSpacing( 5 );
    connect( m_pListView, SIGNAL( executed( Q3ListViewItem* ) ),
	     this, SLOT( slotListViewExec( Q3ListViewItem* ) ) );

    //Create ListBox
    Q3GroupBox* gbListBox = new Q3GroupBox( 1, Qt::Horizontal, "KListBox", this);
    m_pListBox = new KListBox( gbListBox );
    hBox->addWidget( gbListBox );
    connect( m_pListBox, SIGNAL( executed( Q3ListBoxItem* ) ),
	     this, SLOT( slotListBoxExec( Q3ListBoxItem* ) ) );

    //Initialize buttons
    cbListView->setChecked( !m_pListView->allColumnsShowFocus() );
    m_pbgMode->setButton( TopLevel::Extended );
    slotSwitchMode( TopLevel::Extended );

    //Fill container widgets
    for( int i = 0; i < 10; i++ ) {
      new Q3IconViewItem( m_pIconView, QString("Item%1").arg(i), QPixmap(item_xpm) );

      Q3ListViewItem* lv = new Q3ListViewItem( m_pListView, QString("Item%1").arg(i), QString("Text%1").arg(i) );
      lv->setPixmap( 0, QPixmap(item_xpm));
      lv->setPixmap( 1, QPixmap(item_xpm));
      
      new Q3ListBoxPixmap( m_pListBox, QPixmap(item_xpm), QString("Item%1").arg(i));
    }

    connect( m_pIconView, SIGNAL( clicked( Q3IconViewItem* ) ),
	     this, SLOT( slotClicked( Q3IconViewItem* ) ) );
    connect( m_pIconView, SIGNAL( doubleClicked( Q3IconViewItem* ) ),
	     this, SLOT( slotDoubleClicked( Q3IconViewItem* ) ) );
}

void TopLevel::slotSwitchMode( int id ) 
{
  m_pIconView->clearSelection();
  m_pListView->clearSelection();
  m_pListBox->clearSelection();

  switch( id ) {
  case TopLevel::NoSelection:
    m_pIconView->setSelectionMode( KIconView::NoSelection );
    m_pListView->setSelectionMode( Q3ListView::NoSelection );
    m_pListBox->setSelectionMode( KListBox::NoSelection );
    break;
  case TopLevel::Single:
    m_pIconView->setSelectionMode( KIconView::Single );
    m_pListView->setSelectionMode( Q3ListView::Single );
    m_pListBox->setSelectionMode( KListBox::Single );
    break;
  case TopLevel::Multi:
    m_pIconView->setSelectionMode( KIconView::Multi );
    m_pListView->setSelectionMode( Q3ListView::Multi );
    m_pListBox->setSelectionMode( KListBox::Multi );
    break;
  case TopLevel::Extended:
    m_pIconView->setSelectionMode( KIconView::Extended );
    m_pListView->setSelectionMode( Q3ListView::Extended );
    m_pListBox->setSelectionMode( KListBox::Extended );
    break;
  default:
    Q_ASSERT(0);
  }
}

void TopLevel::slotIconViewExec( Q3IconViewItem* item )
{
  m_plblWidget->setText("KIconView");
  m_plblSignal->setText("executed");
  if( item ) 
    m_plblItem->setText( item->text() );
  else
    m_plblItem->setText("Viewport");
}

void TopLevel::slotListViewExec( Q3ListViewItem* item )
{
  m_plblWidget->setText("KListView");
  m_plblSignal->setText("executed");
  if( item ) 
    m_plblItem->setText( item->text(0) );
  else
    m_plblItem->setText("Viewport");
}

void TopLevel::slotListBoxExec( Q3ListBoxItem* item )
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
    KCmdLineArgs::init( argc, argv, "test", "Test" ,"test app" ,"1.0" );
    app = new KApplication;

    TopLevel *toplevel = new TopLevel(0, "itemcontainertest");

    toplevel->show();
    toplevel->resize( 600, 300 );
    app->setMainWidget(toplevel);
    app->exec();
}

#include "itemcontainertest.moc"
