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

// $Id$
// $Log$
//
// Revision 1.104  1999/03/04 17:49:17  ettrich
// more fixes for Qt-2.0
//
// Revision 1.103  1999/03/02 15:56:40  kulow
// CVS_SILENT replacing klocale->translate with i18n
//
// Revision 1.102  1999/03/01 23:35:24  kulow
// CVS_SILENT ported to Qt 2.0
//
// Revision 1.101  1999/02/21 20:21:20  radej
// sven: added default to avoid warning about something not handled in a switch
//
// Revision 1.100  1999/02/11 19:27:20  koss
// reimported because of wrong commit
//
// Revision 1.97  1999/02/10 19:12:18  koss
// Added insertLineSeparator() - convenience function
//
// Revision 1.96  1999/01/18 10:57:11  kulow
// .moc files are back in kdelibs. Built fine here using automake 1.3
//
// Revision 1.95  1999/01/15 09:31:30  kulow
// it's official - kdelibs builds with srcdir != builddir. For this I
// automocifized it, the generated rules are easier to maintain than
// selfwritten rules. I have to fight with some bugs of this tool, but
// generally it's better than keeping them updated by hand.
//
// Revision 1.94  1998/12/09 13:44:30  radej
// sven: iconify() -> hide() even when floating. Commented out debug output.
//
// Revision 1.93  1998/12/02 16:08:26  radej
// sven: hide toolbar items better when session management wants toolbar flat
//
// Revision 1.92  1998/11/21 19:27:19  radej
// sven: doubleClicked signal for buttons.
//
// Revision 1.91  1998/11/18 01:00:02  radej
// sven: set*BarPos(Flat) works now (I hope)
//
// Revision 1.90  1998/11/11 14:32:08  radej
// sven: *Bars can be made flat by MMB (Like in Netscape, but this works)
//
// Revision 1.89  1998/11/10 14:12:46  radej
// sven: windows-style handle smaller
//
// Revision 1.88  1998/11/09 17:58:34  radej
// sven: Fix for IconText=3 (pixmap wider then text; kfm's wheel)
//
// Revision 1.87  1998/11/09 00:29:16  radej
// sven: IconText 3 (text under pixmap)
//
// Revision 1.86  1998/11/06 16:48:20  radej
// sven: nicer docking, some bugfixes
//
// Revision 1.85  1998/11/06 15:08:48  radej
// sven: finished handles. Comments?
//
// Revision 1.84  1998/11/06 12:54:52  radej
// sven: radioGroup is in. handle changed again (broken in vertical mode)
//
// Revision 1.83  1998/11/05 18:23:31  radej
// sven: new look for *Bar handles (unfinished)
//
// Revision 1.82  1998/10/10 11:28:53  radej
// sven: Fixed popup position (if too close to bottom of screen, pops above)
//
// Revision 1.81  1998/10/09 12:42:19  radej
// sven: New: (un) highlight sugnals, Autorepeat buttons, button down when
//       pressed. kdetest/kwindowtest updated. This is Binary COMPATIBLE.
//
// Revision 1.80  1998/10/05 15:09:53  kulow
// purify (and me) likes initialized members, so I choose one (like the compiler
// would do :)
//
// Revision 1.79  1998/09/15 05:56:45  antlarr
// I've added a setIconText function to change the state of a variable
// in KToolBar
//
// Revision 1.78  1998/09/01 20:22:23  kulow
// I renamed all old qt header files to the new versions. I think, this looks
// nicer (and gives the change in configure a sense :)
//
// Revision 1.77  1998/08/30 21:04:33  radej
// sven: Minor improvement for docking in KTM
//
// Revision 1.76  1998/08/09 17:49:28  radej
// sven: fixed a bug - uninitialized toolbarHeight/Width - thanks to Harry Porten
//
// Revision 1.75  1998/08/09 14:01:17  radej
// sven: reintroduced makeDisabledPixmap code, and dumped QIconSet. Fixed a bug
//       with paletteChange too.
//
// Revision 1.74  1998/08/06 15:39:01  radej
// sven: Popups & delayedPopups. Uses QIconSet. Needs Qt-1.4x
//
// Revision 1.73  1998/07/29 12:48:29  ssk
// Removed more warnings, possible portability problems and ANSI violations.
//
// Revision 1.72  1998/07/23 09:42:26  radej
// sven: improved styles
//
// Revision 1.71  1998/06/18 13:41:42  radej
// sven: fixed invisible disabled buttons (I already wanted to defend it as
//       a feature, like You don't have to see it, it's disabled!)
//
// Revision 1.70  1998/06/18 08:56:57  radej
// sven: removed debug output.
//
// Revision 1.69  1998/06/17 12:58:30  radej
// sven: Removed compiler warning.
//
// Revision 1.68  1998/06/16 17:20:46  radej
// sven: Fixed Warnings when switching palettes (separators hav no pixmaps)
//
// Revision 1.67  1998/05/19 14:10:05  radej
// Bugfixes: Unhighlighting a handle and catching the fast click
//
// Revision 1.66  1998/05/13 09:56:31  radej
// Changelog order
//
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
// Bugfixes for moving + opaque moving
//
// Revision 1.60  1998/05/02 18:31:01  radej
// Improved docking
//
// Revision 1.59  1998/04/28 09:17:28  radej
// New moving and docking BINARY INCOMPATIBLE
//
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
#include <qtooltip.h> 
// Fixed white bg on disabled buttons in icontext mode and
// positioning on signal appearanceChanged - sven 24.3.1998
//-------------------------------------------------------------------------


