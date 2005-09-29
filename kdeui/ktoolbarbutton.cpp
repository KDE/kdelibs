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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <config.h>
#include <string.h>

#include "ktoolbarbutton.h"
#include "ktoolbar.h"

#include <qstyle.h>
#include <qimage.h>
#include <qtimer.h>
#include <qdrawutil.h>
#include <qtooltip.h>
#include <qbitmap.h>
#include <qpopupmenu.h>
#include <qcursor.h>
#include <qpainter.h>
#include <qlayout.h>

#include <kapplication.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kiconeffect.h>
#include <kiconloader.h>

// needed to get our instance
#include <kmainwindow.h>

template class QIntDict<KToolBarButton>;

class KToolBarButtonPrivate
{
public:
  KToolBarButtonPrivate()
  {
    m_buttonDown  = false;

    m_noStyle     = false;
    m_isSeparator = false;
    m_isRadio     = false;
    m_highlight   = false;
    m_isRaised    = false;
    m_isActive    = false;

    m_iconName    = QString::null;
    m_iconText    = KToolBar::IconOnly;
    m_iconSize    = 0;

    m_parent   = 0;
    m_instance = KGlobal::instance();
  }
  ~KToolBarButtonPrivate()
  {
  }

  int     m_id;
  bool    m_buttonDown : 1;
  bool    m_noStyle: 1;
  bool    m_isSeparator: 1;
  bool    m_isRadio: 1;
  bool    m_highlight: 1;
  bool    m_isRaised: 1;
  bool    m_isActive: 1;

  QString m_iconName;

  KToolBar *m_parent;
  KToolBar::IconText m_iconText;
  int m_iconSize;
  QSize size;

  QPoint m_mousePressPos;

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
  QToolButton::setTextLabel(_txt);
  d->m_instance = _instance;

  d->m_parent = dynamic_cast<KToolBar*>(_parent);
  if (d->m_parent) {
    connect(d->m_parent, SIGNAL( modechange() ),
            this,         SLOT( modeChange() ));
  }

  setFocusPolicy( NoFocus );

  // connect all of our slots and start trapping events
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
  QToolButton::setTextLabel(txt);

  d->m_parent = dynamic_cast<KToolBar*>(_parent);
  if (d->m_parent) {
    connect(d->m_parent, SIGNAL( modechange() ),
            this,         SLOT( modeChange() ));
  }

  setFocusPolicy( NoFocus );

  // connect all of our slots and start trapping events
  connect(this, SIGNAL( clicked() ),
          this, SLOT( slotClicked() ));
  connect(this, SIGNAL( pressed() ),
          this, SLOT( slotPressed() ));
  connect(this, SIGNAL( released() ),
          this, SLOT( slotReleased() ));
  installEventFilter(this);

  // set our pixmap and do our initial setup
  setIconSet( QIconSet( pixmap ));
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
  if (d->m_parent) {
    d->m_highlight = d->m_parent->highlight();
    d->m_iconText  = d->m_parent->iconText();

    d->m_iconSize = d->m_parent->iconSize();
  }
  if (!d->m_iconName.isNull())
    setIcon(d->m_iconName);

  // we'll start with the size of our pixmap
  int pix_width  = d->m_iconSize;
  if ( d->m_iconSize == 0 ) {
      if (d->m_parent && !strcmp(d->m_parent->name(), "mainToolBar"))
          pix_width = IconSize( KIcon::MainToolbar );
      else
          pix_width = IconSize( KIcon::Toolbar );
  }
  int pix_height = pix_width;

  int text_height = 0;
  int text_width = 0;

  QToolTip::remove(this);
  if (d->m_iconText != KToolBar::IconOnly)
  {
    // okay, we have to deal with fonts.  let's get our information now
    QFont tmp_font = KGlobalSettings::toolBarFont();

    // now parse out our font sizes from our chosen font
    QFontMetrics fm(tmp_font);

    text_height = fm.lineSpacing();
    text_width  = fm.width(textLabel());

    // none of the other modes want tooltips
  }
  else
  {
    QToolTip::add(this, textLabel());
  }

