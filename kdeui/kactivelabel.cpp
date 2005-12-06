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

#include "kactivelabel.h"

#include <ktoolinvocation.h>
#include <qregexp.h>
#include <q3simplerichtext.h>
#include <kdebug.h>
#include <QFocusEvent>
#include <QWhatsThis>

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
   q->setTextFormat(Qt::RichText);
   q->setVScrollBarMode(Q3ScrollView::AlwaysOff);
   q->setHScrollBarMode(Q3ScrollView::AlwaysOff);
   q->setFrameStyle(QFrame::NoFrame);
   q->setFocusPolicy( Qt::TabFocus );
   updatePalette();

   QObject::connect(q, SIGNAL(linkClicked(const QString &)),
                    q, SLOT(openLink(const QString &)));
}

void KActiveLabelPrivate::updatePalette()
{
    QPalette p = q->palette();
    p.setBrush(QColorGroup::Base, p.brush(QPalette::Normal, QColorGroup::Background));
    p.setColor(QColorGroup::Text, p.color(QPalette::Normal, QColorGroup::Foreground));
    q->setPalette(p);
}

KActiveLabel::KActiveLabel(QWidget * parent, const char * name)
 : Q3TextBrowser(parent, name)
{
    d = new KActiveLabelPrivate(this);
}

KActiveLabel::KActiveLabel(const QString &text, QWidget * parent, const char * name)
 : Q3TextBrowser(parent, name)
{
    d = new KActiveLabelPrivate(this);
    setText(text);
}

KActiveLabel::~KActiveLabel()
{
    delete d;
}

void KActiveLabel::openLink(const QString & link)
{
   QRegExp whatsthis("whatsthis:/*([^/].*)");
   if (whatsthis.exactMatch(link)) {
      QWhatsThis::showText(QCursor::pos(),whatsthis.cap(1));
      return;
   }

   QStringList args;
   args << "exec" << link;
   KToolInvocation::kdeinitExec("kfmclient", args);
}

void KActiveLabel::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

void KActiveLabel::focusInEvent( QFocusEvent* fe )
{
   Q3TextBrowser::focusInEvent(fe);
   if(fe->reason() == Qt::TabFocusReason || fe->reason() == Qt::BacktabFocusReason)
      selectAll(true);
}

void KActiveLabel::focusOutEvent( QFocusEvent* fe )
{
   Q3TextBrowser::focusOutEvent(fe);
   if(fe->reason() == Qt::TabFocusReason || fe->reason() == Qt::BacktabFocusReason)
      selectAll(false);
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
        Q3TextBrowser::keyPressEvent( e );
    }
}

bool KActiveLabel::event(QEvent *e)
{
    // call the base implementation first so it updates
    // our palette
    const bool result = Q3TextBrowser::event(e);
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

   QString txt = text();
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
