#include "pixmap.h"

#include <QDebug>
#include <QPixmap>
#include <QBitmap>

#include <value_binding.h>
#include <object_binding.h>

using namespace KJSEmbed;

PixmapBinding::PixmapBinding( KJS::ExecState *exec, const QPixmap &value )
    : ValueBinding(exec, value )
{
    StaticBinding::publish( exec, this, Pixmap::methods() );
    StaticBinding::publish( exec, this, ValueFactory::methods() );
}

namespace PixmapNS
{
START_VARIANT_METHOD( callalphaChannel, QPixmap )
    QPixmap cppValue = value.alphaChannel();
    result = KJSEmbed::createValue(exec, "QPixmap", cppValue );
END_VARIANT_METHOD

START_VARIANT_METHOD( callcopy, QPixmap )
    if( args.size() == 1)
    {
        QRect arg0 = KJSEmbed::extractValue<QRect>(exec,args, 0);
        QPixmap cppValue = value.copy(arg0);
        result = KJSEmbed::createValue(exec, "QPixmap", cppValue );
    }
    else if( args.size() == 4)
    {
        int arg0 = KJSEmbed::extractInt(exec, args, 0);
        int arg1 = KJSEmbed::extractInt(exec, args, 1);
        int arg2 = KJSEmbed::extractInt(exec, args, 2);
        int arg3 = KJSEmbed::extractInt(exec, args, 3);
        QPixmap cppValue = value.copy(arg0, arg1, arg2, arg3);
        result = KJSEmbed::createValue(exec, "QPixmap", cppValue );
    }
END_VARIANT_METHOD

START_VARIANT_METHOD( callcreateHeuristicMask, QPixmap )
    bool arg0 = KJSEmbed::extractBool(exec, args, 0);
    QBitmap cppValue = value.createHeuristicMask(arg0);
    result = KJSEmbed::createValue(exec, "QBitmap", cppValue );
END_VARIANT_METHOD

START_VARIANT_METHOD( callcreateMaskFromColor, QPixmap )
    QColor arg0 = KJSEmbed::extractValue<QColor>(exec,args, 0);
    QBitmap cppValue = value.createMaskFromColor(arg0);
    result = KJSEmbed::createValue(exec, "QBitmap", cppValue );
END_VARIANT_METHOD

START_VARIANT_METHOD( calldefaultDepth, QPixmap )
    int cppValue = value.defaultDepth();
    result = KJS::Number(cppValue);
END_VARIANT_METHOD

START_VARIANT_METHOD( calldepth, QPixmap )
    int cppValue = value.depth();
    result = KJS::Number(cppValue);
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
        QColor arg0 = KJSEmbed::extractValue<QColor>(exec,args, 0);
        value.fill(arg0);
    }
    else if( args.size() == 2)
    {
        QWidget* arg0 = KJSEmbed::extractObject<QWidget>(exec,args, 0);
        QPoint arg1 = KJSEmbed::extractValue<QPoint>(exec,args, 1);
        value.fill(arg0, arg1);
    }
END_VARIANT_METHOD

START_VARIANT_METHOD( callfromImage, QPixmap )
    QImage arg0 = KJSEmbed::extractValue<QImage>(exec,args, 0);
    Qt::ImageConversionFlags arg1 = (Qt::ImageConversionFlags)KJSEmbed::extractInt(exec, args, 1);
    QPixmap cppValue = value.fromImage(arg0, arg1);
    result = KJSEmbed::createValue(exec, "QPixmap", cppValue );
END_VARIANT_METHOD

START_VARIANT_METHOD( callgrabWidget, QPixmap )
    if( args.size() == 2)
    {
        QWidget* arg0 = KJSEmbed::extractObject<QWidget>(exec,args, 0);
        QRect arg1 = KJSEmbed::extractValue<QRect>(exec,args, 1);
        QPixmap cppValue = value.grabWidget(arg0, arg1);
        result = KJSEmbed::createValue(exec, "QPixmap", cppValue );
    }
    else if( args.size() == 5)
    {
        QWidget* arg0 = KJSEmbed::extractObject<QWidget>(exec,args, 0);
        int arg1 = KJSEmbed::extractInt(exec, args, 1);
        int arg2 = KJSEmbed::extractInt(exec, args, 2);
        int arg3 = KJSEmbed::extractInt(exec, args, 3);
        int arg4 = KJSEmbed::extractInt(exec, args, 4);
        QPixmap cppValue = value.grabWidget(arg0, arg1, arg2, arg3, arg4);
        result = KJSEmbed::createValue(exec, "QPixmap", cppValue );
    }
END_VARIANT_METHOD

START_VARIANT_METHOD( callhasAlpha, QPixmap )
    bool cppValue = value.hasAlpha();
    result = KJS::Boolean(cppValue);
END_VARIANT_METHOD

START_VARIANT_METHOD( callhasAlphaChannel, QPixmap )
    bool cppValue = value.hasAlphaChannel();
    result = KJS::Boolean(cppValue);
END_VARIANT_METHOD

START_VARIANT_METHOD( callheight, QPixmap )
    int cppValue = value.height();
    result = KJS::Number(cppValue);
END_VARIANT_METHOD

START_VARIANT_METHOD( callisNull, QPixmap )
    bool cppValue = value.isNull();
    result = KJS::Boolean(cppValue);
END_VARIANT_METHOD

