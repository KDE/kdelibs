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

#include <QtGui/QPixmap>
#include <QtGui/QPainter>
#include <QtCore/QVector>

#include <kiconloader.h>
#include <kdebug.h>


class KPixmapSequence::Private : public QSharedData
{
public:
    QVector<QPixmap> mFrames;

    void loadSequence(const QPixmap& bigPixmap, const QSize &frameSize);
};


void KPixmapSequence::Private::loadSequence(const QPixmap& bigPixmap, const QSize &frameSize)
{
    if(bigPixmap.isNull()) {
        kDebug() << "Invalid pixmap specified.";
    }
    else {
        QSize size(frameSize);
        if(!size.isValid()) {
            size = QSize(bigPixmap.width(), bigPixmap.width());
        }
        if(bigPixmap.width() % size.width() ||
           bigPixmap.height() % size.height()) {
            kWarning() << "Invalid framesize.";
            return;
        }

        mFrames.resize((bigPixmap.height()/size.height()) * (bigPixmap.width()/size.width()));

        int pos = 0;
        for (int row = 0; row < bigPixmap.height()/size.height(); ++row) {
            for (int col = 0; col < bigPixmap.width()/size.width(); ++col) {
                QPixmap pix = QPixmap(size);
                pix.fill(Qt::transparent);
                QPainter painter(&pix);
                painter.drawPixmap(QPoint(0, 0), bigPixmap, QRect(col*size.width(), row*size.height(), size.width(), size.height()));
                painter.end();
                mFrames[pos++] = pix;
            }
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


KPixmapSequence::KPixmapSequence(const QString &xdgIconName, int size)
        : d(new Private)
{
    d->loadSequence(QPixmap(KIconLoader::global()->iconPath(xdgIconName, -size)), QSize(size, size));
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
    if (d->mFrames.size() == 0) {
        kWarning() << "No frame loaded";
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
    return d->mFrames.at(index);
}
