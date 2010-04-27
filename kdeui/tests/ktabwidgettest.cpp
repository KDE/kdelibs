#include <QtGui/QLayout>
#include <QtGui/QKeyEvent>
#include <QtGui/QMenu>
#include <QtCore/QMimeData>

#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kinputdialog.h>
#include <kdebug.h>

#include "ktabwidgettest.h"

Test::Test( QWidget* parent )
  :KVBox( parent ), mChange(0), mLeftWidget(0), mRightWidget(0),
  mLeftPopup( false ), mRightPopup( false ), mTabbarContextPopup( false ), mContextPopup( false )

{
  resize( 600,300 );

  mWidget = new KTabWidget( this );
  mWidget->addTab( new QLabel( "Testlabel 1", 0 ), "&One" );
  mWidget->addTab( new QLabel( "Testlabel 2", 0 ), "Two" );
  mWidget->addTab( new QWidget(), SmallIcon( "konsole" ), "Three" );
  mWidget->addTab( new QWidget(), "Four" );
  mWidget->setTabTextColor( 0, Qt::red );
  mWidget->setTabTextColor( 1, Qt::blue );
  mWidget->setUsesScrollButtons( false ); // corresponding checkbox is unchecked by default

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
  QGridLayout * gridlayout = new QGridLayout( grid );

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

  QCheckBox * elideText = new QCheckBox( "Elide text", grid );
  gridlayout->addWidget( elideText, 5, 0 );
  connect( elideText, SIGNAL( toggled(bool) ), this, SLOT( toggleEliding(bool) ) );

  QCheckBox * scrollButtons = new QCheckBox( "Enable scroll buttons", grid );
  gridlayout->addWidget( scrollButtons, 5, 1 );
  connect( scrollButtons, SIGNAL( toggled(bool) ), this, SLOT( toggleScrollButtons(bool) ) );
}

void Test::currentChanged(QWidget* w)
{
  mWidget->setTabTextColor( mWidget->indexOf(w), Qt::black );
}

void Test::addTab()
{
  mWidget->addTab( new QWidget(), SmallIcon( "konsole" ), QString("This is tab %1").arg( mWidget->count()+1 ) );
}

void Test::testCanDecode(const QDragMoveEvent *e, bool &accept /* result */)
{
  if ( e->mimeData()->hasText() )    // don't accept=false if it cannot be decoded!
    accept = true;
}

void Test::receivedDropEvent( QDropEvent *e )
{
  if (e->mimeData()->hasText()) {
    mWidget->addTab( new QWidget(), e->mimeData()->text() );
  }
}

void Test::receivedDropEvent( QWidget *w, QDropEvent *e )
{
  if (e->mimeData()->hasText()) {
    mWidget->setTabText( mWidget->indexOf( w ), e->mimeData()->text() );
  }
}

void Test::initiateDrag( QWidget *w )
{
   QDrag *drag = new QDrag( this );
   QMimeData *mimeData = new QMimeData;
   mimeData->setText(mWidget->tabText( mWidget->indexOf(w)));
   drag->setMimeData(mimeData);
   drag->start(); // do NOT delete d.
}

void Test::removeCurrentTab()
{
  if ( mWidget->count()==1 ) return;

  mWidget->removeTab( mWidget->currentIndex() );
}

void Test::toggleLeftButton(bool state)
{
  if (state) {
    if (!mLeftWidget) {
      mLeftWidget = new QToolButton( mWidget );
      connect( mLeftWidget, SIGNAL( clicked() ), SLOT( addTab() ) );
      mLeftWidget->setIcon( SmallIcon( "tab-new" ) );
      mLeftWidget->setText("New");
      mLeftWidget->setToolTip("New");
      mLeftWidget->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
      mLeftWidget->adjustSize();
    //mLeftWidget->setGeometry( 0, 0, h, h );
      mLeftWidget->setMenu(mLeftPopup);
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
      mLeftPopup->addAction(SmallIcon( "tab-new" ), "Empty Tab");
      mLeftPopup->addAction(SmallIcon( "tab-new" ), "Empty Tab After First");
      mLeftPopup->addSeparator();
      mLeftPopup->addAction(SmallIcon( "tab-new" ), "Button Tab");
      mLeftPopup->addAction(SmallIcon( "tab-new" ), "Label Tab");
      connect(mLeftPopup, SIGNAL(triggered(QAction*)), SLOT(leftPopupActivated(QAction*)));
    }
    mLeftWidget->setMenu(mLeftPopup);
  }
  else
    mLeftWidget->setMenu(0);
}

