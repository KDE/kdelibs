#include <qpainter.h>
#include <qdrawutl.h>
#include <qpalette.h>
#include <qstring.h>
#include <qframe.h>

#include "ktopwidget.h"

#include "kmenubar.moc"

#include <klocale.h>
#include <kapp.h>
#include <kwm.h>

#define CONTEXT_TOP 1
#define CONTEXT_BOTTOM 2
#define CONTEXT_FLOAT 3

// uncomment this to have menubar raised:

#define MENUBAR_IS_RAISED

// Moving with KToolBoxManager
_menuBar::_menuBar (QWidget *parent, const char *name)
  : QMenuBar (parent, name)
 {
#ifndef MENUBAR_IS_RAISED
   setFrameStyle(NoFrame);
#endif
	
   //MD (17-9-97)
   setLineWidth(1);
 }

_menuBar::~_menuBar ()
 {
 }

static bool standalone_menubar = FALSE;

static QPixmap* miniGo = 0;

/*************************************************************/

  title = 0;

  Parent = parent;        // our father
  oldWFlags = getWFlags();
  menu = new _menuBar (frame);
  frame = new QFrame (this);
  frame->setFrameStyle(NoFrame);

  // WARNING: this is a hack for qt-1.3
  // Real qt-1.3 support should use heightFromWidth() in 
  // resizeEvent. But this will not work for qt-1.2.
  // Let us wait until qt-1.3 is released.  Matthias
  frame->installEventFilter(menu);


  menu = new QMenuBar (frame);
  menu->setLineWidth( 1 );
  oldMenuFrameStyle = menu->frameStyle();

  connect (menu, SIGNAL(activated(int)), this, SLOT(slotActivated(int)));
  connect (menu, SIGNAL(highlighted(int)), this, SLOT(slotHighlighted(int)));
  handle = new QFrame (this);
  handle->setMouseTracking( TRUE );
  handle->installEventFilter(this);
  handle->show();
  handle->raise();
