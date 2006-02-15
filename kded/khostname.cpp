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

#include <qfile.h>
#include <qtextstream.h>
#include <qregexp.h>

#include <dcopclient.h>

#include <kcmdlineargs.h>
#include <kapplication.h>
#include <klocale.h>
#include <kaboutdata.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kprocess.h>
#include <kde_file.h>

static KCmdLineOptions options[] = {
   { "+old", I18N_NOOP("Old hostname"), 0 },
   { "+new", I18N_NOOP("New hostname"), 0 },
   KCmdLineLastOption
};

static const char appName[] = "kdontchangethehostname";
static const char appVersion[] = "1.1";

class KHostName
{
public:
   KHostName();

   void changeX();
   void changeDcop();
   void changeStdDirs(const QByteArray &type);
   void changeSessionManager();

protected:
   QByteArray oldName;
   QByteArray newName;
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

   home = ::getenv("HOME");
   if (home.isEmpty())
   {
      fprintf(stderr, "%s", i18n("Error: HOME environment variable not set.\n").toLocal8Bit().data());
      exit(1);
   }

   display = ::getenv("DISPLAY");
   // strip the screen number from the display
   display.replace(QRegExp("\\.[0-9]+$"), "");
   if (display.isEmpty())
   {
      fprintf(stderr, "%s", i18n("Error: DISPLAY environment variable not set.\n").toLocal8Bit().data());
      exit(1);
   }
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
   QString cmd = "xauth list";
   FILE *xFile = popen(QFile::encodeName(cmd), "r");
   if (!xFile)
   {
      fprintf(stderr, "Warning: Can't run xauth.\n");
      return;
   }
   QList<QByteArray> lines;
   {
      char buf[1024+1];
      while (!feof(xFile))
      {
         QByteArray line = fgets(buf, 1024, xFile);
         if (line.length())
            line.truncate(line.length()-1); // Strip LF.
         if (!line.isEmpty())
            lines.append(line);
      }
   }
   pclose(xFile);

   foreach ( QByteArray it, lines )
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

      QByteArray newNetId = newName+netId.mid(i);

      cmd = "xauth remove "+KProcess::quote(netId);
      system(QFile::encodeName(cmd));
      cmd = "xauth add ";
      cmd += KProcess::quote(newNetId);
      cmd += " ";
      cmd += KProcess::quote(authName);
      cmd += " ";
      cmd += KProcess::quote(authKey);
      system(QFile::encodeName(cmd));
   }
}

