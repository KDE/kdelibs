/* This file is part of the KDE libraries
    Copyright (C) 1997, 1998 Stephan Kulow (coolo@kde.org)
              (C) 1997, 1998 Mark Donohoe (donohoe@kde.org)
              (C) 1997, 1998 Sven Radej (sven@lisa.exp.univie.ac.at)
              (C) 1997, 1998 Matthias Ettrich (ettrich@kde.org)
              
    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

// CHANGES:
// Solved one-button problem and added handle-highlighting - sven 5.1.1998
// Some fixes of yesterday fixes - sven 6.1. 1998
// Fixing of fixes - one button, three buttons + separator,
// Improved engine, no handles if not movable. - sven 22. 1. 1998.
// Matthias - setting size of toolbar (bin incompat) 22.1.98
// Merged sven's & Matthias' changes 23-25. 1.98
// Fixed small KWM-close bug (reported by Coolo)  sven 8.2.1998
// boolett proofing by Marcin Dalecki 06.03.1998
// BINARY INCOMPATIBLE CHANGES sven 19.3. 1998:
//  - optional icons+text, variable size  with reading options from
//    from ~/.kderc
//  - Fixing of bug reported by Antonio Larrosa (Thanks!)
//  - Optional highlighting of buttons
//  - added sizeHint()
//  - sample (default) ~./kderc group for toolbar:
//      [Toolbar style]
//      IconText=0        0=icons, 1=icons+text
//      Highlighting=1    0=No, 1=yes
//      Size=26           height of one row in toolbar

//-------------------------------------------------------------------------

#include <qpainter.h>
#include <qtooltip.h> 
#include <qdrawutl.h>
#include <qpalette.h>
#include <qbitmap.h>
#include <qstring.h>

#include <qlist.h>
#include <qpainter.h>
#include <qpalette.h>
#include <qbitmap.h>
#include <qstring.h>
#include <qframe.h>
#include <qbutton.h>
//#include "kbutton.h"
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
// Use enums instead of defines. We are C++ and NOT C !
enum {
    CONTEXT_LEFT = 0,
    CONTEXT_RIGHT = 1,
    CONTEXT_TOP = 2,
    CONTEXT_BOTTOM = 3,
    CONTEXT_FLOAT = 4
};
    if ( !isEnabled() )
enum {
    ITEM_LINED = 0,
    ITEM_BUTTON = 1,
    ITEM_COMBO = 2,
    ITEM_FRAME = 3,
    ITEM_TOGGLE = 4,
};


#define KToolBarItem QWidget

KToolBarFrame::KToolBarFrame (int ID, QWidget *parent,
                              const char *name) : QFrame (parent, name)
{
  id = ID;
  right = false;
}

KToolBarCombo::KToolBarCombo(bool rw, int ID,
                             QWidget *parent, const char *name) : KCombo (rw, parent, name),
     id(ID), right(false), autoSized(false)
{ 
}

void KToolBarCombo::enable (bool enable)
{
  QWidget::setEnabled (enable);
}


KToolBarLined::KToolBarLined(const char *text, int ID,
                             QWidget *parent, const char *name) : KLined (parent, name)
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

//------------------------------Buttons
KToolBarButton::KToolBarButton( const QPixmap& pixmap, int ID,
                                QWidget *_parent, const char *name,
                                int item_size, const char *txt) : QButton( _parent, name )
{
  parentWidget = (KToolBar *) _parent;
  raised = false;
  setFocusPolicy( NoFocus );
  id = ID;
  if (txt)
  btext=txt;
  modeChange ();
  if ( ! pixmap.isNull() )
    enabledPixmap = pixmap;
  else
    {
      warning(klocale->translate("KToolBarButton: pixmap is empty, perhaps some missing file"));
      enabledPixmap.resize( item_size-4, item_size-4);
    }
  makeDisabledPixmap();
  setPixmap( enabledPixmap );
  connect (parentWidget, SIGNAL( modechange() ), this, SLOT( modeChange() ));
  
  connect( this, SIGNAL( clicked() ), this, SLOT( ButtonClicked() ) );
  connect(this, SIGNAL( pressed() ), this, SLOT( ButtonPressed() ) );
  connect(this, SIGNAL( released() ), this, SLOT( ButtonReleased() ) );
  right = false;
}

void KToolBarButton::leaveEvent(QEvent *)
{
  if (isToggleButton() == false)
    if( raised != false )
    {
      raised = false;
      repaint();
    }
}

void KToolBarButton::enterEvent(QEvent *)
{
  if (highlight == 1)
    if (isToggleButton() == false)
      if ( isEnabled() )
      {
        raised = true;
        repaint(false);
      }
}

void KToolBarButton::beToggle(bool flag)
{
  setToggleButton(flag);
  if (flag == true)
    connect (this, SIGNAL(toggled(bool)), this, SLOT(ButtonToggled()));
  else
    disconnect (this, SIGNAL(toggled(bool)), this, SLOT(ButtonToggled()));
}

void KToolBarButton::on(bool flag)
{
  if(isToggleButton() == true)
    setOn(flag);
  else
  {
    setDown(flag);
    leaveEvent((QEvent *) 0);
  }
  repaint();
}
  init();
void KToolBarButton::drawButton( QPainter *_painter )
{
  /* Don't ask why, but the widget works without it prefectly! */
  //  QColorGroup g = QWidget::colorGroup();
  //
  //  if (isOn())
  //    qDrawShadePanel(_painter, 0, 0, width(), height(), g , true, 2);

  // We are not using KButton any more!
  if ( isDown() || isOn() )
  {
    if ( style() == WindowsStyle )
      qDrawWinButton(_painter, 0, 0, width(), height(), colorGroup(), true );
    else
      qDrawShadePanel(_painter, 0, 0, width(), height(), colorGroup(), true, 2, 0L );
  }
  else if ( raised )
  {
    if ( style() == WindowsStyle )
      qDrawWinButton( _painter, 0, 0, width(), height(), colorGroup(), false );
    else
      qDrawShadePanel( _painter, 0, 0, width(), height(), colorGroup(), false, 2, 0L );
  }

  int dx, dy;
  
  if ( pixmap() )
    if (!icontext)
    {
      dx = ( width() - pixmap()->width() ) / 2;
      dy = ( height() - pixmap()->height() ) / 2;
      if ( isDown() && style() == WindowsStyle ) {
        ++dx;
        ++dy;
      }
      _painter->drawPixmap( dx, dy, *pixmap() );
    }
    else
    {
      dx = 1;
      dy = ( height() - pixmap()->height() ) / 2;
      if ( isDown() && style() == WindowsStyle ) {
        ++dx;
        ++dy;
      }
      _painter->drawPixmap( dx, dy, *pixmap() );

      int tf = AlignVCenter|AlignLeft;
      if (!isEnabled())
        tf |= GrayText;
      dx= pixmap()->width();
      _painter->setFont(buttonFont);
      if(raised)
        _painter->setPen(blue);
      _painter->drawText(dx, 0, width()-dx, height(), tf, btext);
    }
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
  else {
    warning(klocale->translate("KToolBarButton: pixmap is empty, perhaps some missing file"));
    enabledPixmap.resize(width()-4, height()-4);
  }
  QButton::setPixmap( enabledPixmap );
}            

