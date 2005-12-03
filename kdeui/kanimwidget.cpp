// -*- c-basic-offset: 2 -*-

/* This file is part of the KDE libraries
   Copyright (C) 2000 Kurt Granroth <granroth@kde.org>

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
#include <kanimwidget.h>
#include <qpixmap.h>
#include <qtimer.h>
#include <qpainter.h>
#include <qimage.h>
#include <ktoolbar.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <QMouseEvent>

class KAnimWidgetPrivate
{
public:
  bool                   loadingCompleted : 1;
  bool                   initDone         : 1;
  bool                   transparent      : 1;
  int                    frames;
  int                    current_frame;
  QPixmap                pixmap;
  QTimer                 timer;
  QString                icon_name;
  int                    size;
};

KAnimWidget::KAnimWidget( const QString& icons, int size, QWidget *parent,
                          const char *name )
  : QFrame( parent, name ),
    d( new KAnimWidgetPrivate )
{
  connect( &d->timer, SIGNAL(timeout()), this, SLOT(slotTimerUpdate()));

  if (parent && parent->inherits( "KToolBar" ))
    connect(parent, SIGNAL(modechange()), this, SLOT(updateIcons()));

  d->loadingCompleted = false;
  d->size = size;
  d->initDone = false;
  setIcons( icons );
  setFrameStyle( StyledPanel | Sunken );
}

KAnimWidget::~KAnimWidget()
{
  d->timer.stop();

  delete d; d = 0;
}

void KAnimWidget::start()
{
  d->current_frame = 0;
  d->timer.start( 50 );
}

void KAnimWidget::stop()
{
  d->current_frame = 0;
  d->timer.stop();
  repaint();
}

void KAnimWidget::setSize( int size )
{
  if ( d->size == size )
    return;

  d->size = size;
  updateIcons();
}

void KAnimWidget::setIcons( const QString& icons )
{
  if ( d->icon_name == icons )
    return;

  d->icon_name = icons;
  updateIcons();
}

QString KAnimWidget::icons( ) const
{
   return d->icon_name;
}

int KAnimWidget::size( ) const
{
   return d->size;
}


void KAnimWidget::showEvent(QShowEvent* e)
{
  if (!d->initDone)
  {
     d->initDone = true;
     updateIcons();
  }
  QFrame::showEvent(e);
}

void KAnimWidget::hideEvent(QHideEvent* e)
{
  QFrame::hideEvent(e);
}

void KAnimWidget::enterEvent( QEvent *e )
{
  setFrameStyle( Panel | Raised );

  QFrame::enterEvent( e );
}

void KAnimWidget::leaveEvent( QEvent *e )
{
  setFrameStyle( StyledPanel | Sunken );

  QFrame::leaveEvent( e );
}

void KAnimWidget::mousePressEvent( QMouseEvent *e )
{
  QFrame::mousePressEvent( e );
}

void KAnimWidget::mouseReleaseEvent( QMouseEvent *e )
{
  if ( e->button() == Qt::LeftButton &&
       rect().contains( e->pos() ) )
    emit clicked();

  QFrame::mouseReleaseEvent( e );
}

void KAnimWidget::slotTimerUpdate()
{
  if(!isVisible())
    return;

  d->current_frame++;
  if (d->current_frame == d->frames)
     d->current_frame = 0;

  // TODO
  // We have to clear the widget when repainting a transparent image
  // By doing it like this we get a bit of flicker though. A better
  // way might be to merge it with the background in drawContents.
  repaint(d->transparent);
}

void KAnimWidget::drawContents( QPainter *p )
{
  if ( d->pixmap.isNull() )
    return;

  int w = d->pixmap.width();
  int h = w;
  int x = (width()  - w) / 2;
  int y = (height() - h) / 2;
  p->drawPixmap(QPoint(x, y), d->pixmap, QRect(0, d->current_frame*h, w, h));
}

void KAnimWidget::updateIcons()
{
  if (!d->initDone)
     return;

  if (parent()->inherits( "KToolBar" ))
    d->size = ((KToolBar*)parent())->iconSize();
  if (!d->size)
     d->size = KGlobal::iconLoader()->currentSize(KIcon::MainToolbar);

  QString path = KGlobal::iconLoader()->iconPath(d->icon_name, -d->size);
  QImage img(path);

  if (img.isNull())
     return;

  d->current_frame = 0;
  d->frames = img.height() / img.width();
  d->transparent = img.hasAlphaBuffer();
  if (d->pixmap.width() != d->size)
  {
     img = img.smoothScale(d->size, d->size*d->frames);
  }
  d->pixmap = img;

  setFixedSize( d->size+2, d->size+2 );
  resize( d->size+2, d->size+2 );
}

void KAnimWidget::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

#include "kanimwidget.moc"
