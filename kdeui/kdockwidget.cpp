/* This file is part of the KDE libraries
   Copyright (C) 2000 Max Judin <novaprint@mtu-net.ru>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/
#include "kdockwidget.h"
#include "kdockwidget_private.h"
#include <kdocktabctl.h>

#include <qlayout.h>
#include <qpainter.h>
#include <qobjcoll.h>

#ifndef NO_KDE2
 #include <kapp.h>
 #include <kconfig.h>
 #include <ktoolbar.h>
 #include <kpopupmenu.h>
#else
 #include <qapp.h>
 #include <qtoolbar.h>
 #include <qpopupmenu.h>
#endif

#define DOCK_CONFIG_VERSION "0.0.5"

static const char*close_xpm[]={
"5 5 2 1",
"# c black",
". c None",
"#...#",
".#.#.",
"..#..",
".#.#.",
"#...#"};

static const char*dockback_xpm[]={
"5 5 2 1",
"# c black",
". c None",
"#....",
".#...",
"..#.#",
"...##",
"..###"};

static const char*not_close_xpm[]={
"5 5 2 1",
"# c black",
". c None",
"#####",
"#...#",
"#...#",
"#...#",
"#####"};

/**
 * A special kind of KMainWindow that is able to have dockwidget child widgets.
 *
 * The main widget should be a dockwidget where other dockwidgets can be docked to
 * the left, right, top, bottom or to the middle.
 * Furthermore, the KDockMainWindow has got the KDocManager and some data about the dock states.
 *
 * @author Max Judin.
 * @version $Id$
*/
KDockMainWindow::KDockMainWindow( QWidget* parent, const char *name, WFlags f)
:KMainWindow( parent, name, f )
{
  QString new_name = QString(name) + QString("_DockManager");
  dockManager = new KDockManager( this, new_name.latin1() );
  mainDockWidget = 0L;
}

KDockMainWindow::~KDockMainWindow()
{
}

void KDockMainWindow::setMainDockWidget( KDockWidget* mdw )
{
  if ( mainDockWidget == mdw ) return;
  mainDockWidget = mdw;
}

void KDockMainWindow::setView( QWidget *view )
{
  if ( view->isA("KDockWidget") ){
    if ( view->parent() != this ) ((KDockWidget*)view)->applyToWidget( this );
  }

#ifndef NO_KDE2
  KMainWindow::setCentralWidget(view);
#else
  QMainWindow::setCentralWidget(view);
#endif
}

KDockWidget* KDockMainWindow::createDockWidget( const QString& name, const QPixmap &pixmap, QWidget* parent, const QString& strCaption, const QString& strTabPageLabel)
{
  return new KDockWidget( dockManager, name.latin1(), pixmap, parent, strCaption, strTabPageLabel );
}

void KDockMainWindow::makeDockVisible( KDockWidget* dock )
{
  if ( dock != 0L)
    dock->makeDockVisible();
}

void KDockMainWindow::makeDockInvisible( KDockWidget* dock )
{
  if ( dock != 0L)
    dock->undock();
}

void KDockMainWindow::makeWidgetDockVisible( QWidget* widget )
{
  makeDockVisible( dockManager->findWidgetParentDock(widget) );
}

#ifndef NO_KDE2
void KDockMainWindow::writeDockConfig( KConfig* c, QString group )
{
  dockManager->writeConfig( c, group );
}

void KDockMainWindow::readDockConfig( KConfig* c, QString group )
{
  dockManager->readConfig( c, group );
}
#endif

/*************************************************************************/
KDockWidgetAbstractHeaderDrag::KDockWidgetAbstractHeaderDrag( KDockWidgetAbstractHeader* parent, KDockWidget* dock, const char* name )
:QFrame( parent, name )
{
  dw = dock;
  installEventFilter( dock->dockManager() );
}
/*************************************************************************/
KDockWidgetHeaderDrag::KDockWidgetHeaderDrag( KDockWidgetAbstractHeader* parent, KDockWidget* dock, const char* name )
:KDockWidgetAbstractHeaderDrag( parent, dock, name )
{
}

void KDockWidgetHeaderDrag::paintEvent( QPaintEvent* )
{
  QPixmap drawBuffer( width(), height() );
  QPainter paint;

  paint.begin( &drawBuffer );
  paint.fillRect( drawBuffer.rect(), QBrush(colorGroup().brush(QColorGroup::Background)) );

  paint.setPen( colorGroup().light() );
  paint.drawLine( 1, 3, 1, 2 );
  paint.drawLine( 1, 2, width(), 2 );

  paint.setPen( colorGroup().mid() );
  paint.drawLine( 1, 4, width(), 4 );
  paint.drawLine( width(), 4, width(), 3 );

  paint.setPen( colorGroup().light() );
  paint.drawLine( 1, 6, 1, 5 );
  paint.drawLine( 1, 5, width(), 5 );

  paint.setPen( colorGroup().mid() );
  paint.drawLine( 1, 7, width(), 7 );
  paint.drawLine( width(), 7, width(), 6 );

  bitBlt( this,0,0,&drawBuffer,0,0,width(),height() );
  paint.end();
}
/*************************************************************************/
KDockWidgetAbstractHeader::KDockWidgetAbstractHeader( KDockWidget* parent, const char* name )
:QFrame( parent, name )
{
}
/*************************************************************************/
KDockWidgetHeader::KDockWidgetHeader( KDockWidget* parent, const char* name )
:KDockWidgetAbstractHeader( parent, name )
{
  layout = new QHBoxLayout( this );
  layout->setResizeMode( QLayout::Minimum );

  drag = new KDockWidgetHeaderDrag( this, parent );

  closeButton = new KDockButton_Private( this, "DockCloseButton" );
  closeButton->setPixmap(close_xpm);
  closeButton->setFixedSize(9,9);
  connect( closeButton, SIGNAL(clicked()), parent, SIGNAL(headerCloseButtonClicked()));
  connect( closeButton, SIGNAL(clicked()), parent, SLOT(undock()));

  stayButton = new KDockButton_Private( this, "DockStayButton" );
  stayButton->setToggleButton( true );
  stayButton->setPixmap(not_close_xpm);
  stayButton->setFixedSize(9,9);
  connect( stayButton, SIGNAL(clicked()), this, SLOT(slotStayClicked()));

  dockbackButton = new KDockButton_Private( this, "DockbackButton" );
  dockbackButton->setPixmap(dockback_xpm);
  dockbackButton->setFixedSize(9,9);
  connect( dockbackButton, SIGNAL(clicked()), parent, SIGNAL(headerDockbackButtonClicked()));
  connect( dockbackButton, SIGNAL(clicked()), parent, SLOT(dockBack()));

  layout->addWidget( drag );
  layout->addWidget( dockbackButton );
  layout->addWidget( stayButton );
  layout->addWidget( closeButton );
  layout->activate();
  drag->setFixedHeight( layout->minimumSize().height() );
}

