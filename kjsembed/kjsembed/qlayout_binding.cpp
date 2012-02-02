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
#include "qlayout_binding.h"
#include "static_binding.h"
#include "kjseglobal.h"

#include <kjs/object.h>
#include <QtCore/QDebug>

#include <QWidget>
#include <QAction>
#include <QLayout>
#include <QtUiTools/QUiLoader>

namespace KJSEmbed
{
  QUiLoader *uiLoader();
}


using namespace KJSEmbed;

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

KJSO_SIMPLE_BINDING_CTOR( Layout, QLayout, QObjectBinding )
KJSO_QOBJECT_BIND( Layout, QLayout )

KJSO_START_CTOR( Layout, QLayout, 0 )
//  qDebug("Layout::CTOR(): args.size()=%d", args.size());

    if( args.size() > 0 )
    {
        QString layoutName = toQString(args[0]->toString(exec));
        QObject *parentObject = 0;
        KJSEmbed::QObjectBinding *parentImp = KJSEmbed::extractBindingImp<KJSEmbed::QObjectBinding>(exec, args[1] );
        if( parentImp )
        {
            parentObject = parentImp->object<QObject>();
        }

        QLayout *layout = uiLoader()->createLayout(layoutName, parentObject, "QLayout");
        if( layout )
        {
          KJS::JSObject *layoutObject = new Layout(exec, layout);//KJSEmbed::createQObject(exec, layout);
            return layoutObject;
        }
        return KJS::throwError(exec, KJS::GeneralError, i18n("'%1' is not a valid QLayout.",
                                                             layoutName));
        // return KJSEmbed::throwError(exec, i18n("'%1' is not a valid QLayout.").arg(layoutName));
    }

    // should Trow error incorrect args
    return KJS::throwError(exec, KJS::GeneralError, i18n("Must supply a layout name."));
    // return KJSEmbed::throwError(exec, i18n("Must supply a layout name."));
KJSO_END_CTOR

//kate: indent-spaces on; indent-width 4; replace-tabs on; indent-mode cstyle;
