/*
    This file is part of KNewStuff2.
    Copyright (c) 2006, 2007 Josef Spillner <spillner@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "qstarframe.h"

#include <QtGui/QPainter>
#include <QtGui/QPixmap>

#include <kstandarddirs.h>

QStarFrame::QStarFrame(QWidget *parent)
        : QFrame(parent)
{
    setFixedHeight(24);
    setFrameStyle(QFrame::Sunken | QFrame::Panel);

    m_rating = 0;
}

void QStarFrame::slotRating(int rating)
{
    m_rating = rating;

    drawstars();
}

void QStarFrame::drawstars()
{
    QString starpath = KStandardDirs::locate("data", "knewstuff/pics/ghns_star.png");
    QString graystarpath = KStandardDirs::locate("data", "knewstuff/pics/ghns_star_gray.png");

    QPixmap star(starpath);
    QPixmap graystar(graystarpath);

    int wpixels = (int)(width() * (float)m_rating / 100.0);

    QPainter p;
    p.begin(this);
    int w = star.width();
    for (int i = 0; i < wpixels; i += star.width()) {
        w = wpixels - i;
        if (w > star.width()) w = star.width();
        p.drawPixmap(i, 0, star, 0, 0, w, -1);
    }
    p.drawPixmap(wpixels, 0, graystar, w, 0, graystar.width() - w, -1);
    wpixels += graystar.width() - w;
    for (int i = wpixels; i < width(); i += graystar.width()) {
        w = width() - i;
        if (w > graystar.width()) w = graystar.width();
        p.drawPixmap(i, 0, graystar, 0, 0, w, -1);
    }
    p.end();
}

void QStarFrame::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e);

    drawstars();
}

#include "qstarframe.moc"
