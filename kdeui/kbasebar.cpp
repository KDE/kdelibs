/* This file is part of the KDE libraries
    Copyright (C) 1997, 1998 Stephan Kulow (coolo@kde.org)
              (C) 1997, 1998 Mark Donohoe (donohoe@kde.org)
              (C) 1997, 1998 Sven Radej (radej@kde.org)
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

// Do not add this to makefiles or stuff cuz it is unused. I commited this
// so I can track changes and revisions. If you want to try this, mail me
// and I'll send you my working files.

// KBaseBar is/will be a base class for KToolBar and KToolBar. In future
// KToolBar and KMenuBar and (future) KToolBox will inherit this class.
// This is not yet finished and present things do not depend on it
// Expect big changes here. Most of stuff comes from KToolBar,
// So it inherits it's copyrights.
// New stuff is: buttons with popups, delayed popups and smarter
// management (?)
// This notice will be removed later.

// $Id$
// $Log$

#include <qpainter.h>
#include <qtooltip.h> 
#include <qdrawutl.h>
#include <qpalette.h>
#include <qbitmap.h>
#include <qstring.h>
#include <qrect.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "kbasebar.h"
#include <ktopwidget.h>
#include <klocale.h>
#include <kapp.h>
#include <kwm.h>
#include <ktoolboxmgr.h>


// Use enums instead of defines. We are C++ and NOT C !
enum {
    CONTEXT_LEFT = 0,
    CONTEXT_RIGHT = 1,
    CONTEXT_TOP = 2,
    CONTEXT_BOTTOM = 3,
    CONTEXT_FLOAT = 4
};

// this should be adjustable (in faar future... )
#define MIN_AUTOSIZE 150

// delay im ms (microsoft seconds) before delayed popup pops up
#define POPUP_DELAY 500

KBaseBarItem::KBaseBarItem (Item *_item, itemType _type, int _id,
                            bool _myItem)
{
  id = _id;
  right=false;
  autoSized=false;
  type=_type;
  item = _item;
  myItem = _myItem;
}

KBaseBarItem::~KBaseBarItem ()
{
  // Delete this item if localy constructed
  if (myItem)
    delete item;
}

/*** A very important button which can be menuBarButton or toolBarButton ***/

KBaseBarButton::KBaseBarButton( const QPixmap& pixmap, int _id,
                                QWidget *_parent, const char *name,
                                int item_size, const char *txt,
                                bool _mb) : QButton( _parent, name )
{
  sep=false;
  delayPopup = false;
  parentWidget = (KBaseBar *) _parent;
  raised = false;

  myPopup = 0;
  toolBarButton = !_mb;
  
  setFocusPolicy( NoFocus );
  id = _id;
  if (txt)
  btext=txt;
  if ( ! pixmap.isNull() )
    enabledPixmap = pixmap;
  else
    {
      warning(klocale->translate("KBaseBarButton: pixmap is empty, perhaps some missing file"));
      enabledPixmap.resize( item_size-4, item_size-4);
    }
  modeChange ();
  makeDisabledPixmap();
  setPixmap( enabledPixmap );
  connect (parentWidget, SIGNAL( modechange() ), this, SLOT( modeChange() ));
  
  connect( this, SIGNAL( clicked() ), this, SLOT( ButtonClicked() ) );
  connect(this, SIGNAL( pressed() ), this, SLOT( ButtonPressed() ) );
  connect(this, SIGNAL( released() ), this, SLOT( ButtonReleased() ) );
}

KBaseBarButton::KBaseBarButton( QWidget *parentWidget, const char *name )
  : QButton( parentWidget , name)
{
  resize(6,6);
  hide();
  youreSeparator();
}

void KBaseBarButton::beToggle(bool flag)
{
  setToggleButton(flag);
  if (flag == true)
    connect (this, SIGNAL(toggled(bool)), this, SLOT(ButtonToggled()));
  else
    disconnect (this, SIGNAL(toggled(bool)), this, SLOT(ButtonToggled()));
}

void KBaseBarButton::on(bool flag)
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

