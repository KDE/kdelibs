/*
This file is part of KDE

 Copyright (C) 1999-2000 Waldo Bastian (bastian@kde.org)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
//----------------------------------------------------------------------------
//
// KDE Http Cache cleanup tool

#include <time.h>
#include <stdlib.h>
#include <zlib.h>

#include <QtCore/QDir>
#include <QtCore/QString>
#include <QtDBus/QtDBus>

#include <kcomponentdata.h>
#include <klocale.h>
#include <kcmdlineargs.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kprotocolmanager.h>

#include <unistd.h>

#include <kdebug.h>

time_t currentDate;
int m_maxCacheAge;
int m_maxCacheSize;

static const char appFullName[] = "org.kde.kio_http_cache_cleaner";
static const char appName[] = "kio_http_cache_cleaner";

static const char version[] = "9";

struct FileInfo {
   QString name;
   int size; // Size in Kb.
//    int age;
   int rating;

   bool operator<( const struct FileInfo &info )
   {
     return ( rating < info.rating);
   }
};

template class QList<FileInfo*>;

class FileInfoList : public QList<FileInfo*>
{
public:
   FileInfoList() : QList<FileInfo*>() { }
};

// !START OF SYNC!
// Keep the following in sync with the cache code in http.cpp
#define CACHE_REVISION "9\n"

FileInfo *readEntry( const QString &filename)
{
   QByteArray CEF = QFile::encodeName(filename);
   gzFile fs = gzopen( CEF.data(), "r");
   if (!fs)
      return 0;

   char buffer[401];
   bool ok = true;

  // CacheRevision
  if (ok && (!gzgets(fs, buffer, 400)))
      ok = false;
   if (ok && (strcmp(buffer, CACHE_REVISION) != 0))
      ok = false;

   // Full URL
   if (ok && (!gzgets(fs, buffer, 400)))
      ok = false;

   time_t creationDate;
   int age =0;

   // Creation Date
   if (ok && (!gzgets(fs, buffer, 400)))
      ok = false;
   if (ok)
   {
      creationDate = (time_t) strtoul(buffer, 0, 10);
      age = (int) difftime(currentDate, creationDate);
      if ( m_maxCacheAge && ( age > m_maxCacheAge))
      {
         ok = false; // Expired
      }
   }

   // Expiration Date
   if (ok && (!gzgets(fs, buffer, 400)))
      ok = false;
   if (ok)
   {
//WABA: It seems I slightly misunderstood the meaning of "Expire:" header.
#if 0
      time_t expireDate;
      expireDate = (time_t) strtoul(buffer, 0, 10);
      if (expireDate && (expireDate < currentDate))
         ok = false; // Expired
#endif
   }

   // ETag
   if (ok && (!gzgets(fs, buffer, 400)))
      ok = false;
   if (ok)
   {
      // Ignore ETag
   }

   // Last-Modified
   if (ok && (!gzgets(fs, buffer, 400)))
      ok = false;
   if (ok)
   {
      // Ignore Last-Modified
   }


   gzclose(fs);
   if (ok)
   {
      FileInfo *info = new FileInfo;

      int freq=0;
      FILE* hitdata = fopen( QFile::encodeName(CEF+"_freq"), "r+");
      if (hitdata && (freq=fgetc(hitdata))!=EOF)
      {
         freq+=fgetc(hitdata)<<8;
         if (freq>0)
            info->rating=age/freq;
         else
            info->rating=age;
         fclose(hitdata);
      }
      else
         info->rating = age;
      return info;
   }

   unlink( CEF.data());
   return 0;
}
// Keep the above in sync with the cache code in http.cc
// !END OF SYNC!

void scanDirectory(FileInfoList &fileEntries, const QString &name, const QString &strDir)
{
   QDir dir(strDir);
   if (!dir.exists()) return;

   QFileInfoList newEntries = dir.entryInfoList();

   if (newEntries.count()==0) return; // Directory not accessible ??

   foreach ( const QFileInfo &qFileInfo, newEntries ) {
       if (qFileInfo.isFile())
       {
          FileInfo *fileInfo = readEntry( strDir + '/' + qFileInfo.fileName());
          if (fileInfo)
          {
             fileInfo->name = name + '/' + qFileInfo.fileName();
             fileInfo->size = (qFileInfo.size() + 1023) / 1024;
             fileEntries.append(fileInfo);
          }
       }
   }
}

extern "C" KDE_EXPORT int kdemain(int argc, char **argv)
{
   KCmdLineArgs::init( argc, argv, appName, "kdelibs4",
		       ki18n("KDE HTTP cache maintenance tool"), version,
		       ki18n("KDE HTTP cache maintenance tool"), KCmdLineArgs::CmdLineArgNone);

   KCmdLineOptions options;
   options.add("clear-all", ki18n("Empty the cache"));

   KCmdLineArgs::addCmdLineOptions( options );

   KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

   bool deleteAll = args->isSet("clear-all");

   KComponentData ins( appName );

   if (!deleteAll)
   {
      if (!QDBusConnection::sessionBus().isConnected())
      {
         QDBusError error(QDBusConnection::sessionBus().lastError());
         fprintf(stderr, "%s: Could not connect to D-Bus! (%s: %s)\n", appName,
                 qPrintable(error.name()), qPrintable(error.message()));
         return 1;
      }

      if (!QDBusConnection::sessionBus().registerService(appFullName))
      {
         fprintf(stderr, "%s: Already running!\n", appName);
         return 0;
      }
   }

   currentDate = time(0);
   m_maxCacheAge = KProtocolManager::maxCacheAge();
   m_maxCacheSize = KProtocolManager::maxCacheSize();

   if (deleteAll)
      m_maxCacheSize = -1;

   QString strCacheDir = KGlobal::dirs()->saveLocation("cache", "http");

   QDir cacheDir( strCacheDir );
   if (!cacheDir.exists())
   {
      fprintf(stderr, "%s: '%s' does not exist.\n", appName, qPrintable(strCacheDir));
      return 0;
   }

   const QStringList dirs = cacheDir.entryList( );

   FileInfoList cachedEntries;

   for(QStringList::ConstIterator it = dirs.begin();
       it != dirs.end();
       ++it)
   {
      if (it->at(0) != '.')
      {
         scanDirectory( cachedEntries, *it, strCacheDir + '/' + *it);
      }
   }

   qSort(cachedEntries.begin(), cachedEntries.end());

   int maxCachedSize = m_maxCacheSize / 2;

   Q_FOREACH( FileInfo *fileInfo , cachedEntries )
   {
      if (fileInfo->size > maxCachedSize)
      {
         QByteArray filename = QFile::encodeName( strCacheDir + '/' + fileInfo->name);
         unlink(filename.data());
//         kDebug (7113) << appName << ": Object too big, deleting '" << filename.data() << "' (" << result<< ")";
      }
   }

   int totalSize = 0;

   Q_FOREACH( FileInfo *fileInfo , cachedEntries )
   {
      if ((totalSize + fileInfo->size) > m_maxCacheSize)
      {
         QByteArray filename = QFile::encodeName( strCacheDir + '/' + fileInfo->name);
         unlink(filename.data());
//         kDebug (7113) << appName << ": Cache too big, deleting '" << filename.data() << "' (" << fileInfo->size << ")";
      }
      else
      {
         totalSize += fileInfo->size;
// fprintf(stderr, "Keep in cache: %s %d %d total = %d\n", fileInfo->name.toAscii().constData(), fileInfo->size, fileInfo->age, totalSize);
      }
   }
   kDebug (7113) << appName << ": Current size of cache = " << totalSize << " kB.";
   return 0;
}