int KMenuBar::idAt( int index )
{
  return menu->heightForWidth( max_width - 9);
  if (position == Floating) // What now?
   {
     // Khm... I'm resized from kwm
     // menu bar installs eventFilter on parent, so we don't have
     // to bother with resizing her
     frame->setGeometry( 9, 0, width()-9, height());
     frame->resize(menu->width(), menu->height());
     if (height() != frame->height() ||
         width() != frame->width()+9)
      {
        //warning ("resize");
        resize(frame->width()+9, frame->height());
      }
     handle->setGeometry(0,0,9,height());
   }
  else
   {
     // I will be resized from KtopLevel
	 
	 // MD (17-9-97) change offset from 11 pixels to 9 pixels
     frame->setGeometry (9, 0, width()-9, height());
	 
     if (menu->height() != height())
      {
        frame->resize(frame->width(), menu->height());
        resize(width(), menu->height());
      }
     handle->setGeometry(0,0,9,height());
   }
  {
    resize(width(), heightForWidth(width()));
void KMenuBar::ContextCallback( int index )
  }
  switch ( index )
{
  int i;
  i = context->exec();
  switch (i)
   {
    case CONTEXT_TOP:
      setMenuBarPos( Top );
      break;
      if (position == Floating)
      setMenuBarPos( Bottom );
    case CONTEXT_FLOAT:
      if (position == Floating || position == FloatingSystem){
        setMenuBarPos (lastPosition);
      else {
        setMenuBarPos( Floating );
      break;
	break;
   }

  handle->repaint (false);
}
  context->insertItem( klocale->translate("Top"),  CONTEXT_TOP );
  context->insertItem( klocale->translate("Bottom"), CONTEXT_BOTTOM );
  context->insertItem( klocale->translate("Floating"), CONTEXT_FLOAT );
  connect( context, SIGNAL( activated( int ) ), this,
	   SLOT( ContextCallback( int ) ) );
  
  context->insertItem( i18n("Bottom"), CONTEXT_BOTTOM );
  context->insertItem( i18n("Floating"), CONTEXT_FLOAT );
  position = Top;
  moving = TRUE;
  transparent = false;

  slotReadConfig();

  mgr =0;

}
  if (position == Floating)
     recreate (Parent, oldWFlags, QPoint (oldX, oldY), TRUE);
  
  // MD: Get a seg. fault if following line included.
  // Sven recommeds, as a temporary measure, remove it.
  //delete context; 
  
  debug ("KMenuBar Destructor: finished");
KMenuBar::~KMenuBar()
{
void KMenuBar::mousePressEvent ( QMouseEvent * )
    delete context;

    }
void KMenuBar::paintEvent(QPaintEvent *)
}
void KMenuBar::paintEvent(QPaintEvent *)
{
  //QApplication::sendEvent(menu, e);
  menu->repaint();
}

void KMenuBar::closeEvent (QCloseEvent *e)
{
     context->changeItem (klocale->translate("Float"), CONTEXT_FLOAT);
   {
     position = lastPosition;
     recreate (Parent, oldWFlags, QPoint (oldX, oldY), TRUE);
     context->changeItem (i18n("Float"), CONTEXT_FLOAT);
     emit moved (position);
     e->ignore();
     return;
   }
  e->accept();
}

void KMenuBar::leaveEvent (QEvent *e){
  QApplication::sendEvent(menu, e);
  QPoint p;
  if (mgr)
    if (ev->type() == Event_MouseButtonPress){
      pointerOffset = mapFromGlobal(handle->mapToGlobal(((QMouseEvent*)ev)->pos()));
      if ( moving && ((QMouseEvent*)ev)->button() != LeftButton)
	context->popup( handle->mapToGlobal(((QMouseEvent*)ev)->pos()), 0 );
      else
	handle->grabMouse(sizeAllCursor);
      return TRUE;
    }
    if (ev->type() == Event_MouseButtonRelease){
      handle->releaseMouse();
    }
    if (ev->type() == Event_MouseMove){
      if (!moving || mouseGrabber() != handle)
	return TRUE;
      if (position != Floating){
	p = mapFromGlobal(QCursor::pos()) - pointerOffset;
	if (p.x()*p.x()+p.y()*p.y()<169)
	  return TRUE;
	XUngrabPointer( qt_xdisplay(), CurrentTime );
 	setMenuBarPos(Floating);
	QApplication::syncX();
	while(XGrabPointer( qt_xdisplay(), handle->winId(), TRUE,
			    ButtonPressMask | ButtonReleaseMask |
			    PointerMotionMask | EnterWindowMask | LeaveWindowMask,
			    GrabModeAsync, GrabModeAsync,
			    None, sizeAllCursor.handle(), 
			    CurrentTime ) != GrabSuccess);
	handle->grabMouse(sizeAllCursor);
      }
      move(QCursor::pos() - pointerOffset);    
      p = QCursor::pos() - pointerOffset - (Parent->mapToGlobal(QPoint(0,0)) + parentOffset);
      if (p.x()*p.x()+p.y()*p.y()<169){
	releaseMouse();
	setMenuBarPos(lastPosition);
	QApplication::syncX();
	while(XGrabPointer( qt_xdisplay(), handle->winId(), TRUE,
			    ButtonPressMask | ButtonReleaseMask |
			    PointerMotionMask | EnterWindowMask | LeaveWindowMask,
			    GrabModeAsync, GrabModeAsync,
			    None, sizeAllCursor.handle(),
			    CurrentTime ) != GrabSuccess);
	handle->grabMouse(sizeAllCursor);
      }
      return TRUE;
		//debug ("KMenuBar: moving done");
    if (ev->type() == Event_Paint){
      }
      QPainter *paint = new QPainter();

      paint->begin( handle );
      qDrawShadePanel( paint, 0, 0, 9, handle->height(),
                       g , FALSE, 1);
      paint->setPen( g.light() );
      stipple_height = 3;
      while ( stipple_height < handle->height()-3 ) {
	paint->drawPoint( 1, stipple_height+1);
	paint->drawPoint( 4, stipple_height);
	stipple_height+=3;
          paint.drawLine(0, h-a+3, h, 0-a+3);
      paint->setPen( g.dark() );
      stipple_height = 4;
      while ( stipple_height < handle->height()-3 ) {
	paint->drawPoint( 2, stipple_height+1);
	paint->drawPoint( 5, stipple_height);
	stipple_height+=3;
      }
      paint->end();
      return TRUE;
          a +=6;
        }
        return TRUE;
      }
    }
  }
  return FALSE;
  moving = flag; 

