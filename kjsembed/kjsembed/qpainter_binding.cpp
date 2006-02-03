#include "qpainter_binding.h"
#include "object_binding.h"
#include "static_binding.h"
#include "global.h"

#include <kjs/object.h>
#include <qdebug.h>

#include <QWidget>
#include <QPainter>
#include <QImage>
#include <QPixmap>
#include <QPen>
#include <QBrush>
#include <QLine>
#include <QPolygon>
#include <QPoint>

#include <QFrame>

using namespace KJSEmbed;

QPaintDevice *extractPaintDevice( KJS::ExecState *exec, KJS::JSValue *arg)
{
    QPaintDevice *device = 0;

    ObjectBinding *imp = extractBindingImp<ObjectBinding>(exec,arg);
    if( imp )
    {
        device = imp->object<QPaintDevice>();
    }
    else
    {
        ValueBinding *valueImp = extractBindingImp<ValueBinding>(exec,arg);
        if( valueImp->variant().type() == QVariant::Pixmap ||
                valueImp->variant().type() == QVariant::Image )
        {
            device = static_cast<QPaintDevice*>( valueImp->pointer() );
        }
    }
    return device;
}

START_OBJECT_METHOD( callPainterBegin, QPainter )
    result = KJS::Boolean(false);
    QPaintDevice *device = extractPaintDevice(exec, args[0]);
    if( device )
    {
        result = KJS::Boolean(object->begin(device));
    }
END_OBJECT_METHOD

START_OBJECT_METHOD( callPainterEnd, QPainter )
    result = KJS::Boolean(object->end());
END_OBJECT_METHOD

START_OBJECT_METHOD( callbackground, QPainter )
    QBrush cppValue = object->background();
    result = KJSEmbed::createValue(exec, "QBrush", cppValue );
END_OBJECT_METHOD

START_OBJECT_METHOD( callbackgroundMode, QPainter )
    Qt::BGMode cppValue = object->backgroundMode();
    result = KJS::Number(cppValue);
END_OBJECT_METHOD

START_OBJECT_METHOD( callboundingRect, QPainter )
    if( args.size() == 3 )
    {
        QRect arg0 = KJSEmbed::extractValue<QRect>(exec,args, 0);
        int arg1 = KJSEmbed::extractInt(exec, args, 1);
        QString arg2 = KJSEmbed::extractQString(exec, args, 2);
        QRect cppValue = object->boundingRect(arg0, arg1, arg2);
        result = KJSEmbed::createValue(exec, "QRect", cppValue );
    }
    else if( args.size() == 6)
    {
        int arg0 = KJSEmbed::extractInt(exec, args, 0);
        int arg1 = KJSEmbed::extractInt(exec, args, 1);
        int arg2 = KJSEmbed::extractInt(exec, args, 2);
        int arg3 = KJSEmbed::extractInt(exec, args, 3);
        int arg4 = KJSEmbed::extractInt(exec, args, 4);
        QString arg5 = KJSEmbed::extractQString(exec, args, 5);
        QRect cppValue = object->boundingRect(arg0, arg1, arg2, arg3, arg4, arg5);
        result = KJSEmbed::createValue(exec, "QRect", cppValue );
    }
END_OBJECT_METHOD

START_OBJECT_METHOD( callbrush, QPainter )
    QBrush cppValue = object->brush();
    result = KJSEmbed::createValue(exec, "QBrush", cppValue );
END_OBJECT_METHOD

START_OBJECT_METHOD( callbrushOrigin, QPainter )
    QPoint cppValue = object->brushOrigin();
    result = KJSEmbed::createValue(exec, "QPoint", cppValue );
END_OBJECT_METHOD

