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

#ifndef KNEWSTUFF2_UI_QSTARFRAME_H
#define KNEWSTUFF2_UI_QSTARFRAME_H

#include <QtGui/QFrame>

/**
 * Rating visualization class.
 *
 * Ratings between 0 and 100 can be displayed graphically with this
 * class in a way much more appealing than with numbers or progress
 * bars.
 *
 * This class is used internally by the KDXSRating class.
 *
 * @internal
 */
class QStarFrame: public QFrame
{
    Q_OBJECT
public:
    QStarFrame(QWidget *parent);
public Q_SLOTS:
    void slotRating(int rating);
protected:
    void paintEvent(QPaintEvent *e);
private:
    void drawstars();

    int m_rating;
};

#endif
