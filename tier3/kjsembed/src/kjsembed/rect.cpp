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
#include "rect.h"

#include <QtCore/QDebug>
#include <QtCore/QRect>
#include <QtCore/QPoint>
#include <QtCore/QSize>

using namespace KJSEmbed;

RectBinding::RectBinding( KJS::ExecState *exec, const QRect &value )
    : VariantBinding(exec, value )
{
    StaticBinding::publish( exec, this, Rect::methods() );
    StaticBinding::publish( exec, this, VariantFactory::methods() );
}

namespace RectNS
{

START_VARIANT_METHOD( calladjust, QRect )
    int arg0 = KJSEmbed::extractInt(exec, args, 0);
    int arg1 = KJSEmbed::extractInt(exec, args, 1);
    int arg2 = KJSEmbed::extractInt(exec, args, 2);
    int arg3 = KJSEmbed::extractInt(exec, args, 3);
    value.adjust(arg0, arg1, arg2, arg3);
END_VARIANT_METHOD

START_VARIANT_METHOD( calladjusted, QRect )
    int arg0 = KJSEmbed::extractInt(exec, args, 0);
    int arg1 = KJSEmbed::extractInt(exec, args, 1);
    int arg2 = KJSEmbed::extractInt(exec, args, 2);
    int arg3 = KJSEmbed::extractInt(exec, args, 3);
    QRect cppValue = value.adjusted(arg0, arg1, arg2, arg3);
    result = KJSEmbed::createVariant(exec, "QRect", cppValue );
END_VARIANT_METHOD

START_VARIANT_METHOD( callbottom, QRect )
    int cppValue = value.bottom();
    result = KJS::jsNumber(cppValue);
END_VARIANT_METHOD

START_VARIANT_METHOD( callbottomLeft, QRect )
    QPoint cppValue = value.bottomLeft();
    result = KJSEmbed::createVariant(exec, "QPoint", cppValue );
END_VARIANT_METHOD

START_VARIANT_METHOD( callbottomRight, QRect )
    QPoint cppValue = value.bottomRight();
    result = KJSEmbed::createVariant(exec, "QPoint", cppValue );
END_VARIANT_METHOD

START_VARIANT_METHOD( callcenter, QRect )
    QPoint cppValue = value.center();
    result = KJSEmbed::createVariant(exec, "QPoint", cppValue );
END_VARIANT_METHOD

START_VARIANT_METHOD( callcontains, QRect )
    // 1
    if( args.size() == 2 /* &&  !KJSEmbed::extractVariant<QPoint>(exec,args, 0).isNull() */)
    {
        QPoint arg0 = KJSEmbed::extractVariant<QPoint>(exec,args, 0);
        bool arg1 = KJSEmbed::extractBool(exec, args, 1);
        bool cppValue = value.contains(arg0, arg1);
        result = KJS::jsBoolean(cppValue);
    }
    //2
    else if( args.size() == 3 )
    {
        int arg0 = KJSEmbed::extractInt(exec, args, 0);
        int arg1 = KJSEmbed::extractInt(exec, args, 1);
        bool arg2 = KJSEmbed::extractBool(exec, args, 2);
        bool cppValue = value.contains(arg0, arg1, arg2);
        result = KJS::jsBoolean(cppValue);
    }
    // 3
    else
    {
        int arg0 = KJSEmbed::extractInt(exec, args, 0);
        int arg1 = KJSEmbed::extractInt(exec, args, 1);
        bool cppValue = value.contains(arg0, arg1);
        result = KJS::jsBoolean(cppValue);
    }
END_VARIANT_METHOD

START_VARIANT_METHOD( callheight, QRect )
    int cppValue = value.height();
    result = KJS::jsNumber(cppValue);
END_VARIANT_METHOD

START_VARIANT_METHOD( callintersect, QRect )
    QRect arg0 = KJSEmbed::extractVariant<QRect>(exec,args, 0);
    QRect cppValue = value.intersected(arg0);
    result = KJSEmbed::createVariant(exec, "QRect", cppValue );
END_VARIANT_METHOD

START_VARIANT_METHOD( callintersects, QRect )
    QRect arg0 = KJSEmbed::extractVariant<QRect>(exec,args, 0);
    bool cppValue = value.intersects(arg0);
    result = KJS::jsBoolean(cppValue);
END_VARIANT_METHOD

START_VARIANT_METHOD( callisEmpty, QRect )
    bool cppValue = value.isEmpty();
    result = KJS::jsBoolean(cppValue);
END_VARIANT_METHOD

START_VARIANT_METHOD( callisNull, QRect )
    bool cppValue = value.isNull();
    result = KJS::jsBoolean(cppValue);
END_VARIANT_METHOD

START_VARIANT_METHOD( callisValid, QRect )
    bool cppValue = value.isValid();
    result = KJS::jsBoolean(cppValue);
END_VARIANT_METHOD

START_VARIANT_METHOD( callleft, QRect )
    int cppValue = value.left();
    result = KJS::jsNumber(cppValue);
END_VARIANT_METHOD

START_VARIANT_METHOD( callmoveBottom, QRect )
    int arg0 = KJSEmbed::extractInt(exec, args, 0);
    value.moveBottom(arg0);
END_VARIANT_METHOD

START_VARIANT_METHOD( callmoveBottomLeft, QRect )
    QPoint arg0 = KJSEmbed::extractVariant<QPoint>(exec,args, 0);
    value.moveBottomLeft(arg0);
END_VARIANT_METHOD

START_VARIANT_METHOD( callmoveBottomRight, QRect )
    QPoint arg0 = KJSEmbed::extractVariant<QPoint>(exec,args, 0);
    value.moveBottomRight(arg0);
END_VARIANT_METHOD

START_VARIANT_METHOD( callmoveCenter, QRect )
    QPoint arg0 = KJSEmbed::extractVariant<QPoint>(exec,args, 0);
    value.moveCenter(arg0);
END_VARIANT_METHOD

START_VARIANT_METHOD( callmoveLeft, QRect )
    int arg0 = KJSEmbed::extractInt(exec, args, 0);
    value.moveLeft(arg0);
END_VARIANT_METHOD

START_VARIANT_METHOD( callmoveRight, QRect )
    int arg0 = KJSEmbed::extractInt(exec, args, 0);
    value.moveRight(arg0);
END_VARIANT_METHOD

START_VARIANT_METHOD( callmoveTo, QRect )
    if( args.size() == 1)
    {
        QPoint arg0 = KJSEmbed::extractVariant<QPoint>(exec,args, 0);
        value.moveTo(arg0);
    }
    else
    {
        int arg0 = KJSEmbed::extractInt(exec, args, 0);
        int arg1 = KJSEmbed::extractInt(exec, args, 1);
        value.moveTo(arg0, arg1);
    }
END_VARIANT_METHOD

START_VARIANT_METHOD( callmoveTop, QRect )
    int arg0 = KJSEmbed::extractInt(exec, args, 0);
    value.moveTop(arg0);
END_VARIANT_METHOD

START_VARIANT_METHOD( callmoveTopLeft, QRect )
    QPoint arg0 = KJSEmbed::extractVariant<QPoint>(exec,args, 0);
    value.moveTopLeft(arg0);
END_VARIANT_METHOD

START_VARIANT_METHOD( callmoveTopRight, QRect )
    QPoint arg0 = KJSEmbed::extractVariant<QPoint>(exec,args, 0);
    value.moveTopRight(arg0);
END_VARIANT_METHOD

START_VARIANT_METHOD( callnormalized, QRect )
    QRect cppValue = value.normalized();
    result = KJSEmbed::createVariant(exec, "QRect", cppValue );
END_VARIANT_METHOD

START_VARIANT_METHOD( callright, QRect )
    int cppValue = value.right();
    result = KJS::jsNumber(cppValue);
END_VARIANT_METHOD

START_VARIANT_METHOD( callsetBottom, QRect )
    int arg0 = KJSEmbed::extractInt(exec, args, 0);
    value.setBottom(arg0);
END_VARIANT_METHOD

START_VARIANT_METHOD( callsetBottomLeft, QRect )
    QPoint arg0 = KJSEmbed::extractVariant<QPoint>(exec,args, 0);
    value.setBottomLeft(arg0);
END_VARIANT_METHOD

START_VARIANT_METHOD( callsetBottomRight, QRect )
    QPoint arg0 = KJSEmbed::extractVariant<QPoint>(exec,args, 0);
    value.setBottomRight(arg0);
END_VARIANT_METHOD

START_VARIANT_METHOD( callsetCoords, QRect )
    int arg0 = KJSEmbed::extractInt(exec, args, 0);
    int arg1 = KJSEmbed::extractInt(exec, args, 1);
    int arg2 = KJSEmbed::extractInt(exec, args, 2);
    int arg3 = KJSEmbed::extractInt(exec, args, 3);
    value.setCoords(arg0, arg1, arg2, arg3);
END_VARIANT_METHOD

START_VARIANT_METHOD( callsetHeight, QRect )
    int arg0 = KJSEmbed::extractInt(exec, args, 0);
    value.setHeight(arg0);
END_VARIANT_METHOD

START_VARIANT_METHOD( callsetLeft, QRect )
        int arg0 = KJSEmbed::extractInt(exec, args, 0);
        value.setLeft(arg0);
END_VARIANT_METHOD

START_VARIANT_METHOD( callsetRect, QRect )
    int arg0 = KJSEmbed::extractInt(exec, args, 0);
    int arg1 = KJSEmbed::extractInt(exec, args, 1);
    int arg2 = KJSEmbed::extractInt(exec, args, 2);
    int arg3 = KJSEmbed::extractInt(exec, args, 3);
    value.setRect(arg0, arg1, arg2, arg3);
END_VARIANT_METHOD

START_VARIANT_METHOD( callsetRight, QRect )
    int arg0 = KJSEmbed::extractInt(exec, args, 0);
    value.setRight(arg0);
END_VARIANT_METHOD

START_VARIANT_METHOD( callsetSize, QRect )
    QSize arg0 = KJSEmbed::extractVariant<QSize>(exec,args, 0);
    value.setSize(arg0);
END_VARIANT_METHOD

START_VARIANT_METHOD( callsetTop, QRect )
    int arg0 = KJSEmbed::extractInt(exec, args, 0);
    value.setTop(arg0);
END_VARIANT_METHOD

START_VARIANT_METHOD( callsetTopLeft, QRect )
    QPoint arg0 = KJSEmbed::extractVariant<QPoint>(exec,args, 0);
    value.setTopLeft(arg0);
END_VARIANT_METHOD

START_VARIANT_METHOD( callsetTopRight, QRect )
    QPoint arg0 = KJSEmbed::extractVariant<QPoint>(exec,args, 0);
    value.setTopRight(arg0);
END_VARIANT_METHOD

START_VARIANT_METHOD( callsetWidth, QRect )
    int arg0 = KJSEmbed::extractInt(exec, args, 0);
    value.setWidth(arg0);
END_VARIANT_METHOD

START_VARIANT_METHOD( callsetX, QRect )
    int arg0 = KJSEmbed::extractInt(exec, args, 0);
    value.setX(arg0);
END_VARIANT_METHOD

START_VARIANT_METHOD( callsetY, QRect )
    int arg0 = KJSEmbed::extractInt(exec, args, 0);
    value.setY(arg0);
END_VARIANT_METHOD

START_VARIANT_METHOD( callsize, QRect )
    QSize cppValue = value.size();
    result = KJSEmbed::createVariant(exec, "QSize", cppValue );
END_VARIANT_METHOD

START_VARIANT_METHOD( calltop, QRect )
    int cppValue = value.top();
    result = KJS::jsNumber(cppValue);
END_VARIANT_METHOD

START_VARIANT_METHOD( calltopLeft, QRect )
    QPoint cppValue = value.topLeft();
    result = KJSEmbed::createVariant(exec, "QPoint", cppValue );
END_VARIANT_METHOD

START_VARIANT_METHOD( calltopRight, QRect )
    QPoint cppValue = value.topRight();
    result = KJSEmbed::createVariant(exec, "QPoint", cppValue );
END_VARIANT_METHOD

START_VARIANT_METHOD( calltranslate, QRect )
    if( args.size() == 1)
    {
        QPoint arg0 = KJSEmbed::extractVariant<QPoint>(exec,args, 0);
        value.translate(arg0);
    }
    else
    {
        int arg0 = KJSEmbed::extractInt(exec, args, 0);
        int arg1 = KJSEmbed::extractInt(exec, args, 1);
        value.translate(arg0, arg1);
    }
END_VARIANT_METHOD

START_VARIANT_METHOD( calltranslated, QRect )
    if( args.size() == 1 )
    {
        QPoint arg0 = KJSEmbed::extractVariant<QPoint>(exec,args, 0);
        QRect cppValue = value.translated(arg0);
        result = KJSEmbed::createVariant(exec, "QRect", cppValue );
    }
    else
    {
        int arg0 = KJSEmbed::extractInt(exec, args, 0);
        int arg1 = KJSEmbed::extractInt(exec, args, 1);
        QRect cppValue = value.translated(arg0, arg1);
        result = KJSEmbed::createVariant(exec, "QRect", cppValue );
    }
END_VARIANT_METHOD

START_VARIANT_METHOD( callunite, QRect )
    QRect arg0 = KJSEmbed::extractVariant<QRect>(exec,args, 0);
    QRect cppValue = value.united(arg0);
    result = KJSEmbed::createVariant(exec, "QRect", cppValue );
END_VARIANT_METHOD

START_VARIANT_METHOD( callwidth, QRect )
    int cppValue = value.width();
    result = KJS::jsNumber(cppValue);
END_VARIANT_METHOD

START_VARIANT_METHOD( callx, QRect )
    int cppValue = value.x();
    result = KJS::jsNumber(cppValue);
END_VARIANT_METHOD

START_VARIANT_METHOD( cally, QRect )
    int cppValue = value.y();
    result = KJS::jsNumber(cppValue);
END_VARIANT_METHOD
}

