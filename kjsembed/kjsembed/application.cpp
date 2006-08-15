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
#include "application.h"

#include <QDebug>
#include <QApplication>
#include <QStringList>
using namespace KJSEmbed;

CoreApplicationBinding::CoreApplicationBinding( KJS::ExecState *exec, QCoreApplication *app )
    : QObjectBinding(exec, app )
{
    StaticBinding::publish( exec, this, CoreApplication::methods() );
    setOwnership(CPPOwned);
}

namespace CoreApplicationNS
{

START_STATIC_OBJECT_METHOD( callExit )
    int exitCode = KJSEmbed::extractInt( exec, args, 0 );
    QCoreApplication::exit(exitCode);
END_STATIC_OBJECT_METHOD

}

START_STATIC_METHOD_LUT( CoreApplication )
    {"exit", 0, KJS::DontDelete|KJS::ReadOnly, &CoreApplicationNS::callExit}
END_METHOD_LUT

NO_ENUMS( CoreApplication )
NO_METHODS( CoreApplication )

START_CTOR( CoreApplication, QCoreApplication, 0)
    return new KJSEmbed::CoreApplicationBinding( exec, QCoreApplication::instance () );
END_CTOR

//kate: indent-spaces on; indent-width 4; replace-tabs on; indent-mode cstyle;