void KDockWidgetHeader::setTopLevel( bool isTopLevel )
{
  if ( isTopLevel ){
    KDockWidget* par = (KDockWidget*)parent();
    if( par) {
      if( par->isDockBackPossible())
        dockbackButton->show();
      else
        dockbackButton->hide();
    }
    stayButton->hide();
    closeButton->hide();
    drag->setEnabled( true );
  } else {
    dockbackButton->hide();
    stayButton->show();
    closeButton->show();
  }
  layout->activate();
  updateGeometry();
}

void KDockWidgetHeader::slotStayClicked()
{
  closeButton->setEnabled( !stayButton->isOn() );
  drag->setEnabled( !stayButton->isOn() );
}

#ifndef NO_KDE2
void KDockWidgetHeader::saveConfig( KConfig* c )
{
  c->writeEntry( QString("%1%2").arg(parent()->name()).arg(":stayButton"), stayButton->isOn() );
}

void KDockWidgetHeader::loadConfig( KConfig* c )
{
  stayButton->setOn( c->readBoolEntry( QString("%1%2").arg(parent()->name()).arg(":stayButton"), false ) );
  slotStayClicked();
}
#endif

/*************************************************************************/
KDockWidget::KDockWidget( KDockManager* dockManager, const char* name, const QPixmap &pixmap, QWidget* parent, const QString& strCaption, const QString& strTabPageLabel, WFlags f)
: QWidget( parent, name, f )
	,formerBrotherDockWidget(0L)
  ,currentDockPos(DockNone)
  ,formerDockPos(DockNone)
  ,prevSideDockPosBeforeDrag(DockNone)
{
  d = new KDockWidgetPrivate();  // create private data

  layout = new QVBoxLayout( this );
  layout->setResizeMode( QLayout::Minimum );

  manager = dockManager;
  manager->childDock->append( this );
  installEventFilter( manager );

  header = 0L;
  setHeader( new KDockWidgetHeader( this, "AutoCreatedDockHeader" ) );
  if( strCaption == 0L)
    setCaption( name );
  else
    setCaption( strCaption);

  if( strTabPageLabel == " ")
    setTabPageLabel( caption());
  else
    setTabPageLabel( strTabPageLabel);

  eDocking = DockFullDocking;
  sDocking = DockFullSite;

  isGroup = false;
  isTabGroup = false;

  pix = new QPixmap(pixmap);
  widget = 0L;

  applyToWidget( parent, QPoint(0,0) );
}

KDockWidget::~KDockWidget()
{
  if ( !manager->undockProcess ){
    undock();
  }
  delete pix;
  emit iMBeingClosed();
  manager->childDock->remove( this );
  delete d; // destroy private data
}

void KDockWidget::setHeader( KDockWidgetAbstractHeader* h )
{
  if ( !h ) return;

  if ( header ){
    delete header;
    delete layout;
    header = h;
    layout = new QVBoxLayout( this );
    layout->setResizeMode( QLayout::Minimum );
    layout->addWidget( header );
     setWidget( widget );    
  } else {
    header = h;
    layout->addWidget( header );
  }
}

void KDockWidget::setEnableDocking( int pos )
{
  eDocking = pos;
  updateHeader();
}

void KDockWidget::updateHeader()
{
  if ( parent() ){
    if ( (parent() == manager->main) || isGroup || (eDocking == KDockWidget::DockNone) ){
      header->hide();
    } else {
      header->setTopLevel( false );
      header->show();
    }
  } else {
    header->setTopLevel( true );
    header->show();
  }
}

void KDockWidget::applyToWidget( QWidget* s, const QPoint& p )
{
  if ( parent() != s ){
    hide();
    reparent(s, 0, QPoint(0,0), false);
  }

  if ( s && s->inherits("KDockMainWindow") ){
    ((KDockMainWindow*)s)->setView( this );
  }

  if ( s == manager->main ){
      setGeometry( QRect(QPoint(0,0), manager->main->geometry().size()) );
  }

  if ( !s ){
    move(p);
  }

  updateHeader();
}

void KDockWidget::show()
{
  if ( parent() || manager->main->isVisible() )
    if ( !parent() ){
     emit manager->setDockDefaultPos( this );
     emit setDockDefaultPos();
     if ( parent() ){
        makeDockVisible();
      } else {
        QWidget::show();
      }
    } else {
     QWidget::show();
    }
}

bool KDockWidget::event( QEvent *event )
{
  switch ( event->type() )
  {
    case QEvent::ChildRemoved:
      if ( widget == ((QChildEvent*)event)->child() ) widget = 0L;
      break;
    case QEvent::Show:
      if ( widget ) widget->show();
      emit manager->change();
      break;
    case QEvent::Hide:
      if ( widget ) widget->hide();
      emit manager->change();
      break;
    case QEvent::CaptionChange:
      if ( parentWidget() ){
        if ( parent()->inherits("KDockSplitter") ){
          ((KDockSplitter*)(parent()))->updateName();
        }
        if ( parentTabGroup() ){
          setDockTabName( parentTabGroup() );
          parentTabGroup()->setPageCaption( this, tabPageLabel() );
        }
      }
      break;
		case QEvent::Close:
			emit iMBeingClosed();
			break;
    default:
      break;
  }
  return QWidget::event( event );
}

