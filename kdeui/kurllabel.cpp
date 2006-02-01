/* This file is part of the KDE libraries
   Copyright (C) 1998 Kurt Granroth <granroth@kde.org>
   Copyright (C) 2000 Peter Putzer <putzer@kde.org>
   Copyright (C) 2005 Jaroslaw Staniek <js@iidea.pl>

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

#include <qcolor.h>
#include <qtimer.h>
#include <qtooltip.h>
#include <qpixmap.h>
#include <qpainter.h>
#include <qstyle.h>
#include <qapplication.h>

#include <kcursor.h>
#include <kglobalsettings.h>

#include "kurllabel.h"

class KURLLabel::Private
{
public:
  Private (const QString& url, KURLLabel* label)
    : URL (url),
      LinkColor (KGlobalSettings::linkColor()),
      HighlightedLinkColor (Qt::red),
      Tip(url),
      Cursor (0L),
      Underline (true),
      UseTips (false),
      Glow (true),
      Float (false),
      RealUnderline (true),
      MousePressed(false),
      WasInsideRect(false),
      MarginAltered(false),
      Timer (new QTimer (label))
  {
    connect (Timer, SIGNAL (timeout ()), label, SLOT (updateColor ()));
  }

  ~Private ()
  {
      delete Cursor;
  }

  QString URL;
  QPixmap AltPixmap;

  QColor LinkColor;
  QColor HighlightedLinkColor;

  QString Tip;
  QCursor* Cursor;
  bool Underline:1;
  bool UseTips:1;
  bool Glow:1;
  bool Float:1;
  bool RealUnderline:1;
  bool MousePressed:1;
  bool WasInsideRect:1;
  bool MarginAltered:1;
  QPixmap RealPixmap;

  QTimer* Timer;
};

KURLLabel::KURLLabel (const QString& url, const QString& text,
                        QWidget* parent, const char* name)
  : QLabel (!text.isNull() ? text : url, parent, name),
    d (new Private (url, this))
{
  setFont (font());
  setUseCursor (true);
  setLinkColor (d->LinkColor);
  setFocusPolicy( QWidget::StrongFocus ); //better accessibility
  setMouseTracking (true);
}

KURLLabel::KURLLabel (QWidget* parent, const char* name)
  : QLabel (parent, name),
    d (new Private (QString::null, this))
{
  setFont (font());
  setUseCursor (true);
  setLinkColor (d->LinkColor);
  setFocusPolicy( QWidget::StrongFocus ); //better accessibility
  setMouseTracking (true);
}

KURLLabel::~KURLLabel ()
{
  delete d;
}

void KURLLabel::mouseReleaseEvent (QMouseEvent* e)
{
  QLabel::mouseReleaseEvent (e);
  if (!d->MousePressed)
    return;
  d->MousePressed = false;
  QRect r( activeRect() );
  if (!r.contains(e->pos()))
    return;

  setLinkColor (d->HighlightedLinkColor);
  d->Timer->start (300);

  switch (e->button())
    {
    case LeftButton:
      emit leftClickedURL ();
      emit leftClickedURL (d->URL);
      break;

    case MidButton:
      emit middleClickedURL ();
      emit middleClickedURL (d->URL);
      break;

    case RightButton:
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
  newFont.setUnderline (d->Underline);

  QLabel::setFont (newFont);
}

void KURLLabel::setUnderline (bool on)
{
  d->Underline = on;

  setFont (font());
}

void KURLLabel::updateColor ()
{
  d->Timer->stop();

  QRect r( activeRect() );
  if (!(d->Glow || d->Float) || !r.contains (mapFromGlobal(QCursor::pos())))
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

void KURLLabel::unsetCursor ()
{
	delete d->Cursor;
	d->Cursor = 0;
}

void KURLLabel::setCursor ( const QCursor& cursor )
{
	delete d->Cursor;
	d->Cursor = new QCursor( cursor );
}

void KURLLabel::setUseCursor (bool on, QCursor* cursor)
{
  if (on)
    {
      if (cursor)
        KURLLabel::setCursor (*cursor);
      else
        KURLLabel::setCursor (KCursor::handCursor());
    }
  else
    KURLLabel::unsetCursor ();
}

bool KURLLabel::useCursor () const
{
  return d->Cursor;
}

void KURLLabel::setUseTips (bool on)
{
  d->UseTips = on;

  if (on) {
    QToolTip::add (this, activeRect(), d->Tip);
  } else
    QToolTip::remove (this);
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

  QRect r( activeRect() );
  if (!r.contains( static_cast<QMouseEvent*>(e)->pos() ))
    return;

  if (!d->AltPixmap.isNull() && pixmap())
    {
      d->RealPixmap = *pixmap();
      setPixmap (d->AltPixmap);
    }

  if (d->Glow || d->Float)
    {
      d->Timer->stop();

      setLinkColor (d->HighlightedLinkColor);

      d->RealUnderline = d->Underline;

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
  if (e && e->type() == QEvent::ParentPaletteChange)
  {
    // use parentWidget() unless you are a toplevel widget, then try qAapp
    QPalette p = parentWidget() ? parentWidget()->palette() : qApp->palette();
    p.setBrush(QColorGroup::Base, p.brush(QPalette::Normal, QColorGroup::Background));
    p.setColor(QColorGroup::Foreground, palette().active().foreground());
    setPalette(p);
    d->LinkColor = KGlobalSettings::linkColor();
    setLinkColor(d->LinkColor);
    return true;
  }
  else if (e->type() == QEvent::Paint) {
    const bool result = QLabel::event(e);
    if (result && hasFocus()) {
        QPainter p(this);
        QRect r( activeRect() );
        style().drawPrimitive( QStyle::PE_FocusRect, &p, r, colorGroup() );
    }
    return result;
  }
  else if (e->type() == QEvent::KeyPress) {
    QKeyEvent* ke = static_cast<QKeyEvent*>(e);
    if (ke->key() == Qt::Key_Enter || ke->key() == Qt::Key_Return) {
      setLinkColor (d->HighlightedLinkColor);
      d->Timer->start (300);
      emit leftClickedURL ();
      emit leftClickedURL (d->URL);
      ke->accept();
      return true;
    }
  }
  else if (e->type() == QEvent::MouseButtonPress) {
    QRect r( activeRect() );
    d->MousePressed = r.contains(static_cast<QMouseEvent*>(e)->pos());
  }
  else if (e->type() == QEvent::MouseMove) {
    if (d->Cursor) {
      QRect r( activeRect() );
      bool inside = r.contains(static_cast<QMouseEvent*>(e)->pos());
      if (d->WasInsideRect != inside) {
        if (inside)
          QLabel::setCursor(*d->Cursor);
        else
          QLabel::unsetCursor();
        d->WasInsideRect = inside;
      }
    }
  }
  return QLabel::event(e);
}

QRect KURLLabel::activeRect() const
{
  QRect r( contentsRect() );
  if (text().isEmpty() || (!d->MarginAltered && sizePolicy() == QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed)))
      return r; //fixed size is sometimes used with pixmap
  int hAlign = QApplication::horizontalAlignment( alignment() );
  int indentX = (hAlign && indent()>0) ? indent() : 0;
  QFontMetrics fm(font());
  r.setWidth( QMIN(fm.width(text()), r.width()));
  if ( hAlign & AlignLeft )
      r.moveLeft(r.left() + indentX);
  if ( hAlign & AlignCenter )
      r.moveLeft((contentsRect().width()-r.width())/2+margin());
  if ( hAlign & AlignRight )
      r.moveLeft(contentsRect().width()-r.width()-indentX+margin());
  int add = QMIN(3, margin());
  r = QRect(r.left()-add, r.top()-add, r.width()+2*add, r.height()+2*add);
  return r;
}

void KURLLabel::setMargin( int margin )
{
  QLabel::setMargin(margin);
  d->MarginAltered = true;
}

void KURLLabel::setFocusPolicy( FocusPolicy policy )
{
  QLabel::setFocusPolicy(policy);
  if (!d->MarginAltered) {
      QLabel::setMargin(policy == NoFocus ? 0 : 3); //better default : better look when focused
  }
}

void KURLLabel::setSizePolicy ( QSizePolicy policy )
{
  QLabel::setSizePolicy(policy);
  if (!d->MarginAltered && policy.horData()==QSizePolicy::Fixed && policy.verData()==QSizePolicy::Fixed) {
      QLabel::setMargin(0); //better default : better look when fixed size
  }
}

void KURLLabel::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

#include "kurllabel.moc"
