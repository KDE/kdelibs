/* This file is part of the KDE libraries
   Copyright (C) 2000 Kurt Granroth <granroth@kde.org>
   Copyright (C) 2006 Hamish Rodda <rodda@kde.org>

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

#include <kanimatedbutton.h>

#include <QAction>
#include <QPixmap>
#include <QTimer>
#include <QImage>
#include <QToolBar>
#include <QPainter>

#include <kdebug.h>
#include <kiconloader.h>

class KAnimatedButtonPrivate
{
public:
    KAnimatedButtonPrivate(KAnimatedButton *qq)
        : q(qq)
    {
    }

    void updateCurrentIcon();

    KAnimatedButton *q;

  int                    frames;
  int                    current_frame;
  QPixmap                pixmap;
  QTimer                 timer;
  QString                icon_name;
  QVector<QPixmap*>      framesCache; // We keep copies of each frame so that
                                      // the icon code can properly cache them in QPixmapCache,
                                      // and not fill it up with dead copies
};

KAnimatedButton::KAnimatedButton( QWidget *parent )
    : QToolButton(parent), d(new KAnimatedButtonPrivate(this))
{
  connect( &d->timer, SIGNAL(timeout()), this, SLOT(slotTimerUpdate()));
}

KAnimatedButton::~KAnimatedButton()
{
  d->timer.stop();
  qDeleteAll(d->framesCache);

  delete d;
}

void KAnimatedButton::start()
{
  d->current_frame = 0;
  d->timer.start( 50 );
}

void KAnimatedButton::stop()
{
  d->current_frame = 0;
  d->timer.stop();
  d->updateCurrentIcon();
}

void KAnimatedButton::setIcons( const QString& icons )
{
  if ( d->icon_name == icons )
    return;

  d->icon_name = icons;
  updateIcons();
}

QString KAnimatedButton::icons( ) const
{
   return d->icon_name;
}

void KAnimatedButton::slotTimerUpdate()
{
  if(!isVisible())
    return;

  d->current_frame++;
  if (d->current_frame == d->frames)
     d->current_frame = 0;

  d->updateCurrentIcon();
}

void KAnimatedButtonPrivate::updateCurrentIcon()
{
  if (pixmap.isNull())
    return;

  int w = pixmap.width();
  int h = w;


  QPixmap* frame = framesCache[current_frame];
  if (!frame)
  {
    frame = new QPixmap(w, h);
    frame->fill(Qt::transparent);
    QPainter p(frame);
    p.drawPixmap(QPoint(0,0), pixmap, QRect(0, current_frame * h, w, h));
    p.end();
    framesCache[current_frame] = frame;
  }

  q->setIcon(QIcon(*frame));
}

void KAnimatedButton::updateIcons()
{
  QString path = KIconLoader::global()->iconPath(d->icon_name, -iconDimensions());
  QImage img(path);

  if (img.isNull())
     return;

  d->current_frame = 0;
  d->frames = img.height() / img.width();
  if (d->pixmap.width() != iconDimensions())
  {
     img = img.scaled(iconDimensions(), iconDimensions()*d->frames, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
  }
  d->pixmap = QPixmap::fromImage(img);

  qDeleteAll(d->framesCache);
  d->framesCache.resize(d->frames);

  d->updateCurrentIcon();
}

int KAnimatedButton::iconDimensions() const
{
  return qMin(iconSize().width(), iconSize().height());
}

#include "kanimatedbutton.moc"
