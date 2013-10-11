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


#ifndef RECT_H
#define RECT_H

#include <kjsembed/variant_binding.h>
#include <kjsembed/static_binding.h>

class QRect;

namespace KJSEmbed
{
    /** @internal Implements the binding for QRect. */
    class RectBinding : public VariantBinding
    {
        public:
            RectBinding( KJS::ExecState *exec, const QRect &value );
    };

    /**
    * Constructors:
    *
    * Methods:
    */
    KJS_BINDING( Rect )
}
#endif

//kate: indent-spaces on; indent-width 4; replace-tabs on; indent-mode cstyle;