void KHostName::changeDcop()
{
   DCOPCString fname = DCOPClient::dcopServerFile(oldName);
   DCOPCString origFName = fname;
   FILE *dcopFile = fopen(fname.data(), "r");
   if (!dcopFile)
   {
      fprintf(stderr, "Warning: Can't open '%s' for reading.\n", fname.data());
      return;
   }

   QByteArray line1, line2;
   {
     char buf[1024+1];
     line1 = fgets(buf, 1024, dcopFile);
     if (line1.length())
            line1.truncate(line1.length()-1); // Strip LF.

     line2 = fgets(buf, 1024, dcopFile);
     if (line2.length())
            line2.truncate(line2.length()-1); // Strip LF.
   }
   fclose(dcopFile);

   QByteArray oldNetId = line1;

   if (!newName.isEmpty())
   {
      int i = line1.lastIndexOf(':');
      if (i == -1)
      {
         fprintf(stderr, "Warning: File '%s' has unexpected format.\n", fname.data());
         return;
      }
      line1 = "local/"+newName+line1.mid(i);
      QByteArray newNetId = line1;
      fname = DCOPClient::dcopServerFile(newName);
      unlink(fname.data());
      dcopFile = fopen(fname.data(), "w");
      if (!dcopFile)
      {
         fprintf(stderr, "Warning: Can't open '%s' for writing.\n", fname.data());
         return;
      }

      fputs(line1.data(), dcopFile);
      fputc('\n', dcopFile);
      fputs(line2.data(), dcopFile);
      fputc('\n', dcopFile);

      fclose(dcopFile);

      DCOPCString compatLink = DCOPClient::dcopServerFileOld(newName);
      ::symlink(fname.data(), compatLink.data()); // Compatibility link

      // Update .ICEauthority
      QString cmd = "iceauth list "+KProcess::quote("netid="+oldNetId);
      FILE *iceFile = popen(QFile::encodeName(cmd), "r");
      if (!iceFile)
      {
         fprintf(stderr, "Warning: Can't run iceauth.\n");
         return;
      }
      QList<QByteArray> lines;
      {
         char buf[1024+1];
         while (!feof(iceFile))
         {
            QByteArray line = fgets(buf, 1024, iceFile);
            if (line.length())
               line.truncate(line.length()-1); // Strip LF.
            if (!line.isEmpty())
               lines.append(line);
         }
      }
      pclose(iceFile);

	  foreach ( QByteArray it, lines )
      {
         QList<QByteArray> entries = split(it);
         if (entries.count() != 5)
            continue;

         QByteArray protName = entries[0];
         QByteArray netId = entries[2];
         QByteArray authName = entries[3];
         QByteArray authKey = entries[4];
         if (netId != oldNetId)
            continue;

         cmd = "iceauth add ";
         cmd += KProcess::quote(protName);
         cmd += " '' ";
         cmd += KProcess::quote(newNetId);
         cmd += " ";
         cmd += KProcess::quote(authName);
         cmd += " ";
         cmd += KProcess::quote(authKey);
         system(QFile::encodeName(cmd));
      }
   }

   // Remove old entries
   {
      QString cmd = "iceauth remove "+KProcess::quote("netid="+oldNetId);
      system(QFile::encodeName(cmd));
      unlink(origFName.data());
      origFName = DCOPClient::dcopServerFileOld(oldName); // Compatibility link
      unlink(origFName.data());
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
      system(("lnusertemp "+type).data());
   }
}

void KHostName::changeSessionManager()
{
   DCOPCString sm = ::getenv("SESSION_MANAGER");
   if (sm.isEmpty())
   {
      fprintf(stderr, "Warning: No session management specified.\n");
      return;
   }
   int i = sm.lastIndexOf(':');
   if ((i == -1) || (sm.left(6) != "local/"))
   {
      fprintf(stderr, "Warning: Session Management socket '%s' has unexpected format.\n", sm.data());
      return;
   }
   sm = "local/"+newName+sm.mid(i);
   DCOPCString name = "SESSION_MANAGER";
   QByteArray params;
   QDataStream stream(&params, QIODevice::WriteOnly);
   stream << name << sm;
   DCOPClient *client = new DCOPClient();
   if (!client->attach())
   {
      fprintf(stderr, "Warning: DCOP communication problem, can't fix Session Management.\n");
      delete client;
      return;
   }
   DCOPCString launcher = KApplication::launcher();
   client->send(launcher, launcher, "setLaunchEnv(QCString,QCString)", params);
   delete client;
}

int main(int argc, char **argv)
{
   KLocale::setMainCatalog("kdelibs");
   KAboutData d(appName, I18N_NOOP("KDontChangeTheHostName"), appVersion,
                I18N_NOOP("Informs KDE about a change in hostname"),
                KAboutData::License_GPL, "(c) 2001 Waldo Bastian");
   d.addAuthor("Waldo Bastian", I18N_NOOP("Author"), "bastian@kde.org");

   KCmdLineArgs::init(argc, argv, &d);
   KCmdLineArgs::addCmdLineOptions(options);

   KInstance k(&d);

   KHostName hn;

   if(!getenv("XAUTHLOCALHOSTNAME"))
       hn.changeX();

   hn.changeDcop();
   hn.changeStdDirs("socket");
   hn.changeStdDirs("tmp");
   hn.changeSessionManager();
}

