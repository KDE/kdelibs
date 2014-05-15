/* This file is part of the KDE libraries
   Copyright (c) 2005-2007 David Jarvie <software@astrojar.org.uk>

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

#ifndef KTIMEZONESTEST_P_H
#define KTIMEZONESTEST_P_H

#include <kconfiggroup.h>
#include <kstandarddirs.h>
#include <QDebug>
#include <QtCore/QDir>
#include "qtest_kde.h"

class TimeZoneTestData
{
public:
    QString dataDir() const { return mDataDir; }

    void cleanupTimeZoneTest()
    {
        removeDir(QLatin1String("ktimezonestest/Africa"));
        removeDir(QLatin1String("ktimezonestest/America"));
        removeDir(QLatin1String("ktimezonestest/Asia"));
        removeDir(QLatin1String("ktimezonestest/Europe"));
        removeDir(QLatin1String("ktimezonestest"));
        QFile::remove(KStandardDirs::locateLocal("config", "ktimezonedrc", false, KGlobal::mainComponent()));
    }

    void setupTimeZoneTest()
    {
        cleanupTimeZoneTest();

        mDataDir = QDir::homePath() + "/.kde-unit-test/ktimezonestest";
        QVERIFY(QDir().mkpath(mDataDir));
        writeZoneTab(false);
        QDir dir(mDataDir);
        QVERIFY(dir.mkdir("Africa"));
        QVERIFY(QFile::copy(QString::fromLatin1(KDESRCDIR) + QLatin1String("/Cairo"), mDataDir + QLatin1String("/Africa/Cairo")));
        QVERIFY(QFile::copy(QString::fromLatin1(KDESRCDIR) + QLatin1String("/Johannesburg"), mDataDir + QLatin1String("/Africa/Johannesburg")));
        QVERIFY(dir.mkdir("America"));
        QFile::copy(QString::fromLatin1(KDESRCDIR) + QLatin1String("/Los_Angeles"), mDataDir + QLatin1String("/America/Los_Angeles"));
        QVERIFY(dir.mkdir("Asia"));
        QFile::copy(QString::fromLatin1(KDESRCDIR) + QLatin1String("/Dili"), mDataDir + QLatin1String("/Asia/Dili"));
        QVERIFY(dir.mkdir("Europe"));
        QFile::copy(QString::fromLatin1(KDESRCDIR) + QLatin1String("/Berlin"), mDataDir + QLatin1String("/Europe/Berlin"));
        QFile::copy(QString::fromLatin1(KDESRCDIR) + QLatin1String("/London"), mDataDir + QLatin1String("/Europe/London"));
        QFile::copy(QString::fromLatin1(KDESRCDIR) + QLatin1String("/Paris"), mDataDir + QLatin1String("/Europe/Paris"));

        KConfig config("ktimezonedrc");
        KConfigGroup group(&config, "TimeZones");
        group.writeEntry("ZoneinfoDir", mDataDir);
        group.writeEntry("Zonetab", QString(mDataDir + QString::fromLatin1("/zone.tab")));
        group.writeEntry("LocalZone", QString::fromLatin1("Europe/Paris"));
        config.sync();
    }

private:
    void removeDir(const QString &subdir)
    {
        QDir local(QDir::homePath() + QLatin1String("/.kde-unit-test/") + subdir);
        foreach(const QString &file, local.entryList(QDir::Files))
            if(!local.remove(file))
                qWarning("%s: removing failed", qPrintable( file ));
        QCOMPARE((int)local.entryList(QDir::Files).count(), 0);
        local.cdUp();
        QString subd = subdir;
        subd.remove(QRegExp("^.*/"));
        local.rmpath(subd);
    }

public:
    void writeZoneTab(bool testcase)
    {
        QFile f(mDataDir + QLatin1String("/zone.tab"));
        qDebug() << "Writing" << f.fileName();
        QVERIFY(f.open(QIODevice::WriteOnly));
        QTextStream fStream(&f);
        if (testcase)
            fStream << "DE	+5230+01322	Europe/Berlin\n"
                       "EG	+3003+03115	Africa/Cairo\n"
                       "FR	+4852+00220	Europe/Paris\n"
                       "XX	-512830+0001845	Europe/London	Greater Britain\n"
                       "TL	-0833+12535	Asia/Dili\n"
                       "US	+340308-1181434	America/Los_Angeles	Pacific Time\n"
                       "ZA	-2615+02800	Africa/Johannesburg\n";
        else
            fStream << "EG	+3003+03115	Africa/Cairo\n"
                       "FR	+4852+00220	Europe/Paris\n"
                       "GB	+512830-0001845	Europe/London	Great Britain\n"
                       "US	+340308-1181434	America/Los_Angeles	Pacific Time\n"
                       "ZA	-2615+02800	Africa/Johannesburg\n";
        f.close();
    }

    QString mDataDir;
};

#endif //KTIMEZONESTEST_P_H