KDockWidget* KDockWidget::manualDock( KDockWidget* target, DockPosition dockPos, int spliPos, QPoint pos, bool check, int tabIndex )
{
  bool succes = true; // tested flag

  // check allowed this dock submit this operations
  if ( !(eDocking & (int)dockPos) ){
    succes = false;
  }

  // check allowed target submit this operations
  if ( target && !(target->sDocking & (int)dockPos) ){
    succes = false;
  }

  if ( parent() && !parent()->inherits("KDockSplitter") && !parentTabGroup() ){
    succes = false;
  }

  if ( !succes ){
    // try to make another manualDock
    KDockWidget* dock_result = 0L;
    if ( target && !check ){
      KDockWidget::DockPosition another__dockPos = KDockWidget::DockNone;
      switch ( dockPos ){
        case KDockWidget::DockLeft  : another__dockPos = KDockWidget::DockRight ; break;
        case KDockWidget::DockRight : another__dockPos = KDockWidget::DockLeft  ; break;
        case KDockWidget::DockTop   : another__dockPos = KDockWidget::DockBottom; break;
        case KDockWidget::DockBottom: another__dockPos = KDockWidget::DockTop   ; break;
        default: break;
      }
      dock_result = target->manualDock( this, another__dockPos, spliPos, pos, true, tabIndex );
    }
    return dock_result;
  }
  // end check block

  undock();

  if ( !target ){
    move( pos );
    show();
    emit manager->change();
    return this;
  }

  KDockTabGroup* parentTab = target->parentTabGroup();
  if ( parentTab ){
    // add to existing TabGroup
    applyToWidget( parentTab );
    parentTab->insertPage( this, tabPageLabel(), -1, tabIndex );
    parentTab->setPixmap( this, *pix );
    setDockTabName( parentTab );
    if( !toolTipStr.isEmpty())
      parentTab->setToolTip( this, toolTipStr);

    currentDockPos = KDockWidget::DockCenter;
    emit manager->change();
    return (KDockWidget*)parentTab->parent();
  }

  // create a new dockwidget that will contain the target and this
  QWidget* parentDock = target->parentWidget();
  KDockWidget* newDock = new KDockWidget( manager, "tempName", QPixmap(""), parentDock );
  newDock->currentDockPos = target->currentDockPos;

  if ( dockPos == KDockWidget::DockCenter ){
    newDock->isTabGroup = true;
  } else {
    newDock->isGroup = true;
  }
  newDock->eDocking = (target->eDocking & eDocking) & (~(int)KDockWidget::DockCenter);

  newDock->applyToWidget( parentDock );

  if ( !parentDock ){
    // dock to a toplevel dockwidget means newDock is toplevel now
    newDock->move( target->frameGeometry().topLeft() );
    newDock->resize( target->geometry().size() );
    if ( target->isVisibleToTLW() ) newDock->show();
  }

  // redirect the dockback button to the new dockwidget
  if( target->formerBrotherDockWidget != 0L) {
    newDock->formerBrotherDockWidget = target->formerBrotherDockWidget;
    if( formerBrotherDockWidget != 0L)
      QObject::connect( newDock->formerBrotherDockWidget, SIGNAL(iMBeingClosed()),
                        newDock, SLOT(loseFormerBrotherDockWidget()) );
      target->loseFormerBrotherDockWidget();
    }
  newDock->formerDockPos = target->formerDockPos;

  if ( dockPos == KDockWidget::DockCenter )
  {
    KDockTabGroup* tab = new KDockTabGroup( newDock, "_dock_tab");
    newDock->setWidget( tab );

    target->applyToWidget( tab );
    applyToWidget( tab );


    tab->insertPage( target, target->tabPageLabel() );
    tab->setPixmap( target, *(target->pix) );
    if( !target->toolTipString().isEmpty())
      tab->setToolTip( target, target->toolTipString());

    tab->insertPage( this, tabPageLabel(), -1, tabIndex );
    tab->setPixmap( this, *pix );
    if( !toolTipString().isEmpty())
      tab->setToolTip( this, toolTipString());

    setDockTabName( tab );
    tab->show();

    currentDockPos = DockCenter;
    target->formerDockPos = target->currentDockPos;
    target->currentDockPos = DockCenter;
  }
  else {
    // if to dock not to the center of the target dockwidget,
    // dock to newDock
    KDockSplitter* panner = 0L;
    if ( dockPos == KDockWidget::DockTop  || dockPos == KDockWidget::DockBottom ) panner = new KDockSplitter( newDock, "_dock_split_", Horizontal, spliPos );
    if ( dockPos == KDockWidget::DockLeft || dockPos == KDockWidget::DockRight  ) panner = new KDockSplitter( newDock, "_dock_split_", Vertical , spliPos );
    newDock->setWidget( panner );

    panner->setFocusPolicy( NoFocus );
    target->applyToWidget( panner );
    applyToWidget( panner );
    target->formerDockPos = target->currentDockPos;
    if ( dockPos == KDockWidget::DockRight) {
      panner->activate( target, this );
      currentDockPos = KDockWidget::DockRight;
      target->currentDockPos = KDockWidget::DockLeft;
    }
    else if( dockPos == KDockWidget::DockBottom) {
      panner->activate( target, this );
      currentDockPos = KDockWidget::DockBottom;
      target->currentDockPos = KDockWidget::DockTop;
    }
    else if( dockPos == KDockWidget::DockTop) {
      panner->activate( this, target );
      currentDockPos = KDockWidget::DockTop;
      target->currentDockPos = KDockWidget::DockBottom;
    }
    else if( dockPos == KDockWidget::DockLeft) {
      panner->activate( this, target );
      currentDockPos = KDockWidget::DockLeft;
      target->currentDockPos = KDockWidget::DockRight;
    }
    target->show();
    show();
    panner->show();
  }

  if ( parentDock ){
    if ( parentDock->inherits("KDockSplitter") ){
      KDockSplitter* sp = (KDockSplitter*)parentDock;
      sp->deactivate();
      if ( sp->getFirst() == target )
        sp->activate( newDock, 0L );
      else
        sp->activate( 0L, newDock );
    }
  }

  newDock->show();
  emit target->docking( this, dockPos );
  emit manager->replaceDock( target, newDock );
  emit manager->change();

  return newDock;
}

KDockTabGroup* KDockWidget::parentTabGroup()
{
  if ( !parent() ) return 0L;
  QWidget* candidate = parentWidget()->parentWidget();
  if ( candidate && candidate->inherits("KDockTabGroup") ) return (KDockTabGroup*)candidate;
  return 0L;
}

