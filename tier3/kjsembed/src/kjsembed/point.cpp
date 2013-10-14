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
#include "point.h"

#include <QtCore/QDebug>
#include <QtCore/QPoint>

using namespace KJSEmbed;

const KJS::ClassInfo PointBinding::info = { "QPoint", &VariantBinding::info, 0, 0 };
PointBinding::PointBinding( KJS::ExecState *exec, const QPoint &value )
    : VariantBinding(exec, value )
{
    StaticBinding::publish( exec, this, Point::methods() );
    StaticBinding::publish( exec, this, VariantFactory::methods() );
}

namespace PointNS
{

START_VARIANT_METHOD( callisNull, QPoint )
    bool cppValue = value.isNull();
    result = KJS::jsBoolean(cppValue);
END_VARIANT_METHOD

START_VARIANT_METHOD( callmanhattanLength, QPoint )
    int cppValue = value.manhattanLength();
    result = KJS::jsNumber(cppValue);
END_VARIANT_METHOD

START_VARIANT_METHOD( callrx, QPoint )
    int cppValue = value.rx();
    result = KJS::jsNumber(cppValue);
END_VARIANT_METHOD

START_VARIANT_METHOD( callry, QPoint )
    int cppValue = value.ry();
    result = KJS::jsNumber(cppValue);
END_VARIANT_METHOD

START_VARIANT_METHOD( callsetX, QPoint )
    int arg0 = KJSEmbed::extractInt(exec, args, 0);
    value.setX(arg0);
END_VARIANT_METHOD

START_VARIANT_METHOD( callsetY, QPoint )
    int arg0 = KJSEmbed::extractInt(exec, args, 0);
    value.setY(arg0);
END_VARIANT_METHOD

START_VARIANT_METHOD( callx, QPoint )
    int cppValue = value.x();
    result = KJS::jsNumber(cppValue);
END_VARIANT_METHOD

START_VARIANT_METHOD( cally, QPoint )
    int cppValue = value.y();
    result = KJS::jsNumber(cppValue);
END_VARIANT_METHOD

}

START_METHOD_LUT( Point )
    {"isNull", 0, KJS::DontDelete|KJS::ReadOnly, &PointNS::callisNull},
    {"manhattanLength", 0, KJS::DontDelete|KJS::ReadOnly, &PointNS::callmanhattanLength},
    {"rx", 0, KJS::DontDelete|KJS::ReadOnly, &PointNS::callrx},
    {"ry", 0, KJS::DontDelete|KJS::ReadOnly, &PointNS::callry},
    {"setX", 0, KJS::DontDelete|KJS::ReadOnly, &PointNS::callsetX},
    {"setY", 0, KJS::DontDelete|KJS::ReadOnly, &PointNS::callsetY},
    {"x", 0, KJS::DontDelete|KJS::ReadOnly, &PointNS::callx},
    {"y", 0, KJS::DontDelete|KJS::ReadOnly, &PointNS::cally}
END_METHOD_LUT

NO_ENUMS( Point )
NO_STATICS( Point )

START_CTOR( Point, QPoint, 0 )
    if( args.size() == 2 )
    {
        return new KJSEmbed::PointBinding(exec,
                                QPoint( KJSEmbed::extractInt( exec, args, 0 ),
                                        KJSEmbed::extractInt( exec, args, 1 )
                                        ) );
    }

    return new KJSEmbed::PointBinding( exec, QPoint() );
END_CTOR

//kate: indent-spaces on; indent-width 4; replace-tabs on; indent-mode cstyle;

