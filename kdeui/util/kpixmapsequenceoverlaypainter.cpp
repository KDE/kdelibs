/*
  Copyright 2009 Sebastian Trueg <trueg@kde.org>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/

#include "kpixmapsequenceoverlaypainter.h"
#include "kpixmapsequence.h"

#include <QtGui/QWidget>
#include <QtCore/QPoint>
#include <QtCore/QRect>
#include <QtGui/QPainter>
#include <QtCore/QTimer>
#include <QtCore/QEvent>
#include <QtCore/QPointer>
#include <QtCore/QCoreApplication>

#include <kdebug.h>


class KPixmapSequenceOverlayPainter::Private
{
public:
    void _k_timeout();
    void paintFrame();

    KPixmapSequence& sequence();

    QRect pixmapRect();

    KPixmapSequence m_sequence;
    QPointer<QWidget> m_widget;
    Qt::Alignment m_alignment;
    QPoint m_offset;
    QRect m_rect;

    QTimer m_timer;
    int m_counter;

    bool m_started;

    KPixmapSequenceOverlayPainter *q;
};


void KPixmapSequenceOverlayPainter::Private::_k_timeout()
{
    ++m_counter;
    m_counter %= sequence().frameCount();
    if (m_widget)
        m_widget->update(pixmapRect());
}


void KPixmapSequenceOverlayPainter::Private::paintFrame()
{
    QPainter p(m_widget);
    p.drawPixmap(pixmapRect(), sequence().frameAt(m_counter), QRect(QPoint(0, 0), sequence().frameSize()));
}


KPixmapSequence& KPixmapSequenceOverlayPainter::Private::sequence()
{
    // make sure we have a valid default sequence
    if(m_sequence.isEmpty())
        m_sequence = KPixmapSequence("process-working", 22);

    return m_sequence;
}


QRect KPixmapSequenceOverlayPainter::Private::pixmapRect()
{
    QRect rect(m_rect);
    if(!rect.isValid())
        rect = m_widget->rect();

    QPoint pos(rect.topLeft());
    if (m_alignment & Qt::AlignHCenter)
        pos.setX(rect.center().x() - (sequence().frameSize().width() / 2));
    else if (m_alignment & Qt::AlignRight)
        pos.setX(rect.right() - sequence().frameSize().width());

    if (m_alignment & Qt::AlignVCenter)
        pos.setY(rect.center().y() - (sequence().frameSize().height() / 2));
    else if (m_alignment & Qt::AlignBottom)
        pos.setY(rect.bottom() - sequence().frameSize().height());

    pos += m_offset;

    return QRect( pos, sequence().frameSize());
}


KPixmapSequenceOverlayPainter::KPixmapSequenceOverlayPainter(QObject *parent)
        : QObject(parent),
        d(new Private)
{
    d->q = this;
    d->m_widget = 0;
    d->m_alignment = Qt::AlignCenter;
    d->m_started = false;
    setInterval(200);
    connect(&d->m_timer, SIGNAL(timeout()), this, SLOT(_k_timeout()));
}


KPixmapSequenceOverlayPainter::~KPixmapSequenceOverlayPainter()
{
    stop();
    delete d;
}


KPixmapSequence KPixmapSequenceOverlayPainter::sequence() const
{
    return d->sequence();
}


int KPixmapSequenceOverlayPainter::interval() const
{
    return d->m_timer.interval();
}


QRect KPixmapSequenceOverlayPainter::rect() const
{
    if(d->m_rect.isValid()) {
        return d->m_rect;
    }
    else if(d->m_widget) {
        return d->m_widget->rect();
    }
    else {
        return QRect();
    }
}


Qt::Alignment KPixmapSequenceOverlayPainter::alignment() const
{
    return d->m_alignment;
}


QPoint KPixmapSequenceOverlayPainter::offset() const
{
    return d->m_offset;
}


void KPixmapSequenceOverlayPainter::setSequence(const KPixmapSequence &seq)
{
    bool restart = d->m_started;
    stop();
    d->m_sequence = seq;
    if(restart) start();
}


void KPixmapSequenceOverlayPainter::setInterval(int msecs)
{
    d->m_timer.setInterval(msecs);
}


void KPixmapSequenceOverlayPainter::setWidget(QWidget *w)
{
    stop();
    d->m_widget = w;
}


void KPixmapSequenceOverlayPainter::setRect(const QRect &rect)
{
    bool restart = d->m_started;
    stop();
    d->m_rect = rect;
    if(restart) start();
}


void KPixmapSequenceOverlayPainter::setAlignment(Qt::Alignment align)
{
    bool restart = d->m_started;
    stop();
    d->m_alignment = align;
    if(restart) start();
}


void KPixmapSequenceOverlayPainter::setOffset(const QPoint &offset)
{
    bool restart = d->m_started;
    stop();
    d->m_offset = offset;
    if(restart) start();
}


void KPixmapSequenceOverlayPainter::start()
{
    if (d->m_widget) {
        stop();

        d->m_counter = 0;
        d->m_started = true;
        d->m_widget->installEventFilter(this);
        if(d->m_widget->isVisible()) {
            d->m_timer.start();
            d->m_widget->update(d->pixmapRect());
        }
    }
}


void KPixmapSequenceOverlayPainter::stop()
{
    d->m_timer.stop();
    if (d->m_widget) {
        d->m_started = false;
        d->m_widget->removeEventFilter(this);
        d->m_widget->update(d->pixmapRect());
    }
}


bool KPixmapSequenceOverlayPainter::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == d->m_widget ) {
        switch (event->type()) {
        case QEvent::Paint:
            // make sure we paint after everyone else including other event filters
            obj->removeEventFilter(this); // don't recurse...
            QCoreApplication::sendEvent(obj, event);
            d->paintFrame();
            obj->installEventFilter(this); // catch on...
            return true;
        break;
        case QEvent::Hide:
            d->m_timer.stop();
        break;
        case QEvent::Show:
            if(d->m_started) {
                d->m_timer.start();
                d->m_widget->update(d->pixmapRect());
            }
        break;
        default:
        break;
        }
    }

    return false;
}

#include "kpixmapsequenceoverlaypainter.moc"