void KDockWidget::undock()
{
  QWidget* parentW = parentWidget();
  if ( !parentW ){
    hide();
    return;
  }

  formerDockPos = currentDockPos;
  currentDockPos = KDockWidget::DockDesktop;

  manager->blockSignals(true);
  manager->undockProcess = true;

  bool isV = parentW->isVisibleToTLW();

  KDockTabGroup* parentTab = parentTabGroup();
  if ( parentTab ){
    d->index = parentTab->index( this); // memorize the page position in the tab widget
    parentTab->removePage( this );
    formerBrotherDockWidget = (KDockWidget*)parentTab->getFirstPage();
    QObject::connect( formerBrotherDockWidget, SIGNAL(iMBeingClosed()),
                      this, SLOT(loseFormerBrotherDockWidget()) );
    applyToWidget( 0L );
    if ( parentTab->pageCount() == 1 ){

      // last subdock widget in the tab control
      KDockWidget* lastTab = (KDockWidget*)parentTab->getFirstPage();
      parentTab->removePage( lastTab );
      lastTab->applyToWidget( 0L );
      lastTab->move( parentTab->mapToGlobal(parentTab->frameGeometry().topLeft()) );

      // KDockTabGroup always have a parent that is a KDockWidget
      KDockWidget* parentOfTab = (KDockWidget*)parentTab->parent();
      delete parentTab; // KDockTabGroup

      QWidget* parentOfDockWidget = parentOfTab->parentWidget();
      if ( parentOfDockWidget == 0L ){
          if ( isV ) lastTab->show();
      } else {
        if ( parentOfDockWidget->inherits("KDockSplitter") ){
          KDockSplitter* split = (KDockSplitter*)parentOfDockWidget;
          lastTab->applyToWidget( split );
          split->deactivate();
          if ( split->getFirst() == parentOfTab ){
            split->activate( lastTab );
            if ( ((KDockWidget*)split->parent())->splitterOrientation == Vertical )
              emit ((KDockWidget*)split->getAnother(parentOfTab))->docking( parentOfTab, KDockWidget::DockLeft );
            else
              emit ((KDockWidget*)split->getAnother(parentOfTab))->docking( parentOfTab, KDockWidget::DockTop );
          } else {
            split->activate( 0L, lastTab );
            if ( ((KDockWidget*)split->parent())->splitterOrientation == Vertical )
              emit ((KDockWidget*)split->getAnother(parentOfTab))->docking( parentOfTab, KDockWidget::DockRight );
            else
              emit ((KDockWidget*)split->getAnother(parentOfTab))->docking( parentOfTab, KDockWidget::DockBottom );
          }
          split->show();
        } else {
          lastTab->applyToWidget( parentOfDockWidget );
        }
        lastTab->show();
      }
      manager->blockSignals(false);
      emit manager->replaceDock( parentOfTab, lastTab );
      lastTab->currentDockPos = parentOfTab->currentDockPos;
			emit parentOfTab->iMBeingClosed();
      manager->blockSignals(true);
      delete parentOfTab;

    } else {
      setDockTabName( parentTab );
    }
  } else {
/*********************************************************************************************/
    if ( parentW->inherits("KDockSplitter") ){
      KDockSplitter* parentSplitterOfDockWidget = (KDockSplitter*)parentW;
      d->splitPosInPercent = parentSplitterOfDockWidget->separatorPos();

      KDockWidget* secondWidget = (KDockWidget*)parentSplitterOfDockWidget->getAnother( this );
      KDockWidget* group        = (KDockWidget*)parentSplitterOfDockWidget->parentWidget();
      formerBrotherDockWidget = secondWidget;
      applyToWidget( 0L );
      group->hide();

      if( formerBrotherDockWidget != 0L)
        QObject::connect( formerBrotherDockWidget, SIGNAL(iMBeingClosed()),
                          this, SLOT(loseFormerBrotherDockWidget()) );

      if ( !group->parentWidget() ){
        secondWidget->applyToWidget( 0L, group->frameGeometry().topLeft() );
        secondWidget->resize( group->width(), group->height() );
      } else {
        QWidget* obj = group->parentWidget();
        secondWidget->applyToWidget( obj );
        if ( obj->inherits("KDockSplitter") ){
          KDockSplitter* parentOfGroup = (KDockSplitter*)obj;
          parentOfGroup->deactivate();

          if ( parentOfGroup->getFirst() == group )
            parentOfGroup->activate( secondWidget );
          else
            parentOfGroup->activate( 0L, secondWidget );
        }
      }
      secondWidget->currentDockPos = group->currentDockPos;
      secondWidget->formerDockPos  = group->formerDockPos;
      delete parentSplitterOfDockWidget;
      manager->blockSignals(false);
      emit manager->replaceDock( group, secondWidget );
			emit group->iMBeingClosed();
      manager->blockSignals(true);
      delete group;

      if ( isV ) secondWidget->show();
    } else {
      applyToWidget( 0L );
    }
/*********************************************************************************************/
  }
  manager->blockSignals(false);
  emit manager->change();
  manager->undockProcess = false;
}

void KDockWidget::setWidget( QWidget* mw )
{
  if ( !mw ) return;

  if ( mw->parent() != this ){
    mw->reparent(this, 0, QPoint(0,0), false);
  }

  widget = mw;
  delete layout;

  layout = new QVBoxLayout( this );
  layout->setResizeMode( QLayout::Minimum );

  layout->addWidget( header );
  layout->addWidget( widget,1 );
}

void KDockWidget::setDockTabName( KDockTabGroup* tab )
{
  QString listOfName;
  QString listOfCaption;
  for ( QWidget* w = tab->getFirstPage(); w; w = tab->getNextPage( w ) ){
    listOfCaption.append( w->caption() ).append(",");
    listOfName.append( w->name() ).append(",");
  }
  listOfCaption.remove( listOfCaption.length()-1, 1 );
  listOfName.remove( listOfName.length()-1, 1 );

  tab->parentWidget()->setName( listOfName.utf8() );
  tab->parentWidget()->setCaption( listOfCaption );

  tab->parentWidget()->repaint( false ); // KDockWidget->repaint
  if ( tab->parentWidget()->parent() )
    if ( tab->parentWidget()->parent()->inherits("KDockSplitter") )
      ((KDockSplitter*)(tab->parentWidget()->parent()))->updateName();
}