  switch (d->m_iconText)
  {
  case KToolBar::IconOnly:
    mysize = QSize(pix_width, pix_height);
    break;

  case KToolBar::IconTextRight:
    mysize = QSize(pix_width + text_width + 4, pix_height);
    break;

  case KToolBar::TextOnly:
    mysize = QSize(text_width + 4, text_height);
    break;

  case KToolBar::IconTextBottom:
    mysize = QSize((text_width + 4 > pix_width) ? text_width + 4 : pix_width, pix_height + text_height);
    break;

  default:
    break;
  }

  mysize = style().sizeFromContents(QStyle::CT_ToolButton, this, mysize).
               expandedTo(QApplication::globalStrut());

  // make sure that this isn't taller then it is wide
  if (mysize.height() > mysize.width())
    mysize.setWidth(mysize.height());

  d->size = mysize;
  updateGeometry();
}

void KToolBarButton::setTextLabel( const QString& text, bool tipToo)
{
  if (text.isNull())
    return;

  QString txt(text);
  if (txt.endsWith(QString::fromLatin1("...")))
    txt.truncate(txt.length() - 3);

  QToolButton::setTextLabel(txt, tipToo);
  update();
}

void KToolBarButton::setText( const QString& text)
{
  setTextLabel(text, true);
  modeChange();
}

void KToolBarButton::setIcon( const QString &icon )
{
  d->m_iconName = icon;
  if (d->m_parent)
    d->m_iconSize = d->m_parent->iconSize();
  // QObject::name() return "const char *" instead of QString.
  if (d->m_parent && !strcmp(d->m_parent->name(), "mainToolBar"))
    QToolButton::setIconSet( d->m_instance->iconLoader()->loadIconSet(
        d->m_iconName, KIcon::MainToolbar, d->m_iconSize ));
  else
    QToolButton::setIconSet( d->m_instance->iconLoader()->loadIconSet(
        d->m_iconName, KIcon::Toolbar, d->m_iconSize ));
}

void KToolBarButton::setIconSet( const QIconSet &iconset )
{
  QToolButton::setIconSet( iconset );
}

// remove?
void KToolBarButton::setPixmap( const QPixmap &pixmap )
{
  if( pixmap.isNull()) // called by QToolButton
  {
    QToolButton::setPixmap( pixmap );
    return;
  }
  QIconSet set = iconSet();
  set.setPixmap( pixmap, QIconSet::Automatic, QIconSet::Active );
  QToolButton::setIconSet( set );
}

void KToolBarButton::setDefaultPixmap( const QPixmap &pixmap )
{
  QIconSet set = iconSet();
  set.setPixmap( pixmap, QIconSet::Automatic, QIconSet::Normal );
  QToolButton::setIconSet( set );
}

void KToolBarButton::setDisabledPixmap( const QPixmap &pixmap )
{
  QIconSet set = iconSet();
  set.setPixmap( pixmap, QIconSet::Automatic, QIconSet::Disabled );
  QToolButton::setIconSet( set );
}

void KToolBarButton::setDefaultIcon( const QString& icon )
{
  QIconSet set = iconSet();
  QPixmap pm;
  if (d->m_parent && !strcmp(d->m_parent->name(), "mainToolBar"))
    pm = d->m_instance->iconLoader()->loadIcon( icon, KIcon::MainToolbar,
        d->m_iconSize );
  else
    pm = d->m_instance->iconLoader()->loadIcon( icon, KIcon::Toolbar,
        d->m_iconSize );
  set.setPixmap( pm, QIconSet::Automatic, QIconSet::Normal );
  QToolButton::setIconSet( set );
}

void KToolBarButton::setDisabledIcon( const QString& icon )
{
  QIconSet set = iconSet();
  QPixmap pm;
  if (d->m_parent && !strcmp(d->m_parent->name(), "mainToolBar"))
    pm = d->m_instance->iconLoader()->loadIcon( icon, KIcon::MainToolbar,
        d->m_iconSize );
  else
    pm = d->m_instance->iconLoader()->loadIcon( icon, KIcon::Toolbar,
        d->m_iconSize );
  set.setPixmap( pm, QIconSet::Automatic, QIconSet::Disabled );
  QToolButton::setIconSet( set );
}

