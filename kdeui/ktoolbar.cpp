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

// $Id$
// $Log$
// Revision 1.65  1998/05/12 10:47:31  radej
// Fixed sizehint - returns more real width
//
// Revision 1.64  1998/05/07 23:12:30  radej
// Fix for optional highlighting of handle
//
// Revision 1.63  1998/05/05 16:53:31  radej
// This damned opaque moving...
//
// Revision 1.62  1998/05/04 16:38:19  radej
// Revision 1.64  1998/05/07 23:12:30  radej
// Fix for optional highlighting of handle
// Revision 1.60  1998/05/02 18:31:01  radej
// Improved docking
//
// Revision 1.59  1998/04/28 09:17:28  radej
// Revision 1.62  1998/05/04 16:38:19  radej
// Bugfixes for moving + opaque moving
// Revision 1.58  1998/04/27 19:22:41  ettrich
// Matthias: the nifty feature that you can globally change the size of the
//   toolbars broke the nifty-as-well feature that a client can pass another
//   size than 26 to the toolbar´s constructor.
//
//   I hope I found a solution which allows both. If you pass an argument
//   to the constructor, than a new attribute fixed_size is set.
//
// Revision 1.57  1998/04/26 13:30:16  kulow
// fixed typo
//
// Revision 1.56  1998/04/23 16:08:56  radej
// Fixed a bug reported by Thomas Tanghus
//
// Revision 1.54  1998/04/21 20:37:03  radej
// Added insertWidget and some reorganisation - BINARY INCOMPATIBLE
//
// Revision 1.53  1998/04/16 18:47:19  radej
// Removed some debug text before beta4
//
//
//-------------------------------------------------------------------------
// OLD CHANGES:
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
// Fixed white bg on disabled buttons in icontext mode and
// positioning on signal appearanceChanged - sven 24.3.1998
//-------------------------------------------------------------------------


#include <qpainter.h>
#include <qtooltip.h> 
#include <qdrawutl.h>
#include <qpalette.h>
#include <qbitmap.h>
#include <qstring.h>

#include <qpainter.h>
#include <qrect.h>
#include <qpalette.h>
#include <qbitmap.h>
#include <qstring.h>
#include <qframe.h>
#include <qbutton.h>
#include "ktoolbar.h"
#include "klined.h"
#include "kcombo.h"
#include <ktopwidget.h>
#include <klocale.h>
#include <kapp.h>
#include <kwm.h>
#include <ktoolboxmgr.h>
  default:
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
// this should be adjustable (in faar future... )
#define MIN_AUTOSIZE 150

KToolBarItem::KToolBarItem (Item *_item, itemType _type, int _id,
                            bool _myItem)
{
  id = _id;
  right=false;
  autoSized=false;
  type=_type;
  item = _item;
  myItem = _myItem;
}

KToolBarItem::~KToolBarItem ()
{
  // Delete this item if localy constructed
  if (myItem)
    delete item;
}

KToolBarButton::KToolBarButton( const QPixmap& pixmap, int _id,
                                QWidget *_parent, const char *name,
                                int item_size, const char *txt) : QButton( _parent, name )
{
  parentWidget = (KToolBar *) _parent;
  raised = false;
  setFocusPolicy( NoFocus );
  id = _id;
  if (txt)
  btext=txt;
  if ( ! pixmap.isNull() )
    enabledPixmap = pixmap;
  else
    {
      warning(klocale->translate("KToolBarButton: pixmap is empty, perhaps some missing file"));
      enabledPixmap.resize( item_size-4, item_size-4);
    }
  modeChange ();
  makeDisabledPixmap();
  setPixmap( enabledPixmap );
  connect (parentWidget, SIGNAL( modechange() ), this, SLOT( modeChange() ));
  
  connect( this, SIGNAL( clicked() ), this, SLOT( ButtonClicked() ) );
  connect(this, SIGNAL( pressed() ), this, SLOT( ButtonPressed() ) );
  connect(this, SIGNAL( released() ), this, SLOT( ButtonReleased() ) );
  //right = false;
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
        _painter->setPen(palette().disabled().dark());
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
  if(ImASeparator())  {
    makeDisabledPixmap();
    if ( !isEnabled() ) 
      QButton::setPixmap( disabledPixmap );
    repaint(false); // no need to delete it first therefore only false
  }
}
  bool doUpdate=false;

