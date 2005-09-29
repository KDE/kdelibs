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

#include <kapplication.h>
#include <qregexp.h>
#include <qwhatsthis.h>
#include <qsimplerichtext.h>
#include <kdebug.h>

KActiveLabel::KActiveLabel(QWidget * parent, const char * name)
 : QTextBrowser(parent, name)
{
   init();
}

KActiveLabel::KActiveLabel(const QString &text, QWidget * parent, const char * name)
 : QTextBrowser(parent, name)
{
   init();
   setText(text);
}

void KActiveLabel::init()
{
   setTextFormat(Qt::RichText);
   setVScrollBarMode(QScrollView::AlwaysOff);
   setHScrollBarMode(QScrollView::AlwaysOff);
   setFrameStyle(QFrame::NoFrame);
   setFocusPolicy( QWidget::TabFocus );
   paletteChanged();

   connect(this, SIGNAL(linkClicked(const QString &)),
           this, SLOT(openLink(const QString &)));
   if (kapp)
   {
      connect(kapp, SIGNAL(kdisplayPaletteChanged()),
              this, SLOT(paletteChanged()));
   }
}

void KActiveLabel::paletteChanged()
{
   QPalette p = kapp ? kapp->palette() : palette();
   p.setBrush(QColorGroup::Base, p.brush(QPalette::Normal, QColorGroup::Background));
   p.setColor(QColorGroup::Text, p.color(QPalette::Normal, QColorGroup::Foreground));
   setPalette(p);
}

void KActiveLabel::openLink(const QString & link)
{
   QRegExp whatsthis("whatsthis:/*([^/].*)");
   if (whatsthis.exactMatch(link)) {
      QWhatsThis::display(whatsthis.cap(1));
      return;
   }

   QStringList args;
   args << "exec" << link;
   kapp->kdeinitExec("kfmclient", args);
}

void KActiveLabel::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

void KActiveLabel::focusInEvent( QFocusEvent* fe )
{
   QTextBrowser::focusInEvent(fe);
   if(fe->reason() == QFocusEvent::Tab || fe->reason() == QFocusEvent::Backtab)
      selectAll(true);
}

void KActiveLabel::focusOutEvent( QFocusEvent* fe )
{
   QTextBrowser::focusOutEvent(fe);
   if(fe->reason() == QFocusEvent::Tab || fe->reason() == QFocusEvent::Backtab)
      selectAll(false);
}

void KActiveLabel::keyPressEvent( QKeyEvent *e )
{
    switch ( e->key() )
    {
    case Key_Down:
    case Key_Up:
    case Key_Left:
    case Key_Right:
        // jump over QTextEdit's key navigation breakage.
        // we're not interested in keyboard navigation within the text
        QWidget::keyPressEvent( e );
        break;
    default:
        QTextBrowser::keyPressEvent( e );
    }
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
   QSimpleRichText rt(txt, font());
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
