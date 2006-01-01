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

#include <qcolor.h>
#include <qtimer.h>
#include <qpixmap.h>
#include <qapplication.h>
#include <QMouseEvent>

#include <kcursor.h>
#include <kglobalsettings.h>

#include "kurllabel.h"

class KURLLabel::Private
{
public:
  Private (const QString& url, KURLLabel* label)
    : URL (url),
      TextUnderline (true),
      LinkColor (KGlobalSettings::linkColor()),
      HighlightedLinkColor (Qt::red),
      Tip(url),
      Cursor (0L),
      UseTips (false),
      UseCursor (false),
      Glow (true),
      Float (false),
      RealUnderline (true),
      Timer (new QTimer (label))
  {
    connect (Timer, SIGNAL (timeout ()), label, SLOT (updateColor ()));
  }

  ~Private ()
  {
  }

  QString URL;
  QPixmap AltPixmap;

  bool TextUnderline;
  QColor LinkColor;
  QColor HighlightedLinkColor;

  QString Tip;
  QCursor* Cursor;
  bool UseTips:1;
  bool UseCursor:1;
  bool Glow:1;
  bool Float:1;
  bool RealUnderline:1;
  QPixmap RealPixmap;

  QTimer* Timer;
};

KURLLabel::KURLLabel (const QString& url, const QString& text,
                        QWidget* parent)
  : QLabel (!text.isNull() ? text : url, parent),
    d (new Private (url, this))
{
  setFont (font());
  setCursor (KCursor::handCursor());
  setLinkColor (d->LinkColor);
}

KURLLabel::KURLLabel (QWidget* parent)
  : QLabel (parent),
    d (new Private (QString(), this))
{
  setFont (font());
  setCursor (KCursor::handCursor());
  setLinkColor (d->LinkColor);
}

KURLLabel::~KURLLabel ()
{
  delete d;
}

void KURLLabel::mouseReleaseEvent (QMouseEvent* e)
{
  QLabel::mouseReleaseEvent (e);

  setLinkColor (d->HighlightedLinkColor);
  d->Timer->start (300);

  switch (e->button())
    {
    case Qt::LeftButton:
      emit leftClickedURL ();
      emit leftClickedURL (d->URL);
      break;

    case Qt::MidButton:
      emit middleClickedURL ();
      emit middleClickedURL (d->URL);
      break;

    case Qt::RightButton:
      emit rightClickedURL ();
      emit rightClickedURL (d->URL);
      break;

    default:
      ; // nothing
    }
}

void KURLLabel::setFont (const QFont& f)
{
  QFont newFont = f;
  newFont.setUnderline (d->TextUnderline);

  QLabel::setFont (newFont);
}

void KURLLabel::setUnderline (bool on)
{
  d->TextUnderline = on;

  setFont (font());
}

void KURLLabel::updateColor ()
{
  d->Timer->stop();

  if (!(d->Glow || d->Float) || !rect().contains (mapFromGlobal(QCursor::pos())))
    setLinkColor (d->LinkColor);
}

void KURLLabel::setLinkColor (const QColor& col)
{
  QPalette p = palette();
  p.setColor (QColorGroup::Foreground, col);
  setPalette (p);

  update();
}

void KURLLabel::setURL (const QString& url)
{
  if ( d->Tip == d->URL ) { // update the tip as well
    d->Tip = url;
    setUseTips( d->UseTips );
  }

  d->URL = url;
}

const QString& KURLLabel::url () const
{
  return d->URL;
}

void KURLLabel::setUseCursor (bool on, QCursor* cursor)
{
  d->UseCursor = on;
  d->Cursor = cursor;

  if (on)
    {
      if (cursor)
        setCursor (*cursor);
      else
        setCursor (KCursor::handCursor());
    }
  else
    unsetCursor();
}

bool KURLLabel::useCursor () const
{
  return d->UseCursor;
}

void KURLLabel::setUseTips (bool on)
{
  d->UseTips = on;

  if (on)
    setToolTip(d->Tip);
  else
    setToolTip(QString());
}

void KURLLabel::setTipText (const QString& tip)
{
  d->Tip = tip;

  setUseTips (d->UseTips);
}

bool KURLLabel::useTips () const
{
  return d->UseTips;
}

const QString& KURLLabel::tipText () const
{
  return d->Tip;
}

void KURLLabel::setHighlightedColor (const QColor& highcolor)
{
  d->LinkColor = highcolor;

  if (!d->Timer->isActive())
    setLinkColor (highcolor);
}

void KURLLabel::setHighlightedColor (const QString& highcolor)
{
  setHighlightedColor (QColor (highcolor));
}

void KURLLabel::setSelectedColor (const QColor& selcolor)
{
  d->HighlightedLinkColor = selcolor;

  if (d->Timer->isActive())
    setLinkColor (selcolor);
}

void KURLLabel::setSelectedColor (const QString& selcolor)
{
  setSelectedColor (QColor (selcolor));
}

void KURLLabel::setGlow (bool glow)
{
  d->Glow = glow;
}

void KURLLabel::setFloat (bool do_float)
{
  d->Float = do_float;
}

bool KURLLabel::isGlowEnabled () const
{
  return d->Glow;
}

bool KURLLabel::isFloatEnabled () const
{
  return d->Float;
}

void KURLLabel::setAltPixmap (const QPixmap& altPix)
{
  d->AltPixmap = altPix;
}

const QPixmap* KURLLabel::altPixmap () const
{
  return &d->AltPixmap;
}

void KURLLabel::enterEvent (QEvent* e)
{
  QLabel::enterEvent (e);

  if (!d->AltPixmap.isNull() && pixmap())
    {
      d->RealPixmap = *pixmap();
      setPixmap (d->AltPixmap);
    }

  if (d->Glow || d->Float)
    {
      d->Timer->stop();

      setLinkColor (d->HighlightedLinkColor);

      d->RealUnderline = d->TextUnderline;

      if (d->Float)
        setUnderline (true);
    }

  emit enteredURL ();
  emit enteredURL (d->URL);
}

void KURLLabel::leaveEvent (QEvent* e)
{
  QLabel::leaveEvent (e);

  if (!d->AltPixmap.isNull() && pixmap())
    setPixmap (d->RealPixmap);

  if ((d->Glow || d->Float) && !d->Timer->isActive())
    setLinkColor (d->LinkColor);

  setUnderline (d->RealUnderline);

  emit leftURL ();
  emit leftURL (d->URL);
}

bool KURLLabel::event (QEvent *e)
{
  if (e && e->type() == QEvent::PaletteChange)
  {
    // use parentWidget() unless you are a toplevel widget, then try qAapp
    QPalette p = parentWidget() ? parentWidget()->palette() : qApp->palette();
    p.setBrush(QColorGroup::Base, p.brush(QPalette::Normal, QColorGroup::Background));
    p.setColor(QColorGroup::Foreground, palette().color(QPalette::Active,QPalette::Foreground));
    setPalette(p);
    d->LinkColor = KGlobalSettings::linkColor();
    setLinkColor(d->LinkColor);
    return true;
  }
  else
    return QLabel::event(e);  
}


void KURLLabel::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

#include "kurllabel.moc"