void KToolBarButton::makeDisabledPixmap()
{
  QPalette pal = palette();
  QColorGroup g = pal.disabled();

  // Prepare the disabledPixmap for drawing
  
  disabledPixmap.detach(); // prevent flicker
  disabledPixmap.resize(enabledPixmap.width(), enabledPixmap.height());
  disabledPixmap.fill( g.background() );
  const QBitmap *mask = enabledPixmap.mask();
  bool allocated = false;
  if (!mask) {// This shouldn't occur anymore!
    mask = new QBitmap(enabledPixmap.createHeuristicMask());
    allocated = true;
  } 
  
  QBitmap bitmap = *mask; // YES! make a DEEP copy before setting the mask!   
  bitmap.setMask(*mask);
  
  QPainter p;
  p.begin( &disabledPixmap );
  p.setPen( g.light() );
  p.drawPixmap(1, 1, bitmap);
  p.setPen( g.mid() );
  p.drawPixmap(0, 0, bitmap);
  p.end();
  
  if (allocated) // This shouldn't occur anymore!
    delete mask;
}
  icontext=config->readNumEntry("IconText", 0);
void KToolBarButton::paletteChange(const QPalette &)
{
  if( ID() != -1 )  {
    makeDisabledPixmap();
    if ( !isEnabled() ) 
      QButton::setPixmap( disabledPixmap );
    repaint(false); // no need to delete it first therefore only false
  }
}
  bool doUpdate=false;

