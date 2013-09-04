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

#include "plugintest.h"

#include <QDebug>
#include <kservice.h>
#include <kservicetypetrader.h>
#include <kpluginlocator.h>
#include <kshell.h>

#include <qcommandlineparser.h>

#include <QDir>
#include <QElapsedTimer>
#include <QFileInfo>
#include <QMap>
#include <QStandardPaths>
#include <QStringList>
#include <QTimer>
#include <QJsonObject>

// #include <iostream>
// #include <iomanip>

static QTextStream cout(stdout);


class PluginTestPrivate {
public:
    QString pluginName;
    QCommandLineParser *parser;
};

PluginTest::PluginTest(int& argc, char** argv, QCommandLineParser *parser) :
    QCoreApplication(argc, argv)
{
    d = new PluginTestPrivate;
    d->parser = parser;
    QTimer::singleShot(0, this, SLOT(runMain()));
}

PluginTest::~PluginTest()
{
    delete d;
}

void PluginTest::runMain()
{
    // measure performance
    QElapsedTimer timer;
    qint64 nanoSec;
    int runs = 1;
    QList<int> timings;

    cout << "-- PluginLocator Test --" << endl;
    bool ok = true;

    timer.start();
    for (int _i = 0; _i < runs; _i++) {
        timer.restart();
        if (!loadFromKService("time")) ok = false;
        timings << timer.nsecsElapsed();
    }
    report(timings, "KServiceTrader Querying");

    timings.clear();

    for (int _i = 0; _i < runs; _i++) {
        timer.restart();
        if (!loadFromMetaData()) ok = false;
        timings << timer.nsecsElapsed();
    }

    report(timings, "Metadata Querying");

    if (ok) {
        cout << "All tests finished successfully";
        exit(0);
    }
    exit(1);
}

void PluginTest::report(QList< int > timings, const QString& msg)
{
    int i = 0;
    qulonglong totalTime = 0;

    int unitDiv = 1000;
    QString unit = "microsec";

    foreach (int t, timings) {
        //int msec = t/1000;
        //cout << "  Run " << i << ": " << msec << " microsec\n";
        totalTime += t;

        //i++;
    }
    QString av = QString::number((totalTime/timings.count()/unitDiv), 'f', 1);
    cout << " Average: " << av << " " << unit << " (" << msg << ")" << endl;
}



bool PluginTest::loadFromKService(const QString &name)
{
    bool ok = false;
    QString constraint = QString("[X-KDE-PluginInfo-Name] == '%1'").arg(name);
    KService::List offers = KServiceTypeTrader::self()->query("Plasma/DataEngine",
                                                              constraint);
    QString error;

    if (offers.isEmpty()) {
        //qDebug() << "offers are empty for " << name << " with constraint " << constraint;
    } else {
        QVariantList allArgs;
        allArgs << offers.first()->storageId();
        const QString _n = offers.first()->property("Name").toString();
        if (!_n.isEmpty()) {
            //cout << "Found Dataengine: " << _n << endl;
            ok = true;
        } else {
            //cout << "Nothing found. " << endl;
        }
    }

    return ok;
}


bool PluginTest::loadFromMetaData(const QString& pluginName)
{
    bool ok = false;

    //cout << "Looking for PluginName: " << pluginName << endl;
    const QStringList res = KPluginLocator::locatePlugin(pluginName);

//     foreach (const QString &r, res) {
//         //ok = true;
//         //cout << "  Found plugin: " << r << endl;
//     }
    //cout << "Found " << res.count() << " plugins." << endl;
    ok = res.count() > 0;

    return ok;
}

#include "moc_plugintest.cpp"