void Test::leftPopupActivated(QAction *action)
{
  switch (mLeftPopup->actions().indexOf(action)){
    case 0: mWidget->addTab( new QWidget(), QString("Tab %1").arg( mWidget->count()+1 ) );
            break;
    case 1: mWidget->insertTab( 1, new QWidget(), QString("Tab %1").arg( mWidget->count()+1 ) );
            break;
    case 3: mWidget->addTab( new QPushButton( "Testbutton" ), QString("Tab %1").arg( mWidget->count()+1 ) );
            break;
    case 4: mWidget->addTab( new QLabel( "Testlabel" ), QString("Tab %1").arg( mWidget->count()+1 ) );
            break;
  }
}

void Test::toggleRightButton(bool state)
{
if (state) {
    if ( !mRightWidget) {
      mRightWidget = new QToolButton( mWidget );
      QObject::connect( mRightWidget, SIGNAL( clicked() ), SLOT( removeCurrentTab() ) );
      mRightWidget->setIcon( SmallIcon( "tab-close" ) );
      mRightWidget->setText("Close");
      mRightWidget->setToolTip("Close");
      mRightWidget->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
      mRightWidget->adjustSize();
    //mRightButton->setGeometry( 0, 0, h, h );
      mRightWidget->setMenu(mRightPopup);
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
      mRightPopup->addAction(SmallIcon( "tab-close" ), "Current Tab");
      mRightPopup->addSeparator();
      mRightPopup->addAction(SmallIcon( "tab-close" ), "Most Left Tab");
      mRightPopup->addAction(SmallIcon( "tab-close" ), "Most Right Tab");
      connect(mRightPopup, SIGNAL(triggered(QAction*)), SLOT(rightPopupActivated(QAction*)));
    }
    mRightWidget->setMenu(mRightPopup);
  }
  else
    mRightWidget->setMenu(0);
}

void Test::rightPopupActivated(QAction *action)
{
  switch (mRightPopup->actions().indexOf(action)) {
    case 0: removeCurrentTab();
            break;
    case 2: if ( mWidget->count() >1) {
              mWidget->removeTab( 0 );
            }
            break;
    case 3: int count = mWidget->count();
            if (count>1) {
              mWidget->removeTab( count-1 );
            }
            break;
  }
}

void Test::toggleTabPosition(bool state)
{
  mWidget->setTabPosition(state ? QTabWidget::South : QTabWidget::North );
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

  int idx = mWidget->indexOf( w );
  mContextPopup = new QMenu(this);
  mContextPopup->addAction( "Activate Tab");
  mContextPopup->addSeparator();
  mContextPopup->addAction(SmallIcon( "konsole" ), "Set This Icon");
  mContextPopup->addAction(SmallIcon( "konqueror" ), "Set This Icon");
  mContextPopup->addSeparator();
  mContextPopup->addAction( mWidget->isTabEnabled(idx) ? "Disable Tab" : "Enable Tab");
  mContextPopup->addAction( mWidget->tabToolTip(idx).isEmpty() ? "Set Tooltip" : "Remove Tooltip");
  connect(mContextPopup, SIGNAL(triggered(QAction*)), SLOT(contextMenuActivated(QAction*)));

  mContextWidgetIndex = mWidget->indexOf( w );
  mContextPopup->popup(p);
}

