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
#include "svg_binding.h"

#include <QtSvg/QSvgWidget>
#include <QtCore/QDebug>

#include <kjs/object.h>

#include "qwidget_binding.h"
#include "static_binding.h"
#include "kjseglobal.h"

using namespace KJSEmbed;

namespace SvgRendererNS {
START_QOBJECT_METHOD( animationDuration, QSvgRenderer )
    result = KJS::jsNumber(object->animationDuration());
END_QOBJECT_METHOD

START_QOBJECT_METHOD( defaultSize, QSvgRenderer )
    result = KJSEmbed::createVariant(exec, "QSize", object->defaultSize() );
END_QOBJECT_METHOD

START_QOBJECT_METHOD( isValid, QSvgRenderer )
    result = KJS::jsBoolean(object->isValid());
END_QOBJECT_METHOD

START_QOBJECT_METHOD( animated, QSvgRenderer )
    result = KJS::jsBoolean(object->animated());
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

KJSO_SIMPLE_BINDING_CTOR( SvgRenderer, QSvgRenderer, QObjectBinding )
KJSO_QOBJECT_BIND( SvgRenderer, QSvgRenderer )

KJSO_START_CTOR( SvgRenderer, QSvgRenderer, 0 )
    QSvgRenderer *renderer = 0;
    if( args.size() == 1 )
    {
        QObject *arg0 = KJSEmbed::extractObject<QObject>(exec, args, 0, 0);
        renderer = new QSvgRenderer(arg0);
    }
    else if( args.size() == 2 )
    {
        QString arg0 = KJSEmbed::extractVariant<QString>(exec, args, 0 );
        QObject *arg1 = KJSEmbed::extractObject<QObject>(exec, args, 1, 0);
        renderer = new QSvgRenderer(arg0,arg1);
    }
    else
    {
        renderer = new QSvgRenderer();
    }

    KJS::JSObject *rendererObject = new SvgRenderer( exec, renderer );
    return rendererObject;
KJSO_END_CTOR

namespace SvgWidgetNS
{
START_QOBJECT_METHOD( renderer, QSvgWidget )
    result = KJSEmbed::createQObject( exec, object->renderer(), ObjectBinding::QObjOwned );
END_QOBJECT_METHOD
}

START_METHOD_LUT( SvgWidget )
    {"renderer", 0, KJS::DontDelete|KJS::ReadOnly, &SvgWidgetNS::renderer}
END_METHOD_LUT

NO_ENUMS( SvgWidget )
NO_STATICS( SvgWidget )

KJSO_SIMPLE_BINDING_CTOR( SvgWidget, QSvgWidget, QWidgetBinding )
KJSO_QOBJECT_BIND( SvgWidget, QSvgWidget )

KJSO_START_CTOR( SvgWidget, QSvgWidget, 0 )
    QSvgWidget *widget = 0;
    if( args.size() == 1 )
    {
        QWidget *arg0 = KJSEmbed::extractObject<QWidget>(exec, args, 0, 0);
        widget = new QSvgWidget(arg0);
    }
    else if( args.size() == 2 )
    {
        QString arg0 = KJSEmbed::extractVariant<QString>(exec, args, 0 );
        QWidget *arg1 = KJSEmbed::extractObject<QWidget>(exec, args, 1, 0);
        widget = new QSvgWidget(arg0,arg1);
    }
    else
    {
        widget = new QSvgWidget();
    }

    KJS::JSObject *rendererObject = new SvgWidget( exec, widget );
    return rendererObject;
KJSO_END_CTOR

//kate: indent-spaces on; indent-width 4; replace-tabs on; indent-mode cstyle;
