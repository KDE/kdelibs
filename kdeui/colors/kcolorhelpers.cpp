/*
 * This file is part of the KDE project.
 * Copyright © 2010 Christoph Feck <christoph@maxiom.de>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License version 2 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "kcolorhelpers_p.h"

#include <QtGui/QColor>
#include <QtGui/QPainter>
#include <QtGui/QPixmap>

namespace KDEPrivate {

void fillOpaqueRect(QPainter *painter, const QRect &rect, const QBrush &brush)
{
    if (!brush.isOpaque()) {
        QPixmap chessboardPattern(16, 16);
        QPainter patternPainter(&chessboardPattern);
        patternPainter.fillRect(0, 0, 8, 8, Qt::black);
        patternPainter.fillRect(8, 8, 8, 8, Qt::black);
        patternPainter.fillRect(0, 8, 8, 8, Qt::white);
        patternPainter.fillRect(8, 0, 8, 8, Qt::white);
        patternPainter.end();
        painter->fillRect(rect, QBrush(chessboardPattern));
    }
    painter->fillRect(rect, brush);
}

} // namespace KDEPrivate