START_OBJECT_METHOD( calldrawArc, QPainter )
    if( args.size() == 3 )
    {
        QRect arg0 = KJSEmbed::extractValue<QRect>(exec,args, 0);
        int arg1 = KJSEmbed::extractInt(exec, args, 1);
        int arg2 = KJSEmbed::extractInt(exec, args, 2);
        object->drawArc(arg0, arg1, arg2);
    }
    else if( args.size() == 6 )
    {
        int arg0 = KJSEmbed::extractInt(exec, args, 0);
        int arg1 = KJSEmbed::extractInt(exec, args, 1);
        int arg2 = KJSEmbed::extractInt(exec, args, 2);
        int arg3 = KJSEmbed::extractInt(exec, args, 3);
        int arg4 = KJSEmbed::extractInt(exec, args, 4);
        int arg5 = KJSEmbed::extractInt(exec, args, 5);
        object->drawArc(arg0, arg1, arg2, arg3, arg4, arg5);
    }
END_OBJECT_METHOD

START_OBJECT_METHOD( calldrawChord, QPainter )
    if( args.size() == 3 )
    {
        QRect arg0 = KJSEmbed::extractValue<QRect>(exec,args, 0);
        int arg1 = KJSEmbed::extractInt(exec, args, 1);
        int arg2 = KJSEmbed::extractInt(exec, args, 2);
        object->drawChord(arg0, arg1, arg2);
    }
    else if ( args.size() == 6 )
    {
        int arg0 = KJSEmbed::extractInt(exec, args, 0);
        int arg1 = KJSEmbed::extractInt(exec, args, 1);
        int arg2 = KJSEmbed::extractInt(exec, args, 2);
        int arg3 = KJSEmbed::extractInt(exec, args, 3);
        int arg4 = KJSEmbed::extractInt(exec, args, 4);
        int arg5 = KJSEmbed::extractInt(exec, args, 5);
        object->drawChord(arg0, arg1, arg2, arg3, arg4, arg5);
    }
END_OBJECT_METHOD

START_OBJECT_METHOD( calldrawConvexPolygon, QPainter )
    QPolygon arg0 = KJSEmbed::extractValue<QPolygon>(exec,args, 0);
    object->drawConvexPolygon(arg0);
END_OBJECT_METHOD

START_OBJECT_METHOD( calldrawEllipse, QPainter )
    if ( args.size() == 4 )
    {
        int arg0 = KJSEmbed::extractInt(exec, args, 0);
        int arg1 = KJSEmbed::extractInt(exec, args, 1);
        int arg2 = KJSEmbed::extractInt(exec, args, 2);
        int arg3 = KJSEmbed::extractInt(exec, args, 3);
        object->drawEllipse(arg0, arg1, arg2, arg3);
    }
    else if ( args.size() == 1 )
    {
        QRect arg0 = KJSEmbed::extractValue<QRect>(exec,args, 0);
        object->drawEllipse(arg0);
    }
END_OBJECT_METHOD

START_OBJECT_METHOD( calldrawImage, QPainter )
    if ( args.size() == 2 )
    {
        QPoint arg0 = KJSEmbed::extractValue<QPoint>(exec,args, 0);
        QImage arg1 = KJSEmbed::extractValue<QImage>(exec,args, 1);
        object->drawImage(arg0, arg1);
    }
    else if ( args.size() == 4 )
    {
        QPoint arg0 = KJSEmbed::extractValue<QPoint>(exec,args, 0);
        QImage arg1 = KJSEmbed::extractValue<QImage>(exec,args, 1);
        QRect arg2 = KJSEmbed::extractValue<QRect>(exec,args, 2);
        Qt::ImageConversionFlags arg3 = (Qt::ImageConversionFlags)KJSEmbed::extractInt(exec, args, 3);
        object->drawImage(arg0, arg1, arg2, arg3);
    }
    else if ( args.size() == 8 )
    {
        int arg0 = KJSEmbed::extractInt(exec, args, 0);
        int arg1 = KJSEmbed::extractInt(exec, args, 1);
        QImage arg2 = KJSEmbed::extractValue<QImage>(exec,args, 2);
        int arg3 = KJSEmbed::extractInt(exec, args, 3);
        int arg4 = KJSEmbed::extractInt(exec, args, 4);
        int arg5 = KJSEmbed::extractInt(exec, args, 5);
        int arg6 = KJSEmbed::extractInt(exec, args, 6);
        Qt::ImageConversionFlags arg7 = (Qt::ImageConversionFlags)KJSEmbed::extractInt(exec, args, 7);
        object->drawImage(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7);
    }
