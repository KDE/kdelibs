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
#include "image.h"
#include "variant_binding.h"
#include "object_binding.h"

#include <QtCore/QDebug>
#include <QImage>
#include <QtCore/QPoint>
#include <QtCore/QSize>

using namespace KJSEmbed;

const KJS::ClassInfo ImageBinding::info = { "QImage", &VariantBinding::info, 0, 0 };
ImageBinding::ImageBinding( KJS::ExecState *exec, const QImage &value )
    : VariantBinding(exec, value )
{
    StaticBinding::publish( exec, this, VariantFactory::methods() );
    StaticBinding::publish( exec, this, Image::methods() );
}

namespace ImageNS
{
START_VARIANT_METHOD( callallGray, QImage )
    bool cppValue = value.allGray();
    result = KJS::jsBoolean(cppValue);
END_VARIANT_METHOD

START_VARIANT_METHOD( callalphaChannel, QImage )
    QImage cppValue = value.alphaChannel();
    result = KJSEmbed::createVariant(exec, "QImage", cppValue );
END_VARIANT_METHOD

START_VARIANT_METHOD( callcopy, QImage )
    int arg0 = KJSEmbed::extractInt(exec, args, 0);
    int arg1 = KJSEmbed::extractInt(exec, args, 1);
    int arg2 = KJSEmbed::extractInt(exec, args, 2);
    int arg3 = KJSEmbed::extractInt(exec, args, 3);
    QImage cppValue = value.copy(arg0, arg1, arg2, arg3);
    result = KJSEmbed::createVariant(exec, "QImage", cppValue );
END_VARIANT_METHOD

START_VARIANT_METHOD( callcreateAlphaMask, QImage )
    Qt::ImageConversionFlags arg0 = (Qt::ImageConversionFlags)KJSEmbed::extractInt(exec, args, 0);
    QImage cppValue = value.createAlphaMask(arg0);
    result = KJSEmbed::createVariant(exec, "QImage", cppValue );
END_VARIANT_METHOD

START_VARIANT_METHOD( callcreateHeuristicMask, QImage )
    bool arg0 = KJSEmbed::extractBool(exec, args, 0);
    QImage cppValue = value.createHeuristicMask(arg0);
    result = KJSEmbed::createVariant(exec, "QImage", cppValue );
END_VARIANT_METHOD

START_VARIANT_METHOD( calldepth, QImage )
    int cppValue = value.depth();
    result = KJS::jsNumber(cppValue);
END_VARIANT_METHOD

START_VARIANT_METHOD( calldevType, QImage )
    int cppValue = value.devType();
    result = KJS::jsNumber(cppValue);
END_VARIANT_METHOD

START_VARIANT_METHOD( calldotsPerMeterX, QImage )
    int cppValue = value.dotsPerMeterX();
    result = KJS::jsNumber(cppValue);
END_VARIANT_METHOD

START_VARIANT_METHOD( calldotsPerMeterY, QImage )
    int cppValue = value.dotsPerMeterY();
    result = KJS::jsNumber(cppValue);
END_VARIANT_METHOD

START_VARIANT_METHOD( callfill, QImage )
    uint arg0 = KJSEmbed::extractVariant<uint>(exec,args, 0);
    value.fill(arg0);
END_VARIANT_METHOD

START_VARIANT_METHOD( callhasAlphaChannel, QImage )
    bool cppValue = value.hasAlphaChannel();
    result = KJS::jsBoolean(cppValue);
END_VARIANT_METHOD

START_VARIANT_METHOD( callheight, QImage )
    int cppValue = value.height();
    result = KJS::jsNumber(cppValue);
END_VARIANT_METHOD

START_VARIANT_METHOD( callinvertPixels, QImage )
    QImage::InvertMode arg0 = (QImage::InvertMode)KJSEmbed::extractInt(exec, args, 0);
    value.invertPixels(arg0);
END_VARIANT_METHOD

START_VARIANT_METHOD( callisGrayscale, QImage )
    bool cppValue = value.isGrayscale();
    result = KJS::jsBoolean(cppValue);
END_VARIANT_METHOD

START_VARIANT_METHOD( callisNull, QImage )
    bool cppValue = value.isNull();
    result = KJS::jsBoolean(cppValue);
END_VARIANT_METHOD

START_VARIANT_METHOD( callload, QImage )
    QString arg0 = KJSEmbed::extractQString(exec, args, 0);
    QByteArray arg1 = KJSEmbed::extractQString( exec, args, 1 ).toLatin1();
    bool cppValue = value.load(arg0, arg1);
    result = KJS::jsBoolean(cppValue);
END_VARIANT_METHOD

START_VARIANT_METHOD( callmirrored, QImage )
    bool arg0 = KJSEmbed::extractBool(exec, args, 0);
    bool arg1 = KJSEmbed::extractBool(exec, args, 1);
    QImage cppValue = value.mirrored(arg0, arg1);
    result = KJSEmbed::createVariant(exec, "QImage", cppValue );
END_VARIANT_METHOD

START_VARIANT_METHOD( callnumBytes, QImage )
    int cppValue = value.byteCount();
    result = KJS::jsNumber(cppValue);
END_VARIANT_METHOD

START_VARIANT_METHOD( callnumColors, QImage )
    int cppValue = value.colorCount();
    result = KJS::jsNumber(cppValue);
END_VARIANT_METHOD

START_VARIANT_METHOD( calloffset, QImage )
    QPoint cppValue = value.offset();
    result = KJSEmbed::createVariant(exec, "QPoint", cppValue );
END_VARIANT_METHOD

START_VARIANT_METHOD( callpixel, QImage )
    int arg0 = KJSEmbed::extractInt(exec, args, 0);
    int arg1 = KJSEmbed::extractInt(exec, args, 1);
    QRgb cppValue = value.pixel(arg0, arg1);
    result = KJS::jsNumber(cppValue);
END_VARIANT_METHOD

START_VARIANT_METHOD( callpixelIndex, QImage )
    int arg0 = KJSEmbed::extractInt(exec, args, 0);
    int arg1 = KJSEmbed::extractInt(exec, args, 1);
    int cppValue = value.pixelIndex(arg0, arg1);
    result = KJS::jsNumber(cppValue);
END_VARIANT_METHOD

START_VARIANT_METHOD( callrect, QImage )
    QRect cppValue = value.rect();
    result = KJSEmbed::createVariant(exec, "QRect", cppValue );
END_VARIANT_METHOD

START_VARIANT_METHOD( callrgbSwapped, QImage )
    QImage cppValue = value.rgbSwapped();
    result = KJSEmbed::createVariant(exec, "QImage", cppValue );
END_VARIANT_METHOD

START_VARIANT_METHOD( callsave, QImage )
    QString arg0 = KJSEmbed::extractQString(exec, args, 0);
    QByteArray arg1 = KJSEmbed::extractQString( exec, args, 1 ).toLatin1();
    int arg2 = KJSEmbed::extractInt(exec, args, 2);
    bool cppValue = value.save(arg0, arg1, arg2);
    result = KJS::jsBoolean(cppValue);
END_VARIANT_METHOD

START_VARIANT_METHOD( callscaled, QImage )
    if( args.size() == 3)
    {
        QSize arg0 = KJSEmbed::extractVariant<QSize>(exec,args, 0);
        Qt::AspectRatioMode arg1 = (Qt::AspectRatioMode)KJSEmbed::extractInt(exec, args, 1);
        Qt::TransformationMode arg2 = (Qt::TransformationMode)KJSEmbed::extractInt(exec, args, 2);
        QImage cppValue = value.scaled(arg0, arg1, arg2);
        result = KJSEmbed::createVariant(exec, "QImage", cppValue );
    }
    else if( args.size() == 4)
    {
        int arg0 = KJSEmbed::extractInt(exec, args, 0);
        int arg1 = KJSEmbed::extractInt(exec, args, 1);
        Qt::AspectRatioMode arg2 = (Qt::AspectRatioMode)KJSEmbed::extractInt(exec, args, 2);
        Qt::TransformationMode arg3 = (Qt::TransformationMode)KJSEmbed::extractInt(exec, args, 3);
        QImage cppValue = value.scaled(arg0, arg1, arg2, arg3);
        result = KJSEmbed::createVariant(exec, "QImage", cppValue );
    }
END_VARIANT_METHOD

START_VARIANT_METHOD( callscaledToHeight, QImage )
    int arg0 = KJSEmbed::extractInt(exec, args, 0);
    Qt::TransformationMode arg1 = (Qt::TransformationMode)KJSEmbed::extractInt(exec, args, 1);
    QImage cppValue = value.scaledToHeight(arg0, arg1);
    result = KJSEmbed::createVariant(exec, "QImage", cppValue );
END_VARIANT_METHOD

START_VARIANT_METHOD( callscaledToWidth, QImage )
    int arg0 = KJSEmbed::extractInt(exec, args, 0);
    Qt::TransformationMode arg1 = (Qt::TransformationMode)KJSEmbed::extractInt(exec, args, 1);
    QImage cppValue = value.scaledToWidth(arg0, arg1);
    result = KJSEmbed::createVariant(exec, "QImage", cppValue );
END_VARIANT_METHOD

START_VARIANT_METHOD( callserialNumber, QImage )
    int cppValue = value.serialNumber();
    result = KJS::jsNumber(cppValue);
END_VARIANT_METHOD

START_VARIANT_METHOD( callsetAlphaChannel, QImage )
    QImage arg0 = KJSEmbed::extractVariant<QImage>(exec,args, 0);
    value.setAlphaChannel(arg0);
END_VARIANT_METHOD

START_VARIANT_METHOD( callsetColor, QImage )
    int arg0 = KJSEmbed::extractInt(exec, args, 0);
    QRgb arg1 = (QRgb)KJSEmbed::extractInt(exec, args, 1);
    value.setColor(arg0, arg1);
END_VARIANT_METHOD

START_VARIANT_METHOD( callsetDotsPerMeterX, QImage )
    int arg0 = KJSEmbed::extractInt(exec, args, 0);
    value.setDotsPerMeterX(arg0);
END_VARIANT_METHOD

START_VARIANT_METHOD( callsetDotsPerMeterY, QImage )
    int arg0 = KJSEmbed::extractInt(exec, args, 0);
    value.setDotsPerMeterY(arg0);
END_VARIANT_METHOD

START_VARIANT_METHOD( callsetNumColors, QImage )
    int arg0 = KJSEmbed::extractInt(exec, args, 0);
    value.setColorCount(arg0);
END_VARIANT_METHOD

START_VARIANT_METHOD( callsetOffset, QImage )
    QPoint arg0 = KJSEmbed::extractVariant<QPoint>(exec,args, 0);
    value.setOffset(arg0);
END_VARIANT_METHOD

START_VARIANT_METHOD( callsetPixel, QImage )
    int arg0 = KJSEmbed::extractInt(exec, args, 0);
    int arg1 = KJSEmbed::extractInt(exec, args, 1);
    uint arg2 = KJSEmbed::extractVariant<uint>(exec,args, 2);
    value.setPixel(arg0, arg1, arg2);
END_VARIANT_METHOD

START_VARIANT_METHOD( callsize, QImage )
    QSize cppValue = value.size();
    result = KJSEmbed::createVariant(exec, "QSize", cppValue );
END_VARIANT_METHOD

START_VARIANT_METHOD( callvalid, QImage )
    int arg0 = KJSEmbed::extractInt(exec, args, 0);
    int arg1 = KJSEmbed::extractInt(exec, args, 1);
    bool cppValue = value.valid(arg0, arg1);
    result = KJS::jsBoolean(cppValue);
END_VARIANT_METHOD

START_VARIANT_METHOD( callwidth, QImage )
    int cppValue = value.width();
    result = KJS::jsNumber(cppValue);
END_VARIANT_METHOD

}

