#include <qcheckbox.h>
#include <qlayout.h>
#include <q3dragobject.h>
//Added by qt3to4:
#include <QGridLayout>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QLabel>
#include <QMenu>

#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kinputdialog.h>
#include <kdebug.h>

#include "ktabwidgettest.h"

Test::Test( QWidget* parent, const char *name )
  :Q3VBox( parent, name ), mChange(0), mLeftWidget(0), mRightWidget(0),
  mLeftPopup( false ), mRightPopup( false ), mTabbarContextPopup( false ), mContextPopup( false )

{
  resize( 600,300 );

  mWidget = new KTabWidget( this );
  mWidget->addTab( new QLabel( "Testlabel 1", mWidget ), "One" );
  mWidget->addTab( new QLabel( "Testlabel 2", mWidget ), "Two" );
  mWidget->addTab( new QWidget( mWidget), SmallIcon( "konsole" ), "Three" );
  mWidget->addTab( new QWidget( mWidget), "Four" );
#warning setTabColor not in Qt-4.0
#if 0
  mWidget->setTabColor( mWidget->page(0), Qt::red );
  mWidget->setTabColor( mWidget->page(1), Qt::blue );
#endif

  connect( mWidget, SIGNAL( currentChanged( QWidget * ) ), SLOT( currentChanged( QWidget * ) ) );
  connect( mWidget, SIGNAL( contextMenu( QWidget *, const QPoint & )), SLOT(contextMenu( QWidget *, const QPoint & )));
  connect( mWidget, SIGNAL( contextMenu( const QPoint & )), SLOT(tabbarContextMenu( const QPoint & )));
  connect( mWidget, SIGNAL( mouseDoubleClick( QWidget * )), SLOT(mouseDoubleClick( QWidget * )));
  connect( mWidget, SIGNAL( mouseMiddleClick() ), SLOT(addTab() ));
  connect( mWidget, SIGNAL( mouseMiddleClick( QWidget * )), SLOT(mouseMiddleClick( QWidget * )));
  connect( mWidget, SIGNAL( closeRequest( QWidget * )), SLOT(mouseMiddleClick( QWidget * )));
  connect( mWidget, SIGNAL( testCanDecode(const QDragMoveEvent *, bool & )), SLOT(testCanDecode(const QDragMoveEvent *, bool & )));
  connect( mWidget, SIGNAL( receivedDropEvent( QDropEvent * )), SLOT(receivedDropEvent( QDropEvent * )));
  connect( mWidget, SIGNAL( receivedDropEvent( QWidget *, QDropEvent * )), SLOT(receivedDropEvent( QWidget *, QDropEvent * )));
  connect( mWidget, SIGNAL( initiateDrag( QWidget * )), SLOT(initiateDrag( QWidget * )));
  connect( mWidget, SIGNAL( movedTab( int, int )), SLOT(movedTab( int, int )));
  mWidget->setTabReorderingEnabled( true );

  QWidget * grid = new QWidget(this);
  QGridLayout * gridlayout = new QGridLayout( grid, 5, 2 );

  QPushButton * addTab = new QPushButton( "Add Tab", grid );
  gridlayout->addWidget( addTab, 0, 0 );
  connect( addTab, SIGNAL( clicked() ), SLOT( addTab() ) );

  QPushButton * removeTab = new QPushButton( "Remove Current Tab", grid );
  gridlayout->addWidget( removeTab, 0, 1 );
  connect( removeTab, SIGNAL( clicked() ), SLOT( removeCurrentTab() ) );

  mLeftButton = new QCheckBox( "Show left button", grid );
  gridlayout->addWidget( mLeftButton, 1, 0 );
  connect( mLeftButton, SIGNAL( toggled(bool) ), SLOT( toggleLeftButton(bool) ) );
  mLeftButton->setChecked(true);

  QCheckBox * leftPopup = new QCheckBox( "Enable left popup", grid );
  gridlayout->addWidget( leftPopup, 2, 0 );
  connect( leftPopup, SIGNAL( toggled(bool) ), SLOT( toggleLeftPopup(bool) ) );
  leftPopup->setChecked(true);

  mRightButton = new QCheckBox( "Show right button", grid );
  gridlayout->addWidget( mRightButton, 1, 1 );
  connect( mRightButton, SIGNAL( toggled(bool) ), SLOT( toggleRightButton(bool) ) );
  mRightButton->setChecked(true);

  QCheckBox * rightPopup = new QCheckBox( "Enable right popup", grid );
  gridlayout->addWidget( rightPopup, 2, 1 );
  connect( rightPopup, SIGNAL( toggled(bool) ), SLOT( toggleRightPopup(bool) ) );
  rightPopup->setChecked(true);

  mTabsBottom = new QCheckBox( "Show tabs at bottom", grid );
  gridlayout->addWidget( mTabsBottom, 3, 0 );
  connect( mTabsBottom, SIGNAL( toggled(bool) ), SLOT( toggleTabPosition(bool) ) );

  QCheckBox * tabshape = new QCheckBox( "Triangular tab shape", grid );
  gridlayout->addWidget( tabshape, 3, 1 );
  connect( tabshape, SIGNAL( toggled(bool) ), SLOT( toggleTabShape(bool) ) );

  QCheckBox *tabClose = new QCheckBox( "Close button on icon hover", grid );
  gridlayout->addWidget( tabClose, 4, 0 );
  connect( tabClose, SIGNAL( toggled(bool) ), SLOT( toggleCloseButtons(bool) ) );
  tabClose->setChecked(true);

  QCheckBox * showlabels = new QCheckBox( "Show labels", grid );
  gridlayout->addWidget( showlabels, 4, 1 );
  connect( showlabels, SIGNAL( toggled(bool) ), this, SLOT( toggleLabels(bool) ) );
}

