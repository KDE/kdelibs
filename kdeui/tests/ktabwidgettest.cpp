#include <qcheckbox.h>
#include <qlayout.h>
#include <qdragobject.h>
#include <qinputdialog.h>

#include "tab.h"

Test::Test( QWidget* parent, const char *name )
  :QVBox( parent, name ), mChange(0), mRed( false ), mLeftPopup( false ), mRightPopup( false ), mContextPopup( false ), mTabbarContextPopup( false )
{
  resize( 600,300 );

  mWidget = new KTabWidget( this );
  mWidget->setLeftButtonPixmap( SmallIcon( "tab_new" ) );
  mWidget->setRightButtonPixmap( SmallIcon( "tab_remove" ) );
  mList.append( mWidget->addChangeableTab( "One" ) );
  mList.append( mWidget->addChangeableTab( "Twoooooo" ) );
  mList.append( mWidget->addChangeableTab( "Threeeeeeee" ) );
  mList.append( mWidget->addChangeableTab( "Four" ) );
  mWidget->changeTab( mList[0], new QLabel( "Testlabel 1", mWidget ) );
  mWidget->changeTab( mList[1], new QLabel( "Testlabel 2", mWidget ) );
  mWidget->changeTab( mList[0], Qt::red );
  mWidget->changeTab( mList[1], Qt::blue );
  mWidget->changeTab( mList[1], QString("Two") );
  mWidget->changeTab( mList[2], SmallIcon( "konsole" ), "Three" );

  connect( mWidget, SIGNAL( currentChanged( QWidget * ) ), this, SLOT( currentChanged( QWidget * ) ) );
  connect( mWidget, SIGNAL( contextMenu( QWidget *, const QPoint & )), this, SLOT(contextMenu( QWidget *, const QPoint & )));
  connect( mWidget, SIGNAL( tabbarContextMenu( const QPoint & )), this, SLOT(tabbarContextMenu( const QPoint & )));
  connect( mWidget, SIGNAL( mouseDoubleClick( QWidget * )), this, SLOT(mouseDoubleClick( QWidget * )));
  connect( mWidget, SIGNAL( mouseMiddleClick( QWidget * )), this, SLOT(mouseMiddleClick( QWidget * )));
  connect( mWidget, SIGNAL( receivedDropEvent( QDropEvent * )), this, SLOT(receivedDropEvent( QDropEvent * )));
  connect( mWidget, SIGNAL( receivedDropEvent( QWidget *, QDropEvent * )), this, SLOT(receivedDropEvent( QWidget *, QDropEvent * )));
  connect( mWidget, SIGNAL( dragInitiated( QWidget * )), this, SLOT(dragInitiated( QWidget * )));
  connect( mWidget, SIGNAL( movedTab( int, int, int )), this, SLOT(movedTab( int, int, int )));
  mWidget->setTabReorderingEnabled( true );

  QWidget * grid = new QWidget(this);
  QGridLayout * gridlayout = new QGridLayout( grid, 5, 2 );

  QPushButton * addTab = new QPushButton( "Add Tab", grid );
  gridlayout->addWidget( addTab, 0, 0 );
  connect( addTab, SIGNAL( clicked() ), this, SLOT( addTab() ) );
  connect( mWidget, SIGNAL( leftButtonClicked() ), this, SLOT( addTab() ) );

  QPushButton * removeTab = new QPushButton( "Remove Current Tab", grid );
  gridlayout->addWidget( removeTab, 0, 1 );
  connect( removeTab, SIGNAL( clicked() ), this, SLOT( removeCurrentTab() ) );
  connect( mWidget, SIGNAL( rightButtonClicked() ), this, SLOT( removeCurrentTab() ) );

  mLeftButton = new QCheckBox( "Show left button", grid );
  gridlayout->addWidget( mLeftButton, 1, 0 );
  connect( mLeftButton, SIGNAL( toggled(bool) ), this, SLOT( toggleLeftButton(bool) ) );
  mLeftButton->setChecked(true);

  QCheckBox * leftPopup = new QCheckBox( "Enable left popup", grid );
  gridlayout->addWidget( leftPopup, 2, 0 );
  connect( leftPopup, SIGNAL( toggled(bool) ), this, SLOT( toggleLeftPopup(bool) ) );
  leftPopup->setChecked(true);

  mRightButton = new QCheckBox( "Show right button", grid );
  gridlayout->addWidget( mRightButton, 1, 1 );
  connect( mRightButton, SIGNAL( toggled(bool) ), this, SLOT( toggleRightButton(bool) ) );
  mRightButton->setChecked(true);

  QCheckBox * rightPopup = new QCheckBox( "Enable right popup", grid );
  gridlayout->addWidget( rightPopup, 2, 1 );
  connect( rightPopup, SIGNAL( toggled(bool) ), this, SLOT( toggleRightPopup(bool) ) );
  rightPopup->setChecked(true);

  mTabsBottom = new QCheckBox( "Show tabs at bottom", grid );
  gridlayout->addWidget( mTabsBottom, 3, 0 );
  connect( mTabsBottom, SIGNAL( toggled(bool) ), this, SLOT( toggleTabPosition(bool) ) );

  QCheckBox * tabshape = new QCheckBox( "Triangular tab shape", grid );
  gridlayout->addWidget( tabshape, 3, 1 );
  connect( tabshape, SIGNAL( toggled(bool) ), this, SLOT( toggleTabShape(bool) ) );

  QCheckBox *tabClose = new QCheckBox( "Close buttons on tabs", grid );
  gridlayout->addWidget( tabClose, 4, 0 );
  connect( tabClose, SIGNAL( toggled(bool) ), this, SLOT( toggleCloseButtons(bool) ) );

  mTimer = new QTimer( this );
  srand( time(0) );
  connect( mTimer, SIGNAL(timeout()), this, SLOT(timerDone()) );
  mTimer->start( 3000 ); // 3 seconds single-shot timer
}

