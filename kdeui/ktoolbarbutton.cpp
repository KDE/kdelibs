/* This file is part of the KDE libraries
    Copyright (C) 1997, 1998 Stephan Kulow (coolo@kde.org)
              (C) 1997, 1998 Mark Donohoe (donohoe@kde.org)
              (C) 1997, 1998 Sven Radej (radej@kde.org)
              (C) 1997, 1998 Matthias Ettrich (ettrich@kde.org)
			  (C) 1999 Chris Schlaeger (cs@kde.org)

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
// Revision 1.1  1999/09/21 11:03:53  waba
// WABA: Clean up interface
//

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "ktoolbarbutton.h"
#include "ktoolbar.h"

#include <qtimer.h>
#include <qdrawutil.h>
#include <qtooltip.h>
#include <qbitmap.h>

#include <kapp.h>
#include <kglobal.h>
#include <kstyle.h>

template QIntDict<KToolBarButton>;

// Delay in ms before delayed popup pops up
#define POPUP_DELAY 500

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
  sep=false;
  delayPopup = false;
  parentWidget = (KToolBar *) _parent;
  raised = false;
  myPopup = 0L;
  radio = false;
  toolBarButton = !_mb;

  setFocusPolicy( NoFocus );
  id = _id;
  if (!txt.isNull())
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
}

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
  // From Kai-Uwe Sattler <kus@iti.CS.Uni-Magdeburg.De>
  if ((KToolBarButton *)o == this && ev->type () == QEvent::MouseButtonDblClick)
  {
    //debug ("Doubleclick");
    emit doubleClicked (id);
    return true;
  }

  if ((KToolBarButton *) o == this)
    if ((ev->type() == QEvent::MouseButtonPress ||
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
      {
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
          myPopup->hide();        //Sven: proposed by Carsten Pfeiffer
          emit clicked( id );
          //myPopup->setActiveItem(0 /*myPopup->idAt(1)*/); // set first active
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
  return false;
}



void KToolBarButton::drawButton( QPainter *_painter )
{
  if(kapp->kstyle()){
    KStyle::KToolButtonType iconType;
    switch(icontext){
    case 0:
        iconType = KStyle::Icon;
        break;
    case 1:
        iconType = KStyle::IconTextRight;
        break;
    case 2:
        iconType = KStyle::Text;
        break;
    case 3:
    default:
        iconType = KStyle::IconTextBottom;
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

  else if ( raised )
  {
    if ( style() == WindowsStyle )
      qDrawWinButton( _painter, 0, 0, width(), height(), colorGroup(), false );
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
        dx= 1;
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

    if (!btext.isNull())
    {
      int tf = AlignBottom|AlignHCenter;
      if (!isEnabled())
        _painter->setPen(palette().disabled().dark());
      dy= pixmap()->height();
      dx = 2;

      if ( isDown() && style() == WindowsStyle )
      {
        ++dx;
        ++dy;
      }

      if (toolBarButton)
        _painter->setFont(buttonFont);
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
  if (toolBarButton) // I might be a menuBarButton
  {
    buttonFont.setFamily("Helvetica");
    buttonFont.setPointSize(10);
    buttonFont.setBold(false);
    buttonFont.setItalic(false);
    buttonFont.setCharSet(font().charSet());

    fnt=buttonFont;
  }
  else
    fnt=KGlobal::generalFont();

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
    i = enabledPixmap.convertToImage();
    i = i.smoothScale(enabledPixmap.width()*factor,
                      enabledPixmap.height()*factor);
    enabledPixmap.resize (i.width(), i.height());
    enabledPixmap.convertFromImage(i );
  }
*/

  highlight=parentWidget->highlight;
  switch (icontext)
  {
  case 0:
    QToolTip::remove(this);
    QToolTip::add(this, btext);
    resize (myWidth, _size-2);
    break;

  case 1:
    QToolTip::remove(this);
    resize (fm.width(btext)+myWidth, _size-2); // +2+_size-2
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
  else
  {
    QToolTip::remove(this);
    QToolTip::add(this, btext);
    resize (myWidth, _size-2);
  }
  */

}

void KToolBarButton::makeDisabledPixmap()
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
  // if popup is visible we don't get this
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

KToolBarButtonList::KToolBarButtonList()
{
   setAutoDelete(false);
}

#include "ktoolbarbutton.moc"

