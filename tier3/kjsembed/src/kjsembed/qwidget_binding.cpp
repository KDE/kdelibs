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
#include "qwidget_binding.h"
#include "static_binding.h"
#include "qpainter_binding.h"
#include "kjseglobal.h"

#include <kjs/object.h>
#include <QtCore/QDebug>

#include <QWidget>
#include <QAction>
#include <QLayout>
#include <QtUiTools/QUiLoader>

#include <QtCore/QFile>

namespace KJSEmbed
{
    static QUiLoader *gUiLoader = 0L;

    QUiLoader *uiLoader()
    {
        if( gUiLoader == 0 )
	  gUiLoader = new QUiLoader();
        return gUiLoader;
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
    QPoint pt = KJSEmbed::extractVariant<QPoint>(exec, args, 0);
    int x,y;
    QWidget *child = 0;
    if( pt.isNull())
    {
        x = KJSEmbed::extractVariant<int>(exec, args, 0);
        y = KJSEmbed::extractVariant<int>(exec, args, 1);
        child = object->childAt(x,y);
    }
    else
    {
        child = object->childAt(pt);
    }
    result = new QWidgetBinding(exec, child);
END_QOBJECT_METHOD
START_QOBJECT_METHOD( focusWidget, QWidget )
    result = KJSEmbed::createQObject(exec, object->focusWidget() );
END_QOBJECT_METHOD
START_QOBJECT_METHOD( heightForWidth, QWidget )
//	  qDebug() << "heightForWidth() object=" << object << " imp=" << imp;
    int width = KJSEmbed::extractInt(exec, args, 0);
    result = KJS::jsNumber( object->heightForWidth(width));
END_QOBJECT_METHOD
START_QOBJECT_METHOD( mapFrom, QWidget )
    QWidget *w = KJSEmbed::extractObject<QWidget>(exec, args, 0, 0);
    QPoint pt = KJSEmbed::extractVariant<QPoint>(exec, args, 1);
    result = KJSEmbed::createVariant(exec, "QPoint", object->mapFrom(w, pt));
END_QOBJECT_METHOD
START_QOBJECT_METHOD( mapFromGlobal, QWidget )
    QPoint pt = KJSEmbed::extractVariant<QPoint>(exec, args, 0);
    result = KJSEmbed::createVariant(exec, "QPoint", object->mapFromGlobal(pt));
END_QOBJECT_METHOD
START_QOBJECT_METHOD( mapFromParent, QWidget )
    QPoint pt = KJSEmbed::extractVariant<QPoint>(exec, args, 0);
    result = KJSEmbed::createVariant(exec, "QPoint", object->mapFromParent(pt));
END_QOBJECT_METHOD
START_QOBJECT_METHOD( mapTo, QWidget )
    QWidget *w = KJSEmbed::extractObject<QWidget>(exec, args, 0, 0);
    QPoint pt = KJSEmbed::extractVariant<QPoint>(exec, args, 1);
    result = KJSEmbed::createVariant(exec, "QPoint", object->mapTo(w, pt));
END_QOBJECT_METHOD
START_QOBJECT_METHOD( mapToParent, QWidget )
    QPoint pt = KJSEmbed::extractVariant<QPoint>(exec, args, 0);
    result = KJSEmbed::createVariant(exec, "QPoint", object->mapToParent(pt));
END_QOBJECT_METHOD
START_QOBJECT_METHOD( mapToGlobal, QWidget )
    QPoint pt = KJSEmbed::extractVariant<QPoint>(exec, args, 0);
    result = KJSEmbed::createVariant(exec, "QPoint", object->mapToGlobal(pt));
END_QOBJECT_METHOD

}

START_METHOD_LUT( QWidgetBinding )
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

NO_ENUMS( QWidgetBinding )
NO_STATICS( QWidgetBinding )

KJSO_SIMPLE_BINDING_CTOR( QWidgetBinding, QWidget, QObjectBinding )
KJSO_QOBJECT_BIND(QWidgetBinding, QWidget)

KJSO_START_CTOR( QWidgetBinding, QWidget, 0 )
    //qDebug() << "QWidgetBinding::CTOR args.size()=" << args.size();
    if( args.size() > 0 )
    {
        QString widgetName = toQString(args[0]->toString(exec));
        QWidget *parentWidget = 0;
        KJSEmbed::QObjectBinding *parentImp = KJSEmbed::extractBindingImp<KJSEmbed::QObjectBinding>(exec, args[1] );
        if( parentImp )
        {
            parentWidget = parentImp->object<QWidget>();
        }

        QWidget* widget = uiLoader()->createWidget(widgetName, parentWidget, "QWidget");
        if( widget )
            return new QWidgetBinding(exec, widget);

        return KJS::throwError(exec, KJS::TypeError, toUString(i18n("'%1' is not a valid QWidget.", widgetName)));
    }
    else
    {
        QWidget* widget = new QWidget();
        if (widget)
            return new QWidgetBinding(exec, widget);
    }

    return KJS::throwError(exec, KJS::GeneralError, toUString(i18n("Must supply a widget name.")));
END_CTOR

//kate: indent-spaces on; indent-width 4; replace-tabs on; indent-mode cstyle;