void KBaseBarButton::toggle()
{
  setOn(!isOn());
  repaint();
}

void KBaseBarButton::setText( const char *text)
{
 btext = text;
 modeChange();
 repaint (false);
}

void KBaseBarButton::setPixmap( const QPixmap &pixmap )
{
  if ( ! pixmap.isNull() )
    enabledPixmap = pixmap;
  else {
    warning(klocale->translate("KBaseBarButton: pixmap is empty, perhaps some missing file"));
    enabledPixmap.resize(width()-4, height()-4);
  }
  QButton::setPixmap( enabledPixmap );
}            

void KBaseBarButton::setPopup (QPopupMenu *p)
{
  myPopup = p;
  p->installEventFilter (this);
}

void KBaseBarButton::setDelayedPopup (QPopupMenu *p)
{
  delayPopup = true;
  delayTimer = new QTimer (this);
  connect (delayTimer, SIGNAL(timeout ()), this, SLOT(slotDelayTimeout()));
  setPopup(p);
}

void KBaseBarButton::setEnabled( bool enabled )
{
  QButton::setPixmap( (enabled ? enabledPixmap : disabledPixmap) );
  QButton::setEnabled( enabled );
}


void KBaseBarButton::leaveEvent(QEvent *)
{
  if (isToggleButton() == false)
    if( raised != false )
    {
      raised = false;
      repaint(false);
    }
  if (delayPopup)
    delayTimer->stop();
}

void KBaseBarButton::enterEvent(QEvent *)
{
  if (highlight == 1)
    if (isToggleButton() == false)
      if ( isEnabled() )
      {
        raised = true;
        repaint(false);
      }
}

bool KBaseBarButton::eventFilter (QObject *o, QEvent *ev)
{
  if ((QPopupMenu *) o != myPopup)
    return false; // just in case
  
  switch (ev->type())
  {
    case Event_MouseButtonDblClick:
    case Event_MouseButtonPress:
      debug ("Got press | doubleclick");
      // If I get this, it means that popup is visible
      {
      QRect r(geometry());
      r.moveTopLeft(parentWidget->mapToGlobal(pos()));
      if (r.contains(QCursor::pos()))   // on button
        return true; // ignore
      }
      break;
    
    case Event_MouseButtonRelease:
      debug ("Got release");
      if (!myPopup->geometry().contains(QCursor::pos())) // not in menu...
      {
        QRect r(geometry());
        r.moveTopLeft(parentWidget->mapToGlobal(pos()));

        if (r.contains(QCursor::pos()))   // but on button
        {
          myPopup->setActiveItem(0 /*myPopup->idAt(1)*/); // set first active
          return true;  // ignore release
        }
      }
      break;

    case Event_Hide:
      debug ("Got hide");
      on(false);
      return false;
      break;
  }
  return false;
}



