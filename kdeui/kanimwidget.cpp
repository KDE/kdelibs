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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/
#include <kanimwidget.h>
#include <qstringlist.h>
#include <qpixmap.h>
#include <qtimer.h>
#include <qpainter.h>
#include <ktoolbar.h>
#include <kdebug.h>
#include <kiconloader.h>

class KAnimWidget::KAnimWidgetPrivate
{
public:
  KAnimWidgetPrivate()
  {
    size = -1;
  }

  QStringList            icons;
  QString                icon_name;
  QTimer                 timer;
  int                    size;
  bool                   loadingCompleted;
  QPixmap                pixmap;

  QValueList<QPixmap>           pixmaps;
  QValueList<QPixmap>::Iterator iter;
};

KAnimWidget::KAnimWidget( const QString& icons, int size, QWidget *parent,
                          const char *name )
  : QFrame( parent, name ),
    d( new KAnimWidgetPrivate )
{
  connect( &d->timer, SIGNAL(timeout()), this, SLOT(slotTimerUpdate()));

  if (parent->inherits( "KToolBar" ))
    connect(parent, SIGNAL(modechange()), this, SLOT(updateIcons()));

  d->loadingCompleted = false;
  d->size = size;
  setIcons( icons );
  setFrameStyle( StyledPanel | Sunken );
}

KAnimWidget::KAnimWidget( const QStringList& icons, int size,
                          QWidget *parent, const char *name )
  : QFrame( parent, name ),
    d(new KAnimWidgetPrivate)
{
  connect( &d->timer, SIGNAL(timeout()), this, SLOT(slotTimerUpdate()));

  if (parent->inherits( "KToolBar" ))
    connect(parent, SIGNAL(modechange()), this, SLOT(updateIcons()));

  d->loadingCompleted = false;
  d->size = size;
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
  d->iter   = d->pixmaps.begin();
  d->pixmap = *d->iter;
  d->timer.start( 50 );
}

void KAnimWidget::stop()
{
  d->iter   = d->pixmaps.begin();
  d->pixmap = *d->iter;
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

void KAnimWidget::setIcons( const QStringList& icons )
{
  if ( d->icons == icons )
    return;

  d->icons = icons;
  updateIcons();
}

void KAnimWidget::setIcons( const QString& icons )
{
  if ( d->icon_name == icons )
    return;

  d->icon_name = icons;
  updateIcons();
}

void KAnimWidget::hideEvent(QHideEvent* e)
{
  QFrame::hideEvent(e);
}

void KAnimWidget::enterEvent( QEvent *e )
{
  setFrameStyle( WinPanel | Raised );

  QFrame::enterEvent( e );
}

void KAnimWidget::leaveEvent( QEvent *e )
{
  setFrameStyle( StyledPanel | Sunken );

  QFrame::enterEvent( e );
}

void KAnimWidget::mousePressEvent( QMouseEvent *e )
{
  emit clicked();

  QFrame::mousePressEvent( e );
}

void KAnimWidget::slotTimerUpdate()
{
  if(!isVisible())
    return;

  if(!d->loadingCompleted)
    loadRemainingIcons();

  if ( d->iter == d->pixmaps.end() )
    d->iter = d->pixmaps.begin();

  d->pixmap = *d->iter;

  ++d->iter;
  repaint(false);
}

void KAnimWidget::drawContents( QPainter *p )
{
  if ( d->pixmap.isNull() )
    return;

  QPixmap pm(QSize(this->width(), this->height()));
  QPainter p2 (&pm);
  int x, y;

  pm.fill (p->backgroundColor());

  if ( d->pixmap.width() != width() || d->pixmap.height() != height() )
  {
    x = (width()  - d->pixmap.width()) / 2;
    y = (height() - d->pixmap.height()) / 2;
  }
  else
  {
    x = 0;
    y = 0;
  }

  p2.drawPixmap(x, y, d->pixmap);
  p->drawPixmap( 0, 0, pm);
}

void KAnimWidget::updateIcons()
{
  if (parent()->inherits( "KToolBar" ))
    d->size = ((KToolBar*)parent())->iconSize();

  if (d->icon_name.isEmpty() == false)
    d->icons = KGlobal::iconLoader()->loadAnimated(d->icon_name, KIcon::MainToolbar, d->size);

  d->pixmaps.clear();
  d->pixmaps.append( MainBarIcon( *d->icons.begin(), d->size ) );
  d->loadingCompleted = false;
  d->iter   = d->pixmaps.begin();
  d->pixmap = *d->iter;

  if ( d->pixmap.width() != (width()+2) || d->pixmap.height() != (height()+2) ) {
    setFixedSize( d->pixmap.width()+2, d->pixmap.height()+2);
    resize(d->pixmap.width()+2, d->pixmap.height()+2);
  }

  if( d->timer.isActive())
    loadRemainingIcons();
}

void KAnimWidget::loadRemainingIcons()
{
  QStringList::Iterator it = d->icons.begin();
  for( it++; it != d->icons.end(); ++it)
  {
    d->pixmaps.append( MainBarIcon( *it, d->size ) );
  }

  d->loadingCompleted = true;
}

#include "kanimwidget.moc"
