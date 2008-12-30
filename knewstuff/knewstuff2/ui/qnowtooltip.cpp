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

#include "qnowtooltip.h"

#include <QtGui/QLabel>
#include <QtCore/QTimer>

void QNowToolTip::display(QWidget *parent, const QRect& rect, const QString& text)
{
    QLabel *l = new QLabel(parent,
                           Qt::Window |
                           Qt::FramelessWindowHint |
                           Qt::WindowStaysOnTopHint |
                           Qt::Dialog);

    //l->setPaletteBackgroundColor(QColor(255, 255, 200)); // FIXME KDE4PORT
    l->setText(text);
    l->move(rect.topLeft());
    l->show();

    QTimer::singleShot(2000, l, SLOT(deleteLater()));
}

