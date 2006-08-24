/*
 * This file is part of the CSS implementation for KDE.
 *
 * Copyright (C) 1999-2003 Lars Knoll (knoll@kde.org)
 *           (C) David Carson  <dacarson@gmail.com>
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
 *
 */
#include "helper.h"
#include "khtmllayout.h"
#include <qmap.h>
#include <qpainter.h>
#include <dom/dom_string.h>
#include <xml/dom_stringimpl.h>
#include <rendering/render_object.h>
#include <qptrlist.h>
#include <kstaticdeleter.h>
#include <kapplication.h>
#include <kconfig.h>
#include <qtooltip.h>

using namespace DOM;
using namespace khtml;

namespace khtml {
  QPainter *printpainter;
}

void khtml::setPrintPainter( QPainter *printer )
{
    printpainter = printer;
}


double calcHue(double temp1, double temp2, double hueVal)
{
    if (hueVal < 0)
        hueVal++;
    else if (hueVal > 1)
        hueVal--;
    if (hueVal * 6 < 1)
        return temp1 + (temp2 - temp1) * hueVal * 6;
    if (hueVal * 2 < 1)
        return temp2;
    if (hueVal * 3 < 2)
        return temp1 + (temp2 - temp1) * (2.0 / 3.0 - hueVal) * 6;
    return temp1;
}

// Explanation of this algorithm can be found in the CSS3 Color Module
// specification at http://www.w3.org/TR/css3-color/#hsl-color with further
// explanation available at http://en.wikipedia.org/wiki/HSL_color_space

// all values are in the range of 0 to 1.0
QRgb khtml::qRgbaFromHsla(double h, double s, double l, double a)
{
    double temp2 = l < 0.5 ? l * (1.0 + s) : l + s - l * s;
    double temp1 = 2.0 * l - temp2;

    return qRgba(calcHue(temp1, temp2, h + 1.0 / 3.0) * 255, calcHue(temp1, temp2, h) * 255, calcHue(temp1, temp2, h - 1.0 / 3.0) * 255, a * 255);
}

/** finds out the background color of an element
 * @param obj render object
 * @return the background color. It is guaranteed that a valid color is returned.
 */
QColor khtml::retrieveBackgroundColor(const RenderObject *obj)
{
  QColor result;
  while (!obj->isCanvas()) {
    result = obj->style()->backgroundColor();
    if (result.isValid()) return result;

    obj = obj->container();
  }/*wend*/

  // everything transparent? Use base then.
  return obj->style()->palette().active().base();
}

/** checks whether the given colors have enough contrast
 * @returns @p true if contrast is ok.
 */
bool khtml::hasSufficientContrast(const QColor &c1, const QColor &c2)
{
// New version from Germain Garand, better suited for contrast measurement
#if 1

#define HUE_DISTANCE 40
#define CONTRAST_DISTANCE 10

  int h1, s1, v1, h2, s2, v2;
  int hdist = -CONTRAST_DISTANCE;
  c1.hsv(&h1,&s1,&v1);
  c2.hsv(&h2,&s2,&v2);
  if(h1!=-1 && h2!=-1) { // grey values have no hue
      hdist = kAbs(h1-h2);
      if (hdist > 180) hdist = 360-hdist;
      if (hdist < HUE_DISTANCE) {
          hdist -= HUE_DISTANCE;
          // see if they are high key or low key colours
          bool hk1 = h1>=45 && h1<=225;
          bool hk2 = h2>=45 && h2<=225;
          if (hk1 && hk2)
              hdist = (5*hdist)/3;
          else if (!hk1 && !hk2)
              hdist = (7*hdist)/4;
      }
      hdist = kMin(hdist, HUE_DISTANCE*2);
  }
  return hdist + (kAbs(s1-s2)*128)/(160+kMin(s1,s2)) + kAbs(v1-v2) > CONTRAST_DISTANCE;

#undef CONTRAST_DISTANCE
#undef HUE_DISTANCE

#else	// orginal fast but primitive version by me (LS)

// ### arbitrary value, to be adapted if necessary (LS)
#define CONTRAST_DISTANCE 32

  if (kAbs(c1.red() - c2.red()) > CONTRAST_DISTANCE) return true;
  if (kAbs(c1.green() - c2.green()) > CONTRAST_DISTANCE) return true;
  if (kAbs(c1.blue() - c2.blue()) > CONTRAST_DISTANCE) return true;

  return false;

#undef CONTRAST_DISTANCE

#endif
}