bool KDockWidget::mayBeHide()
{
  bool f = (parent() != manager->main);
  return ( !isGroup && !isTabGroup && f && isVisible() && ( eDocking != (int)KDockWidget::DockNone ) );
}

bool KDockWidget::mayBeShow()
{
  bool f = (parent() != manager->main);
  return ( !isGroup && !isTabGroup && f && !isVisible() );
}

void KDockWidget::changeHideShowState()
{
  if ( mayBeHide() ){
    undock();
    return;
  }

  if ( mayBeShow() ){
    if ( manager->main->inherits("KDockMainWindow") ){
      ((KDockMainWindow*)manager->main)->makeDockVisible(this);
    } else {
      makeDockVisible();
    }
  }
}

void KDockWidget::makeDockVisible()
{
  if ( isVisible() ) return;

  if ( parentTabGroup() ){
    parentTabGroup()->setVisiblePage( this );
  }
  QWidget* p = parentWidget();
  while ( p ){
    if ( !p->isVisible() ) p->show();
    p = p->parentWidget();
  }
  if( parent() == 0L) // is undocked
    dockBack();
  show();
}

void KDockWidget::loseFormerBrotherDockWidget()
{
  if( formerBrotherDockWidget != 0L)
  	QObject::disconnect( formerBrotherDockWidget, SIGNAL(iMBeingClosed()),
	  									   this, SLOT(loseFormerBrotherDockWidget()) );
	formerBrotherDockWidget = 0L;
	repaint();
}

void KDockWidget::dockBack()
{
  if( formerBrotherDockWidget) {
		// search all children if it tries to dock back to a child
		bool found = false;
		QObjectList* cl = queryList("KDockWidget");
   	QObjectListIt it( *cl );
	  QObject * obj;
   	while ( (obj=it.current()) != 0 ) {
      ++it;
      QWidget* widg = (QWidget*)obj;
      if( widg == formerBrotherDockWidget)
				found = true;
   	}
   	delete cl;

		if( !found) {
			// can dock back to the former brother dockwidget
			manualDock( formerBrotherDockWidget, formerDockPos, d->splitPosInPercent, QPoint(0,0), false, d->index);
			formerBrotherDockWidget = 0L;
			makeDockVisible();
			return;
		}
	}

	// else dockback to the dockmainwindow (default behaviour)
  manualDock( ((KDockMainWindow*)manager->main)->getMainDockWidget(), formerDockPos, d->splitPosInPercent, QPoint(0,0), false, d->index);
  formerBrotherDockWidget = 0L;
  if (parent())
    makeDockVisible();
}

bool KDockWidget::isDockBackPossible()
{
  if( formerBrotherDockWidget == 0L)
    return false;
  else
    return true;
}

/**************************************************************************************/

KDockManager::KDockManager( QWidget* mainWindow , const char* name )
:QObject( 0, name )
{
  main = mainWindow;
  main->installEventFilter( this );

  undockProcess = false;

  menuData = new QList<MenuDockData>;
  menuData->setAutoDelete( true );
  menuData->setAutoDelete( true );

#ifndef NO_KDE2
  menu = new KPopupMenu();
#else
  menu = new QPopupMenu();
#endif

  connect( menu, SIGNAL(aboutToShow()), SLOT(slotMenuPopup()) );
  connect( menu, SIGNAL(activated(int)), SLOT(slotMenuActivated(int)) );

  childDock = new QObjectList();
  childDock->setAutoDelete( false );
  mg = 0L;
  draging = false;
  dropCancel = false;
}

KDockManager::~KDockManager()
{
  delete menuData;
  delete menu;

  QObjectListIt it( *childDock );
  KDockWidget * obj;

  while ( (obj=(KDockWidget*)it.current()) ) {
    delete obj;
  }
  delete childDock;
}

void KDockManager::activate()
{
  QObjectListIt it( *childDock );
  KDockWidget * obj;

  while ( (obj=(KDockWidget*)it.current()) ) {
    ++it;
    if ( obj->widget ) obj->widget->show();
    if ( !obj->parentTabGroup() ){
        obj->show();
    }
  }
  if ( !main->inherits("QDialog") ) main->show();
}

bool KDockManager::eventFilter( QObject *obj, QEvent *event )
{
  if ( obj == main && event->type() == QEvent::Resize && main->children() ){
    QWidget* fc = (QWidget*)main->children()->getFirst();
    if ( fc )
      fc->setGeometry( QRect(QPoint(0,0), main->geometry().size()) );
  }

  #ifdef KeyPress
  #undef KeyPress
  #endif  
  if ( event->type() ==QEvent::KeyPress ){
    if ( ((QKeyEvent*)event)->key() == Qt::Key_Escape ){
      if ( draging ){
        dropCancel = true;
        draging = false;
        drop();
      }
    }
  }

  if ( obj->inherits("KDockWidgetAbstractHeaderDrag") ){
    KDockWidget* ww = 0L;
    KDockWidget* curdw = ((KDockWidgetAbstractHeaderDrag*)obj)->dockWidget();
    switch ( event->type() ){
      case QEvent::MouseButtonRelease:
        if ( ((QMouseEvent*)event)->button() == LeftButton ){
          if ( draging && !dropCancel ){
            drop();
          }
          draging = false;
          dropCancel = false;
        }
        break;
      case QEvent::MouseMove:
        if ( draging ) {
          ww = findDockWidgetAt( QCursor::pos() );
          KDockWidget* oldMoveWidget = currentMoveWidget;
          if ( currentMoveWidget  && ww == currentMoveWidget ) { //move
            dragMove( currentMoveWidget, currentMoveWidget->mapFromGlobal( QCursor::pos() ) );
            break;
          }

          if ( !ww && (curdw->eDocking & (int)KDockWidget::DockDesktop) == 0 ){
              currentMoveWidget = ww;
              curPos = KDockWidget::DockDesktop;
              mg->movePause();
          } else {
            if ( oldMoveWidget && ww != currentMoveWidget ) { //leave
              currentMoveWidget = ww;
              curPos = KDockWidget::DockDesktop;
              mg->resize( storeW, storeH );
              mg->moveContinue();
            }
          }

          if ( oldMoveWidget != ww && ww ) { //enter ww
            currentMoveWidget = ww;
            curPos = KDockWidget::DockDesktop;
            storeW = mg->width();
            storeH = mg->height();
            mg->movePause();
          }
        } else {
          if ( (((QMouseEvent*)event)->state() == LeftButton) &&  !dropCancel ){
            if ( curdw->eDocking != (int)KDockWidget::DockNone ){
              dropCancel = false;
              curdw->setFocus();
              qApp->processOneEvent();
              startDrag( curdw );
            }
          }
        }
        break;
      default:
        break;
    }
  }
  return QObject::eventFilter( obj, event );
}