void Test::currentChanged(QWidget* w)
{
  mWidget->changeTab( w, Qt::black );
}

void Test::addTab()
{
  mList.append( mWidget->addChangeableTab( QString("Tab %1").arg(mList.size()+1) ) );
}

void Test::receivedDropEvent( QDropEvent *e )
{
  QString dropText;
  if (QTextDrag::decode(e, dropText)) {
    mList.append( mWidget->addChangeableTab( dropText ) );
  }
}

void Test::receivedDropEvent( QWidget *w, QDropEvent *e )
{
  QString dropText;
  if (QTextDrag::decode(e, dropText)) {
    mWidget->changeTab( w, dropText );
  }
}

void Test::dragInitiated( QWidget *w )
{
   QDragObject *d = new QTextDrag( mWidget->label( mWidget->indexOf( w ) ), this );
   d->dragCopy(); // do NOT delete d.
}

void Test::removeCurrentTab()
{
  if (mList.size()==1) return;

  IntList::iterator it = mList.at( mWidget->currentPageIndex() );
  mList.erase( it );
  mWidget->removePage( mWidget->currentPage() );
}

void Test::toggleLeftButton(bool state)
{
  mWidget->setLeftButton(state);
}

void Test::toggleLeftPopup(bool state)
{
  if (state) {
    if (!mLeftPopup) {
      mLeftPopup = new QPopupMenu(this);
      mLeftPopup->insertItem(SmallIcon( "tab_new" ), "Empty Tab", 0);
      mLeftPopup->insertItem(SmallIcon( "tab_new" ), "Empty Tab After First", 3);
      mLeftPopup->insertSeparator();
      mLeftPopup->insertItem(SmallIcon( "tab_new" ), "Button Tab", 1);
      mLeftPopup->insertItem(SmallIcon( "tab_new" ), "Label Tab", 2);
      connect(mLeftPopup, SIGNAL(activated(int)), SLOT(leftPopupActivated(int)));
    }
    mWidget->setLeftButtonPopup(mLeftPopup);
  }
  else
    mWidget->setLeftButtonPopup(0);
}

void Test::leftPopupActivated(int item)
{
  switch (item) {
    case 0: mList.append( mWidget->addChangeableTab( QString("Tab %1").arg(mList.size()+1) ) );
            break;
    case 1: mList.append( mWidget->addChangeableTab( QString("Tab %1").arg(mList.size()+1), new QPushButton( "Testbutton", mWidget ) ) );
            break;
    case 2: mList.append( mWidget->addChangeableTab( QString("Tab %1").arg(mList.size()+1), new QLabel( "Testlabel", mWidget ) ) );
    case 3: IntList::iterator it = mList.begin();
            it++;
            if (*it)
              mList.insert(it, mWidget->insertChangeableTab( QString("Tab %1").arg(mList.size()+1), 1 ) );
	    else
              mList.append( mWidget->addChangeableTab( QString("Tab %1").arg(mList.size()+1) ) );
  }
}

