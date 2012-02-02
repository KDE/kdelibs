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
#include "pixmap.h"

#include <QtCore/QDebug>
#include <QPixmap>
#include <QBitmap>
#include <QWidget>

#include <variant_binding.h>
#include <object_binding.h>

using namespace KJSEmbed;

const KJS::ClassInfo PixmapBinding::info = { "QPixmap", &VariantBinding::info, 0, 0 };
PixmapBinding::PixmapBinding( KJS::ExecState *exec, const QPixmap &value )
    : VariantBinding(exec, value )
{
    StaticBinding::publish( exec, this, Pixmap::methods() );
    StaticBinding::publish( exec, this, VariantFactory::methods() );
}

namespace PixmapNS
{
START_VARIANT_METHOD( callalphaChannel, QPixmap )
    QPixmap cppValue = value.alphaChannel();
    result = KJSEmbed::createVariant(exec, "QPixmap", cppValue );
END_VARIANT_METHOD

START_VARIANT_METHOD( callcopy, QPixmap )
    if( args.size() == 1)
    {
        QRect arg0 = KJSEmbed::extractVariant<QRect>(exec,args, 0);
        QPixmap cppValue = value.copy(arg0);
        result = KJSEmbed::createVariant(exec, "QPixmap", cppValue );
    }
    else if( args.size() == 4)
    {
        int arg0 = KJSEmbed::extractInt(exec, args, 0);
        int arg1 = KJSEmbed::extractInt(exec, args, 1);
        int arg2 = KJSEmbed::extractInt(exec, args, 2);
        int arg3 = KJSEmbed::extractInt(exec, args, 3);
        QPixmap cppValue = value.copy(arg0, arg1, arg2, arg3);
        result = KJSEmbed::createVariant(exec, "QPixmap", cppValue );
    }
END_VARIANT_METHOD

START_VARIANT_METHOD( callcreateHeuristicMask, QPixmap )
    bool arg0 = KJSEmbed::extractBool(exec, args, 0);
    QBitmap cppValue = value.createHeuristicMask(arg0);
    result = KJSEmbed::createVariant(exec, "QBitmap", cppValue );
END_VARIANT_METHOD

START_VARIANT_METHOD( callcreateMaskFromColor, QPixmap )
    QColor arg0 = KJSEmbed::extractVariant<QColor>(exec,args, 0);
    QBitmap cppValue = value.createMaskFromColor(arg0);
    result = KJSEmbed::createVariant(exec, "QBitmap", cppValue );
END_VARIANT_METHOD

START_VARIANT_METHOD( calldefaultDepth, QPixmap )
    int cppValue = value.defaultDepth();
    result = KJS::jsNumber(cppValue);
END_VARIANT_METHOD

START_VARIANT_METHOD( calldepth, QPixmap )
    int cppValue = value.depth();
    result = KJS::jsNumber(cppValue);
END_VARIANT_METHOD

START_VARIANT_METHOD( callfill, QPixmap )
    if( args.size() == 3)
    {
        QWidget* arg0 = KJSEmbed::extractObject<QWidget>(exec,args, 0);
        int arg1 = KJSEmbed::extractInt(exec, args, 1);
        int arg2 = KJSEmbed::extractInt(exec, args, 2);
        value.fill(arg0, arg1, arg2);
    }
    else if( args.size() == 1)
    {
        QColor arg0 = KJSEmbed::extractVariant<QColor>(exec,args, 0);
        value.fill(arg0);
    }
    else if( args.size() == 2)
    {
        QWidget* arg0 = KJSEmbed::extractObject<QWidget>(exec,args, 0);
        QPoint arg1 = KJSEmbed::extractVariant<QPoint>(exec,args, 1);
        value.fill(arg0, arg1);
    }
END_VARIANT_METHOD

START_VARIANT_METHOD( callfromImage, QPixmap )
    QImage arg0 = KJSEmbed::extractVariant<QImage>(exec,args, 0);
    Qt::ImageConversionFlags arg1 = (Qt::ImageConversionFlags)KJSEmbed::extractInt(exec, args, 1);
    QPixmap cppValue = QPixmap::fromImage(arg0, arg1);
    result = KJSEmbed::createVariant(exec, "QPixmap", cppValue );
END_VARIANT_METHOD

START_VARIANT_METHOD( callgrabWidget, QPixmap )
    if( args.size() == 2)
    {
        QWidget* arg0 = KJSEmbed::extractObject<QWidget>(exec,args, 0);
        QRect arg1 = KJSEmbed::extractVariant<QRect>(exec,args, 1);
        QPixmap cppValue = value.grabWidget(arg0, arg1);
        result = KJSEmbed::createVariant(exec, "QPixmap", cppValue );
    }
    else if( args.size() == 5)
    {
        QWidget* arg0 = KJSEmbed::extractObject<QWidget>(exec,args, 0);
        int arg1 = KJSEmbed::extractInt(exec, args, 1);
        int arg2 = KJSEmbed::extractInt(exec, args, 2);
        int arg3 = KJSEmbed::extractInt(exec, args, 3);
        int arg4 = KJSEmbed::extractInt(exec, args, 4);
        QPixmap cppValue = value.grabWidget(arg0, arg1, arg2, arg3, arg4);
        result = KJSEmbed::createVariant(exec, "QPixmap", cppValue );
    }
END_VARIANT_METHOD

START_VARIANT_METHOD( callhasAlpha, QPixmap )
    bool cppValue = value.hasAlpha();
    result = KJS::jsBoolean(cppValue);
END_VARIANT_METHOD

START_VARIANT_METHOD( callhasAlphaChannel, QPixmap )
    bool cppValue = value.hasAlphaChannel();
    result = KJS::jsBoolean(cppValue);
END_VARIANT_METHOD

START_VARIANT_METHOD( callheight, QPixmap )
    int cppValue = value.height();
    result = KJS::jsNumber(cppValue);
END_VARIANT_METHOD

START_VARIANT_METHOD( callisNull, QPixmap )
    bool cppValue = value.isNull();
    result = KJS::jsBoolean(cppValue);
END_VARIANT_METHOD

START_VARIANT_METHOD( callisQBitmap, QPixmap )
    bool cppValue = value.isQBitmap();
    result = KJS::jsBoolean(cppValue);
END_VARIANT_METHOD

START_VARIANT_METHOD( callload, QPixmap )
    QString arg0 = KJSEmbed::extractQString(exec, args, 0);
    QByteArray arg1 = KJSEmbed::extractQString( exec, args, 1 ).toLatin1();
    Qt::ImageConversionFlags arg2 = (Qt::ImageConversionFlags) KJSEmbed::extractInt(exec, args, 2);
    bool cppValue = value.load(arg0, arg1, arg2);
    result = KJS::jsBoolean(cppValue);
END_VARIANT_METHOD

START_VARIANT_METHOD( callmask, QPixmap )
    QBitmap cppValue = value.mask();
    result = KJSEmbed::createVariant(exec, "QBitmap", cppValue );
END_VARIANT_METHOD

START_VARIANT_METHOD( callrect, QPixmap )
    QRect cppValue = value.rect();
    result = KJSEmbed::createVariant(exec, "QRect", cppValue );
END_VARIANT_METHOD

START_VARIANT_METHOD( callsave, QPixmap )
    QString arg0 = KJSEmbed::extractQString(exec, args, 0);
    QByteArray arg1 = KJSEmbed::extractQString( exec, args, 1 ).toLatin1();
    int arg2 = KJSEmbed::extractInt(exec, args, 2, -1);
    bool cppValue = value.save(arg0, arg1, arg2);
    result = KJS::jsBoolean(cppValue);
END_VARIANT_METHOD

START_VARIANT_METHOD( callscaled, QPixmap )
    if( args.size() == 3 )
    {
        QSize arg0 = KJSEmbed::extractVariant<QSize>(exec,args, 0);
        Qt::AspectRatioMode arg1 = (Qt::AspectRatioMode) KJSEmbed::extractInt(exec, args, 1);
        Qt::TransformationMode arg2 = (Qt::TransformationMode) KJSEmbed::extractInt(exec, args, 3);
        QPixmap cppValue = value.scaled(arg0, arg1, arg2);
        result = KJSEmbed::createVariant(exec, "QPixmap", cppValue );
    }
    else if ( args.size() == 4)
    {
        int arg0 = KJSEmbed::extractInt(exec, args, 0);
        int arg1 = KJSEmbed::extractInt(exec, args, 1);
        Qt::AspectRatioMode arg2 = (Qt::AspectRatioMode) KJSEmbed::extractInt(exec, args, 2);
        Qt::TransformationMode arg3 = (Qt::TransformationMode) KJSEmbed::extractInt(exec, args, 3);
        QPixmap cppValue = value.scaled(arg0, arg1, arg2, arg3);
        result = KJSEmbed::createVariant(exec, "QPixmap", cppValue );
    }
END_VARIANT_METHOD

START_VARIANT_METHOD( callscaledToHeight, QPixmap )
    int arg0 = KJSEmbed::extractInt(exec, args, 0);
    Qt::TransformationMode arg1  = (Qt::TransformationMode) KJSEmbed::extractInt(exec, args, 1);
    QPixmap cppValue = value.scaledToHeight(arg0, arg1);
    result = KJSEmbed::createVariant(exec, "QPixmap", cppValue );
END_VARIANT_METHOD

START_VARIANT_METHOD( callscaledToWidth, QPixmap )
    int arg0 = KJSEmbed::extractInt(exec, args, 0);
    Qt::TransformationMode arg1  = (Qt::TransformationMode) KJSEmbed::extractInt(exec, args, 1);
    QPixmap cppValue = value.scaledToWidth(arg0, arg1);
    result = KJSEmbed::createVariant(exec, "QPixmap", cppValue );
END_VARIANT_METHOD

START_VARIANT_METHOD( callserialNumber, QPixmap )
    int cppValue = value.serialNumber();
    result = KJS::jsNumber(cppValue);
END_VARIANT_METHOD

START_VARIANT_METHOD( callsetAlphaChannel, QPixmap )
    QPixmap arg0 = KJSEmbed::extractVariant<QPixmap>(exec,args, 0);
    value.setAlphaChannel(arg0);
END_VARIANT_METHOD

START_VARIANT_METHOD( callsetMask, QPixmap )
    QBitmap arg0 = KJSEmbed::extractVariant<QBitmap>(exec,args, 0);
    value.setMask(arg0);
END_VARIANT_METHOD

START_VARIANT_METHOD( callsize, QPixmap )
    QSize cppValue = value.size();
    result = KJSEmbed::createVariant(exec, "QSize", cppValue );
END_VARIANT_METHOD

START_VARIANT_METHOD( calltoImage, QPixmap )
    QImage cppValue = value.toImage();
    result = KJSEmbed::createVariant(exec, "QImage", cppValue );
END_VARIANT_METHOD

START_VARIANT_METHOD( callwidth, QPixmap )
    int cppValue = value.width();
    result = KJS::jsNumber(cppValue);
END_VARIANT_METHOD

}

