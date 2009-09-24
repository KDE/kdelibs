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

#include "kpixmapsequencewidget.h"
#include "kpixmapsequenceoverlaypainter.h"
#include "kpixmapsequence.h"

#include <kdebug.h>


class KPixmapSequenceWidget::Private
{
public:
    KPixmapSequenceOverlayPainter m_painter;
};


KPixmapSequenceWidget::KPixmapSequenceWidget(QWidget *parent)
        : QWidget(parent),
        d(new Private)
{
    d->m_painter.setWidget(this);
    setSequence(d->m_painter.sequence());
}


KPixmapSequenceWidget::~KPixmapSequenceWidget()
{
    delete d;
}


KPixmapSequence KPixmapSequenceWidget::sequence() const
{
    return d->m_painter.sequence();
}


int KPixmapSequenceWidget::interval() const
{
    return d->m_painter.interval();
}


QSize KPixmapSequenceWidget::sizeHint() const
{
    if(d->m_painter.sequence().isValid())
        return d->m_painter.sequence().frameSize();
    else
        return QWidget::sizeHint();
}


void KPixmapSequenceWidget::setSequence(const KPixmapSequence &seq)
{
    d->m_painter.setSequence(seq);
    if(seq.isValid()) {
        setFixedSize(seq.frameSize());
        d->m_painter.start();
    }
    else {
        d->m_painter.stop();
    }
}


void KPixmapSequenceWidget::setInterval(int msecs)
{
    d->m_painter.setInterval(msecs);
}

#include "kpixmapsequencewidget.moc"
