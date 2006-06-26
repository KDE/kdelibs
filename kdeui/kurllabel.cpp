// Copyright (C) 2000 Peter Putzer

// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.

// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
// 02110-1301  USA

#include <QtCore/QTimer>
#include <QtGui/QApplication>
#include <QtGui/QMouseEvent>

#include <kcursor.h>
#include <kglobalsettings.h>

#include "kurllabel.h"

class KUrlLabel::Private
{
  public:
    Private( const QString& _url, KUrlLabel* _parent )
      : parent( _parent ),
        url( _url ),
        textUnderlined( true ),
        realUnderlined( true ),
        tipText( url ),
        linkColor( KGlobalSettings::linkColor() ),
        highlightedLinkColor( Qt::red ),
        cursor( 0 ),
        useTips( false ),
        useCursor( false ),
        glowEnabled( true ),
        floatEnabled( false ),
        timer( new QTimer( parent ) )
    {
      connect( timer, SIGNAL( timeout() ), parent, SLOT( updateColor() ) );
    }

    ~Private ()
    {
    }

    void updateColor()
    {
      timer->stop();

      if ( !(glowEnabled || floatEnabled) || !parent->rect().contains( parent->mapFromGlobal( QCursor::pos() ) ) )
        setLinkColor( linkColor );
    }

    void setLinkColor( const QColor& color )
    {
      QPalette palette = parent->palette();
      palette.setColor( QPalette::Foreground, color );
      parent->setPalette( palette );

      parent->update();
    }


    KUrlLabel *parent;

    QString url;
    bool textUnderlined;
    bool realUnderlined;
    QString tipText;
    QColor linkColor;
    QColor highlightedLinkColor;
    QCursor* cursor;
    bool useTips;
    bool useCursor;
    bool glowEnabled;
    bool floatEnabled;
    QPixmap alternatePixmap;
    QPixmap realPixmap;
    QTimer* timer;
};

KUrlLabel::KUrlLabel( const QString& url, const QString& text, QWidget* parent )
  : QLabel( !text.isNull() ? text : url, parent ),
    d( new Private( url, this ) )
{
  setFont( font() );
  setCursor( KCursor::handCursor() );
  d->setLinkColor( d->linkColor );
}

KUrlLabel::KUrlLabel( QWidget* parent )
  : QLabel( parent ),
    d( new Private( QString(), this ) )
{
  setFont( font() );
  setCursor( KCursor::handCursor() );
  d->setLinkColor( d->linkColor );
}

KUrlLabel::~KUrlLabel()
{
  delete d;
}

void KUrlLabel::mouseReleaseEvent( QMouseEvent* event )
{
  QLabel::mouseReleaseEvent( event );

  d->setLinkColor( d->highlightedLinkColor );
  d->timer->start( 300 );

  switch ( event->button() ) {
    case Qt::LeftButton:
      emit leftClickedUrl();
      emit leftClickedUrl( d->url );
      break;

    case Qt::MidButton:
      emit middleClickedUrl();
      emit middleClickedUrl( d->url );
      break;

    case Qt::RightButton:
      emit rightClickedUrl();
      emit rightClickedUrl( d->url );
      break;

    default:
      break;
  }
}

void KUrlLabel::setFont( const QFont& font )
{
  QFont newFont = font;
  newFont.setUnderline( d->textUnderlined );

  QLabel::setFont( newFont );
}

void KUrlLabel::setUnderline( bool on )
{
  d->textUnderlined = on;

  setFont( font() );
}

void KUrlLabel::setUrl( const QString& url )
{
  if ( d->tipText == d->url ) { // update the tip as well
    d->tipText = url;
    setUseTips( d->useTips );
  }

  d->url = url;
}

const QString& KUrlLabel::url() const
{
  return d->url;
}

void KUrlLabel::setUseCursor( bool on, QCursor* cursor )
{
  d->useCursor = on;
  d->cursor = cursor;

  if ( on ) {
    if ( cursor )
      setCursor( *cursor );
    else
      setCursor( KCursor::handCursor() );
  } else
    unsetCursor();
}

bool KUrlLabel::useCursor() const
{
  return d->useCursor;
}

void KUrlLabel::setUseTips( bool on )
{
  d->useTips = on;

  if ( on )
    setToolTip( d->tipText );
  else
    setToolTip( QString() );
}

void KUrlLabel::setTipText( const QString& tipText )
{
  d->tipText = tipText;

  setUseTips( d->useTips );
}

bool KUrlLabel::useTips() const
{
  return d->useTips;
}

const QString& KUrlLabel::tipText() const
{
  return d->tipText;
}

void KUrlLabel::setHighlightedColor( const QColor& color )
{
  d->linkColor = color;

  if ( !d->timer->isActive() )
    d->setLinkColor( color );
}

void KUrlLabel::setHighlightedColor( const QString& color )
{
  setHighlightedColor( QColor( color ) );
}

void KUrlLabel::setSelectedColor( const QColor& color )
{
  d->highlightedLinkColor = color;

  if ( d->timer->isActive() )
    d->setLinkColor( color );
}

void KUrlLabel::setSelectedColor( const QString& color )
{
  setSelectedColor( QColor( color ) );
}

void KUrlLabel::setGlowEnabled( bool glowEnabled )
{
  d->glowEnabled = glowEnabled;
}

void KUrlLabel::setFloatEnabled( bool floatEnabled )
{
  d->floatEnabled = floatEnabled;
}

bool KUrlLabel::isGlowEnabled() const
{
  return d->glowEnabled;
}

bool KUrlLabel::isFloatEnabled() const
{
  return d->floatEnabled;
}

void KUrlLabel::setAlternatePixmap( const QPixmap& pixmap )
{
  d->alternatePixmap = pixmap;
}

const QPixmap* KUrlLabel::alternatePixmap() const
{
  return &d->alternatePixmap;
}

void KUrlLabel::enterEvent( QEvent* event )
{
  QLabel::enterEvent( event );

  if ( !d->alternatePixmap.isNull() && pixmap() ) {
    d->realPixmap = *pixmap();
    setPixmap( d->alternatePixmap );
  }

  if ( d->glowEnabled || d->floatEnabled ) {
    d->timer->stop();

    d->setLinkColor( d->highlightedLinkColor );

    d->realUnderlined = d->textUnderlined;

    if ( d->floatEnabled )
      setUnderline( true );
  }

  emit enteredUrl();
  emit enteredUrl( d->url );
}

void KUrlLabel::leaveEvent( QEvent* event )
{
  QLabel::leaveEvent( event );

  if ( !d->alternatePixmap.isNull() && pixmap() )
    setPixmap( d->realPixmap );

  if ( (d->glowEnabled || d->floatEnabled) && !d->timer->isActive() )
    d->setLinkColor( d->linkColor );

  setUnderline( d->realUnderlined );

  emit leftUrl();
  emit leftUrl( d->url );
}

bool KUrlLabel::event( QEvent *event )
{
  if ( event->type() == QEvent::PaletteChange ) {
    /**
     * Use parentWidget() unless you are a toplevel widget, then try qAapp
     */
    QPalette palette = parentWidget() ? parentWidget()->palette() : qApp->palette();

    palette.setBrush( QPalette::Base, palette.brush( QPalette::Normal, QPalette::Background ) );
    palette.setColor( QPalette::Foreground, this->palette().color( QPalette::Active, QPalette::Foreground ) );
    setPalette( palette );

    d->linkColor = KGlobalSettings::linkColor();
    d->setLinkColor( d->linkColor );

    return true;
  } else
    return QLabel::event( event );
}

#include "kurllabel.moc"