START_VARIANT_METHOD( callisQBitmap, QPixmap )
    bool cppValue = value.isQBitmap();
    result = KJS::Boolean(cppValue);
END_VARIANT_METHOD

START_VARIANT_METHOD( callload, QPixmap )
    QString arg0 = KJSEmbed::extractQString(exec, args, 0);
    QByteArray arg1 = KJSEmbed::extractQString( exec, args, 1 ).toLatin1();
    Qt::ImageConversionFlags arg2 = (Qt::ImageConversionFlags) KJSEmbed::extractInt(exec, args, 2);
    bool cppValue = value.load(arg0, arg1, arg2);
    result = KJS::Boolean(cppValue);
END_VARIANT_METHOD

START_VARIANT_METHOD( callmask, QPixmap )
    QBitmap cppValue = value.mask();
    result = KJSEmbed::createValue(exec, "QBitmap", cppValue );
END_VARIANT_METHOD

START_VARIANT_METHOD( callrect, QPixmap )
    QRect cppValue = value.rect();
    result = KJSEmbed::createValue(exec, "QRect", cppValue );
END_VARIANT_METHOD

START_VARIANT_METHOD( callsave, QPixmap )
    QString arg0 = KJSEmbed::extractQString(exec, args, 0);
    QByteArray arg1 = KJSEmbed::extractQString( exec, args, 1 ).toLatin1();
    int arg2 = KJSEmbed::extractInt(exec, args, 2, -1);
    bool cppValue = value.save(arg0, arg1, arg2);
    result = KJS::Boolean(cppValue);
END_VARIANT_METHOD

START_VARIANT_METHOD( callscaled, QPixmap )
    if( args.size() == 3 )
    {
        QSize arg0 = KJSEmbed::extractValue<QSize>(exec,args, 0);
        Qt::AspectRatioMode arg1 = (Qt::AspectRatioMode) KJSEmbed::extractInt(exec, args, 1);
        Qt::TransformationMode arg2 = (Qt::TransformationMode) KJSEmbed::extractInt(exec, args, 3);
        QPixmap cppValue = value.scaled(arg0, arg1, arg2);
        result = KJSEmbed::createValue(exec, "QPixmap", cppValue );
    }
    else if ( args.size() == 4)
    {
        int arg0 = KJSEmbed::extractInt(exec, args, 0);
        int arg1 = KJSEmbed::extractInt(exec, args, 1);
        Qt::AspectRatioMode arg2 = (Qt::AspectRatioMode) KJSEmbed::extractInt(exec, args, 2);
        Qt::TransformationMode arg3 = (Qt::TransformationMode) KJSEmbed::extractInt(exec, args, 3);
        QPixmap cppValue = value.scaled(arg0, arg1, arg2, arg3);
        result = KJSEmbed::createValue(exec, "QPixmap", cppValue );
    }
END_VARIANT_METHOD

START_VARIANT_METHOD( callscaledToHeight, QPixmap )
    int arg0 = KJSEmbed::extractInt(exec, args, 0);
    Qt::TransformationMode arg1  = (Qt::TransformationMode) KJSEmbed::extractInt(exec, args, 1);
    QPixmap cppValue = value.scaledToHeight(arg0, arg1);
    result = KJSEmbed::createValue(exec, "QPixmap", cppValue );
END_VARIANT_METHOD

START_VARIANT_METHOD( callscaledToWidth, QPixmap )
    int arg0 = KJSEmbed::extractInt(exec, args, 0);
    Qt::TransformationMode arg1  = (Qt::TransformationMode) KJSEmbed::extractInt(exec, args, 1);
    QPixmap cppValue = value.scaledToWidth(arg0, arg1);
    result = KJSEmbed::createValue(exec, "QPixmap", cppValue );
END_VARIANT_METHOD

START_VARIANT_METHOD( callserialNumber, QPixmap )
    int cppValue = value.serialNumber();
    result = KJS::Number(cppValue);
END_VARIANT_METHOD

START_VARIANT_METHOD( callsetAlphaChannel, QPixmap )
    QPixmap arg0 = KJSEmbed::extractValue<QPixmap>(exec,args, 0);
    value.setAlphaChannel(arg0);
END_VARIANT_METHOD

START_VARIANT_METHOD( callsetMask, QPixmap )
    QBitmap arg0 = KJSEmbed::extractValue<QBitmap>(exec,args, 0);
    value.setMask(arg0);
END_VARIANT_METHOD

START_VARIANT_METHOD( callsize, QPixmap )
    QSize cppValue = value.size();
    result = KJSEmbed::createValue(exec, "QSize", cppValue );
END_VARIANT_METHOD

START_VARIANT_METHOD( calltoImage, QPixmap )
    QImage cppValue = value.toImage();
    result = KJSEmbed::createValue(exec, "QImage", cppValue );
END_VARIANT_METHOD

START_VARIANT_METHOD( callwidth, QPixmap )
    int cppValue = value.width();
    result = KJS::Number(cppValue);
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
        return new KJSEmbed::PixmapBinding(exec,
                                QPixmap( KJSEmbed::extractQString( exec, args, 0 ),
                                                KJSEmbed::extractValue<QByteArray>( exec, args, 1 ).constData(),
                                                (Qt::ImageConversionFlags)KJSEmbed::extractInt( exec, args, 2 )
                                        ) );
    }
    return new KJSEmbed::PixmapBinding( exec, QPixmap() );
END_CTOR


