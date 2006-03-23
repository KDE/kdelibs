/* This file is part of the KDE libraries
    Copyright (C) 2005, 2006 KJSEmbed Authors
    See included AUTHORS file.

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
#include <QDebug>
#include <QBrush>
#include <QPixmap>
#include <QColor>
#include <QGradient>

#include "brush.h"

using namespace KJSEmbed;

BrushBinding::BrushBinding( KJS::ExecState *exec, const QBrush &value )
    : ValueBinding(exec, value )
{
    StaticBinding::publish( exec, this, Brush::methods() );
    StaticBinding::publish( exec, this, ValueFactory::methods() );
}

namespace BrushNS
{

START_VARIANT_METHOD( callcolor, QBrush )
    QColor cppValue = value.color();
    result = KJSEmbed::createValue(exec, "QColor", cppValue );
END_VARIANT_METHOD

START_VARIANT_METHOD( callgradient, QBrush )
    const QGradient *cppValue = value.gradient();
    result = KJSEmbed::createObject<QGradient>(exec, "QGradient", cppValue);
END_VARIANT_METHOD

START_VARIANT_METHOD( callisOpaque, QBrush )
    bool cppValue = value.isOpaque();
    result = KJS::Boolean(cppValue);
END_VARIANT_METHOD

START_VARIANT_METHOD( callsetColor, QBrush )
    QColor arg0 = KJSEmbed::extractValue<QColor>(exec,args, 0);
    value.setColor(arg0);
END_VARIANT_METHOD

START_VARIANT_METHOD( callsetStyle, QBrush )
    Qt::BrushStyle arg0 = (Qt::BrushStyle)KJSEmbed::extractInt(exec, args, 0);
    value.setStyle(arg0);
END_VARIANT_METHOD

START_VARIANT_METHOD( callsetTexture, QBrush )
    QPixmap arg0 = KJSEmbed::extractValue<QPixmap>(exec,args, 0);
    value.setTexture(arg0);
END_VARIANT_METHOD

START_VARIANT_METHOD( callstyle, QBrush )
    Qt::BrushStyle cppValue = value.style();
    result = KJS::Number(cppValue);
END_VARIANT_METHOD

START_VARIANT_METHOD( calltexture, QBrush )
    QPixmap cppValue = value.texture();
    result = KJSEmbed::createValue(exec, "QPixmap", cppValue );
END_VARIANT_METHOD

}

START_METHOD_LUT( Brush )
    {"color", 0, KJS::DontDelete|KJS::ReadOnly, &BrushNS::callcolor},
    {"gradient", 0, KJS::DontDelete|KJS::ReadOnly, &BrushNS::callgradient},
    {"isOpaque", 0, KJS::DontDelete|KJS::ReadOnly, &BrushNS::callisOpaque},
    {"setColor", 1, KJS::DontDelete|KJS::ReadOnly, &BrushNS::callsetColor},
    {"setStyle", 1, KJS::DontDelete|KJS::ReadOnly, &BrushNS::callsetStyle},
    {"setTexture", 1, KJS::DontDelete|KJS::ReadOnly, &BrushNS::callsetTexture},
    {"style", 0, KJS::DontDelete|KJS::ReadOnly, &BrushNS::callstyle},
    {"texture", 0, KJS::DontDelete|KJS::ReadOnly, &BrushNS::calltexture}
END_METHOD_LUT

NO_ENUMS( Brush )
NO_STATICS( Brush )

START_CTOR( Brush, QBrush, 0)
    if( args.size() == 0 )
    {
        return new KJSEmbed::BrushBinding( exec, QBrush() );
    }
    else if( args.size() == 1 )
    {
        return new KJSEmbed::BrushBinding( exec,
                                    QBrush(KJSEmbed::extractValue<QPixmap>(exec, args, 0)
                                            ) );
    }
    else if( args.size() == 2 )
    {
        return new KJSEmbed::BrushBinding(exec,
                                    QBrush( KJSEmbed::extractValue<QColor>(exec, args, 0),
                                            (Qt::BrushStyle)KJSEmbed::extractInt( exec, args, 1 )
                                            ) );
    }
    return new KJSEmbed::BrushBinding( exec, QBrush() );

END_CTOR