void Test::currentChanged(QWidget* w)
{
#if 0
  mWidget->setTabColor( w, Qt::black );
#endif
}

void Test::addTab()
{
  mWidget->addTab( new QWidget( mWidget ), SmallIcon( "konsole" ), QString("Tab %1").arg( mWidget->count()+1 ) );
}

void Test::testCanDecode(const QDragMoveEvent *e, bool &accept /* result */)
{
  if ( Q3TextDrag::canDecode(e) )    // don't accept=false if it cannot be decoded!
    accept = true;
}

void Test::receivedDropEvent( QDropEvent *e )
{
  QString dropText;
  if (Q3TextDrag::decode(e, dropText)) {
    mWidget->addTab( new QWidget( mWidget), dropText );
  }
}

void Test::receivedDropEvent( QWidget *w, QDropEvent *e )
{
  QString dropText;
  if (Q3TextDrag::decode(e, dropText)) {
    mWidget->changeTab( w, dropText );
  }
}

void Test::initiateDrag( QWidget *w )
{
   Q3DragObject *d = new Q3TextDrag( mWidget->label( mWidget->indexOf( w ) ), this );
   d->dragCopy(); // do NOT delete d.
}

void Test::removeCurrentTab()
{
  if ( mWidget->count()==1 ) return;

  mWidget->removePage( mWidget->currentPage() );
}

void Test::toggleLeftButton(bool state)
{
  if (state) {
    if (!mLeftWidget) {
      mLeftWidget = new QToolButton( mWidget );
      connect( mLeftWidget, SIGNAL( clicked() ), SLOT( addTab() ) );
      mLeftWidget->setIconSet( SmallIcon( "tab_new" ) );
      mLeftWidget->setTextLabel("New");
      mLeftWidget->setTextPosition(QToolButton::BesideIcon);
      mLeftWidget->adjustSize();
    //mLeftWidget->setGeometry( 0, 0, h, h );
      mLeftWidget->setPopup(mLeftPopup);
      mWidget->setCornerWidget( mLeftWidget, Qt::TopLeftCorner );
    }
    mLeftWidget->show();
  }
  else
    mLeftWidget->hide();
}

