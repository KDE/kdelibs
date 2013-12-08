/* This file is part of the KDE libraries
    Copyright (C) 2011 David Faure <faure@kde.org>

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

#include <QObject>

#include <qtest_kde.h>

#include <ksharedconfig.h>

#include <kconfig.h>
#include <kconfiggroup.h>
#include <kconfiggroup_kurl.h>
#include <qstandardpaths.h>

class KConfigCompatTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void testKUrl();
    void testDefaultName();
    void cleanupTestCase();
};

void KConfigCompatTest::initTestCase()
{
    // Qt5 TODO: should be done by qtestlib+qstandardpaths
    QString xdgConfigHome = QDir::home().canonicalPath() + "/.qttest/config";
    qputenv("XDG_CONFIG_HOME", QFile::encodeName(xdgConfigHome));
}

void KConfigCompatTest::cleanupTestCase()
{
    QFile::remove(QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + QLatin1String("/kconfig_compat_test"));
}

void KConfigCompatTest::testKUrl()
{
    KConfig config("kconfig_compat_test");
    {
        KConfigGroup cg(&config, "Group");
        cg.writeEntry("kurlEntry", KUrl("http://www.kde.org")); // DO NOT PORT TO QUrl
    }
    {
        KConfigGroup cg(&config, "Group");
        QCOMPARE(cg.readEntry("kurlEntry", KUrl()), KUrl("http://www.kde.org"));
    }
}

void KConfigCompatTest::testDefaultName()
{
    QCOMPARE(KSharedConfig::openConfig()->name(), QString::fromLatin1("myapprc"));
    QCOMPARE(KGlobal::config()->name(), QString::fromLatin1("myapprc"));
}

QTEST_KDEMAIN_CORE_WITH_COMPONENTNAME(KConfigCompatTest, "myapp")

#include "kconfigcompattest.moc"