void Test::toggleRightButton(bool state)
{
  mWidget->setRightButton(state);
}

void Test::toggleRightPopup(bool state)
{
  if (state) {
    if (!mRightPopup) {
      mRightPopup = new QPopupMenu(this);
      mRightPopup->insertItem(SmallIcon( "tab_remove" ), "Current Tab", 1);
      mRightPopup->insertSeparator();
      mRightPopup->insertItem(SmallIcon( "tab_remove" ), "Most Left Tab", 0);
      mRightPopup->insertItem(SmallIcon( "tab_remove" ), "Most Right Tab", 2);
      connect(mRightPopup, SIGNAL(activated(int)), SLOT(rightPopupActivated(int)));
    }
    mWidget->setRightButtonPopup(mRightPopup);
  }
  else
    mWidget->setRightButtonPopup(0);
}

void Test::rightPopupActivated(int item)
{
  IntList::iterator it;
  switch (item) {
    case 0: if (mList.size()>1) {
              it = mList.at( 0 );
              mList.erase( it );
              mWidget->removePage( mWidget->page(0) );
            }
            break;
    case 1: removeCurrentTab();
            break;
    case 2: int count = mList.size();
            if (count>1) {
              it = mList.at( count-1 );
              mList.erase( it );
              mWidget->removePage( mWidget->page(count-1) );
            }
  }
}

void Test::toggleTabPosition(bool state)
{
  mWidget->setTabPosition(state ? QTabWidget::Bottom : QTabWidget::Top);
}

void Test::toggleTabShape(bool state)
{
  mWidget->setTabShape(state ? QTabWidget::Triangular : QTabWidget::Rounded);
}

void Test::toggleCloseButtons(bool state)
{
  mWidget->setCloseButtons( state );
}

void Test::contextMenu(QWidget *w, const QPoint &p)
{
  if (mContextPopup)
      delete mContextPopup;

  mContextPopup = new QPopupMenu(this);
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

  mTabbarContextPopup = new QPopupMenu(this);
  mTabbarContextPopup->insertItem(SmallIcon( "tab_new" ), mWidget->isLeftButton() ? "Hide \"Add\" Button" : "Show \"Add\" Button", 0);
  mTabbarContextPopup->insertItem(SmallIcon( "tab_remove" ), mWidget->isRightButton() ? "Hide \"Remove\" Button" : "Show \"Remove\" Button", 1);
  mTabbarContextPopup->insertSeparator();
  mTabbarContextPopup->insertItem(mWidget->tabPosition()==QTabWidget::Top ? "Put Tabbar to Bottom" : "Put Tabbar to Top", 2);
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
  QString text = QInputDialog::getText(
            "Rename Tab", "Enter new name:", QLineEdit::Normal,
            mWidget->label( mWidget->indexOf( w ) ), &ok, this );
  if ( ok && !text.isEmpty() ) {
     mWidget->changeTab( w, text );
     mWidget->changeTab( w, Qt::green );
  }
}

void Test::mouseMiddleClick(QWidget *w)
{
  if (mList.size()==1) return;

  IntList::iterator it = mList.at( mWidget->indexOf(w) );
  mList.erase( it );
  mWidget->removePage( w );
}

void Test::movedTab(int from, int to, int newId)
{
  IntList::iterator it;

  it = mList.at( from );
  mList.erase( it );

  it = mList.at( to );
  mList.insert( it, newId );
}

void Test::timerDone()
{
  int j = ( rand() % mList.size() );
  QString one = QString(" ... %1").arg( j * (1 + rand() % 25) + (1 + rand() % 25) );
  qDebug("Changing %d to %s\n", j, one.latin1() );

  if ( j%2 ) {
    mWidget->changeTab( mList[j], new QLabel("Testlabel"+one, mWidget) );
    //mWidget->setRightButton( false );
  }
  else {
    mWidget->changeTab( mList[j], new QPushButton("Testbutton"+one, mWidget) );
    //mWidget->setRightButton( true );
  }

  QColor c = (mRed) ? Qt::red : Qt::blue;
  mWidget->changeTab( mList[j], c );
  mRed = !mRed;
}

#include "tab.moc"