void Test::toggleLeftPopup(bool state)
{
  if (state) {
    if (!mLeftPopup) {
      mLeftPopup = new QMenu(this);
      mLeftPopup->insertItem(SmallIcon( "tab_new" ), "Empty Tab", 0);
      mLeftPopup->insertItem(SmallIcon( "tab_new" ), "Empty Tab After First", 3);
      mLeftPopup->insertSeparator();
      mLeftPopup->insertItem(SmallIcon( "tab_new" ), "Button Tab", 1);
      mLeftPopup->insertItem(SmallIcon( "tab_new" ), "Label Tab", 2);
      connect(mLeftPopup, SIGNAL(activated(int)), SLOT(leftPopupActivated(int)));
    }
    mLeftWidget->setPopup(mLeftPopup);
  }
  else
    mLeftWidget->setPopup(0);
}

void Test::leftPopupActivated(int item)
{
  switch (item) {
    case 0: mWidget->addTab( new QWidget( mWidget), QString("Tab %1").arg( mWidget->count()+1 ) );
            break;
    case 1: mWidget->addTab( new QPushButton( "Testbutton", mWidget ), QString("Tab %1").arg( mWidget->count()+1 ) );
            break;
    case 2: mWidget->addTab( new QLabel( "Testlabel", mWidget ), QString("Tab %1").arg( mWidget->count()+1 ) );
            break;
    case 3: mWidget->insertTab( new QWidget( mWidget), QString("Tab %1").arg( mWidget->count()+1 ), 1 );
  }
}

void Test::toggleRightButton(bool state)
{
if (state) {
    if ( !mRightWidget) {
      mRightWidget = new QToolButton( mWidget );
      QObject::connect( mRightWidget, SIGNAL( clicked() ), SLOT( removeCurrentTab() ) );
      mRightWidget->setIconSet( SmallIcon( "tab_remove" ) );
      mRightWidget->setTextLabel("Close");
      mRightWidget->setTextPosition(QToolButton::BesideIcon);
      mRightWidget->adjustSize();
    //mRightButton->setGeometry( 0, 0, h, h );
      mRightWidget->setPopup(mRightPopup);
      mWidget->setCornerWidget( mRightWidget, Qt::TopRightCorner );
    }
    mRightWidget->show();
  }
  else
    mRightWidget->hide();
}

void Test::toggleRightPopup(bool state)
{
  if (state) {
    if (!mRightPopup) {
      mRightPopup = new QMenu(this);
      mRightPopup->insertItem(SmallIcon( "tab_remove" ), "Current Tab", 1);
      mRightPopup->insertSeparator();
      mRightPopup->insertItem(SmallIcon( "tab_remove" ), "Most Left Tab", 0);
      mRightPopup->insertItem(SmallIcon( "tab_remove" ), "Most Right Tab", 2);
      connect(mRightPopup, SIGNAL(activated(int)), SLOT(rightPopupActivated(int)));
    }
    mRightWidget->setPopup(mRightPopup);
  }
  else
    mRightWidget->setPopup(0);
}

void Test::rightPopupActivated(int item)
{
  switch (item) {
    case 0: if ( mWidget->count() >1) {
              mWidget->removePage( mWidget->page(0) );
            }
            break;
    case 1: removeCurrentTab();
            break;
    case 2: int count = mWidget->count();
            if (count>1) {
              mWidget->removePage( mWidget->page(count-1) );
            }
  }
}

void Test::toggleTabPosition(bool state)
{
  mWidget->setTabPosition(state ? QTabWidget::North : QTabWidget::South );
}

void Test::toggleTabShape(bool state)
{
  mWidget->setTabShape(state ? QTabWidget::Triangular : QTabWidget::Rounded);
}

void Test::toggleCloseButtons(bool state)
{
  mWidget->setHoverCloseButton( state );
}

