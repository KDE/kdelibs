/*
 * This file is part of the CSS implementation for KDE.
 *
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
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
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * $Id$
 */
#include "csshelper.h"

#include <qfontmetrics.h>
#include <qfontinfo.h>
#include <qpaintdevice.h>

#include "rendering/render_style.h"
#include "css_valueimpl.h"
#include "dom/css_value.h"

#include "kdebug.h"

using namespace DOM;
using namespace khtml;

int khtml::computeLength(DOM::CSSPrimitiveValueImpl *val, RenderStyle *style, Metrics m)
{
    unsigned short type = val->primitiveType();
    float factor = 1.;
    switch(type)
    {
    case CSSPrimitiveValue::CSS_EMS:
    case CSSPrimitiveValue::CSS_EXS:
    {
        if(type == CSSPrimitiveValue::CSS_EMS)
        {
#if 0
            QFontMetrics fm(style->font());
            factor = fm.height();
#else
            QFontInfo fi(style->font());
            factor = fi.pointSize();
#endif

            //kdDebug( 6080 ) << "EM = " << factor << endl;
        }
        else
        {
            QFontMetrics fm(style->font());
            QRect b = fm.boundingRect('x');
            factor = b.height();
        }
        break;
    }
    case CSSPrimitiveValue::CSS_PX:
        break;
    case CSSPrimitiveValue::CSS_CM:
        if(m == MetricScreen)
            factor = QPaintDevice::x11AppDpiY()/2.54; //72dpi/(2.54 cm/in)
        else
            factor = 300./2.54; //300dpi/(2.54 cm/in)
        break;
    case CSSPrimitiveValue::CSS_MM:
        if(m == MetricScreen)
            factor = QPaintDevice::x11AppDpiY()/25.4;
        else
            factor = 300./25.4;
        break;
    case CSSPrimitiveValue::CSS_IN:
        if(m == MetricScreen)
            factor = QPaintDevice::x11AppDpiY();
        else
            factor = 300.;
        break;
    case CSSPrimitiveValue::CSS_PT:
        if(m == MetricScreen)
            factor = QPaintDevice::x11AppDpiY()/72.;
        else
            factor = 300./72.;
        break;
    case CSSPrimitiveValue::CSS_PC:
        // 1 pc == 12 pt
        if(m == MetricScreen)
            factor = QPaintDevice::x11AppDpiY()*12./72.;
        else
            factor = 300./72.*12.;
        break;
    default:
        return -1;
    }
    return (int)(val->getFloatValue(type)*factor);
}

DOMString khtml::parseURL(const DOMString &url)
{
    QString s = url.string().stripWhiteSpace();
    if(s.find("url(") == 0 && s[s.length()-1] == ')')
        s = s.mid(4, s.length()-5);

    s = s.stripWhiteSpace();

    if(s[0] == '\"' && s[s.length()-1] == '\"')
        s = s.mid(1, s.length()-2);

    if(s[0] == '\'' && s[s.length()-1] == '\'')
        s = s.mid(1, s.length()-2);

    // remove unnecessary newlines etc from beginning/end
    // another "just like Netscape" feature
    s = s.stripWhiteSpace();

    return DOMString(s.unicode(), s.length());
}

