/* This file is part of the KDE libraries
    Copyright (C) 2005, 2006 KJSEmbed Authors
    See included AUTHORS file.

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
#include <QDebug>

#include <kjsembed/object_binding.h>
#include <kjsembed/value_binding.h>
#include <kjsembed/qobject_binding.h>
#include <kjs/object.h>

#include "bind_qworkspace.h"

using namespace KJSEmbed;

namespace WorkspaceNS {

START_QOBJECT_METHOD( activeWindow, QWorkspace )
    QWidget *w = object->activeWindow();
    qDebug() << "Workspace active window is " << w;

    result = KJSEmbed::createQObject( exec, w );

    qDebug() << "Elvis has left the building";

END_QOBJECT_METHOD


START_QOBJECT_METHOD( addWindow, QWorkspace )
    QWidget *arg0 = KJSEmbed::extractObject<QWidget>( exec, args, 0 );
    int arg1 = KJSEmbed::extractInt( exec, args, 1 );

    QWidget *w = object->addWindow( arg0, (Qt::WFlags) arg1 );
    result = KJSEmbed::createQObject( exec, w );
END_QOBJECT_METHOD


};

NO_ENUMS( Workspace )

START_METHOD_LUT( Workspace )
        {"activeWindow", 0, KJS::DontDelete|KJS::ReadOnly, &WorkspaceNS::activeWindow },
        {"addWindow", 1, KJS::DontDelete|KJS::ReadOnly, &WorkspaceNS::addWindow }
END_METHOD_LUT

NO_STATICS( Workspace )

START_CTOR( Workspace, QWorkspace, 0 )
    QWorkspace *parent = KJSEmbed::extractObject<QWorkspace>( exec, args, 0, 0 );
    QWorkspace *a = new QWorkspace( parent );
    KJSEmbed::QObjectBinding *obj = new KJSEmbed::QObjectBinding( exec, a );

    StaticBinding::publish( exec, obj, Workspace::methods() );
    return obj;
END_CTOR


