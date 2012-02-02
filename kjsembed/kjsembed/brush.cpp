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
#include "brush.h"

#include <QtCore/QDebug>
#include <QBrush>
#include <QPixmap>
#include <QColor>

#include "pixmap.h"
#include "color.h"
#include "util.h"

using namespace KJSEmbed;

const KJS::ClassInfo BrushBinding::info = { "QBrush", &VariantBinding::info, 0, 0 };
BrushBinding::BrushBinding( KJS::ExecState *exec, const QBrush &value )
    : VariantBinding(exec, value )
{
    StaticBinding::publish( exec, this, Brush::methods() );
    StaticBinding::publish( exec, this, VariantFactory::methods() );
}

namespace BrushNS
{

START_VARIANT_METHOD( callcolor, QBrush )
    QColor cppValue = value.color();
    result = KJSEmbed::createVariant(exec, "QColor", cppValue );
END_VARIANT_METHOD

START_VARIANT_METHOD( callgradient, QBrush )
    const QGradient *cppValue = value.gradient();
    result = KJSEmbed::createObject<QGradient>(exec, "QGradient", cppValue);
END_VARIANT_METHOD

START_VARIANT_METHOD( callisOpaque, QBrush )
    bool cppValue = value.isOpaque();
    result = KJS::jsBoolean(cppValue);
END_VARIANT_METHOD

START_VARIANT_METHOD( callsetColor, QBrush )
    QColor arg0 = KJSEmbed::extractVariant<QColor>(exec,args, 0);
    value.setColor(arg0);
END_VARIANT_METHOD

START_VARIANT_METHOD( callsetStyle, QBrush )
    Qt::BrushStyle arg0 = (Qt::BrushStyle)KJSEmbed::extractInt(exec, args, 0);
    value.setStyle(arg0);
END_VARIANT_METHOD

START_VARIANT_METHOD( callsetTexture, QBrush )
    QPixmap arg0 = KJSEmbed::extractVariant<QPixmap>(exec,args, 0);
    value.setTexture(arg0);
END_VARIANT_METHOD

START_VARIANT_METHOD( callstyle, QBrush )
    Qt::BrushStyle cppValue = value.style();
    result = KJS::jsNumber(cppValue);
END_VARIANT_METHOD

START_VARIANT_METHOD( calltexture, QBrush )
    QPixmap cppValue = value.texture();
    result = KJSEmbed::createVariant(exec, "QPixmap", cppValue );
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
        KJS::JSValue* value0 = args[0];
        KJS::JSObject* obj0 = value0->toObject(exec);
        if(obj0)
        {
            if(obj0->inherits(&PixmapBinding::info))
            {
                QPixmap arg0 = KJSEmbed::extractVariant<QPixmap>(exec, args, 0);
                return new KJSEmbed::BrushBinding(exec, QBrush(arg0));
            }
            if(obj0->inherits(&BrushBinding::info))
            {
                QBrush arg0 = KJSEmbed::extractVariant<QBrush>(exec, args, 0);
                return new KJSEmbed::BrushBinding(exec, QBrush(arg0));
            }
//             if(obj0->inherits(&GradientBinding::info))
//             {
//                 QGradient arg0 = KJSEmbed::extractVariant<QGradient>(exec, args, 0);
//                 return new KJSEmbed::BrushBinding(exec, QBrush(arg0));
//             }
        }
        else if(isBasic(value0))
        {
                Qt::BrushStyle arg0 = (Qt::BrushStyle)KJSEmbed::extractInt(exec, args, 0);
                return new KJSEmbed::BrushBinding(exec, QBrush(arg0));
        }
    }
    else if( args.size() == 2 )
    {
        KJS::JSValue* value0= args[0];
        KJS::JSValue* value1= args[1];
        KJS::JSObject* obj0 = value0->toObject(exec);
        KJS::JSObject* obj1 = value1->toObject(exec);

        if(obj0 && obj1 && obj0->inherits(&ColorBinding::info) && obj1->inherits(&PixmapBinding::info))
        {
            QColor arg0 = KJSEmbed::extractVariant<QColor>(exec, args, 0);
            QPixmap arg1 = KJSEmbed::extractVariant<QPixmap>(exec, args, 1);
            return new KJSEmbed::BrushBinding(exec, QBrush(arg0, arg1));
        }
        if(obj1 && isBasic(value0) && obj1->inherits(&PixmapBinding::info))
        {
            Qt::GlobalColor arg0 = (Qt::GlobalColor)KJSEmbed::extractInt(exec, args, 0);
            QPixmap arg1 = KJSEmbed::extractVariant<QPixmap>(exec, args, 1);
            return new KJSEmbed::BrushBinding(exec, QBrush(arg0, arg1));
        }
        if(obj0 && obj0->inherits(&ColorBinding::info) && isBasic(value1))
        {
            QColor arg0 = KJSEmbed::extractVariant<QColor>(exec, args, 0);
            Qt::BrushStyle arg1 = (Qt::BrushStyle)KJSEmbed::extractInt(exec, args, 1);
            return new KJSEmbed::BrushBinding(exec, QBrush(arg0, arg1));
        }
        if(isBasic(value0) && isBasic(value1))
        {
            Qt::GlobalColor arg0  = (Qt::GlobalColor)KJSEmbed::extractInt(exec, args, 0);
            Qt::BrushStyle arg1 = (Qt::BrushStyle)KJSEmbed::extractInt(exec, args, 1);
            return new KJSEmbed::BrushBinding(exec, QBrush(arg0, arg1));
        }
    }
    return new KJSEmbed::BrushBinding( exec, QBrush() );

END_CTOR
//kate: indent-spaces on; indent-width 4; replace-tabs on; indent-mode cstyle;