KDockWidget* KDockManager::findDockWidgetAt( const QPoint& pos )
{
  if (currentDragWidget->eDocking == (int)KDockWidget::DockNone ) return 0L;

  QWidget* p = QApplication::widgetAt( pos );
  if ( !p ) return 0L;
  QWidget* w = 0L;
  findChildDockWidget( w, p, p->mapFromParent(pos) );
  if ( !w ){
    if ( !p->inherits("KDockWidget") ) return 0L;
    w = p;
  }
  if ( qt_find_obj_child( w, "KDockSplitter", "_dock_split_" ) ) return 0L;
  if ( qt_find_obj_child( w, "KDockTabGroup", "_dock_tab" ) ) return 0L;
  if ( childDockWidgetList->find(w) != -1 ) return 0L;
  if ( currentDragWidget->isGroup && ((KDockWidget*)w)->parentTabGroup() ) return 0L;

  KDockWidget* www = (KDockWidget*)w;
  if ( www->sDocking == (int)KDockWidget::DockNone ) return 0L;

  KDockWidget::DockPosition curPos = KDockWidget::DockDesktop;
  QPoint cpos  = www->mapFromGlobal( pos );

  int ww = www->widget->width() / 3;
  int hh = www->widget->height() / 3;

	if ( cpos.y() <= hh ){
    curPos = KDockWidget::DockTop;
	} else
    if ( cpos.y() >= 2*hh ){
      curPos = KDockWidget::DockBottom;
    } else
      if ( cpos.x() <= ww ){
        curPos = KDockWidget::DockLeft;
      } else
        if ( cpos.x() >= 2*ww ){
          curPos = KDockWidget::DockRight;
        } else
            curPos = KDockWidget::DockCenter;

  if ( !(www->sDocking & (int)curPos) ) return 0L;
  if ( !(currentDragWidget->eDocking & (int)curPos) ) return 0L;
  if ( www->manager != this ) return 0L;

  return www;
}

void KDockManager::findChildDockWidget( QWidget*& ww, const QWidget* p, const QPoint& pos )
{
  if ( p->children() ) {
    QWidget *w;
    QObjectListIt it( *p->children() );
    it.toLast();
    while ( it.current() ) {
      if ( it.current()->isWidgetType() ) {
        w = (QWidget*)it.current();
        if ( w->isVisible() && w->geometry().contains(pos) ) {
          if ( w->inherits("KDockWidget") ) ww = w;
          findChildDockWidget( ww, w, w->mapFromParent(pos) );
          return;
        }
      }
      --it;
    }
  }
  return;
}

void KDockManager::findChildDockWidget( const QWidget* p, WidgetList*& list )
{
  if ( p->children() ) {
    QWidget *w;
    QObjectListIt it( *p->children() );
    it.toLast();
    while ( it.current() ) {
      if ( it.current()->isWidgetType() ) {
        w = (QWidget*)it.current();
        if ( w->isVisible() ) {
          if ( w->inherits("KDockWidget") ) list->append( w );
          findChildDockWidget( w, list );
        }
      }
      --it;
    }
  }
  return;
}

void KDockManager::startDrag( KDockWidget* w )
{
  if(( w->currentDockPos == KDockWidget::DockLeft) || ( w->currentDockPos == KDockWidget::DockRight)
   || ( w->currentDockPos == KDockWidget::DockTop) || ( w->currentDockPos == KDockWidget::DockBottom)) {
    w->prevSideDockPosBeforeDrag = w->currentDockPos;

    if ( w->parentWidget()->inherits("KDockSplitter") ){
      KDockSplitter* parentSplitterOfDockWidget = (KDockSplitter*)(w->parentWidget());
      w->d->splitPosInPercent = parentSplitterOfDockWidget->separatorPos();
    }
  }

  currentMoveWidget = 0L;
  currentDragWidget = w;
  childDockWidgetList = new WidgetList();
  childDockWidgetList->append( w );
  findChildDockWidget( w, childDockWidgetList );
  
  if ( mg ) delete mg;
  mg = new KDockMoveManager( w );
  curPos = KDockWidget::DockDesktop;
  draging = true;
  mg->doMove();
}

void KDockManager::dragMove( KDockWidget* dw, QPoint pos )
{
	QPoint p = dw->mapToGlobal( dw->widget->pos() );
  KDockWidget::DockPosition oldPos = curPos;

  QSize r = dw->widget->size();
  if ( dw->parentTabGroup() ){
    curPos = KDockWidget::DockCenter;
  	if ( oldPos != curPos ) mg->setGeometry( p.x()+2, p.y()+2, r.width()-4, r.height()-4 );
    return;
  }

  int w = r.width() / 3;
  int h = r.height() / 3;

  if ( pos.y() <= h ){
    curPos = KDockWidget::DockTop;
    w = r.width();
  } else
    if ( pos.y() >= 2*h ){
      curPos = KDockWidget::DockBottom;
      p.setY( p.y() + 2*h );
      w = r.width();
    } else
      if ( pos.x() <= w ){
        curPos = KDockWidget::DockLeft;
        h = r.height();
      } else
        if ( pos.x() >= 2*w ){
          curPos = KDockWidget::DockRight;
          p.setX( p.x() + 2*w );
          h = r.height();
        } else
          {
            curPos = KDockWidget::DockCenter;
            p.setX( p.x() + w );
            p.setY( p.y() + h );
          }

  if ( oldPos != curPos ) mg->setGeometry( p.x(), p.y(), w, h );
}

