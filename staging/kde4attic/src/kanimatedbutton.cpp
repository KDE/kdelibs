/* This file is part of the KDE libraries
   Copyright (C) 2000 Kurt Granroth <granroth@kde.org>
   Copyright (C) 2006 Hamish Rodda <rodda@kde.org>
   Copyright (C) 2008 Pino Toscano <pino@kde.org>

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

#include <QImageReader>
#include <QMovie>
#include <QPainter>
#include <QPixmap>
#include <QTimer>
#include <QToolButton>

class KAnimatedButtonPrivate
{
public:
    KAnimatedButtonPrivate(KAnimatedButton *qq)
        : q(qq), movie(0)
    {
    }

    void updateIcons();
    void updateCurrentIcon();
    void _k_movieFrameChanged(int number);
    void _k_movieFinished();
    void _k_timerUpdate();

    KAnimatedButton *q;
    QMovie *movie;

  int                    frames;
  int                    current_frame;
  QPixmap                pixmap;
  QTimer                 timer;
  QString                icon_path;
  QVector<QPixmap*>      framesCache; // We keep copies of each frame so that
                                      // the icon code can properly cache them in QPixmapCache,
                                      // and not fill it up with dead copies
};

KAnimatedButton::KAnimatedButton( QWidget *parent )
    : QToolButton(parent), d(new KAnimatedButtonPrivate(this))
{
  connect( &d->timer, SIGNAL(timeout()), this, SLOT(_k_timerUpdate()));
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

void KAnimatedButton::setAnimationPath(const QString &path)
{
  if ( d->icon_path == path )
    return;

  d->timer.stop();
  d->icon_path = path;
  d->updateIcons();
}

QString KAnimatedButton::animationPath() const
{
   return d->icon_path;
}

void KAnimatedButtonPrivate::_k_timerUpdate()
{
  if(!q->isVisible())
    return;

  current_frame++;
  if (current_frame == frames)
     current_frame = 0;

  updateCurrentIcon();
}

void KAnimatedButtonPrivate::updateCurrentIcon()
{
  if (pixmap.isNull())
    return;

  QPixmap* frame = framesCache[current_frame];
  if (!frame)
  {
        const int icon_size = qMin(pixmap.width(), pixmap.height());
        const int row_size = pixmap.width() / icon_size;
        const int row = current_frame / row_size;
        const int column = current_frame % row_size;
        frame = new QPixmap(icon_size, icon_size);
    frame->fill(Qt::transparent);
    QPainter p(frame);
        p.drawPixmap(QPoint(0, 0), pixmap, QRect(column * icon_size, row * icon_size, icon_size, icon_size));
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

void KAnimatedButtonPrivate::updateIcons()
{
    pixmap = QPixmap();
    QMovie *newMovie = 0;
    QImageReader reader(icon_path);
    if (QMovie::supportedFormats().contains(reader.format())) {
        newMovie = new QMovie(icon_path);
        frames = 0;
        newMovie->setCacheMode(QMovie::CacheAll);
        QObject::connect(newMovie, SIGNAL(frameChanged(int)), q, SLOT(_k_movieFrameChanged(int)));
        QObject::connect(newMovie, SIGNAL(finished()), q, SLOT(_k_movieFinished()));
    } else {
        const QPixmap pix(icon_path);
        if (pix.isNull())
            return;

        const int icon_size = qMin(pix.width(), pix.height());
        if ((pix.height() % icon_size != 0) || (pix.width() % icon_size != 0))
            return;

        frames = (pix.height() / icon_size) * (pix.width() / icon_size);
        pixmap = pix;
    }

    current_frame = 0;
    qDeleteAll(framesCache);
    framesCache.fill(0);
    framesCache.resize(frames);
    delete movie;
    movie = newMovie;

    if (movie) {
        movie->jumpToFrame(0);
        _k_movieFrameChanged(0);
    } else {
        updateCurrentIcon();
    }
}

#include "moc_kanimatedbutton.cpp"
