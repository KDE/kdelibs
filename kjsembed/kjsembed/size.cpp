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
#include "size.h"

#include <QtCore/QDebug>
#include <QtCore/QSize>

using namespace KJSEmbed;

const KJS::ClassInfo SizeBinding::info = { "QSize", &VariantBinding::info, 0, 0 };
SizeBinding::SizeBinding( KJS::ExecState *exec, const QSize &value )
    : VariantBinding(exec, value )
{
    StaticBinding::publish( exec, this, Size::methods() );
    StaticBinding::publish( exec, this, VariantFactory::methods() );
}

namespace SizeNS
{

START_VARIANT_METHOD( callboundedTo, QSize )
    QSize arg0 = KJSEmbed::extractVariant<QSize>(exec,args, 0);
    QSize cppValue = value.boundedTo(arg0);
    result = KJSEmbed::createVariant(exec, "QSize", cppValue );
END_VARIANT_METHOD

START_VARIANT_METHOD( callexpandedTo, QSize )
    QSize arg0 = KJSEmbed::extractVariant<QSize>(exec,args, 0);
    QSize cppValue = value.expandedTo(arg0);
    result = KJSEmbed::createVariant(exec, "QSize", cppValue );
END_VARIANT_METHOD

START_VARIANT_METHOD( callheight, QSize )
    int cppValue = value.height();
    result = KJS::jsNumber(cppValue);
END_VARIANT_METHOD

START_VARIANT_METHOD( callisEmpty, QSize )
    bool cppValue = value.isEmpty();
    result = KJS::jsBoolean(cppValue);
END_VARIANT_METHOD

START_VARIANT_METHOD( callisNull, QSize )
    bool cppValue = value.isNull();
    result = KJS::jsBoolean(cppValue);
END_VARIANT_METHOD

START_VARIANT_METHOD( callisValid, QSize )
    bool cppValue = value.isValid();
    result = KJS::jsBoolean(cppValue);
END_VARIANT_METHOD

START_VARIANT_METHOD( callrheight, QSize )
    int cppValue = value.rheight();
    result = KJS::jsNumber(cppValue);
END_VARIANT_METHOD

START_VARIANT_METHOD( callrwidth, QSize )
    int cppValue = value.rwidth();
    result = KJS::jsNumber(cppValue);
END_VARIANT_METHOD

START_VARIANT_METHOD( callscale, QSize )
    if( args.size() == 2 )
    {
        QSize arg0 = KJSEmbed::extractVariant<QSize>(exec,args, 0);
        Qt::AspectRatioMode arg1 = (Qt::AspectRatioMode)KJSEmbed::extractInt(exec, args, 1);
        value.scale(arg0, arg1);
    }
    else if( args.size() == 3 )
    {
        int arg0 = KJSEmbed::extractInt(exec, args, 0);
        int arg1 = KJSEmbed::extractInt(exec, args, 1);
        Qt::AspectRatioMode arg2 = (Qt::AspectRatioMode)KJSEmbed::extractInt(exec, args, 2);
        value.scale(arg0, arg1, arg2);
    }
END_VARIANT_METHOD

START_VARIANT_METHOD( callsetHeight, QSize )
    int arg0 = KJSEmbed::extractInt(exec, args, 0);
    value.setHeight(arg0);
END_VARIANT_METHOD

START_VARIANT_METHOD( callsetWidth, QSize )
    int arg0 = KJSEmbed::extractInt(exec, args, 0);
    value.setWidth(arg0);
END_VARIANT_METHOD

START_VARIANT_METHOD( calltranspose, QSize )
    value.transpose();
END_VARIANT_METHOD

START_VARIANT_METHOD( callwidth, QSize )
    int cppValue = value.width();
    result = KJS::jsNumber(cppValue);
END_VARIANT_METHOD

}

START_METHOD_LUT( Size )
    {"boundedTo", 0, KJS::DontDelete|KJS::ReadOnly, &SizeNS::callboundedTo},
    {"expandedTo", 0, KJS::DontDelete|KJS::ReadOnly, &SizeNS::callexpandedTo},
    {"height", 0, KJS::DontDelete|KJS::ReadOnly, &SizeNS::callheight},
    {"isEmpty", 0, KJS::DontDelete|KJS::ReadOnly, &SizeNS::callisEmpty},
    {"isNull", 0, KJS::DontDelete|KJS::ReadOnly, &SizeNS::callisNull},
    {"isValid", 0, KJS::DontDelete|KJS::ReadOnly, &SizeNS::callisValid},
    {"rheight", 0, KJS::DontDelete|KJS::ReadOnly, &SizeNS::callrheight},
    {"rwidth", 0, KJS::DontDelete|KJS::ReadOnly, &SizeNS::callrwidth},
    {"scale", 2, KJS::DontDelete|KJS::ReadOnly, &SizeNS::callscale},
    {"setHeight", 0, KJS::DontDelete|KJS::ReadOnly, &SizeNS::callsetHeight},
    {"setWidth", 0, KJS::DontDelete|KJS::ReadOnly, &SizeNS::callsetWidth},
    {"transpose", 0, KJS::DontDelete|KJS::ReadOnly, &SizeNS::calltranspose},
    {"width", 0, KJS::DontDelete|KJS::ReadOnly, &SizeNS::callwidth}
END_METHOD_LUT

NO_ENUMS( Size )
NO_STATICS( Size )

START_CTOR( Size, QSize, 0)
    if( args.size() == 0 )
    {
        return new KJSEmbed::SizeBinding( exec, QSize() );
    }
    else if( args.size() == 2 )
    {
        return new KJSEmbed::SizeBinding(exec,
                                QSize( KJSEmbed::extractInt( exec, args, 0 ),
                                        KJSEmbed::extractInt( exec, args, 1 )
                                        ) );
    }
    return new KJSEmbed::SizeBinding( exec, QSize() );
END_CTOR

//kate: indent-spaces on; indent-width 4; replace-tabs on; indent-mode cstyle;

