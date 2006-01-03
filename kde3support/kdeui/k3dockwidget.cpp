
/* This file is part of the KDE libraries
   Copyright (C) 2000 Max Judin <novaprint@mtu-net.ru>
   Copyright (C) 2002,2003 Joseph Wenninger <jowenn@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#include "k3dockwidget.h"
#include "k3dockwidget_private.h"
#include "k3dockwidget_p.h"

#include <qapplication.h>
#include <qlayout.h>
#include <qpainter.h>
#include <q3strlist.h>
#include <qcursor.h>
#include <qwidget.h>
#include <qtabwidget.h>
#include <qtooltip.h>
#include <qstyle.h>
#include <QMouseEvent>
#include <QObject>

#ifndef NO_KDE2
#include <kconfig.h>
#include <kglobal.h>
#include <klocale.h>
#include <ktoolbar.h>
#include <k3popupmenu.h>
#include <kwin.h>
#include <kdebug.h>
#include <kglobalsettings.h>

#include "config.h"
#ifdef Q_WS_X11
#include <X11/X.h>
#include <X11/Xlib.h>
#include <qx11info_x11.h>
#endif
#else
#include <q3toolbar.h>
#include <q3popupmenu.h>
#endif

#include <stdlib.h>

#undef BORDERLESS_WINDOWS

#define DOCK_CONFIG_VERSION "0.0.5"

static const char* const dockback_xpm[]={
"6 6 2 1",
"# c black",
". c None",
"......",
".#....",
"..#..#",
"...#.#",
"....##",
"..####"};

static const char* const todesktop_xpm[]={
"5 5 2 1",
"# c black",
". c None",
"####.",
"##...",
"#.#..",
"#..#.",
"....#"};

static const char* const not_close_xpm[]={
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
 * Furthermore, the K3DockMainWindow has got the KDocManager and some data about the dock states.
 *
 * @author Max Judin.
*/
K3DockMainWindow::K3DockMainWindow( QWidget* parent, const char *name, Qt::WFlags f)
:KMainWindow( parent, name, f )
{
  QString new_name = QString(name) + QString("_DockManager");
  dockManager = new K3DockManager( this, new_name.latin1() );
  mainDockWidget = 0L;
}

K3DockMainWindow::~K3DockMainWindow()
{
	delete dockManager;
}

void K3DockMainWindow::setMainDockWidget( K3DockWidget* mdw )
{
  if ( mainDockWidget == mdw ) return;
  mainDockWidget = mdw;
  dockManager->setMainDockWidget2(mdw);
}

void K3DockMainWindow::setView( QWidget *view )
{
  if ( view->isA("K3DockWidget") ){
    if ( view->parent() != this ) ((K3DockWidget*)view)->applyToWidget( this );
  }

#ifndef NO_KDE2
  KMainWindow::setCentralWidget(view);
#else
  Q3MainWindow::setCentralWidget(view);
#endif
}

K3DockWidget* K3DockMainWindow::createDockWidget( const QString& name, const QPixmap &pixmap, QWidget* parent, const QString& strCaption, const QString& strTabPageLabel)
{
  return new K3DockWidget( dockManager, name.latin1(), pixmap, parent, strCaption, strTabPageLabel );
}

void K3DockMainWindow::makeDockVisible( K3DockWidget* dock )
{
  if ( dock )
    dock->makeDockVisible();
}

void K3DockMainWindow::makeDockInvisible( K3DockWidget* dock )
{
  if ( dock )
    dock->undock();
}

void K3DockMainWindow::makeWidgetDockVisible( QWidget* widget )
{
  makeDockVisible( dockManager->findWidgetParentDock(widget) );
}

void K3DockMainWindow::writeDockConfig(QDomElement &base)
{
  dockManager->writeConfig(base);
}

void K3DockMainWindow::readDockConfig(QDomElement &base)
{
  dockManager->readConfig(base);
}

#ifndef NO_KDE2
void K3DockMainWindow::writeDockConfig( KConfig* c, QString group )
{
  dockManager->writeConfig( c, group );
}

void K3DockMainWindow::readDockConfig( KConfig* c, QString group )
{
  dockManager->readConfig( c, group );
}
#endif

void K3DockMainWindow::slotDockWidgetUndocked()
{
  QObject* pSender = (QObject*) sender();
  if (!pSender->inherits("K3DockWidget")) return;
  K3DockWidget* pDW = (K3DockWidget*) pSender;
  emit dockWidgetHasUndocked( pDW);
}

/*************************************************************************/
K3DockWidgetAbstractHeaderDrag::K3DockWidgetAbstractHeaderDrag( K3DockWidgetAbstractHeader* parent, K3DockWidget* dock, const char* name )
:QFrame( parent, name )
{
  dw = dock;
  installEventFilter( dock->dockManager() );
}
/*************************************************************************/
K3DockWidgetHeaderDrag::K3DockWidgetHeaderDrag( K3DockWidgetAbstractHeader* parent, K3DockWidget* dock, const char* name )
:K3DockWidgetAbstractHeaderDrag( parent, dock, name )
{
}

void K3DockWidgetHeaderDrag::paintEvent( QPaintEvent* )
{
  QPainter paint;

  paint.begin( this );
  QStyleOption qso;
  qso.init(this);
  qso.rect = QRect( 0, 0, width(), height() );
  style()->drawPrimitive( QStyle::PE_Q3DockWindowSeparator, &qso, &paint, this );

  paint.end();
}
/*************************************************************************/
K3DockWidgetAbstractHeader::K3DockWidgetAbstractHeader( K3DockWidget* parent, const char* name )
:QFrame( parent, name )
{
}
/*************************************************************************/
K3DockWidgetHeader::K3DockWidgetHeader( K3DockWidget* parent, const char* name )
:K3DockWidgetAbstractHeader( parent, name )
{
#ifdef BORDERLESS_WINDOWS
  setCursor(QCursor(Qt::ArrowCursor));
#endif
  d = new K3DockWidgetHeaderPrivate( this );

  layout = new QHBoxLayout( this );
  layout->setResizeMode( QLayout::Minimum );

  drag = new K3DockWidgetHeaderDrag( this, parent );

  closeButton = new K3DockButton_Private( this, "DockCloseButton" );
  QToolTip::add( closeButton, i18n("Close") );
  closeButton->setIcon( QIcon( style()->standardPixmap(QStyle::SP_TitleBarCloseButton) ) );
//  closeButton->setFixedSize(closeButton->pixmap()->width(),closeButton->pixmap()->height());
  connect( closeButton, SIGNAL(clicked()), parent, SIGNAL(headerCloseButtonClicked()));
  connect( closeButton, SIGNAL(clicked()), parent, SLOT(undock()));

  stayButton = new K3DockButton_Private( this, "DockStayButton" );
  QToolTip::add( stayButton, i18n("Freeze the window geometry", "Freeze") );
  stayButton->setToggleButton( true );
  stayButton->setIcon( QIcon(not_close_xpm) );
//  stayButton->setFixedSize(closeButton->pixmap()->width(),closeButton->pixmap()->height());
  connect( stayButton, SIGNAL(clicked()), this, SLOT(slotStayClicked()));

  dockbackButton = new K3DockButton_Private( this, "DockbackButton" );
  QToolTip::add( dockbackButton, i18n("Dock this window", "Dock") );
  dockbackButton->setIcon( QIcon(dockback_xpm));
//  dockbackButton->setFixedSize(closeButton->pixmap()->width(),closeButton->pixmap()->height());
  connect( dockbackButton, SIGNAL(clicked()), parent, SIGNAL(headerDockbackButtonClicked()));
  connect( dockbackButton, SIGNAL(clicked()), parent, SLOT(dockBack()));

  d->toDesktopButton = new K3DockButton_Private( this, "ToDesktopButton" );
  QToolTip::add( d->toDesktopButton, i18n("Detach") );
  d->toDesktopButton->setIcon( QIcon(todesktop_xpm));
//  d->toDesktopButton->setFixedSize(closeButton->pixmap()->width(),closeButton->pixmap()->height());
  connect( d->toDesktopButton, SIGNAL(clicked()), parent, SLOT(toDesktop()));
  stayButton->hide();

  d->dummy = new QWidget( this );
//  d->dummy->setFixedSize( 1,closeButton->pixmap()->height() );


  layout->addWidget( drag );
  layout->addWidget( dockbackButton );
  layout->addWidget( d->toDesktopButton );
  layout->addWidget( d->dummy);
  layout->addWidget( stayButton );
  layout->addWidget( closeButton );
  layout->activate();
  d->dummy->hide();
  drag->setFixedHeight( layout->minimumSize().height() );
}

void K3DockWidgetHeader::setTopLevel( bool isTopLevel )
{
  d->topLevel = isTopLevel;
  if ( isTopLevel ){
    K3DockWidget* par = (K3DockWidget*)parent();
    if( par && par->isDockBackPossible() )
        dockbackButton->show();
    else
        dockbackButton->hide();

    stayButton->hide();
    closeButton->hide();
    d->toDesktopButton->hide();
    drag->setEnabled( true );
  } else {
    dockbackButton->hide();
    stayButton->hide();
    if (!d->forceCloseButtonHidden) closeButton->show();
    if( d->showToDesktopButton )
      d->toDesktopButton->show();
  }
  layout->activate();

   bool dontShowDummy=drag->isVisibleTo(this) || dockbackButton->isVisibleTo(this) ||
        d->toDesktopButton->isVisibleTo(this) || stayButton->isVisibleTo(this) ||
        closeButton->isVisibleTo(this);
   for (Q3PtrListIterator<K3DockButton_Private> it( d->btns );it.current();++it) {
        dontShowDummy=dontShowDummy || (it.current()->isVisibleTo(this));
   }
   if (dontShowDummy) d->dummy->hide(); else d->dummy->show();

  updateGeometry();
}

void K3DockWidgetHeader::forceCloseButtonHidden(bool hidden) {
  d->forceCloseButtonHidden=hidden;
  if (hidden) closeButton->hide();
  else closeButton->show();
}

K3DockWidgetHeaderDrag *K3DockWidgetHeader::dragPanel() {
	return drag;
}

void K3DockWidgetHeader::setDragPanel( K3DockWidgetHeaderDrag* nd )
{
  if ( !nd ) return;

  delete layout;
  layout = new QHBoxLayout( this );
  layout->setResizeMode( QLayout::Minimum );

  delete drag;
  drag = nd;
  if (drag->parentWidget()!=this) {
	drag->reparent(this,QPoint(0,0));
  }


  layout->addWidget( drag );
  layout->addWidget( dockbackButton );
  layout->addWidget( d->dummy );
  layout->addWidget( d->toDesktopButton );
  layout->addWidget( stayButton );
  bool dontShowDummy=drag->isVisibleTo(this) || dockbackButton->isVisibleTo(this) ||
	d->toDesktopButton->isVisibleTo(this) || stayButton->isVisibleTo(this) ||
	closeButton->isVisibleTo(this);
  for (Q3PtrListIterator<K3DockButton_Private> it( d->btns );it.current();++it) {
      layout->addWidget(it.current());
	dontShowDummy=dontShowDummy || (it.current()->isVisibleTo(this));
  }
  if (dontShowDummy) d->dummy->hide(); else d->dummy->show();
  layout->addWidget( closeButton );
  layout->activate();
  kdDebug(282)<<"KdockWidgetHeader::setDragPanel:minimum height="<<layout->minimumSize().height()<<endl;
  //FIXME somebody left this here, but we don't know what the hell it's for.
  drag->setFixedHeight( closeButton->height()); // /*layout->minimumS*/sizeHint().height() );
}

void K3DockWidgetHeader::addButton(K3DockButton_Private* btn) {
	if (!btn) return;

	if (btn->parentWidget()!=this) {
		btn->reparent(this,QPoint(0,0));
	}
	btn->setFixedSize(closeButton->pixmap()->width(),closeButton->pixmap()->height());
	if (!d->btns.containsRef(btn)) d->btns.append(btn);

	btn->show();

	delete layout;
	layout = new QHBoxLayout( this );
	layout->setResizeMode( QLayout::Minimum );

	layout->addWidget( drag );
 	layout->addWidget( dockbackButton );
	layout->addWidget( d->toDesktopButton );
	layout->addWidget( d->dummy);
	layout->addWidget( stayButton );
	 bool dontShowDummy=drag->isVisibleTo(this) || dockbackButton->isVisibleTo(this) ||
	        d->toDesktopButton->isVisibleTo(this) || stayButton->isVisibleTo(this) ||
        	closeButton->isVisibleTo(this);
	 for (Q3PtrListIterator<K3DockButton_Private> it( d->btns );it.current();++it) {
	        layout->addWidget(it.current());
		dontShowDummy=dontShowDummy || (it.current()->isVisibleTo(this));
   	}
  	if (dontShowDummy) d->dummy->hide(); else d->dummy->show();
	layout->addWidget( closeButton );
	layout->activate();
	drag->setFixedHeight( layout->minimumSize().height() );
}

void K3DockWidgetHeader::removeButton(K3DockButton_Private* btn) {
	if (btn->parentWidget()==this) {
		if (d->btns.containsRef(btn)) d->btns.removeRef(btn);
		delete btn;
	}
}


void K3DockWidgetHeader::slotStayClicked()
{
  setDragEnabled(!stayButton->isOn());
}

bool K3DockWidgetHeader::dragEnabled() const
{
  return drag->isEnabled();
}

void K3DockWidgetHeader::showUndockButton(bool show)
{
  kdDebug(282)<<"K3DockWidgetHeader::showUndockButton("<<show<<")"<<endl;
  if( d->showToDesktopButton == show )
    return;

  d->showToDesktopButton = show;
  if( !show || d->topLevel )
    d->toDesktopButton->hide( );
  else
    d->toDesktopButton->show( );
}

