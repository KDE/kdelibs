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

class KUrlLabel::Private
{
public:
  Private (const QString& url, KUrlLabel* label)
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

KUrlLabel::KUrlLabel (const QString& url, const QString& text,
                        QWidget* parent)
  : QLabel (!text.isNull() ? text : url, parent),
    d (new Private (url, this))
{
  setFont (font());
  setCursor (KCursor::handCursor());
  setLinkColor (d->LinkColor);
}

KUrlLabel::KUrlLabel (QWidget* parent)
  : QLabel (parent),
    d (new Private (QString(), this))
{
  setFont (font());
  setCursor (KCursor::handCursor());
  setLinkColor (d->LinkColor);
}

KUrlLabel::~KUrlLabel ()
{
  delete d;
}

void KUrlLabel::mouseReleaseEvent (QMouseEvent* e)
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

void KUrlLabel::setFont (const QFont& f)
{
  QFont newFont = f;
  newFont.setUnderline (d->TextUnderline);

  QLabel::setFont (newFont);
}

void KUrlLabel::setUnderline (bool on)
{
  d->TextUnderline = on;

  setFont (font());
}

void KUrlLabel::updateColor ()
{
  d->Timer->stop();

  if (!(d->Glow || d->Float) || !rect().contains (mapFromGlobal(QCursor::pos())))
    setLinkColor (d->LinkColor);
}

void KUrlLabel::setLinkColor (const QColor& col)
{
  QPalette p = palette();
  p.setColor (QColorGroup::Foreground, col);
  setPalette (p);

  update();
}

void KUrlLabel::setURL (const QString& url)
{
  if ( d->Tip == d->URL ) { // update the tip as well
    d->Tip = url;
    setUseTips( d->UseTips );
  }

  d->URL = url;
}

const QString& KUrlLabel::url () const
{
  return d->URL;
}

void KUrlLabel::setUseCursor (bool on, QCursor* cursor)
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

bool KUrlLabel::useCursor () const
{
  return d->UseCursor;
}

void KUrlLabel::setUseTips (bool on)
{
  d->UseTips = on;

  if (on)
    setToolTip(d->Tip);
  else
    setToolTip(QString());
}

void KUrlLabel::setTipText (const QString& tip)
{
  d->Tip = tip;

  setUseTips (d->UseTips);
}

bool KUrlLabel::useTips () const
{
  return d->UseTips;
}

const QString& KUrlLabel::tipText () const
{
  return d->Tip;
}

void KUrlLabel::setHighlightedColor (const QColor& highcolor)
{
  d->LinkColor = highcolor;

  if (!d->Timer->isActive())
    setLinkColor (highcolor);
}

void KUrlLabel::setHighlightedColor (const QString& highcolor)
{
  setHighlightedColor (QColor (highcolor));
}

void KUrlLabel::setSelectedColor (const QColor& selcolor)
{
  d->HighlightedLinkColor = selcolor;

  if (d->Timer->isActive())
    setLinkColor (selcolor);
}

void KUrlLabel::setSelectedColor (const QString& selcolor)
{
  setSelectedColor (QColor (selcolor));
}

void KUrlLabel::setGlow (bool glow)
{
  d->Glow = glow;
}

void KUrlLabel::setFloat (bool do_float)
{
  d->Float = do_float;
}

bool KUrlLabel::isGlowEnabled () const
{
  return d->Glow;
}

bool KUrlLabel::isFloatEnabled () const
{
  return d->Float;
}

void KUrlLabel::setAltPixmap (const QPixmap& altPix)
{
  d->AltPixmap = altPix;
}

const QPixmap* KUrlLabel::altPixmap () const
{
  return &d->AltPixmap;
}

void KUrlLabel::enterEvent (QEvent* e)
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

void KUrlLabel::leaveEvent (QEvent* e)
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

bool KUrlLabel::event (QEvent *e)
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


void KUrlLabel::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

#include "kurllabel.moc"