void KBaseBarButton::drawButton( QPainter *_painter )
{
  // We are not using KButton any more!
  
  // Dman, this has so meny returns that I have to do my job here:

  
  if ( raised )
  {
    if ( style() == WindowsStyle )
      qDrawWinButton( _painter, 0, 0, width(), height(), colorGroup(), false );
    else
      qDrawShadePanel( _painter, 0, 0, width(), height(), colorGroup(), false, 2, 0L );
  }

  else if ( isDown() || isOn() )
  {
    if ( style() == WindowsStyle )
      qDrawWinButton(_painter, 0, 0, width(), height(), colorGroup(), true );
    else
      qDrawShadePanel(_painter, 0, 0, width(), height(), colorGroup(), true, 2, 0L );
  }
  
  int dx, dy;
  
  
  if (icontext == 0) // icon only
  {
    if (pixmap())
    {
      dx = ( width() - pixmap()->width() ) / 2;
      dy = ( height() - pixmap()->height() ) / 2;
      if ( isDown() && style() == WindowsStyle )
      {
        ++dx;
        ++dy;
      }
      _painter->drawPixmap( dx, dy, *pixmap() );
      //return;
    }
  }
  else if (icontext == 1) // icon and text (if any)
  {
    if (pixmap())
    {
      dx = 1;
      dy = ( height() - pixmap()->height() ) / 2;
      if ( isDown() && style() == WindowsStyle )
      {
        ++dx;
        ++dy;
      }
      _painter->drawPixmap( dx, dy, *pixmap() );
    }

    if (!btext.isEmpty())
    {
      int tf = AlignVCenter|AlignLeft;
      if (!isEnabled())
        _painter->setPen(palette().disabled().dark());
      if (pixmap())
        dx= pixmap()->width();
      else
        dx= 1;
      if (toolBarButton)
        _painter->setFont(buttonFont);
      if(raised)
        _painter->setPen(blue);
      _painter->drawText(dx, 0, width()-dx, height(), tf, btext);
    }
    //return;
  }
  else if (icontext == 2) // only text, even if there is a icon
  {
    if (!btext.isEmpty())
    {
      int tf = AlignVCenter|AlignLeft;
      if (!isEnabled())
        _painter->setPen(palette().disabled().dark());
      dx= 1;
      if (toolBarButton)
        _painter->setFont(buttonFont);
      if(raised)
        _painter->setPen(blue);
      _painter->drawText(dx, 0, width()-dx, height(), tf, btext);
    }
    //return;
  }

  if (myPopup)
  {
    if (isEnabled())
      qDrawArrow (_painter, DownArrow, WindowsStyle, false,
                  width()-5, height()-5, 0, 0, colorGroup (), true);
    else
      qDrawArrow (_painter, DownArrow, WindowsStyle, false,
                  width()-5, height()-5, 0, 0, colorGroup (), false);
  }
}