START_METHOD_LUT( Image )
    {"allGray", 0, KJS::DontDelete|KJS::ReadOnly, &ImageNS::callallGray},
    {"alphaChannel", 0, KJS::DontDelete|KJS::ReadOnly, &ImageNS::callalphaChannel},
    {"copy", 4, KJS::DontDelete|KJS::ReadOnly, &ImageNS::callcopy},
    {"createAlphaMask", 0, KJS::DontDelete|KJS::ReadOnly, &ImageNS::callcreateAlphaMask},
    {"createHeuristicMask", 0, KJS::DontDelete|KJS::ReadOnly, &ImageNS::callcreateHeuristicMask},
    {"depth", 0, KJS::DontDelete|KJS::ReadOnly, &ImageNS::calldepth},
    {"dotsPerMeterX", 0, KJS::DontDelete|KJS::ReadOnly, &ImageNS::calldotsPerMeterX},
    {"dotsPerMeterY", 0, KJS::DontDelete|KJS::ReadOnly, &ImageNS::calldotsPerMeterY},
    {"fill", 0, KJS::DontDelete|KJS::ReadOnly, &ImageNS::callfill},
    {"hasAlphaChannel", 0, KJS::DontDelete|KJS::ReadOnly, &ImageNS::callhasAlphaChannel},
    {"height", 0, KJS::DontDelete|KJS::ReadOnly, &ImageNS::callheight},
    {"invertPixels", 0, KJS::DontDelete|KJS::ReadOnly, &ImageNS::callinvertPixels},
    {"isGrayscale", 0, KJS::DontDelete|KJS::ReadOnly, &ImageNS::callisGrayscale},
    {"isNull", 0, KJS::DontDelete|KJS::ReadOnly, &ImageNS::callisNull},
    {"load", 1, KJS::DontDelete|KJS::ReadOnly, &ImageNS::callload},
    {"mirrored", 1, KJS::DontDelete|KJS::ReadOnly, &ImageNS::callmirrored},
    {"numBytes", 0, KJS::DontDelete|KJS::ReadOnly, &ImageNS::callnumBytes},
    {"numColors", 0, KJS::DontDelete|KJS::ReadOnly, &ImageNS::callnumColors},
    {"offset", 0, KJS::DontDelete|KJS::ReadOnly, &ImageNS::calloffset},
    {"pixel", 1, KJS::DontDelete|KJS::ReadOnly, &ImageNS::callpixel},
    {"pixelIndex", 1, KJS::DontDelete|KJS::ReadOnly, &ImageNS::callpixelIndex},
    {"rect", 0, KJS::DontDelete|KJS::ReadOnly, &ImageNS::callrect},
    {"rgbSwapped", 0, KJS::DontDelete|KJS::ReadOnly, &ImageNS::callrgbSwapped},
    {"save", 2, KJS::DontDelete|KJS::ReadOnly, &ImageNS::callsave},
    {"scaled", 3, KJS::DontDelete|KJS::ReadOnly, &ImageNS::callscaled},
    {"scaledToHeight", 1, KJS::DontDelete|KJS::ReadOnly, &ImageNS::callscaledToHeight},
    {"scaledToWidth", 1, KJS::DontDelete|KJS::ReadOnly, &ImageNS::callscaledToWidth},
    {"serialNumber", 0, KJS::DontDelete|KJS::ReadOnly, &ImageNS::callserialNumber},
    {"setAlphaChannel", 0, KJS::DontDelete|KJS::ReadOnly, &ImageNS::callsetAlphaChannel},
    {"setColor", 1, KJS::DontDelete|KJS::ReadOnly, &ImageNS::callsetColor},
    {"setDotsPerMeterX", 0, KJS::DontDelete|KJS::ReadOnly, &ImageNS::callsetDotsPerMeterX},
    {"setDotsPerMeterY", 0, KJS::DontDelete|KJS::ReadOnly, &ImageNS::callsetDotsPerMeterY},
    {"setNumColors", 0, KJS::DontDelete|KJS::ReadOnly, &ImageNS::callsetNumColors},
    {"setOffset", 0, KJS::DontDelete|KJS::ReadOnly, &ImageNS::callsetOffset},
    {"setPixel", 2, KJS::DontDelete|KJS::ReadOnly, &ImageNS::callsetPixel},
    {"size", 0, KJS::DontDelete|KJS::ReadOnly, &ImageNS::callsize},
    {"valid", 1, KJS::DontDelete|KJS::ReadOnly, &ImageNS::callvalid},
    {"width", 0, KJS::DontDelete|KJS::ReadOnly, &ImageNS::callwidth}
END_METHOD_LUT

NO_ENUMS( Image )
NO_STATICS( Image )

START_CTOR( Image, QImage, 0)
    if( args.size() == 0 )
    {
        return new KJSEmbed::ImageBinding( exec, QImage() );
    }
    if( args.size() == 1 )
    {
        return new KJSEmbed::ImageBinding( exec, QImage( KJSEmbed::extractQString(exec, args, 0 ) ) );
    }
    else if( args.size() == 2 )
    {
        return new KJSEmbed::ImageBinding( exec,
                QImage( KJSEmbed::extractVariant<QSize>(exec,args, 0 ),
                        (QImage::Format)KJSEmbed::extractInt( exec, args, 1 )) );
    }
    else if( args.size() == 3 )
    {
        return new KJSEmbed::ImageBinding(exec,
                QImage( KJSEmbed::extractInt( exec, args, 0 ),
                        KJSEmbed::extractInt( exec, args, 1 ),
                        (QImage::Format)KJSEmbed::extractInt( exec, args, 2 )) );
    }
    return new KJSEmbed::ImageBinding( exec, QImage() );
END_CTOR

//kate: indent-spaces on; indent-width 4; replace-tabs on; indent-mode cstyle;
