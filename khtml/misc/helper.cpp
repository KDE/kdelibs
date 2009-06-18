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
#include <QtCore/QMap>
#include <QtGui/QPainter>
#include <dom/dom_string.h>
#include <xml/dom_stringimpl.h>
#include <rendering/render_object.h>
#include <kconfig.h>
#include <ksharedconfig.h>
#include <kconfiggroup.h>
#include <QtGui/QToolTip>
#include "css/cssvalues.h"

using namespace DOM;
using namespace khtml;

namespace khtml {

QPainter *printpainter;

void setPrintPainter( QPainter *printer )
{
    printpainter = printer;
}

void findWordBoundary(QChar *chars, int len, int position, int *start, int *end)
{
    if (chars[position].isSpace()) {
        int pos = position;
        while (pos >= 0 && chars[pos].isSpace())
            pos--;
        *start = pos+1;
        pos = position;
        while (pos < (int)len && chars[pos].isSpace())
            pos++;
        *end = pos;
    } else if (chars[position].isPunct()) {
        int pos = position;
        while (pos >= 0 && chars[pos].isPunct())
            pos--;
        *start = pos+1;
        pos = position;
        while (pos < (int)len && chars[pos].isPunct())
            pos++;
        *end = pos;
    } else {
        int pos = position;
        while (pos >= 0 && !chars[pos].isSpace() && !chars[pos].isPunct())
            pos--;
        *start = pos+1;
        pos = position;
        while (pos < (int)len && !chars[pos].isSpace() && !chars[pos].isPunct())
            pos++;
        *end = pos;
    }
}

}

// color mapping code
struct colorMap {
    int css_value;
    QRgb color;
};

static const colorMap cmap[] = {
    { CSS_VAL_AQUA, 0xFF00FFFF },
    { CSS_VAL_BLACK, 0xFF000000 },
    { CSS_VAL_BLUE, 0xFF0000FF },
    { CSS_VAL_CRIMSON, 0xFFDC143C },
    { CSS_VAL_FUCHSIA, 0xFFFF00FF },
    { CSS_VAL_GRAY, 0xFF808080 },
    { CSS_VAL_GREEN, 0xFF008000  },
    { CSS_VAL_INDIGO, 0xFF4B0082 },
    { CSS_VAL_LIME, 0xFF00FF00 },
    { CSS_VAL_MAROON, 0xFF800000 },
    { CSS_VAL_NAVY, 0xFF000080 },
    { CSS_VAL_OLIVE, 0xFF808000  },
    { CSS_VAL_ORANGE, 0xFFFFA500 },
    { CSS_VAL_PURPLE, 0xFF800080 },
    { CSS_VAL_RED, 0xFFFF0000 },
    { CSS_VAL_SILVER, 0xFFC0C0C0 },
    { CSS_VAL_TEAL, 0xFF008080  },
    { CSS_VAL_WHITE, 0xFFFFFFFF },
    { CSS_VAL_YELLOW, 0xFFFFFF00 },
    { CSS_VAL_TRANSPARENT, transparentColor },
    { CSS_VAL_GREY, 0xff808080 },
    { 0, 0 }
};

struct uiColors {
    int css_value;
    const char * configGroup;
    const char * configEntry;
QPalette::ColorGroup group;
QPalette::ColorRole role;
};

const char * const wmgroup = "WM";
const char * const generalgroup = "General";