QPopupMenu *KToolBarButton::popup()
{
  // obsolete
  // KDE4: remove me
  return QToolButton::popup();
}

void KToolBarButton::setPopup(QPopupMenu *p, bool)
{
  QToolButton::setPopup(p);
  QToolButton::setPopupDelay(-1);
}


void KToolBarButton::setDelayedPopup (QPopupMenu *p, bool)
{
  QToolButton::setPopup(p);
  QToolButton::setPopupDelay(QApplication::startDragTime());
}

void KToolBarButton::leaveEvent(QEvent *)
{
  if( d->m_isRaised || d->m_isActive )
  {
    d->m_isRaised = false;
    d->m_isActive = false;
    repaint(false);
  }

  emit highlighted(d->m_id, false);
}

void KToolBarButton::enterEvent(QEvent *)
{
  if (d->m_highlight)
  {
    if (isEnabled())
    {
      d->m_isActive = true;
      if (!isToggleButton())
        d->m_isRaised = true;
    }
    else
    {
      d->m_isRaised = false;
      d->m_isActive = false;
    }

    repaint(false);
  }
  emit highlighted(d->m_id, true);
}

bool KToolBarButton::eventFilter(QObject *o, QEvent *ev)
{
  if ((KToolBarButton *)o == this)
  {

    // Popup the menu when the left mousebutton is pressed and the mouse
    // is moved by a small distance.
    if (QToolButton::popup())
    {
      if (ev->type() == QEvent::MouseButtonPress)
      {
        QMouseEvent* mev = static_cast<QMouseEvent*>(ev);
        d->m_mousePressPos = mev->pos();
      }
      else if (ev->type() == QEvent::MouseMove)
      {
        QMouseEvent* mev = static_cast<QMouseEvent*>(ev);
        if ((mev->pos() - d->m_mousePressPos).manhattanLength()
              > KGlobalSettings::dndEventDelay())
        {
          openPopup();
          return true;
        }
      }
    }

    if (d->m_isRadio &&
	(ev->type() == QEvent::MouseButtonPress ||
         ev->type() == QEvent::MouseButtonRelease ||
         ev->type() == QEvent::MouseButtonDblClick) && isOn())
      return true;

    // From Kai-Uwe Sattler <kus@iti.CS.Uni-Magdeburg.De>
    if (ev->type() == QEvent::MouseButtonDblClick)
    {
      emit doubleClicked(d->m_id);
      return false;
    }
  }

  return QToolButton::eventFilter(o, ev);
}

void KToolBarButton::mousePressEvent( QMouseEvent * e )
{
  d->m_buttonDown = true;

  if ( e->button() == MidButton )
  {
    // Get QToolButton to show the button being down while pressed
    QMouseEvent ev( QEvent::MouseButtonPress, e->pos(), e->globalPos(), LeftButton, e->state() );
    QToolButton::mousePressEvent(&ev);
    return;
  }
  QToolButton::mousePressEvent(e);
}

void KToolBarButton::mouseReleaseEvent( QMouseEvent * e )
{
  Qt::ButtonState state = Qt::ButtonState(e->button() | (e->state() & KeyButtonMask));
  if ( e->button() == MidButton )
  {
    QMouseEvent ev( QEvent::MouseButtonRelease, e->pos(), e->globalPos(), LeftButton, e->state() );
    QToolButton::mouseReleaseEvent(&ev);
  }
  else
    QToolButton::mouseReleaseEvent(e);

  if ( !d->m_buttonDown )
    return;
  d->m_buttonDown = false;

  if ( hitButton( e->pos() ) )
    emit buttonClicked( d->m_id, state );
}