#include <qpainter.h>
#include <qtooltip.h>
#include <qdrawutil.h>
#include <qpalette.h>
#include <qbitmap.h>
#include <qstring.h>
#include <qframe.h>
#include <qbutton.h>
#include <qrect.h>
//#include <qimage.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "ktoolbar.h"
#include "klined.h"
#include "kseparator.h"
#include <ktopwidget.h>
#include <klocale.h>
#include <kapp.h>
#include <kwm.h>
#include <ktoolboxmgr.h>
#include <kstyle.h>

    CONTEXT_FLOAT = 4
enum {
    CONTEXT_LEFT = 0,
    CONTEXT_RIGHT = 1,
    CONTEXT_TOP = 2,
    CONTEXT_BOTTOM = 3,
    CONTEXT_FLOAT = 4,
    CONTEXT_FLAT = 5
};

// this should be adjustable (in faar future... )
#define MIN_AUTOSIZE 150

// delay im ms (microsoft seconds) before delayed popup pops up
#define POPUP_DELAY 500


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

/*** A very important button which can be menuBarButton or toolBarButton ***/

KToolBarButton::KToolBarButton( QWidget *parentWidget, const char *name )
  : QButton( parentWidget , name)
{
  resize(6,6);
  hide();
  youreSeparator();
  radio = false;
}

