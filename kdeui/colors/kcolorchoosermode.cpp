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

#include "kcolorchoosermode_p.h"

#include <QtGui/QColor>

namespace KDEPrivate {

qreal getComponentValue(const QColor &color, KColorChooserMode chooserMode)
{
    switch (chooserMode) {
    case ChooserRed:
        return color.redF();
    case ChooserGreen:
        return color.greenF();
    case ChooserBlue:
        return color.blueF();
    case ChooserHue:
        return color.hueF();
    case ChooserSaturation:
        return color.saturationF();
    default:
        return color.valueF();
    }
}

void setComponentValue(QColor &color, KColorChooserMode chooserMode, qreal value)
{
    if (chooserMode >= ChooserRed) {
        if (chooserMode == ChooserRed) {
            color.setRedF(value);
        } else if (chooserMode == ChooserGreen) {
            color.setGreenF(value);
        } else { // chooserMode == ChooserBlue
            color.setBlueF(value);
        }
    } else {
        qreal h, s, v, a;
        color.getHsvF(&h, &s, &v, &a);
        if (chooserMode == ChooserHue) {
            h = value;
        } else if (chooserMode == ChooserSaturation) {
            s = value;
        } else { // chooserMode == ChooserValue
            v = value;
        }
        color.setHsvF(h, s, v, a);
    }
}

} // namespace KDEPrivate