void K3DockWidgetHeader::setDragEnabled(bool b)
{
  stayButton->setOn(!b);
  closeButton->setEnabled(b);
  drag->setEnabled(b);
}

#ifndef NO_KDE2
void K3DockWidgetHeader::saveConfig( KConfig* c )
{
  c->writeEntry( QString("%1%2").arg(parent()->name()).arg(":stayButton"), QVariant(stayButton->isOn()) );
}

void K3DockWidgetHeader::loadConfig( KConfig* c )
{
  setDragEnabled( !c->readEntry( QString("%1%2").arg(parent()->name()).arg(":stayButton"), QVariant(false) ).toBool() );
}
#endif

/*************************************************************************/

class K3DockManager::K3DockManagerPrivate
{
public:
  /**
   * This rectangle is used to highlight the current dockposition. It stores global screen coordinates.
   */
  QRect dragRect;

  /**
   * This rectangle is used to erase the previously highlighted dockposition. It stores global screen coordinates.
   */
  QRect oldDragRect;

  /**
   * This flag stores the information if dragging is ready to start. Used between mousePress and mouseMove event.
   */
  bool readyToDrag;

  /**
   * This variable stores the offset of the mouse cursor to the upper left edge of the current drag widget.
   */
  QPoint dragOffset;

  /**
   * These flags store information about the splitter behavior
   */
  bool splitterOpaqueResize;
  bool splitterKeepSize;
  bool splitterHighResolution;

  QPointer<K3DockWidget> mainDockWidget;

  QList<QObject*> containerDocks;

  QPointer<K3DockWidget> leftContainer;
  QPointer<K3DockWidget> topContainer;
  QPointer<K3DockWidget> rightContainer;
  QPointer<K3DockWidget> bottomContainer;
  int m_readDockConfigMode;
};


/*************************************************************************/
K3DockWidget::K3DockWidget( K3DockManager* dockManager, const char* name, const QPixmap &pixmap, QWidget* parent, const QString& strCaption, const QString& strTabPageLabel, Qt::WFlags f)
#ifdef BORDERLESS_WINDOWS
: QWidget( parent, name, f )//| WType_Dialog | WStyle_Customize | WStyle_NoBorder )
#else
: QWidget( parent, name, f )
#endif
  ,formerBrotherDockWidget(0L)
  ,currentDockPos(DockNone)
  ,formerDockPos(DockNone)
  ,widget(0L)
  ,pix(new QPixmap(pixmap))
  ,prevSideDockPosBeforeDrag(DockNone)
  ,isGroup(false)
{
  d = new K3DockWidgetPrivate();  // create private data

  d->_parent = parent;

  layout = new QVBoxLayout( this );
  layout->setResizeMode( QLayout::Minimum );

  manager = dockManager;
  manager->childDock->append( this );
  installEventFilter( manager );

  eDocking = DockFullDocking;
  sDocking = DockFullSite;

  header = 0L;
  setHeader( new K3DockWidgetHeader( this, "AutoCreatedDockHeader" ) );

  if( strCaption.isNull() )
    setCaption( name );
  else
    setCaption( strCaption);

  if( strTabPageLabel == " ")
    setTabPageLabel( caption());
  else
    setTabPageLabel( strTabPageLabel);

  isTabGroup = false;
  d->isContainer =false;
  setIcon( pixmap);
  widget = 0L;

  QObject::connect(this, SIGNAL(hasUndocked()), manager->main, SLOT(slotDockWidgetUndocked()) );
  applyToWidget( parent, QPoint(0,0) );
}

void K3DockWidget::setPixmap(const QPixmap& pixmap) {
	delete pix;
	pix=new QPixmap(pixmap);
	setIcon(*pix);
	K3DockTabGroup *dtg=parentDockTabGroup();
	if (dtg)
		dtg->changeTab(this,pixmap,dtg->tabLabel(this));
	 QWidget *contWid=parentDockContainer();
         if (contWid) {
         	K3DockContainer *x = dynamic_cast<K3DockContainer*>(contWid);
                if (x) {
                        x->setPixmap(this,pixmap);
                }
         }
}

const QPixmap& K3DockWidget::pixmap() const {
	return *pix;
}

K3DockWidget::~K3DockWidget()
{
  d->pendingDtor = true;
  if ( !manager->undockProcess ){
    d->blockHasUndockedSignal = true;
    undock();
    d->blockHasUndockedSignal = false;
  }

  if (latestK3DockContainer()) {
    K3DockContainer *x = dynamic_cast<K3DockContainer*>(latestK3DockContainer());
    if (x) {
      x->removeWidget(this);
    }
  }
  emit iMBeingClosed();
  if (manager->d) manager->d->containerDocks.remove(this);
  manager->childDock->remove( this );
  delete pix;
  delete d; // destroy private data
  d=0;
}

void K3DockWidget::paintEvent(QPaintEvent* pe)
{
  QWidget::paintEvent(pe);
  QPainter paint;
  paint.begin( this );
  QStyleOption option;
  option.init(this);
  option.rect = QRect( 0, 0, width(), height() );
  style()->drawPrimitive (QStyle::PE_FrameDockWidget, &option, &paint, this);
  paint.end();
}

void K3DockWidget::leaveEvent(QEvent *e)
{
	QWidget::leaveEvent(e);
#ifdef BORDERLESS_WINDOWS
	if (parent()) return;
//	setCursor(QCursor(ArrowCursor));
#endif
}

void K3DockWidget::mousePressEvent(QMouseEvent* mme)
{
#ifdef BORDERLESS_WINDOWS
	if (!parent())
	{
		kdDebug(282)<<"K3DockWidget::mousePressEvent"<<endl;

		bool bbottom;
		bool bleft;
		bool bright;
		bool btop;
		int styleheight;
		QPoint mp;
		mp=mme->pos();
      		styleheight=2*style().pixelMetric(QStyle::PM_DefaultFrameWidth,this);
		bbottom=mp.y()>=height()-styleheight;
		btop=mp.y()<=styleheight;
		bleft=mp.x()<=styleheight;
		bright=mp.x()>=width()-styleheight;
		kdDebug(282)<<"mousemovevent"<<endl;
 		d->resizing=true;
		if (bright)
		{
			if (btop)
			{
				d->resizeMode=K3DockWidgetPrivate::ResizeTopRight;
				d->resizePos=QPoint(width(),0)-mme->pos();

			}
			else
			{
				d->resizePos=QPoint(width(),height())-mme->pos();
				if (bbottom) d->resizeMode=K3DockWidgetPrivate::ResizeBottomRight;
				else d->resizeMode=K3DockWidgetPrivate::ResizeRight;
			}
		}
		else if (bleft)
		{
			if (btop) setCursor(QCursor(Qt::SizeFDiagCursor));
			else
			if (bbottom) setCursor(QCursor(Qt::SizeBDiagCursor));
			else setCursor(QCursor(Qt::SizeHorCursor));
		}
		else
		if (bbottom)
		{
			d->resizeMode=K3DockWidgetPrivate::ResizeBottom;
			d->resizePos=QPoint(0,height())-mme->pos();
		}
		else
		if  (btop) setCursor(QCursor(Qt::SizeVerCursor));
		else d->resizing=false;

		if (d->resizing) grabMouse(cursor());

	}
#endif
	QWidget::mousePressEvent(mme);
}

void K3DockWidget::mouseReleaseEvent(QMouseEvent* ev)
{
#ifdef BORDERLESS_WINDOWS
	d->resizing=false;
	releaseMouse();
#endif
	QWidget::mouseReleaseEvent(ev);
}

void  K3DockWidget::mouseMoveEvent(QMouseEvent* mme)
{
	QWidget::mouseMoveEvent(mme);
#ifdef BORDERLESS_WINDOWS
	if (parent()) return;

	if (d->resizing)
	{
		switch (d->resizeMode)
		{
			case K3DockWidgetPrivate::ResizeRight:
				resize(mme->pos().x()+d->resizePos.x(),height());
				break;
			case K3DockWidgetPrivate::ResizeBottomRight:
				resize(mme->pos().x()+d->resizePos.x(),mme->pos().y()+d->resizePos.y());
				break;
			case K3DockWidgetPrivate::ResizeBottom:
				resize(width(),mme->pos().y()+d->resizePos.y());
				break;
			default:
				break;
		}
		return;
	}


	bool bbottom;
	bool bleft;
	bool bright;
	bool btop;
	int styleheight;
	QPoint mp;
	mp=mme->pos();
      	styleheight=2*style().pixelMetric(QStyle::PM_DefaultFrameWidth,this);
	bbottom=mp.y()>=height()-styleheight;
	btop=mp.y()<=styleheight;
	bleft=mp.x()<=styleheight;
	bright=mp.x()>=width()-styleheight;
	kdDebug(282)<<"mousemovevent"<<endl;
	if (bright)
	{
		if (btop) setCursor(QCursor(Qt::SizeBDiagCursor));
		else
		if (bbottom) setCursor(QCursor(Qt::SizeFDiagCursor));
		else setCursor(QCursor(Qt::SizeHorCursor));
	}
	else if (bleft)
	{
		if (btop) setCursor(QCursor(Qt::SizeFDiagCursor));
		else
		if (bbottom) setCursor(QCursor(Qt::SizeBDiagCursor));
		else setCursor(QCursor(Qt::SizeHorCursor));
	}
	else
	if (bbottom ||  btop) setCursor(QCursor(Qt::SizeVerCursor));
	else setCursor(QCursor(Qt::ArrowCursor));
#endif
}

void K3DockWidget::setLatestK3DockContainer(QWidget* container)
{
	if (container)
	{
		if (dynamic_cast<K3DockContainer*>(container))
			d->container=container;
		else
			d->container=0;
	}
}

QWidget* K3DockWidget::latestK3DockContainer()
{
	if (!(d->container)) return 0;
	if (dynamic_cast<K3DockContainer*>(d->container.operator->())) return d->container;
	return 0;
}



K3DockWidgetAbstractHeader *K3DockWidget::getHeader() {
	return header;
}

void K3DockWidget::setHeader( K3DockWidgetAbstractHeader* h )
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
//  kdDebug(282)<<caption()<<": K3DockWidget::setHeader"<<endl;
  setEnableDocking(eDocking);
}

void K3DockWidget::setEnableDocking( int pos )
{
  eDocking = pos;
  if( header && header->inherits( "K3DockWidgetHeader" ) )
     ( ( K3DockWidgetHeader* ) header )->showUndockButton( pos & DockDesktop );
  updateHeader();
}

void K3DockWidget::updateHeader()
{
  if ( parent() ){
#ifdef BORDERLESS_WINDOWS
      layout->setMargin(0);
      setMouseTracking(false);
      setCursor(QCursor(Qt::ArrowCursor));
#endif

    if ( (parent() == manager->main) || isGroup || (eDocking == K3DockWidget::DockNone) ){
      header->hide();
    } else {
      header->setTopLevel( false );
      if (widget && dynamic_cast<K3DockContainer*>(widget))
        header->hide();
      else
        header->show();
    }
  } else {
    header->setTopLevel( true );
    header->show();
#ifdef BORDERLESS_WINDOWS
      layout->setMargin(2*style().pixelMetric(QStyle::PM_DefaultFrameWidth,this));
      setMouseTracking(true);
#endif
  }
}

void K3DockWidget::applyToWidget( QWidget* s, const QPoint& p )
{
  if ( parent() != s )
  {
    hide();
    reparent(s, 0, QPoint(0,0), false);
  }

  if ( s && s->inherits("K3DockMainWindow") ){
    ((K3DockMainWindow*)s)->setView( this );
  }

  if ( manager && s == manager->main ){
      setGeometry( QRect(QPoint(0,0), manager->main->geometry().size()) );
  }

  if ( !s )
  {
    move(p);

#ifndef NO_KDE2
#ifdef Q_WS_X11
    if (d->transient && d->_parent)
      XSetTransientForHint( QX11Info::display(), winId(), d->_parent->winId() );

#ifdef BORDERLESS_WINDOWS
    KWin::setType( winId(), NET::Override); //d->windowType );
//      setWFlags(WStyle_Customize | WStyle_NoBorder | WStyle_Tool);
#else
    KWin::setType( winId(), d->windowType );
#endif // BORDERLESS_WINDOW
#endif // Q_WS_X11
#endif

  }
  updateHeader();

  setIcon(*pix);
}

void K3DockWidget::show()
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

#ifndef NO_KDE2

void K3DockWidget::setDockWindowType (NET::WindowType windowType)
{
  d->windowType = windowType;
  applyToWidget( parentWidget(), QPoint(0,0) );
}

#endif

void K3DockWidget::setDockWindowTransient (QWidget *parent, bool transientEnabled)
{
  d->_parent = parent;
  d->transient = transientEnabled;
  applyToWidget( parentWidget(), QPoint(0,0) );
}

QWidget *K3DockWidget::transientTo() {
	if (d->transient && d->_parent) return d->_parent; else return 0;
}