START_METHOD_LUT( Pixmap )
    {"alphaChannel", 0, KJS::DontDelete|KJS::ReadOnly, &PixmapNS::callalphaChannel},
    {"copy", 3, KJS::DontDelete|KJS::ReadOnly, &PixmapNS::callcopy},
    {"createHeuristicMask", 0, KJS::DontDelete|KJS::ReadOnly, &PixmapNS::callcreateHeuristicMask},
    {"createMaskFromColor", 0, KJS::DontDelete|KJS::ReadOnly, &PixmapNS::callcreateMaskFromColor},
    {"defaultDepth", 0, KJS::DontDelete|KJS::ReadOnly, &PixmapNS::calldefaultDepth},
    {"depth", 0, KJS::DontDelete|KJS::ReadOnly, &PixmapNS::calldepth},
    {"fill", 2, KJS::DontDelete|KJS::ReadOnly, &PixmapNS::callfill},
    {"fromImage", 1, KJS::DontDelete|KJS::ReadOnly, &PixmapNS::callfromImage},
    {"grabWidget", 4, KJS::DontDelete|KJS::ReadOnly, &PixmapNS::callgrabWidget},
    {"hasAlpha", 0, KJS::DontDelete|KJS::ReadOnly, &PixmapNS::callhasAlpha},
    {"hasAlphaChannel", 0, KJS::DontDelete|KJS::ReadOnly, &PixmapNS::callhasAlphaChannel},
    {"height", 0, KJS::DontDelete|KJS::ReadOnly, &PixmapNS::callheight},
    {"isNull", 0, KJS::DontDelete|KJS::ReadOnly, &PixmapNS::callisNull},
    {"isQBitmap", 0, KJS::DontDelete|KJS::ReadOnly, &PixmapNS::callisQBitmap},
    {"load", 2, KJS::DontDelete|KJS::ReadOnly, &PixmapNS::callload},
    {"mask", 0, KJS::DontDelete|KJS::ReadOnly, &PixmapNS::callmask},
    {"rect", 0, KJS::DontDelete|KJS::ReadOnly, &PixmapNS::callrect},
    {"save", 2, KJS::DontDelete|KJS::ReadOnly, &PixmapNS::callsave},
    {"scaled", 3, KJS::DontDelete|KJS::ReadOnly, &PixmapNS::callscaled},
    {"scaledToHeight", 1, KJS::DontDelete|KJS::ReadOnly, &PixmapNS::callscaledToHeight},
    {"scaledToWidth", 1, KJS::DontDelete|KJS::ReadOnly, &PixmapNS::callscaledToWidth},
    {"serialNumber", 0, KJS::DontDelete|KJS::ReadOnly, &PixmapNS::callserialNumber},
    {"setAlphaChannel", 0, KJS::DontDelete|KJS::ReadOnly, &PixmapNS::callsetAlphaChannel},
    {"setMask", 0, KJS::DontDelete|KJS::ReadOnly, &PixmapNS::callsetMask},
    {"size", 0, KJS::DontDelete|KJS::ReadOnly, &PixmapNS::callsize},
    {"toImage", 0, KJS::DontDelete|KJS::ReadOnly, &PixmapNS::calltoImage},
    {"width", 0, KJS::DontDelete|KJS::ReadOnly, &PixmapNS::callwidth}