/* Mapping system settings to CSS 2
* Tried hard to get an appropriate mapping - schlpbch
*/
static const uiColors uimap[] = {
	// Active window border.
    { CSS_VAL_ACTIVEBORDER, wmgroup, "background", QPalette::Active, QPalette::Light },
	// Active window caption.
    { CSS_VAL_ACTIVECAPTION, wmgroup, "background", QPalette::Active, QPalette::Text },
        // Text in caption, size box, and scrollbar arrow box.
    { CSS_VAL_CAPTIONTEXT, wmgroup, "activeForeground", QPalette::Active, QPalette::Text },
	// Face color for three-dimensional display elements.
    { CSS_VAL_BUTTONFACE, wmgroup, 0, QPalette::Inactive, QPalette::Button },
	// Dark shadow for three-dimensional display elements (for edges facing away from the light source).
    { CSS_VAL_BUTTONHIGHLIGHT, wmgroup, 0, QPalette::Inactive, QPalette::Light },
	// Shadow color for three-dimensional display elements.
    { CSS_VAL_BUTTONSHADOW, wmgroup, 0, QPalette::Inactive, QPalette::Shadow },
	// Text on push buttons.
    { CSS_VAL_BUTTONTEXT, wmgroup, "buttonForeground", QPalette::Inactive, QPalette::ButtonText },
	// Dark shadow for three-dimensional display elements.
    { CSS_VAL_THREEDDARKSHADOW, wmgroup, 0, QPalette::Inactive, QPalette::Dark },
	// Face color for three-dimensional display elements.
    { CSS_VAL_THREEDFACE, wmgroup, 0, QPalette::Inactive, QPalette::Button },
	// Highlight color for three-dimensional display elements.
    { CSS_VAL_THREEDHIGHLIGHT, wmgroup, 0, QPalette::Inactive, QPalette::Light },
	// Light color for three-dimensional display elements (for edges facing the light source).
    { CSS_VAL_THREEDLIGHTSHADOW, wmgroup, 0, QPalette::Inactive, QPalette::Midlight },
	// Dark shadow for three-dimensional display elements.
    { CSS_VAL_THREEDSHADOW, wmgroup, 0, QPalette::Inactive, QPalette::Shadow },

    // Inactive window border.
    { CSS_VAL_INACTIVEBORDER, wmgroup, "background", QPalette::Disabled, QPalette::Background },
    // Inactive window caption.
    { CSS_VAL_INACTIVECAPTION, wmgroup, "inactiveBackground", QPalette::Disabled, QPalette::Background },
    // Color of text in an inactive caption.
    { CSS_VAL_INACTIVECAPTIONTEXT, wmgroup, "inactiveForeground", QPalette::Disabled, QPalette::Text },
    { CSS_VAL_GRAYTEXT, wmgroup, 0, QPalette::Disabled, QPalette::Text },

	// Menu background
    { CSS_VAL_MENU, generalgroup, "background", QPalette::Inactive, QPalette::Background },
	// Text in menus
    { CSS_VAL_MENUTEXT, generalgroup, "foreground", QPalette::Inactive, QPalette::Background },

        // Text of item(s) selected in a control.
    { CSS_VAL_HIGHLIGHT, generalgroup, "selectBackground", QPalette::Inactive, QPalette::Background },

    // Text of item(s) selected in a control.
    { CSS_VAL_HIGHLIGHTTEXT, generalgroup, "selectForeground", QPalette::Inactive, QPalette::Background },

	// Background color of multiple document interface.
    { CSS_VAL_APPWORKSPACE, generalgroup, "background", QPalette::Inactive, QPalette::Text },

	// Scroll bar gray area.
    { CSS_VAL_SCROLLBAR, generalgroup, "background", QPalette::Inactive, QPalette::Background },

	// Window background.
    { CSS_VAL_WINDOW, generalgroup, "windowBackground", QPalette::Inactive, QPalette::Base },
	// Window frame.
    { CSS_VAL_WINDOWFRAME, generalgroup, "windowBackground", QPalette::Inactive, QPalette::Background },
        // WindowText
    { CSS_VAL_WINDOWTEXT, generalgroup, "windowForeground", QPalette::Inactive, QPalette::Text },
    { CSS_VAL_TEXT, generalgroup, 0, QPalette::Inactive, QPalette::Text },
    { 0, 0, 0, QPalette::NColorGroups, QPalette::NColorRoles }
};

QColor khtml::colorForCSSValue( int css_value )
{
    // try the regular ones first
    const colorMap *col = cmap;
    while ( col->css_value && col->css_value != css_value )
	++col;
    if ( col->css_value )
	return QColor::fromRgba(col->color);
    else if ( css_value == CSS_VAL_INVERT )
        return QColor();

    const uiColors *uicol = uimap;
    while ( uicol->css_value && uicol->css_value != css_value )
	++uicol;
#ifndef APPLE_CHANGES
    if ( !uicol->css_value ) {
	if ( css_value == CSS_VAL_INFOBACKGROUND )
	    return QToolTip::palette().color( QPalette::Inactive, QPalette::Background );
	else if ( css_value == CSS_VAL_INFOTEXT )
	    return QToolTip::palette().color( QPalette::Inactive, QPalette::Foreground );
	else if ( css_value == CSS_VAL_BACKGROUND ) {
	    KConfig bckgrConfig("kdesktoprc", KConfig::NoGlobals);
	    // Desktop background.
	    return bckgrConfig.group( "Desktop0" ).readEntry("Color1", qApp->palette().color( QPalette::Disabled, QPalette::Background ) );
	}
	return QColor();
    }
#endif

    const QPalette &pal = qApp->palette();
    QColor c = pal.color( uicol->group, uicol->role );
#ifndef APPLE_CHANGES
    if ( uicol->configEntry ) {
	KSharedConfig::Ptr globalConfig = KGlobal::config();
	c = globalConfig->group( uicol->configGroup ).readEntry( uicol->configEntry, c );
    }
#endif

    return c;
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

    return qRgba(static_cast<int>(calcHue(temp1, temp2, h + 1.0 / 3.0) * 255),
                 static_cast<int>(calcHue(temp1, temp2, h) * 255),
                 static_cast<int>(calcHue(temp1, temp2, h - 1.0 / 3.0) * 255),
                 static_cast<int>(a * 255));
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
  return obj->style()->palette().color( QPalette::Active, QPalette::Base );
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
  c1.getHsv(&h1,&s1,&v1);
  c2.getHsv(&h2,&s2,&v2);
  if(h1!=-1 && h2!=-1) { // grey values have no hue
      hdist = qAbs(h1-h2);
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
      hdist = qMin(hdist, HUE_DISTANCE*2);
  }
  return hdist + (qAbs(s1-s2)*128)/(160+qMin(s1,s2)) + qAbs(v1-v2) > CONTRAST_DISTANCE;

#undef CONTRAST_DISTANCE
#undef HUE_DISTANCE

#else	// orginal fast but primitive version by me (LS)

// ### arbitrary value, to be adapted if necessary (LS)
#define CONTRAST_DISTANCE 32

  if (qAbs(c1.Qt::red() - c2.Qt::red()) > CONTRAST_DISTANCE) return true;
  if (qAbs(c1.Qt::green() - c2.Qt::green()) > CONTRAST_DISTANCE) return true;
  if (qAbs(c1.Qt::blue() - c2.Qt::blue()) > CONTRAST_DISTANCE) return true;

  return false;

#undef CONTRAST_DISTANCE

#endif
}