bool K3DockWidget::event( QEvent *event )
{
  switch ( event->type() )
  {
    #undef FocusIn
    case QEvent::FocusIn:
      if (widget && !d->pendingFocusInEvent) {
         d->pendingFocusInEvent = true;
         widget->setFocus();
      }
      d->pendingFocusInEvent = false;
      break;
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
    case QEvent::WindowTitleChange:
      if ( parentWidget() ){
        if ( parent()->inherits("K3DockSplitter") ){
          ((K3DockSplitter*)(parent()))->updateName();
        }
        if ( parentDockTabGroup() ){
          setDockTabName( parentDockTabGroup() );
          parentDockTabGroup()->setTabLabel( this, tabPageLabel() );
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

K3DockWidget *K3DockWidget::findNearestDockWidget(DockPosition pos)
{
	if (!parent()) return 0;
	if (!parent()->inherits("K3DockSplitter")) return 0;
	Qt::Orientation orientation= ( ( pos == K3DockWidget::DockLeft) ||
	                               ( pos == K3DockWidget::DockRight ) ) ? Qt::Vertical : Qt::Horizontal;
		if (((K3DockSplitter*)(parent()))->orientation() == orientation)
		{
			K3DockWidget *neighbor=
				( ( pos == K3DockWidget::DockLeft ) || ( pos == K3DockWidget::DockTop ) ) ?
				static_cast<K3DockWidget*>( ( ( K3DockSplitter* )( parent() ) )->getFirst() ) :
				static_cast<K3DockWidget*>( ( ( K3DockSplitter* )( parent() ) )->getLast() );

			if (neighbor==this)
  			return (static_cast<K3DockWidget*>(parent()->parent())->findNearestDockWidget(pos));
			else
			if (neighbor->getWidget() && qobject_cast<K3DockTabGroup*>(neighbor->getWidget()))
				return (K3DockWidget*)(((K3DockTabGroup*)neighbor->getWidget())->page(0));
			else
			return neighbor;
		}
		else
		return (static_cast<K3DockWidget*>(parent()->parent())->findNearestDockWidget(pos));

	return 0;
}


K3DockWidget* K3DockWidget::manualDock( K3DockWidget* target, DockPosition dockPos, int spliPos, QPoint pos, bool check, int tabIndex )
{
  if (this == target)
    return 0L;  // docking to itself not possible

//  kdDebug(282)<<"manualDock called "<<endl;
  bool success = true; // tested flag

  // Check to make sure that we can dock in to the position wee
  // were told to dock in to
  if ( !(eDocking & (int)dockPos) ){
    success = false;
//  kdDebug(282)<<"K3DockWidget::manualDock(): success = false (1)"<<endl;
  }

  // fix for apps which use a value > 100%. The splitter position must be between 0..100
  // The old behavior on high resolution was 0..10000. So likely the value is >100.
  if (spliPos > 100) {
      spliPos = spliPos / 100;
      kdDebug(282) << "K3DockWidget::manualDock(): fix splitter position: " << spliPos << endl;
  }

  K3DockWidget *tmpTarget = 0;
  switch (dockPos) {
	case Qt::DockLeft:
		tmpTarget=dockManager()->d->leftContainer;
		break;
	case Qt::DockRight:
		tmpTarget=dockManager()->d->rightContainer;
		break;
	case Qt::DockBottom:
		tmpTarget=dockManager()->d->bottomContainer;
		break;
	case Qt::DockTop:
		tmpTarget=dockManager()->d->topContainer;
		break;
	default:
		tmpTarget = 0;
  }

  //If we're not the target, and the target is our dock manager's main window
  //dock into the temp target as chosen above
  if ( tmpTarget && target && this != tmpTarget && target == dockManager()->d->mainDockWidget )
    return manualDock(tmpTarget,DockCenter,spliPos,pos,check,tabIndex);

  // check if the target allows us to oock into the requested position
  if ( target && !(target->sDocking & (int)dockPos) ){
    success = false;
//  kdDebug(282)<<"K3DockWidget::manualDock(): success = false (2)"<<endl;
  }

  /* if we have a parent, and it's not a K3DockSplitter, and we don't have a parent
   * dock tab group, and our parent isn't a K3DockContainer, and we have no explicit
   * parent dock container...we can't do much yet */
  if ( parent() && !parent()->inherits("K3DockSplitter") && !parentDockTabGroup() &&
  	!(dynamic_cast<K3DockContainer*>(parent())) && !parentDockContainer()){
//  kdDebug(282)<<"K3DockWidget::manualDock(): success = false (3)"<<endl;
//  kdDebug(282)<<parent()->name()<<endl;
    success = false;
  }

  /* If all of our attempts to dock back so far have failed and we have a target
   * and we're not being called recursively (i guess that's what check is for)
   * then attempt to dock back to ourselves. */
  if ( !success ){
    // try to make another manualDock
    K3DockWidget* dock_result = 0L;
    if ( target && !check ){
      K3DockWidget::DockPosition another__dockPos = K3DockWidget::DockNone;
      switch ( dockPos ){
        case K3DockWidget::DockLeft  : another__dockPos = K3DockWidget::DockRight ; break;
        case K3DockWidget::DockRight : another__dockPos = K3DockWidget::DockLeft  ; break;
        case K3DockWidget::DockTop   : another__dockPos = K3DockWidget::DockBottom; break;
        case K3DockWidget::DockBottom: another__dockPos = K3DockWidget::DockTop   ; break;
        default: break;
      }
      dock_result = target->manualDock( this, another__dockPos, spliPos, pos, true, tabIndex );
    }
    return dock_result;
  }
  // end check block

  d->blockHasUndockedSignal = true;
  undock();
  d->blockHasUndockedSignal = false;

  //we have no docking target, so we're undocking
  if ( !target ){
    move( pos );
    show();
    emit manager->change();
    return this;
  }

//  kdDebug(282)<<"Looking for  K3DockTabGroup"<<endl;
  K3DockTabGroup* parentTab = target->parentDockTabGroup();
  if ( parentTab ){
    // add to existing TabGroup
    applyToWidget( parentTab );
    parentTab->insertTab( this, icon() ? *icon() : QPixmap(),
                          tabPageLabel(), tabIndex );

    QWidget *wantTransient=parentTab->transientTo();
    target->setDockWindowTransient(wantTransient,wantTransient);

    setDockTabName( parentTab );
    if( !toolTipStr.isEmpty())
      parentTab->setTabToolTip( this, toolTipStr);

    currentDockPos = K3DockWidget::DockCenter;
    emit manager->change();
    return (K3DockWidget*)parentTab->parent();
  }
  else
  {
//  	kdDebug(282)<<"Looking for  K3DockContainer"<<endl;
  	QWidget *contWid=target->parentDockContainer();
	  if (!contWid) contWid=target->widget;
	  if (contWid)
	  {
	  	K3DockContainer *cont=dynamic_cast<K3DockContainer*>(contWid);
		  if (cont)
		  {
			if (latestK3DockContainer() && (latestK3DockContainer()!=contWid)) {
				K3DockContainer* dc = dynamic_cast<K3DockContainer*>(latestK3DockContainer());
				if (dc) {
					dc->removeWidget(this);
				}
			}
//			kdDebug(282)<<"K3DockContainerFound"<<endl;
			applyToWidget( contWid );
			cont->insertWidget( this, icon() ? *icon() : QPixmap(),
						tabPageLabel(), tabIndex );
			setLatestK3DockContainer(contWid);
//			setDockTabName( parentTab );
			if( !toolTipStr.isEmpty())
			cont->setToolTip( this, toolTipStr);

			currentDockPos = K3DockWidget::DockCenter;
			emit manager->change();
			return (K3DockWidget*)(cont->parentDockWidget());

		  }
	  }
  }

  // create a new dockwidget that will contain the target and this
  QWidget* parentDock = target->parentWidget();
  K3DockWidget* newDock = new K3DockWidget( manager, "tempName", QPixmap(""), parentDock );
  newDock->currentDockPos = target->currentDockPos;

  if ( dockPos == K3DockWidget::DockCenter ){
    newDock->isTabGroup = true;
  } else {
    newDock->isGroup = true;
  }
  newDock->eDocking = (target->eDocking & eDocking) & (~(int)K3DockWidget::DockCenter);

  newDock->applyToWidget( parentDock );

  if ( !parentDock ){
    // dock to a toplevel dockwidget means newDock is toplevel now
    newDock->move( target->frameGeometry().topLeft() );
    newDock->resize( target->geometry().size() );
    if ( target->isVisibleToTLW() ) newDock->show();
  }

  // redirect the dockback button to the new dockwidget
  if( target->formerBrotherDockWidget ) {
    newDock->setFormerBrotherDockWidget(target->formerBrotherDockWidget);
    if( formerBrotherDockWidget )
      target->loseFormerBrotherDockWidget();
    }
  newDock->formerDockPos = target->formerDockPos;


 // HERE SOMETING CREATING CONTAINERS SHOULD BE ADDED !!!!!
  if ( dockPos == K3DockWidget::DockCenter )
  {
    K3DockTabGroup* tab = new K3DockTabGroup( newDock, "_dock_tab");
    QObject::connect(tab, SIGNAL(currentChanged(QWidget*)), d, SLOT(slotFocusEmbeddedWidget(QWidget*)));
    newDock->setWidget( tab );

    target->applyToWidget( tab );
    applyToWidget( tab );


    tab->insertTab( target, target->icon() ? *(target->icon()) : QPixmap(),
                    target->tabPageLabel() );



    if( !target->toolTipString().isEmpty())
     tab->setTabToolTip( target, target->toolTipString());

    tab->insertTab( this, icon() ? *icon() : QPixmap(),
                    tabPageLabel(), tabIndex );

    QRect geom=newDock->geometry();
    QWidget *wantTransient=tab->transientTo();
    newDock->setDockWindowTransient(wantTransient,wantTransient);
    newDock->setGeometry(geom);

    if( !toolTipString().isEmpty())
      tab->setTabToolTip( this, toolTipString());

    setDockTabName( tab );
    tab->show();

    currentDockPos = DockCenter;
    target->formerDockPos = target->currentDockPos;
    target->currentDockPos = DockCenter;
  }
  else {
    // if to dock not to the center of the target dockwidget,
    // dock to newDock
    K3DockSplitter* panner = 0L;
    if ( dockPos == K3DockWidget::DockTop  || dockPos == K3DockWidget::DockBottom ) panner = new K3DockSplitter( newDock, "_dock_split_", Qt::Horizontal, spliPos );
    if ( dockPos == K3DockWidget::DockLeft || dockPos == K3DockWidget::DockRight  ) panner = new K3DockSplitter( newDock, "_dock_split_", Qt::Vertical , spliPos );
    newDock->setWidget( panner );

    panner->setOpaqueResize(manager->splitterOpaqueResize());
    panner->setKeepSize(manager->splitterKeepSize());
    panner->setFocusPolicy( Qt::NoFocus );
    target->applyToWidget( panner );
    applyToWidget( panner );
    target->formerDockPos = target->currentDockPos;
    if ( dockPos == K3DockWidget::DockRight) {
      panner->activate( target, this );
      currentDockPos = K3DockWidget::DockRight;
      target->currentDockPos = K3DockWidget::DockLeft;
    }
    else if( dockPos == K3DockWidget::DockBottom) {
      panner->activate( target, this );
      currentDockPos = K3DockWidget::DockBottom;
      target->currentDockPos = K3DockWidget::DockTop;
    }
    else if( dockPos == K3DockWidget::DockTop) {
      panner->activate( this, target );
      currentDockPos = K3DockWidget::DockTop;
      target->currentDockPos = K3DockWidget::DockBottom;
    }
    else if( dockPos == K3DockWidget::DockLeft) {
      panner->activate( this, target );
      currentDockPos = K3DockWidget::DockLeft;
      target->currentDockPos = K3DockWidget::DockRight;
    }
    target->show();
    show();
    panner->show();
  }

  if ( parentDock ){
    if ( parentDock->inherits("K3DockSplitter") ){
      K3DockSplitter* sp = (K3DockSplitter*)parentDock;
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

K3DockTabGroup* K3DockWidget::parentDockTabGroup() const
{
  if ( !parent() ) return 0L;
  QWidget* candidate = parentWidget()->parentWidget();
  if ( candidate && candidate->inherits("K3DockTabGroup") ) return (K3DockTabGroup*)candidate;
  return 0L;
}

QWidget *K3DockWidget::parentDockContainer() const
{
  if (!parent()) return 0L;
  QWidget* candidate = parentWidget()->parentWidget();
  if (candidate && dynamic_cast<K3DockContainer*>(candidate)) return candidate;
  return 0L;
}


void K3DockWidget::setForcedFixedWidth(int w)
{
	d->forcedWidth=w;
	setFixedWidth(w);
	if (!parent()) return;
	if (parent()->inherits("K3DockSplitter"))
		qobject_cast<K3DockSplitter*>(parent())->setForcedFixedWidth(this,w);
}

void K3DockWidget::setForcedFixedHeight(int h)
{
	d->forcedHeight=h;
	setFixedHeight(h);
	if (!parent()) return;
	if (parent()->inherits("K3DockSplitter"))
		qobject_cast<K3DockSplitter*>(parent())->setForcedFixedHeight(this,h);
}

int K3DockWidget::forcedFixedWidth()
{
	return d->forcedWidth;
}

int K3DockWidget::forcedFixedHeight()
{
	return d->forcedHeight;
}

void K3DockWidget::restoreFromForcedFixedSize()
{
	d->forcedWidth=-1;
	d->forcedHeight=-1;
	setMinimumWidth(0);
	setMaximumWidth(32000);
	setMinimumHeight(0);
	setMaximumHeight(32000);
	if (!parent()) return;
	if (parent()->inherits("K3DockSplitter"))
		qobject_cast<K3DockSplitter*>(parent())->restoreFromForcedFixedSize(this);
}

void K3DockWidget::toDesktop()
{
   QPoint p = mapToGlobal( QPoint( -30, -30 ) );
   if( p.x( ) < 0 )
      p.setX( 0 );
   if( p.y( ) < 0 )
      p.setY( 0 );
   manualDock( 0, DockDesktop, 50, p );
}

K3DockWidget::DockPosition K3DockWidget::currentDockPosition() const
{
	return currentDockPos;
}

void K3DockWidget::undock()
{
//  kdDebug(282)<<"K3DockWidget::undock()"<<endl;

  manager->d->dragRect = QRect ();
  manager->drawDragRectangle ();

  QWidget* parentW = parentWidget();
  if ( !parentW ){
    hide();
    if (!d->blockHasUndockedSignal)
      emit hasUndocked();
    return;
  }

  formerDockPos = currentDockPos;
  currentDockPos = K3DockWidget::DockDesktop;

  manager->blockSignals(true);
  manager->undockProcess = true;

  bool isV = parentW->isVisibleToTLW();

  //UNDOCK HAS TO BE IMPLEMENTED CORRECTLY :)
  K3DockTabGroup* parentTab = parentDockTabGroup();
  if ( parentTab ){
    d->index = parentTab->indexOf( this); // memorize the page position in the tab widget
    parentTab->removePage( this );
/*
    QWidget *wantTransient=parentTab->transientTo();
    target->setDockWindowTransient(wantTransient,wantTransient);
 */
    setFormerBrotherDockWidget((K3DockWidget*)parentTab->page(0));
    applyToWidget( 0L );
    if ( parentTab->count() == 1 ){

      // last subdock widget in the tab control
      K3DockWidget* lastTab = (K3DockWidget*)parentTab->page(0);
      parentTab->removePage( lastTab );
/*      QWidget *wantTransient=parentTab->transientTo();
      target->setDockWindowTransient(wantTransient,wantTransient);*/

      lastTab->applyToWidget( 0L );
      lastTab->move( parentTab->mapToGlobal(parentTab->frameGeometry().topLeft()) );

      // K3DockTabGroup always have a parent that is a K3DockWidget
      K3DockWidget* parentOfTab = (K3DockWidget*)parentTab->parent();
      delete parentTab; // K3DockTabGroup

      QWidget* parentOfDockWidget = parentOfTab->parentWidget();
      if ( !parentOfDockWidget ){
          if ( isV ) lastTab->show();
      } else {
        if ( parentOfDockWidget->inherits("K3DockSplitter") ){
          K3DockSplitter* split = (K3DockSplitter*)parentOfDockWidget;
          lastTab->applyToWidget( split );
          split->deactivate();
          if ( split->getFirst() == parentOfTab ){
            split->activate( lastTab );
            if ( ((K3DockWidget*)split->parent())->splitterOrientation == Qt::Vertical )
              emit ((K3DockWidget*)split->getAnother(parentOfTab))->docking( parentOfTab, K3DockWidget::DockLeft );
            else
              emit ((K3DockWidget*)split->getAnother(parentOfTab))->docking( parentOfTab, K3DockWidget::DockTop );
          } else {
            split->activate( 0L, lastTab );
            if ( ((K3DockWidget*)split->parent())->splitterOrientation == Qt::Vertical )
              emit ((K3DockWidget*)split->getAnother(parentOfTab))->docking( parentOfTab, K3DockWidget::DockRight );
            else
              emit ((K3DockWidget*)split->getAnother(parentOfTab))->docking( parentOfTab, K3DockWidget::DockBottom );
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
  //QWidget* containerWidget = (QWidget*)parent();
  bool undockedFromContainer=false;
  if (d->container)
  {
//	  kdDebug(282)<<"undocked from dockcontainer"<<endl;
	  undockedFromContainer=true;
	  K3DockContainer* dc = dynamic_cast<K3DockContainer*>(d->container.operator->());
	  if (dc) {
		  dc->undockWidget(this);
		  setFormerBrotherDockWidget(dc->parentDockWidget());
	  }
	  applyToWidget( 0L );
  }
   if (!undockedFromContainer) {
/*********************************************************************************************/
    if ( parentW->inherits("K3DockSplitter") ){
      K3DockSplitter* parentSplitterOfDockWidget = (K3DockSplitter*)parentW;
      d->splitPosInPercent = parentSplitterOfDockWidget->separatorPosInPercent();

      K3DockWidget* secondWidget = (K3DockWidget*)parentSplitterOfDockWidget->getAnother( this );
      K3DockWidget* group        = (K3DockWidget*)parentSplitterOfDockWidget->parentWidget();
      setFormerBrotherDockWidget(secondWidget);
      applyToWidget( 0L );
      group->hide();

      if ( !group->parentWidget() ){
        secondWidget->applyToWidget( 0L, group->frameGeometry().topLeft() );
        secondWidget->resize( group->width(), group->height() );
      } else {
        QWidget* obj = group->parentWidget();
        secondWidget->applyToWidget( obj );
        if ( obj->inherits("K3DockSplitter") ){
          K3DockSplitter* parentOfGroup = (K3DockSplitter*)obj;
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
      if (!d->pendingDtor) {
        // don't reparent in the dtor of this
        applyToWidget( 0L );
      }
    }
/*********************************************************************************************/
  }
  }
  manager->blockSignals(false);
  if (!d->blockHasUndockedSignal)
    emit manager->change();
  manager->undockProcess = false;

  if (!d->blockHasUndockedSignal)
    emit hasUndocked();
}

void K3DockWidget::setWidget( QWidget* mw )
{
  if ( !mw ) return;

  if ( mw->parent() != this ){
    mw->reparent(this, 0, QPoint(0,0), false);
  }

#ifdef BORDERLESS_WINDOWS
  if (!mw->ownCursor()) mw->setCursor(QCursor(Qt::ArrowCursor));
#endif
  widget = mw;
  delete layout;

  layout = new QVBoxLayout( this );
  layout->setResizeMode( QLayout::Minimum );

  K3DockContainer* dc = dynamic_cast<K3DockContainer*>(widget);
  if (dc)
  {
    d->isContainer=true;
    manager->d->containerDocks.append(this);
  }
  else
  {
  	d->isContainer=false;
  }

  {
     header->show();
     layout->addWidget( header );
     layout->addWidget( widget,1 );
  }
  updateHeader();
  emit widgetSet(mw);
}

void K3DockWidget::setDockTabName( K3DockTabGroup* tab )
{
  QString listOfName;
  QString listOfCaption;
  for ( int i = 0; i < tab->count(); ++i ) {
    QWidget *w = tab->page( i );
    listOfCaption.append( w->caption() ).append(",");
    listOfName.append( w->name() ).append(",");
  }
  listOfCaption.remove( listOfCaption.length()-1, 1 );
  listOfName.remove( listOfName.length()-1, 1 );

  tab->parentWidget()->setName( listOfName.utf8() );
  tab->parentWidget()->setCaption( listOfCaption );

  tab->parentWidget()->repaint( false ); // K3DockWidget->repaint
  if ( tab->parentWidget()->parent() )
    if ( tab->parentWidget()->parent()->inherits("K3DockSplitter") )
      ((K3DockSplitter*)(tab->parentWidget()->parent()))->updateName();
}

bool K3DockWidget::mayBeHide() const
{
  bool f = (parent() != manager->main);
  return ( !isGroup && !isTabGroup && f && isVisible() && ( eDocking != (int)K3DockWidget::DockNone ) );
}

bool K3DockWidget::mayBeShow() const
{
  bool f = (parent() != manager->main);
  return ( !isGroup && !isTabGroup && f && !isVisible() );
}

void K3DockWidget::changeHideShowState()
{
  if ( mayBeHide() ){
    undock();
    return;
  }

  if ( mayBeShow() ){
    if ( manager->main->inherits("K3DockMainWindow") ){
      ((K3DockMainWindow*)manager->main)->makeDockVisible(this);
    } else {
      makeDockVisible();
    }
  }
}

void K3DockWidget::makeDockVisible()
{
  if ( parentDockTabGroup() ){
    parentDockTabGroup()->showPage( this );
  }
  if (parentDockContainer()) {
    QWidget *contWid=parentDockContainer();
    K3DockContainer *x = dynamic_cast<K3DockContainer*>(contWid);
    if (x) {
      x->showWidget(this);
    }
  }
  if ( isVisible() ) return;

  QWidget* p = parentWidget();
  while ( p ){
    if ( !p->isVisible() )
      p->show();
    p = p->parentWidget();
  }
  if( !parent() ) // is undocked
    dockBack();
  show();
}

void K3DockWidget::setFormerBrotherDockWidget(K3DockWidget *dockWidget)
{
  formerBrotherDockWidget = dockWidget;
  if( formerBrotherDockWidget )
    QObject::connect( formerBrotherDockWidget, SIGNAL(iMBeingClosed()),
                      this, SLOT(loseFormerBrotherDockWidget()) );
}

void K3DockWidget::loseFormerBrotherDockWidget()
{
  if( formerBrotherDockWidget )
    QObject::disconnect( formerBrotherDockWidget, SIGNAL(iMBeingClosed()),
                         this, SLOT(loseFormerBrotherDockWidget()) );
  formerBrotherDockWidget = 0L;
  repaint();
}

void K3DockWidget::dockBack()
{
  if( formerBrotherDockWidget) {
    // search all children if it tries to dock back to a child
    bool found = false;
    QList<K3DockWidget *> cl = findChildren<K3DockWidget *>();
	foreach ( K3DockWidget *obj, cl ) {
		while ( !found && obj!= 0 ) {
  		QWidget* widg = qobject_cast<QWidget*>(obj);
			if( widg == formerBrotherDockWidget)
				found = true;
		}
	}

    if( !found) {
      // can dock back to the former brother dockwidget
      manualDock( formerBrotherDockWidget, formerDockPos, d->splitPosInPercent, QPoint(0,0), false, d->index);
      formerBrotherDockWidget = 0L;
      makeDockVisible();
      return;
    }
  }

  // else dockback to the dockmainwindow (default behavior)
  manualDock( ((K3DockMainWindow*)manager->main)->getMainDockWidget(), formerDockPos, d->splitPosInPercent, QPoint(0,0), false, d->index);
  formerBrotherDockWidget = 0L;
  if (parent())
    makeDockVisible();
}

bool K3DockWidget::isDockBackPossible() const
{
  if( !(formerBrotherDockWidget) || !(formerBrotherDockWidget->dockSite() & formerDockPos))
    return false;
  else
    return true;
}

/**************************************************************************************/


K3DockManager::K3DockManager( QWidget* mainWindow , const char* name )
:QObject( mainWindow, name )
  ,main(mainWindow)
  ,currentDragWidget(0L)
  ,currentMoveWidget(0L)
  ,childDockWidgetList(0L)
  ,autoCreateDock(0L)
  ,storeW(0)
  ,storeH(0)
  ,dragging(false)
  ,undockProcess(false)
  ,dropCancel(true)
{
  d = new K3DockManagerPrivate;

  d->readyToDrag = false;
  d->mainDockWidget=0;

#ifndef NO_KDE2
  d->splitterOpaqueResize = KGlobalSettings::opaqueResize();
#else
  d->splitterOpaqueResize = false;
#endif

  d->splitterKeepSize = false;
  d->splitterHighResolution = false;
  d->m_readDockConfigMode = WrapExistingWidgetsOnly; // default as before

  main->installEventFilter( this );

  undockProcess = false;

  menuData = new Q3PtrList<MenuDockData>;
  menuData->setAutoDelete( true );
  menuData->setAutoDelete( true );

#ifndef NO_KDE2
  menu = new K3PopupMenu();
#else
  menu = new Q3PopupMenu();
#endif

  connect( menu, SIGNAL(aboutToShow()), SLOT(slotMenuPopup()) );
  connect( menu, SIGNAL(activated(int)), SLOT(slotMenuActivated(int)) );

  childDock = new QList<QObject*>();
}


void K3DockManager::setMainDockWidget2(K3DockWidget *w)
{
  d->mainDockWidget=w;
}

K3DockManager::~K3DockManager()
{
  delete menuData;
  delete menu;

  K3DockWidget * obj;
  foreach ( QObject *o, *childDock ) {
  	obj=(K3DockWidget*)o;
    delete obj;
  }

  delete childDock;
  delete d;
  d=0;
}

void K3DockManager::activate()
{
  K3DockWidget * obj;
  foreach ( QObject *o, *childDock ) {
  	obj=(K3DockWidget*)o;
    if ( obj->widget ) obj->widget->show();
    if ( !obj->parentDockTabGroup() ){
        obj->show();
    }
  }
  if ( !main->inherits("QDialog") ) main->show();
}

bool K3DockManager::eventFilter( QObject *obj, QEvent *event )
{

  if ( obj->inherits("K3DockWidgetAbstractHeaderDrag") ){
    K3DockWidget* pDockWdgAtCursor = 0L;
    K3DockWidget* curdw = ((K3DockWidgetAbstractHeaderDrag*)obj)->dockWidget();
    switch ( event->type() ){
      case QEvent::MouseButtonDblClick:
        if (curdw->currentDockPos == K3DockWidget::DockDesktop)  curdw->dockBack();
        else
	{
		curdw->toDesktop();
		// curdw->manualDock (0, K3DockWidget::DockDesktop);
	}
        break;

      case QEvent::MouseButtonPress:
        if ( ((QMouseEvent*)event)->button() == Qt::LeftButton ){
          if ( curdw->eDocking != (int)K3DockWidget::DockNone ){
            dropCancel = true;
            curdw->setFocus();
            qApp->processOneEvent();

            currentDragWidget = curdw;
            currentMoveWidget = 0L;
            childDockWidgetList = new QWidgetList();
            childDockWidgetList->append( curdw );
            findChildDockWidget( curdw, childDockWidgetList );

            //d->oldDragRect = QRect(); should fix rectangle not erased problem
            d->dragRect = QRect(curdw->geometry());
            QPoint p = curdw->mapToGlobal(QPoint(0,0));
            d->dragRect.moveTopLeft(p);
            drawDragRectangle();
            d->readyToDrag = true;

            d->dragOffset = QCursor::pos()-currentDragWidget->mapToGlobal(QPoint(0,0));
          }

        }
        break;
      case QEvent::MouseButtonRelease:
        if ( ((QMouseEvent*)event)->button() == Qt::LeftButton ){
          if ( dragging ){
            if ( !dropCancel )
              drop();
            else
              cancelDrop();
          }
          if (d->readyToDrag) {
              d->readyToDrag = false;
              //d->oldDragRect = QRect(); should fix rectangle not erased problem
              d->dragRect = QRect(curdw->geometry());
              QPoint p = curdw->mapToGlobal(QPoint(0,0));
              d->dragRect.moveTopLeft(p);
              drawDragRectangle();
              currentDragWidget = 0L;
              delete childDockWidgetList;
              childDockWidgetList = 0L;
          }
          dragging = false;
          dropCancel = true;
        }
        break;
      case QEvent::MouseMove:
        if ( dragging ) {

#ifdef BORDERLESS_WINDOWS
//BEGIN TEST
	  K3DockWidget *oldMoveWidget;
	  if (!curdw->parent())
	  {
	  	curdw->move(QCursor::pos()-d->dragOffset);
   	        pDockWdgAtCursor = findDockWidgetAt( QCursor::pos()-QPoint(0,d->dragOffset.y()+3) );
                oldMoveWidget = currentMoveWidget;
	  }
	  else
	  {
	        pDockWdgAtCursor = findDockWidgetAt( QCursor::pos() );
                oldMoveWidget = currentMoveWidget;
	  }
//END TEST
#else
	  pDockWdgAtCursor = findDockWidgetAt( QCursor::pos() );
          K3DockWidget* oldMoveWidget = currentMoveWidget;
#endif

	  if ( currentMoveWidget  && pDockWdgAtCursor == currentMoveWidget ) { //move
            dragMove( currentMoveWidget, currentMoveWidget->mapFromGlobal( QCursor::pos() ) );
            break;
          } else {
            if (dropCancel && curdw) {
              d->dragRect = QRect(curdw->geometry());
              QPoint p = curdw->mapToGlobal(QPoint(0,0));
              d->dragRect.moveTopLeft(p);
            }else
              d->dragRect = QRect();

            drawDragRectangle();
          }

          if ( !pDockWdgAtCursor && !(curdw->eDocking & (int)K3DockWidget::DockDesktop) ){
              // just moving at the desktop
              currentMoveWidget = pDockWdgAtCursor;
              curPos = K3DockWidget::DockDesktop;
          } else {
            if ( oldMoveWidget && pDockWdgAtCursor != currentMoveWidget ) { //leave
              currentMoveWidget = pDockWdgAtCursor;
              curPos = K3DockWidget::DockDesktop;
            }
          }

          if ( oldMoveWidget != pDockWdgAtCursor && pDockWdgAtCursor ) { //enter pDockWdgAtCursor
            currentMoveWidget = pDockWdgAtCursor;
            curPos = K3DockWidget::DockDesktop;
          }
        } else {
          if (d->readyToDrag) {
            d->readyToDrag = false;
          }
          if ( (((QMouseEvent*)event)->state() == Qt::LeftButton) &&
               (curdw->eDocking != (int)K3DockWidget::DockNone) ) {
            startDrag( curdw);
          }
        }
	break;
      default:
        break;
    }
  }
  return QObject::eventFilter( obj, event );
}

K3DockWidget* K3DockManager::findDockWidgetAt( const QPoint& pos )
{
  dropCancel = true;

  if (!currentDragWidget)
    return 0L; // pointer access safety

  if (currentDragWidget->eDocking == (int)K3DockWidget::DockNone ) return 0L;

  QWidget* p = QApplication::widgetAt( pos );
  if ( !p ) {
    dropCancel = false;
    return 0L;
  }
#if defined(_OS_WIN32_) || defined(Q_OS_WIN32)
  p = p->topLevelWidget();
#endif
  QWidget* w = 0L;
  findChildDockWidget( w, p, p->mapFromGlobal(pos) );
  if ( !w ){
    if ( !p->inherits("K3DockWidget") ) {
      return 0L;
    }
    w = p;
  }
  if ( findChild<K3DockSplitter*>("_dock_split_")) return 0L;
  if ( findChild<K3DockTabGroup*>( "_dock_tab" )) return 0L;
  if (dynamic_cast<K3DockContainer*>(w)) return 0L;

  if (!childDockWidgetList) return 0L;
  if ( childDockWidgetList->indexOf(w) != -1 ) return 0L;
  if ( currentDragWidget->isGroup && ((K3DockWidget*)w)->parentDockTabGroup() ) return 0L;

  K3DockWidget* www = (K3DockWidget*)w;
  if ( www->sDocking == (int)K3DockWidget::DockNone ) return 0L;
  if( !www->widget )
    return 0L;

  K3DockWidget::DockPosition curPos = K3DockWidget::DockDesktop;
  QPoint cpos  = www->mapFromGlobal( pos );

  int ww = www->widget->width() / 3;
  int hh = www->widget->height() / 3;

  if ( cpos.y() <= hh ){
    curPos = K3DockWidget::DockTop;
  } else
    if ( cpos.y() >= 2*hh ){
      curPos = K3DockWidget::DockBottom;
    } else
      if ( cpos.x() <= ww ){
        curPos = K3DockWidget::DockLeft;
      } else
        if ( cpos.x() >= 2*ww ){
          curPos = K3DockWidget::DockRight;
        } else
            curPos = K3DockWidget::DockCenter;

  if ( !(www->sDocking & (int)curPos) ) return 0L;
  if ( !(currentDragWidget->eDocking & (int)curPos) ) return 0L;
  if ( www->manager != this ) return 0L;

  dropCancel = false;
  return www;
}

void K3DockManager::findChildDockWidget( QWidget*& ww, const QWidget* p, const QPoint& pos )
{
  if ( !p->children().isEmpty() ) {
    QWidget *w;
    foreach( QObject* o, p->children() )
    {
      if ( o->isWidgetType() ) {
        w = (QWidget*)o;
        if ( w->isVisible() && w->geometry().contains(pos) ) {
          if ( w->inherits("K3DockWidget") ) ww = w;
          findChildDockWidget( ww, w, w->mapFromParent(pos) );
          return;
        }
      }
    }
  }
  return;
}

void K3DockManager::findChildDockWidget( const QWidget* p, QWidgetList*& list )
{
  if ( !p->children().isEmpty() ) {
    QWidget *w;
    foreach( QObject* o, p->children() )
    {
      if ( o->isWidgetType() ) {
        w = (QWidget*)o;
        if ( w->isVisible() ) {
          if ( w->inherits("K3DockWidget") ) list->append( w );
          findChildDockWidget( w, list );
        }
      }
    }
  }
  return;
}

void K3DockManager::startDrag( K3DockWidget* w )
{
  if(( w->currentDockPos == K3DockWidget::DockLeft) || ( w->currentDockPos == K3DockWidget::DockRight)
   || ( w->currentDockPos == K3DockWidget::DockTop) || ( w->currentDockPos == K3DockWidget::DockBottom)) {
    w->prevSideDockPosBeforeDrag = w->currentDockPos;

    if ( w->parentWidget()->inherits("K3DockSplitter") ){
      K3DockSplitter* parentSplitterOfDockWidget = (K3DockSplitter*)(w->parentWidget());
      w->d->splitPosInPercent = parentSplitterOfDockWidget->separatorPosInPercent();
    }
  }

  curPos = K3DockWidget::DockDesktop;
  dragging = true;

  QApplication::setOverrideCursor(QCursor(Qt::SizeAllCursor));
}

void K3DockManager::dragMove( K3DockWidget* dw, QPoint pos )
{
  QPoint p = dw->mapToGlobal( dw->widget->pos() );
  K3DockWidget::DockPosition oldPos = curPos;

  QSize r = dw->widget->size();
  if ( dw->parentDockTabGroup() ){
    curPos = K3DockWidget::DockCenter;
    if ( oldPos != curPos ) {
      d->dragRect.setRect( p.x()+2, p.y()+2, r.width()-4, r.height()-4 );
    }
    return;
  }

  int w = r.width() / 3;
  int h = r.height() / 3;

  if ( pos.y() <= h ){
    curPos = K3DockWidget::DockTop;
    w = r.width();
  } else
    if ( pos.y() >= 2*h ){
      curPos = K3DockWidget::DockBottom;
      p.setY( p.y() + 2*h );
      w = r.width();
    } else
      if ( pos.x() <= w ){
        curPos = K3DockWidget::DockLeft;
        h = r.height();
      } else
        if ( pos.x() >= 2*w ){
          curPos = K3DockWidget::DockRight;
          p.setX( p.x() + 2*w );
          h = r.height();
        } else
          {
            curPos = K3DockWidget::DockCenter;
            p.setX( p.x() + w );
            p.setY( p.y() + h );
          }

  if ( oldPos != curPos ) {
    d->dragRect.setRect( p.x(), p.y(), w, h );
    drawDragRectangle();
  }
}


void K3DockManager::cancelDrop()
{
  QApplication::restoreOverrideCursor();

  delete childDockWidgetList;
  childDockWidgetList = 0L;

  d->dragRect = QRect();  // cancel drawing
  drawDragRectangle();    // only the old rect will be deleted
}


void K3DockManager::drop()
{
  d->dragRect = QRect();  // cancel drawing
  drawDragRectangle();    // only the old rect will be deleted

  QApplication::restoreOverrideCursor();

  delete childDockWidgetList;
  childDockWidgetList = 0L;

  if ( dropCancel ) return;
  if ( !currentMoveWidget && (!(currentDragWidget->eDocking & (int)K3DockWidget::DockDesktop)) ) {
    d->dragRect = QRect();  // cancel drawing
    drawDragRectangle();    // only the old rect will be deleted
    return;
  }
  if ( !currentMoveWidget && !currentDragWidget->parent() ) {
    currentDragWidget->move( QCursor::pos() - d->dragOffset );
  }
  else {
    // curPos is the current target DockPosition.
    // currentDragWidget->prevSideDockPosBeforeDrag is where the dockwidget comes from.
    // currentDragWidget->formerDockPos is the position *before* the dockwidget was in
    // position currentDragWidget->prevSideDockPosBeforeDrag.
    int splitPos = currentDragWidget->d->splitPosInPercent;
    K3DockWidget::DockPosition previousPosition = currentDragWidget->prevSideDockPosBeforeDrag;

//    kdDebug() << splitPos << endl;
//    kdDebug() << "curPos: " << curPos << endl;
//    kdDebug() << "formerDockPos: " << currentDragWidget->formerDockPos<< endl;
//    kdDebug() << "prevSideDockPosBeforeDrag: " << currentDragWidget->prevSideDockPosBeforeDrag<< endl;

    // Now we *need* to "invert" the procentual value, if the dockwidget moves from top/left
    // to bottom/right or vice versa. This keeps the dockwidget's size on its new position.
    // A special case is, when the dock position was DockNone, then we have to look for the
    // formerDockPos to get things right.
    if( (curPos != previousPosition)
       && (curPos != K3DockWidget::DockCenter) && (curPos != K3DockWidget::DockDesktop)) {

      if (previousPosition == K3DockWidget::DockNone)
        previousPosition = currentDragWidget->formerDockPos;

      switch( previousPosition ) {
      case K3DockWidget::DockLeft:
        if(curPos != K3DockWidget::DockTop && curPos != K3DockWidget::DockLeft)
          splitPos = 100 - splitPos;
        break;

      case K3DockWidget::DockRight:
        if(curPos != K3DockWidget::DockBottom && curPos != K3DockWidget::DockRight)
          splitPos = 100 - splitPos;
        break;

      case K3DockWidget::DockTop:
        if(curPos != K3DockWidget::DockLeft && curPos != K3DockWidget::DockTop )
          splitPos = 100 - splitPos;
        break;

      case K3DockWidget::DockBottom:
        if(curPos != K3DockWidget::DockRight && curPos != K3DockWidget::DockBottom )
          splitPos = 100 - splitPos;
        break;

      default: break;
      }
    }
    // set new prevSideDockPosBeforeDrag
    currentDragWidget->prevSideDockPosBeforeDrag = curPos;
    currentDragWidget->manualDock( currentMoveWidget, curPos , splitPos, QCursor::pos() - d->dragOffset );
    currentDragWidget->makeDockVisible();
  }
}


static QDomElement createStringEntry(QDomDocument &doc, const QString &tagName, const QString &str)
{
    QDomElement el = doc.createElement(tagName);

    el.appendChild(doc.createTextNode(str));
    return el;
}


static QDomElement createBoolEntry(QDomDocument &doc, const QString &tagName, bool b)
{
    return createStringEntry(doc, tagName, QLatin1String(b? "true" : "false"));
}


static QDomElement createNumberEntry(QDomDocument &doc, const QString &tagName, int n)
{
    return createStringEntry(doc, tagName, QString::number(n));
}


static QDomElement createRectEntry(QDomDocument &doc, const QString &tagName, const QRect &rect)
{
    QDomElement el = doc.createElement(tagName);

    QDomElement xel = doc.createElement("x");
    xel.appendChild(doc.createTextNode(QString::number(rect.x())));
    el.appendChild(xel);
    QDomElement yel = doc.createElement("y");
    yel.appendChild(doc.createTextNode(QString::number(rect.y())));
    el.appendChild(yel);
    QDomElement wel = doc.createElement("width");
    wel.appendChild(doc.createTextNode(QString::number(rect.width())));
    el.appendChild(wel);
    QDomElement hel = doc.createElement("height");
    hel.appendChild(doc.createTextNode(QString::number(rect.height())));
    el.appendChild(hel);

    return el;
}


static QDomElement createListEntry(QDomDocument &doc, const QString &tagName,
                                   const QString &subTagName, const QStringList &list)
{
  QDomElement el = doc.createElement(tagName);

  foreach( QString s, list )
  {
    QDomElement subel = doc.createElement(subTagName);
    subel.appendChild(doc.createTextNode(s));
    el.appendChild(subel);
  }

  return el;
}


static QString stringEntry(QDomElement &base, const QString &tagName)
{
    return base.namedItem(tagName).firstChild().toText().data();
}


static bool boolEntry(QDomElement &base, const QString &tagName)
{
    return base.namedItem(tagName).firstChild().toText().data() == "true";
}


static int numberEntry(QDomElement &base, const QString &tagName)
{
    return stringEntry(base, tagName).toInt();
}


static QRect rectEntry(QDomElement &base, const QString &tagName)
{
    QDomElement el = base.namedItem(tagName).toElement();

    int x = numberEntry(el, "x");
    int y = numberEntry(el, "y");
    int width = numberEntry(el, "width");
    int height = numberEntry(el,  "height");

    return QRect(x, y, width, height);
}


static Q3StrList listEntry(QDomElement &base, const QString &tagName, const QString &subTagName)
{
    Q3StrList list;

    for( QDomNode n = base.namedItem(tagName).firstChild(); !n.isNull(); n = n.nextSibling() )
    {
        QDomElement subel = n.toElement();
        if (subel.tagName() == subTagName)
            list.append(subel.firstChild().toText().data().latin1());
    }

    return list;
}


void K3DockManager::writeConfig(QDomElement &base)
{
    // First of all, clear the tree under base
    while (!base.firstChild().isNull())
        base.removeChild(base.firstChild());
    QDomDocument doc = base.ownerDocument();

    QStringList nameList;
    QString mainWidgetStr;

    // collect widget names
    QStringList nList;
    foreach( QObject *o, *childDock )
    {
        K3DockWidget* dw = (K3DockWidget*) o;
        if ( dw->parent() == main )
            mainWidgetStr = QLatin1String(dw->name());
        nList.append(dw->name());
    }

    for (QObjectList::iterator it = d->containerDocks.begin();it != d->containerDocks.end();++it)
    {
        K3DockContainer* dc = dynamic_cast<K3DockContainer*>(((K3DockWidget*)(*it))->widget);
        if (dc) {
                dc->prepareSave(nList);
        }
    }

    QStringList::Iterator nListIt=nList.begin();
    while ( nListIt!=nList.end() ) {
        K3DockWidget *obj = getDockWidgetFromName( *nListIt);
        if ((obj->isGroup && (!obj->d->isContainer)) && (nameList.indexOf( obj->firstName.latin1() ) == -1
                             || nameList.indexOf(obj->lastName.latin1()) == -1)) {
            // Skip until children are saved (why?)
            ++nListIt;
//            nList.next();
//falk?            if ( !nList.current() ) nList.first();
            continue;
        }

        QDomElement groupEl;
	if (obj->d->isContainer) {
      		K3DockContainer* x = dynamic_cast<K3DockContainer*>(obj->widget);
       		if (x) {
			groupEl=doc.createElement("dockContainer");
          		x->save(groupEl);
       		}
    	} else
        if (obj->isGroup) {
            //// Save a group
            groupEl = doc.createElement("splitGroup");

            groupEl.appendChild(createStringEntry(doc, "firstName", obj->firstName));
            groupEl.appendChild(createStringEntry(doc, "secondName", obj->lastName));
            groupEl.appendChild(createNumberEntry(doc, "orientation", (int)obj->splitterOrientation));
            groupEl.appendChild(createNumberEntry(doc, "separatorPos", ((K3DockSplitter*)obj->widget)->separatorPosInPercent()));
        } else if (obj->isTabGroup) {
            //// Save a tab group
            groupEl = doc.createElement("tabGroup");

            QStringList list;
            for ( int i = 0; i < ((K3DockTabGroup*)obj->widget)->count(); ++i )
                list.append( ((K3DockTabGroup*)obj->widget)->page( i )->name() );
            groupEl.appendChild(createListEntry(doc, "tabs", "tab", list));
            groupEl.appendChild(createNumberEntry(doc, "currentTab", ((K3DockTabGroup*)obj->widget)->currentPageIndex()));
            if (!obj->parent()) {
                groupEl.appendChild(createStringEntry(doc, "dockBackTo", obj->formerBrotherDockWidget ? obj->formerBrotherDockWidget->name() : ""));
                groupEl.appendChild(createNumberEntry(doc, "dockBackToPos", obj->formerDockPos));
            }
        } else {
            //// Save an ordinary dock widget
            groupEl = doc.createElement("dock");
            groupEl.appendChild(createStringEntry(doc, "tabCaption", obj->tabPageLabel()));
            groupEl.appendChild(createStringEntry(doc, "tabToolTip", obj->toolTipString()));
            if (!obj->parent()) {
                groupEl.appendChild(createStringEntry(doc, "dockBackTo", obj->formerBrotherDockWidget ? obj->formerBrotherDockWidget->name() : ""));
                groupEl.appendChild(createNumberEntry(doc, "dockBackToPos", obj->formerDockPos));
            }
        }

        groupEl.appendChild(createStringEntry(doc, "name", QLatin1String(obj->name())));
        groupEl.appendChild(createBoolEntry(doc, "hasParent", obj->parent()));
        if ( !obj->parent() ) {
            groupEl.appendChild(createRectEntry(doc, "geometry", QRect(main->frameGeometry().topLeft(), main->size())));
            groupEl.appendChild(createBoolEntry(doc, "visible", obj->isVisible()));
        }
        if (obj->header && obj->header->inherits("K3DockWidgetHeader")) {
            K3DockWidgetHeader *h = static_cast<K3DockWidgetHeader*>(obj->header);
            groupEl.appendChild(createBoolEntry(doc, "dragEnabled", h->dragEnabled()));
        }

        base.appendChild(groupEl);
        nameList.append(obj->name());
        nList.remove(nListIt);
        nListIt=nList.begin();
    }

    if (main->inherits("K3DockMainWindow")) {
        K3DockMainWindow *dmain = (K3DockMainWindow*)main;
        QString centralWidgetStr = QString(dmain->centralWidget()? dmain->centralWidget()->name() : "");
        base.appendChild(createStringEntry(doc, "centralWidget", centralWidgetStr));
        QString mainDockWidgetStr = QString(dmain->getMainDockWidget()? dmain->getMainDockWidget()->name() : "");
        base.appendChild(createStringEntry(doc, "mainDockWidget", mainDockWidgetStr));
    } else {
        base.appendChild(createStringEntry(doc, "mainWidget", mainWidgetStr));
    }

    base.appendChild(createRectEntry(doc, "geometry", QRect(main->frameGeometry().topLeft(), main->size())));
}


void K3DockManager::readConfig(QDomElement &base)
{
    if (base.namedItem("group").isNull()
        && base.namedItem("tabgroup").isNull()
        && base.namedItem("dock").isNull()
	&& base.namedItem("dockContainer").isNull()) {
        activate();
        return;
    }

    autoCreateDock = new QObjectList;

    bool isMainVisible = main->isVisible();
    main->hide();

    QObjectList::iterator it = childDock->begin();
    K3DockWidget *obj1;
    while ( (obj1=(K3DockWidget*)(*it)) ) {
        if ( !obj1->isGroup && !obj1->isTabGroup ) {
            if ( obj1->parent() )
                obj1->undock();
            else
                obj1->hide();
        }
        ++it;
    }

    // firstly, recreate all common dockwidgets
    for( QDomNode n = base.firstChild(); !n.isNull(); n = n.nextSibling() )
    {
        QDomElement childEl = n.toElement();
        if (childEl.tagName() != "dock") continue;

        // Read an ordinary dock widget
        K3DockWidget *obj = getDockWidgetFromName(stringEntry(childEl, "name"));
        obj->setTabPageLabel(stringEntry(childEl, "tabCaption"));
        obj->setToolTipString(stringEntry(childEl, "tabToolTip"));

        if (!boolEntry(childEl, "hasParent")) {
            QRect r = rectEntry(childEl, "geometry");
            obj = getDockWidgetFromName(stringEntry(childEl, "name"));
            obj->applyToWidget(0);
            obj->setGeometry(r);
            if (boolEntry(childEl, "visible"))
                obj->QWidget::show();
        }

        if (obj && obj->header && obj->header->inherits("K3DockWidgetHeader")) {
            K3DockWidgetHeader *h = static_cast<K3DockWidgetHeader*>(obj->header);
            h->setDragEnabled(boolEntry(childEl, "dragEnabled"));
        }
    }

    // secondly, now iterate again and create the groups and tabwidgets, apply the dockwidgets to them
    for( QDomNode n = base.firstChild(); !n.isNull(); n = n.nextSibling() )
    {
        QDomElement childEl = n.toElement();
        if (childEl.isNull()) continue;

        K3DockWidget *obj = 0;

	if (childEl.tagName() == "dockContainer") {

		K3DockWidget *cont=getDockWidgetFromName(stringEntry(childEl, "name"));
		kdDebug(282)<<"dockContainer: "<<stringEntry(childEl,"name")<<endl;
		if (!(cont->d->isContainer)) {
			kdDebug(282)<<"restoration of dockContainer is only supported for already existing dock containers"<<endl;
		} else {
			K3DockContainer *dc=dynamic_cast<K3DockContainer*>(cont->getWidget());
			if (!dc) kdDebug(282)<<"Error while trying to handle dockcontainer configuration restoration"<<endl;
				else {
					dc->load(childEl);
					removeFromAutoCreateList(cont);
				}

		}
	}
	else
        if (childEl.tagName() == "splitGroup") {
            // Read a group
            QString name = stringEntry(childEl, "name");
            QString firstName = stringEntry(childEl, "firstName");
            QString secondName = stringEntry(childEl, "secondName");
            int orientation = numberEntry(childEl, "orientation");
            int separatorPos = numberEntry(childEl, "separatorPos");

            K3DockWidget *first = getDockWidgetFromName(firstName);
            K3DockWidget *second = getDockWidgetFromName(secondName);
            if (first && second) {
                obj = first->manualDock(second,
                                        (orientation == (int)Qt::Vertical)? K3DockWidget::DockLeft : K3DockWidget::DockTop,
                                        separatorPos);
                if (obj)
                    obj->setName(name.latin1());
            }
        } else if (childEl.tagName() == "tabGroup") {
            // Read a tab group
            QString name = stringEntry(childEl, "name");
            Q3StrList list = listEntry(childEl, "tabs", "tab");

            K3DockWidget *d1 = getDockWidgetFromName( list.first() );
            list.next();
            K3DockWidget *d2 = getDockWidgetFromName( list.current() );

            K3DockWidget *obj = d2->manualDock( d1, K3DockWidget::DockCenter );
            if (obj) {
                K3DockTabGroup *tab = (K3DockTabGroup*)obj->widget;
                list.next();
                while (list.current() && obj) {
                    K3DockWidget *tabDock = getDockWidgetFromName(list.current());
                    obj = tabDock->manualDock(d1, K3DockWidget::DockCenter);
                    list.next();
                }
                if (obj) {
                    obj->setName(name.latin1());
                    tab->showPage(tab->page(numberEntry(childEl, "currentTab")));
                }
            }
        } else {
            continue;
        }

        if (!boolEntry(childEl, "hasParent")) {
            QRect r = rectEntry(childEl, "geometry");
            obj = getDockWidgetFromName(stringEntry(childEl, "name"));
            obj->applyToWidget(0);
            obj->setGeometry(r);
            if (boolEntry(childEl, "visible"))
                obj->QWidget::show();
        }

        if (obj && obj->header && obj->header->inherits("K3DockWidgetHeader")) {
            K3DockWidgetHeader *h = static_cast<K3DockWidgetHeader*>(obj->header);
            h->setDragEnabled(boolEntry(childEl, "dragEnabled"));
        }
    }

    // thirdly, now that all ordinary dockwidgets are created,
    // iterate them again and link them with their corresponding dockwidget for the dockback action
    for( QDomNode n = base.firstChild(); !n.isNull(); n = n.nextSibling() )
    {
        QDomElement childEl = n.toElement();

        if (childEl.tagName() != "dock" && childEl.tagName() != "tabGroup")
            continue;

        K3DockWidget *obj = 0;

        if (!boolEntry(childEl, "hasParent")) {
            // Read a common toplevel dock widget
            obj = getDockWidgetFromName(stringEntry(childEl, "name"));
            QString name = stringEntry(childEl, "dockBackTo");
            if (!name.isEmpty()) {
                obj->setFormerBrotherDockWidget(getDockWidgetFromName(name));
            }
            obj->formerDockPos = K3DockWidget::DockPosition(numberEntry(childEl, "dockBackToPos"));
            obj->updateHeader();
        }
    }

    if (main->inherits("K3DockMainWindow")) {
        K3DockMainWindow *dmain = (K3DockMainWindow*)main;

        QString mv = stringEntry(base, "centralWidget");
        if (!mv.isEmpty() && getDockWidgetFromName(mv) ) {
            K3DockWidget *mvd  = getDockWidgetFromName(mv);
            mvd->applyToWidget(dmain);
            mvd->show();
            dmain->setCentralWidget(mvd);
        }
        QString md = stringEntry(base, "mainDockWidget");
        if (!md.isEmpty() && getDockWidgetFromName(md)) {
            K3DockWidget *mvd  = getDockWidgetFromName(md);
            dmain->setMainDockWidget(mvd);
        }
    } else {
        QString mv = stringEntry(base, "mainWidget");
        if (!mv.isEmpty() && getDockWidgetFromName(mv)) {
            K3DockWidget *mvd  = getDockWidgetFromName(mv);
            mvd->applyToWidget(main);
            mvd->show();
        }

        // only resize + move non-mainwindows
        QRect mr = rectEntry(base, "geometry");
        main->move(mr.topLeft());
        main->resize(mr.size());
    }

    if (isMainVisible)
        main->show();

    if (d->m_readDockConfigMode == WrapExistingWidgetsOnly) {
        finishReadDockConfig(); // remove empty dockwidgets
    }
}

void K3DockManager::removeFromAutoCreateList(K3DockWidget* pDockWidget)
{
    if (!autoCreateDock) return;
    autoCreateDock->removeAt(autoCreateDock->indexOf(pDockWidget));
}

void K3DockManager::finishReadDockConfig()
{
    delete autoCreateDock;
    autoCreateDock = 0;
}

void K3DockManager::setReadDockConfigMode(int mode)
{
    d->m_readDockConfigMode = mode;
}

#ifndef NO_KDE2
void K3DockManager::writeConfig( KConfig* c, QString group )
{
  //debug("BEGIN Write Config");
  if ( !c ) c = KGlobal::config();
  if ( group.isEmpty() ) group = "dock_setting_default";

  c->setGroup( group );
  c->writeEntry( "Version", DOCK_CONFIG_VERSION );

  QStringList nameList;
  QStringList findList;
  QObjectList::iterator it = childDock->begin();
  K3DockWidget * obj;

  // collect K3DockWidget's name
  QStringList nList;
  while ( (obj=(K3DockWidget*)(*it)) ) {
    ++it;
    //debug("  +Add subdock %s", obj->name());
    nList.append( obj->name() );
    if ( obj->parent() == main )
      c->writeEntry( "Main:view", obj->name() );
  }

//  kdDebug(282)<<QString("list size: %1").arg(nList.count())<<endl;
  for (QObjectList::iterator it = d->containerDocks.begin() ;it != d->containerDocks.end(); ++it)
  {
        K3DockContainer* dc = dynamic_cast<K3DockContainer*>(((K3DockWidget*)(*it))->widget);
	if (dc) {
		dc->prepareSave(nList);
	}
  }
//  kdDebug(282)<<QString("new list size: %1").arg(nList.count())<<endl;

  QStringList::Iterator nListIt=nList.begin();
  while ( nListIt!=nList.end() ){
    //debug("  -Try to save %s", nList.current());
    obj = getDockWidgetFromName( *nListIt );
    QString cname = obj->name();
    if ( obj->header ){
      obj->header->saveConfig( c );
    }
    if (obj->d->isContainer) {
       K3DockContainer* x = dynamic_cast<K3DockContainer*>(obj->widget);
       if (x) {
          x->save(c,group);
       }
    }
/*************************************************************************************************/
    if ( obj->isGroup ){
      if ( (findList.find( obj->firstName ) != findList.end()) && (findList.find( obj->lastName ) != findList.end() )){

        c->writeEntry( cname+":type", "GROUP");
        if ( !obj->parent() ){
          c->writeEntry( cname+":parent", "___null___");
          c->writeEntry( cname+":geometry", QRect(obj->frameGeometry().topLeft(), obj->size()) );
          c->writeEntry( cname+":visible", QVariant(obj->isVisible()) );
        } else {
          c->writeEntry( cname+":parent", "yes");
        }
        c->writeEntry( cname+":first_name", obj->firstName );
        c->writeEntry( cname+":last_name", obj->lastName );
        c->writeEntry( cname+":orientation", QVariant(obj->splitterOrientation) );
        c->writeEntry( cname+":sepPos", QVariant(((K3DockSplitter*)obj->widget)->separatorPosInPercent()) );

        nameList.append( obj->name() );
        findList.append( obj->name() );
        //debug("  Save %s", nList.current());
        nList.remove(nListIt);
        nListIt=nList.begin(); //nList.first();
      } else {
/*************************************************************************************************/
        //debug("  Skip %s", nList.current());
        //if ( findList.find( obj->firstName ) == -1 )
        //  debug("  ? Not found %s", obj->firstName);
        //if ( findList.find( obj->lastName ) == -1 )
        //  debug("  ? Not found %s", obj->lastName);
        ++nListIt;
        // if ( !nList.current() ) nList.first();
	if (nListIt==nList.end()) nListIt=nList.begin();
      }
    } else {
/*************************************************************************************************/
      if ( obj->isTabGroup){
        c->writeEntry( cname+":type", "TAB_GROUP");
        if ( !obj->parent() ){
          c->writeEntry( cname+":parent", "___null___");
          c->writeEntry( cname+":geometry", QRect(obj->frameGeometry().topLeft(), obj->size()) );
          c->writeEntry( cname+":visible", QVariant(obj->isVisible()) );
          c->writeEntry( cname+":dockBackTo", obj->formerBrotherDockWidget ? obj->formerBrotherDockWidget->name() : "");
          c->writeEntry( cname+":dockBackToPos", QVariant(obj->formerDockPos) );
        } else {
          c->writeEntry( cname+":parent", "yes");
        }
        QStringList list;
        for ( int i = 0; i < ((K3DockTabGroup*)obj->widget)->count(); ++i )
          list.append( ((K3DockTabGroup*)obj->widget)->page( i )->name() );
        c->writeEntry( cname+":tabNames", list );
        c->writeEntry( cname+":curTab", QVariant(((K3DockTabGroup*)obj->widget)->currentPageIndex()) );

        nameList.append( obj->name() );
        findList.append( obj->name() ); // not really need !!!
        //debug("  Save %s", nList.current());
        nList.remove(nListIt);
        nListIt=nList.begin();
      } else {
/*************************************************************************************************/
        c->writeEntry( cname+":tabCaption", obj->tabPageLabel());
        c->writeEntry( cname+":tabToolTip", obj->toolTipString());
        if ( !obj->parent() ){
          c->writeEntry( cname+":type", "NULL_DOCK");
          c->writeEntry( cname+":geometry", QRect(obj->frameGeometry().topLeft(), obj->size()) );
          c->writeEntry( cname+":visible", QVariant(obj->isVisible()) );
          c->writeEntry( cname+":dockBackTo", obj->formerBrotherDockWidget ? obj->formerBrotherDockWidget->name() : "");
          c->writeEntry( cname+":dockBackToPos", QVariant(obj->formerDockPos) );
        } else {
          c->writeEntry( cname+":type", "DOCK");
        }
        nameList.append( cname.latin1() );
        //debug("  Save %s", nList.current());
        findList.append( obj->name() );
        nList.remove(nListIt);
        nListIt=nList.begin();
      }
    }
  }
  c->writeEntry( "NameList", nameList );

  c->writeEntry( "Main:Geometry", QRect(main->frameGeometry().topLeft(), main->size()) );
  c->writeEntry( "Main:visible", QVariant(main->isVisible()) ); // curently nou use

  if ( main->inherits("K3DockMainWindow") ){
    K3DockMainWindow* dmain = (K3DockMainWindow*)main;
    // for K3DockMainWindow->setView() in readConfig()
    c->writeEntry( "Main:view", dmain->centralWidget() ? dmain->centralWidget()->name():"" );
    c->writeEntry( "Main:dock", dmain->getMainDockWidget()     ? dmain->getMainDockWidget()->name()    :"" );
  }

  c->sync();
  //debug("END Write Config");
}
#include <qmessagebox.h>
void K3DockManager::readConfig( KConfig* c, QString group )
{
  if ( !c ) c = KGlobal::config();
  if ( group.isEmpty() ) group = "dock_setting_default";

  c->setGroup( group );
  QStringList nameList;
  c->readListEntry( "NameList", nameList );
  QString ver = c->readEntry( "Version", "0.0.1" );
  if ( nameList.isEmpty() || ver != DOCK_CONFIG_VERSION ){
    activate();
    return;
  }

  autoCreateDock = new QObjectList();

  bool isMainVisible = main->isVisible();
 // if (isMainVisible)  // CCC
  //QMessageBox::information(0,"","hallo");
//COMMENTED4TESTING  main->hide();

  QObjectList::iterator it = childDock->begin();
  K3DockWidget * obj;

  while ( (obj=(K3DockWidget*)(*it)) ){
    ++it;
    if ( !obj->isGroup && !obj->isTabGroup )
    {
      if ( obj->parent() ) obj->undock(); else obj->hide();
    }
  }

  // firstly, only the common dockwidgets,
  // they must be restored before e.g. tabgroups are restored
  foreach( QString oname, nameList )
  {
    c->setGroup( group );
    QString type = c->readEntry( oname + ":type", QString() );
    obj = 0L;

    if ( type == "NULL_DOCK" || c->readEntry( oname + ":parent", QString() ) == "___null___" ){
      QRect r = c->readRectEntry( oname + ":geometry" );
      obj = getDockWidgetFromName( oname );
      obj->applyToWidget( 0L );
      obj->setGeometry(r);

      c->setGroup( group );
      obj->setTabPageLabel(c->readEntry( oname + ":tabCaption", QString() ));
      obj->setToolTipString(c->readEntry( oname + ":tabToolTip", QString() ));
      if ( c->readEntry( oname + ":visible",QVariant(false) ).toBool() ){
        obj->QWidget::show();
      }
    }

    if ( type == "DOCK"  ){
      obj = getDockWidgetFromName( oname );
      obj->setTabPageLabel(c->readEntry( oname + ":tabCaption", QString() ));
      obj->setToolTipString(c->readEntry( oname + ":tabToolTip", QString() ));
    }

    if (obj && obj->d->isContainer) {
		dynamic_cast<K3DockContainer*>(obj->widget)->load(c,group);
		removeFromAutoCreateList(obj);
    }
    if ( obj && obj->header){
      obj->header->loadConfig( c );
    }
  }

  // secondly, after the common dockwidgets, restore the groups and tabgroups
  foreach( QString oname, nameList )
  {
    c->setGroup( group );
    QString type = c->readEntry( oname + ":type", QString() );
    obj = 0L;

    if ( type == "GROUP" ){
      K3DockWidget* first = getDockWidgetFromName( c->readEntry( oname + ":first_name", QString() ) );
      K3DockWidget* last  = getDockWidgetFromName( c->readEntry( oname + ":last_name",  QString() ) );
      int sepPos = c->readNumEntry( oname + ":sepPos" );

      Qt::Orientation p = (Qt::Orientation)c->readNumEntry( oname + ":orientation" );
      if ( first  && last ){
        obj = first->manualDock( last, ( p == Qt::Vertical ) ? K3DockWidget::DockLeft : K3DockWidget::DockTop, sepPos );
        if (obj){
          obj->setName( oname.latin1() );
        }
      }
    }

    if ( type == "TAB_GROUP" ){
      K3DockWidget* tabDockGroup = 0L;
      QStringList list = c->readListEntry( oname+":tabNames" );
      QStringList::const_iterator listit = list.begin();
      K3DockWidget* d1 = getDockWidgetFromName( *listit++ );
      K3DockWidget* d2 = getDockWidgetFromName( *listit++ );
      tabDockGroup = d2->manualDock( d1, K3DockWidget::DockCenter );
      if ( tabDockGroup ){
        K3DockTabGroup* tab = dynamic_cast<K3DockTabGroup*>(tabDockGroup->widget);
        while ( listit != list.end() && tabDockGroup ){
          K3DockWidget* tabDock = getDockWidgetFromName( *listit++ );
          tabDockGroup = tabDock->manualDock( d1, K3DockWidget::DockCenter );
        }
        if ( tabDockGroup ){
          tabDockGroup->setName( oname.latin1() );
          c->setGroup( group );
          if (tab)
            tab->showPage( tab->page( c->readNumEntry( oname+":curTab" ) ) );
        }
      }
      obj = tabDockGroup;
    }

    if (obj && obj->d->isContainer)  dynamic_cast<K3DockContainer*>(obj->widget)->load(c,group);
    if ( obj && obj->header){
      obj->header->loadConfig( c );
    }
  }

  // thirdly, now that all ordinary dockwidgets are created,
  // iterate them again and link the toplevel ones of them with their corresponding dockwidget for the dockback action
  foreach( QString oname, nameList )
  {
    c->setGroup( group );
    QString type = c->readEntry( oname + ":type", QString() );
    obj = 0L;

    if ( type == "NULL_DOCK" || c->readEntry( oname + ":parent", QString() ) == "___null___" ){
      obj = getDockWidgetFromName( oname );
      c->setGroup( group );
      QString name = c->readEntry( oname + ":dockBackTo", QString() );
      if (!name.isEmpty()) {
          obj->setFormerBrotherDockWidget(getDockWidgetFromName( name ));
      }
      obj->formerDockPos = K3DockWidget::DockPosition(c->readNumEntry( oname + ":dockBackToPos" ));
    }
  }

  if ( main->inherits("K3DockMainWindow") ){
    K3DockMainWindow* dmain = (K3DockMainWindow*)main;

    c->setGroup( group );
    QString mv = c->readEntry( "Main:view" );
    if ( !mv.isEmpty() && getDockWidgetFromName( mv ) ){
      K3DockWidget* mvd  = getDockWidgetFromName( mv );
      mvd->applyToWidget( dmain );
      mvd->show();
      dmain->setView( mvd );
    }
    c->setGroup( group );
    QString md = c->readEntry( "Main:dock" );
    if ( !md.isEmpty() && getDockWidgetFromName( md ) ){
      K3DockWidget* mvd  = getDockWidgetFromName( md );
      dmain->setMainDockWidget( mvd );
    }
  } else {
    c->setGroup( group );
    QString mv = c->readEntry( "Main:view" );
    if ( !mv.isEmpty() && getDockWidgetFromName( mv ) ){
      K3DockWidget* mvd  = getDockWidgetFromName( mv );
      mvd->applyToWidget( main );
      mvd->show();
    }

  }

  // delete all autocreate dock
  if (d->m_readDockConfigMode == WrapExistingWidgetsOnly) {
    finishReadDockConfig(); // remove empty dockwidgets
  }

  c->setGroup( group );

  QRect mr = c->readRectEntry("Main:Geometry");

  if (!main->inherits("K3DockMainWindow"))
    main->move(mr.topLeft());

  main->resize(mr.size());

  if ( isMainVisible ) main->show();
}
#endif


void K3DockManager::dumpDockWidgets() {
  QObjectList::iterator it = childDock->begin();
  K3DockWidget * obj;
  while ( (obj=(K3DockWidget*)(*it)) ) {
    ++it;
    kdDebug(282)<<"K3DockManager::dumpDockWidgets:"<<obj->name()<<endl;
  }

}

K3DockWidget* K3DockManager::getDockWidgetFromName( const QString& dockName )
{
  QObjectList::iterator it = childDock->begin();
  K3DockWidget * obj;
  while ( (obj=(K3DockWidget*)(*it)) ) {
    ++it;
    if ( obj->objectName() == dockName ) return obj;
  }

  K3DockWidget* autoCreate = 0L;
  if ( autoCreateDock ){
    kdDebug(282)<<"Autocreating dock: "<<dockName<<endl;
    autoCreate = new K3DockWidget( this, dockName.latin1(), QPixmap("") );
    autoCreateDock->append( autoCreate );
  }
  return autoCreate;
}
void K3DockManager::setSplitterOpaqueResize(bool b)
{
  d->splitterOpaqueResize = b;
}

bool K3DockManager::splitterOpaqueResize() const
{
  return d->splitterOpaqueResize;
}

void K3DockManager::setSplitterKeepSize(bool b)
{
  d->splitterKeepSize = b;
}

bool K3DockManager::splitterKeepSize() const
{
  return d->splitterKeepSize;
}

void K3DockManager::setSplitterHighResolution(bool b)
{
  d->splitterHighResolution = b;
}

bool K3DockManager::splitterHighResolution() const
{
  return d->splitterHighResolution;
}

void K3DockManager::slotMenuPopup()
{
  menu->clear();
  menuData->clear();

  QObjectList::iterator it = childDock->begin();
  K3DockWidget * obj;
  int numerator = 0;
  while ( (obj=(K3DockWidget*)(*it)) ) {
    ++it;
    if ( obj->mayBeHide() )
    {
      menu->insertItem( i18n("Hide %1").arg(obj->caption()));
      menuData->append( new MenuDockData( obj, true ) );
    }

    if ( obj->mayBeShow() )
    {
      menu->insertItem( i18n("Show %1").arg(obj->caption()));
      menuData->append( new MenuDockData( obj, false ) );
    }
  }
}

void K3DockManager::slotMenuActivated( int id )
{
  MenuDockData* data = menuData->at( id );
  data->dock->changeHideShowState();
}

K3DockWidget* K3DockManager::findWidgetParentDock( QWidget* w ) const
{
  QObjectList::iterator it = childDock->begin();
  K3DockWidget * dock;
  K3DockWidget * found = 0L;

  while ( (dock=(K3DockWidget*)(*it)) ) {
    ++it;
    if ( dock->widget == w ){ found  = dock; break; }
  }
  return found;
}

void K3DockManager::drawDragRectangle()
{
#ifdef BORDERLESS_WINDOWS
	return
#endif
  if (d->oldDragRect == d->dragRect)
    return;

  int i;
  QRect oldAndNewDragRect[2];
  oldAndNewDragRect[0] = d->oldDragRect;
  oldAndNewDragRect[1] = d->dragRect;

  // 2 calls, one for the old and one for the new drag rectangle
  for (i = 0; i <= 1; i++) {
    if (oldAndNewDragRect[i].isEmpty())
      continue;

    K3DockWidget* pDockWdgAtRect = (K3DockWidget*) QApplication::widgetAt( oldAndNewDragRect[i].topLeft(), true );
    if (!pDockWdgAtRect)
      continue;

    bool isOverMainWdg = false;
    bool unclipped;
    K3DockMainWindow* pMain = 0L;
    K3DockWidget* pTLDockWdg = 0L;
    QWidget* topWdg;
    if (pDockWdgAtRect->topLevelWidget() == main) {
      isOverMainWdg = true;
      topWdg = pMain = (K3DockMainWindow*) main;
      //unclipped = pMain->testWFlags( WPaintUnclipped );
      //pMain->setWFlags( WPaintUnclipped );
    }
    else {
      topWdg = pTLDockWdg = (K3DockWidget*) pDockWdgAtRect->topLevelWidget();
      //unclipped = pTLDockWdg->testWFlags( WPaintUnclipped );
      //pTLDockWdg->setWFlags( WPaintUnclipped );
    }

    // draw the rectangle unclipped over the main dock window
    QPainter p;
    p.begin( topWdg );
    /*
    if ( !unclipped ) {
      if (isOverMainWdg)
        pMain->clearWFlags(WPaintUnclipped);
      else
        pTLDockWdg->clearWFlags(WPaintUnclipped);
    }
    */
    // draw the rectangle
    //p.setRasterOp(Qt::NotXorROP);
    QRect r = oldAndNewDragRect[i];
    r.moveTopLeft( r.topLeft() - topWdg->mapToGlobal(QPoint(0,0)) );
    p.drawRect(r.x(), r.y(), r.width(), r.height());
    p.end();
  }

  // memorize the current rectangle for later removing
  d->oldDragRect = d->dragRect;
}

void K3DockManager::setSpecialLeftDockContainer(K3DockWidget* container) {
	d->leftContainer=container;
}

void K3DockManager::setSpecialTopDockContainer(K3DockWidget* container) {
	d->topContainer=container;
}

void K3DockManager::setSpecialRightDockContainer(K3DockWidget* container) {
	d->rightContainer=container;

}

void K3DockManager::setSpecialBottomDockContainer(K3DockWidget* container) {
	d->bottomContainer=container;
}


K3DockArea::K3DockArea( QWidget* parent, const char *name)
:QWidget( parent, name)
{
  QString new_name = QString(name) + QString("_DockManager");
  dockManager = new K3DockManager( this, new_name.latin1() );
  mainDockWidget = 0L;
}

K3DockArea::~K3DockArea()
{
	delete dockManager;
}

K3DockWidget* K3DockArea::createDockWidget( const QString& name, const QPixmap &pixmap, QWidget* parent, const QString& strCaption, const QString& strTabPageLabel)
{
  return new K3DockWidget( dockManager, name.latin1(), pixmap, parent, strCaption, strTabPageLabel );
}

void K3DockArea::makeDockVisible( K3DockWidget* dock )
{
  if ( dock )
    dock->makeDockVisible();
}

void K3DockArea::makeDockInvisible( K3DockWidget* dock )
{
  if ( dock )
    dock->undock();
}

void K3DockArea::makeWidgetDockVisible( QWidget* widget )
{
  makeDockVisible( dockManager->findWidgetParentDock(widget) );
}

void K3DockArea::writeDockConfig(QDomElement &base)
{
  dockManager->writeConfig(base);
}

void K3DockArea::readDockConfig(QDomElement &base)
{
  dockManager->readConfig(base);
}

void K3DockArea::slotDockWidgetUndocked()
{
  QObject* pSender = (QObject*) sender();
  if (!pSender->inherits("K3DockWidget")) return;
  K3DockWidget* pDW = (K3DockWidget*) pSender;
  emit dockWidgetHasUndocked( pDW);
}

void K3DockArea::resizeEvent(QResizeEvent *rsize)
{
  QWidget::resizeEvent(rsize);
  if (!children().isEmpty()){
#ifndef NO_KDE2
//    kdDebug(282)<<"K3DockArea::resize"<<endl;
#endif
    QList<QWidget *> list = findChildren<QWidget*>();

    foreach( QWidget *w, list )
    {
      w->setGeometry(QRect(QPoint(0,0),size()));
    }
#if 0
    K3DockSplitter *split;
//    for (unsigned int i=0;i<children()->count();i++)
    {
//    	QPtrList<QObject> list(children());
//       QObject *obj=((QPtrList<QObject*>)children())->at(i);
	QObject *obj=children()->getFirst();
       if (split = dynamic_cast<K3DockSplitter*>(obj))
       {
          split->setGeometry( QRect(QPoint(0,0), size() ));
//	  break;
       }
    }
#endif
   }
}

#ifndef NO_KDE2
void K3DockArea::writeDockConfig( KConfig* c, QString group )
{
  dockManager->writeConfig( c, group );
}

void K3DockArea::readDockConfig( KConfig* c, QString group )
{
  dockManager->readConfig( c, group );
}

void K3DockArea::setMainDockWidget( K3DockWidget* mdw )
{
  if ( mainDockWidget == mdw ) return;
  mainDockWidget = mdw;
  mdw->applyToWidget(this);
}
#endif



// KDOCKCONTAINER - AN ABSTRACTION OF THE KDOCKTABWIDGET
K3DockContainer::K3DockContainer(){m_overlapMode=false; m_childrenListBegin=0; m_childrenListEnd=0;}
K3DockContainer::~K3DockContainer(){

	if (m_childrenListBegin)
	{
		struct ListItem *tmp=m_childrenListBegin;
		while (tmp)
		{
			struct ListItem *tmp2=tmp->next;
			free(tmp->data);
			delete tmp;
			tmp=tmp2;
		}
		m_childrenListBegin=0;
		m_childrenListEnd=0;
	}

}

void K3DockContainer::activateOverlapMode(int nonOverlapSize) {
	m_nonOverlapSize=nonOverlapSize;
	m_overlapMode=true;
	if (parentDockWidget() && parentDockWidget()->parent()) {
		kdDebug(282)<<"K3DockContainer::activateOverlapMode: recalculating sizes"<<endl;
		K3DockSplitter *sp= qobject_cast<K3DockSplitter*>(parentDockWidget()->parent());
		if (sp)
			sp->resizeEvent(0);
	}
}

void K3DockContainer::deactivateOverlapMode() {
	if (!m_overlapMode) return;
	m_overlapMode=false;
	if (parentDockWidget() && parentDockWidget()->parent()) {
		kdDebug(282)<<"K3DockContainer::deactivateOverlapMode: recalculating sizes"<<endl;
		K3DockSplitter *sp= qobject_cast<K3DockSplitter*>(parentDockWidget()->parent());
		if (sp)
			sp->resizeEvent(0);
		}
}

bool K3DockContainer::isOverlapMode() {
	return m_overlapMode;
}


bool K3DockContainer::dockDragEnter(K3DockWidget*, QMouseEvent *) { return false;}
bool K3DockContainer::dockDragMove(K3DockWidget*, QMouseEvent *) { return false;}
bool K3DockContainer::dockDragLeave(K3DockWidget*, QMouseEvent *) { return false;}


K3DockWidget *K3DockContainer::parentDockWidget(){return 0;}

QStringList K3DockContainer::containedWidgets() const {
	QStringList tmp;
	for (struct ListItem *it=m_childrenListBegin;it;it=it->next) {
		tmp<<QString(it->data);
	}

	return tmp;
}

void K3DockContainer::showWidget(K3DockWidget *) {
}

void K3DockContainer::insertWidget (K3DockWidget *dw, QPixmap, const QString &, int &)
	{
		struct ListItem *it=new struct ListItem;
		it->data=strdup(dw->name());
		it->next=0;

		if (m_childrenListEnd)
		{
			m_childrenListEnd->next=it;
			it->prev=m_childrenListEnd;
			m_childrenListEnd=it;
		}
		else
		{
			it->prev=0;
			m_childrenListEnd=it;
			m_childrenListBegin=it;
		}
	}
void K3DockContainer::removeWidget (K3DockWidget *dw){
	for (struct ListItem *tmp=m_childrenListBegin;tmp;tmp=tmp->next)
	{
		if (!strcmp(tmp->data,dw->name()))
		{
			free(tmp->data);
			if (tmp->next) tmp->next->prev=tmp->prev;
			if (tmp->prev) tmp->prev->next=tmp->next;
			if (tmp==m_childrenListBegin) m_childrenListBegin=tmp->next;
			if (tmp==m_childrenListEnd) m_childrenListEnd=tmp->prev;
			delete tmp;
			break;
		}
	}
}

//m_children.remove(dw->name());}
void K3DockContainer::undockWidget (K3DockWidget *){;}
void K3DockContainer::setToolTip(K3DockWidget *, QString &){;}
void K3DockContainer::setPixmap(K3DockWidget*,const QPixmap&){;}
void K3DockContainer::load (KConfig*, const QString&){;}
void K3DockContainer::save (KConfig*, const QString&){;}
void K3DockContainer::load (QDomElement&){;}
void K3DockContainer::save (QDomElement&){;}
void K3DockContainer::prepareSave(QStringList &names)
{

	for (struct ListItem *tmp=m_childrenListBegin;tmp; tmp=tmp->next)
		names.remove(tmp->data);
//	for (uint i=0;i<m_children.count();i++)
//	{
//		names.remove(m_children.at(i));
//	}
}


QWidget *K3DockTabGroup::transientTo() {
	QWidget *tT=0;
	for (int i=0;i<count();i++) {
		K3DockWidget *dw=qobject_cast<K3DockWidget*>(page(i));
		QWidget *tmp;
		if ((tmp=dw->transientTo())) {
			if (!tT) tT=tmp;
			else {
				if (tT!=tmp) {
					kdDebug(282)<<"K3DockTabGroup::transientTo: widget mismatch"<<endl;
					return 0;
				}
			}
		}
	}

	kdDebug(282)<<"K3DockTabGroup::transientTo: "<<(tT?"YES":"NO")<<endl;

	return tT;
}

void K3DockWidgetAbstractHeader::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

void K3DockWidgetAbstractHeaderDrag::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

void K3DockWidgetHeaderDrag::virtual_hook( int id, void* data )
{ K3DockWidgetAbstractHeaderDrag::virtual_hook( id, data ); }

void K3DockWidgetHeader::virtual_hook( int id, void* data )
{ K3DockWidgetAbstractHeader::virtual_hook( id, data ); }

void K3DockTabGroup::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

void K3DockWidget::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

void K3DockManager::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

void K3DockMainWindow::virtual_hook( int id, void* data )
{ KMainWindow::virtual_hook( id, data ); }

void K3DockArea::virtual_hook( int, void* )
{ /*KMainWindow::virtual_hook( id, data );*/ }


#ifndef NO_INCLUDE_MOCFILES // for Qt-only projects, because tmake doesn't take this name
#include "k3dockwidget.moc"
#endif

//kate: indent-mode csands; space-indent on; indent-width 2;
