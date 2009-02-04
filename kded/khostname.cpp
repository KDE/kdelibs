/*  This file is part of the KDE libraries
 *  Copyright (C) 2001 Waldo Bastian <bastian@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation;
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include <QtCore/QFile>
#include <QtCore/QRegExp>
#include <QtCore/Q_PID>

#include <kcmdlineargs.h>
#include <kapplication.h>
#include <klocale.h>
#include <kaboutdata.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <ktoolinvocation.h>
#include <klauncher_iface.h>
#include <kde_file.h>
#include <QtDBus/QtDBus>

static const char appName[] = "kdontchangethehostname";
static const char appVersion[] = "1.1";

class KHostName
{
public:
   KHostName();

   void changeX();
   void changeStdDirs(const QByteArray &type);
   void changeSessionManager();

protected:
   QString oldName;
   QString newName;
   QString display;
   QByteArray home;
};

KHostName::KHostName()
{
   KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
   if (args->count() != 2)
      args->usage();
   oldName = args->arg(0);
   newName = args->arg(1);
   if (oldName == newName)
      exit(0);

   home = qgetenv("HOME");
   if (home.isEmpty())
   {
      fprintf(stderr, "%s", i18n("Error: HOME environment variable not set.\n").toLocal8Bit().data());
      exit(1);
   }

   display = QString::fromLocal8Bit(qgetenv("DISPLAY"));
   // strip the screen number from the display
   display.remove(QRegExp("\\.[0-9]+$"));
#if defined(Q_WS_X11) || defined(Q_WS_QWS)
   if (display.isEmpty())
   {
      fprintf(stderr, "%s", i18n("Error: DISPLAY environment variable not set.\n").toLocal8Bit().data());
      exit(1);
   }
#endif
}

static QList<QByteArray> split(const QByteArray &str)
{
   const char *s = str.data();
   QList<QByteArray> result;
   while (*s)
   {
      const char *i = strchr(s, ' ');
      if (!i)
      {
         result.append(QByteArray(s));
         return result;
      }
      result.append(QByteArray(s, i-s+1));
      s = i;
      while (*s == ' ') s++;
   }
   return result;
}

void KHostName::changeX()
{
   QProcess proc;
   proc.start("xauth", QStringList() << "-n" << "list");
   if (!proc.waitForFinished())
   {
      fprintf(stderr, "Warning: Can not run xauth.\n");
      return;
   }
   QList<QByteArray> lines;
   {
      while (!proc.atEnd())
      {
         QByteArray line = proc.readLine();
         if (line.length())
            line.truncate(line.length()-1); // Strip LF.
         if (!line.isEmpty())
            lines.append(line);
      }
   }

   foreach ( const QByteArray &it, lines )
   {
      QList<QByteArray> entries = split(it);
      if (entries.count() != 3)
         continue;

      QByteArray netId = entries[0];
      QByteArray authName = entries[1];
      QByteArray authKey = entries[2];

      int i = netId.lastIndexOf(':');
      if (i == -1)
         continue;
      QByteArray netDisplay = netId.mid(i);
      if (netDisplay != display)
         continue;

      i = netId.indexOf('/');
      if (i == -1)
         continue;

      QString newNetId = newName+netId.mid(i);
      QString oldNetId = netId.left(i);

      if (oldNetId != oldName)
	continue;

      QProcess::execute("xauth", QStringList() << "-n" << "remove" << netId);
      QProcess::execute("xauth", QStringList() << "-n" << "add" << newNetId << authName << authKey);
   }
}

void KHostName::changeStdDirs(const QByteArray &type)
{
   // We make links to the old dirs cause we can't delete the old dirs.
   QByteArray oldDir = QFile::encodeName(QString("%1%2-%3").arg(KGlobal::dirs()->localkdedir()).arg(QString( type )).arg(QString( oldName )));
   QByteArray newDir = QFile::encodeName(QString("%1%2-%3").arg(KGlobal::dirs()->localkdedir()).arg(QString( type )).arg(QString( newName )));

   KDE_struct_stat st_buf;

   int result = KDE_lstat(oldDir.data(), &st_buf);
   if (result == 0)
   {
      if (S_ISLNK(st_buf.st_mode))
      {
         char buf[4096+1];
         result = readlink(oldDir.data(), buf, 4096);
         if (result >= 0)
         {
            buf[result] = 0;
            result = symlink(buf, newDir.data());
         }
      }
      else if (S_ISDIR(st_buf.st_mode))
      {
         result = symlink(oldDir.data(), newDir.data());
      }
      else
      {
         result = -1;
      }
   }
   if (result != 0)
   {
       const QString lnusertemp = KGlobal::dirs()->findExe( "lnusertemp" );
       QProcess::execute( lnusertemp, QStringList() << type );
   }
}

void KHostName::changeSessionManager()
{
   QString sm = QString::fromLocal8Bit(qgetenv("SESSION_MANAGER"));
   if (sm.isEmpty())
   {
      fprintf(stderr, "Warning: No session management specified.\n");
      return;
   }
   int i = sm.lastIndexOf(':');
   if ((i == -1) || (sm.left(6) != "local/"))
   {
      fprintf(stderr, "Warning: Session Management socket '%s' has unexpected format.\n", sm.toLocal8Bit().constData());
      return;
   }
   sm = "local/"+newName+sm.mid(i);
   KToolInvocation::klauncher()->call(QDBus::NoBlock, "setLaunchEnv", QByteArray("SESSION_MANAGER"), sm);
}

int main(int argc, char **argv)
{
   KAboutData d(appName, "kdelibs4", ki18n("KDontChangeTheHostName"), appVersion,
                ki18n("Informs KDE about a change in hostname"),
                KAboutData::License_GPL, ki18n("(c) 2001 Waldo Bastian"));
   d.addAuthor(ki18n("Waldo Bastian"), ki18n("Author"), "bastian@kde.org");

   KCmdLineOptions options;
   options.add("+old", ki18n("Old hostname"));
   options.add("+new", ki18n("New hostname"));

   KCmdLineArgs::init(argc, argv, &d);
   KCmdLineArgs::addCmdLineOptions(options);

   KComponentData k(&d);

   KHostName hn;

   hn.changeX();
   hn.changeStdDirs("socket");
   hn.changeStdDirs("tmp");
   hn.changeSessionManager();
}

