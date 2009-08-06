/*
    This file is part of KDE

    Copyright (C) 2004 Waldo Bastian (bastian@kde.org)
    Copyright 2008 David Faure <faure@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public License
    as published by the Free Software Foundation; either
    version 2, or (at your option) version 3.

    This software is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this library; see the file COPYING. If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <QtCore/QDate>
#include <QtCore/QString>

#include <qtest_kde.h>
#include <kstandarddirs.h>

#include "../kcookiejar.cpp"

static KCookieJar *jar;
static QString *lastYear;
static QString *nextYear;
static KConfig *config = 0;


static void FAIL(const QString &msg)
{
   qWarning("%s", msg.toLocal8Bit().data());
   exit(1);
}

static void popArg(QString &command, QString & line)
{
   int i = line.indexOf(' ');
   if (i != -1)
   {
      command = line.left(i);
      line = line.mid(i+1);
   }
   else
   {
      command = line;
      line.clear();
   }
}


static void clearConfig()
{
   delete config;
   QString file = KStandardDirs::locateLocal("config", "kcookiejar-testconfig");
   QFile::remove(file);
   config = new KConfig(file);
   KConfigGroup cg(config, "Cookie Policy");
   cg.writeEntry("RejectCrossDomainCookies", false);
   cg.writeEntry("AcceptSessionCookies", false);
   cg.writeEntry("IgnoreExpirationDate", false);
   cg.writeEntry("CookieGlobalAdvice", "Ask");
   jar->loadConfig(config, false);
}

static void clearCookies()
{
   jar->eatAllCookies();
}

static void saveCookies()
{
   QString file = KStandardDirs::locateLocal("config", "kcookiejar-testcookies");
   QFile::remove(file);
   jar->saveCookies(file);

   // Add an empty domain to the cookies file, just for testing robustness
   QFile f(file);
   f.open(QIODevice::Append);
   f.write("[]\n   \"\"   \"/\"    1584320400  0 h  4  x\n");
   f.close();

   delete jar;
   jar = new KCookieJar();
   clearConfig();
   jar->loadCookies(file);
}

static void processCookie(QString &line)
{
   QString policy;
   popArg(policy, line);
   KCookieAdvice expectedAdvice = KCookieJar::strToAdvice(policy);
   if (expectedAdvice == KCookieDunno)
      FAIL(QString("Unknown accept policy '%1'").arg(policy));

   QString urlStr;
   popArg(urlStr, line);
   KUrl url(urlStr);
   if (!url.isValid())
      FAIL(QString("Invalid URL '%1'").arg(urlStr));
   if (url.isEmpty())
      FAIL(QString("Missing URL"));

   line.replace("%LASTYEAR%", *lastYear);
   line.replace("%NEXTYEAR%", *nextYear);

   KHttpCookieList list = jar->makeCookies(urlStr, line.toUtf8(), 0);

   if (list.isEmpty())
      FAIL(QString("Failed to make cookies from: '%1'").arg(line));

   for(KHttpCookieList::iterator cookieIterator = list.begin();
       cookieIterator != list.end(); ++cookieIterator) {
       KHttpCookie& cookie = *cookieIterator;
       const KCookieAdvice cookieAdvice = jar->cookieAdvice(cookie);
       if (cookieAdvice != expectedAdvice)
           FAIL(urlStr+QString("\n'%2'\nGot advice '%3' expected '%4'").arg(line)
                .arg(KCookieJar::adviceToStr(cookieAdvice))
                .arg(KCookieJar::adviceToStr(expectedAdvice)));
       jar->addCookie(cookie);
   }
}

static void processCheck(QString &line)
{
   QString urlStr;
   popArg(urlStr, line);
   KUrl url(urlStr);
   if (!url.isValid())
      FAIL(QString("Invalid URL '%1'").arg(urlStr));
   if (url.isEmpty())
      FAIL(QString("Missing URL"));

   QString expectedCookies = line;

   QString cookies = jar->findCookies(urlStr, false, 0, 0).trimmed();
   if (cookies != expectedCookies)
      FAIL(urlStr+QString("\nGot '%1' expected '%2'")
              .arg(cookies, expectedCookies));
}

static void processClear(QString &line)
{
   if (line == "CONFIG")
      clearConfig();
   else if (line == "COOKIES")
      clearCookies();
   else
      FAIL(QString("Unknown command 'CLEAR %1'").arg(line));
}

static void processConfig(QString &line)
{
   QString key;
   popArg(key, line);

   if (key.isEmpty())
      FAIL(QString("Missing Key"));

   KConfigGroup cg(config, "Cookie Policy");
   cg.writeEntry(key, line);
   jar->loadConfig(config, false);
}

static void processLine(QString line)
{
   if (line.isEmpty())
      return;

   if (line[0] == '#')
   {
      if (line[1] == '#')
         qDebug("%s", line.toLatin1().constData());
      return;
   }

   QString command;
   popArg(command, line);
   if (command.isEmpty())
      return;

   if (command == "COOKIE")
      processCookie(line);
   else if (command == "CHECK")
      processCheck(line);
   else if (command == "CLEAR")
      processClear(line);
   else if (command == "CONFIG")
      processConfig(line);
   else if (command == "SAVE")
      saveCookies();
   else
      FAIL(QString("Unknown command '%1'").arg(command));
}

static void runRegression(const QString &filename)
{
   FILE *file = fopen(QFile::encodeName( filename ), "r");
   if (!file)
      FAIL(QString("Can't open '%1'").arg(filename));

   char buf[4096];
   while (fgets(buf, sizeof(buf), file))
   {
      int l = strlen(buf);
      if (l)
      {
         l--;
         buf[l] = 0;
      }
      processLine(QString::fromUtf8(buf));
   }
   fclose( file );
   qDebug("%s OK", filename.toLocal8Bit().data());
}

class KCookieJarTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase()
    {
        jar = new KCookieJar;
        KDateTime dt = KDateTime::currentDateTime(KDateTime::Spec::LocalZone());
        lastYear = new QString(QString("%1 01:00:00 GMT").arg(dt.addYears(-1).toString("%:a, %e-%:b-%Y")));
        nextYear = new QString(QString("%1 01:00:00 GMT").arg(dt.addYears(1).toString("%:a, %e-%:b-%Y")));
    }
    void testCookieFile_data()
    {
        QTest::addColumn<QString>("fileName");
        QTest::newRow("cookie.test") << KDESRCDIR "/cookie.test";
        QTest::newRow("cookie_rfc.test") << KDESRCDIR "/cookie_rfc.test";
        QTest::newRow("cookie_saving.test") << KDESRCDIR "/cookie_saving.test";
        QTest::newRow("cookie_settings.test") << KDESRCDIR "/cookie_settings.test";
    }
    void testCookieFile()
    {
        QFETCH(QString, fileName);
        clearConfig();
        runRegression(fileName);
    }

    void testParseUrl_data()
    {
        QTest::addColumn<QString>("url");
        QTest::addColumn<bool>("expectedResult");
        QTest::addColumn<QString>("expectedFqdn");
        QTest::addColumn<QString>("expectedPath");
        QTest::newRow("empty") << "" << false << "" << "";
        QTest::newRow("url with no path") << "http://bugs.kde.org" << true << "bugs.kde.org" << "/";
        QTest::newRow("url with path") << "http://bugs.kde.org/foo" << true << "bugs.kde.org" << "/foo";
        QTest::newRow("just a host") << "bugs.kde.org" << false << "" << "";
    }
    void testParseUrl()
    {
        QFETCH(QString, url);
        QFETCH(bool, expectedResult);
        QFETCH(QString, expectedFqdn);
        QFETCH(QString, expectedPath);
        QString fqdn;
        QString path;
        bool result = KCookieJar::parseUrl(url, fqdn, path);
        QCOMPARE(result, expectedResult);
        QCOMPARE(fqdn, expectedFqdn);
        QCOMPARE(path, expectedPath);
    }

    void testExtractDomains_data()
    {
        QTest::addColumn<QString>("fqdn");
        QTest::addColumn<QStringList>("expectedDomains");
        QTest::newRow("empty") << "" << (QStringList() << "localhost");
        QTest::newRow("ipv4") << "1.2.3.4" << (QStringList() << "1.2.3.4");
        QTest::newRow("ipv6") << "[fe80::213:d3ff:fef4:8c92]" << (QStringList() << "[fe80::213:d3ff:fef4:8c92]");
        QTest::newRow("bugs.kde.org") << "bugs.kde.org" << (QStringList() << "bugs.kde.org" << ".bugs.kde.org" << "kde.org" << ".kde.org");

    }
    void testExtractDomains()
    {
        QFETCH(QString, fqdn);
        QFETCH(QStringList, expectedDomains);
        KCookieJar jar;
        QStringList lst;
        jar.extractDomains(fqdn, lst);
        QCOMPARE(lst, expectedDomains);
    }
};

QTEST_KDEMAIN(KCookieJarTest, NoGUI)

#include "kcookiejartest.moc"
