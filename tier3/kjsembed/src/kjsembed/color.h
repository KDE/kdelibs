/* This file is part of the KDE libraries
    Copyright (C) 2005, 2006 Ian Reinhart Geiser <geiseri@kde.org>
    Copyright (C) 2005, 2006 Matt Broadstone <mbroadst@gmail.com>
    Copyright (C) 2005, 2006 Richard J. Moore <rich@kde.org>
    Copyright (C) 2005, 2006 Erik L. Bunce <kde@bunce.us>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef COLOR_H
#define COLOR_H

#include "variant_binding.h"
#include "static_binding.h"

class QColor;

namespace KJSEmbed
{
    /** @internal Implements the binding for QColor. */
    class ColorBinding : public VariantBinding
    {
        public:
            ColorBinding( KJS::ExecState *exec, const QColor &value );
            virtual const KJS::ClassInfo* classInfo() const { return &info; }
            static const KJS::ClassInfo info;
    };

    /**
    * Constructors:
    * Color( name )
    * Color( r, g, b)
    * Color( r, g, b, a)
    *
    * Methods:
    * setAlpha()
    * setBlue()
    * setCyan()
    * setGreen()
    * setRbg(r, g, b, a=255)
    * setCmyk(c, m, y, k, a=255)
    * setHsv(h, s, v, a=255)
    * setNamedColor( name )
    * alpha()
    * blue()
    * cyan()
    * green()
    * hue()
    * magenta()
    * red()
    * yello()
    * saturation()
    * light( factor )
    * dark( factor )
    * spec()
    * convertTo(spec)
    */
    KJS_BINDING( Color )

}
#endif
//kate: indent-spaces on; indent-width 4; replace-tabs on; indent-mode cstyle;
