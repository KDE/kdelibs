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

#include <qevent.h>
#include <qstyle.h>
#include <qimage.h>
#include <qtimer.h>
#include <qdrawutil.h>
#include <qbitmap.h>
#include <qmenu.h>
#include <qcursor.h>
#include <qpainter.h>
#include <qlayout.h>
#include <qapplication.h>

#include <kdebug.h>
#include <kinstance.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kiconeffect.h>
#include <kiconloader.h>

// needed to get our instance
#include <kmainwindow.h>

template class Q3IntDict<KToolBarButton>;

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
KToolBarButton::KToolBarButton( QWidget *_parent )
  : QToolButton( _parent )
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

  setFocusPolicy( Qt::NoFocus );

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

  setFocusPolicy( Qt::NoFocus );

  // connect all of our slots and start trapping events
  connect(this, SIGNAL( clicked() ),
          this, SLOT( slotClicked() ));
  connect(this, SIGNAL( pressed() ),
          this, SLOT( slotPressed() ));
  connect(this, SIGNAL( released() ),
          this, SLOT( slotReleased() ));
  installEventFilter(this);

  // set our pixmap and do our initial setup
  setIcon( QIcon( pixmap ));
  modeChange();
}

KToolBarButton::~KToolBarButton()
{
  delete d; d = 0;
}

void KToolBarButton::initStyleOption(QStyleOptionToolButton* opt) const
{
  opt->init(this);
  opt->font      = KGlobalSettings::toolBarFont();
  opt->icon      = icon();
  opt->iconSize  = QSize(d->m_iconSize, d->m_iconSize);
  opt->text      = textLabel();
  if (d->m_iconText == KToolBar::IconTextBottom) {
    opt->toolButtonStyle = Qt::ToolButtonTextUnderIcon;
  } else if (d->m_iconText == KToolBar::IconOnly)
    opt->toolButtonStyle = Qt::ToolButtonIconOnly;
  else if (d->m_iconText ==   KToolBar::TextOnly)
    opt->toolButtonStyle = Qt::ToolButtonTextBesideIcon;
  else 
    opt->toolButtonStyle = Qt::ToolButtonTextBesideIcon;

  opt->features  = QStyleOptionToolButton::None; //We don't Qt know about the menu, since we don't want the split-button!
	//### delay stuff?
  opt->subControls       = QStyle::SC_ToolButton;
  opt->activeSubControls = 0; //### FIXME: !!
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
      if (d->m_parent && !(d->m_parent->objectName()== QLatin1String("mainToolBar")))
          pix_width = IconSize( KIcon::MainToolbar );
      else
          pix_width = IconSize( KIcon::Toolbar );
  }
  int pix_height = pix_width;

  int text_height = 0;
  int text_width = 0;

  setToolTip(QString());
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
    setToolTip(textLabel());
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

  QStyleOptionToolButton opt;
  initStyleOption(&opt);
  mysize = style()->sizeFromContents(QStyle::CT_ToolButton, &opt,  mysize, this).
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
  if (txt.endsWith(QLatin1String("...")))
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
  if (d->m_parent && !(d->m_parent->objectName() == QLatin1String("mainToolBar")))
    QToolButton::setIcon( d->m_instance->iconLoader()->loadIconSet(
        d->m_iconName, KIcon::MainToolbar, d->m_iconSize ));
  else
    QToolButton::setIcon( d->m_instance->iconLoader()->loadIconSet(
        d->m_iconName, KIcon::Toolbar, d->m_iconSize ));
}


// remove?
void KToolBarButton::setPixmap( const QPixmap &pixmap )
{
  if( pixmap.isNull()) // called by QToolButton
  {
    QToolButton::setPixmap( pixmap );
    return;
  }
  QIcon set = iconSet();
  set.setPixmap( pixmap, QIcon::Automatic, QIcon::Active );
  QToolButton::setIcon( set );
}

void KToolBarButton::setDefaultPixmap( const QPixmap &pixmap )
{
  QIcon set = iconSet();
  set.setPixmap( pixmap, QIcon::Automatic, QIcon::Normal );
  QToolButton::setIcon( set );
}

void KToolBarButton::setDisabledPixmap( const QPixmap &pixmap )
{
  QIcon set = iconSet();
  set.setPixmap( pixmap, QIcon::Automatic, QIcon::Disabled );
  QToolButton::setIcon( set );
}

void KToolBarButton::setDefaultIcon( const QString& icon )
{
  QIcon set = iconSet();
  QPixmap pm;
  if (d->m_parent && !(d->m_parent->objectName() == QLatin1String("mainToolBar")))
    pm = d->m_instance->iconLoader()->loadIcon( icon, KIcon::MainToolbar,
        d->m_iconSize );
  else
    pm = d->m_instance->iconLoader()->loadIcon( icon, KIcon::Toolbar,
        d->m_iconSize );
  set.setPixmap( pm, QIcon::Automatic, QIcon::Normal );
  QToolButton::setIcon( set );
}

