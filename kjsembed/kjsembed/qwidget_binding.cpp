#include "qwidget_binding.h"
#include "static_binding.h"
#include "global.h"

#include <kjs/object.h>
#include <qdebug.h>

#include <QWidget>
#include <QAction>
#include <QLayout>

#include "widgetfactory.h"
#include <QFile>

namespace KJSEmbed
{
    static WidgetFactory *gWidgetFactory = 0L;

    WidgetFactory *widgetFactory()
    {
        if( gWidgetFactory == 0 )
            gWidgetFactory = new WidgetFactory;
        return gWidgetFactory;
    }
}


using namespace KJSEmbed;

namespace WidgetNS
{

START_QOBJECT_METHOD( adjustSize, QWidget )
    object->adjustSize();
END_QOBJECT_METHOD
START_QOBJECT_METHOD( grabMouse, QWidget )
    object->grabMouse();
END_QOBJECT_METHOD
START_QOBJECT_METHOD( grabKeyboard, QWidget )
    object->grabKeyboard();
END_QOBJECT_METHOD
START_QOBJECT_METHOD( releaseMouse, QWidget )
    object->releaseMouse();
END_QOBJECT_METHOD
START_QOBJECT_METHOD( releaseKeyboard, QWidget )
    object->releaseKeyboard();
END_QOBJECT_METHOD
START_QOBJECT_METHOD( childAt, QWidget )
    QPoint pt = KJSEmbed::extractValue<QPoint>(exec, args, 0);
    int x,y;
    QWidget *child = 0;
    if( pt.isNull())
    {
        x = KJSEmbed::extractValue<int>(exec, args, 0);
        y = KJSEmbed::extractValue<int>(exec, args, 1);
        child = object->childAt(x,y);
    }
    else
    {
        child = object->childAt(pt);
    }
    result = KJSEmbed::createQObject(exec, child);
END_QOBJECT_METHOD
START_QOBJECT_METHOD( focusWidget, QWidget )
    result = KJSEmbed::createQObject(exec, object->focusWidget() );
END_QOBJECT_METHOD
START_QOBJECT_METHOD( heightForWidth, QWidget )
    int width = KJSEmbed::extractValue<int>(exec, args, 0);
    result = KJS::Number( object->heightForWidth(width));
END_QOBJECT_METHOD
START_QOBJECT_METHOD( mapFrom, QWidget )
    QWidget *w = KJSEmbed::extractObject<QWidget>(exec, args, 0, 0);
    QPoint pt = KJSEmbed::extractValue<QPoint>(exec, args, 1);
    result = KJSEmbed::createValue(exec, "QPoint", object->mapFrom(w, pt));
END_QOBJECT_METHOD
START_QOBJECT_METHOD( mapFromGlobal, QWidget )
    QPoint pt = KJSEmbed::extractValue<QPoint>(exec, args, 0);
    result = KJSEmbed::createValue(exec, "QPoint", object->mapFromGlobal(pt));
END_QOBJECT_METHOD
START_QOBJECT_METHOD( mapFromParent, QWidget )
    QPoint pt = KJSEmbed::extractValue<QPoint>(exec, args, 0);
    result = KJSEmbed::createValue(exec, "QPoint", object->mapFromParent(pt));
END_QOBJECT_METHOD
START_QOBJECT_METHOD( mapTo, QWidget )
    QWidget *w = KJSEmbed::extractObject<QWidget>(exec, args, 0, 0);
    QPoint pt = KJSEmbed::extractValue<QPoint>(exec, args, 1);
    result = KJSEmbed::createValue(exec, "QPoint", object->mapTo(w, pt));
END_QOBJECT_METHOD
START_QOBJECT_METHOD( mapToParent, QWidget )
    QPoint pt = KJSEmbed::extractValue<QPoint>(exec, args, 0);
    result = KJSEmbed::createValue(exec, "QPoint", object->mapToParent(pt));
END_QOBJECT_METHOD
START_QOBJECT_METHOD( mapToGlobal, QWidget )
    QPoint pt = KJSEmbed::extractValue<QPoint>(exec, args, 0);
    result = KJSEmbed::createValue(exec, "QPoint", object->mapToGlobal(pt));
END_QOBJECT_METHOD

}

START_METHOD_LUT( Widget )
    {"adjustSize", 0, KJS::DontDelete|KJS::ReadOnly, &WidgetNS::adjustSize},
    {"grabMouse", 0, KJS::DontDelete|KJS::ReadOnly, &WidgetNS::grabMouse},
    {"grabKeyboard", 0, KJS::DontDelete|KJS::ReadOnly, &WidgetNS::grabKeyboard},
    {"releaseMouse", 0, KJS::DontDelete|KJS::ReadOnly, &WidgetNS::releaseMouse},
    {"releaseKeyboard", 0, KJS::DontDelete|KJS::ReadOnly, &WidgetNS::releaseKeyboard},
    {"childAt", 1, KJS::DontDelete|KJS::ReadOnly, &WidgetNS::childAt},
    {"focusWidget", 0, KJS::DontDelete|KJS::ReadOnly, &WidgetNS::focusWidget},
    {"heightForWidth", 1, KJS::DontDelete|KJS::ReadOnly, &WidgetNS::heightForWidth},
    {"mapFrom", 2, KJS::DontDelete|KJS::ReadOnly, &WidgetNS::mapFrom},
    {"mapFromGlobal", 1, KJS::DontDelete|KJS::ReadOnly, &WidgetNS::mapFromGlobal},
    {"mapFromParent", 1, KJS::DontDelete|KJS::ReadOnly, &WidgetNS::mapFromParent},
    {"mapTo", 2, KJS::DontDelete|KJS::ReadOnly, &WidgetNS::mapTo},
    {"mapToParent", 1, KJS::DontDelete|KJS::ReadOnly, &WidgetNS::mapToParent},
    {"mapToGlobal", 1, KJS::DontDelete|KJS::ReadOnly, &WidgetNS::mapToGlobal}
