/******************************************************************************
*   Copyright 2013 Sebastian Kügler <sebas@kde.org>                           *
*                                                                             *
*   This library is free software; you can redistribute it and/or             *
*   modify it under the terms of the GNU Library General Public               *
*   License as published by the Free Software Foundation; either              *
*   version 2 of the License, or (at your option) any later version.          *
*                                                                             *
*   This library is distributed in the hope that it will be useful,           *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of            *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU          *
*   Library General Public License for more details.                          *
*                                                                             *
*   You should have received a copy of the GNU Library General Public License *
*   along with this library; see the file COPYING.LIB.  If not, write to      *
*   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,      *
*   Boston, MA 02110-1301, USA.                                               *
*******************************************************************************/

#include "pluginlocatortest.h"

#include <qtest.h>
#include <QDebug>

#include <kplugininfo.h>
#include <kplugintrader.h>

#include <QJsonObject>

QTEST_MAIN(PluginTest)

void PluginTest::findPlugin()
{
    const QString pluginName("fakeplugin");
    const QString serviceType("KService/NSA");
    const QString category("Examples");

    const QString constraint = QString("[X-KDE-PluginInfo-Name] == '%1'").arg(pluginName);
    KPluginInfo::List res = KPluginTrader::self()->query(serviceType, constraint);
    QVERIFY(res.count() > 0);
}


#include "moc_pluginlocatortest.cpp"