void KToolBarButton::setDisabledIcon( const QString& icon )
{
  QIcon set = iconSet();
  QPixmap pm;
  if (d->m_parent && !(d->m_parent->objectName()==QLatin1String("mainToolBar")))
    pm = d->m_instance->iconLoader()->loadIcon( icon, KIcon::MainToolbar,
        d->m_iconSize );
  else
    pm = d->m_instance->iconLoader()->loadIcon( icon, KIcon::Toolbar,
        d->m_iconSize );
  set.setPixmap( pm, QIcon::Automatic, QIcon::Disabled );
  QToolButton::setIcon( set );
}


void KToolBarButton::setPopup(QMenu *p, bool)
{
  QToolButton::setPopup(p);
  QToolButton::setPopupDelay(-1);
}


void KToolBarButton::setDelayedPopup (QMenu *p, bool)
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

  if ( e->button() == Qt::MidButton && hitButton( e->pos() ) )
  {
    setDown(true);
    emit QAbstractButton::pressed();
    return;
  }
  QToolButton::mousePressEvent(e);
}

void KToolBarButton::mouseReleaseEvent( QMouseEvent * e )
{
#if 0 // Qt3 hack to make button normal again after MMB, I think it's not needed with Qt4 anymore (see QAbstractButton::mouseReleaseEvent)
  if ( e->button() == Qt::MidButton )
  {
    QMouseEvent ev( QEvent::MouseButtonPress, e->pos(), e->globalPos(), Qt::LeftButton, e->buttons(), e->modifiers() );
    QToolButton::mouseReleaseEvent(&ev);
  }
  else
#endif
    QToolButton::mouseReleaseEvent(e);

  if ( !d->m_buttonDown )
    return;
  d->m_buttonDown = false;

  if ( hitButton( e->pos() ) ) {
#ifdef QT3_SUPPORT
    Qt::ButtonState state = Qt::ButtonState(e->button() | (e->state() & Qt::KeyboardModifierMask));
    emit buttonClicked( d->m_id, state );
#endif
    emit buttonClicked( d->m_id, e->button(), e->modifiers() );
  }
}

void KToolBarButton::paintEvent( QPaintEvent*e )
{
  QPainter painter(this);
  QStyle::State flags   = QStyle::State_None;
  QStyle::SubControls active = QStyle::SC_None;

  if (isDown()) {
    flags  |= QStyle::State_Sunken;
    active |= QStyle::SC_ToolButton;
  }
  if (isEnabled()) 	flags |= QStyle::State_Enabled;
  if (isOn()) 		flags |= QStyle::State_On;
  if (isEnabled() && hasMouse())	flags |= QStyle::State_Raised;
  if (hasFocus())	flags |= QStyle::State_HasFocus;

  // Draw a styled toolbutton
  QStyleOptionToolButton opt;
  initStyleOption(&opt);
  opt.state             = flags;
  opt.activeSubControls = active;

  style()->drawComplexControl(QStyle::CC_ToolButton, &opt, &painter, this);

  if (QToolButton::popup())
  {
    QStyle::State arrowFlags = QStyle::State_None;

    if (isDown())	arrowFlags |= QStyle::State_Sunken;
    if (isEnabled()) 	arrowFlags |= QStyle::State_Enabled;

    QStyleOption opt;
    opt.init(this);
    opt.rect  = QRect(width()-7, height()-7, 7, 7);
    opt.state = arrowFlags;
    style()->drawPrimitive(QStyle::PE_IndicatorArrowDown, &opt, &painter);
  }
}

void KToolBarButton::changeEvent(QEvent* e)
{
  switch (e->type())
  {
    case QEvent::PaletteChange:
    case QEvent::ApplicationPaletteChange:
      if(!d->m_isSeparator)
      {
        modeChange();
        repaint(false); // no need to delete it first therefore only false
      }
      break;
    case QEvent::FontChange:
    case QEvent::ApplicationFontChange:
      if (d->m_iconText != KToolBar::IconOnly)
        modeChange();
    default: ; //Shadup!
  }
}

void KToolBarButton::slotClicked()
{
  emit clicked( d->m_id );

  // emit buttonClicked when the button was clicked while being in an extension popupmenu
  if ( d->m_parent && !d->m_parent->rect().contains( geometry().center() ) ) {
    Qt::ButtonState state = QApplication::mouseButtons();
    if ( ( state & Qt::MouseButtonMask ) == Qt::NoButton )
      state = Qt::ButtonState( Qt::LeftButton | state );
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