void Test::contextMenu(QWidget *w, const QPoint &p)
{
  if (mContextPopup)
      delete mContextPopup;

  mContextPopup = new QMenu(this);
  mContextPopup->insertItem( "Activate Tab", 4);
  mContextPopup->insertSeparator();
  mContextPopup->insertItem(SmallIcon( "konsole" ), "Set This Icon", 0);
  mContextPopup->insertItem(SmallIcon( "konqueror" ), "Set This Icon", 1);
  mContextPopup->insertSeparator();
  mContextPopup->insertItem( mWidget->isTabEnabled(w) ? "Disable Tab" : "Enable Tab", 2);
  mContextPopup->insertItem( mWidget->tabToolTip(w).isEmpty() ? "Set Tooltip" : "Remove Tooltip", 3);
  connect(mContextPopup, SIGNAL(activated(int)), SLOT(contextMenuActivated(int)));

  mContextWidget = w;
  mContextPopup->popup(p);
}

void Test::contextMenuActivated(int item)
{
  switch (item) {
    case 0: mWidget->changeTab( mContextWidget, SmallIcon( "konsole" ), mWidget->label( mWidget->indexOf( mContextWidget ) )  );
            break;
    case 1: mWidget->changeTab( mContextWidget, SmallIcon( "konqueror" ), mWidget->label( mWidget->indexOf( mContextWidget ) ) );
            break;
    case 2: mWidget->setTabEnabled( mContextWidget, !(mWidget->isTabEnabled(mContextWidget)) );
            break;
    case 3: if ( mWidget->tabToolTip(mContextWidget).isEmpty() )
              mWidget->setTabToolTip( mContextWidget, "This is a tool tip.");
            else
              mWidget->removeTabToolTip( mContextWidget );
            break;
    case 4: mWidget->showPage( mContextWidget );
  }
  delete mContextPopup;
  mContextPopup = 0;
}

void Test::tabbarContextMenu(const QPoint &p)
{
  if (mTabbarContextPopup)
      delete mTabbarContextPopup;

  mTabbarContextPopup = new QMenu(this);
  mTabbarContextPopup->insertItem(SmallIcon( "tab_new" ), mLeftWidget->isVisible() ? "Hide \"Add\" Button" : "Show \"Add\" Button", 0);
  mTabbarContextPopup->insertItem(SmallIcon( "tab_remove" ), mRightWidget->isVisible() ? "Hide \"Remove\" Button" : "Show \"Remove\" Button", 1);
  mTabbarContextPopup->insertSeparator();
  mTabbarContextPopup->insertItem(mWidget->tabPosition()==QTabWidget::North ? "Put Tabbar to Bottom" : "Put Tabbar to Top", 2);
  connect(mTabbarContextPopup, SIGNAL(activated(int)), SLOT(tabbarContextMenuActivated(int)));

  mTabbarContextPopup->popup(p);
}

void Test::tabbarContextMenuActivated(int item)
{
  switch (item) {
    case 0: mLeftButton->toggle();
            break;
    case 1: mRightButton->toggle();
            break;
    case 2: mTabsBottom->toggle();
  }
  delete mTabbarContextPopup;
  mTabbarContextPopup = 0;
}

void Test::mouseDoubleClick(QWidget *w)
{
  bool ok;
  QString text = KInputDialog::getText(
            "Rename Tab", "Enter new name:",
            mWidget->label( mWidget->indexOf( w ) ), &ok, this );
  if ( ok && !text.isEmpty() ) {
     mWidget->changeTab( w, text );
#if 0
     mWidget->setTabColor( w, Qt::green );
#endif
  }
}

void Test::mouseMiddleClick(QWidget *w)
{
  if ( mWidget->count()==1 ) return;

  mWidget->removePage( w );
}

void Test::movedTab(int from, int to)
{
  kdDebug() << "Moved tab from index " << from << " to " << to << endl;
}

void Test::toggleLabels(bool state)
{
  mLeftWidget->setUsesTextLabel(state);
  mLeftWidget->adjustSize();
  mRightWidget->setUsesTextLabel(state);
  mRightWidget->adjustSize();
  mWidget->hide();   // trigger update
  mWidget->show();
}

int main(int argc, char** argv )
{
    KCmdLineArgs::init(argc, argv, "ktabwidgettest", "KTabWidgetTest", "ktabwidget test app", "1.0");
    KApplication app;
    Test *t = new Test();

    app.setMainWidget( t );
    t->show();
    app.exec();
}

#include "ktabwidgettest.moc"
