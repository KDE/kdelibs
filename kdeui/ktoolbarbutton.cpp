/* This file is part of the KDE libraries
    Copyright (C) 1997, 1998 Stephan Kulow (coolo@kde.org)
              (C) 1997, 1998 Mark Donohoe (donohoe@kde.org)
              (C) 1997, 1998 Sven Radej (radej@kde.org)
              (C) 1997, 1998 Matthias Ettrich (ettrich@kde.org)
              (C) 1999 Chris Schlaeger (cs@kde.org)
              (C) 1999 Kurt Granroth (granroth@kde.org)

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

#include <config.h>
#include <string.h>

#include "ktoolbarbutton.h"
#include "ktoolbar.h"

#include <qimage.h>
#include <qtimer.h>
#include <qdrawutil.h>
#include <qtooltip.h>
#include <qbitmap.h>
#include <qpopupmenu.h>

#include <kapp.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kstyle.h>
#include <kglobalsettings.h>
#include <kiconeffect.h>
#include <kiconloader.h>

// needed to get our instance
#include <kmainwindow.h>

template class QIntDict<KToolBarButton>;

// Delay in ms before delayed popup pops up
#define POPUP_DELAY 500

class KToolBarButtonPrivate
{
public:
  KToolBarButtonPrivate()
  {
    m_noStyle     = false;
    m_isSeparator = false;
    m_isPopup     = false;
    m_isToggle    = false;
    m_isRadio     = false;
    m_highlight   = false;
    m_isRaised    = false;
    m_isActive    = false;

    m_iconName    = QString::null;
    m_iconText    = KToolBar::IconOnly;
    m_iconSize    = 0;
    m_delayTimer  = 0L;
    m_popup       = 0L;

    m_disabledIconName = QString::null;
    m_defaultIconName  = QString::null;

    m_instance = KGlobal::instance();
  }
  ~KToolBarButtonPrivate()
  {
    delete m_delayTimer; m_delayTimer = 0;
  }

  int     m_id;
  bool    m_noStyle: 1;
  bool    m_isSeparator: 1;
  bool    m_isPopup: 1;
  bool    m_isToggle: 1;
  bool    m_isRadio: 1;
  bool    m_highlight: 1;
  bool    m_isRaised: 1;
  bool    m_isActive: 1;

  QString m_iconName;
  QString m_disabledIconName;
  QString m_defaultIconName;

  KToolBar *m_parent;
  KToolBar::IconText m_iconText;
  int m_iconSize;

  QTimer     *m_delayTimer;
  QPopupMenu *m_popup;

  KInstance  *m_instance;
};

// This will construct a separator
KToolBarButton::KToolBarButton( QWidget *_parent, const char *_name )
  : QToolButton( _parent , _name)
{
  d = new KToolBarButtonPrivate;

  resize(6,6);
  hide();
  d->m_isSeparator = true;
}

KToolBarButton::KToolBarButton( const QString& _icon, int _id,
                                QWidget *_parent, const char *_name,
                                const QString &_txt, KInstance *_instance )
    : QToolButton( _parent, _name ), d( 0 )
{
  d = new KToolBarButtonPrivate;

  d->m_id     = _id;
  d->m_parent = (KToolBar*)_parent;
  setTextLabel(_txt);
  d->m_instance = _instance;

  setFocusPolicy( NoFocus );

  // connect all of our slots and start trapping events
  connect(d->m_parent, SIGNAL( modechange() ),
          this,         SLOT( modeChange() ));

  connect(this, SIGNAL( clicked() ),
          this, SLOT( slotClicked() ) );
  connect(this, SIGNAL( pressed() ),
          this, SLOT( slotPressed() ) );
  connect(this, SIGNAL( released() ),
          this, SLOT( slotReleased() ) );
  installEventFilter(this);

  d->m_iconName = _icon;

  // do our initial setup
  modeChange();
}

KToolBarButton::KToolBarButton( const QPixmap& pixmap, int _id,
                                QWidget *_parent, const char *name,
                                const QString& txt)
    : QToolButton( _parent, name ), d( 0 )
{
  d = new KToolBarButtonPrivate;

  d->m_id       = _id;
  d->m_parent   = (KToolBar *) _parent;
  setTextLabel(txt);

  setFocusPolicy( NoFocus );

  // connect all of our slots and start trapping events
  connect(d->m_parent, SIGNAL( modechange()),
          this,        SLOT(modeChange()));

  connect(this, SIGNAL( clicked() ),
          this, SLOT( slotClicked() ));
  connect(this, SIGNAL( pressed() ),
          this, SLOT( slotPressed() ));
  connect(this, SIGNAL( released() ),
          this, SLOT( slotReleased() ));
  installEventFilter(this);

  // set our pixmap and do our initial setup
  setPixmap(pixmap);
  modeChange();
}

KToolBarButton::~KToolBarButton()
{
  delete d; d = 0;
}

void KToolBarButton::modeChange()
{
  QSize mysize;

  // grab a few global variables for use in this function and others
  d->m_highlight = d->m_parent->highlight();
  d->m_iconText  = d->m_parent->iconText();

  d->m_iconSize = d->m_parent->iconSize();
  if (!d->m_iconName.isNull())
    setIcon(d->m_iconName);
  if (!d->m_disabledIconName.isNull())
    setDisabledIcon(d->m_disabledIconName);
  if (!d->m_defaultIconName.isNull())
    setDefaultIcon(d->m_defaultIconName);

  // we'll go with the size of our pixmap (plus a bit of padding) as
  // the default size...
  int pix_width  = activePixmap.width() + 6;
  int pix_height = activePixmap.height() + 6;

  // handle the simplest case (Icon only) now so we don't do an
  // unneccesary object instantiation and the like
  if (d->m_iconText == KToolBar::IconOnly)
  {
    QToolTip::remove(this);
    QToolTip::add(this, textLabel());
    mysize = QSize(pix_width, pix_height);
    setMinimumSize( mysize );
    updateGeometry();
    return;
  }

  // okay, we have to deal with fonts.  let's get our information now
  QFont tmp_font;

  tmp_font = KGlobalSettings::toolBarFont();

  // now parse out our font sizes from our chosen font
  QFontMetrics fm(tmp_font);

  int text_height = fm.lineSpacing();
  int text_width  = fm.width(textLabel());

  // none of the other modes want tooltips
  QToolTip::remove(this);
  switch (d->m_iconText)
  {
  case KToolBar::IconTextRight:
    mysize = QSize((pix_width + text_width) + 6, pix_height);
    break;

  case KToolBar::TextOnly:
    mysize = QSize(text_width + 10, text_height + 6);
    break;

  case KToolBar::IconTextBottom:
    mysize = QSize((text_width + 10 > pix_width) ? text_width + 10 : pix_width, pix_height + text_height + 3);
    break;

  default:
    break;
  }

  // make sure that this isn't taller then it is wide
  if (mysize.height() > mysize.width())
    mysize.setWidth(mysize.height());

  setMinimumSize( mysize );
  updateGeometry();
}

void KToolBarButton::setEnabled( bool enabled )
{
  QButton::setEnabled( enabled );
  QButton::setPixmap( (isEnabled() ? defaultPixmap : disabledPixmap) );
}

void KToolBarButton::setTextLabel( const QString& text)
{
  if (text.isNull())
    return;

  QString txt(text);
  if (txt.right(3) == QString::fromLatin1("..."))
    txt.truncate(txt.length() - 3);

  QToolButton::setTextLabel(txt);
  update();
}

void KToolBarButton::setText( const QString& text)
{
  setTextLabel(text);
  modeChange();
}

void KToolBarButton::setIcon( const QString &icon )
{
  setIcon( icon, false );
}

void KToolBarButton::setIcon( const QString &icon, bool )
{
  d->m_iconName = icon;
  d->m_iconSize = d->m_parent->iconSize();
  // QObject::name() return "const char *" instead of QString.
  if (!strcmp(d->m_parent->name(), "mainToolBar"))
  {
    setPixmap( MainBarIcon(icon, d->m_iconSize, KIcon::ActiveState, d->m_instance), false );
    setDisabledPixmap( MainBarIcon(icon, d->m_iconSize, KIcon::DisabledState, d->m_instance) );
    setDefaultPixmap( MainBarIcon(icon, d->m_iconSize, KIcon::DefaultState, d->m_instance) );
  } else
  {
    setPixmap( BarIcon(icon, d->m_iconSize, KIcon::ActiveState, d->m_instance), false );
    setDisabledPixmap( BarIcon(icon, d->m_iconSize, KIcon::DisabledState, d->m_instance) );
    setDefaultPixmap( BarIcon(icon, d->m_iconSize, KIcon::DefaultState, d->m_instance) );
  }
}

void KToolBarButton::setIconSet( const QIconSet &iconset, bool )
{
    // TODO. Do the opposite. Port all this code to QIconSet.
    setPixmap( iconset.pixmap( QIconSet::Automatic, QIconSet::Active ), false );
    setDisabledPixmap( iconset.pixmap( QIconSet::Automatic, QIconSet::Disabled ) );
    setDefaultPixmap( iconset.pixmap( QIconSet::Automatic, QIconSet::Normal ) );
}

// obsolete?
void KToolBarButton::setDisabledIcon( const QString &icon )
{
  d->m_disabledIconName = icon;
  d->m_iconSize         = d->m_parent->iconSize();
  if (!strcmp(d->m_parent->name(), "mainToolBar"))
    setDisabledPixmap( MainBarIcon(icon, d->m_iconSize, KIcon::DisabledState, d->m_instance) );
  else
    setDisabledPixmap( BarIcon(icon, d->m_iconSize, KIcon::DisabledState, d->m_instance) );
}

// obsolete?
void KToolBarButton::setDefaultIcon( const QString &icon )
{
  d->m_defaultIconName = icon;
  d->m_iconSize        = d->m_parent->iconSize();
  if (!strcmp(d->m_parent->name(), "mainToolBar"))
    setDefaultPixmap( MainBarIcon(icon, d->m_iconSize, KIcon::DefaultState, d->m_instance) );
  else
    setDefaultPixmap( BarIcon(icon, d->m_iconSize, KIcon::DefaultState, d->m_instance) );
}

void KToolBarButton::setPixmap( const QPixmap &pixmap )
{
  setPixmap( pixmap, true );
}

void KToolBarButton::setPixmap( const QPixmap &pixmap, bool generate )
{
  activePixmap = pixmap;

  if ( generate )
  {
    // These pixmaps are derived from the active one.
    makeDefaultPixmap();
    makeDisabledPixmap();
  }
  else
  {
    if (defaultPixmap.isNull())
      defaultPixmap = activePixmap;
    if (disabledPixmap.isNull())
      disabledPixmap = activePixmap;
  }

  QButton::setPixmap( isEnabled() ? defaultPixmap : disabledPixmap );
}

void KToolBarButton::setDefaultPixmap( const QPixmap &pixmap )
{
  defaultPixmap = pixmap;
  QButton::setPixmap( isEnabled() ? defaultPixmap : disabledPixmap );
}

void KToolBarButton::setDisabledPixmap( const QPixmap &pixmap )
{
  disabledPixmap = pixmap;
  QButton::setPixmap( isEnabled() ? defaultPixmap : disabledPixmap );
}

void KToolBarButton::setPopup(QPopupMenu *p)
{
  setPopup(p,false);
}

void KToolBarButton::setPopup(QPopupMenu *p, bool toggle)
{
  d->m_popup = p;
  d->m_isToggle  = toggle;
  p->installEventFilter(this);
}

QPopupMenu *KToolBarButton::popup()
{
  return d->m_popup;
}

void KToolBarButton::setDelayedPopup (QPopupMenu *p, bool toggle )
{
  d->m_isPopup   = true;

  if (!d->m_delayTimer)
  {
    d->m_delayTimer = new QTimer(this);
    connect(d->m_delayTimer, SIGNAL(timeout()),
           this,             SLOT(slotDelayTimeout()));
  }

  setPopup(p, toggle);
}

void KToolBarButton::leaveEvent(QEvent *)
{
  if( d->m_isRaised || d->m_isActive )
  {
    QButton::setPixmap(isEnabled() ? defaultPixmap : disabledPixmap);
    d->m_isRaised = false;
    d->m_isActive = false;
    repaint(false);
  }

  if (d->m_isPopup)
    d->m_delayTimer->stop();

  emit highlighted(d->m_id, false);
}

void KToolBarButton::enterEvent(QEvent *)
{
  if (d->m_highlight)
  {
    if (isEnabled())
    {
      QButton::setPixmap(activePixmap);

      d->m_isActive = true;
      if (!isToggleButton())
        d->m_isRaised = true;
    }
    else
    {
      QButton::setPixmap(disabledPixmap);
      d->m_isRaised = false;
      d->m_isActive = false;
    }

    repaint(false);
  }
  emit highlighted(d->m_id, true);
}

bool KToolBarButton::eventFilter(QObject *o, QEvent *ev)
{
  // From Kai-Uwe Sattler <kus@iti.CS.Uni-Magdeburg.De>
  if ((KToolBarButton *)o == this && ev->type() == QEvent::MouseButtonDblClick)
  {
    emit doubleClicked(d->m_id);
    return true;
  }

  if ((KToolBarButton *) o == this)
    if ((ev->type() == QEvent::MouseButtonPress ||
         ev->type() == QEvent::MouseButtonRelease ||
         ev->type() == QEvent::MouseButtonDblClick) && d->m_isRadio && isOn())
      return true;

  if ((QPopupMenu *) o != d->m_popup)
    return false; // just in case

  switch (ev->type())
  {
    case QEvent::MouseButtonDblClick:
    case QEvent::MouseButtonPress:
    {
      // If I get this, it means that popup is visible
      QRect r(geometry());
      r.moveTopLeft(d->m_parent->mapToGlobal(pos()));
      if (r.contains(QCursor::pos()))   // on button
        return true; // ignore
      break;
    }

    case QEvent::MouseButtonRelease:
      if (!d->m_popup->geometry().contains(QCursor::pos())) // not in menu...
      {
        QRect r(geometry());
        r.moveTopLeft(d->m_parent->mapToGlobal(pos()));

        if (r.contains(QCursor::pos()))   // but on button
        {
          if( !isToggleButton() )
            d->m_popup->hide();        //Sven: proposed by Carsten Pfeiffer
          // Make the button normal again :) Dawit A.
          if( d->m_isToggle )
            setToggle( false );
          emit clicked( d->m_id );
          return true;  // ignore release
        }
      }
      if ( d->m_isToggle )
        setToggle( false );  //Change the button to normal mode (DA)
      break;

    case QEvent::Hide:
      on(false);
      return false;
  default:
      break;
  }
  return false;
}

void KToolBarButton::drawButton( QPainter *_painter )
{
  if(kapp->kstyle()){
    KStyle::KToolButtonType iconType;
    switch(d->m_iconText){
    case KToolBar::IconOnly:
        iconType = KStyle::Icon;
        break;
    case KToolBar::IconTextRight:
        iconType = KStyle::IconTextRight;
        break;
    case KToolBar::TextOnly:
        iconType = KStyle::Text;
        break;
    case KToolBar::IconTextBottom:
    default:
        iconType = KStyle::IconTextBottom;
        break;
    }
    QFont ref_font(KGlobalSettings::toolBarFont());
    kapp->kstyle()->drawKToolBarButton(_painter, 0, 0, width(), height(),
      isEnabled()? colorGroup() : palette().disabled(), isDown() || isOn(),
      d->m_isRaised, isEnabled(), d->m_popup != 0L, iconType, textLabel(),
      pixmap(), &ref_font, this);
    return;
  }

  if ( isDown() || isOn() )
  {
    if ( style().guiStyle() == WindowsStyle )
      qDrawWinButton(_painter, 0, 0, width(), height(), colorGroup(), true );
    else
      qDrawShadePanel(_painter, 0, 0, width(), height(), colorGroup(), true, 2, 0L );
  }

  else if ( d->m_isRaised )
  {
    if ( style().guiStyle() == WindowsStyle )
      qDrawWinButton( _painter, 0, 0, width(), height(), colorGroup(), false );
    else
      qDrawShadePanel( _painter, 0, 0, width(), height(), colorGroup(), false, 2, 0L );
  }

  int dx, dy;

  QFont tmp_font(KGlobalSettings::toolBarFont());
  QFontMetrics fm(tmp_font);

  if (d->m_iconText == KToolBar::IconOnly) // icon only
  {
    if (pixmap())
    {
      dx = ( width() - pixmap()->width() ) / 2;
      dy = ( height() - pixmap()->height() ) / 2;
      if ( isDown() && style().guiStyle() == WindowsStyle )
      {
        ++dx;
        ++dy;
      }
      _painter->drawPixmap( dx, dy, *pixmap() );
    }
  }
  else if (d->m_iconText == KToolBar::IconTextRight) // icon and text (if any)
  {
    if (pixmap())
    {
      dx = 4;
      dy = ( height() - pixmap()->height() ) / 2;
      if ( isDown() && style().guiStyle() == WindowsStyle )
      {
        ++dx;
        ++dy;
      }
      _painter->drawPixmap( dx, dy, *pixmap() );
    }

    if (!textLabel().isNull())
    {
      int tf = AlignVCenter|AlignLeft;
      if (pixmap())
        dx = 4 + pixmap()->width() + 2;
      else
        dx = 4;
      dy = 0;
      if ( isDown() && style().guiStyle() == WindowsStyle )
      {
        ++dx;
        ++dy;
      }

      _painter->setFont(KGlobalSettings::toolBarFont());
      if(d->m_isRaised)
        _painter->setPen(KGlobalSettings::toolBarHighlightColor());
      _painter->drawText(dx, dy, width()-dx, height(), tf, textLabel());
    }
  }
  else if (d->m_iconText == KToolBar::TextOnly)
  {
    if (!textLabel().isNull())
    {
      int tf = AlignTop|AlignLeft;
      if (!isEnabled())
        _painter->setPen(palette().disabled().dark());
      dx = (width() - fm.width(textLabel())) / 2;
      dy = (height() - fm.lineSpacing()) / 2;
      if ( isDown() && style().guiStyle() == WindowsStyle )
      {
        ++dx;
        ++dy;
      }

      _painter->setFont(KGlobalSettings::toolBarFont());
      if(d->m_isRaised)
        _painter->setPen(KGlobalSettings::toolBarHighlightColor());
      _painter->drawText(dx, dy, fm.width(textLabel()), fm.lineSpacing(), tf, textLabel());
    }
  }
  else if (d->m_iconText == KToolBar::IconTextBottom)
  {
    if (pixmap())
    {
      dx = (width() - pixmap()->width()) / 2;
      dy = (height() - fm.lineSpacing() - pixmap()->height()) / 2;
      if ( isDown() && style().guiStyle() == WindowsStyle )
      {
        ++dx;
        ++dy;
      }
      _painter->drawPixmap( dx, dy, *pixmap() );
    }

    if (!textLabel().isNull())
    {
      int tf = AlignBottom|AlignHCenter;
      dx = (width() - fm.width(textLabel())) / 2;
      dy = height() - fm.lineSpacing() - 4;

      if ( isDown() && style().guiStyle() == WindowsStyle )
      {
        ++dx;
        ++dy;
      }

      _painter->setFont(KGlobalSettings::toolBarFont());
      if(d->m_isRaised)
        _painter->setPen(KGlobalSettings::toolBarHighlightColor());
      _painter->drawText(dx, dy, fm.width(textLabel()), fm.lineSpacing(), tf, textLabel());
    }
  }

  if (d->m_popup)
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
  if(!d->m_isSeparator)
  {
    makeDisabledPixmap();
    if ( !isEnabled() )
      QButton::setPixmap( disabledPixmap );
    else
      QButton::setPixmap( defaultPixmap );
    repaint(false); // no need to delete it first therefore only false
  }
}

void KToolBarButton::makeDefaultPixmap()
{
  if (d->m_isSeparator)
    return;

  if (!strcmp(d->m_parent->name(), "mainToolBar"))
      defaultPixmap = d->m_instance->iconLoader()->iconEffect()->apply(
          activePixmap, KIcon::MainToolbar, KIcon::DefaultState );
  else
      defaultPixmap = d->m_instance->iconLoader()->iconEffect()->apply(
          activePixmap, KIcon::Toolbar, KIcon::DefaultState );
}

void KToolBarButton::makeDisabledPixmap()
{
  if (d->m_isSeparator)
    return;             // No pixmaps for separators

  if (!strcmp(d->m_parent->name(), "mainToolBar"))
      disabledPixmap = d->m_instance->iconLoader()->iconEffect()->apply(
          activePixmap, KIcon::MainToolbar, KIcon::DisabledState );
  else
      disabledPixmap = d->m_instance->iconLoader()->iconEffect()->apply(
          activePixmap, KIcon::Toolbar, KIcon::DisabledState );
}

void KToolBarButton::showMenu()
{
  // calculate that position carefully!!
  d->m_isRaised = true;
  repaint (false);

  QPoint p;
  // Calculate position from the toolbar button, only if the button is in the toolbar !
  // If we are in the overflow menu, use the mouse position (as Qt does)
  bool bInToolbar = QRect( 0, 0, d->m_parent->width(), d->m_parent->height() ).intersects( QRect( pos(), size() ) );
  if (bInToolbar)
  {
    p = mapToGlobal( QPoint( 0, 0 ) );
    if ( p.y() + height() + d->m_popup->sizeHint().height() > KApplication::desktop()->height() )
        p.setY( p.y() - d->m_popup->sizeHint().height() );
    else
        p.setY( p.y() + height( ));
  }
  else
    p = QCursor::pos();

  if ( d->m_isToggle )
      setToggle( true ); // Turns the button into a ToggleButton ...
  d->m_popup->popup(p);
}

void KToolBarButton::slotDelayTimeout()
{
  d->m_delayTimer->stop();
  showMenu();
}

void KToolBarButton::slotClicked()
{
  if (d->m_popup && !d->m_isPopup)
    showMenu();
  else
    emit clicked( d->m_id );
}

void KToolBarButton::slotPressed()
{
  if (d->m_popup)
  {
    if (d->m_isPopup)
    {
      d->m_delayTimer->stop(); // just in case?
      d->m_delayTimer->start(POPUP_DELAY, true);
      return;
    }
    else
      showMenu();
  }
  else
    emit pressed( d->m_id );
}

void KToolBarButton::slotReleased()
{
  if (d->m_popup && d->m_isPopup)
    d->m_delayTimer->stop();

  emit released( d->m_id );
}

void KToolBarButton::slotToggled()
{
  emit toggled( d->m_id );
}

void KToolBarButton::setNoStyle(bool no_style)
{
    d->m_noStyle = no_style;

    makeDefaultPixmap();
    d->m_iconText = KToolBar::IconTextRight;
    repaint(false);
}

void KToolBarButton::setRadio (bool f)
{
    if ( d )
	d->m_isRadio = f;
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

void KToolBarButton::setToggle(bool flag)
{
  setToggleButton(flag);
  if (flag == true)
    connect(this, SIGNAL(toggled(bool)), this, SLOT(slotToggled()));
  else
    disconnect(this, SIGNAL(toggled(bool)), this, SLOT(slotToggled()));
}

// KToolBarButtonList
KToolBarButtonList::KToolBarButtonList()
{
   setAutoDelete(false);
}

#include "ktoolbarbutton.moc"