END_OBJECT_METHOD

START_OBJECT_METHOD( calldrawLine, QPainter )
    if( args.size() == 1 )
    {
        QLine arg0 = KJSEmbed::extractValue<QLine>(exec,args, 0);
        object->drawLine(arg0);
    }
    else if ( args.size() == 2 )
    {
        QPoint arg0 = KJSEmbed::extractValue<QPoint>(exec,args, 0);
        QPoint arg1 = KJSEmbed::extractValue<QPoint>(exec,args, 1);
        object->drawLine(arg0, arg1);
    }
    else if ( args.size() == 4 )
    {
        int arg0 = KJSEmbed::extractInt(exec, args, 0);
        int arg1 = KJSEmbed::extractInt(exec, args, 1);
        int arg2 = KJSEmbed::extractInt(exec, args, 2);
        int arg3 = KJSEmbed::extractInt(exec, args, 3);
        object->drawLine(arg0, arg1, arg2, arg3);
    }
END_OBJECT_METHOD

START_OBJECT_METHOD( calldrawPie, QPainter )
    if( args.size() == 3 )
    {
        QRect arg0 = KJSEmbed::extractValue<QRect>(exec,args, 0);
        int arg1 = KJSEmbed::extractInt(exec, args, 1);
        int arg2 = KJSEmbed::extractInt(exec, args, 2);
        object->drawPie(arg0, arg1, arg2);
    }
    else if (args.size() == 6 )
    {
        int arg0 = KJSEmbed::extractInt(exec, args, 0);
        int arg1 = KJSEmbed::extractInt(exec, args, 1);
        int arg2 = KJSEmbed::extractInt(exec, args, 2);
        int arg3 = KJSEmbed::extractInt(exec, args, 3);
        int arg4 = KJSEmbed::extractInt(exec, args, 4);
        int arg5 = KJSEmbed::extractInt(exec, args, 5);
        object->drawPie(arg0, arg1, arg2, arg3, arg4, arg5);
    }
END_OBJECT_METHOD

START_OBJECT_METHOD( calldrawPixmap, QPainter )
    if ( args.size() == 2)
    {
        QPoint arg0 = KJSEmbed::extractValue<QPoint>(exec,args, 0);
        QPixmap arg1 = KJSEmbed::extractValue<QPixmap>(exec,args, 1);
        object->drawPixmap(arg0, arg1);
    }
    else if ( args.size() == 3 )
    {
        QPoint arg0 = KJSEmbed::extractValue<QPoint>(exec,args, 0);
        QPixmap arg1 = KJSEmbed::extractValue<QPixmap>(exec,args, 1);
        QRect arg2 = KJSEmbed::extractValue<QRect>(exec,args, 2);
        object->drawPixmap(arg0, arg1, arg2);
    }
END_OBJECT_METHOD

START_OBJECT_METHOD( calldrawPoint, QPainter )
    if( args.size() == 1 )
    {
        QPoint arg0 = KJSEmbed::extractValue<QPoint>(exec,args, 0);
        object->drawPoint(arg0);
    }
    else if (args.size() == 2 )
    {
        int arg0 = KJSEmbed::extractInt(exec, args, 0);
        int arg1 = KJSEmbed::extractInt(exec, args, 1);
        object->drawPoint(arg0, arg1);
    }
END_OBJECT_METHOD

START_OBJECT_METHOD( calldrawPoints, QPainter )
    QPolygon arg0 = KJSEmbed::extractValue<QPolygon>(exec,args, 0);
    object->drawPoints(arg0);
END_OBJECT_METHOD

START_OBJECT_METHOD( calldrawPolygon, QPainter )
    QPolygon arg0 = KJSEmbed::extractValue<QPolygon>(exec,args, 0);
    Qt::FillRule arg1 = (Qt::FillRule)KJSEmbed::extractInt(exec, args, 1,  Qt::OddEvenFill );
    object->drawPolygon(arg0, arg1);
END_OBJECT_METHOD

START_OBJECT_METHOD( calldrawPolyline, QPainter )
    QPolygon arg0 = KJSEmbed::extractValue<QPolygon>(exec,args, 0);
    object->drawPolyline(arg0);
