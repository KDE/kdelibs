/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
 * Copyright (C) 2007 Zack Rusin <zack@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
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

#include <kdeui_export.h>
#include <QPainter>

class QColor;

/**
 * A set of methods used to do graphics operations, like blendColor.
 */
namespace KGraphicsUtils {
    /**
     * Blend 2 colors into a new color with the strenght of the blend based on the alpha channel of the second color.
     * @code
        QColor white(Qt::White);
        white.setAlpha(130);
        QColor lighter = KGraphicsUtils::blendColor(myColor, white);
       @endcode
     * @param one the starting point
     * @param two the end point where we blend towards.
     * @param comp the CompositionMode used to do the blending.
     */
    KDEUI_EXPORT QColor blendColor(const QColor &one, const QColor &two, QPainter::CompositionMode comp = QPainter::CompositionMode_SourceOver);

}
