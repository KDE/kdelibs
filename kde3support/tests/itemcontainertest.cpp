/*
* Tests the item container widgets K3IconView, K3ListView, KListWidget
*
* Copyright 2000 by Michael Reiher <michael.reiher@gmx.de>
*
* Licensed under the GNU General Public License version 2
*/

#include <QtGui/QLayout>
#include <QtGui/QRadioButton>
#include <QtGui/QCheckBox>
#include <QtGui/QLabel>
#include <QtGui/QGroupBox>
#include <QtGui/QButtonGroup>

#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kglobal.h>
#include <kconfig.h>
#include <k3iconview.h>
#include <k3listview.h>
#include <klistwidget.h>

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


TopLevel::TopLevel(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle("Item container test application");

    QHBoxLayout* hBox = new QHBoxLayout( this );
    QVBoxLayout* vBox = new QVBoxLayout;
    hBox->addSpacing( 5 );
    hBox->addItem( vBox );
    vBox->setParent( hBox );


    //Selection mode selection
    QGroupBox* selectionModeBox = new QGroupBox("Selection Mode",this);
    m_pbgMode = new QButtonGroup(this); 
    m_pbgMode->addButton(new QRadioButton("NoSlection", selectionModeBox), TopLevel::NoSelection );
    m_pbgMode->addButton(new QRadioButton("Single", selectionModeBox), TopLevel::Single );
    m_pbgMode->addButton(new QRadioButton("Multi", selectionModeBox), TopLevel::Multi );
    m_pbgMode->addButton(new QRadioButton("Extended", selectionModeBox), TopLevel::Extended );
    m_pbgMode->setExclusive( true );
    vBox->addWidget( selectionModeBox );

    connect( m_pbgMode, SIGNAL( buttonClicked( int ) ),
	     this, SLOT( slotSwitchMode( int ) ) );

    //Signal labels
    QGroupBox* gbWiget = new QGroupBox(  "Widget", this);
    m_plblWidget = new QLabel( gbWiget );
    vBox->addWidget( gbWiget );
    QGroupBox* gbSignal = new QGroupBox(  "emitted Signal", this);
    m_plblSignal = new QLabel( gbSignal );
    vBox->addWidget( gbSignal );
    QGroupBox* gbItem = new QGroupBox(  "on Item", this);
    m_plblItem = new QLabel( gbItem );
    vBox->addWidget( gbItem );

    QGroupBox* bgListView = new QGroupBox( "K3ListView", this);
    QCheckBox* cbListView = new QCheckBox("Single Column", bgListView);
    vBox->addWidget( bgListView );
    connect( cbListView, SIGNAL( toggled( bool ) ),
	     this, SLOT( slotToggleSingleColumn( bool ) ) );

    KGlobal::config()->reparseConfiguration();

    //Create IconView
    QGroupBox* gbIconView = new QGroupBox( "K3IconView", this);
    m_pIconView = new K3IconView( gbIconView );
    hBox->addWidget( gbIconView );
    hBox->addSpacing( 5 );
    connect( m_pIconView, SIGNAL( executed( Q3IconViewItem* ) ),
	     this, SLOT( slotIconViewExec( Q3IconViewItem* ) ) );

    //Create ListView
    QGroupBox* gbListView = new QGroupBox(  "K3ListView", this);
    m_pListView = new K3ListView( gbListView );
    m_pListView->addColumn("Item");
    m_pListView->addColumn("Text");
    hBox->addWidget( gbListView );
    hBox->addSpacing( 5 );
    connect( m_pListView, SIGNAL( executed( Q3ListViewItem* ) ),
	     this, SLOT( slotListViewExec( Q3ListViewItem* ) ) );

    //Create ListBox
    QGroupBox* gbListBox = new QGroupBox(  "KListWidget", this);
    m_pListBox = new KListWidget( gbListBox );
    hBox->addWidget( gbListBox );
    connect( m_pListBox, SIGNAL( executed( QListWidgetItem* ) ),
	     this, SLOT( slotListBoxExec( QListWidgetItem* ) ) );

    //Initialize buttons
    cbListView->setChecked( !m_pListView->allColumnsShowFocus() );
    m_pbgMode->button( TopLevel::Extended )->setChecked(true);
    slotSwitchMode( TopLevel::Extended );

    //Fill container widgets
    for( int i = 0; i < 10; i++ ) {
      new Q3IconViewItem( m_pIconView, QString("Item%1").arg(i), QPixmap(item_xpm) );

      Q3ListViewItem* lv = new Q3ListViewItem( m_pListView, QString("Item%1").arg(i), QString("Text%1").arg(i) );
      lv->setPixmap( 0, QPixmap(item_xpm));
      lv->setPixmap( 1, QPixmap(item_xpm));
      
      QListWidgetItem* listWidgetItem = new QListWidgetItem(m_pListBox);
      listWidgetItem->setIcon( QPixmap(item_xpm) );
      listWidgetItem->setText( QString("Item%1").arg(i) );
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
    m_pIconView->setSelectionMode( K3IconView::NoSelection );
    m_pListView->setSelectionMode( Q3ListView::NoSelection );
    m_pListBox->setSelectionMode( KListWidget::NoSelection );
    break;
  case TopLevel::Single:
    m_pIconView->setSelectionMode( K3IconView::Single );
    m_pListView->setSelectionMode( Q3ListView::Single );
    m_pListBox->setSelectionMode( KListWidget::SingleSelection );
    break;
  case TopLevel::Multi:
    m_pIconView->setSelectionMode( K3IconView::Multi );
    m_pListView->setSelectionMode( Q3ListView::Multi );
    m_pListBox->setSelectionMode( KListWidget::MultiSelection );
    break;
  case TopLevel::Extended:
    m_pIconView->setSelectionMode( K3IconView::Extended );
    m_pListView->setSelectionMode( Q3ListView::Extended );
    m_pListBox->setSelectionMode( KListWidget::ExtendedSelection );
    break;
  default:
    Q_ASSERT(0);
  }
}

void TopLevel::slotIconViewExec( Q3IconViewItem* item )
{
  m_plblWidget->setText("K3IconView");
  m_plblSignal->setText("executed");
  if( item ) 
    m_plblItem->setText( item->text() );
  else
    m_plblItem->setText("Viewport");
}

void TopLevel::slotListViewExec( Q3ListViewItem* item )
{
  m_plblWidget->setText("K3ListView");
  m_plblSignal->setText("executed");
  if( item ) 
    m_plblItem->setText( item->text(0) );
  else
    m_plblItem->setText("Viewport");
}

void TopLevel::slotListBoxExec( QListWidgetItem* item )
{
  m_plblWidget->setText("KListWidget");
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
    KCmdLineArgs::init( argc, argv, "test", 0, ki18n("Test"), "1.0", ki18n("test app"));
    KApplication app;

    TopLevel *toplevel = new TopLevel(0);

    toplevel->show();
    toplevel->resize( 600, 300 );
    app.exec();
}

#include "itemcontainertest.moc"