END_OBJECT_METHOD

START_OBJECT_METHOD( calldrawRect, QPainter )
    if (args.size() == 1 )
    {
        QRect arg0 = KJSEmbed::extractValue<QRect>(exec,args, 0);
        object->drawRect(arg0);
    }
    else if ( args.size() == 4 )
    {
        int arg0 = KJSEmbed::extractInt(exec, args, 0);
        int arg1 = KJSEmbed::extractInt(exec, args, 1);
        int arg2 = KJSEmbed::extractInt(exec, args, 2);
        int arg3 = KJSEmbed::extractInt(exec, args, 3);
        object->drawRect(arg0, arg1, arg2, arg3);
    }
END_OBJECT_METHOD

START_OBJECT_METHOD( calldrawRoundRect, QPainter )
    if ( args.size() == 2 )
    {
        QRect arg0 = KJSEmbed::extractValue<QRect>(exec,args, 0);
        int arg1 = KJSEmbed::extractInt(exec, args, 1);
        int arg2 = KJSEmbed::extractInt(exec, args, 2);
        object->drawRoundRect(arg0, arg1, arg2);
    }
    else if ( args.size() == 6 )
    {
        int arg0 = KJSEmbed::extractInt(exec, args, 0);
        int arg1 = KJSEmbed::extractInt(exec, args, 1);
        int arg2 = KJSEmbed::extractInt(exec, args, 2);
        int arg3 = KJSEmbed::extractInt(exec, args, 3);
        int arg4 = KJSEmbed::extractInt(exec, args, 4);
        int arg5 = KJSEmbed::extractInt(exec, args, 5);
        object->drawRoundRect(arg0, arg1, arg2, arg3, arg4, arg5);
    }
END_OBJECT_METHOD

START_OBJECT_METHOD( calldrawText, QPainter )
    if( args.size() == 2 )
    {
        QPoint arg0 = KJSEmbed::extractValue<QPoint>(exec,args, 0);
        QString arg1 = KJSEmbed::extractQString(exec, args, 1);
        object->drawText(arg0, arg1);
    }
    else if ( args.size() == 3 )
    {
        QRect arg0 = KJSEmbed::extractValue<QRect>(exec,args, 0);
        int arg1 = KJSEmbed::extractInt(exec, args, 1);
        QString arg2 = KJSEmbed::extractQString(exec, args, 2);
        QRect* arg3 = 0;
        object->drawText(arg0, arg1, arg2, arg3);
    }
    else if ( args.size () == 6 )
    {
        int arg0 = KJSEmbed::extractInt(exec, args, 0);
        int arg1 = KJSEmbed::extractInt(exec, args, 1);
        int arg2 = KJSEmbed::extractInt(exec, args, 2);
        int arg3 = KJSEmbed::extractInt(exec, args, 3);
        int arg4 = KJSEmbed::extractInt(exec, args, 4);
        QString arg5 = KJSEmbed::extractQString(exec, args, 5);
        QRect* arg6 = 0;
        object->drawText(arg0, arg1, arg2, arg3, arg4, arg5, arg6);
    }
END_OBJECT_METHOD

START_OBJECT_METHOD( calltranslate, QPainter )
    if( args.size() == 2 )
    {
        int arg0 = KJSEmbed::extractInt(exec, args, 0);
        int arg1 = KJSEmbed::extractInt(exec, args, 1);
        object->translate(arg0,arg1);
    }
    else if( args.size() == 1 )
    {
        QPoint arg0 = KJSEmbed::extractValue<QPoint>(exec,args, 0);
        object->translate(arg0);
    }
END_OBJECT_METHOD