void KDockManager::drop()
{
  mg->stop();
  delete childDockWidgetList;
  if ( dropCancel ) return;
  if ( !currentMoveWidget && ((currentDragWidget->eDocking & (int)KDockWidget::DockDesktop) == 0) ) return;

  if ( !currentMoveWidget && !currentDragWidget->parent() )
    currentDragWidget->move( mg->x(), mg->y() );
  else {
    int splitPos = currentDragWidget->d->splitPosInPercent;
    // do we have to calculate 100%-splitPosInPercent?
    if( (curPos != currentDragWidget->prevSideDockPosBeforeDrag) && (curPos != KDockWidget::DockCenter) && (curPos != KDockWidget::DockDesktop)) {
      switch( currentDragWidget->prevSideDockPosBeforeDrag) {
      case KDockWidget::DockLeft:   if(curPos != KDockWidget::DockTop)    splitPos = 100-splitPos; break;
      case KDockWidget::DockRight:  if(curPos != KDockWidget::DockBottom) splitPos = 100-splitPos; break;
      case KDockWidget::DockTop:    if(curPos != KDockWidget::DockLeft)   splitPos = 100-splitPos; break;
      case KDockWidget::DockBottom: if(curPos != KDockWidget::DockRight)  splitPos = 100-splitPos; break;
      default: break;
      }
    }
    currentDragWidget->manualDock( currentMoveWidget, curPos , splitPos, QPoint(mg->x(), mg->y()) );
    currentDragWidget->makeDockVisible();
  }
}

#ifndef NO_KDE2
void KDockManager::writeConfig( KConfig* c, QString group )
{
  //debug("BEGIN Write Config");
  if ( !c ) c = kapp->config();
  if ( group.isEmpty() ) group = "dock_setting_default";

  c->setGroup( group );
  c->writeEntry( "Version", DOCK_CONFIG_VERSION );

  QStrList nameList;
  QStrList findList;
  QObjectListIt it( *childDock );
  KDockWidget * obj;

  // collect KDockWidget's name
  QStrList nList;
  while ( (obj=(KDockWidget*)it.current()) ) {
    ++it;
    //debug("  +Add subdock %s", obj->name());
    nList.append( obj->name() );
    if ( obj->parent() == main )
      c->writeEntry( "Main:view", obj->name() );
  }

  nList.first();
  while ( nList.current() ){
    //debug("  -Try to save %s", nList.current());
    obj = getDockWidgetFromName( nList.current() );
    QString cname = obj->name();
    if ( obj->header ){
      obj->header->saveConfig( c );
    }
/*************************************************************************************************/
    if ( obj->isGroup ){
      if ( findList.find( obj->firstName.latin1() ) != -1 && findList.find( obj->lastName.latin1() ) != -1 ){

        c->writeEntry( cname+":type", "GROUP");
        if ( !obj->parent() ){
          c->writeEntry( cname+":parent", "___null___");
          c->writeEntry( cname+":geometry", QRect(obj->frameGeometry().topLeft(), obj->size()) );
          c->writeEntry( cname+":visible", obj->isVisible());
        } else {
          c->writeEntry( cname+":parent", "yes");
        }
        c->writeEntry( cname+":first_name", obj->firstName );
        c->writeEntry( cname+":last_name", obj->lastName );
        c->writeEntry( cname+":orientation", (int)obj->splitterOrientation );
        c->writeEntry( cname+":sepPos", ((KDockSplitter*)obj->widget)->separatorPos() );

        nameList.append( obj->name() );
        findList.append( obj->name() );
        //debug("  Save %s", nList.current());
        nList.remove();
        nList.first();
      } else {
/*************************************************************************************************/
        //debug("  Skip %s", nList.current());
        //if ( findList.find( obj->firstName ) == -1 )
        //  debug("  ? Not found %s", obj->firstName);
        //if ( findList.find( obj->lastName ) == -1 )
        //  debug("  ? Not found %s", obj->lastName);
        nList.next();
        if ( !nList.current() ) nList.first();
      }
    } else {
/*************************************************************************************************/
      if ( obj->isTabGroup){
        c->writeEntry( cname+":type", "TAB_GROUP");
        if ( !obj->parent() ){
          c->writeEntry( cname+":parent", "___null___");
          c->writeEntry( cname+":geometry", QRect(obj->frameGeometry().topLeft(), obj->size()) );
          c->writeEntry( cname+":visible", obj->isVisible());
        } else {
          c->writeEntry( cname+":parent", "yes");
        }
        QStrList list;
        for ( QWidget* w = ((KDockTabGroup*)obj->widget)->getFirstPage(); w; w = ((KDockTabGroup*)obj->widget)->getNextPage( w ) ){
          list.append( w->name() );
        }
        c->writeEntry( cname+":tabNames", list );
        c->writeEntry( cname+":curTab", ((KDockTabGroup*)obj->widget)->visiblePageId() );

        nameList.append( obj->name() );
        findList.append( obj->name() ); // not realy need !!!
        //debug("  Save %s", nList.current());
        nList.remove();
        nList.first();
      } else {
/*************************************************************************************************/
        if ( !obj->parent() ){
          c->writeEntry( cname+":type", "NULL_DOCK");
          c->writeEntry( cname+":geometry", QRect(obj->frameGeometry().topLeft(), obj->size()) );
          c->writeEntry( cname+":visible", obj->isVisible());
        } else {
          c->writeEntry( cname+":type", "DOCK");
        }
        nameList.append( cname.latin1() );
        //debug("  Save %s", nList.current());
        findList.append( obj->name() );
        nList.remove();
        nList.first();
      }
    }
  }
  c->writeEntry( "NameList", nameList );

  c->writeEntry( "Main:Geometry", QRect(main->frameGeometry().topLeft(), main->size()) );
  c->writeEntry( "Main:visible", main->isVisible()); // curently nou use

  if ( main->inherits("KDockMainWindow") ){
    KDockMainWindow* dmain = (KDockMainWindow*)main;
    // for KDockMainWindow->setView() in reafConfig()
    c->writeEntry( "Main:view", dmain->centralWidget() ? dmain->centralWidget()->name():"" );
    c->writeEntry( "Main:dock", dmain->getMainDockWidget()     ? dmain->getMainDockWidget()->name()    :"" );
  }

  c->sync();
  //debug("END Write Config");
}

