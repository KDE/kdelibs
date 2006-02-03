#include "image.h"
#include "value_binding.h"
#include "object_binding.h"

#include <QDebug>
#include <QImage>
#include <QPoint>
#include <QSize>

using namespace KJSEmbed;

ImageBinding::ImageBinding( KJS::ExecState *exec, const QImage &value )
    : ValueBinding(exec, value )
{
    StaticBinding::publish( exec, this, ValueFactory::methods() );
    StaticBinding::publish( exec, this, Image::methods() );
}

namespace ImageNS
{
START_VARIANT_METHOD( callallGray, QImage )
    bool cppValue = value.allGray();
    result = KJS::Boolean(cppValue);
END_VARIANT_METHOD

START_VARIANT_METHOD( callalphaChannel, QImage )
    QImage cppValue = value.alphaChannel();
    result = KJSEmbed::createValue(exec, "QImage", cppValue );
END_VARIANT_METHOD

START_VARIANT_METHOD( callcopy, QImage )
    int arg0 = KJSEmbed::extractInt(exec, args, 0);
    int arg1 = KJSEmbed::extractInt(exec, args, 1);
    int arg2 = KJSEmbed::extractInt(exec, args, 2);
    int arg3 = KJSEmbed::extractInt(exec, args, 3);
    QImage cppValue = value.copy(arg0, arg1, arg2, arg3);
    result = KJSEmbed::createValue(exec, "QImage", cppValue );
END_VARIANT_METHOD

START_VARIANT_METHOD( callcreateAlphaMask, QImage )
    Qt::ImageConversionFlags arg0 = (Qt::ImageConversionFlags)KJSEmbed::extractInt(exec, args, 0);
    QImage cppValue = value.createAlphaMask(arg0);
    result = KJSEmbed::createValue(exec, "QImage", cppValue );
END_VARIANT_METHOD

START_VARIANT_METHOD( callcreateHeuristicMask, QImage )
    bool arg0 = KJSEmbed::extractBool(exec, args, 0);
    QImage cppValue = value.createHeuristicMask(arg0);
    result = KJSEmbed::createValue(exec, "QImage", cppValue );
END_VARIANT_METHOD

START_VARIANT_METHOD( calldepth, QImage )
    int cppValue = value.depth();
    result = KJS::Number(cppValue);
END_VARIANT_METHOD

START_VARIANT_METHOD( calldevType, QImage )
    int cppValue = value.devType();
    result = KJS::Number(cppValue);
END_VARIANT_METHOD

START_VARIANT_METHOD( calldotsPerMeterX, QImage )
    int cppValue = value.dotsPerMeterX();
    result = KJS::Number(cppValue);
END_VARIANT_METHOD

START_VARIANT_METHOD( calldotsPerMeterY, QImage )
    int cppValue = value.dotsPerMeterY();
    result = KJS::Number(cppValue);
END_VARIANT_METHOD

START_VARIANT_METHOD( callfill, QImage )
    uint arg0 = KJSEmbed::extractValue<uint>(exec,args, 0);
    value.fill(arg0);
END_VARIANT_METHOD

START_VARIANT_METHOD( callhasAlphaChannel, QImage )
    bool cppValue = value.hasAlphaChannel();
    result = KJS::Boolean(cppValue);
END_VARIANT_METHOD

START_VARIANT_METHOD( callheight, QImage )
    int cppValue = value.height();
    result = KJS::Number(cppValue);
END_VARIANT_METHOD

START_VARIANT_METHOD( callinvertPixels, QImage )
    QImage::InvertMode arg0 = (QImage::InvertMode)KJSEmbed::extractInt(exec, args, 0);
    value.invertPixels(arg0);
END_VARIANT_METHOD

START_VARIANT_METHOD( callisGrayscale, QImage )
    bool cppValue = value.isGrayscale();
    result = KJS::Boolean(cppValue);
END_VARIANT_METHOD

START_VARIANT_METHOD( callisNull, QImage )
    bool cppValue = value.isNull();
    result = KJS::Boolean(cppValue);
END_VARIANT_METHOD

START_VARIANT_METHOD( callload, QImage )
    QString arg0 = KJSEmbed::extractQString(exec, args, 0);
    QByteArray arg1 = KJSEmbed::extractQString( exec, args, 1 ).toLatin1();
    bool cppValue = value.load(arg0, arg1);
    result = KJS::Boolean(cppValue);
END_VARIANT_METHOD

START_VARIANT_METHOD( callmirrored, QImage )
    bool arg0 = KJSEmbed::extractBool(exec, args, 0);
    bool arg1 = KJSEmbed::extractBool(exec, args, 1);
    QImage cppValue = value.mirrored(arg0, arg1);
    result = KJSEmbed::createValue(exec, "QImage", cppValue );
END_VARIANT_METHOD

START_VARIANT_METHOD( callnumBytes, QImage )
    int cppValue = value.numBytes();
    result = KJS::Number(cppValue);
END_VARIANT_METHOD

START_VARIANT_METHOD( callnumColors, QImage )
    int cppValue = value.numColors();
    result = KJS::Number(cppValue);
END_VARIANT_METHOD

START_VARIANT_METHOD( calloffset, QImage )
    QPoint cppValue = value.offset();
    result = KJSEmbed::createValue(exec, "QPoint", cppValue );
END_VARIANT_METHOD

START_VARIANT_METHOD( callpixel, QImage )
    int arg0 = KJSEmbed::extractInt(exec, args, 0);
    int arg1 = KJSEmbed::extractInt(exec, args, 1);
    QRgb cppValue = value.pixel(arg0, arg1);
    result = KJS::Number(cppValue);
END_VARIANT_METHOD

START_VARIANT_METHOD( callpixelIndex, QImage )
    int arg0 = KJSEmbed::extractInt(exec, args, 0);
    int arg1 = KJSEmbed::extractInt(exec, args, 1);
    int cppValue = value.pixelIndex(arg0, arg1);
    result = KJS::Number(cppValue);
END_VARIANT_METHOD

START_VARIANT_METHOD( callrect, QImage )
    QRect cppValue = value.rect();
    result = KJSEmbed::createValue(exec, "QRect", cppValue );
END_VARIANT_METHOD

START_VARIANT_METHOD( callrgbSwapped, QImage )
    QImage cppValue = value.rgbSwapped();
    result = KJSEmbed::createValue(exec, "QImage", cppValue );
END_VARIANT_METHOD

START_VARIANT_METHOD( callsave, QImage )
    QString arg0 = KJSEmbed::extractQString(exec, args, 0);
    QByteArray arg1 = KJSEmbed::extractQString( exec, args, 1 ).toLatin1();
    int arg2 = KJSEmbed::extractInt(exec, args, 2);
    bool cppValue = value.save(arg0, arg1, arg2);
    result = KJS::Boolean(cppValue);
END_VARIANT_METHOD

START_VARIANT_METHOD( callscaled, QImage )
    if( args.size() == 3)
    {
        QSize arg0 = KJSEmbed::extractValue<QSize>(exec,args, 0);
        Qt::AspectRatioMode arg1 = (Qt::AspectRatioMode)KJSEmbed::extractInt(exec, args, 1);
        Qt::TransformationMode arg2 = (Qt::TransformationMode)KJSEmbed::extractInt(exec, args, 2);
        QImage cppValue = value.scaled(arg0, arg1, arg2);
        result = KJSEmbed::createValue(exec, "QImage", cppValue );
    }
    else if( args.size() == 4)
    {
        int arg0 = KJSEmbed::extractInt(exec, args, 0);
        int arg1 = KJSEmbed::extractInt(exec, args, 1);
        Qt::AspectRatioMode arg2 = (Qt::AspectRatioMode)KJSEmbed::extractInt(exec, args, 2);
        Qt::TransformationMode arg3 = (Qt::TransformationMode)KJSEmbed::extractInt(exec, args, 3);
        QImage cppValue = value.scaled(arg0, arg1, arg2, arg3);
        result = KJSEmbed::createValue(exec, "QImage", cppValue );
    }
END_VARIANT_METHOD

START_VARIANT_METHOD( callscaledToHeight, QImage )
    int arg0 = KJSEmbed::extractInt(exec, args, 0);
    Qt::TransformationMode arg1 = (Qt::TransformationMode)KJSEmbed::extractInt(exec, args, 1);
    QImage cppValue = value.scaledToHeight(arg0, arg1);
    result = KJSEmbed::createValue(exec, "QImage", cppValue );
END_VARIANT_METHOD

START_VARIANT_METHOD( callscaledToWidth, QImage )
    int arg0 = KJSEmbed::extractInt(exec, args, 0);
    Qt::TransformationMode arg1 = (Qt::TransformationMode)KJSEmbed::extractInt(exec, args, 1);
    QImage cppValue = value.scaledToWidth(arg0, arg1);
    result = KJSEmbed::createValue(exec, "QImage", cppValue );
END_VARIANT_METHOD

START_VARIANT_METHOD( callserialNumber, QImage )
    int cppValue = value.serialNumber();
    result = KJS::Number(cppValue);
END_VARIANT_METHOD

START_VARIANT_METHOD( callsetAlphaChannel, QImage )
    QImage arg0 = KJSEmbed::extractValue<QImage>(exec,args, 0);
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
    value.setNumColors(arg0);
END_VARIANT_METHOD

START_VARIANT_METHOD( callsetOffset, QImage )
    QPoint arg0 = KJSEmbed::extractValue<QPoint>(exec,args, 0);
    value.setOffset(arg0);
END_VARIANT_METHOD

START_VARIANT_METHOD( callsetPixel, QImage )
    int arg0 = KJSEmbed::extractInt(exec, args, 0);
    int arg1 = KJSEmbed::extractInt(exec, args, 1);
    uint arg2 = KJSEmbed::extractValue<uint>(exec,args, 2);
    value.setPixel(arg0, arg1, arg2);
END_VARIANT_METHOD

START_VARIANT_METHOD( callsize, QImage )
    QSize cppValue = value.size();
    result = KJSEmbed::createValue(exec, "QSize", cppValue );
END_VARIANT_METHOD

START_VARIANT_METHOD( callvalid, QImage )
    int arg0 = KJSEmbed::extractInt(exec, args, 0);
    int arg1 = KJSEmbed::extractInt(exec, args, 1);
    bool cppValue = value.valid(arg0, arg1);
    result = KJS::Boolean(cppValue);
END_VARIANT_METHOD

START_VARIANT_METHOD( callwidth, QImage )
    int cppValue = value.width();
    result = KJS::Number(cppValue);
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
                QImage( KJSEmbed::extractValue<QSize>(exec,args, 0 ),
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