void KToolBarButton::modeChange()
{
  buttonFont.setFamily("Helvetica");
  buttonFont.setPointSize(10);
  buttonFont.setBold(false);
  buttonFont.setItalic(false);
  buttonFont.setCharSet(font().charSet());
  
  QFontMetrics fm(buttonFont);
  
  icontext=parentWidget->icon_text;
  _size=parentWidget->item_size;
  highlight=parentWidget->highlight;
  if (icontext) //Calculate my size
  {
    QToolTip::remove(this);
    resize (fm.width(btext)+_size, _size-2); // +2+_size-2
  }
  else
  {
    QToolTip::remove(this);
    QToolTip::add(this, btext);
    resize (_size-2, _size-2);
  }
}
  int rightOffset;
KToolBarButton::KToolBarButton( QWidget *parentWidget, const char *name )
  : QButton( parentWidget , name)
{
  resize(6,6);
  hide();
  id = -1;
  right = false;
}
    else
void KToolBarButton::enable( bool enabled )
{
  QButton::setPixmap( (enabled ? enabledPixmap : disabledPixmap) );
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
KToolBar::KToolBar(QWidget *parent, const char *name, int _item_size)
  : QFrame( parent, name )
{
  item_size = _item_size;
  init();
  Parent = parent;        // our father
  max_width=-1;
  title = 0;
  mouseEntered=false;
  localResize=false;
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
  fullWidth=true;
  
  position = Top;
  moving = true;
  setFrameStyle(NoFrame);
  setLineWidth( 1 );
  resize( width(), item_size );
  items.setAutoDelete(true);
  enableFloating (true);
  // To make touch-sensitive handle - sven 040198
  setMouseTracking(true);
  haveAutoSized=false;      // do we have autosized item - sven 220198
  connect (kapp, SIGNAL(appearanceChanged()), this, SLOT(slotReadConfig()));
  slotReadConfig();
}

void KToolBar::slotReadConfig()
{
  int tsize;
  int _highlight;
  int icontext;
  
  KConfig *config = kapp->getConfig();
  config->setGroup("Toolbar style");

  icontext=config->readNumEntry("IconText", -1);
  if (icontext==-1)
  {
    config->writeEntry("IconText", 0 , true, true);
    config->sync();
    icontext=0;
  }
  
  tsize=config->readNumEntry("Size", -1);
  if (tsize==-1)
  {
    config->writeEntry("Size", item_size, true, true);
    config->sync();
    tsize=26;
  }

  _highlight =config->readNumEntry("Highlighting", -1);
  if (_highlight==-1)
  {
    config->writeEntry("Highlighting", 1, true, true);
    config->sync();
    _highlight=1;
  }

  bool doUpdate=false;
 /********************\
  if (tsize != item_size && tsize>20)
  {
    item_size = tsize;
    doUpdate=true;
  }

  if (icontext != icon_text)
  {
    icon_text=icontext;
    doUpdate=true;
  }
  
  if (_highlight != highlight)
  {
    highlight = _highlight;
    doUpdate=true;
  }
  
  if (doUpdate)
    emit modechange(); // tell buttons what happened
    if (isVisible ())
      updateRects(true);
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
     recreate (Parent, oldWFlags, QPoint (oldX, oldY), true);
  
  for ( KToolBarItem *b = items.first(); b!=0L; b=items.next() )
    items.remove();
  
  // MD: Get a seg. fault if following line included.
  // Sven recommeds, as a temporary measure, remove it.
  //delete context;
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
  updateRects(true);
}

void KToolBar::layoutHorizontal ()
{
  int offset=3+9+4;
  int rightOffset;
  int yOffset=1;
  KToolBarItem *autoSize = 0;
  int mywidth;
  int widest=0;
  
  horizontal = true; // sven - 040198
  if (position == Floating)
  //debug ("Ho, ho, hooo... Up-Date!!! (horizontal)");
  
  if (position == Floating)
      if (haveAutoSized == false)
          mywidth = width();
      else
          mywidth = (width()>100+offset)?width():100+offset;

  else
    if (max_width != -1)
      mywidth = max_width;
    else
      mywidth = Parent->width();

  if (fullWidth == true)
    toolbarWidth = mywidth;
  else
    toolbarWidth = offset;
  item->show();
  rightOffset=mywidth;
  toolbarHeight= item_size;
    updateRects( true );
  for ( KToolBarItem *b = items.first(); b; b=items.next() )
   {
     widest = (b->width()>widest)?b->width():widest;
     if (fullWidth == true)
      {
        if (isItemRight(b) == true)
         {
           rightOffset=rightOffset-3-b->width();

           if (rightOffset <= (offset+3))
            {
              yOffset += item_size;
              toolbarHeight += item_size;
            }
           b->move (rightOffset, yOffset);
         }
        else // Not right
         {
           int myWidth = 0;
           if (isItemAutoSized(b) == true)
            {
              autoSize = b;
              myWidth = 100; // Min width for autosized
            }
           else
             myWidth = b->width();
           if (offset > (rightOffset-myWidth+3))
            {
              offset =3+4+9;
              yOffset += item_size;
              toolbarHeight += item_size;
            }
           // Put it *really* *really* there!
           // This is a workaround for a Qt-1.32 bug.
           XMoveWindow(qt_xdisplay(), b->winId(), offset, yOffset);
           
           // This is what *should* be sufficient
           b->move( offset, yOffset );
           
           offset += myWidth + 3;
         }
      }
     else // Not fullwidth, autosize
      {
        if (offset > (mywidth-b->width()-3) && offset != 16)
         {
           offset = 3+9+4;
           yOffset += item_size;
           toolbarHeight += item_size;
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
  toolbarWidth=(toolbarWidth<(widest+16))?(widest+16):toolbarWidth;
}

void KToolBar::layoutVertical ()
{
  int offset=3+9+4;
  int yOffset=3;
  int widest;

  horizontal=false; // sven - 040198

  //debug ("Ho, ho, hooo... Up-Date!!! (vertical)");
 
  toolbarHeight = offset;
  toolbarWidth= item_size;
  widest =  item_size;
  
  // I have (had) ten thousand problems here....

  for ( KToolBarItem *b = items.first(); b; b=items.next() )
   {
     if (offset > ((max_height)-(b->height()+3)))
      {
        offset = 4+9+3;
        yOffset += widest;
        toolbarWidth += item_size;
        // Put it *really* *really* there!
        // This is a workaround for a Qt-1.32 bug.
        XMoveWindow(qt_xdisplay(), b->winId(), yOffset, offset);
        b->move( yOffset, offset );
        if (isItemAutoSized(b) == true)
          b->resize ((widest>100)?widest:100, b->height());
        if ((yOffset + b->width()+3) > toolbarWidth) // is item wider than we are
           toolbarWidth = b->width() +yOffset+2;
        if ((b->width() +3) > widest)
          widest =(b->width() +3);
        offset += b->height()+3;
      }
     else
      {
        // Put it *really* *really* there!
        // This is a workaround for a Qt-1.32 bug.
        XMoveWindow(qt_xdisplay(), b->winId(), yOffset, offset);
        b->move( yOffset, offset );
        if (isItemAutoSized(b) == true)
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
      if (items.count() == 0)
        layoutHorizontal ();
      else if (width() <= height ()-10)
        layoutVertical ();
      else if(width() >= height ()+10)
        layoutHorizontal ();
      break;

    case Left:
    case Right:
      layoutVertical ();
      break;
   }

    if (res == true)
    {
        localResize = true;
        resize (toolbarWidth, toolbarHeight);
        kapp->processEvents(); // finaly I can regulate this!!!
        localResize = false;   // Why didn't I get to this before?
    }
    // Or else.
  
}

void KToolBar::mousePressEvent ( QMouseEvent *m )
{
    if ((horizontal && m->x()<9) || (!horizontal && m->y()<9))
    {
        pointerOffset = m->pos();
        if (moving)
            if (m->button() != LeftButton)
                context->popup( mapToGlobal( m->pos() ), 0 );
            else
                grabMouse(sizeAllCursor);
    }
}

void KToolBar::resizeEvent( QResizeEvent *)
{
    /* Newest - sven */

    if (position == Floating)     // are we floating? If yes...
        if (!localResize)         // call from updateRects? if *_NOT_*...
            updateRects(true);    // ...update (i.e. WM resized us)
}

void KToolBar::paintEvent(QPaintEvent *)
{
  //MD Lots of rewrite
  
  // This code should be shared with the aequivalent in kmenubar!
  // (Marcin Dalecki).
  
  int stipple_height;

  QColorGroup g = QWidget::colorGroup();
  QPainter *paint = new QPainter();
  paint->begin( this );
  if (moving)
  {
    // Took higlighting handle from kmenubar - sven 040198
    QBrush b;
    if (mouseEntered)
      b = kapp->selectColor; // this is much more logical then
                            // the hardwired value used before!!
    else
      b = QWidget::backgroundColor();

  
    // Handle point
    if (horizontal)
    {
      qDrawShadePanel( paint, 0, 0, 9, toolbarHeight,
                       g , false, 1, &b);

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
    else
    {
      qDrawShadePanel( paint, 0, 0, toolbarWidth, 9,
                       g , false, 1, &b);

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

    }
  } //endif moving
  
#ifdef TOOLBAR_IS_RAISED

  if (position != Floating)
    qDrawShadePanel(paint, 0, 0, width(), height(), g , false, 1);

#else

  if (position != Floating)
    qDrawShadeRect(paint, 0, 0, width(), height(), g , true, 1);

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
      return false;;
      break;
    case ITEM_COMBO:
      return ((KToolBarCombo *) w)->isAuto();
      break;
    case ITEM_FRAME:
      return ((KToolBarFrame *) w)->isAuto();
      break;  
   }
  return false;
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
  return false;
}

void KToolBar::closeEvent (QCloseEvent *e)
{
  if (position == Floating)
   {
     setBarPos(lastPosition);
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
			    const char *_text, int index )
{
  KToolBarButton *button = new KToolBarButton( pixmap, id, this,
                                               0L, item_size, _text);
  if ( index == -1 )
    items.append( button );
  else
    items.insert( index, button );
  
  connect(button, SIGNAL(clicked(int)), this, SLOT(ButtonClicked(int)));
  connect(button, SIGNAL(released(int)), this, SLOT(ButtonReleased(int)));
  connect(button, SIGNAL(pressed(int)), this, SLOT(ButtonPressed(int)));
  button->enable( enabled );

  button->show();
  updateRects( true );
  return items.at();
}

/// Inserts a button with connection.

int KToolBar::insertButton( const QPixmap& pixmap, int id, const char *signal,
			    const QObject *receiver, const char *slot, bool enabled,
			    const char *_text, int index )
{
  KToolBarButton *button = new KToolBarButton( pixmap, id, this,
                                               0L, item_size, _text);
  if ( index == -1 ) 
    items.append( button );
  else
    items.insert( index, button );
		
  connect( button, signal, receiver, slot );
  button->enable( enabled );
  button->show();
  updateRects( true );
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
	
  updateRects( true );
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
  frame -> resize (_size, item_size-2);
  frame->show();
  updateRects(true);
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
  lined->resize(size, item_size-2);
  lined->enable(enabled);
  lined->show();
  updateRects(true);
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
  combo->setAutoResize(true);
  combo->resize(size, item_size);
  combo->enable(enabled);
  combo->show();
  updateRects(true);
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
  combo->resize(size, item_size);
  combo->enable(enabled);
  combo->show();
  updateRects(true);
  return items.at();
}

/// Removes item by ID

void KToolBar::removeItem (int id)
{
  for (KToolBarItem *b = items.first(); b; b=items.next())
    if (getID(b) == id )
    {
        if(isItemAutoSized(b))
            haveAutoSized=false;
       items.remove();
     }
  //updateRects(true);
  emit moved (position);
}

/// ******** Tools

/// misc
void KToolBar::addConnection (int id, const char *signal,
                              const QObject *receiver, const char *slot)
{
  for (KToolBarItem *b = items.first(); b; b=items.next())
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
  for (KToolBarItem *b = items.first(); b; b=items.next())
    if (getID(b) == id )
      if (getType(b) == ITEM_FRAME)
        return ((KToolBarFrame *) b);
  return 0;
}

/// Common
void KToolBar::setItemEnabled( int id, bool enabled )
{
  for (KToolBarItem *b = items.first(); b; b=items.next())
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
  for (KToolBarItem *b = items.first(); b; b=items.next())
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

         default: return;
        }
       haveAutoSized=true;
     }
  updateRects(true);
}

