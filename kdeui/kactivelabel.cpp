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

#include <QWhatsThis>
#include <Q3SimpleRichText>

#include <ktoolinvocation.h>

#include "kactivelabel.h"

class KActiveLabelPrivate
{
public:
    KActiveLabelPrivate(KActiveLabel *qq);

    void updatePalette();

    KActiveLabel *q;
};

KActiveLabelPrivate::KActiveLabelPrivate(KActiveLabel *qq)
    : q(qq)
{
   q->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
   q->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
   q->setFrameStyle(QFrame::NoFrame);
   q->setFocusPolicy(Qt::TabFocus);
   updatePalette();

   QObject::connect(q, SIGNAL(anchorClicked (const QUrl &)),
                    q, SLOT(openLink(const QUrl &)));
}

void KActiveLabelPrivate::updatePalette()
{
    QPalette p = q->palette();
    p.setBrush(QColorGroup::Base, p.brush(QPalette::Normal, QColorGroup::Background));
    p.setColor(QColorGroup::Text, p.color(QPalette::Normal, QColorGroup::Foreground));
    q->setPalette(p);
}

KActiveLabel::KActiveLabel(QWidget * parent)
 : KTextBrowser(parent)
{
    d = new KActiveLabelPrivate(this);
}

KActiveLabel::KActiveLabel(const QString &text, QWidget * parent)
 : KTextBrowser(parent)
{
    d = new KActiveLabelPrivate(this);
    setHtml(text);
}

KActiveLabel::~KActiveLabel()
{
    delete d;
}

void KActiveLabel::openLink(const QUrl & link)
{
   QRegExp whatsthis("whatsthis:/*([^/].*)");
   if (whatsthis.exactMatch(link.toString())) {
      QWhatsThis::showText(QCursor::pos(),whatsthis.cap(1));
      return;
   }

   QStringList args;
   args << "exec" << link.toString();
   KToolInvocation::kdeinitExec("kfmclient", args);
}

void KActiveLabel::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

void KActiveLabel::focusInEvent( QFocusEvent* fe )
{
   KTextBrowser::focusInEvent(fe);
   if(fe->reason() == Qt::TabFocusReason || fe->reason() == Qt::BacktabFocusReason)
      selectAll();
}

void KActiveLabel::focusOutEvent( QFocusEvent* fe )
{
   KTextBrowser::focusOutEvent(fe);
   if(fe->reason() == Qt::TabFocusReason || fe->reason() == Qt::BacktabFocusReason)
      selectAll(); //TODO reimplement: deselect text
}

void KActiveLabel::keyPressEvent( QKeyEvent *e )
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

bool KActiveLabel::event(QEvent *e)
{
    // call the base implementation first so it updates
    // our palette
    const bool result = KTextBrowser::event(e);
    if (e->type() == QEvent::ApplicationPaletteChange) {
        d->updatePalette();
    }
    return result;
}

QSize KActiveLabel::minimumSizeHint() const
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

QSize KActiveLabel::sizeHint() const
{
   return minimumSizeHint();
}

#include "kactivelabel.moc"
