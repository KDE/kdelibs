#include <qcheckbox.h>
#include <qlayout.h>

#include "tab.h"

Test::Test( QWidget* parent, const char *name )
  :QVBox( parent, name ), mChange(0), mRed( false ), mLeftPopup( false ), mRightPopup( false ), mContextPopup( false )
{
  resize( 600,300 );

  mWidget = new KTabWidget( this );
  mWidget->setRightButton( true );
  mWidget->setLeftButton( true );
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
  connect( mWidget, SIGNAL( mouseDoubleClick( QWidget * )), this, SLOT(mouseDoubleClick( QWidget * )));
  connect( mWidget, SIGNAL( mouseMiddleClick( QWidget * )), this, SLOT(mouseMiddleClick( QWidget * )));

  QWidget * grid = new QWidget(this);
  QGridLayout * gridlayout = new QGridLayout( grid, 4, 2 );

  QPushButton * addTab = new QPushButton( "Add Tab", grid );
  gridlayout->addWidget( addTab, 0, 0 );
  connect( addTab, SIGNAL( clicked() ), this, SLOT( addTab() ) );
  connect( mWidget, SIGNAL( leftButtonClicked() ), this, SLOT( addTab() ) );

  QPushButton * removeTab = new QPushButton( "Remove Current Tab", grid );
  gridlayout->addWidget( removeTab, 0, 1 );
  connect( removeTab, SIGNAL( clicked() ), this, SLOT( removeCurrentTab() ) );
  connect( mWidget, SIGNAL( rightButtonClicked() ), this, SLOT( removeCurrentTab() ) );

  QCheckBox * leftButton = new QCheckBox( "Show left button", grid );
  gridlayout->addWidget( leftButton, 1, 0 );
  leftButton->setChecked(true);
  connect( leftButton, SIGNAL( toggled(bool) ), this, SLOT( toggleLeftButton(bool) ) );

  QCheckBox * leftPopup = new QCheckBox( "Enable left popup", grid );
  gridlayout->addWidget( leftPopup, 2, 0 );
  connect( leftPopup, SIGNAL( toggled(bool) ), this, SLOT( toggleLeftPopup(bool) ) );

  QCheckBox * rightButton = new QCheckBox( "Show right button", grid );
  gridlayout->addWidget( rightButton, 1, 1 );
  rightButton->setChecked(true);
  connect( rightButton, SIGNAL( toggled(bool) ), this, SLOT( toggleRightButton(bool) ) );

  QCheckBox * rightPopup = new QCheckBox( "Enable right popup", grid );
  gridlayout->addWidget( rightPopup, 2, 1 );
  connect( rightPopup, SIGNAL( toggled(bool) ), this, SLOT( toggleRightPopup(bool) ) );

  QCheckBox * tabsbottom = new QCheckBox( "Show tabs at bottom", grid );
  gridlayout->addWidget( tabsbottom, 3, 0 );
  connect( tabsbottom, SIGNAL( toggled(bool) ), this, SLOT( toggleTabPosition(bool) ) );

  QCheckBox * tabshape = new QCheckBox( "Triangular tab shape", grid );
  gridlayout->addWidget( tabshape, 3, 1 );
  connect( tabshape, SIGNAL( toggled(bool) ), this, SLOT( toggleTabShape(bool) ) );

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

void Test::contextMenu(QWidget *w, const QPoint &p)
{
  if (!mContextPopup) {
      mContextPopup = new QPopupMenu(this);
      mContextPopup->insertItem(SmallIcon( "konsole" ), "Set This Icon", 0);
      mContextPopup->insertItem(SmallIcon( "konqueror" ), "Set This Icon", 1);
      mContextPopup->insertItem(SmallIcon( "kicker" ), "Set This Icon", 2);
      connect(mContextPopup, SIGNAL(activated(int)), SLOT(contextMenuActivated(int)));
  }
  mContextWidget = w;
  mContextPopup->popup(p);
}

void Test::contextMenuActivated(int item)
{
  switch (item) {
    case 0: mWidget->changeTab( mContextWidget, SmallIcon( "konsole" ), "Konsole" );
            break;
    case 1: mWidget->changeTab( mContextWidget, SmallIcon( "konqueror" ), "Konqueror" );
            break;
    case 2: mWidget->changeTab( mContextWidget, SmallIcon( "kicker" ), "Kicker" );
            break;
  }
}

void Test::mouseDoubleClick(QWidget *w)
{
  mWidget->changeTab( w, Qt::green );
}

void Test::mouseMiddleClick(QWidget *w)
{
  if (mList.size()==1) return;

  IntList::iterator it = mList.at( mWidget->indexOf(w) );
  mList.erase( it );
  mWidget->removePage( w );
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
