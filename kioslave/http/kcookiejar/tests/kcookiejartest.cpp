/*
    This file is part of KDE 

    Copyright (C) 2004 Waldo Bastian (bastian@kde.org)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License 
    version 2 as published by the Free Software Foundation.

    This software is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this library; see the file COPYING. If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <stdio.h>
#include <stdlib.h>

#include <qdatetime.h>
#include <qstring.h>

#include <kapplication.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kstandarddirs.h>

#include "../kcookiejar.cpp"

static const char *description = "KCookiejar regression test";

static KCookieJar *jar;
static QString *lastYear;
static QString *nextYear;
static KConfig *config = 0;


static KCmdLineOptions options[] =
{
    { "+testfile", "Regression test to run", 0},
    KCmdLineLastOption
};

static void FAIL(const QString &msg)
{
   qWarning("%s", msg.toLocal8Bit().data());
   exit(1);
}

static void popArg(QString &command, QString & line)
{
   int i = line.find(' ');
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
   QString file = locateLocal("config", "kcookiejar-testconfig");
   QFile::remove(file);
   config = new KConfig(file);
   config->setGroup("Cookie Policy");
   config->writeEntry("RejectCrossDomainCookies", false);
   config->writeEntry("AcceptSessionCookies", false);
   config->writeEntry("IgnoreExpirationDate", false);
   config->writeEntry("CookieGlobalAdvice", "Ask");
   jar->loadConfig(config, false);
}

static void clearCookies()
{
   jar->eatAllCookies();
}

static void saveCookies()
{
   QString file = locateLocal("config", "kcookiejar-testcookies");
   QFile::remove(file);
   jar->saveCookies(file);
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
   KURL url(urlStr);
   if (!url.isValid())
      FAIL(QString("Invalid URL '%1'").arg(urlStr));
   if (url.isEmpty())
      FAIL(QString("Missing URL"));

   line.replace("%LASTYEAR%", *lastYear);
   line.replace("%NEXTYEAR%", *nextYear);

   KHttpCookieList list = jar->makeCookies(urlStr, line.toUtf8(), 0);

   if (list.isEmpty())
      FAIL(QString("Failed to make cookies from: '%1'").arg(line));

   for(KHttpCookie *cookie = list.first();
       cookie; cookie = list.next())
   {
      KCookieAdvice cookieAdvice = jar->cookieAdvice(cookie);
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
   KURL url(urlStr);
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

   config->setGroup("Cookie Policy");
   config->writeEntry(key, line);
   jar->loadConfig(config, false);
}

static void processLine(QString line)
{
   if (line.isEmpty())
      return;

   if (line[0] == '#')
   {
      if (line[1] == '#')
         qWarning("%s", line.latin1());
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
   qWarning("%s OK", filename.toLocal8Bit().data());
}

int main(int argc, char *argv[])
{
   QString arg1;
   QString arg2;
   QString result;

   lastYear = new QString(QString("Fri, 04-May-%1 01:00:00 GMT").arg(QDate::currentDate().year()-1));
   nextYear = new QString(QString(" expires=Fri, 04-May-%1 01:00:00 GMT").arg(QDate::currentDate().year()+1));

   KAboutData about("kcookietest", "kcookietest", "1.0", description, KAboutData::License_GPL, "(C) 2004 Waldo Bastian");
   KCmdLineArgs::init( argc, argv, &about);

   KCmdLineArgs::addCmdLineOptions( options );

   KInstance a("kcookietest");

   KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
   if (args->count() != 1)
      KCmdLineArgs::usage();

   jar = new KCookieJar;

   clearConfig();

   const QString file = args->url(0).path();
   runRegression(file);
   return 0;
}
