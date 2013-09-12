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

#include <qtest.h>
#include <QDebug>

#include <kplugininfo.h>
#include <kplugintrader.h>

#include <QJsonObject>

QTEST_MAIN(PluginTest)

void PluginTest::findPluginNoConstraints()
{
    QString constraint;
    const QString serviceType("KService/NSA");
    KPluginInfo::List res;

    constraint = QString();
    res = KPluginTrader::self()->query(QString(), serviceType, constraint);
    QVERIFY(res.count() > 0);
}

void PluginTest::findPluginName()
{
    QString constraint;
    const QString pluginName("fakeplugin");
    const QString serviceType("KService/NSA");
    KPluginInfo::List res;

    constraint = QString("[X-KDE-PluginInfo-Name] == '%1'").arg(pluginName);
    res = KPluginTrader::self()->query(QString(), serviceType, constraint);
    QVERIFY(res.count() > 0);
}


void PluginTest::findPluginCategory()
{
    const QString serviceType("KService/NSA");
    const QString category("Examples");
    KPluginInfo::List res;

    const QString constraint = QString("[X-KDE-PluginInfo-Category] == '%1'").arg(category);
    res = KPluginTrader::self()->query(QString(), serviceType, constraint);
    QVERIFY(res.count() > 0);
}

void PluginTest::findPluginComplex()
{
    const QString serviceType("KService/NSA");
    const QString category("Examples");
    KPluginInfo::List res;

    const QString constraint = QString("([X-KDE-PluginInfo-Category] == '%1') AND ([X-KDE-PluginInfo-Email] == 'sebas@kde.org')").arg(category);
    res = KPluginTrader::self()->query(QString(), serviceType, constraint);
    QVERIFY(res.count() > 0);
}

void PluginTest::findPluginEmpty()
{
    const QString serviceType("KService/NSA");
    const QString category("Examples");
    KPluginInfo::List res;

    const QString constraint = QString("([X-KDE-PluginInfo-Category] == '%1') AND ([X-KDE-PluginInfo-Email] == 'prrrrt')").arg(category);
    res = KPluginTrader::self()->query(QString(), serviceType, constraint);
    QVERIFY(res.count() == 0);
}


void PluginTest::loadPlugin()
{
    const QString pluginName("fakeplugin");
    const QString serviceType("KService/NSA");
    const QString constraint = QString("[X-KDE-PluginInfo-Name] == '%1'").arg(pluginName);

    QObject* plugin = KPluginTrader::createInstanceFromQuery<QObject>(QString(), serviceType, constraint, this);
    QVERIFY(plugin != 0);
    if (plugin) {
        //qDebug() << "Plugin loaded successfully" << plugin->objectName();
        QVERIFY(plugin->objectName() == QStringLiteral("Test Plugin Spy"));
    } else {
        //qDebug() << "Plugin failed to loaded";

    }
}


#include "moc_pluginlocatortest.cpp"

