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
#include <kjs/object.h>

#include "bind_qaction.h"

using namespace KJSEmbed;

namespace ActionNS {
};

NO_ENUMS( Action )
NO_METHODS( Action )
NO_STATICS( Action )

START_CTOR( Action, QAction, 1 )
    QObject *parent = KJSEmbed::extractObject<QObject>( exec, args, 0 );
    QAction *a = new QAction( parent );
    return new KJSEmbed::QObjectBinding( exec, a );
END_CTOR