END_METHOD_LUT

NO_STATICS( Pixmap )
NO_ENUMS( Pixmap )
START_CTOR( Pixmap, QPixmap, 0)
    if( args.size() == 0 )
    {
        return new KJSEmbed::PixmapBinding( exec, QPixmap() );
    }
    else if( args.size() == 1 )
    {
        return new KJSEmbed::PixmapBinding( exec,
                                        QPixmap( KJSEmbed::extractQString(exec,args, 0) ) );
    }
    else if( args.size() == 2 )
    {
        return new KJSEmbed::PixmapBinding(exec,
                                QPixmap( KJSEmbed::extractInt( exec, args, 0 ),
                                                KJSEmbed::extractInt( exec, args, 1 )
                                        ) );
    }
    else if( args.size() == 3 )
    {
        QString tmp = toQString(args[2]->toObject(exec)->className());
        qDebug() << tmp;

        return new KJSEmbed::PixmapBinding(exec,
                                QPixmap( KJSEmbed::extractQString( exec, args, 0 ),
                                                KJSEmbed::extractVariant<QByteArray>( exec, args, 1 ).constData(),
                                                (Qt::ImageConversionFlags)KJSEmbed::extractInt( exec, args, 2 )
                                        ) );
    }
    return new KJSEmbed::PixmapBinding( exec, QPixmap() );
END_CTOR


//kate: indent-spaces on; indent-width 4; replace-tabs on; indent-mode cstyle;