START_METHOD_LUT( Rect )
    {"adjust", 3, KJS::DontDelete|KJS::ReadOnly, &RectNS::calladjust},
    {"adjusted", 3, KJS::DontDelete|KJS::ReadOnly, &RectNS::calladjusted},
    {"bottom", 0, KJS::DontDelete|KJS::ReadOnly, &RectNS::callbottom},
    {"bottomLeft", 0, KJS::DontDelete|KJS::ReadOnly, &RectNS::callbottomLeft},
    {"bottomRight", 0, KJS::DontDelete|KJS::ReadOnly, &RectNS::callbottomRight},
    {"center", 0, KJS::DontDelete|KJS::ReadOnly, &RectNS::callcenter},
    {"contains", 2, KJS::DontDelete|KJS::ReadOnly, &RectNS::callcontains},
    {"height", 0, KJS::DontDelete|KJS::ReadOnly, &RectNS::callheight},
    {"intersect", 0, KJS::DontDelete|KJS::ReadOnly, &RectNS::callintersect},
    {"intersects", 0, KJS::DontDelete|KJS::ReadOnly, &RectNS::callintersects},
    {"isEmpty", 0, KJS::DontDelete|KJS::ReadOnly, &RectNS::callisEmpty},
    {"isNull", 0, KJS::DontDelete|KJS::ReadOnly, &RectNS::callisNull},
    {"isValid", 0, KJS::DontDelete|KJS::ReadOnly, &RectNS::callisValid},
    {"left", 0, KJS::DontDelete|KJS::ReadOnly, &RectNS::callleft},
    {"moveBottom", 0, KJS::DontDelete|KJS::ReadOnly, &RectNS::callmoveBottom},
    {"moveBottomLeft", 0, KJS::DontDelete|KJS::ReadOnly, &RectNS::callmoveBottomLeft},
    {"moveBottomRight", 0, KJS::DontDelete|KJS::ReadOnly, &RectNS::callmoveBottomRight},
    {"moveCenter", 0, KJS::DontDelete|KJS::ReadOnly, &RectNS::callmoveCenter},
    {"moveLeft", 0, KJS::DontDelete|KJS::ReadOnly, &RectNS::callmoveLeft},
    {"moveRight", 0, KJS::DontDelete|KJS::ReadOnly, &RectNS::callmoveRight},
    {"moveTo", 1, KJS::DontDelete|KJS::ReadOnly, &RectNS::callmoveTo},
    {"moveTop", 0, KJS::DontDelete|KJS::ReadOnly, &RectNS::callmoveTop},
    {"moveTopLeft", 0, KJS::DontDelete|KJS::ReadOnly, &RectNS::callmoveTopLeft},
    {"moveTopRight", 0, KJS::DontDelete|KJS::ReadOnly, &RectNS::callmoveTopRight},
    {"normalized", 0, KJS::DontDelete|KJS::ReadOnly, &RectNS::callnormalized},
    {"right", 0, KJS::DontDelete|KJS::ReadOnly, &RectNS::callright},
    {"setBottom", 0, KJS::DontDelete|KJS::ReadOnly, &RectNS::callsetBottom},
    {"setBottomLeft", 0, KJS::DontDelete|KJS::ReadOnly, &RectNS::callsetBottomLeft},
    {"setBottomRight", 0, KJS::DontDelete|KJS::ReadOnly, &RectNS::callsetBottomRight},
    {"setCoords", 3, KJS::DontDelete|KJS::ReadOnly, &RectNS::callsetCoords},
    {"setHeight", 0, KJS::DontDelete|KJS::ReadOnly, &RectNS::callsetHeight},
    {"setLeft", 0, KJS::DontDelete|KJS::ReadOnly, &RectNS::callsetLeft},
    {"setRect", 3, KJS::DontDelete|KJS::ReadOnly, &RectNS::callsetRect},
    {"setRight", 0, KJS::DontDelete|KJS::ReadOnly, &RectNS::callsetRight},
    {"setSize", 0, KJS::DontDelete|KJS::ReadOnly, &RectNS::callsetSize},
    {"setTop", 0, KJS::DontDelete|KJS::ReadOnly, &RectNS::callsetTop},
    {"setTopLeft", 0, KJS::DontDelete|KJS::ReadOnly, &RectNS::callsetTopLeft},
    {"setTopRight", 0, KJS::DontDelete|KJS::ReadOnly, &RectNS::callsetTopRight},
    {"setWidth", 0, KJS::DontDelete|KJS::ReadOnly, &RectNS::callsetWidth},
    {"setX", 0, KJS::DontDelete|KJS::ReadOnly, &RectNS::callsetX},
    {"setY", 0, KJS::DontDelete|KJS::ReadOnly, &RectNS::callsetY},
    {"size", 0, KJS::DontDelete|KJS::ReadOnly, &RectNS::callsize},
    {"top", 0, KJS::DontDelete|KJS::ReadOnly, &RectNS::calltop},
    {"topLeft", 0, KJS::DontDelete|KJS::ReadOnly, &RectNS::calltopLeft},
    {"topRight", 0, KJS::DontDelete|KJS::ReadOnly, &RectNS::calltopRight},
    {"translate", 1, KJS::DontDelete|KJS::ReadOnly, &RectNS::calltranslate},
    {"translated", 1, KJS::DontDelete|KJS::ReadOnly, &RectNS::calltranslated},
    {"unite", 0, KJS::DontDelete|KJS::ReadOnly, &RectNS::callunite},
    {"width", 0, KJS::DontDelete|KJS::ReadOnly, &RectNS::callwidth},
    {"x", 0, KJS::DontDelete|KJS::ReadOnly, &RectNS::callx},
    {"y", 0, KJS::DontDelete|KJS::ReadOnly, &RectNS::cally}
END_METHOD_LUT

NO_ENUMS( Rect )
NO_STATICS( Rect )

START_CTOR( Rect, QRect, 0)
    if( args.size() == 2 )
    {
    /*
                    return KJSEmbed::RectBinding( exec,
                            QRect( KJSEmbed::extractQString(exec,args,0 ) ) );
    */
    }
    else if( args.size() == 4 )
    {
        return new KJSEmbed::RectBinding(exec,
                                    QRect( KJSEmbed::extractInt( exec, args, 0 ),
                                            KJSEmbed::extractInt( exec, args, 1 ),
                                            KJSEmbed::extractInt( exec, args, 2 ),
                                            KJSEmbed::extractInt( exec, args, 3 )
                                        ) );
    }

    return new KJSEmbed::RectBinding( exec, QRect() );
END_CTOR

//kate: indent-spaces on; indent-width 4; replace-tabs on; indent-mode cstyle;
