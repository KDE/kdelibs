/*
  Copyright 2008 Aurélien Gâteau <agateau@kde.org>
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

#include "kpixmapsequence.h"

#include <QPixmap>
#include <QPainter>
#include <QtCore/QVector>
#include <QtCore/QtDebug>

class KPixmapSequence::Private : public QSharedData
{
public:
    QVector<QPixmap> mFrames;

    void loadSequence(const QPixmap& bigPixmap, const QSize &frameSize);
};


void KPixmapSequence::Private::loadSequence(const QPixmap& bigPixmap, const QSize &frameSize)
{
    if(bigPixmap.isNull()) {
        qWarning() << "Invalid pixmap specified.";
        return;
    }

    QSize size(frameSize);
    if(!size.isValid()) {
        size = QSize(bigPixmap.width(), bigPixmap.width());
    }
    if(bigPixmap.width() % size.width() ||
       bigPixmap.height() % size.height()) {
        qWarning() << "Invalid framesize.";
        return;
    }

    const int rowCount = bigPixmap.height() / size.height();
    const int colCount = bigPixmap.width() / size.width();
    mFrames.resize(rowCount * colCount);

    int pos = 0;
    for (int row = 0; row < rowCount; ++row) {
        for (int col = 0; col < colCount; ++col) {
            QPixmap pix = bigPixmap.copy(col * size.width(), row * size.height(), size.width(), size.height());
            mFrames[pos++] = pix;
        }
    }
}


KPixmapSequence::KPixmapSequence()
        : d(new Private)
{
}


KPixmapSequence::KPixmapSequence(const KPixmapSequence &other)
{
    d = other.d;
}


KPixmapSequence::KPixmapSequence(const QPixmap &bigPixmap, const QSize &frameSize)
        : d(new Private)
{
    d->loadSequence(bigPixmap, frameSize);
}


KPixmapSequence::KPixmapSequence(const QString &fullPath, int size)
        : d(new Private)
{
    d->loadSequence(QPixmap(fullPath), QSize(size, size));
}


KPixmapSequence::~KPixmapSequence()
{
}


KPixmapSequence &KPixmapSequence::operator=(const KPixmapSequence & other)
{
    d = other.d;
    return *this;
}


bool KPixmapSequence::isValid() const
{
    return !isEmpty();
}


bool KPixmapSequence::isEmpty() const
{
    return d->mFrames.isEmpty();
}


QSize KPixmapSequence::frameSize() const
{
    if (isEmpty()) {
        qWarning() << "No frame loaded";
        return QSize();
    }
    return d->mFrames[0].size();
}


int KPixmapSequence::frameCount() const
{
    return d->mFrames.size();
}


QPixmap KPixmapSequence::frameAt(int index) const
{
    if (isEmpty()) {
        qWarning() << "No frame loaded";
        return QPixmap();
    }
    return d->mFrames.at(index);
}
