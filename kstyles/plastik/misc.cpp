/*
 * Copyright 2003, Sandro Giessl <ceebx@users.sourceforge.net>
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

#include <qcolor.h>
#include "misc.h"

QColor alphaBlendColors(const QColor &bgColor, const QColor &fgColor, const int a)
{

    // normal button...
    QRgb rgb = bgColor.rgb();
    QRgb rgb_b = fgColor.rgb();
    int alpha = a;
    if(alpha>255) alpha = 255;
    if(alpha<0) alpha = 0;
    int inv_alpha = 255 - alpha;

    QColor result  = QColor( qRgb(qRed(rgb_b)*inv_alpha/255 + qRed(rgb)*alpha/255,
                                  qGreen(rgb_b)*inv_alpha/255 + qGreen(rgb)*alpha/255,
                                  qBlue(rgb_b)*inv_alpha/255 + qBlue(rgb)*alpha/255) );

    return result;
}