void KDockManager::readConfig( KConfig* c, QString group )
{
  if ( !c ) c = kapp->config();
  if ( group.isEmpty() ) group = "dock_setting_default";

  c->setGroup( group );
  QStrList nameList;
  c->readListEntry( "NameList", nameList );
  QString ver = c->readEntry( "Version", "0.0.1" );
  nameList.first();
  if ( !nameList.current() || ver != DOCK_CONFIG_VERSION ){
    activate();
    return;
  }

  autoCreateDock = new QObjectList();
  autoCreateDock->setAutoDelete( true );

  bool isMainVisible = main->isVisible();
  main->hide();

  QObjectListIt it( *childDock );
  KDockWidget * obj;

  while ( (obj=(KDockWidget*)it.current()) ){
    ++it;
    if ( !obj->isGroup && !obj->isTabGroup )
    {
      if ( obj->parent() ) obj->undock(); else obj->hide();
    }
  }

  nameList.first();
  while ( nameList.current() ){
    QString oname = nameList.current();
    c->setGroup( group );
    QString type = c->readEntry( oname + ":type" );
    obj = 0L;

    if ( type == "GROUP" ){
      KDockWidget* first = getDockWidgetFromName( c->readEntry( oname + ":first_name" ) );
      KDockWidget* last  = getDockWidgetFromName( c->readEntry( oname + ":last_name"  ) );
      int sepPos = c->readNumEntry( oname + ":sepPos" );

      Orientation p = (Orientation)c->readNumEntry( oname + ":orientation" );
      if ( first  && last ){
        obj = first->manualDock( last, ( p == Vertical ) ? KDockWidget::DockLeft : KDockWidget::DockTop, sepPos );
        if (obj){
          obj->setName( oname.latin1() );
        }
      }
    }

    if ( type == "TAB_GROUP" ){
      QStrList list;
      KDockWidget* tabDockGroup = 0L;
      c->readListEntry( oname+":tabNames", list );
      KDockWidget* d1 = getDockWidgetFromName( list.first() );
      list.next();
      KDockWidget* d2 = getDockWidgetFromName( list.current() );
      tabDockGroup = d2->manualDock( d1, KDockWidget::DockCenter );
      if ( tabDockGroup ){
        KDockTabGroup* tab = (KDockTabGroup*)tabDockGroup->widget;
        list.next();
        while ( list.current() && tabDockGroup ){
          KDockWidget* tabDock = getDockWidgetFromName( list.current() );
          tabDockGroup = tabDock->manualDock( d1, KDockWidget::DockCenter );
          list.next();
        }
        if ( tabDockGroup ){
          tabDockGroup->setName( oname.latin1() );
  				c->setGroup( group );
          tab->setVisiblePage( c->readNumEntry( oname+":curTab" ) );
        }
      }
      obj = tabDockGroup;
    }

    if ( type == "NULL_DOCK" || c->readEntry( oname + ":parent") == "___null___" ){
      QRect r = c->readRectEntry( oname + ":geometry" );
      obj = getDockWidgetFromName( oname );
      obj->applyToWidget( 0L );
      obj->setGeometry(r);

  		c->setGroup( group );
      if ( c->readBoolEntry( oname + ":visible" ) ){
        obj->QWidget::show();
      }
    }

    if ( type == "DOCK"  ){
      obj = getDockWidgetFromName( oname );
    }

    if ( obj && obj->header){
      obj->header->loadConfig( c );
    }
    nameList.next();
	}

  if ( main->inherits("KDockMainWindow") ){
    KDockMainWindow* dmain = (KDockMainWindow*)main;

  	c->setGroup( group );
    QString mv = c->readEntry( "Main:view" );
    if ( !mv.isEmpty() && getDockWidgetFromName( mv ) ){
      KDockWidget* mvd  = getDockWidgetFromName( mv );
      mvd->applyToWidget( dmain );
      mvd->show();
      dmain->setView( mvd );
    }
  	c->setGroup( group );
    QString md = c->readEntry( "Main:dock" );
    if ( !md.isEmpty() && getDockWidgetFromName( md ) ){
      KDockWidget* mvd  = getDockWidgetFromName( md );
      dmain->setMainDockWidget( mvd );
    }
  } else {
  	c->setGroup( group );
    QString mv = c->readEntry( "Main:view" );
    if ( !mv.isEmpty() && getDockWidgetFromName( mv ) ){
      KDockWidget* mvd  = getDockWidgetFromName( mv );
      mvd->applyToWidget( main );
      mvd->show();
    }
  
  }
  // delete all autocreate dock
  delete autoCreateDock;
  autoCreateDock = 0L;

  c->setGroup( group );
  QRect mr = c->readRectEntry("Main:Geometry");
  main->setGeometry(mr);
  if ( isMainVisible ) main->show();
}
#endif

KDockWidget* KDockManager::getDockWidgetFromName( const QString& dockName )
{
  QObjectListIt it( *childDock );
  KDockWidget * obj;
  while ( (obj=(KDockWidget*)it.current()) ) {
    ++it;
		if ( QString(obj->name()) == dockName ) return obj;
  }

  KDockWidget* autoCreate = 0L;
  if ( autoCreateDock ){
    autoCreate = new KDockWidget( this, dockName.latin1(), QPixmap("") );
    autoCreateDock->append( autoCreate );
  }
	return autoCreate;
}

void KDockManager::slotMenuPopup()
{
  menu->clear();
  menuData->clear();

  QObjectListIt it( *childDock );
  KDockWidget * obj;
  int numerator = 0;
  while ( (obj=(KDockWidget*)it.current()) ) {
	  ++it;
    if ( obj->mayBeHide() )
    {
      menu->insertItem( *obj->pix, QString("Hide ") + obj->caption(), numerator++ );
      menuData->append( new MenuDockData( obj, true ) );
    }

    if ( obj->mayBeShow() )
    {
      menu->insertItem( *obj->pix, QString("Show ") + obj->caption(), numerator++ );
      menuData->append( new MenuDockData( obj, false ) );
    }
  }
}

void KDockManager::slotMenuActivated( int id )
{
  MenuDockData* data = menuData->at( id );
  data->dock->changeHideShowState();
}

KDockWidget* KDockManager::findWidgetParentDock( QWidget* w )
{
  QObjectListIt it( *childDock );
  KDockWidget * dock;
  KDockWidget * found = 0L;

  while ( (dock=(KDockWidget*)it.current()) ) {
	  ++it;
    if ( dock->widget == w ){ found  = dock; break; }
  }
  return found;
}

#ifndef NO_KDE2 // for Qt-only projects, because tmake doesn't take this name
#include "kdockwidget.moc"
#endif
