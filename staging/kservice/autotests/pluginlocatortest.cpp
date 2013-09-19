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
#include "nsaplugin.h"

#include <QtTest>
#include <QDebug>

#include <kplugininfo.h>
#include <kplugintrader.h>

#include <QJsonObject>

QTEST_MAIN(PluginTest)

void PluginTest::findPlugin_data()
{
    QTest::addColumn<QString>("serviceType");
    QTest::addColumn<QString>("constraint");
    QTest::addColumn<int>("expectedResult");

    const QString _st = "KService/NSA";
    QString _c = "";
    QTest::newRow("no constraints") << _st << _c << 1;

    _c = QString("[X-KDE-PluginInfo-Name] == '%1'").arg("fakeplugin");
    QTest::newRow("by pluginname") << _st << _c << 1;

    _c = QString("[X-KDE-PluginInfo-Category] == '%1'").arg("Examples");
    QTest::newRow("by category") << _st << _c << 1;

    _c = QString("([X-KDE-PluginInfo-Category] == 'Examples') AND ([X-KDE-PluginInfo-Email] == 'sebas@kde.org')");
    QTest::newRow("complex query") << _st << _c << 1;

    _c = QString("([X-KDE-PluginInfo-Category] == 'Examples') AND ([X-KDE-PluginInfo-Email] == 'prrrrt')");
    QTest::newRow("empty query") << _st << _c << 0;
}

void PluginTest::findPlugin()
{
    QFETCH(QString, serviceType);
    QFETCH(QString, constraint);
    QFETCH(int, expectedResult);
    const KPluginInfo::List res = KPluginTrader::self()->query(QString(), serviceType, constraint);
    QCOMPARE(res.count(), expectedResult);
}

void PluginTest::findSomething()
{
    const KPluginInfo::List res = KPluginTrader::self()->query(QString());
    QVERIFY(res.count() > 0);
}

void PluginTest::loadPlugin()
{
    const QString pluginName("fakeplugin");
    const QString serviceType("KService/NSA");
    const QString constraint = QString("[X-KDE-PluginInfo-Name] == '%1'").arg(pluginName);

    QObject* plugin = KPluginTrader::createInstanceFromQuery<QObject>(QString(), serviceType, constraint, this);
    QVERIFY(plugin != 0);
    QCOMPARE(plugin->objectName(), QStringLiteral("Test Plugin Spy"));
}

#include "moc_pluginlocatortest.cpp"
