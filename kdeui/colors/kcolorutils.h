/* This file is part of the KDE project
 * Copyright (C) 2007 Matthew Woehlke <mw_triad@users.sourceforge.net>
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

#ifndef KCOLORUTILS_H
#define KCOLORUTILS_H

#include <kdeui_export.h>
#include <QPainter>

class QColor;

/**
 * A set of methods used to work with colors.
 */
namespace KColorUtils {
    /**
     * Blend two colors into a new color by linear combination.
     * @code
        QColor lighter = KColorUtils::mix(myColor, Qt::white)
     * @endcode
     * @param c1 first color.
     * @param c2 second color.
     * @param bias weight to be used for the mix. @p bias <= 0 gives @p c1,
     * @p bias >= 1 gives @p c2. @p bias == 0.5 gives a 50% blend of @p c1
     * and @p c2.
     */
    KDEUI_EXPORT QColor mix(const QColor &one, const QColor &two,
                            qreal bias = 0.5);

    /**
     * Blend two colors into a new color by painting the second color over the
     * first using the specified composition mode.
     * @code
        QColor white(Qt::white);
        white.setAlphaF(0.5);
        QColor lighter = KColorUtils::overlayColors(myColor, white);
       @endcode
     * @param base the base color (alpha channel is ignored).
     * @param paint the color to be overlayed onto the base color.
     * @param comp the CompositionMode used to do the blending.
     */
    KDEUI_EXPORT QColor overlayColors(const QColor &base, const QColor &paint,
                                      QPainter::CompositionMode comp = QPainter::CompositionMode_SourceOver);

}

#endif // KCOLORUTILS_H
// kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on;
