/*********************************************************************************
 *                                                                               *
 *   Copyright (C) 2005, 2009 by Albert Astals Cid <aacid@kde.org>               *
 *                                                                               *
 * This library is free software; you can redistribute it and/or                 *
 * modify it under the terms of the GNU Lesser General Public                    *
 * License as published by the Free Software Foundation; either                  *
 * version 2.1 of the License, or (at your option) version 3, or any             *
 * later version accepted by the membership of KDE e.V. (or its                  *
 * successor approved by the membership of KDE e.V.), which shall                *
 * act as a proxy defined in Section 6 of version 3 of the license.              *
 *                                                                               *
 * This library is distributed in the hope that it will be useful,               *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of                *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU             *
 * Lesser General Public License for more details.                               *
 *                                                                               *
 * You should have received a copy of the GNU Lesser General Public              *
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>. *
 *                                                                               *
 *********************************************************************************/

#include "kfontutils.h"

#include <qpainter.h>

qreal KFontUtils::adaptFontSize(QPainter &painter, const QString &string, qreal width, qreal height, qreal maxFontSize, qreal minFontSize, AdaptFontSizeOptions flags)
{
    qreal size = maxFontSize;
    QRectF boundingRect;
    bool done = false;

    while (!done && size > minFontSize) {
        QFont f = painter.font();
        f.setPointSizeF(size);
        painter.setFont(f);
        int qtFlags = Qt::AlignCenter | Qt::TextWordWrap;
        if (flags & DoNotAllowWordWrap) {
            qtFlags = qtFlags & ~Qt::TextWordWrap;
        }
        boundingRect = painter.boundingRect(QRectF(0, 0, width, height), qtFlags, string);
        if (boundingRect.width() == 0 || boundingRect.height() == 0) {
            return -1;
        } else if (boundingRect.width() > width || boundingRect.height() > height) {
            size = qMin(width * size / boundingRect.width(), height * size / boundingRect.height());
        } else {
            done = true;
        }
    }

    return size;
}

qreal KFontUtils::adaptFontSize(QPainter &painter, const QString &text, const QSizeF &availableSize, qreal maxFontSize, qreal minFontSize, AdaptFontSizeOptions flags)
{
    return adaptFontSize(painter, text, availableSize.width(), availableSize.height(), maxFontSize, minFontSize, flags);
}

