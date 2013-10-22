/*
 *   Copyright (C) 2013 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "solidextensionplugin.h"

#include <QtQml>

#include "devices.h"
#include "solid/deviceinterface.h"

SolidExtensionPlugin::SolidExtensionPlugin(QObject *parent)
    : QQmlExtensionPlugin(parent)
{
}

void SolidExtensionPlugin::registerTypes(const char *uri)
{
    using namespace Solid;

    Q_ASSERT(QLatin1String(uri) == QLatin1String("org.kde.solid"));

    qmlRegisterType<Devices> (uri, 1, 0, "Devices");
}


