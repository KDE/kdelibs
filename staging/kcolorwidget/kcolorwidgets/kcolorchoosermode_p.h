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

#ifndef KDEUI_COLORS_KCOLORCHOOSERMODE_PRIVATE_H
#define KDEUI_COLORS_KCOLORCHOOSERMODE_PRIVATE_H

#include "kcolorchoosermode.h"

#include <QtCore/QtGlobal>

class QColor;

namespace KDEPrivate {

// get/set color component
extern qreal getComponentValue(const QColor &color, KColorChooserMode chooserMode);
extern void setComponentValue(QColor &color, KColorChooserMode chooserMode, qreal value);

// number of linear gradient ranges needed for color component
static inline int componentValueSteps(KColorChooserMode chooserMode)
{
    if (chooserMode == ChooserHue) {
        return 6;
    } else {
        return 1;
    }
}

// color component that is used for X in the XY selector
static inline KColorChooserMode chooserXMode(KColorChooserMode chooserMode)
{
    if (chooserMode >= ChooserRed) {
        return chooserMode == ChooserRed ? ChooserGreen : ChooserRed;
    } else {
        return chooserMode == ChooserHue ? ChooserSaturation : ChooserHue;
    }
}

// color component that is used for Y in the XY selector
static inline KColorChooserMode chooserYMode(KColorChooserMode chooserMode)
{
    if (chooserMode >= ChooserRed) {
        return chooserMode == ChooserBlue ? ChooserGreen : ChooserBlue;
    } else {
        return chooserMode == ChooserValue ? ChooserSaturation : ChooserValue;
    }
}

static inline int componentXSteps(KColorChooserMode chooserMode)
{
    return componentValueSteps(chooserXMode(chooserMode));
}

static inline int componentYSteps(KColorChooserMode chooserMode)
{
    return componentValueSteps(chooserYMode(chooserMode));
}

static inline qreal getComponentX(const QColor &color, KColorChooserMode chooserMode)
{
    return getComponentValue(color, chooserXMode(chooserMode));
}

static inline qreal getComponentY(const QColor &color, KColorChooserMode chooserMode)
{
    return getComponentValue(color, chooserYMode(chooserMode));
}

static inline void setComponentX(QColor &color, KColorChooserMode chooserMode, qreal x)
{
    setComponentValue(color, chooserXMode(chooserMode), x);
}

static inline void setComponentY(QColor &color, KColorChooserMode chooserMode, qreal y)
{
    setComponentValue(color, chooserYMode(chooserMode), y);
}

} // namespace KDEPrivate

#endif /* KDEUI_COLORS_KCOLORCHOOSERMODE_PRIVATE_H */