void KToolBarButton::drawButton( QPainter *_painter )
{
  QStyle::SFlags flags   = QStyle::Style_Default;
  QStyle::SCFlags active = QStyle::SC_None;

  if (isDown()) {
    flags  |= QStyle::Style_Down;
    active |= QStyle::SC_ToolButton;
  }
  if (isEnabled()) 	flags |= QStyle::Style_Enabled;
  if (isOn()) 		flags |= QStyle::Style_On;
  if (isEnabled() && hasMouse())	flags |= QStyle::Style_Raised;
  if (hasFocus())	flags |= QStyle::Style_HasFocus;

  // Draw a styled toolbutton
  style().drawComplexControl(QStyle::CC_ToolButton, _painter, this, rect(),
	colorGroup(), flags, QStyle::SC_ToolButton, active, QStyleOption());

  int dx, dy;
  QFont tmp_font(KGlobalSettings::toolBarFont());
  QFontMetrics fm(tmp_font);
  QRect textRect;
  int textFlags = 0;

  if (d->m_iconText == KToolBar::IconOnly) // icon only
  {
    QPixmap pixmap = iconSet().pixmap( QIconSet::Automatic,
        isEnabled() ? (d->m_isActive ? QIconSet::Active : QIconSet::Normal) :
            	QIconSet::Disabled,
        isOn() ? QIconSet::On : QIconSet::Off );
    if( !pixmap.isNull())
    {
      dx = ( width() - pixmap.width() ) / 2;
      dy = ( height() - pixmap.height() ) / 2;
      if ( isDown() && style().styleHint(QStyle::SH_GUIStyle) == WindowsStyle )
      {
        ++dx;
        ++dy;
      }
      _painter->drawPixmap( dx, dy, pixmap );
    }
  }
  else if (d->m_iconText == KToolBar::IconTextRight) // icon and text (if any)
  {
    QPixmap pixmap = iconSet().pixmap( QIconSet::Automatic,
        isEnabled() ? (d->m_isActive ? QIconSet::Active : QIconSet::Normal) :
            	QIconSet::Disabled,
        isOn() ? QIconSet::On : QIconSet::Off );
    if( !pixmap.isNull())
    {
      dx = 4;
      dy = ( height() - pixmap.height() ) / 2;
      if ( isDown() && style().styleHint(QStyle::SH_GUIStyle) == WindowsStyle )
      {
        ++dx;
        ++dy;
      }
      _painter->drawPixmap( dx, dy, pixmap );
    }

    if (!textLabel().isNull())
    {
      textFlags = AlignVCenter|AlignLeft;
      if (!pixmap.isNull())
        dx = 4 + pixmap.width() + 2;
      else
        dx = 4;
      dy = 0;
      if ( isDown() && style().styleHint(QStyle::SH_GUIStyle) == WindowsStyle )
      {
        ++dx;
        ++dy;
      }
      textRect = QRect(dx, dy, width()-dx, height());
    }
  }
  else if (d->m_iconText == KToolBar::TextOnly)
  {
    if (!textLabel().isNull())
    {
      textFlags = AlignVCenter|AlignLeft;
      dx = (width() - fm.width(textLabel())) / 2;
      dy = (height() - fm.lineSpacing()) / 2;
      if ( isDown() && style().styleHint(QStyle::SH_GUIStyle) == WindowsStyle )
      {
        ++dx;
        ++dy;
      }
      textRect = QRect( dx, dy, fm.width(textLabel()), fm.lineSpacing() );
    }
  }
  else if (d->m_iconText == KToolBar::IconTextBottom)
  {
    QPixmap pixmap = iconSet().pixmap( QIconSet::Automatic,
        isEnabled() ? (d->m_isActive ? QIconSet::Active : QIconSet::Normal) :
            	QIconSet::Disabled,
        isOn() ? QIconSet::On : QIconSet::Off );
    if( !pixmap.isNull())
    {
      dx = (width() - pixmap.width()) / 2;
      dy = (height() - fm.lineSpacing() - pixmap.height()) / 2;
      if ( isDown() && style().styleHint(QStyle::SH_GUIStyle) == WindowsStyle )
      {
        ++dx;
        ++dy;
      }
      _painter->drawPixmap( dx, dy, pixmap );
    }

    if (!textLabel().isNull())
    {
      textFlags = AlignBottom|AlignHCenter;
      dx = (width() - fm.width(textLabel())) / 2;
      dy = height() - fm.lineSpacing() - 4;

      if ( isDown() && style().styleHint(QStyle::SH_GUIStyle) == WindowsStyle )
      {
        ++dx;
        ++dy;
      }
      textRect = QRect( dx, dy, fm.width(textLabel()), fm.lineSpacing() );
    }
  }

  // Draw the text at the position given by textRect, and using textFlags
  if (!textLabel().isNull() && !textRect.isNull())
  {
      _painter->setFont(KGlobalSettings::toolBarFont());
      if (!isEnabled())
        _painter->setPen(palette().disabled().dark());
      else if(d->m_isRaised)
        _painter->setPen(KGlobalSettings::toolBarHighlightColor());
      else
	_painter->setPen( colorGroup().buttonText() );
      _painter->drawText(textRect, textFlags, textLabel());
  }

  if (QToolButton::popup())
  {
    QStyle::SFlags arrowFlags = QStyle::Style_Default;

    if (isDown())	arrowFlags |= QStyle::Style_Down;
    if (isEnabled()) 	arrowFlags |= QStyle::Style_Enabled;

      style().drawPrimitive(QStyle::PE_ArrowDown, _painter,
          QRect(width()-7, height()-7, 7, 7), colorGroup(),
	  arrowFlags, QStyleOption() );
  }
}