KToolBarButton::KToolBarButton( const QPixmap& pixmap, int _id,
                                QWidget *_parent, const char *name,
                                int item_size, const QString& txt,
                                bool _mb) : QButton( _parent, name )
{
  
  delayPopup = false;
  parentWidget = (KToolBar *) _parent;
  if (txt)
  myPopup = 0L;
  radio = false;
  toolBarButton = !_mb;

  setFocusPolicy( NoFocus );
  id = _id;
    warning(i18n("KToolBarButton: pixmap is empty, perhaps some missing file"));
    btext = txt;
  if ( ! pixmap.isNull() )
    enabledPixmap = pixmap;
  else
  {
    // How about jumping to text mode if no pixmap? (sven)
    warning("KToolBarButton: pixmap is empty, perhaps some missing file");
    enabledPixmap.resize( item_size-4, item_size-4);
  }
  modeChange ();
  makeDisabledPixmap();
  setPixmap(enabledPixmap);

  connect (parentWidget, SIGNAL( modechange() ), this, SLOT( modeChange() ));
  connect( this, SIGNAL( clicked() ), this, SLOT( ButtonClicked() ) );
  connect(this, SIGNAL( pressed() ), this, SLOT( ButtonPressed() ) );
  connect(this, SIGNAL( released() ), this, SLOT( ButtonReleased() ) );
  installEventFilter(this);
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

void KToolBarButton::toggle()
{
  setOn(!isOn());
  repaint();
}
void KToolBarButton::setText( const QString& text)
{
  btext = text;
  modeChange();
  repaint (false);
    warning(i18n("KToolBarButton: pixmap is empty, perhaps some missing file"));

void KToolBarButton::setPixmap( const QPixmap &pixmap )
{
  if ( ! pixmap.isNull() )
    enabledPixmap = pixmap;
  else {
    warning("KToolBarButton: pixmap is empty, perhaps some missing file");
    enabledPixmap.resize(width()-4, height()-4);
  }
  QButton::setPixmap( enabledPixmap );
}


void KToolBarButton::setPopup (QPopupMenu *p)
{
  myPopup = p;
  p->installEventFilter (this);
}

void KToolBarButton::setDelayedPopup (QPopupMenu *p)
{
  delayPopup = true;
  delayTimer = new QTimer (this);
  connect (delayTimer, SIGNAL(timeout ()), this, SLOT(slotDelayTimeout()));
  setPopup(p);
}

void KToolBarButton::setEnabled( bool enabled )
{
  QButton::setPixmap( (enabled ? enabledPixmap : disabledPixmap) );
  QButton::setEnabled( enabled );

}


void KToolBarButton::leaveEvent(QEvent *)
{
  if (isToggleButton() == false)
    if( raised != false )
    {
      raised = false;
      repaint(false);
    }
  if (delayPopup)
    delayTimer->stop();

  emit highlighted (id, false);
}

void KToolBarButton::enterEvent(QEvent *)
{
  if (highlight == 1)
  {
    if (isToggleButton() == false)
      if (isEnabled())
        raised = true;

    repaint(false);
  }
  emit highlighted (id, true);
}

void KToolBarButton::setRadio (bool f)
{ /*
  if (f && !radio)  // if was not and now is
    installEventFilter(this);
  else if (!f && radio) // if now isn't and was (man!)
    removeEventFilter(this);
  */
  radio = f;

}

bool KToolBarButton::eventFilter (QObject *o, QEvent *ev)
{
      
  if ((KToolBarButton *)o == this && ev->type () == QEvent::MouseButtonDblClick)
  {
    //debug ("Doubleclick");
    emit doubleClicked (id);
    return true;
  }

  if ((KToolBarButton *) o == this)
  
         ev->type() == QEvent::MouseButtonRelease ||
         ev->type() == QEvent::MouseButtonDblClick) && radio && isOn())
      return true;

  if ((QPopupMenu *) o != myPopup)
    return false; // just in case

  switch (ev->type())
  {
    case QEvent::MouseButtonDblClick:
    case QEvent::MouseButtonPress:
      //debug ("Got press | doubleclick");
      // If I get this, it means that popup is visible
    
      QRect r(geometry());
      r.moveTopLeft(parentWidget->mapToGlobal(pos()));
      if (r.contains(QCursor::pos()))   // on button
        return true; // ignore
      }
      break;

    case QEvent::MouseButtonRelease:
      //debug ("Got release");
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

    case QEvent::Hide:
      //debug ("Got hide");
      on(false);
      return false;
      break;
  default:
      break;
  }

    kapp->kstyle()->drawKToolBarButton(_painter, 0, 0, width(), height(),
      isEnabled()? colorGroup() : palette().disabled(), isDown() || isOn(),
      raised, isEnabled(), myPopup != NULL, iconType, btext, pixmap(),
      &buttonFont);
    return;
  }
    
  if ( isDown() || isOn() )
  {
    if ( style() == WindowsStyle )
      qDrawWinButton(_painter, 0, 0, width(), height(), colorGroup(), true );
    else
      qDrawShadePanel(_painter, 0, 0, width(), height(), colorGroup(), true, 2, 0L );
  }

  
  {
  
  
    else
      qDrawShadePanel( _painter, 0, 0, width(), height(), colorGroup(), false, 2, 0L );
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

    if (!btext.isNull())
    {
      int tf = AlignVCenter|AlignLeft;
      if (!isEnabled())
        _painter->setPen(palette().disabled().dark());
      if (pixmap())
        dx= pixmap()->width();
      else
      
      dy = 0;
      if ( isDown() && style() == WindowsStyle )
      {
        ++dx;
        ++dy;
      }

      if (toolBarButton)
        _painter->setFont(buttonFont);
      if(raised)
        _painter->setPen(blue);
      _painter->drawText(dx, dy, width()-dx, height(), tf, btext);
    }
  }
  else if (icontext == 2) // only text, even if there is a icon
  {
    if (!btext.isNull())
    {
      int tf = AlignVCenter|AlignLeft;
      if (!isEnabled())
        _painter->setPen(palette().disabled().dark());
      dx= 1;
      dy= 0;
      if ( isDown() && style() == WindowsStyle )
      {
        ++dx;
        ++dy;
      }

      if (toolBarButton)
        _painter->setFont(buttonFont);
      if(raised)
        _painter->setPen(blue);
      _painter->drawText(dx, dy, width()-dx, height(), tf, btext);
    }
  }
  else if (icontext == 3)
  {
    if (pixmap())
    {
      dx = (width() - pixmap()->width()) / 2;
      dy = 1;
      if ( isDown() && style() == WindowsStyle )
      {
        ++dx;
        ++dy;
      }
      _painter->drawPixmap( dx, dy, *pixmap() );
    }

      
    {
      int tf = AlignBottom|AlignHCenter;
      if (!isEnabled())
        _painter->setPen(palette().disabled().dark());
      dy= pixmap()->height();
      

      if ( isDown() && style() == WindowsStyle )
      {
        ++dx;
        ++dy;
      }

      if (toolBarButton)
  
      if(raised)
        _painter->setPen(blue);
      _painter->drawText(0, 0, width(), height()-4, tf, btext);
    }
  }
//#warning What about Icontext=3

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

void KToolBarButton::paletteChange(const QPalette &)
{
  if(!ImASeparator())
  {
    makeDisabledPixmap();
    if ( !isEnabled() )
      QButton::setPixmap( disabledPixmap );
    else
      QButton::setPixmap( enabledPixmap );
    repaint(false); // no need to delete it first therefore only false
  }
}

void KToolBarButton::modeChange()
{
  int myWidth;

  myWidth = enabledPixmap.width();

  QFont fnt;

  //Jesus, I must have been drunk...
  
  {
    buttonFont.setFamily("Helvetica");
    buttonFont.setPointSize(10);
    fnt=kapp->generalFont;
    buttonFont.setItalic(false);
    buttonFont.setCharSet(font().charSet());
  
    fnt=buttonFont;
  }
  else
    fnt=kapp->generalFont();

  QFontMetrics fm(fnt);

  if (toolBarButton)
    icontext=parentWidget->icon_text;
  else
    icontext = 1;

  _size=parentWidget->item_size;
  if (myWidth < _size)
    myWidth = _size;

// Nice but doesn't work
/*
  if (_size > 28)
  {
    double factor=_size/26;
    QImage i;
  
    i = i.smoothScale(enabledPixmap.width()*factor,
                      enabledPixmap.height()*factor);
    enabledPixmap.resize (i.width(), i.height());
    enabledPixmap.convertFromImage(i );
  }
*/

  highlight=parentWidget->highlight;
    
  {
  case 0:
    QToolTip::remove(this);
    QToolTip::add(this, btext);
    
    break;

  case 1:
    QToolTip::remove(this);
    
    break;

  case 2:
    QToolTip::remove(this);
    resize (fm.width(btext)+6, _size-2); // +2+_size-2
    break;

  case 3:
    QToolTip::remove(this);
    resize ((fm.width(btext)+6>myWidth)?fm.width(btext)+6:myWidth, _size-2);
    break;
  }

  /*
  if (icontext > 0) //Calculate my size
  {
    QToolTip::remove(this);
    resize (fm.width(btext)+myWidth, _size-2); // +2+_size-2
  }
  
  {
    QToolTip::remove(this);
    QToolTip::add(this, btext);
    resize (myWidth, _size-2);
  }
  */

}

