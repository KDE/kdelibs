/*  This file is part of the KDE project
    Copyright (C) 2007 Matthias Kretz <kretz@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

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
#include <QtGlobal>
#include <kcomponentdata.h>
#include <ksharedconfig.h>
#include <kconfiggroup.h>
#include <kglobal.h>

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

    {
        KComponentData componentData("globalcleanuptest");
        // letting componentData go out of scope here simulates the behaviour of KCmdLineArgs
    }
    KSharedConfigPtr cfg = KGlobal::config();
    KConfigGroup group = cfg->group("test");
    group.writeEntry("test", 1);

    KComponentData componentData2("globalcleanuptest2");
    cfg = componentData2.config();
    group = cfg->group("test");
    group.writeEntry("test", 1);

    qrand(); // make sure the global static that kills us is accessed
}