END_METHOD_LUT

NO_ENUMS( Widget )
NO_STATICS( Widget )

START_CTOR( Widget, Widget, 0 )
    if( args.size() > 0 )
    {
        QString widgetName = args[0]->toString(exec).qstring();
        QWidget *parentWidget = 0;
        KJSEmbed::QObjectBinding *parentImp = KJSEmbed::extractBindingImp<KJSEmbed::QObjectBinding>(exec, args[1] );
        if( parentImp )
        {
            parentWidget = parentImp->object<QWidget>();
        }

        QWidget *widget = widgetFactory()->widget(widgetName, parentWidget, "QWidget");
        if( widget )
        {
            KJS::JSObject *widgetObject = KJSEmbed::createQObject(exec, widget);
            StaticBinding::publish( exec, widgetObject, Widget::methods() );
            return widgetObject;
        }
        return KJS::throwError(exec, KJS::GeneralError, i18n("'%1' is not a valid QWidget.").arg(widgetName));
        // return KJSEmbed::throwError(exec, i18n("'%1' is not a valid QWidget.").arg(widgetName));
    }
    // Trow error incorrect args
    return KJS::throwError(exec, KJS::GeneralError, i18n("Must supply a widget name."));
    // return KJSEmbed::throwError(exec, i18n("Must supply a widget name."));
END_CTOR

namespace LayoutNS
{
START_QOBJECT_METHOD( addWidget, QLayout )
    QWidget *w = KJSEmbed::extractObject<QWidget>(exec, args, 0, 0);
    object->addWidget(w);
END_QOBJECT_METHOD
START_QOBJECT_METHOD( removeWidget, QLayout )
    QWidget *w = KJSEmbed::extractObject<QWidget>(exec, args, 0, 0);
    object->removeWidget(w);
END_QOBJECT_METHOD
START_QOBJECT_METHOD( parentWidget, QLayout )
    QWidget *w = object->parentWidget();
    result = KJSEmbed::createQObject(exec,w);
END_QOBJECT_METHOD

}
START_METHOD_LUT( Layout )
    {"addWidget", 1, KJS::DontDelete|KJS::ReadOnly, &LayoutNS::addWidget},
    {"removeWidget", 1, KJS::DontDelete|KJS::ReadOnly, &LayoutNS::removeWidget},
    {"parentWidget", 0, KJS::DontDelete|KJS::ReadOnly, &LayoutNS::parentWidget}
END_METHOD_LUT

NO_ENUMS( Layout )
NO_STATICS( Layout )

START_CTOR( Layout, Layout, 0 )
    if( args.size() > 0 )
    {
        QString layoutName = args[0]->toString(exec).qstring();
        QObject *parentObject = 0;
        KJSEmbed::QObjectBinding *parentImp = KJSEmbed::extractBindingImp<KJSEmbed::QObjectBinding>(exec, args[1] );
        if( parentImp )
        {
            parentObject = parentImp->object<QObject>();
        }

        QLayout *layout = widgetFactory()->layout(layoutName, parentObject, "QLayout");
        if( layout )
        {
            KJS::JSObject *layoutObject = KJSEmbed::createQObject(exec, layout);
            StaticBinding::publish( exec, layoutObject, Layout::methods() );
            return layoutObject;
        }
        return KJS::throwError(exec, KJS::GeneralError, i18n("'%1' is not a valid QLayout.").arg(layoutName));
        // return KJSEmbed::throwError(exec, i18n("'%1' is not a valid QLayout.").arg(layoutName));
    }
    // Trow error incorrect args
    return KJS::throwError(exec, KJS::GeneralError, i18n("Must supply a layout name."));
    // return KJSEmbed::throwError(exec, i18n("Must supply a layout name."));
END_CTOR

NO_METHODS( Action )
NO_ENUMS( Action )
NO_STATICS( Action )

START_CTOR( Action, Action, 0 )
    if( args.size() == 2 )
    {
        QObject *parent = KJSEmbed::extractObject<QObject>(exec, args, 0, 0);
        QString actionName = KJSEmbed::extractQString(exec, args, 1);

        QAction *action = widgetFactory()->action(parent, actionName);
        if( action )
        {
            KJS::JSObject *actionObject = KJSEmbed::createQObject( exec, action );
            StaticBinding::publish( exec, actionObject, Action::methods() );
            return actionObject;
        }
        else
        {
            return KJS::throwError(exec, KJS::GeneralError, i18n("Action takes 2 args."));
            // return KJSEmbed::throwError(exec, i18n("Action takes 2 args."));
        }
    }
    // Trow error incorrect args
    return KJS::throwError(exec, KJS::GeneralError, i18n("Must supply a valid parent."));
    // return KJSEmbed::throwError(exec, i18n("Must supply a valid parent."));
END_CTOR
