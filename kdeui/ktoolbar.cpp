#include <qpainter.h>
#include <qtooltip.h> 
#include <qdrawutl.h>
#include <qpalette.h>
#include <qbitmap.h>
#include <qstring.h>

#include <qlist.h>
#include <qpalette.h>
#include <qbitmap.h>
#include <qstring.h>
#include <qframe.h>
#include <qbutton.h>
#include "kbutton.h"
#include "klined.h"
#include "kcombo.h"
#include "ktopwidget.h"

#include "ktoolbar.moc"
#include <klocale.h>
#include <kapp.h>
#include <kwm.h>
  default:
// Since I didn't get any answers about should
// toolbar be raise or not, I leave it to
// Kalle, the Librarian:
// uncomment this to have toolbar raised:

// MD (17-9-97) Raised toolbar style introduced

#define TOOLBAR_IS_RAISED
      qDrawArrow (_painter, DownArrow, WindowsStyle, false,

// MD (17-9-97) Height reduced from 34 pixels
#define TOOLBARHEIGHT 26

#define CONTEXT_LEFT 0
#define CONTEXT_RIGHT 1
#define CONTEXT_TOP 2
#define CONTEXT_BOTTOM 3
#define CONTEXT_FLOAT 4

#define ITEM_LINED 0
#define ITEM_BUTTON 1
#define ITEM_COMBO 2
#define ITEM_FRAME 3
#define ITEM_TOGGLE 4

#define KToolBarItem QWidget

KToolBarFrame::KToolBarFrame (int ID, QWidget *parent,
                              const char *name) : QFrame (parent, name)
 {
   id = ID;
   right = false;
 }

KToolBarCombo::KToolBarCombo(bool rw, int ID,
                             QWidget *parent, const char *name) : KCombo (rw, parent, name)
{ 
  id = ID;
  right = false;
}

KToolBarLined::KToolBarLined(const char *text, int ID,
                             QWidget *parent, const char *name) : 
  KLined (parent, name)
{
  id = ID;
  setText (text);
  autoSized = false;
  right = false;
}

void KToolBarLined::enable (bool enable)
{
  QWidget::setEnabled (enable);
}

void KToolBarCombo::enable (bool enable)
{
  QWidget::setEnabled (enable);
}


KToolBarButton::KToolBarButton( const QPixmap& pixmap, int ID,
		QWidget *parent, const char *name) : KButton( parent, name )
{
  resize(24,24);
  id = ID;
  if ( ! pixmap.isNull() ) 
    enabledPixmap = pixmap;
  else 
    {
      warning(klocale->translate("KToolBarButton: pixmap is empty, perhaps some missing file"));
      enabledPixmap.resize( 22, 22);
    };
  makeDisabledPixmap();
  KButton::setPixmap( enabledPixmap );
  connect( this, SIGNAL( clicked() ), 
	   this, SLOT( ButtonClicked() ) );
  connect(this, SIGNAL( pressed() ), this, SLOT( ButtonPressed() ) );
  connect(this, SIGNAL( released() ), this, SLOT( ButtonReleased() ) );
  right = false;
}

void KToolBarButton::leaveEvent(QEvent *e)
{
  if (isToggleButton() == FALSE)
    KButton::leaveEvent(e);
}

void KToolBarButton::enterEvent(QEvent *e)
{
  if (isToggleButton() == FALSE)
    KButton::enterEvent(e);
}

void KToolBarButton::beToggle(bool flag)
{
  setToggleButton(flag);
  if (flag == TRUE)
    connect (this, SIGNAL(toggled(bool)), this, SLOT(ButtonToggled()));
  else
    disconnect (this, SIGNAL(toggled(bool)), this, SLOT(ButtonToggled()));
}

void KToolBarButton::on(bool flag)
{
  setOn(flag);
  repaint();
}
  init();
void KToolBarButton::drawButton( QPainter *_painter )
{
  QColorGroup g = QWidget::colorGroup();
  if (isOn())
    qDrawShadePanel(_painter, 0, 0, width(), height(), g , TRUE, 2);
  KButton::drawButton(_painter);
}

void KToolBarButton::toggle()
{
  setOn(!isOn());
  repaint();
}

void KToolBarButton::setPixmap( const QPixmap &pixmap )
{
  if ( ! pixmap.isNull() )
    enabledPixmap = pixmap;
  else
    {
      warning(klocale->translate("KToolBarButton: pixmap is empty, perhaps some missing file"));
      enabledPixmap.resize( 22, 22);
    }
  // makeDisabledPixmap();
  KButton::setPixmap( enabledPixmap );
}            

void KToolBarButton::makeDisabledPixmap()
{
  QPalette pal = palette();
  QColorGroup g = pal.disabled();
  
  // Find the outline of the colored portion of the normal pixmap
  
  QBitmap *pmm = (QBitmap*) enabledPixmap.mask();
  QPixmap pm;
  if (pmm != NULL) 
    {
      pmm->setMask( *pmm );
      pm = *pmm;
    } 
  else 
    {
      pm.resize(22 , 22);
      enabledPixmap.fill(this, 0, 0);
      // warning("KToolBarButton::makeDisabledPixmap: mask is null.");
    };
  
  // Prepare the disabledPixmap for drawing
  
  disabledPixmap.resize(22,22);
  disabledPixmap.fill( g.background() );
  
  // Draw the outline found above in highlight and then overlay a grey version
  // for a cheesy 3D effect ! BTW. this is the way that Qt 1.2+ does it for
  // Windows style
  
  QPainter p;
  p.begin( &disabledPixmap );
  p.setPen( g.light() );
  p.drawPixmap(1, 1, pm);
  p.setPen( g.text() );
  p.drawPixmap(0, 0, pm);
  p.end();
}
  icontext=config->readNumEntry("IconText", 0);
void KToolBarButton::paletteChange(const QPalette &)
{
  if( ID() != -1 ) 
    {
      makeDisabledPixmap();
      if ( !isEnabled() )
	KButton::setPixmap( disabledPixmap );
      repaint( TRUE );
    }
}
  bool doUpdate=false;
KToolBarButton::KToolBarButton( QWidget *parentWidget, const char *name )
  : KButton( parentWidget , name)
{
  resize(6,6);
  hide();
  id = -1;
  right = false;
}
    else
void KToolBarButton::enable( bool enabled )
{
  KButton::setPixmap( (enabled ? enabledPixmap : disabledPixmap) );
  setEnabled( enabled );
}
  if (fullWidth == true)
void KToolBarButton::ButtonClicked()
{
  emit clicked( id );
}
         }
