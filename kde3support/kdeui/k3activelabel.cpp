/*  This file is part of the KDE libraries
    Copyright (C) 1999 Waldo Bastian (bastian@kde.org)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; version 2
    of the License.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "k3activelabel.h"

#include <Q3SimpleRichText>
#include <QFocusEvent>

#include <ktoolinvocation.h>

class K3ActiveLabelPrivate
{
public:
    K3ActiveLabelPrivate(K3ActiveLabel *qq);

    void updatePalette();

    K3ActiveLabel *q;
};

K3ActiveLabelPrivate::K3ActiveLabelPrivate(K3ActiveLabel *qq)
    : q(qq)
{
   q->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
   q->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
   q->setFrameStyle(QFrame::NoFrame);
   q->setFocusPolicy(Qt::TabFocus);
   updatePalette();
}

void K3ActiveLabelPrivate::updatePalette()
{
    QPalette p = q->palette();
    p.setBrush(QPalette::Base, p.brush(QPalette::Normal, QPalette::Background));
    p.setColor(QPalette::Text, p.color(QPalette::Normal, QPalette::Foreground));
    q->setPalette(p);
}

K3ActiveLabel::K3ActiveLabel(QWidget * parent)
 : KTextBrowser(parent),d(new K3ActiveLabelPrivate(this))
{
}

K3ActiveLabel::K3ActiveLabel(const QString &text, QWidget * parent)
 : KTextBrowser(parent),d(new K3ActiveLabelPrivate(this))
{
    setHtml(text);
}

K3ActiveLabel::~K3ActiveLabel()
{
    delete d;
}

void K3ActiveLabel::focusInEvent( QFocusEvent* fe )
{
   KTextBrowser::focusInEvent(fe);
   if(fe->reason() == Qt::TabFocusReason || fe->reason() == Qt::BacktabFocusReason)
      selectAll();
}

void K3ActiveLabel::focusOutEvent( QFocusEvent* fe )
{
   KTextBrowser::focusOutEvent(fe);
   if(fe->reason() == Qt::TabFocusReason || fe->reason() == Qt::BacktabFocusReason)
      selectAll(); //TODO reimplement: deselect text
}

void K3ActiveLabel::keyPressEvent( QKeyEvent *e )
{
    switch ( e->key() )
    {
    case Qt::Key_Down:
    case Qt::Key_Up:
    case Qt::Key_Left:
    case Qt::Key_Right:
        // jump over QTextEdit's key navigation breakage.
        // we're not interested in keyboard navigation within the text
        QWidget::keyPressEvent( e );
        break;
    default:
        KTextBrowser::keyPressEvent( e );
    }
}

bool K3ActiveLabel::event(QEvent *e)
{
    // call the base implementation first so it updates
    // our palette
    const bool result = KTextBrowser::event(e);
    if (e->type() == QEvent::ApplicationPaletteChange) {
        d->updatePalette();
    }
    return result;
}

QSize K3ActiveLabel::minimumSizeHint() const
{
   QSize ms = minimumSize();
   if ((ms.width() > 0) && (ms.height() > 0))
      return ms;

   int w = 400;
   if (ms.width() > 0)
      w = ms.width();

   QString txt = toHtml();
   Q3SimpleRichText rt(txt, font());
   rt.setWidth(w - 2*frameWidth() - 10);
   w = 10 + rt.widthUsed() + 2*frameWidth();
   if (w < ms.width())
      w = ms.width();
   int h = rt.height() + 2*frameWidth();
   if ( h < ms.height())
      h = ms.height();

   return QSize(w, h);
}

QSize K3ActiveLabel::sizeHint() const
{
   return minimumSizeHint();
}

#include "k3activelabel.moc"
