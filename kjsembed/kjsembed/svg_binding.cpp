#include <QtSvg/QSvgRenderer>
#include <QtSvg/QSvgWidget>
#include <QDebug>

#include <kjs/object.h>

#include "svg_binding.h"
#include "qwidget_binding.h"
#include "static_binding.h"
#include "global.h"

using namespace KJSEmbed;

namespace SvgRendererNS {
START_QOBJECT_METHOD( animationDuration, QSvgRenderer )
    result = KJS::Number(object->animationDuration());
END_QOBJECT_METHOD

START_QOBJECT_METHOD( defaultSize, QSvgRenderer )
    result = KJSEmbed::createValue(exec, "QSize", object->defaultSize() );
END_QOBJECT_METHOD

START_QOBJECT_METHOD( isValid, QSvgRenderer )
    result = KJS::Boolean(object->isValid());
END_QOBJECT_METHOD

START_QOBJECT_METHOD( animated, QSvgRenderer )
    result = KJS::Boolean(object->animated());
END_QOBJECT_METHOD
}

START_METHOD_LUT( SvgRenderer )
    {"animationDuration", 0, KJS::DontDelete|KJS::ReadOnly, &SvgRendererNS::animationDuration},
    {"defaultSize", 0, KJS::DontDelete|KJS::ReadOnly, &SvgRendererNS::defaultSize},
    {"isValid", 0, KJS::DontDelete|KJS::ReadOnly, &SvgRendererNS::isValid},
    {"animated", 0, KJS::DontDelete|KJS::ReadOnly, &SvgRendererNS::animated}
END_METHOD_LUT

NO_ENUMS( SvgRenderer )
NO_STATICS( SvgRenderer )

START_CTOR( SvgRenderer, QSvgRenderer, 0 )
    QSvgRenderer *renderer = 0;
    if( args.size() == 1 )
    {
        QObject *arg0 = KJSEmbed::extractObject<QObject>(exec, args, 0, 0);
        renderer = new QSvgRenderer(arg0);
    }
    else if( args.size() == 2 )
    {
        QString arg0 = KJSEmbed::extractValue<QString>(exec, args, 0 );
        QObject *arg1 = KJSEmbed::extractObject<QObject>(exec, args, 1, 0);
        renderer = new QSvgRenderer(arg0,arg1);
    }
    else
    {
        renderer = new QSvgRenderer();
    }

    KJS::JSObject *rendererObject = new QObjectBinding( exec, renderer );
    StaticBinding::publish( exec, rendererObject, SvgRenderer::methods() );
    return rendererObject;
END_CTOR

namespace SvgWidgetNS
{
START_QOBJECT_METHOD( renderer, QSvgWidget )
    result = KJSEmbed::createQObject( exec, object->renderer() );
END_QOBJECT_METHOD
}

START_METHOD_LUT( SvgWidget )
    {"renderer", 0, KJS::DontDelete|KJS::ReadOnly, &SvgWidgetNS::renderer}
END_METHOD_LUT

NO_ENUMS( SvgWidget )
NO_STATICS( SvgWidget )

START_CTOR( SvgWidget, QSvgWidget, 0 )
    QSvgWidget *widget = 0;
    if( args.size() == 1 )
    {
        QWidget *arg0 = KJSEmbed::extractObject<QWidget>(exec, args, 0, 0);
        widget = new QSvgWidget(arg0);
    }
    else if( args.size() == 2 )
    {
        QString arg0 = KJSEmbed::extractValue<QString>(exec, args, 0 );
        QWidget *arg1 = KJSEmbed::extractObject<QWidget>(exec, args, 1, 0);
        widget = new QSvgWidget(arg0,arg1);
    }
    else
    {
        widget = new QSvgWidget();
    }

    KJS::JSObject *rendererObject = new QObjectBinding( exec, widget );
    StaticBinding::publish( exec, rendererObject, Widget::methods() );
    StaticBinding::publish( exec, rendererObject, SvgWidget::methods() );
    return rendererObject;
END_CTOR