void KToolBar::alignItemRight(int id, bool yes)
{
  for (KToolBarItem *b = items.first(); b; b=items.next())
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
  updateRects(true);
}

/// Butoons
void KToolBar::setButtonPixmap( int id, const QPixmap& _pixmap )
{
  for (KToolBarItem *b = items.first(); b; b=items.next())
    if (getID(b) == id )
      if (getType(b) == ITEM_BUTTON)
	((KToolBarButton *) b)->setPixmap( _pixmap ); 
}

/// Toggle buttons

void KToolBar::setToggle ( int id, bool yes )
{
  for (KToolBarItem *b = items.first(); b; b=items.next())
    if (getID(b) == id )
      if (getType(b) == ITEM_BUTTON)
         ((KToolBarButton *) b)->beToggle(yes);
}

void KToolBar::toggleButton (int id)
{
  for (KToolBarItem *b = items.first(); b; b=items.next())
    if (getID(b) == id )
      if (getType(b) == ITEM_BUTTON)
       {
         if (((KToolBarButton *) b)->isToggleButton() == true)
           ((KToolBarButton *) b)->toggle();
       }
}

void KToolBar::setButton (int id, bool on)
{
  for (KToolBarItem *b = items.first(); b; b=items.next())
    if (getID(b) == id )
      if (getType(b) == ITEM_BUTTON)
       {
         //if (((KToolBarButton *) b)->isToggleButton() == true)
           ((KToolBarButton *) b)->on(on);
       }
}