void KToolBarButton::ButtonPressed()
{
  emit pressed( id );
}
  //debug ("Ho, ho, hooo... Up-Date!!! (vertical)");
void KToolBarButton::ButtonReleased()
{
  emit released( id );
}
   QApplication::sendEvent() which, as far as I understand, notifies
void KToolBarButton::ButtonToggled()
{
  emit toggled(id);
}

//--------------------------------------------------------------------------------------
/*
 *  KToolBar
 */
	    context->popup( mapToGlobal( m->pos() ), 0 );
KToolBar::KToolBar(QWidget *parent, const char *name)
  : QFrame( parent, name )
{
  init();
  Parent = parent;        // our father
  max_width=-1;
  title = 0;
}
{
void KToolBar::ContextCallback( int index )
{
  switch ( index ) {
   case CONTEXT_LEFT:
     setBarPos( Left );
     break;
   case CONTEXT_RIGHT:
     setBarPos( Right );
     break;
   case CONTEXT_TOP:
     setBarPos( Top );
     break;
   case CONTEXT_BOTTOM:
     setBarPos( Bottom );
     break;
   case CONTEXT_FLOAT:
     if (position == Floating)
       setBarPos (lastPosition);
     else{
       setBarPos( Floating );
       move(QCursor::pos());
       show();
     }
    break;
  }
}
{
void KToolBar::init()
{
  context = new QPopupMenu( 0, "context" );
  context->insertItem( klocale->translate("Left"), CONTEXT_LEFT );
  context->insertItem( klocale->translate("Top"),  CONTEXT_TOP );
  context->insertItem( klocale->translate("Right"), CONTEXT_RIGHT );
  context->insertItem( klocale->translate("Bottom"), CONTEXT_BOTTOM );
  context->insertItem( klocale->translate("Floating"), CONTEXT_FLOAT );
  connect( context, SIGNAL( activated( int ) ), this,
	   SLOT( ContextCallback( int ) ) );
  
  //MD (17-9-97) Toolbar full width by default
  fullWidth=TRUE;
  
  position = Top;
  moving = TRUE;
  setFrameStyle(NoFrame);
  setLineWidth( 1 );
  resize( width(), TOOLBARHEIGHT );
  items.setAutoDelete(TRUE);
  enableFloating (TRUE);
}
    emit (moved(position));
void KToolBar::drawContents ( QPainter *)
{
}

KToolBar::~KToolBar()
{
  // Get a seg. fault in this destructor with the following line included.
  // Don't know why.
  // Sven: I don't get it
int KToolBar::insertLineSeparator( int index )
  if (position == Floating)
     recreate (Parent, oldWFlags, QPoint (oldX, oldY), TRUE);
  
  for ( KToolBarItem *b = items.first(); b!=NULL; b=items.next() )
    items.remove();
  
  // MD: Get a seg. fault if following line included.
  // Sven recommeds, as a temporary measure, remove it.
  //delete context;
  
  debug ("KToolBar Destructor: finished");
}
  if (position == Floating)
void KToolBar::setMaxHeight (int h)
{
  max_height = h;
}
    emit (moved(position));
void KToolBar::setMaxWidth (int w)
{
  max_width = w;
  updateRects(TRUE);
}

void KToolBar::layoutHorizontal ()
{
  int offset=3+9+4;
  int rightOffset;
  int yOffset=1;
  KToolBarItem *autoSize = 0;
  int maxwidth;

  
  if (position == Floating)
    maxwidth = width();
  else
    if (max_width != -1)
      maxwidth = /*Parent->width()-*/max_width;
    else
      maxwidth = Parent->width();

  if (fullWidth == TRUE)
    toolbarWidth = maxwidth;
  else
    toolbarWidth = offset;
  item->show();
  rightOffset=maxwidth;
  toolbarHeight= TOOLBARHEIGHT;
    updateRects( true );
  for ( KToolBarItem *b = items.first(); b!=NULL; b=items.next() )
   {
     if (fullWidth == TRUE)
      {
        if (isItemRight(b) == TRUE)
         {
           rightOffset=rightOffset-3-b->width();

           if (rightOffset <= (offset+3))
            {
              yOffset += TOOLBARHEIGHT;
              toolbarHeight += TOOLBARHEIGHT;
            }
           b->move (rightOffset, yOffset);
         }
        else // Not right
         {
           int myWidth;
           if (isItemAutoSized(b) == TRUE)
            {
              autoSize = b;
              myWidth = 100; // Min width for autosized
            }
           else
             myWidth = b->width();
           if (offset > (rightOffset-myWidth+3))
            {
              offset =3+4+9;
              yOffset += TOOLBARHEIGHT;
              toolbarHeight += TOOLBARHEIGHT;
            }
           b->move( offset, yOffset );
           offset+=myWidth+3;
         }
      }
     else // Not fullwidth, autosize
      {
        if (offset > (maxwidth-b->width()-3))
         {
           offset = 3+9+4;
           yOffset += TOOLBARHEIGHT;
           toolbarHeight += TOOLBARHEIGHT;
           b->move( offset, yOffset );
           offset += b->width()+3;
         }
        else
         {
           b->move( offset, yOffset );
           offset += b->width()+3;
           if (offset > toolbarWidth)
             toolbarWidth = offset;
         }
      }
   }
  if (autoSize != 0)
    autoSize->resize(rightOffset - autoSize->x()-1, autoSize->height() );
  //if (position == Floating)
  //  toolbarWidth++;
}

void KToolBar::layoutVertical ()
{
  int offset=3+9+4;
  int yOffset=3;
  int widest;

  toolbarHeight = offset;

  toolbarWidth= TOOLBARHEIGHT;
  widest =  TOOLBARHEIGHT;
  /*
   I have (had) ten thousand problems here. When toplevel shrinked (vert.) it's ok.
   But when we enlarge toplevel (vert.), it first sets up toolbars and then view.
   do we don't know that toplevel is larger than before.
   Solution: setMaxHeight (int h) is public member here. Call it from your toplevel
   AFTER you have managed Top and Bottom toolbars, menus, statusbars...  with max free height
   */

  for ( KToolBarItem *b = items.first(); b!=NULL; b=items.next() )
   {
     if (offset > ((max_height)-(b->height()+3)))
      {
        offset = 4+9+3;
        yOffset += widest;
        toolbarWidth += TOOLBARHEIGHT;
        b->move( yOffset, offset );
        if (isItemAutoSized(b) == TRUE)
          b->resize ((widest>100)?widest:100, b->height());
        if ((yOffset + b->width()+3) > toolbarWidth) // is item wider than we are
           toolbarWidth = b->width() +yOffset+2;
        if ((b->width() +3) > widest)
          widest =(b->width() +3);
        offset += b->height()+3;
      }
     else
      {
        b->move( yOffset, offset );
        if (isItemAutoSized(b) == TRUE)
             b->resize ((widest>100)?widest:100, b->height());
        if ((yOffset + b->width()+3) > toolbarWidth) // is item wider than we are
           toolbarWidth = b->width() +yOffset+2;
        if ((b->width() +3) > widest)
          widest =(b->width() +3);
        offset += b->height()+3;
        if (offset > toolbarHeight)
          toolbarHeight = offset;
      }
   }
  //if (position == Floating)
  //  toolbarHeight++;
}

void KToolBar::updateRects( bool res )
{
  switch ( position )
   {
    case Top:
    case Bottom:
      layoutHorizontal ();
      break;
    case Floating:
      max_height = height();
      if (width() <= height ()-20)
        layoutVertical ();
      else if(width() >= height ()+20)
        layoutHorizontal ();
      else
        layoutHorizontal ();
      break;

    case Left:
    case Right:
      layoutVertical ();
      break;
   }

  if (res == TRUE)
    resize (toolbarWidth, toolbarHeight);
  //else
  //repaint();
}

void KToolBar::mousePressEvent ( QMouseEvent *m )
{
  pointerOffset = m->pos();
  if ( moving && m->button() != LeftButton)
    context->popup( mapToGlobal( m->pos() ), 0 );
  else
    grabMouse(sizeAllCursor);
}

void KToolBar::resizeEvent( QResizeEvent *e )
{

  if (position == Floating)
   {
     updateRects (FALSE);
     if (e->size().width() != toolbarWidth ||
         e->size().height() != toolbarHeight)
      {
        resize (toolbarWidth, toolbarHeight);
      }
   }
}

void KToolBar::paintEvent(QPaintEvent *)
{
  //  int offset=3;
  
  //MD Lots of rewrite
  int stipple_height;
  
  QColorGroup g = QWidget::colorGroup();
  QPainter *paint = new QPainter();
  paint->begin( this );

  // Handle point
  switch ( position ) {
    
		case Top:
    case Bottom:
      
			qDrawShadePanel( paint, 0, 0, 9, toolbarHeight,
                			 g , FALSE, 1);

			paint->setPen( g.light() );
			paint->drawLine( 9, 0, 9, toolbarHeight);
      stipple_height = 3;
      while ( stipple_height < toolbarHeight-4 ) {
				paint->drawPoint( 1, stipple_height+1);
				paint->drawPoint( 4, stipple_height);
				stipple_height+=3;
			}
			paint->setPen( g.dark() );
      stipple_height = 4;
      while ( stipple_height < toolbarHeight-4 ) {
				paint->drawPoint( 2, stipple_height+1);
				paint->drawPoint( 5, stipple_height);
				stipple_height+=3;
			}

			break;

    case Left:
    case Right:
      
			qDrawShadePanel( paint, 0, 0, toolbarWidth, 9,
              			 		g , FALSE, 1);

			paint->setPen( g.light() );
			paint->drawLine( 0, 9, toolbarWidth, 9);
      stipple_height = 3;
      while ( stipple_height < toolbarWidth-4 ) {
				paint->drawPoint( stipple_height+1, 1);
				paint->drawPoint( stipple_height, 4 );
				stipple_height+=3;
			}
			paint->setPen( g.dark() );
      stipple_height = 4;
      while ( stipple_height < toolbarWidth-4 ) {
				paint->drawPoint( stipple_height+1, 2 );
				paint->drawPoint( stipple_height, 5);
				stipple_height+=3;
			}
			
      break;
    
		case Floating:
		
			if (width() <= height()-20) {
			
				qDrawShadePanel( paint, 0, 0, toolbarWidth, 9,
              			 		g , FALSE, 1);
				paint->setPen( g.light() );
				paint->drawLine( 0, 9, toolbarWidth, 9);
      	stipple_height = 3;
      	while ( stipple_height < toolbarWidth-4 ) {
					paint->drawPoint( stipple_height+1, 1);
					paint->drawPoint( stipple_height, 4 );
					stipple_height+=3;
				}
				paint->setPen( g.dark() );
      	stipple_height = 4;
      	while ( stipple_height < toolbarWidth-4 ) {
					paint->drawPoint( stipple_height+1, 2 );
					paint->drawPoint( stipple_height, 5);
					stipple_height+=3;
				}	
			} else {
			
				qDrawShadePanel( paint, 0, 0, 9, toolbarHeight,
                			 g , FALSE, 1);

				paint->setPen( g.light() );
				paint->drawLine( 9, 0, 9, toolbarHeight);
      	stipple_height = 3;
      	while ( stipple_height < toolbarHeight-4 ) {
					paint->drawPoint( 1, stipple_height+1);
					paint->drawPoint( 4, stipple_height);
					stipple_height+=3;
				}
				paint->setPen( g.dark() );
      	stipple_height = 4;
      	while ( stipple_height < toolbarHeight-4 ) {
					paint->drawPoint( 2, stipple_height+1);
					paint->drawPoint( 5, stipple_height);
					stipple_height+=3;
				}		
			}
		
		  break;
   }

//	MD I left this in to show what it was like before
//	
//   	 if (width() <= height()-20)
//       {
//         qDrawShadePanel( paint, 3, offset, toolbarWidth-8, 4,
//                          g , FALSE, 2);
//         offset+=4;
//         qDrawShadePanel( paint, 3, offset,  toolbarWidth-8, 4,
//                          g , FALSE, 2);
//         offset+=9;
//       }
//  	else if(width() >= height ()+20)
//  	 {
//  	   qDrawShadePanel( paint, offset, 3, 4, toolbarHeight-8,
//  						g , FALSE, 2);
//  	   offset+=4;
//  	   qDrawShadePanel( paint, offset, 3, 4, toolbarHeight-8,
//  						g , FALSE, 2);
//  	   offset+=9;
//  	 }
//  	else
//  	 {
//  	   qDrawShadePanel( paint, offset, 3, 4, toolbarHeight-8,
//  						g , FALSE, 2);
//  	   offset+=4;
//  	   qDrawShadePanel( paint, offset, 3, 4, toolbarHeight-8,
//  						g , FALSE, 2);
//  	   offset+=9;
//  	 }

// Separators
//  for ( KToolBarItem *b = items.first(); b!=NULL; b=items.next() )
//   {
//     switch ( position )
//      {
//       case Top:
//       case Bottom:
//         if ( getID(b) == -1 )   // is a separator
//           qDrawShadePanel(paint, ((KToolBarButton *) b)->x()+2,
//                           ((KToolBarButton *) b)->y(), 2, TOOLBARHEIGHT-4,
//                             g , TRUE, 1);
//         break;
//       case Left:
//       case Right:
//         if ( getID(b) == -1 )  // is a separator
//           qDrawShadePanel(paint, ((KToolBarButton *) b)->x(),
//                           ((KToolBarButton *) b)->y()+2, TOOLBARHEIGHT-4, 2,
//                             g , TRUE, 1);
//         break;
//       case Floating:
//         if (width() <= height()-20)
//          {
//            if ( getID(b) == -1 )  // is a separator
//              qDrawShadePanel(paint, ((KToolBarButton *) b)->x(),
//                              ((KToolBarButton *) b)->y()+2, TOOLBARHEIGHT-4, 2,
//                              g , TRUE, 1);
//          }
//         else if(width() >= height ()+20)
//          {
//            if ( getID(b) == -1 )   // is a separator
//              qDrawShadePanel(paint, ((KToolBarButton *) b)->x()+2,
//                              ((KToolBarButton *) b)->y(), 2, TOOLBARHEIGHT-4,
//                              g , TRUE, 1);
//          }
//         else
//          {
//            if ( getID(b) == -1 )   // is a separator
//              qDrawShadePanel(paint, ((KToolBarButton *) b)->x()+2,
//                              ((KToolBarButton *) b)->y(), 2, TOOLBARHEIGHT-4,
//                              g , TRUE, 1);
//          }
//         break;
//      }
//   }

#ifdef TOOLBAR_IS_RAISED

  if (position != Floating)
    qDrawShadePanel(paint, 0, 0, width(), height(), g , FALSE, 1);
	
#else

  if (position != Floating)
    qDrawShadeRect(paint, 0, 0, width(), height(), g , TRUE, 1);
		
#endif

  paint->end();
  delete paint;
	
}

int KToolBar::getID(QWidget *w)
{
  switch (getType (w))
   {
    case ITEM_LINED:
      return ((KToolBarLined *) w)->ID();
      break;
    case ITEM_BUTTON:
      return ((KToolBarButton *) w)->ID();
      break;
    case ITEM_COMBO:
      return ((KToolBarCombo *) w)->ID();
      break;
    case ITEM_FRAME:
      return ((KToolBarFrame *) w)->ID();
      break;
   }
  return 0;
}

bool KToolBar::isItemAutoSized(QWidget *w)
{
  switch (getType (w))
   {
    case ITEM_LINED:
      return ((KToolBarLined *) w)->isAuto();
      break;
    case ITEM_BUTTON:
      return FALSE;;
      break;
    case ITEM_COMBO:
      return ((KToolBarCombo *) w)->isAuto();
      break;
    case ITEM_FRAME:
      return ((KToolBarFrame *) w)->isAuto();
      break;  
   }
  return FALSE;
}

int KToolBar::getType(QWidget *w)
{
  if (w->isA("KToolBarLined"))
    return ITEM_LINED;
  if (w->isA("KToolBarButton"))
    return ITEM_BUTTON;
  if (w->isA("KToolBarCombo"))
    return ITEM_COMBO;
  if (w->isA("KToolBarFrame"))
    return ITEM_FRAME;
  return 0;
}

bool KToolBar::isItemRight (QWidget *w)
{
  switch (getType (w))
   {
    case ITEM_LINED:
      return ((KToolBarLined *) w)->isRight();
      break;
    case ITEM_BUTTON:
      return ((KToolBarButton *) w)->isRight();
      break;
    case ITEM_COMBO:
      return ((KToolBarCombo *) w)->isRight();
      break;
    case ITEM_FRAME:
      return ((KToolBarFrame *) w)->isRight();
      break;
   }
  return FALSE;
}

void KToolBar::closeEvent (QCloseEvent *e)
{
  if (position == Floating)
   {
     position = lastPosition;
     recreate (Parent, oldWFlags, QPoint (oldX, oldY), TRUE);
     context->changeItem (klocale->translate("Float"), CONTEXT_FLOAT);
     emit moved (position);
     e->ignore();
     return;
   }
  e->accept();
}


void KToolBar::ButtonClicked( int id )
{
  emit clicked( id );
}

void KToolBar::ButtonPressed( int id )
{
  emit pressed( id );
}
      items.remove();
void KToolBar::ButtonReleased( int id )
{
  emit released( id );
}

 /********************\
 *                    *
 * I N T E R F A C E  *
 *                    *
 \********************/

/***** BUTTONS *****/

/// Inserts a button.
int KToolBar::insertButton( const QPixmap& pixmap, int id, bool enabled,
			    const char *tooltiptext, int index )
{
  KToolBarButton *button = new KToolBarButton( pixmap, id, this );
  if ( index == -1 )
    items.append( button );
  else
    items.insert( index, button );
  
  connect(button, SIGNAL(clicked(int)), this, SLOT(ButtonClicked(int)));
  connect(button, SIGNAL(released(int)), this, SLOT(ButtonReleased(int)));
  connect(button, SIGNAL(pressed(int)), this, SLOT(ButtonPressed(int)));
  button->enable( enabled );

  if ( tooltiptext )
    QToolTip::add( button, tooltiptext );
  button->show();
  updateRects( TRUE );
  return items.at();
}

/// Inserts a button with connection.

int KToolBar::insertButton( const QPixmap& pixmap, int id, const char *signal,
			    const QObject *receiver, const char *slot, bool enabled,
			    const char *tooltiptext, int index )
{
  KToolBarButton *button = new KToolBarButton( pixmap, id, this );
  if ( index == -1 ) 
    items.append( button );
  else
    items.insert( index, button );
		
  if ( tooltiptext )
    QToolTip::add( button, tooltiptext );
		
  connect( button, signal, receiver, slot );
  button->enable( enabled );
  button->show();
  updateRects( TRUE );
  // printf("insertButton %d\n",items.at());
  return items.at();
}

/********* SEPARATOR *********/
/// Inserts separator

int KToolBar::insertSeparator( int index )
{
  KToolBarButton *separ = new KToolBarButton( this );
  if ( index == -1 )
    items.append( separ );
  else
    items.insert( index, separ );
	
  updateRects( TRUE );
  return items.at();
}

/********* Frame **********/
/// inserts QFrame

int KToolBar::insertFrame (int _id, int _size, int _index)
{
  KToolBarFrame *frame = new KToolBarFrame (_id, this);
  if (_index == -1)
    items.append (frame);
  else
    items.insert(_index, frame);
  frame -> resize (_size, 24);
  frame->show();
  updateRects(TRUE);
  return items.at();
}  

/************** LINE EDITOR **************/
/// Inserts a KLined. KLined is derived from QLineEdit and has another signal, tabPressed,
//  for completions.

int KToolBar::insertLined(const char *text, int id, const char *signal,
			  const QObject *receiver, const char *slot,
			  bool enabled, const char *tooltiptext, int size, int index)
{
  KToolBarLined *lined = new KToolBarLined (text, id, this);
  if (index == -1)
    items.append (lined);
  else
    items.insert(index, lined);
  if (tooltiptext)
    QToolTip::add( lined, tooltiptext );
  connect( lined, signal, receiver, slot );
  lined->resize(size, 24);
  lined->enable(enabled);
  lined->show();
  updateRects(TRUE);
  return items.at();
}

/************** COMBO BOX **************/
/// Inserts comboBox with QStrList

int KToolBar::insertCombo (QStrList *list, int id, bool writable,
                           const char *signal, QObject *receiver,
                           const char *slot, bool enabled,
                           const char *tooltiptext,
                           int size, int index,
                           KCombo::Policy policy)
{
  KToolBarCombo *combo = new KToolBarCombo (writable, id, this);
  if (index == -1)
    items.append (combo);
  else
    items.insert (index, combo);
  combo->insertStrList (list);
  combo->setInsertionPolicy(policy);
  if (tooltiptext)
    QToolTip::add( combo, tooltiptext );
  connect ( combo, signal, receiver, slot );
  combo->setAutoResize(TRUE);
  combo->resize(size, TOOLBARHEIGHT);
  combo->enable(enabled);
  combo->show();
  updateRects(TRUE);
  return items.at();
}


/// Inserts combo with text

int KToolBar::insertCombo (const char *text, int id, bool writable,
                           const char *signal, QObject *receiver,
                           const char *slot, bool enabled,
                           const char *tooltiptext, int size, int index,
                           KCombo::Policy policy)
{
  KToolBarCombo *combo = new KToolBarCombo (writable, id, this);
  if (index == -1)
    items.append (combo);
  else
    items.insert (index, combo);
  combo->insertItem (text);
  combo->setInsertionPolicy(policy);
  if (tooltiptext)
    QToolTip::add( combo, tooltiptext );
  connect (combo, signal, receiver, slot);
  combo->resize(size, TOOLBARHEIGHT);
  combo->enable(enabled);
  combo->show();
  updateRects(TRUE);
  return items.at();
}

/// Removes item by ID

void KToolBar::removeItem (int id)
{
  for (KToolBarItem *b = items.first(); b!=NULL; b=items.next())
    if (getID(b) == id )
     {
       items.remove();
     }
  //updateRects(TRUE);
  emit moved (position);
}

/// ******** Tools

/// misc
void KToolBar::addConnection (int id, const char *signal,
                              const QObject *receiver, const char *slot)
{
  for (KToolBarItem *b = items.first(); b!=NULL; b=items.next())
    if (getID(b) == id )
     {
       switch(getType(b))
        {
         case ITEM_BUTTON:
           connect (((KToolBarButton *) b), signal, receiver, slot);
           break;
         case ITEM_LINED:
           connect (((KToolBarLined *) b), signal, receiver, slot);
           break;
         case ITEM_COMBO:
           connect (((KToolBarCombo *) b), signal, receiver, slot);
           break;
        }
     }
}

KToolBarFrame *KToolBar::getFrame (int id)
{
  for (KToolBarItem *b = items.first(); b!=NULL; b=items.next())
    if (getID(b) == id )
      if (getType(b) == ITEM_FRAME)
        return ((KToolBarFrame *) b);
  return 0;
}

/// Common
void KToolBar::setItemEnabled( int id, bool enabled )
{
  for (KToolBarItem *b = items.first(); b!=NULL; b=items.next())
    if (getID(b) == id )
     {
       switch(getType(b))
        {
         case ITEM_BUTTON:
           ((KToolBarButton *) b)->enable( enabled );
           break;

         case ITEM_LINED:
           ((KToolBarLined *) b)->enable( enabled );
           break;

         case ITEM_COMBO:
           ((KToolBarCombo *) b)->enable( enabled );
           break;
        }
     }
}

void KToolBar::setItemAutoSized ( int id, bool enabled )
{
  for (KToolBarItem *b = items.first(); b!=NULL; b=items.next())
    if (getID(b) == id )
     {
       switch(getType(b))
        {
         case ITEM_BUTTON:
           return;
           break;

         case ITEM_LINED:
           ((KToolBarLined *) b)->autoSize( enabled );
           break;

         case ITEM_FRAME:
           ((KToolBarFrame *) b)->autoSize( enabled );
           break;

         case ITEM_COMBO:
           ((KToolBarCombo *) b)->autoSize( enabled );
           break;
        }
     }
  updateRects(TRUE);
}

void KToolBar::alignItemRight(int id, bool yes)
{
  for (KToolBarItem *b = items.first(); b!=NULL; b=items.next())
    if (getID(b) == id )
     {
       switch(getType(b))
        {
         case ITEM_BUTTON:
           ((KToolBarButton *) b)->alignRight (yes);
           break;

         case ITEM_LINED:
           ((KToolBarLined *) b)->alignRight (yes);
           break;

         case ITEM_FRAME:
           ((KToolBarFrame *) b)->alignRight (yes);
           break;
           
         case ITEM_COMBO:
           ((KToolBarCombo *) b)->alignRight (yes);
           break;
        }
     }
  updateRects(TRUE);
}

/// Butoons
void KToolBar::setButtonPixmap( int id, const QPixmap& _pixmap )
{
  for (KToolBarItem *b = items.first(); b!=NULL; b=items.next())
    if (getID(b) == id )
      if (getType(b) == ITEM_BUTTON)
	((KToolBarButton *) b)->setPixmap( _pixmap ); 
}

/// Toggle buttons

void KToolBar::setToggle ( int id, bool yes )
{
  for (KToolBarItem *b = items.first(); b!=NULL; b=items.next())
    if (getID(b) == id )
      if (getType(b) == ITEM_BUTTON)
         ((KToolBarButton *) b)->beToggle(yes);
}

void KToolBar::toggleButton (int id)
{
  for (KToolBarItem *b = items.first(); b!=NULL; b=items.next())
    if (getID(b) == id )
      if (getType(b) == ITEM_BUTTON)
       {
         if (((KToolBarButton *) b)->isToggleButton() == TRUE)
           ((KToolBarButton *) b)->toggle();
       }
}

void KToolBar::setButton (int id, bool on)
{
  for (KToolBarItem *b = items.first(); b!=NULL; b=items.next())
    if (getID(b) == id )
      if (getType(b) == ITEM_BUTTON)
       {
         if (((KToolBarButton *) b)->isToggleButton() == TRUE)
           ((KToolBarButton *) b)->on(on);
       }
}

bool KToolBar::isButtonOn (int id)
{
  for (KToolBarItem *b = items.first(); b!=NULL; b=items.next())
    if (getID(b) == id )
      if (getType(b) == ITEM_BUTTON)
       {
         if (((KToolBarButton *) b)->isToggleButton() == TRUE)
           return ((KToolBarButton *) b)->isOn();
       }
  return FALSE;
}

/// Lined
void KToolBar::setLinedText (int id, const char *text)
{
  for (KToolBarItem *b = items.first(); b!=NULL; b=items.next())
    if (getID(b) == id )
      if (getType(b) == ITEM_LINED)
       {
         ((KToolBarLined *) b)->setText(text);
         ((KToolBarLined *) b)->cursorAtEnd();
       }
}

const char *KToolBar::getLinedText (int id )
{
  for (KToolBarItem *b = items.first(); b!=NULL; b=items.next())
    if (getID(b) == id )
      if (getType(b) == ITEM_LINED)
	return ((KToolBarLined *) b)->text();
  return 0;
}

/// Combos
void KToolBar::insertComboItem (int id, const char *text, int index)
{
  for (KToolBarItem *b = items.first(); b!=NULL; b=items.next())
    if (getID(b) == id )
      if (getType(b) == ITEM_COMBO)
       {
         ((KToolBarCombo *) b)->insertItem(text, index);
         ((KToolBarCombo *) b)->cursorAtEnd();
       }
}

void KToolBar::insertComboList (int id, QStrList *list, int index)
{
  for (KToolBarItem *b = items.first(); b!=NULL; b=items.next())
    if (getID(b) == id )
      if (getType(b) == ITEM_COMBO)
	((KToolBarCombo *) b)->insertStrList(list, index);
}

void KToolBar::setCurrentComboItem (int id, int index)
{
  for (KToolBarItem *b = items.first(); b!=NULL; b=items.next())
    if (getID(b) == id )
      if (getType(b) == ITEM_COMBO)
       {
         ((KToolBarCombo *) b)->setCurrentItem(index);
         ((KToolBarCombo *) b)->cursorAtEnd();
       }
}

void KToolBar::removeComboItem (int id, int index)
{
  for (KToolBarItem *b = items.first(); b!=NULL; b=items.next())
    if (getID(b) == id )
      if (getType(b) == ITEM_COMBO)
	((KToolBarCombo *) b)->removeItem(index);
}

void KToolBar::changeComboItem  (int id, const char *text, int index)
{
  for (KToolBarItem *b = items.first(); b!=NULL; b=items.next())
    if (getID(b) == id )
      if (getType(b) == ITEM_COMBO)
       {
         if (index == -1)
          {
            index = ((KToolBarCombo *) b)->currentItem();
            ((KToolBarCombo *) b)->changeItem(text, index);
          }
         else
          {
            ((KToolBarCombo *) b)->changeItem(text, index);
          }
         ((KToolBarCombo *) b)->cursorAtEnd();
       }
}

void KToolBar::clearCombo (int id)
{
  for (KToolBarItem *b = items.first(); b!=NULL; b=items.next())
    if (getID(b) == id )
      if (getType(b) == ITEM_COMBO)
	((KToolBarCombo *) b)->clear();
}

const char *KToolBar::getComboItem (int id, int index)
{
  for (KToolBarItem *b = items.first(); b!=NULL; b=items.next())
    if (getID(b) == id )
      if (getType(b) == ITEM_COMBO)
       {
        if (index == -1)
          index = ((KToolBarCombo *) b)->currentItem();
        return ((KToolBarCombo *) b)->text(index);
       }
  return 0;
}

KToolBarCombo *KToolBar::getCombo (int id)
{
  for (KToolBarItem *b = items.first(); b!=NULL; b=items.next())
    if (getID(b) == id )
      if (getType(b) == ITEM_COMBO)
        return ((KToolBarCombo *) b);
  return 0;
}
/// Toolbar itself

void KToolBar::setFullWidth(bool flag)
{
  fullWidth = flag;
}

void KToolBar::enableMoving(bool flag)
{
  moving = flag; 
}

void KToolBar::setBarPos(BarPosition bpos)
{
  if (position != bpos)
   {
     if (bpos == Floating)
      {
        lastPosition = position;
        position = bpos;
        oldX = x();
        oldY = y();
        oldWFlags = getWFlags();
	QPoint p = mapToGlobal(QPoint(0,0));
	parentOffset = pos();
        hide();
        recreate(0, 0,
                 p, FALSE);
	XSetTransientForHint( qt_xdisplay(), winId(), Parent->topLevelWidget()->winId());
	KWM::setDecoration(winId(), 2);
	KWM::moveToDesktop(winId(), KWM::desktop(Parent->winId()));
	setCaption(""); // this triggers a qt bug
	if (title){
	  setCaption(title);
	}
	else {
	  QString s = Parent->caption();
	  s.append(" [tools]");
	  setCaption(s);
	}
        updateRects (TRUE);
        //show();
        context->changeItem (klocale->translate("UnFloat"), CONTEXT_FLOAT);
        emit moved (bpos);
        return;
      }
     else if (position == Floating) // was floating
      {
        position = bpos;
        hide();
        recreate(Parent, oldWFlags, QPoint(oldX, oldY), TRUE);
        updateRects (TRUE);
        context->changeItem (klocale->translate("Float"), CONTEXT_FLOAT);
        emit moved (bpos);
        return;
      }
     else
      {
        position = bpos;
        emit moved ( bpos );
        return;
      }
   }
}

void KToolBar::enableFloating (bool arrrrrrgh)
{
  context->setItemEnabled (CONTEXT_FLOAT, arrrrrrgh);
}


bool KToolBar::enable(BarStatus stat)
{
  bool mystat = isVisible();

  if ( (stat == Toggle && mystat) || stat == Hide )
   {
     if (position == Floating)
       iconify();
     else
       hide();
   }
  else
    show();

  return ( isVisible() == mystat );
}

/*********************************************************/
/***********    O L D     I N T E R F A C E   ************/
/*********************************************************/

int KToolBar::insertItem(const QPixmap& _pixmap, int _ID, bool enabled,
               char *ToolTipText, int index)

{
  warning ("KToolBar: insertItem is obsolete. Use insertButton");
  return (this->insertButton (_pixmap, _ID, enabled, ToolTipText, index));
}

int KToolBar::insertItem(const QPixmap& _pixmap, int _ID, const char *signal,
               const QObject *receiver, const char *slot,
               bool enabled,
               char *tooltiptext, int index)
{
  warning ("KToolBar: insertItem is obsolete. Use InsertButton");
  return (this->insertButton (_pixmap, _ID, signal, receiver, slot, enabled, tooltiptext, index));
}

void KToolBar::setItemPixmap( int _id, const QPixmap& _pixmap )
{
  warning ("KToolBar: setItemPixmap is obsolete. Use setButtonPixmap");
  this->setButtonPixmap (_id, _pixmap);
}

void KToolBar::mouseMoveEvent(QMouseEvent* /* m */){
  if (!moving || mouseGrabber() != this)
    return;
  if (position != Floating){
    QPoint p = mapFromGlobal(QCursor::pos()) - pointerOffset;
    if (p.x()*p.x()+p.y()*p.y()<169)
      return;
    
    XUngrabPointer( qt_xdisplay(), CurrentTime );
    setBarPos(Floating);
    show();
    QApplication::syncX();
    while(XGrabPointer( qt_xdisplay(), winId(), TRUE,
			ButtonPressMask | ButtonReleaseMask |
			PointerMotionMask | EnterWindowMask | LeaveWindowMask,
			GrabModeAsync, GrabModeAsync,
			None, sizeAllCursor.handle(), 
			CurrentTime ) != GrabSuccess);
    grabMouse(sizeAllCursor);
  }
  move(QCursor::pos() - pointerOffset);    
  QPoint p = QCursor::pos() - pointerOffset - (Parent->mapToGlobal(QPoint(0,0)) + parentOffset);

  if (p.x()*p.x()+p.y()*p.y()<169){
    releaseMouse();
    setBarPos(lastPosition);
    QApplication::syncX();
    while(XGrabPointer( qt_xdisplay(), winId(), TRUE,
			ButtonPressMask | ButtonReleaseMask |
			PointerMotionMask | EnterWindowMask | LeaveWindowMask,
			GrabModeAsync, GrabModeAsync,
			None, sizeAllCursor.handle(), 
			CurrentTime ) != GrabSuccess);
    grabMouse(sizeAllCursor);
  }
}

void KToolBar::mouseReleaseEvent ( QMouseEvent * ){
  releaseMouse();
}



// sven
      ++it;
    }
  }
}

#include "ktoolbar.moc"

