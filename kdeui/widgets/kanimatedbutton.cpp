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

  QPixmap* frame = framesCache[current_frame];
  if (!frame)
  {
        const int icon_size = q->iconDimensions();
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

void KAnimatedButton::updateIcons()
{
    const int icon_size = iconDimensions();
    d->pixmap = QPixmap();
    QMovie *movie = KIconLoader::global()->loadMovie(d->icon_name, KIconLoader::NoGroup, -icon_size);
    if (movie) {
        d->frames = 0;
        movie->setCacheMode(QMovie::CacheAll);
        connect(movie, SIGNAL(frameChanged(int)), this, SLOT(_k_movieFrameChanged(int)));
        connect(movie, SIGNAL(finished()), this, SLOT(_k_movieFinished()));
    } else {
        const QString path = KIconLoader::global()->iconPath(d->icon_name, -icon_size);
        QImage img(path);
        if (img.isNull())
            return;

        if ((img.width() % icon_size != 0) || (img.height() % icon_size != 0))
            return;

        d->frames = (img.height() / icon_size) * (img.width() / icon_size);
        d->pixmap = QPixmap::fromImage(img);
    }

    d->current_frame = 0;
    qDeleteAll(d->framesCache);
    d->framesCache.fill(0);
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