bool KToolBar::isButtonOn (int id)
{
  for (KToolBarItem *b = items.first(); b; b=items.next())
    if (getID(b) == id )
      if (getType(b) == ITEM_BUTTON)
       {
         if (((KToolBarButton *) b)->isToggleButton() == true)
           return ((KToolBarButton *) b)->isOn();
       }
  return false;
}

/// Lined
void KToolBar::setLinedText (int id, const char *text)
{
  for (KToolBarItem *b = items.first(); b; b=items.next())
    if (getID(b) == id )
      if (getType(b) == ITEM_LINED)
       {
         ((KToolBarLined *) b)->setText(text);
         ((KToolBarLined *) b)->cursorAtEnd();
       }
}

const char *KToolBar::getLinedText (int id )
{
  for (KToolBarItem *b = items.first(); b; b=items.next())
    if (getID(b) == id )
      if (getType(b) == ITEM_LINED)
	return ((KToolBarLined *) b)->text();
  return 0;
}

/// Combos
void KToolBar::insertComboItem (int id, const char *text, int index)
{
  for (KToolBarItem *b = items.first(); b; b=items.next())
    if (getID(b) == id )
      if (getType(b) == ITEM_COMBO)
       {
         ((KToolBarCombo *) b)->insertItem(text, index);
         ((KToolBarCombo *) b)->cursorAtEnd();
       }
}