void KMenuBar::enableMoving(bool flag)
{
  moving = flag;
    if (position == FloatingSystem && standalone_menubar == true) {
	return; // Ignore positioning of Mac menubar
     if (mpos == Floating)

        lastPosition = position;
        position = mpos;
        oldX = x();
        oldY = y();
        oldWFlags = getWFlags();
	QPoint p = mapToGlobal(QPoint(0,0));
	parentOffset = pos();
        hide();
        recreate(0, 0,
                 p, TRUE);
 	XSetTransientForHint( qt_xdisplay(), winId(), Parent->topLevelWidget()->winId());
	KWM::setDecoration(winId(), FALSE);
	KWM::moveToDesktop(winId(), KWM::desktop(Parent->winId()));
	setCaption(""); // this triggers a qt bug
	if (title){
	  setCaption(title);
	}
	else {
	  QString s = Parent->caption();
	  s.append(" [menu]");
	  setCaption(s);
	}
	setFrameStyle( QFrame::Panel | QFrame::Raised );
	menu->setFrameStyle( QFrame::Panel | QFrame::Raised );
        context->changeItem (klocale->translate("UnFloat"), CONTEXT_FLOAT);
		*miniGo = px;
          connect( Parent, SIGNAL(destroyed()), obj, SLOT(tlwDestroyed()));
        }
     else if (position == Floating) // was floating

        return;
      }
     else if (position == Floating || position == FloatingSystem) // was floating
      {
        position = mpos;
        context->changeItem (klocale->translate("Float"), CONTEXT_FLOAT);
	menu->setFrameStyle(oldMenuFrameStyle);
//          menu->setMouseTracking(true);
          menu->setFrameStyle(NoFrame);
        }

          setFlat (true);
          return;
        }
        enableFloating (true);
        position = mpos;
        emit moved ( mpos );
        return;
      }
   }
}

void KMenuBar::enableFloating (bool arrrrrrgh)
{
  context->setItemEnabled (CONTEXT_FLOAT, arrrrrrgh);
}

/*******************************************************/

uint KMenuBar::count()
int KMenuBar::insertItem(const char *text,
  return menu->count();
}

int KMenuBar::insertItem(const QString& text,
               const QObject *receiver, const char *member,
               int accel)
int KMenuBar::insertItem( const char *text, int id, int index)
  return menu->insertItem(text, receiver, member, accel);
}

int KMenuBar::insertItem( const char *text, QPopupMenu *popup,
{
  return menu->insertItem(text, id, index);
}
int KMenuBar::insertItem( const QString& text, QPopupMenu *popup,

{
  return->menu->insertItem (pixmap, receiver, member, accel);
}
*/
void KMenuBar::insertSeparator(int index)
{
  menu->insertSeparator(index);
}

void KMenuBar::removeItem( int id )
{
  menu->removeItem(id);
}
void KMenuBar::removeItemAt( int index )
{
  menu->removeItemAt(index);
}
void KMenuBar::clear()
{
  menu->clear();
}

int KMenuBar::accel( int id )
{
  return menu->accel(id);
}
void KMenuBar::setAccel( int key, int id )
const char *KMenuBar::text( int id )
  menu->setAccel(key, id);
}

QString KMenuBar::text( int id )
void KMenuBar::changeItem( const char *text, int id )
  return menu->text(id);
}

void KMenuBar::changeItem( const QString& text, int id )
{
  menu->changeItem(text, id);
}

void KMenuBar::setItemChecked(int id , bool flag)
{
  menu->setItemChecked(id , flag);
}

void KMenuBar::setItemEnabled(int id, bool flag)
{
  menu->setItemEnabled(id, flag);
}


void KMenuBar::slotActivated (int id)
{
  emit activated(id);
}

    emit moved (position); // KTM will call this->updateRects
  }
}
#include "kmenubar.moc"