START_OBJECT_METHOD( calldrawTiledPixmap, QPainter)
    if( args.size() == 3 )
    {
        QRect arg0 = KJSEmbed::extractValue<QRect>(exec,args, 0);
        QPixmap arg1 = KJSEmbed::extractValue<QPixmap>(exec,args, 1);
        QPoint arg2 = KJSEmbed::extractValue<QPoint>(exec,args, 2);
        object->drawTiledPixmap(arg0,arg1,arg2);
    }
    else if( args.size() == 7)
    {
        int arg0 = KJSEmbed::extractInt(exec, args, 0);
        int arg1 = KJSEmbed::extractInt(exec, args, 1);
        int arg2 = KJSEmbed::extractInt(exec, args, 2);
        int arg3 = KJSEmbed::extractInt(exec, args, 3);
        QPixmap arg4 = KJSEmbed::extractValue<QPixmap>(exec,args, 4);
        int arg5 = KJSEmbed::extractInt(exec, args, 5);
        int arg6 = KJSEmbed::extractInt(exec, args, 6);
        object->drawTiledPixmap(arg0,arg1,arg2,arg3,arg4,arg5,arg6);
    }
END_OBJECT_METHOD

START_OBJECT_METHOD( calleraseRect, QPainter)
    if( args.size() == 4)
    {
        int arg0 = KJSEmbed::extractInt(exec, args, 0);
        int arg1 = KJSEmbed::extractInt(exec, args, 1);
        int arg2 = KJSEmbed::extractInt(exec, args, 2);
        int arg3 = KJSEmbed::extractInt(exec, args, 3);
        object->eraseRect(arg0,arg1,arg2,arg3);
    }
    else if (args.size() == 1 )
    {
        QRect arg0 = KJSEmbed::extractValue<QRect>(exec,args, 0);
        object->eraseRect(arg0);
    }
END_OBJECT_METHOD