void KToolBarButton::paletteChange(const QPalette &)
{
  if(!d->m_isSeparator)
  {
    modeChange();
    repaint(false); // no need to delete it first therefore only false
  }
}

bool KToolBarButton::event(QEvent *e)
{
  if (e->type() == QEvent::ParentFontChange || e->type() == QEvent::ApplicationFontChange)
  {
     //If we use toolbar text, apply the settings again, to relayout...
     if (d->m_iconText != KToolBar::IconOnly)
       modeChange();
     return true;
  }

  return QToolButton::event(e);
}


void KToolBarButton::showMenu()
{
  // obsolete
  // KDE4: remove me
}

void KToolBarButton::slotDelayTimeout()
{
  // obsolete
  // KDE4: remove me
}

void KToolBarButton::slotClicked()
{
  emit clicked( d->m_id );

  // emit buttonClicked when the button was clicked while being in an extension popupmenu
  if ( d->m_parent && !d->m_parent->rect().contains( geometry().center() ) ) {
    ButtonState state = KApplication::keyboardMouseState();
    if ( ( state & MouseButtonMask ) == NoButton )
      state = ButtonState( LeftButton | state );
    emit buttonClicked( d->m_id, state ); // Doesn't work with MidButton
  }
}

void KToolBarButton::slotPressed()
{
  emit pressed( d->m_id );
}

void KToolBarButton::slotReleased()
{
  emit released( d->m_id );
}

void KToolBarButton::slotToggled()
{
  emit toggled( d->m_id );
}

void KToolBarButton::setNoStyle(bool no_style)
{
    d->m_noStyle = no_style;

    modeChange();
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
  if(isToggleButton())
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
  if (flag)
    connect(this, SIGNAL(toggled(bool)), this, SLOT(slotToggled()));
  else
    disconnect(this, SIGNAL(toggled(bool)), this, SLOT(slotToggled()));
}

QSize KToolBarButton::sizeHint() const
{
   return d->size;
}

QSize KToolBarButton::minimumSizeHint() const
{
   return d->size;
}

QSize KToolBarButton::minimumSize() const
{
   return d->size;
}

bool KToolBarButton::isRaised() const
{
    return d->m_isRaised;
}

bool KToolBarButton::isActive() const
{
    return d->m_isActive;
}

int KToolBarButton::iconTextMode() const
{
    return static_cast<int>( d->m_iconText );
}

int KToolBarButton::id() const
{
    return d->m_id;
}

// KToolBarButtonList
KToolBarButtonList::KToolBarButtonList()
{
   setAutoDelete(false);
}

void KToolBarButton::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

#include "ktoolbarbutton.moc"