void KBaseBarButton::makeDisabledPixmap()
{
  if (ImASeparator())
    return;             // No pixmaps for separators
  
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

void KBaseBarButton::paletteChange(const QPalette &)
{
  if(ImASeparator())  {
    makeDisabledPixmap();
    if ( !isEnabled() ) 
      QButton::setPixmap( disabledPixmap );
    repaint(false); // no need to delete it first therefore only false
  }
}

void KBaseBarButton::modeChange()
{
  int myWidth;

  myWidth = enabledPixmap.width();

  QFont fnt;
  
  if (toolBarButton)
  {
    buttonFont.setFamily("Helvetica");
    buttonFont.setPointSize(10);
    buttonFont.setBold(false);
    buttonFont.setItalic(false);
    buttonFont.setCharSet(font().charSet());
  
    fnt=buttonFont;
  }
  else
    fnt=kapp->generalFont;

  QFontMetrics fm(fnt);
  
  if (toolBarButton)
    icontext=parentWidget->icon_text;
  else
    icontext = 1;

  _size=parentWidget->item_size;
  if (myWidth < _size)
    myWidth = _size;

  highlight=parentWidget->highlight;
  if (icontext == 1) //Calculate my size
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

void KBaseBarButton::showMenu()
{
  // calculate that position carefully!!
  QPoint p (parentWidget->mapToGlobal(pos()));
  p.setY(p.y()+height());
  raised = true;
  repaint (false);
  myPopup->popup(p);
}

void KBaseBarButton::slotDelayTimeout()
{
  delayTimer->stop();
  showMenu ();
}

void KBaseBarButton::ButtonClicked()
{
  if (myPopup && !delayPopup)
    showMenu();
  else
    emit clicked( id );
}

void KBaseBarButton::ButtonPressed()
{
  if (myPopup)
  {
    if (delayPopup)
    {
      delayTimer->stop(); // just in case?
      delayTimer->start(POPUP_DELAY, true);
      return;
    }
    else
      showMenu();
  }
  else
    emit pressed( id );
}

void KBaseBarButton::ButtonReleased()
{
  // if popup is visible we don't get this
  // (gram of praxis weights more than ton of theory)
  if (myPopup && myPopup->isVisible())
    return;

  if (myPopup && delayPopup)
    delayTimer->stop();
  
  emit released( id );
}

void KBaseBarButton::ButtonToggled()
{
  emit toggled(id);
}

/****************************** Basebar **************************************/

KBaseBar::KBaseBar(QWidget *parent, const char *name, int _item_size)
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

void KBaseBar::ContextCallback( int index )
{
  int i = context->exec();
  switch ( i )
    {
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
      else
	{
	  setBarPos( Floating );
	  move(QCursor::pos());
	  show();
	}
      break;
    }
  
  mouseEntered=false;
  repaint(false);
}

KBaseBar::~KBaseBar()
{

  // what is that?! we do not need to recreate before
  // destroying.... (Matthias)
  //   if (position == Floating)
  //   {
  //     debug ("KBaseBar destructor: about to recreate");
  //     recreate (Parent, oldWFlags, QPoint (oldX, oldY), false);
  //     debug ("KBaseBar destructor: recreated");
  //   }

  // what is that?! toolbaritems are children of the toolbar, which
  // means, qt will delete them for us (Matthias)
  //for ( KBaseBarItem *b = items.first(); b!=0L; b=items.next() )
  // items.remove();

  if (!QApplication::closingDown())
    delete context;

  //debug ("KBaseBar destructor");
}


void KBaseBar::init()
{
  context = new QPopupMenu( 0, "context" );
  context->insertItem( klocale->translate("Left"), CONTEXT_LEFT );
  context->insertItem( klocale->translate("Top"),  CONTEXT_TOP );
  context->insertItem( klocale->translate("Right"), CONTEXT_RIGHT );
  context->insertItem( klocale->translate("Bottom"), CONTEXT_BOTTOM );
  context->insertItem( klocale->translate("Floating"), CONTEXT_FLOAT );
//   connect( context, SIGNAL( activated( int ) ), this,
// 	   SLOT( ContextCallback( int ) ) );
  
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

  mgr =0;
}

void KBaseBar::slotReadConfig()
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

void KBaseBar::drawContents ( QPainter *)
{
}


void KBaseBar::layoutHorizontal ()
{
  int offset=3+9+4; // = 16
  int rightOffset;
  int yOffset=1;
  KBaseBarItem *autoSize = 0;
  int mywidth;
  int widest=0;
  
  horizontal = true; // sven - 040198

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

  rightOffset=mywidth;
  toolbarHeight= item_size;

  for ( KBaseBarItem *b = items.first(); b; b=items.next() )
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

void KBaseBar::layoutVertical ()
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

  for ( KBaseBarItem *b = items.first(); b; b=items.next() )
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

void KBaseBar::updateRects( bool res )
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

void KBaseBar::mousePressEvent ( QMouseEvent *m )
{
  int ox, oy, ow, oh;
  if ((horizontal && m->x()<9) || (!horizontal && m->y()<9))
  {
    pointerOffset = m->pos();
    if (moving)
      if (m->button() != LeftButton)
	{
	  context->popup( mapToGlobal( m->pos() ), 0 );
	  ContextCallback(0);
	}
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
          QList<KBaseBarItem> ons;
          for (KBaseBarItem *b = items.first(); b; b=items.next())
          {
            if (b->isEnabled())
              ons.append(b);
            b->setEnabled(false);
          }
              
          mgr->doMove(true, false, false);

          for (KBaseBarItem *b = ons.first(); b; b=ons.next())
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
        debug ("KBaseBar: moving done");
      }
  }
}

void KBaseBar::slotHotSpot(int hs)
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
void KBaseBar::resizeEvent( QResizeEvent *)
{
  /* Newest - sven */
  
  if (position == Floating)  // are we floating? If yes...
    if (!localResize)        // call from updateRects? if *_NOT_*...
      updateRects(true);     // ...update (i.e. WM resized us)
}

void KBaseBar::paintEvent(QPaintEvent *e)
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
      if (style() == MotifStyle)
        qDrawShadePanel( paint, 0, 0, 9, toolbarHeight,
                         g , false, 1, &b);
      else
        qDrawPlainRect ( paint, 0, 0, 9, toolbarHeight,
                         g.mid(), 1, &b);

      paint->setPen( g.light() );
      if (style() == MotifStyle)
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
      if (style() == MotifStyle)
        qDrawShadePanel( paint, 0, 0, toolbarWidth, 9,
                         g , false, 1, &b);
      else
        qDrawPlainRect( paint, 0, 0, toolbarWidth, 9,
                        g.mid(), 1, &b);

      paint->setPen( g.light() );
      if (style() == MotifStyle)
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

void KBaseBar::closeEvent (QCloseEvent *e)
{
  if (position == Floating)
   {
     setBarPos(lastPosition);
     e->ignore();
     return;
   }
  e->accept();
}

void KBaseBar::leaveEvent (QEvent *)
{
    mouseEntered = false;
    repaint();
}

void KBaseBar::mouseMoveEvent(QMouseEvent* mev)
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

void KBaseBar::mouseReleaseEvent ( QMouseEvent * )
{
  if (mgr)
    {
      debug ("KBaseBar: mouseRelease event (stoping mgr)");
      mgr->stop();
    }
  else
    debug ("KBaseBar: mouseRelease event (no mgr)");
}

/********************* I N T E R F A C E  *********************/

/// Inserts a button.
int KBaseBar::insertButton( const QPixmap& pixmap, int id, bool enabled,
			    const char *_text, int index )
{
  KBaseBarButton *button = new KBaseBarButton( pixmap, id, this, 0L, item_size,
                                               _text);
  KBaseBarItem *item = new KBaseBarItem(button, ITEM_BUTTON, id,
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
int KBaseBar::insertButton( const QPixmap& pixmap, int id, const char *signal,
			    const QObject *receiver, const char *slot, bool enabled,
			    const char *_text, int index )
{
  KBaseBarButton *button = new KBaseBarButton( pixmap, id, this,
                                               0L, item_size, _text);
  KBaseBarItem *item = new KBaseBarItem(button, ITEM_BUTTON, id,
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

/// Inserts a button with popup.
int KBaseBar::insertButton( const QPixmap& pixmap, int id, QPopupMenu *_popup,
                            bool enabled, const char *_text, int index)
{
  KBaseBarButton *button = new KBaseBarButton( pixmap, id, this,
                                               0L, item_size, _text);
  KBaseBarItem *item = new KBaseBarItem(button, ITEM_BUTTON, id,
                                        true);
  button->setPopup(_popup);

  if ( index == -1 ) 
    items.append( item );
  else
    items.insert( index, item );

  item->setEnabled( enabled );
  item->show();
  
  if (position == Floating)
    updateRects( true );
  else if (isVisible())
    emit (moved(position));
  return items.at();
}

/// Inserts separator
int KBaseBar::insertSeparator( int index )
{
  KBaseBarButton *separ = new KBaseBarButton( this );
  KBaseBarItem *item = new KBaseBarItem(separ, ITEM_BUTTON, -1,
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

/// Removes item by ID
void KBaseBar::removeItem (int id)
{
  for (KBaseBarItem *b = items.first(); b; b=items.next())
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

void KBaseBar::clear()
{
  items.clear();
  haveAutoSized=false;
  
  if (position == Floating)
    updateRects( true );
  else if (isVisible())
    emit (moved(position));
}

int KBaseBar::accel (int id)
{

}

void KBaseBar::setAccel (int key, int id)
{

}


void KBaseBar::showItem (int id)
{
  for (KBaseBarItem *b = items.first(); b; b=items.next())
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

void KBaseBar::hideItem (int id)
{
  for (KBaseBarItem *b = items.first(); b; b=items.next())
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
void KBaseBar::addConnection (int id, const char *signal,
                              const QObject *receiver, const char *slot)
{
  for (KBaseBarItem *b = items.first(); b; b=items.next())
    if (b->ID() == id )
           connect (b->getItem(), signal, receiver, slot);
}

/// Common
void KBaseBar::setItemEnabled( int id, bool enabled )
{
  for (KBaseBarItem *b = items.first(); b; b=items.next())
    if (b->ID() == id )
      b->setEnabled(enabled);
}

bool  KBaseBar::isItemEnabled( int id )
{
  for (KBaseBarItem *b = items.first(); b; b=items.next())
    if (b->ID() == id )
      return b->isEnabled();
}



void KBaseBar::setItemAutoSized ( int id, bool enabled )
{
  for (KBaseBarItem *b = items.first(); b; b=items.next())
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

void KBaseBar::alignItemRight(int id, bool yes)
{
  for (KBaseBarItem *b = items.first(); b; b=items.next())
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
void KBaseBar::setButtonPixmap( int id, const QPixmap& _pixmap )
{
  for (KBaseBarItem *b = items.first(); b; b=items.next())
    if (b->ID() == id )
      ((KBaseBarButton *) b->getItem())->setPixmap( _pixmap );
}

void KBaseBar::setButtonText( int id, const char *text )
{
  for (KBaseBarItem *b = items.first(); b; b=items.next())
    if (b->ID() == id )
      ((KBaseBarButton *) b->getItem())->setText(text);
}

void KBaseBar::setDelayedPopup (int id , QPopupMenu *_popup)
{
  for (KBaseBarItem *b = items.first(); b; b=items.next())
    if (b->ID() == id )
      ((KBaseBarButton *) b->getItem())->setDelayedPopup(_popup);
}

/// Toggle buttons
void KBaseBar::setToggle ( int id, bool yes )
{
  for (KBaseBarItem *b = items.first(); b; b=items.next())
    if (b->ID() == id )
    {
      ((KBaseBarButton *) b->getItem())->beToggle(yes);
      connect (b->getItem(), SIGNAL(toggled(int)),
               this, SLOT(ButtonToggled(int)));
    }
}

void KBaseBar::toggleButton (int id)
{
  for (KBaseBarItem *b = items.first(); b; b=items.next())
    if (b->ID() == id )
    {
      if (((KBaseBarButton *) b->getItem())->isToggleButton() == true)
        ((KBaseBarButton *) b->getItem())->toggle();
    }
}

void KBaseBar::setButton (int id, bool on)
{
  for (KBaseBarItem *b = items.first(); b; b=items.next())
    if (b->ID() == id )
      ((KBaseBarButton *) b->getItem())->on(on);
}

bool KBaseBar::isButtonOn (int id)
{
  for (KBaseBarItem *b = items.first(); b; b=items.next())
    if (b->ID() == id )
    {
      if (((KBaseBarButton *) b->getItem())->isToggleButton() == true)
        return ((KBaseBarButton *) b->getItem())->isOn();
    }
  return false;
}

KBaseBarButton* KBaseBar::getButton( int id )
{
  for( KBaseBarItem* b = items.first(); b != NULL; b = items.next() )
    if(b->ID() == id )
      return ((KBaseBarButton *) b->getItem());
  return 0;
}

/************************** T O O L S *************************/

void KBaseBar::setMaxHeight (int h)
{
  max_height = h;
}

void KBaseBar::setMaxWidth (int w)
{
  max_width = w;
  updateRects(true);
}

void KBaseBar::setFullWidth(bool flag)
{
  fullWidth = flag;
}

void KBaseBar::enableMoving(bool flag)
{
  moving = flag; 
}

void KBaseBar::setBarPos(BarPosition bpos)
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

void KBaseBar::enableFloating (bool arrrrrrgh)
{
    context->setItemEnabled (CONTEXT_FLOAT, arrrrrrgh);
}


bool KBaseBar::enable(BarStatus stat)
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

QSize KBaseBar::sizeHint ()
{
  int w=16;
  
  for (KBaseBarItem *b = items.first(); b; b=items.next())
    {
      w+=b->isAuto()?MIN_AUTOSIZE:b->width();
      w+=3;
    }
  szh.setWidth(w);

  return szh;
}

int KBaseBar::heightForWidth (int w)
{

}

int KBaseBar::widthForHeight (int h)
{

}

void KBaseBar::ButtonClicked(int id)
{
  emit clicked(id);
}

void KBaseBar::ButtonPressed(int id)
{
  emit pressed(id);
}

void KBaseBar::ButtonReleased(int id)
{
  emit released(id);
}

void KBaseBar::ButtonToggled(int id)
{
  emit toggled(id);
}

// sven
#include <kbasebar.moc>