START_METHOD_LUT( Painter )
    {"begin", 1, KJS::DontDelete|KJS::ReadOnly, &callPainterBegin },
    {"end", 0, KJS::DontDelete|KJS::ReadOnly, &callPainterEnd },
    {"background", 0, KJS::DontDelete|KJS::ReadOnly, &callbackground},
    {"backgroundMode", 0, KJS::DontDelete|KJS::ReadOnly, &callbackgroundMode},
    {"boundingRect", 6, KJS::DontDelete|KJS::ReadOnly, &callboundingRect},
    {"brush", 0, KJS::DontDelete|KJS::ReadOnly, &callbrush},
    {"brushOrigin", 0, KJS::DontDelete|KJS::ReadOnly, &callbrushOrigin},
    {"drawArc", 6, KJS::DontDelete|KJS::ReadOnly, &calldrawArc},
    {"drawChord", 6, KJS::DontDelete|KJS::ReadOnly, &calldrawChord},
    {"drawConvexPolygon", 1, KJS::DontDelete|KJS::ReadOnly, &calldrawConvexPolygon},
    {"drawEllipse", 3, KJS::DontDelete|KJS::ReadOnly, &calldrawEllipse},
    {"drawImage", 7, KJS::DontDelete|KJS::ReadOnly, &calldrawImage},
    {"drawLine", 3, KJS::DontDelete|KJS::ReadOnly, &calldrawLine},
    //{drawLines", 1, KJS::DontDelete|KJS::ReadOnly, &calldrawLines},
    //{"drawPath", 0, KJS::DontDelete|KJS::ReadOnly, &calldrawPath},
    //{"drawPicture", 2, KJS::DontDelete|KJS::ReadOnly, &calldrawPicture},
    {"drawPie", 6, KJS::DontDelete|KJS::ReadOnly, &calldrawPie},
    {"drawPixmap", 8, KJS::DontDelete|KJS::ReadOnly, &calldrawPixmap},
    {"drawPoint", 2, KJS::DontDelete|KJS::ReadOnly, &calldrawPoint},
    {"drawPoints", 1, KJS::DontDelete|KJS::ReadOnly, &calldrawPoints},
    {"drawPolygon", 2, KJS::DontDelete|KJS::ReadOnly, &calldrawPolygon},
    {"drawPolyline", 1, KJS::DontDelete|KJS::ReadOnly, &calldrawPolyline},
    {"drawRect", 4, KJS::DontDelete|KJS::ReadOnly, &calldrawRect},
    //{"drawRects", 0, KJS::DontDelete|KJS::ReadOnly, &calldrawRects},
    {"drawRoundRect", 5, KJS::DontDelete|KJS::ReadOnly, &calldrawRoundRect},
    {"drawText", 7, KJS::DontDelete|KJS::ReadOnly, &calldrawText},
    {"drawTiledPixmap", 3, KJS::DontDelete|KJS::ReadOnly, &calldrawTiledPixmap},
    {"eraseRect", 1, KJS::DontDelete|KJS::ReadOnly, &calleraseRect},
    //{"fillPath", 1, KJS::DontDelete|KJS::ReadOnly, &callfillPath},
    //{"fillRect", 4, KJS::DontDelete|KJS::ReadOnly, &callfillRect},
    //{"font", 0, KJS::DontDelete|KJS::ReadOnly, &callfont},
    //{"hasClipping", 0, KJS::DontDelete|KJS::ReadOnly, &callhasClipping},
    //{"isActive", 0, KJS::DontDelete|KJS::ReadOnly, &callisActive},
    //{"pen", 0, KJS::DontDelete|KJS::ReadOnly, &callpen},
    //{"renderHints", 0, KJS::DontDelete|KJS::ReadOnly, &callrenderHints},
    //{"restore", 0, KJS::DontDelete|KJS::ReadOnly, &callrestore},
    //{"rotate", 0, KJS::DontDelete|KJS::ReadOnly, &callrotate},
    //{"save", 0, KJS::DontDelete|KJS::ReadOnly, &callsave},
    //{"scale", 1, KJS::DontDelete|KJS::ReadOnly, &callscale},
    //{"setBackground", 0, KJS::DontDelete|KJS::ReadOnly, &callsetBackground},
    //{"setBackgroundColor", 0, KJS::DontDelete|KJS::ReadOnly, &callsetBackgroundColor},
    //{"setBackgroundMode", 0, KJS::DontDelete|KJS::ReadOnly, &callsetBackgroundMode},
    //{"setBrush", 0, KJS::DontDelete|KJS::ReadOnly, &callsetBrush},
    //{"setBrushOrigin", 1, KJS::DontDelete|KJS::ReadOnly, &callsetBrushOrigin},
    //{"setClipPath", 1, KJS::DontDelete|KJS::ReadOnly, &callsetClipPath},
    //{"setClipRect", 4, KJS::DontDelete|KJS::ReadOnly, &callsetClipRect},
    //{"setClipRegion", 1, KJS::DontDelete|KJS::ReadOnly, &callsetClipRegion},
    //{"setClipping", 0, KJS::DontDelete|KJS::ReadOnly, &callsetClipping},
    //{"setFont", 1, KJS::DontDelete|KJS::ReadOnly, &callsetFont},
    //{"setPen", 1, KJS::DontDelete|KJS::ReadOnly, &callsetPen},
    //{"setRenderHint", 1, KJS::DontDelete|KJS::ReadOnly, &callsetRenderHint},
    //{"shear", 2, KJS::DontDelete|KJS::ReadOnly, &callshear},
    //{"strokePath", 1, KJS::DontDelete|KJS::ReadOnly, &callstrokePath},
    {"translate", 1, KJS::DontDelete|KJS::ReadOnly, &calltranslate}
END_METHOD_LUT

NO_ENUMS( Painter )
NO_STATICS( Painter )

START_CTOR( Painter, QPainter, 0 )
    KJS::JSObject *object;

    if( args.size() == 1 )
    {
        QPaintDevice *device = extractPaintDevice(exec, args[0]);
        if ( device  )
        {
            object = new KJSEmbed::ObjectBinding(exec, "Painter", new QPainter(device) );
        }
        else
        {
            KJS::throwError( exec, KJS::GeneralError, QString("Cannot paint to object %1").arg(args[0]->toString(exec).qstring()) );
            // KJSEmbed::throwError( exec, QString("Cannot paint to object %1").arg(args[0].toString(exec).qstring()) );
        }
    }
    else
    {
            object = new KJSEmbed::ObjectBinding(exec, "Painter", new QPainter() );
    }

    StaticBinding::publish( exec, object, ObjectFactory::methods() );
    StaticBinding::publish( exec, object, Painter::methods() );
    return object;
END_CTOR