void KToolBarButton::makeDisabledPixmap()
{
  
    return;             // No pixmaps for separators

  QPalette pal = palette();
  QColorGroup g = pal.disabled();

  // Prepare the disabledPixmap for drawing
    mask = new QBitmap(enabledPixmap.createHeuristicMask());
    allocated = true;
  } 
  
  QBitmap bitmap = *mask; // YES! make a DEEP copy before setting the mask!   
  bool allocated = false;
  
      mask = new QBitmap(enabledPixmap.createHeuristicMask());
      allocated = true;
  }

  QBitmap bitmap = *mask; // YES! make a DEEP copy before setting the mask!
  bitmap.setMask(*mask);

  
  p.setPen( g.mid() );
  p.drawPixmap(0, 0, bitmap);
  p.end();

  // For KStyle mask the pixmap, otherwise bg pixmaps get overwritten (mosfet)
  if(kapp->kstyle())
      disabledPixmap.setMask(*mask);
  if (allocated) // This shouldn't occur anymore!
    delete mask;
}

void KToolBarButton::showMenu()
{
  // calculate that position carefully!!
  raised = true;
  repaint (false);
  QPoint p (parentWidget->mapToGlobal(pos()));
  if (p.y() + height() + myPopup->height() > KApplication::desktop()->height())
    p.setY(p.y() - myPopup->height());
  else
    p.setY(p.y()+height());
  myPopup->popup(p);
}

void KToolBarButton::slotDelayTimeout()
{
  delayTimer->stop();
  showMenu ();
}

void KToolBarButton::ButtonClicked()
{
  if (myPopup && !delayPopup)
    showMenu();
  else
    emit clicked( id );
}

void KToolBarButton::ButtonPressed()
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

void KToolBarButton::ButtonReleased()
{
  
  // (gram of praxis weights more than ton of theory)
  //if (myPopup && myPopup->isVisible())
  //  return;

  if (myPopup && delayPopup)
    delayTimer->stop();

  emit released( id );
}

void KToolBarButton::ButtonToggled()
{
  emit toggled(id);
}
/****************************** Tolbar **************************************/

KToolBar::KToolBar(QWidget *parent, const char *name, int _item_size)
  : QFrame( parent, name )
{
  title = 0;
  item_size = _item_size;
  fixed_size =  (item_size > 0);
  item_size = 26;
  init();
  Parent = parent;        // our father
  max_width=-1;
  mouseEntered=false;
  localResize=false;
  buttonDownOnHandle = FALSE;
}

void KToolBar::ContextCallback( int )
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
	  move(QCursor::pos());
  
	}
      break;
    case CONTEXT_FLAT:
        setFlat (position != Flat);
	break;
    }

  mouseEntered=false;
  repaint(false);
}