void KToolBarButton::modeChange()
{
  int myWidth;
  }
  myWidth = enabledPixmap.width();
  
  buttonFont.setFamily("Helvetica");
  buttonFont.setPointSize(10);
  buttonFont.setBold(false);
  buttonFont.setItalic(false);
  buttonFont.setCharSet(font().charSet());
  
  QFontMetrics fm(buttonFont);
  
  icontext=parentWidget->icon_text;
  _size=parentWidget->item_size;
  if (myWidth < _size)
    myWidth = _size;

  highlight=parentWidget->highlight;
  if (icontext) //Calculate my size
  {
    QToolTip::remove(this);
    resize (fm.width(btext)+myWidth, _size-2); // +2+_size-2
  }
  else
  {
    QToolTip::remove(this);
    QToolTip::add(this, btext);
    resize (myWidth, _size-2);
  }
}
  int rightOffset;
KToolBarButton::KToolBarButton( QWidget *parentWidget, const char *name )
  : QButton( parentWidget , name)
{
  resize(6,6);
  hide();
  youreSeparator ();;
}
    else
void KToolBarButton::setEnabled( bool enabled )
{
  QButton::setPixmap( (enabled ? enabledPixmap : disabledPixmap) );
  QButton::setEnabled( enabled );
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

/****************************** Tolbar **************************************/
	    context->popup( mapToGlobal( m->pos() ), 0 );
KToolBar::KToolBar(QWidget *parent, const char *name, int _item_size)
  : QFrame( parent, name )
{
  item_size = _item_size;
  fixed_size =  (item_size > 0);
  if (!fixed_size)
  item_size = 26;
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
  icon_text = 0;
  highlight = 0;
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
  // This code should be shared with the aequivalent in kmenubar!
  mgr =0;
}

void KToolBar::slotReadConfig()
{
  int tsize;
  int _highlight;
  int icontext;
  bool _transparent;
  
  KConfig *config = kapp->getConfig();
  QString group = config->group();
  config->setGroup("Toolbar style");
  icontext=config->readNumEntry("IconText", 0);
  tsize=config->readNumEntry("Size", 26);
  _highlight =config->readNumEntry("Highlighting", 1);
  _transparent = config->readBoolEntry("TransparentMoving", true);
  config->setGroup(group);

  bool doUpdate=false;
 /********************\
  if (!fixed_size && tsize != item_size && tsize>20)
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
  if (position == Floating)
  if (_transparent != transparent)
  {
    transparent= _transparent;
    doUpdate=false;
  }
  
  if (doUpdate)
    emit modechange(); // tell buttons what happened
    if (isVisible ())
      emit moved (position);
}
    emit (moved(position));
void KToolBar::drawContents ( QPainter *)
{
}

KToolBar::~KToolBar()
{
int KToolBar::insertLineSeparator( int index )
// what is that?! we do not need to recreate before
// destroying.... (Matthias)
//   if (position == Floating)
//   {
//     debug ("KToolBar destructor: about to recreate");
//     recreate (Parent, oldWFlags, QPoint (oldX, oldY), false);
//     debug ("KToolBar destructor: recreated");
//   }

  // what is that?! toolbaritems are children of the toolbar, which
  // means, qt will delete them for us (Matthias)
  //for ( KToolBarItem *b = items.first(); b!=0L; b=items.next() )
  // items.remove();
  
  if (!QApplication::closingDown())
    delete context;
  
  //debug ("KToolBar destructor");
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
  int offset=3+9+4; // = 16
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
          mywidth = (width()>MIN_AUTOSIZE+offset)?width():MIN_AUTOSIZE+offset;

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
    if (b->isAuto())
      widest = (MIN_AUTOSIZE>widest)?MIN_AUTOSIZE:widest;
    else
      widest = (b->width()>widest)?b->width():widest;
     
     if (fullWidth == true)
      {
        if (b->isRight() == true)
         {
           rightOffset=rightOffset-3-b->width();

           if (rightOffset <= (offset+3))
            {
              yOffset += item_size;
              toolbarHeight += item_size;
            }
           XMoveWindow(qt_xdisplay(), b->winId(), rightOffset, yOffset);
           b->move (rightOffset, yOffset);
         }
        else // Not right
         {
           int myWidth = 0;
           if (b->isAuto() == true)
            {
              autoSize = b;
              myWidth = MIN_AUTOSIZE; // Min width for autosized
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
           XMoveWindow(qt_xdisplay(), b->winId(), offset, yOffset);
           b->move( offset, yOffset );
           offset += b->width()+3;
         }
        else
         {
           XMoveWindow(qt_xdisplay(), b->winId(), offset, yOffset);
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
        if (b->isAuto() == true)
          b->resize ((widest>MIN_AUTOSIZE)?widest:MIN_AUTOSIZE, b->height());
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
        if (b->isAuto() == true)
             b->resize ((widest>MIN_AUTOSIZE)?widest:MIN_AUTOSIZE, b->height());
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
      localResize = false;
    }
  /*
   I needed to call processEvents () to flush pending resizeEvent after
   doing resize from here so that resizeEvent () sees the semaphore
   localResize. After examining qt source, I saw that QWidget::resize() calls
   QApplication::sendEvent() which, as far as I understand, notifies
   this->resizeEvent() immediatelly, without waiting. Even worse - it seems
   that it really works. "Worse?", I hear you asking. Yes, even worse indeed
   it is, because I lost my mind trying to get rid of excess resizes, and now
   they vanished mysteriously all by itself (all except two)
   */
}

void KToolBar::mousePressEvent ( QMouseEvent *m )
{
  int ox, oy, ow, oh;
  if ((horizontal && m->x()<9) || (!horizontal && m->y()<9))
  {
    pointerOffset = m->pos();
    if (moving)
      if (m->button() != LeftButton)
        context->popup( mapToGlobal( m->pos() ), 0 );
      else
      {
        //QRect rr(KWM::geometry(Parent->winId(), false));
        QRect rr(Parent->geometry());
        ox = rr.x();
        oy = rr.y();
        ow = rr.width();
        oh = rr.height();
        if (Parent->inherits("KTopLevelWidget"))
        {
          ox += ((KTopLevelWidget *) Parent)->view_left;
          oy += ((KTopLevelWidget *) Parent)->view_top;
          ow = ((KTopLevelWidget *) Parent)->view_right -
            ((KTopLevelWidget *) Parent)->view_left;
          oh = ((KTopLevelWidget *) Parent)->view_bottom -
            ((KTopLevelWidget *) Parent)->view_top;;
        }
            
        int  fat = 25; //ness

        mgr = new KToolBoxManager(this, transparent);

        mgr->addHotSpot(ox, oy, ow, fat);           // top
        mgr->addHotSpot(ox, oy+oh-fat, ow, fat);    // bottom
        mgr->addHotSpot(ox, oy+fat, fat, oh-2*fat); // left
        mgr->addHotSpot(ox+ow-fat, oy+fat, fat, oh-2*fat); //right

        movePos = position;
        connect (mgr, SIGNAL(onHotSpot(int)), SLOT(slotHotSpot(int)));
        if (transparent)
          mgr->doMove(true, false, true);
        else
        {
          QList<KToolBarItem> ons;
          for (KToolBarItem *b = items.first(); b; b=items.next())
          {
            if (b->isEnabled())
              ons.append(b);
            b->setEnabled(false);
          }
              
          mgr->doMove(true, false, false);

          for (KToolBarItem *b = ons.first(); b; b=ons.next())
            b->setEnabled(true);
        }
        if (transparent)
        {
          setBarPos (movePos);

          if (movePos == Floating)
            move (mgr->x(), mgr->y());
          if (!isVisible())
            show();
        }
        delete mgr;
        mgr=0;
        debug ("KToolBar: moving done");
      }
  }
}

void KToolBar::slotHotSpot(int hs)
{
  if (mgr == 0)
    return;
  if (!transparent) // opaque
  {
    switch (hs)
    {
      case 0: //top
        setBarPos(Top);
        break;

      case 1: //bottom
        setBarPos(Bottom);
        break;
      
      case 2: //left
        setBarPos(Left);
        break;
      
      case 3: //right
        setBarPos(Right);
        break;

      case -1: // left all
        setBarPos(Floating);
        break;
    }
    if (position != Floating)
    {
      QPoint p(Parent->mapToGlobal(pos())); // OH GOOOOODDDD!!!!!
      mgr->setGeometry(p.x(), p.y(), width(), height());
    }
    if (!isVisible())
      show();
  }
  else // transparent
  {
    switch (hs)
    {
      case 0: //top
        mgr->setGeometry(0);
        movePos=Top;
        break;

      case 1: //bottom
        mgr->setGeometry(1);
        movePos=Bottom;
        break;

      case 2: //left
        mgr->setGeometry(2);
        movePos=Left;
        break;

      case 3: //right
        mgr->setGeometry(3);
        movePos=Right;
        break;

      case -1: // left all
        mgr->setGeometry(mgr->mouseX(), mgr->mouseY(), width(), height());
        movePos=Floating;
        break;
    }
  }
}
void KToolBar::resizeEvent( QResizeEvent *)
{
  /* Newest - sven */
  
  if (position == Floating)  // are we floating? If yes...
    if (!localResize)        // call from updateRects? if *_NOT_*...
      updateRects(true);     // ...update (i.e. WM resized us)
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
    if (mouseEntered && highlight)
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

  if (position != Floating)
    if ( style() == MotifStyle )
      qDrawShadePanel(paint, 0, 0, width(), height(), g , false, 1);
    //else
      //qDrawShadeRect(paint, 0, 0, width(), height(), g , true, 1);

  paint->end();
  delete paint;
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

void KToolBar::ButtonToggled( int id )
{
  emit toggled( id );
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
  KToolBarButton *button = new KToolBarButton( pixmap, id, this, 0L, item_size,
                                               _text);
  KToolBarItem *item = new KToolBarItem(button, ITEM_BUTTON, id,
                                        true);
  if ( index == -1 )
    items.append( item );
  else
    items.insert( index, item );
  
  connect(button, SIGNAL(clicked(int)), this, SLOT(ButtonClicked(int)));
  connect(button, SIGNAL(released(int)), this, SLOT(ButtonReleased(int)));
  connect(button, SIGNAL(pressed(int)), this, SLOT(ButtonPressed(int)));

  item->setEnabled( enabled );
  item->show();
  if (position == Floating)
    updateRects( true );
  else if (isVisible())
    emit (moved(position));
  return items.at();
}

/// Inserts a button with connection.

int KToolBar::insertButton( const QPixmap& pixmap, int id, const char *signal,
			    const QObject *receiver, const char *slot, bool enabled,
			    const char *_text, int index )
{
  KToolBarButton *button = new KToolBarButton( pixmap, id, this,
                                               0L, item_size, _text);
  KToolBarItem *item = new KToolBarItem(button, ITEM_BUTTON, id,
                                        true);
 
  if ( index == -1 ) 
    items.append( item );
  else
    items.insert( index, item );
		
  connect( button, signal, receiver, slot );
  item->setEnabled( enabled );
  item->show();
  if (position == Floating)
    updateRects( true );
  else if (isVisible())
    emit (moved(position));
  return items.at();
}

/********* SEPARATOR *********/
/// Inserts separator

int KToolBar::insertSeparator( int index )
{
  KToolBarButton *separ = new KToolBarButton( this );
  KToolBarItem *item = new KToolBarItem(separ, ITEM_BUTTON, -1,
                                        true);

  if ( index == -1 )
    items.append( item );
  else
    items.insert( index, item );
	
  if (position == Floating)
    updateRects( true );
  else if (isVisible())
    emit (moved(position));
  return items.at();
}

/********* Frame **********/
/// inserts QFrame

int KToolBar::insertFrame (int _id, int _size, int _index)
{
  debug ("insertFrame is deprecated. use insertWidget");
  
  QFrame *frame;
  bool mine = false;

  // ok I'll do it for you;
  frame = new QFrame (this);
  mine = true;
  
  KToolBarItem *item = new KToolBarItem(frame, ITEM_FRAME, _id, mine);
      
  if (_index == -1)
    items.append (item);
  else
    items.insert(_index, item);
  item-> resize (_size, item_size-2);
  item->show();
  if (position == Floating)
    updateRects( true );
  else if (isVisible())
    emit (moved(position));
  return items.at();
}  
/* A poem all in G-s! No, any widget */

int KToolBar::insertWidget(int _id, int _size, QWidget *_widget, int _index=-1)
{
  KToolBarItem *item = new KToolBarItem(_widget, ITEM_FRAME, _id, false);
  
  if (_index == -1)
    items.append (item);
  else
    items.insert(_index, item);
  item-> resize (_size, item_size-2);
  item->show();
  if (position == Floating)
    updateRects( true );
  else if (isVisible())
    emit (moved(position));
  return items.at();
}

/************** LINE EDITOR **************/
/// Inserts a KLined. KLined is derived from QLineEdit and has another signal, tabPressed,
//  for completions.

int KToolBar::insertLined(const char *text, int id, const char *signal,
			  const QObject *receiver, const char *slot,
			  bool enabled, const char *tooltiptext, int size, int index)
{
  KLined *lined = new KLined (this, 0);
  KToolBarItem *item = new KToolBarItem(lined, ITEM_LINED, id,
                                        true);
 
  
  if (index == -1)
    items.append (item);
  else
    items.insert(index, item);
  if (tooltiptext)
    QToolTip::add( lined, tooltiptext );
  connect( lined, signal, receiver, slot );
  lined->setText(text);
  item->resize(size, item_size-2);
  item->setEnabled(enabled);
  item->show();
  if (position == Floating)
    updateRects( true );
  else if (isVisible())
    emit (moved(position));
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
  KCombo *combo = new KCombo (writable, this);
  KToolBarItem *item = new KToolBarItem(combo, ITEM_COMBO, id,
                                        true);
 
  if (index == -1)
    items.append (item);
  else
    items.insert (index, item);
  combo->insertStrList (list);
  combo->setInsertionPolicy(policy);
  if (tooltiptext)
    QToolTip::add( combo, tooltiptext );
  connect ( combo, signal, receiver, slot );
  combo->setAutoResize(false);
  item->resize(size, item_size-2);
  item->setEnabled(enabled);
  item->show();
  if (position == Floating)
    updateRects( true );
  else if (isVisible())
    emit (moved(position));
  return items.at();
}


/// Inserts combo with text

int KToolBar::insertCombo (const char *text, int id, bool writable,
                           const char *signal, QObject *receiver,
                           const char *slot, bool enabled,
                           const char *tooltiptext, int size, int index,
                           KCombo::Policy policy)
{
  KCombo *combo = new KCombo (writable, this);
  KToolBarItem *item = new KToolBarItem(combo, ITEM_COMBO, id,
                                        true);

  if (index == -1)
    items.append (item);
  else
    items.insert (index, item);
  combo->insertItem (text);
  combo->setInsertionPolicy(policy);
  if (tooltiptext)
    QToolTip::add( combo, tooltiptext );
  connect (combo, signal, receiver, slot);
  combo->setAutoResize(false);
  item->resize(size, item_size-2);
  item->setEnabled(enabled);
  item->show();
  if (position == Floating)
    updateRects( true );
  else if (isVisible())
    emit (moved(position));
  return items.at();
}

/// Removes item by ID

void KToolBar::removeItem (int id)
{
  for (KToolBarItem *b = items.first(); b; b=items.next())
    if (b->ID() == id )
    {
      if(b->isAuto())
        haveAutoSized=false;
      items.remove();
    }
  if (position == Floating)
    updateRects( true );
  else if (isVisible())
    emit moved (position);
}

void KToolBar::showItem (int id)
{
  for (KToolBarItem *b = items.first(); b; b=items.next())
    if (b->ID() == id )
    {
      if(b->isAuto())
        haveAutoSized=false;
      b->show();
    }
  if (position == Floating)
    updateRects( true );
  else if (isVisible())
    emit moved (position);
}

void KToolBar::hideItem (int id)
{
  for (KToolBarItem *b = items.first(); b; b=items.next())
    if (b->ID() == id )
    {
      if(b->isAuto())
        haveAutoSized=false;
      b->hide();
    }
  if (position == Floating)
    updateRects( true );
  else if (isVisible())
    emit moved (position);
    
}
/// ******** Tools

/// misc
void KToolBar::addConnection (int id, const char *signal,
                              const QObject *receiver, const char *slot)
{
  for (KToolBarItem *b = items.first(); b; b=items.next())
    if (b->ID() == id )
           connect (b->getItem(), signal, receiver, slot);
}

/// Common
void KToolBar::setItemEnabled( int id, bool enabled )
{
  for (KToolBarItem *b = items.first(); b; b=items.next())
    if (b->ID() == id )
      b->setEnabled(enabled);
}

void KToolBar::setItemAutoSized ( int id, bool enabled )
{
  for (KToolBarItem *b = items.first(); b; b=items.next())
    if (b->ID() == id )
    {
      b->autoSize(enabled);
      haveAutoSized = true;
      if (position == Floating)
        updateRects( true );
      else if (isVisible())
        emit moved (position);
    }
}

void KToolBar::alignItemRight(int id, bool yes)
{
  for (KToolBarItem *b = items.first(); b; b=items.next())
    if (b->ID() == id )
    {
      b->alignRight (yes);
      if (position == Floating)
        updateRects( true );
      else if (isVisible())
        emit moved (position);
    }
}

/// Butoons
void KToolBar::setButtonPixmap( int id, const QPixmap& _pixmap )
{
  for (KToolBarItem *b = items.first(); b; b=items.next())
    if (b->ID() == id )
      ((KToolBarButton *) b->getItem())->setPixmap( _pixmap );
}

/// Toggle buttons
void KToolBar::setToggle ( int id, bool yes )
{
  for (KToolBarItem *b = items.first(); b; b=items.next())
    if (b->ID() == id )
    {
      ((KToolBarButton *) b->getItem())->beToggle(yes);
      connect (b->getItem(), SIGNAL(toggled(int)),
               this, SLOT(ButtonToggled(int)));
    }
}

void KToolBar::toggleButton (int id)
{
  for (KToolBarItem *b = items.first(); b; b=items.next())
    if (b->ID() == id )
    {
      if (((KToolBarButton *) b->getItem())->isToggleButton() == true)
        ((KToolBarButton *) b->getItem())->toggle();
    }
}

void KToolBar::setButton (int id, bool on)
{
  for (KToolBarItem *b = items.first(); b; b=items.next())
    if (b->ID() == id )
      ((KToolBarButton *) b->getItem())->on(on);
}

bool KToolBar::isButtonOn (int id)
{
  for (KToolBarItem *b = items.first(); b; b=items.next())
    if (b->ID() == id )
    {
      if (((KToolBarButton *) b->getItem())->isToggleButton() == true)
        return ((KToolBarButton *) b->getItem())->isOn();
    }
  return false;
}

/// Lined
void KToolBar::setLinedText (int id, const char *text)
{
  for (KToolBarItem *b = items.first(); b; b=items.next())
    if (b->ID() == id )
    {
      ((KLined *) b->getItem())->setText(text);
      ((KLined *) b->getItem())->cursorAtEnd();
    }
}

const char *KToolBar::getLinedText (int id )
{
  for (KToolBarItem *b = items.first(); b; b=items.next())
    if (b->ID() == id )
      return ((KLined *) b->getItem())->text();
  return 0;
}

/// Combos
void KToolBar::insertComboItem (int id, const char *text, int index)
{
  for (KToolBarItem *b = items.first(); b; b=items.next())
    if (b->ID() == id )
    {
      ((KCombo *) b->getItem())->insertItem(text, index);
      ((KCombo *) b->getItem())->cursorAtEnd();
    }
}

void KToolBar::insertComboList (int id, QStrList *list, int index)
{
  for (KToolBarItem *b = items.first(); b; b=items.next())
    if (b->ID() == id )
	((KCombo *) b->getItem())->insertStrList(list, index);
}

void KToolBar::setCurrentComboItem (int id, int index)
{
  for (KToolBarItem *b = items.first(); b; b=items.next())
    if (b->ID() == id )
    {
      ((KCombo *) b->getItem())->setCurrentItem(index);
      ((KCombo *) b->getItem())->cursorAtEnd();
    }
}

void KToolBar::removeComboItem (int id, int index)
{
  for (KToolBarItem *b = items.first(); b; b=items.next())
    if (b->ID() == id )
      ((KCombo *) b->getItem())->removeItem(index);
}

void KToolBar::changeComboItem  (int id, const char *text, int index)
{
  for (KToolBarItem *b = items.first(); b; b=items.next())
    if (b->ID() == id )
    {
      if (index == -1)
      {
        index = ((KCombo *) b->getItem())->currentItem();
        ((KCombo *) b->getItem())->changeItem(text, index);
      }
      else
      {
        ((KCombo *) b->getItem())->changeItem(text, index);
      }
      ((KCombo *) b->getItem())->cursorAtEnd();
    }
}

void KToolBar::clearCombo (int id)
{
  for (KToolBarItem *b = items.first(); b; b=items.next())
    if (b->ID() == id )
	((KCombo *) b->getItem())->clear();
}

const char *KToolBar::getComboItem (int id, int index)
{
  for (KToolBarItem *b = items.first(); b; b=items.next())
    if (b->ID() == id )
    {
      if (index == -1)
        index = ((KCombo *) b->getItem())->currentItem();
      return ((KCombo *) b->getItem())->text(index);
    }
  return 0;
}

KCombo *KToolBar::getCombo (int id)
{
  for (KToolBarItem *b = items.first(); b; b=items.next())
    if (b->ID() == id )
      return ((KCombo *) b->getItem());
  return 0;
}

KLined *KToolBar::getLined (int id)
{
  for (KToolBarItem *b = items.first(); b!=NULL; b=items.next())
    if (b->ID() == id )
      return ((KLined *) b->getItem());
  return 0;
}


KToolBarButton* KToolBar::getButton( int id )
{
  for( KToolBarItem* b = items.first(); b != NULL; b = items.next() )
    if(b->ID() == id )
      return ((KToolBarButton *) b->getItem());
  return 0;
}

QFrame *KToolBar::getFrame (int id)
{
  for (KToolBarItem *b = items.first(); b; b=items.next())
    if (b->ID() == id )
      return ((QFrame *) b->getItem());
  return 0;
}

QWidget *KToolBar::getWidget (int id)
{
  for (KToolBarItem *b = items.first(); b; b=items.next())
    if (b->ID() == id )
      return (b->getItem());
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
        updateRects (true); // we need this to set us up
        emit moved (bpos);  // this sets up KTW but not toolbar which floats
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
    {
      w+=b->isAuto()?MIN_AUTOSIZE:b->width();
      w+=3;
    }
  szh.setWidth(w);

  return szh;
}

/*************************************************************

Mouse move and drag routines

*************************************************************/


void KToolBar::leaveEvent (QEvent *)
{
    mouseEntered = false;
    repaint();
}

void KToolBar::mouseMoveEvent(QMouseEvent* mev)
{
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
}

void KToolBar::mouseReleaseEvent ( QMouseEvent * ){
  debug ("KToolBar: mouseRelease event");
}

// sven
#include <ktoolbar.moc>
      ++it;
    }
  }
}

#include "ktoolbar.moc"