void KToolBar::insertComboList (int id, QStrList *list, int index)
{
  for (KToolBarItem *b = items.first(); b; b=items.next())
    if (getID(b) == id )
      if (getType(b) == ITEM_COMBO)
	((KToolBarCombo *) b)->insertStrList(list, index);
}

void KToolBar::setCurrentComboItem (int id, int index)
{
  for (KToolBarItem *b = items.first(); b; b=items.next())
    if (getID(b) == id )
      if (getType(b) == ITEM_COMBO)
       {
         ((KToolBarCombo *) b)->setCurrentItem(index);
         ((KToolBarCombo *) b)->cursorAtEnd();
       }
}

void KToolBar::removeComboItem (int id, int index)
{
  for (KToolBarItem *b = items.first(); b; b=items.next())
    if (getID(b) == id )
      if (getType(b) == ITEM_COMBO)
	((KToolBarCombo *) b)->removeItem(index);
}

void KToolBar::changeComboItem  (int id, const char *text, int index)
{
  for (KToolBarItem *b = items.first(); b; b=items.next())
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
  for (KToolBarItem *b = items.first(); b; b=items.next())
    if (getID(b) == id )
      if (getType(b) == ITEM_COMBO)
	((KToolBarCombo *) b)->clear();
}

const char *KToolBar::getComboItem (int id, int index)
{
  for (KToolBarItem *b = items.first(); b; b=items.next())
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
  for (KToolBarItem *b = items.first(); b; b=items.next())
    if (getID(b) == id )
      if (getType(b) == ITEM_COMBO)
        return ((KToolBarCombo *) b);
  return 0;
}

