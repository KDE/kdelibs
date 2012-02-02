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
#include "pen.h"

#include <QtCore/QDebug>
#include <QPen>
#include <QBrush>
#include <QColor>

using namespace KJSEmbed;

const KJS::ClassInfo PenBinding::info = { "QPen", &VariantBinding::info, 0, 0 };
PenBinding::PenBinding( KJS::ExecState *exec, const QPen &value )
    : VariantBinding(exec, value )
{
    StaticBinding::publish( exec, this, Pen::methods() );
    StaticBinding::publish( exec, this, VariantFactory::methods() );
}

namespace PenNS
{

START_VARIANT_METHOD( callbrush, QPen )
    QBrush cppValue = value.brush();
    result = KJSEmbed::createVariant(exec, "QBrush", cppValue );
END_VARIANT_METHOD

START_VARIANT_METHOD( callcapStyle, QPen )
    Qt::PenCapStyle cppValue = value.capStyle();
    result = KJS::jsNumber( cppValue );
END_VARIANT_METHOD

START_VARIANT_METHOD( callcolor, QPen )
    QColor cppValue = value.color();
    result = KJSEmbed::createVariant(exec, "QColor", cppValue );
END_VARIANT_METHOD

START_VARIANT_METHOD( callisSolid, QPen )
    bool cppValue = value.isSolid();
    result = KJS::jsBoolean(cppValue);
END_VARIANT_METHOD

START_VARIANT_METHOD( calljoinStyle, QPen )
    Qt::PenJoinStyle cppValue = value.joinStyle();
    result = KJS::jsNumber( cppValue );
END_VARIANT_METHOD

START_VARIANT_METHOD( callsetBrush, QPen )
    QBrush arg0 = KJSEmbed::extractVariant<QBrush>(exec,args, 0);
    value.setBrush(arg0);
END_VARIANT_METHOD

START_VARIANT_METHOD( callsetCapStyle, QPen )
    Qt::PenCapStyle arg0 = (Qt::PenCapStyle)KJSEmbed::extractInt(exec, args, 0);
    value.setCapStyle(arg0);
END_VARIANT_METHOD

START_VARIANT_METHOD( callsetColor, QPen )
    QColor arg0 = KJSEmbed::extractVariant<QColor>(exec,args, 0);
    value.setColor(arg0);
END_VARIANT_METHOD

START_VARIANT_METHOD( callsetJoinStyle, QPen )
    Qt::PenJoinStyle arg0  = (Qt::PenJoinStyle)KJSEmbed::extractInt(exec, args, 0);
    value.setJoinStyle(arg0);
END_VARIANT_METHOD

START_VARIANT_METHOD( callsetStyle, QPen )
    Qt::PenStyle arg0  = (Qt::PenStyle)KJSEmbed::extractInt(exec, args, 0);
    value.setStyle(arg0);
END_VARIANT_METHOD

START_VARIANT_METHOD( callsetWidth, QPen )
    int arg0 = KJSEmbed::extractInt(exec, args, 0);
    value.setWidth(arg0);
END_VARIANT_METHOD

START_VARIANT_METHOD( callstyle, QPen )
    Qt::PenStyle cppValue = value.style();
    result = KJS::jsNumber( cppValue );
END_VARIANT_METHOD

START_VARIANT_METHOD( callwidth, QPen )
    int cppValue = value.width();
    result = KJS::jsNumber(cppValue);
END_VARIANT_METHOD

}

START_METHOD_LUT( Pen )
    {"brush", 0, KJS::DontDelete|KJS::ReadOnly, &PenNS::callbrush},
    {"capStyle", 0, KJS::DontDelete|KJS::ReadOnly, &PenNS::callcapStyle},
    {"color", 0, KJS::DontDelete|KJS::ReadOnly, &PenNS::callcolor},
    {"isSolid", 0, KJS::DontDelete|KJS::ReadOnly, &PenNS::callisSolid},
    {"joinStyle", 0, KJS::DontDelete|KJS::ReadOnly, &PenNS::calljoinStyle},
    {"setBrush", 1, KJS::DontDelete|KJS::ReadOnly, &PenNS::callsetBrush},
    {"setCapStyle", 1, KJS::DontDelete|KJS::ReadOnly, &PenNS::callsetCapStyle},
    {"setColor", 1, KJS::DontDelete|KJS::ReadOnly, &PenNS::callsetColor},
    {"setJoinStyle", 1, KJS::DontDelete|KJS::ReadOnly, &PenNS::callsetJoinStyle},
    {"setStyle", 1, KJS::DontDelete|KJS::ReadOnly, &PenNS::callsetStyle},
    {"setWidth", 1, KJS::DontDelete|KJS::ReadOnly, &PenNS::callsetWidth},
    {"style", 0, KJS::DontDelete|KJS::ReadOnly, &PenNS::callstyle},
    {"width", 0, KJS::DontDelete|KJS::ReadOnly, &PenNS::callwidth}
END_METHOD_LUT


NO_ENUMS( Pen )
NO_STATICS( Pen )

START_CTOR( Pen, QPen, 0)
    if( args.size() == 1 )
    {
        return new KJSEmbed::PenBinding(exec,
                                    QPen( KJSEmbed::extractVariant<QColor>( exec, args, 0 )
                                            ) );
    }
    else if (args.size() >= 2)
    {
        return new KJSEmbed::PenBinding(exec,
                                QPen( KJSEmbed::extractVariant<QBrush>( exec, args, 0 ),
                                        KJSEmbed::extractInt( exec, args, 1 ),
                                        (Qt::PenStyle)KJSEmbed::extractInt( exec, args, 2, Qt::SolidLine ),
                                        (Qt::PenCapStyle)KJSEmbed::extractInt( exec, args, 3, Qt::SquareCap ),
                                        (Qt::PenJoinStyle)KJSEmbed::extractInt( exec, args, 4, Qt::BevelJoin )
                                        ) );

    }

    return new KJSEmbed::PenBinding( exec, QPen() );
END_CTOR

//kate: indent-spaces on; indent-width 4; replace-tabs on; indent-mode cstyle;
