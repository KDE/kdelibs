#include <qpainter.h>
#include <qdrawutl.h>
#include <qpalette.h>
#include <qstring.h>
#include <qframe.h>

#include "ktopwidget.h"

#include "kmenubar.moc"

#include <klocale.h>
#include <kapp.h>

#define CONTEXT_TOP 1
#define CONTEXT_BOTTOM 2
#define CONTEXT_FLOAT 3

// uncomment this to have menubar raised:

#define MENUBAR_IS_RAISED

// Sven -
// But beware that it looks ugly when toolbar is raised
// I cannot draw shaded panel around menubar, cause it would
// totaly #§%$*% resizing
// If kde comunity wants to have raised menubar do the
// resizing things properly or just uncoment this and suffer uglyness

// Mark Donohoe (17-9-97) -
// With a little reworking of the menubar style one can get round the ugly
// look
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
  menu = new QMenuBar (frame);
  menu->setLineWidth( 1 );
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
      else
      if (position == Floating || position == FloatingSystem){
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
  //setFrameStyle( QFrame::Panel | QFrame::Raised );
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
void KMenuBar::mousePressEvent ( QMouseEvent *m )
    delete context;
  if ( moving )
    context->popup( mapToGlobal( m->pos() ), 0 );

    }
void KMenuBar::paintEvent(QPaintEvent *)
}
  int offset=3;
  QColorGroup g = QWidget::colorGroup();
  QPainter *paint = new QPainter();

  int menubarHeight = menu->height();
  int stipple_height;
  
  paint->begin( this );

  // Handle point

  switch ( position ) {
    case Top:
    case Bottom:
		case Floating:
	
      qDrawShadePanel( paint, 0, 0, 9, menubarHeight,
                       g , FALSE, 1);
			
			paint->setPen( g.light() );
      stipple_height = 3;
      while ( stipple_height < menubarHeight-3 ) {
				paint->drawPoint( 1, stipple_height+1);
				paint->drawPoint( 4, stipple_height);
				stipple_height+=3;
			}
			paint->setPen( g.dark() );
      stipple_height = 4;
      while ( stipple_height < menubarHeight-3 ) {
				paint->drawPoint( 2, stipple_height+1);
				paint->drawPoint( 5, stipple_height);
				stipple_height+=3;
			}
    	break;
	}

#ifdef MENUBAR_IS_RAISED
	
	// MD (17-9-97) Change panel shadow from 2 pixels to 1
	qDrawShadePanel(paint, 0, 0, width(), height(), g , FALSE, 1);
  
#endif
  paint->end();
  delete paint;
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
  return FALSE;
  moving = flag; 

void KMenuBar::enableMoving(bool flag)
void KMenuBar::setMenuBarPos(menuPosition pos)
  moving = flag;
  if (position != pos)
	return; // Ignore positioning of Mac menubar
     if (pos == Floating)

        lastPosition = position;
        position = pos;
        oldX = x();
        oldY = y();
        oldWFlags = getWFlags();
        hide();
        recreate(0, WStyle_Customize | WStyle_Title | WStyle_Minimize | WStyle_DialogBorder,
                 QCursor::pos(), TRUE);
        //updateRects (TRUE);
        //show();
        if (title != 0)
          setCaption (title);
        context->changeItem (klocale->translate("UnFloat"), CONTEXT_FLOAT);
        emit moved (pos);
          connect( Parent, SIGNAL(destroyed()), obj, SLOT(tlwDestroyed()));
        }
     else if (position == Floating) // was floating

        position = pos;
      }
        position = mpos;
        //updateRects (TRUE);
        context->changeItem (klocale->translate("Float"), CONTEXT_FLOAT);
        emit moved (pos);
//          menu->setMouseTracking(true);
          menu->setFrameStyle(NoFrame);
        }

        position = pos;
        emit moved ( pos );
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