void KToolBar::init()
  context = new QPopupMenu( 0, "context" );
  context->insertItem( i18n("Left"), CONTEXT_LEFT );
  
  context->insertItem( i18n("Right"), CONTEXT_RIGHT );
  context->insertItem( i18n("Bottom"), CONTEXT_BOTTOM );
  
  context->insertItem( i18n("Flat"), CONTEXT_FLAT );
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
  transparent = false;
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

  if (!fixed_size && tsize != item_size && tsize>20)
  {
    item_size = tsize;
    doUpdate=true;
  }

  
  {
    if (icontext==3)
      item_size = (item_size<40)?40:item_size;
    icon_text=icontext;
    doUpdate=true;
  }

  if (_highlight != highlight)
  {
    highlight = _highlight;
    doUpdate=true;
  

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

void KToolBar::drawContents ( QPainter *)
{
}

  
{

// what is that?! we do not need to recreate before
// destroying.... (Matthias)

  // OK (sven)

//   if (position == Floating)
//   {
//     debug ("KToolBar destructor: about to recreate");
//     recreate (Parent, oldWFlags, QPoint (oldX, oldY), false);
//     debug ("KToolBar destructor: recreated");
  
  
  // what is that?! toolbaritems are children of the toolbar, which
  // means, qt will delete them for us (Matthias)
  //for ( KToolBarItem *b = items.first(); b!=0L; b=items.next() )
  
  //Uhh... I'm embaresd... (sven)


  // I would never guess that (sven)
  if (!QApplication::closingDown())
    delete context;

  //debug ("KToolBar destructor");
}

void KToolBar::setMaxHeight (int h)
{
  max_height = h;
}

void KToolBar::setMaxWidth (int w)
{
  max_width = w;
  updateRects(true);
}

void KToolBar::layoutHorizontal ()
  
  int offset=3+9+4; // = 16
  int rightOffset;
  int yOffset=1;
  
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
           
              yOffset += item_size;
              toolbarHeight += item_size;
           
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
 
  int yOffset=3;
  int widest;

  

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
      
      break;

    case Left:
    case Right:
      layoutVertical ();
      break;

    default:
      return;
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
void KToolBar::mousePressEvent ( QMouseEvent *m )
   that it really works. "Worse?", I hear you asking. Yes, even worse indeed
   it is, because I lost my mind trying to get rid of excess resizes, and now
  if ((horizontal && m->x()<9) || (!horizontal && m->y()<9))
  {
    pointerOffset = m->pos();
    if (moving)
      if (m->button() == RightButton)
	{
	  context->popup( mapToGlobal( m->pos() ), 0 );
	  ContextCallback(0);
        }
      else if (m->button() == MidButton && position != Floating)
        setFlat (position != Flat);
      else if (position != Flat)
        mouseEntered = true;
        //QRect rr(KWM::geometry(Parent->winId(), false));
        QRect rr(Parent->geometry());
        ox = rr.x();
        oy = rr.y();
        ow = rr.width();
        oh = rr.height();
        if (Parent->inherits("KTMainWindow"))
        {
          ox += ((KTopLevelWidget *) Parent)->view_left;
          oy += ((KTopLevelWidget *) Parent)->view_top;
          ow = ((KTopLevelWidget *) Parent)->view_right -
            ((KTopLevelWidget *) Parent)->view_left;
          oh = ((KTopLevelWidget *) Parent)->view_bottom -
            ((KTopLevelWidget *) Parent)->view_top;;
        }
            
        int  fat = 25; //ness
    {
        mgr = new KToolBoxManager(this, transparent);
{
        //Firt of all discover _your_ position

        if (position == Top )
          mgr->addHotSpot(geometry(), true);             // I'm on top
        else
          mgr->addHotSpot(rr.x(), oy, rr.width(), fat); // top
  
        if (position == Bottom)
          mgr->addHotSpot(geometry(), true);           // I'm on bottom
        else
          mgr->addHotSpot(rr.x(), oy+oh-fat, rr.width(), fat); // bottom
  
        if (position == Left)
          mgr->addHotSpot(geometry(), true);           // I'm on left
        else
          mgr->addHotSpot(ox, oy, fat, oh); // left

        if (position == Right)
          mgr->addHotSpot(geometry(), true);           // I'm on right
        else
          mgr->addHotSpot(ox+ow-fat, oy, fat, oh); //right
        /*
        mgr->addHotSpot(ox, oy, ow, fat);           // top
        mgr->addHotSpot(ox, oy+oh-fat, ow, fat);    // bottom
        mgr->addHotSpot(ox, oy+fat, fat, oh-2*fat); // left
        mgr->addHotSpot(ox+ow-fat, oy+fat, fat, oh-2*fat); //right
        */
        movePos = position;
        connect (mgr, SIGNAL(onHotSpot(int)), SLOT(slotHotSpot(int)));
        if (transparent)
          mgr->doMove(true, false, true);
        else
        {
          /*
          QList<KToolBarItem> ons;
          for (KToolBarItem *b = items.first(); b; b=items.next())
          {
            if (b->isEnabled())
              ons.append(b);
            b->setEnabled(false);
          }
          */
          mgr->doMove(true, false, false);
          /*
          for (KToolBarItem *b = ons.first(); b; b=ons.next())
            b->setEnabled(true);
          */
    
        if (transparent)
        {
          setBarPos (movePos);

          if (movePos == Floating)
            move (mgr->x(), mgr->y());
          if (!isVisible())
            show();
        }
        mouseEntered = false;
        delete mgr;
        mgr=0;
        repaint (false);
        //debug ("KToolBar: moving done");
      }
  }
	{
	    context->popup( mapToGlobal( m->pos() ), 0 );
	    buttonDownOnHandle = FALSE;
	    ContextCallback(0);
        }
      else if (m->button() == MidButton && position != Floating)
	  setFlat (position != Flat);
}

void KToolBar::slotHotSpot(int hs)
{
  if (mgr == 0)
    return;
  if (!transparent) // opaque
  {
    switch (hs)
    {
      
        setBarPos(Top);
        break;

      
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
  
    }
  }
}
void KToolBar::resizeEvent( QResizeEvent *)
{
  /* Newest - sven */

  if (position == Floating)  // are we floating? If yes...
    if (!localResize)        // call from updateRects? if *_NOT_*...
      updateRects(true);     // ...update (i.e. WM resized us)
  

void KToolBar::paintEvent(QPaintEvent *)
{
  if (mgr)
    return;
  

  // This code should be shared with the aequivalent in kmenubar!
  toolbarHeight = height ();
  QBrush b;
  if (mouseEntered && highlight)
                                             colorGroup(), false, &b);
      }
    // Took higlighting handle from kmenubar - sven 040198
    QBrush b;
    if (mouseEntered && highlight)
      b = kapp->selectColor; // this is much more logical then
                            // the hardwired value used before!!
    else
      b = QWidget::backgroundColor();

  
      paint->end();
      delete paint;
      return;
  }

  if (moving)
  {
    // Handle point
    if (horizontal)
    {
      if (style() == MotifStyle)
      {
        qDrawShadePanel( paint, 0, 0, 9, toolbarHeight,
                         g , false, 1, &b);
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
      else // Windows style handle
      {
        int w = 6;
        int h = toolbarHeight;
        paint->setClipRect(0, 2, w, h-4);

        qDrawPlainRect ( paint, 0, 0, 9, toolbarHeight,
                         g.mid(), 0, &b);

        paint->setPen( g.light() );
        int a=0-w;
        while (a <= h+5)
        {
          paint->drawLine(0, h-a, h, 0-a);
          paint->drawLine(0, h-a+1, h, 0-a+1);
          a +=6;
        }
        a=0-w;
        paint->setPen( g.dark() );
        while (a <= h+5)
        {
          paint->drawLine(0, h-a+2, h, 0-a+2);
          paint->drawLine(0, h-a+3, h, 0-a+3);
          a +=6;
        }
      }
      
    else // vertical
    {
      if (style() == MotifStyle)
      {
        qDrawShadePanel( paint, 0, 0, toolbarWidth, 9,
                         g , false, 1, &b);

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
        
        }
      }
      else
      {
        qDrawPlainRect( paint, 0, 0, toolbarWidth, 9,
                        g.mid(), 0, &b);

        int w = toolbarWidth;
        int h = 15;

        paint->setClipRect(2, 0, w-4, 6);

        //qDrawPlainRect ( paint, 0, 0, 9, toolbarHeight,
        //                 g.mid(), 0, &b);

        paint->setPen( g.light() );
        int a = 0-h;
        while (a <= w+h)
        {
          paint->drawLine(w-a, h, w-a+h, 0);
          paint->drawLine(w-a+1, h, w-a+1+h, 0);
          a +=6;
        }
        a = 0-h;
        paint->setPen( g.dark() );
        while (a <= w+h)
        {
          paint->drawLine(w-a+2, h, w-a+2+h, 0);
          paint->drawLine(w-a+3, h, w-a+3+h, 0);
          a +=6;
        }
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

void KToolBar::ButtonDblClicked( int id )
{
  emit doubleClicked( id );
}

void KToolBar::ButtonPressed( int id )
{
  emit pressed( id );
}

void KToolBar::ButtonReleased( int id )
{
  emit released( id );
}

void KToolBar::ButtonToggled( int id )
{
  emit toggled( id );
}

void KToolBar::ButtonHighlighted(int id, bool on )
{
  emit highlighted(id, on);
}


 /********************\
 *                    *
 * I N T E R F A C E  *
 *                    *
 \********************/

/***** BUTTONS *****/

/// Inserts a button.
int KToolBar::insertButton( const QPixmap& pixmap, int id, bool enabled,
			    const QString&_text, int index )
{
  KToolBarButton *button = new KToolBarButton( pixmap, id, this, 0L, item_size,
  
  KToolBarItem *item = new KToolBarItem(button, ITEM_BUTTON, id,
                                        true);
  if ( index == -1 )
    items.append( item );
  else
    items.insert( index, item );

  connect(button, SIGNAL(clicked(int)), this, SLOT(ButtonClicked(int)));
  connect(button, SIGNAL(doubleClicked(int)), this, SLOT(ButtonDblClicked(int)));
  connect(button, SIGNAL(released(int)), this, SLOT(ButtonReleased(int)));
  connect(button, SIGNAL(pressed(int)), this, SLOT(ButtonPressed(int)));
  connect(button, SIGNAL(highlighted(int, bool)), this,
          SLOT(ButtonHighlighted(int, bool)));

  item->setEnabled( enabled );
  item->show();
  if (position == Floating)
    updateRects( true );
  else if (isVisible())
    emit (moved(position));
  return items.at();
}

/// Inserts a button with popup.
int KToolBar::insertButton( const QPixmap& pixmap, int id, QPopupMenu *_popup,
                            bool enabled, const QString&_text, int index)
  if ( index == -1 ) 
  KToolBarButton *button = new KToolBarButton( pixmap, id, this,
                                               0L, item_size, _text);
  KToolBarItem *item = new KToolBarItem(button, ITEM_BUTTON, id,
                                        true);
  button->setPopup(_popup);

  if ( index == -1 )
    items.append( item );
  else
    items.insert( index, item );

  item->setEnabled( enabled );
  item->show();

  
  connect(button, SIGNAL(doubleClicked(int)), this, SLOT(ButtonDblClicked(int)));
  connect(button, SIGNAL(released(int)), this, SLOT(ButtonReleased(int)));
  connect(button, SIGNAL(pressed(int)), this, SLOT(ButtonPressed(int)));
  connect(button, SIGNAL(highlighted(int, bool)), this,
          SLOT(ButtonHighlighted(int, bool)));


  if (position == Floating)
    updateRects( true );
  else if (isVisible())
    emit (moved(position));
  return items.at();
}


/// Inserts a button with connection.

int KToolBar::insertButton( const QPixmap& pixmap, int id, const char *signal,
 
  if ( index == -1 ) 
{
  KToolBarButton *button = new KToolBarButton( pixmap, id, this,
                                               0L, item_size, _text);
  KToolBarItem *item = new KToolBarItem(button, ITEM_BUTTON, id,
                                        true);

  if ( index == -1 )
    items.append( item );
  else
    items.insert( index, item );

  connect(button, SIGNAL(clicked(int)), this, SLOT(ButtonClicked(int)));
  connect(button, SIGNAL(doubleClicked(int)), this, SLOT(ButtonDblClicked(int)));
  connect(button, SIGNAL(released(int)), this, SLOT(ButtonReleased(int)));
  connect(button, SIGNAL(pressed(int)), this, SLOT(ButtonPressed(int)));
  connect(button, SIGNAL(highlighted(int, bool)), this,
          SLOT(ButtonHighlighted(int, bool)));

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


  
/// Inserts line separator

int KToolBar::insertLineSeparator( int index )
{

  KSeparator *separ = new KSeparator(QFrame::VLine, this);

  KToolBarItem *item = new KToolBarItem(separ, ITEM_FRAME, -1, true);
  item->resize( 5, item_size - 2 );

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
      

  // ok I'll do it for you;
  frame = new QFrame (this);
  mine = true;

  KToolBarItem *item = new KToolBarItem(frame, ITEM_FRAME, _id, mine);

  if (_index == -1)
    items.append (item);
  else
    items.insert(_index, item);
}  
  item->show();
  if (position == Floating)
int KToolBar::insertWidget(int _id, int _size, QWidget *_widget, 
  else if (isVisible())
    emit (moved(position));
  return items.at();
  
/* A poem all in G-s! No, any widget */

int KToolBar::insertWidget(int _id, int _size, QWidget *_widget,
		int _index )
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

 
  
			  bool enabled, const QString& tooltiptext, int size, int index)
{
  KLined *lined = new KLined (this, 0);
  KToolBarItem *item = new KToolBarItem(lined, ITEM_LINED, id,
  if (tooltiptext)


  if (index == -1)
    items.append (item);
  else
    items.insert(index, item);
  if (!tooltiptext.isNull())
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
 
                           int size, int index,
                           KCombo::Policy policy)
{
  KCombo *combo = new KCombo (writable, this);
  KToolBarItem *item = new KToolBarItem(combo, ITEM_COMBO, id,
                                        true);
  if (tooltiptext)
  if (index == -1)
    items.append (item);
  else
    items.insert (index, item);
  combo->insertStrList (list);
  combo->setInsertionPolicy(policy);
  if (!tooltiptext.isNull())
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

int KToolBar::insertCombo (const QString& text, int id, bool writable,
                           const char *signal, QObject *receiver,
                           const char *slot, bool enabled,
                           const QString& tooltiptext, int size, int index,
                           KCombo::Policy policy)
{
  KCombo *combo = new KCombo (writable, this);
  KToolBarItem *item = new KToolBarItem(combo, ITEM_COMBO, id,
                                        true);
  if (tooltiptext)
  if (index == -1)
    items.append (item);
  else
    items.insert (index, item);
  combo->insertItem (text);
  combo->setInsertionPolicy(policy);
  if (!tooltiptext.isNull())
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


void KToolBar::setDelayedPopup (int id , QPopupMenu *_popup)
{
  for (KToolBarItem *b = items.first(); b; b=items.next())
    if (b->ID() == id )
      ((KToolBarButton *) b->getItem())->setDelayedPopup(_popup);
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

//Autorepeat buttons
void KToolBar::setAutoRepeat (int id, bool flag /*, int delay, int repeat */)
{
  for (KToolBarItem *b = items.first(); b; b=items.next())
    if (b->ID() == id )
      ((KToolBarButton *) b->getItem())->setAutoRepeat(flag);
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
void KToolBar::setLinedText (int id, const QString& text)
{
  for (KToolBarItem *b = items.first(); b; b=items.next())
    if (b->ID() == id )
    {
      ((KLined *) b->getItem())->setText(text);
      ((KLined *) b->getItem())->cursorAtEnd();
    }
}

QString KToolBar::getLinedText (int id )
{
  for (KToolBarItem *b = items.first(); b; b=items.next())
    if (b->ID() == id )
      return ((KLined *) b->getItem())->text();
  return QString::null;
}

/// Combos
void KToolBar::insertComboItem (int id, const QString& text, int index)
{
  for (KToolBarItem *b = items.first(); b; b=items.next())
    if (b->ID() == id )
    {
      ((KCombo *) b->getItem())->insertItem(text, index);
      ((KCombo *) b->getItem())->cursorAtEnd();
    }
}

void KToolBar::insertComboList (int id, const QStringList &list, int index)
{
  for (KToolBarItem *b = items.first(); b; b=items.next())
    if (b->ID() == id )
	((KCombo *) b->getItem())->insertStringList(list, index);
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

void KToolBar::changeComboItem  (int id, const QString& text, int index)
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

QString KToolBar::getComboItem (int id, int index)
{
  for (KToolBarItem *b = items.first(); b; b=items.next())
    if (b->ID() == id )
    {
      if (index == -1)
        index = ((KCombo *) b->getItem())->currentItem();
      return ((KCombo *) b->getItem())->text(index);
    }
  return QString::null;
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

  moving = flag; 
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
	if (title){
        hide();
	}
	else {
	XSetTransientForHint( qt_xdisplay(), winId(), Parent->topLevelWidget()->winId());
        KWM::setDecoration(winId(), 2);
	KWM::moveToDesktop(winId(), KWM::desktop(Parent->winId()));
	setCaption(""); // this triggers a qt bug
	if (!title.isNull()){
	} else {
	  QString s = Parent->caption();
	  s.append(" [tools]");
	  setCaption(s);
	}
        context->changeItem (i18n("UnFloat"), CONTEXT_FLOAT);
        context->setItemEnabled (CONTEXT_FLAT, FALSE);
        setMouseTracking(true);
        mouseEntered=false;
        wasFullWidth=fullWidth;
        if (!haveAutoSized)   //if we don't have autosized item
            fullWidth=false;  // turn off autosize of toolbar
        else                  // but if we do..
            resize((int) (0.7*width()), height()); // narrow us on 70%
        updateRects (true); // we need this to set us up
        emit moved (bpos);  // this sets up KTW but not toolbar which floats
        setMinimumSize (item_size, item_size);
        return;
      }
      {
        position = bpos;
        hide();
        recreate(Parent, oldWFlags, QPoint(oldX, oldY), true);
        setMinimumSize (30,10); // for flat
        context->changeItem (i18n("Float"), CONTEXT_FLOAT);
        context->setItemEnabled (CONTEXT_FLAT, TRUE);
        setMouseTracking(true);
        mouseEntered = false;
        fullWidth=wasFullWidth;
        // updateRects (true);
        emit moved (bpos); // another bar::updateRects (damn) No! It's ok.
        return;
      }
     else
      {
        if (bpos == Flat)
        {
          setFlat (true);
          return;
        }
        position = bpos;
        enableFloating (true);
        emit moved ( bpos );
        return;
      }
   }
}

void KToolBar::enableFloating (bool arrrrrrgh)
    
    context->setItemEnabled (CONTEXT_FLOAT, arrrrrrgh);
}

void KToolBar::setIconText(int icontext)
{
    

    if (icontext != icon_text)
    {
        icon_text=icontext;
        doUpdate=true;
    }

    if (doUpdate)
        emit modechange(); // tell buttons what happened
    if (isVisible ())
        emit moved (position);
}

bool KToolBar::enable(BarStatus stat)
{
  bool mystat = isVisible();

  if ( (stat == Toggle && mystat) || stat == Hide )
   {
     //if (position == Floating)
       //iconify();
     //else
     hide();       //Sven 09.12.1998: hide in both cases
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
void KToolBar::mouseMoveEvent(QMouseEvent* mev)
{
  if (mgr)
    return;
  
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

void KToolBar::mouseReleaseEvent ( QMouseEvent * )
{
  if (mgr)
      mgr->stop();
}
{
  if (mgr)
    return;
  mouseEntered = false;
  repaint();
  
  if (flag && (position == Floating && position == Flat))

  if (!flag && (position != Flat))
{

  

  if (position == Floating )
  if ( flag == (position == Flat))
    also return;


  if (flag) //flat
  {
    context->changeItem (i18n("UnFlat"), CONTEXT_FLAT);
    lastPosition = position; // test float. I did and it works by miracle!?
    //debug ("Flat");
    position = Flat;
    horizontal = false;
    resize(30, 10);
      b->move(100,100);       // move items out of sight
    enableFloating(false);
    emit moved(Flat); // KTM will block this->updateRects
  }
  else //unflat
  {
    context->changeItem (i18n("Flat"), CONTEXT_FLAT);
    //debug ("Unflat");
    setBarPos(lastPosition);
    enableFloating(true);
    emit moved (position); // KTM will call this->updateRects
  }
}


/*************************************************************************
 *                          KRadioGroup                                  *
 *************************************************************************/


KRadioGroup::KRadioGroup (QWidget *_parent, const char *_name)
: QObject(_parent, _name)
{
  buttons.setAutoDelete(false);
  tb = (KToolBar *)_parent;
  connect (tb, SIGNAL(toggled(int)), this, SLOT(slotToggled(int)));
}

void KRadioGroup::addButton (int id)
{
  for (KToolBarItem *b = tb->items.first(); b; b=tb->items.next())
    if (b->ID() == id )
    {
      buttons.insert(id, (KToolBarButton *) b->getItem());
      ((KToolBarButton *) b->getItem())->setRadio(true);
    }
}

void KRadioGroup::removeButton (int id)
{
  buttons[id]->setRadio(false);
  buttons.remove(id);
}

void KRadioGroup::slotToggled(int id)
{
  if (buttons[id] && buttons[id]->isOn())
  {
    QIntDictIterator<KToolBarButton> it(buttons);
    while (it.current())
    {
      if (it.currentKey() != id)
        it.current()->on(false);
      ++it;
    }
  }
}

#include "ktoolbar.moc"

