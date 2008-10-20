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
#include <QMovie>

#include <kdebug.h>
#include <kiconloader.h>

class KAnimatedButtonPrivate
{
public:
    KAnimatedButtonPrivate(KAnimatedButton *qq)
        : q(qq), movie(0)
    {
    }

    void updateCurrentIcon();
    void _k_movieFrameChanged(int number);
    void _k_movieFinished();

    KAnimatedButton *q;
    QMovie *movie;

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
  delete d->movie;

  delete d;
}

void KAnimatedButton::start()
{
    if (d->movie) {
        d->movie->start();
    } else {
        d->current_frame = 0;
        d->timer.start(50);
    }
}

void KAnimatedButton::stop()
{
    if (d->movie) {
        d->movie->stop();
        d->movie->jumpToFrame(0);
        d->_k_movieFrameChanged(0);
    } else {
       d->current_frame = 0;
       d->timer.stop();
       d->updateCurrentIcon();
    }
}

void KAnimatedButton::setIcons( const QString& icons )
{
  if ( d->icon_name == icons )
    return;

  d->timer.stop();
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

void KAnimatedButtonPrivate::_k_movieFrameChanged(int number)
{
    Q_UNUSED(number);
    q->setIcon(QIcon(movie->currentPixmap()));
}

void KAnimatedButtonPrivate::_k_movieFinished()
{
    // if not running, make it loop
    if (movie->state() == QMovie::NotRunning) {
        movie->start();
    }
}

void KAnimatedButton::updateIcons()
{
    QMovie *movie = KIconLoader::global()->loadMovie(d->icon_name, KIconLoader::NoGroup, -iconDimensions());
    if (movie) {
        d->frames = 0;
        d->pixmap = QPixmap();
        movie->setCacheMode(QMovie::CacheAll);
        connect(movie, SIGNAL(frameChanged(int)), this, SLOT(_k_movieFrameChanged(int)));
        connect(movie, SIGNAL(finished()), this, SLOT(_k_movieFinished()));
    } else {
        const QString path = KIconLoader::global()->iconPath(d->icon_name, -iconDimensions());
        QImage img(path);
        if (img.isNull())
            return;

        d->frames = img.height() / img.width();
        if (d->pixmap.width() != iconDimensions()) {
            img = img.scaled(iconDimensions(), iconDimensions() * d->frames,
                             Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        }
        d->pixmap = QPixmap::fromImage(img);
    }

    d->current_frame = 0;
    qDeleteAll(d->framesCache);
    d->framesCache.resize(d->frames);
    delete d->movie;
    d->movie = movie;

    if (d->movie) {
        d->movie->jumpToFrame(0);
        d->_k_movieFrameChanged(0);
    } else {
        d->updateCurrentIcon();
    }
}

int KAnimatedButton::iconDimensions() const
{
  return qMin(iconSize().width(), iconSize().height());
}

#include "kanimatedbutton.moc"