KToolBarLined *KToolBar::getLined (int id)
{
  for (KToolBarItem *b = items.first(); b!=NULL; b=items.next())
    if (getID(b) == id )
      if (getType(b) == ITEM_LINED)
        return ((KToolBarLined *) b);
  return 0;
}


KToolBarButton* KToolBar::getButton( int id )
{
  for( KToolBarItem* b = items.first(); b != NULL; b = items.next() )
	if( getID( b ) == id )
	  if( getType( b ) == ITEM_BUTTON )
		return ((KToolBarButton *) b);
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
        recreate(0, 0, p, false);
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
        context->changeItem (klocale->translate("UnFloat"), CONTEXT_FLOAT);
        setMouseTracking(true);
        mouseEntered=false;
        wasFullWidth=fullWidth;
        if (!haveAutoSized)   //if we don't have autosized item
            fullWidth=false;  // turn off autosize of toolbar
        else                  // but if we do..
            resize((int) (0.7*width()), height()); // narrow us on 70%
        updateRects (true);
        emit moved (bpos);  // ->ktw::updateRects->bar::updateRects = one excess update (damn)
        return;
      }
     else if (position == Floating) // was floating
      {
        position = bpos;
        hide();
        recreate(Parent, oldWFlags, QPoint(oldX, oldY), true);
        setMinimumSize (item_size, item_size);
        context->changeItem (klocale->translate("Float"), CONTEXT_FLOAT);
        setMouseTracking(true);
        mouseEntered = false;
        fullWidth=wasFullWidth;
        // updateRects (true);
        emit moved (bpos); // another bar::updateRects (damn)
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

QSize KToolBar::sizeHint ()
{
  int w=16;
  
  for (KToolBarItem *b = items.first(); b; b=items.next())
    w+=isItemAutoSized(b)?100:b->width();
  szh.setWidth(w);

  return szh;
}
/*********************************************************/
/***********    O L D     I N T E R F A C E   ************/
/*********************************************************/

int KToolBar::insertItem(const QPixmap& _pixmap, int _ID, bool enabled,
               char *ToolTipText, int index)

{
  warning ("KToolBar: insertItem is obsolete. Use insertButton");
  warning ("******insertItem WILL SOON BE REMOVED!! USE insertButton*****");
  return (this->insertButton (_pixmap, _ID, enabled, ToolTipText, index));
}

int KToolBar::insertItem(const QPixmap& _pixmap, int _ID, const char *signal,
               const QObject *receiver, const char *slot,
               bool enabled,
               char *tooltiptext, int index)
{
  warning ("KToolBar: insertItem is obsolete. Use InsertButton");
  warning ("******insertItem WILL SOON BE REMOVED!! USE insertButton*****");
  return (this->insertButton (_pixmap, _ID, signal, receiver, slot, enabled, tooltiptext, index));
}

void KToolBar::setItemPixmap( int _id, const QPixmap& _pixmap )
{
  warning ("KToolBar: setItemPixmap is obsolete. Use setButtonPixmap");
  warning ("******setItemPixmap WILL SOON BE REMOVED!! USE setButtonPixmap*****");
  this->setButtonPixmap (_id, _pixmap);
}

/*************************************************************

Mouse move and drag routines

*************************************************************/


void KToolBar::leaveEvent (QEvent *)
{
    mouseEntered = false;
    repaint();
}

void KToolBar::mouseMoveEvent(QMouseEvent* mev){


    // Handle highlighting - sven 050198
    if (horizontal)
        if (mev->x() < 9)
        {
            if (!mouseEntered)
            {
                mouseEntered = true;
                repaint();
            }
        }
        else
        {
            if (mouseEntered)
            {
                mouseEntered = false;
                repaint();
            }
        }
    
    else
        if (mev->y() < 9)
        {
            if (!mouseEntered)
            {
                mouseEntered = true;
                repaint();
            }
        }
        else
        {
            if (mouseEntered)
            {
                mouseEntered = false;
                repaint();
            }
        }



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
    while(XGrabPointer( qt_xdisplay(), winId(), true,
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
    while(XGrabPointer( qt_xdisplay(), winId(), true,
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

