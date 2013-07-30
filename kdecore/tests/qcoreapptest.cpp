/*  This file is part of the KDE project
    Copyright (C) 2010 David Faure <faure@kde.org>

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

#include <QtCore/QCoreApplication>
#include <QtCore/QtGlobal>
#include <kcomponentdata.h>
#include <klocalizedstring.h>
#include <kconfiggroup.h>

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

    // Test various kdecore features that Qt-only programs can end up using.
    // E.g. when they open a KDE dialog via KQGuiPlatformPlugin.

    // Test that KLocale works
    const QString ok = i18n("OK");
    Q_UNUSED(ok);

    // Test that KConfig works
    KConfig foo("foorc");
    foo.group("group").writeEntry("test", "val");

    return 0;
}