void Test::contextMenuActivated(QAction *action)
{
  switch (mContextPopup->actions().indexOf(action)) {
  case 0:
      mWidget->setCurrentIndex( mContextWidgetIndex );
      break;
  case 2:
      mWidget->setTabIcon( mContextWidgetIndex, SmallIcon( "konsole" ) );
      break;
  case 3:
      mWidget->setTabIcon( mContextWidgetIndex, SmallIcon( "konqueror" ) );
      break;
  case 4:
      mWidget->setTabEnabled( mContextWidgetIndex, !(mWidget->isTabEnabled(mContextWidgetIndex)) );
      break;
  case 5:
      if ( mWidget->tabToolTip(mContextWidgetIndex).isEmpty() )
          mWidget->setTabToolTip( mContextWidgetIndex, "This is a tool tip.");
      else
          mWidget->setTabToolTip( mContextWidgetIndex, QString() );
      break;
  }
}

void Test::tabbarContextMenu(const QPoint &p)
{
  if (mTabbarContextPopup)
      delete mTabbarContextPopup;

  mTabbarContextPopup = new QMenu(this);
  mTabbarContextPopup->addAction(SmallIcon( "tab-new" ), mLeftWidget->isVisible() ? "Hide \"Add\" Button" : "Show \"Add\" Button");
  mTabbarContextPopup->addAction(SmallIcon( "tab-close" ), mRightWidget->isVisible() ? "Hide \"Remove\" Button" : "Show \"Remove\" Button");
  mTabbarContextPopup->addSeparator();
  mTabbarContextPopup->addAction(mWidget->tabPosition()==QTabWidget::North ? "Put Tabbar to Bottom" : "Put Tabbar to Top");
  connect(mTabbarContextPopup, SIGNAL(triggered(QAction*)), SLOT(tabbarContextMenuActivated(QAction*)));

  mTabbarContextPopup->popup(p);
}

void Test::tabbarContextMenuActivated(QAction *action)
{
  switch (mTabbarContextPopup->actions().indexOf(action)) {
    case 0: mLeftButton->toggle();
            break;
    case 1: mRightButton->toggle();
            break;
    case 3: mTabsBottom->toggle();
            break;
  }
}

void Test::mouseDoubleClick(QWidget *w)
{
  int index = mWidget->indexOf( w );
  bool ok;
  QString text = KInputDialog::getText(
            "Rename Tab", "Enter new name:",
            mWidget->tabText( index ), &ok, this );
  if ( ok && !text.isEmpty() ) {
     mWidget->setTabText( index, text );
     mWidget->setTabTextColor( index, Qt::green );
  }
}

void Test::mouseMiddleClick(QWidget *w)
{
  if ( mWidget->count()==1 ) return;

  mWidget->removeTab( mWidget->indexOf( w ) );
}

void Test::movedTab(int from, int to)
{
  kDebug() << "Moved tab from index " << from << " to " << to;
}

void Test::toggleLabels(bool state)
{
  mLeftWidget->setToolButtonStyle(state?Qt::ToolButtonTextUnderIcon:Qt::ToolButtonIconOnly);
  mLeftWidget->adjustSize();
  mRightWidget->setToolButtonStyle(state?Qt::ToolButtonTextUnderIcon:Qt::ToolButtonIconOnly);
  mRightWidget->adjustSize();
  mWidget->hide();   // trigger update
  mWidget->show();
}

void Test::toggleScrollButtons(bool state)
{
  mWidget->setUsesScrollButtons(state);
}

void Test::toggleEliding(bool state)
{
  mWidget->setAutomaticResizeTabs(state);
  //mWidget->setElideMode(state ? Qt::ElideRight : Qt::ElideNone);
}


int main(int argc, char** argv )
{
    KCmdLineArgs::init(argc, argv, "ktabwidgettest", 0, ki18n("KTabWidgetTest"), "1.0", ki18n("ktabwidget test app"));
    //KApplication::disableAutoDcopRegistration();
    KApplication app;
    Test *t = new Test();
    t->show();
    app.exec();
}

#include "ktabwidgettest.moc"
