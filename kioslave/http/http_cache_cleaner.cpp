/*
    This file is part of KDE

    Copyright (C) 1999 Waldo Bastian (bastian@kde.org)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License 
    version 2 as published by the Free Software Foundation.

    This software is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this library; see the file COPYING. If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
//----------------------------------------------------------------------------
//
// KDE Http Cache cleanup tool
// $Id$

#include <time.h>
#include <stdlib.h>

#include <qdir.h>
#include <qstring.h>
#include <qlist.h>

#include <kinstance.h>
#include <kglobal.h>
#include <kstddirs.h>
#include <dcopclient.h>
#include <kprotocolmanager.h>

#include <unistd.h>

time_t currentDate;
int m_maxCacheAge;
int m_maxCacheSize;

static const char *appName = "kio_http_cache_cleaner";

struct FileInfo {
   QString name;
   int size; // Size in Kb.
   int age; 
};

class FileInfoList : public QList<FileInfo>
{
public:
   FileInfoList() : QList<FileInfo>() { }
   int compareItems(QCollection::Item item1, QCollection::Item item2)
      { return ((FileInfo *)item1)->age - ((FileInfo *)item2)->age; }
};

// !START OF SYNC!
// Keep the following in sync with the cache code in http.cc
#define CACHE_REVISION "2\n"

FileInfo *readEntry( const QString &filename)
{
   QCString CEF = QFile::encodeName(filename);
   FILE *fs = fopen( CEF.data(), "r");
   if (!fs)
      return 0;

   char buffer[41];
   bool ok = true;

  // CacheRevision 
  if (ok && (!fgets(buffer, 40, fs)))
      ok = false;  
   if (ok && (strcmp(buffer, CACHE_REVISION) != 0))
      ok = false;

   time_t creationDate;
   time_t expireDate;
   int age;

   // Creation Date
   if (ok && (!fgets(buffer, 40, fs)))
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
   if (ok && (!fgets(buffer, 40, fs)))
      ok = false;  
   if (ok)
   {
      expireDate = (time_t) strtoul(buffer, 0, 10);
      if (expireDate && (expireDate < currentDate))
         ok = false; // Expired
   }
 
   fclose(fs);
   if (ok)
   {
      FileInfo *info = new FileInfo;
      info->age = age;
      return info;
   }

fprintf(stderr, "%s: Expired entry %s\n", appName, CEF.data());  
   unlink( CEF.data());
   return 0;
}
// Keep the above in sync with the cache code in http.cc
// !END OF SYNC!

void scanDirectory(FileInfoList &fileEntries, const QString &name, const QString &strDir)
{
   QDir dir(strDir);
   if (!dir.exists()) return;
   
   QFileInfoList *newEntries = (QFileInfoList *) dir.entryInfoList();

   if (!newEntries) return; // Directory not accesible ??

   for(QFileInfo *qFileInfo = newEntries->first();
       qFileInfo;
       qFileInfo = newEntries->next())
   {
       if (qFileInfo->isFile())
       {
          FileInfo *fileInfo = readEntry( strDir + "/" + qFileInfo->fileName());
          if (fileInfo)
          {
             fileInfo->name = name + "/" + qFileInfo->fileName();
             fileInfo->size = (qFileInfo->size() + 1023) / 1024;
             fileEntries.append(fileInfo);
          }
       }
   }
}

int main(int, char **)
{
   KInstance ins( appName );

   DCOPClient *dcop = new DCOPClient();

   if (dcop->registerAs(appName, false) != appName)
   {
      fprintf(stderr, "%s: Already running!\n", appName);
      exit(0);
   }

   currentDate = time(0);
   m_maxCacheAge = KProtocolManager::maxCacheAge();
   m_maxCacheSize = KProtocolManager::maxCacheSize();

   QString strCacheDir = KGlobal::dirs()->saveLocation("data", "kio_http/cache");

   QDir cacheDir( strCacheDir );
   if (!cacheDir.exists())
   {
      fprintf(stderr, "%s: '%s' does not exist.\n", appName, strCacheDir.ascii());
      exit(0);
   }
   
   QStringList dirs = cacheDir.entryList( );

   FileInfoList cachedEntries;
  
   for(QStringList::Iterator it = dirs.begin();
       it != dirs.end();
       it++)
   {
      if ((*it)[0] != '.')
      {
         scanDirectory( cachedEntries, *it, strCacheDir + "/" + *it);
      }
   }
 
   cachedEntries.sort();

   int totalSize = 0;

   for(FileInfo *fileInfo = cachedEntries.first();
       fileInfo;
       fileInfo = cachedEntries.next())
   {
      if ((totalSize + fileInfo->size) > m_maxCacheSize)
      {
         QCString filename = QFile::encodeName( strCacheDir + "/" + fileInfo->name);
         int result = unlink(filename.data());
         fprintf(stderr, "%s: Cache too big, deleting '%s' (%d)\n", appName, filename.data(), result);
      }
      else
      {
         totalSize += fileInfo->size;
// fprintf(stderr, "Keep in cache: %s %d %d total = %d\n", fileInfo->name.ascii(), fileInfo->size, fileInfo->age, totalSize);
      }
   }
   fprintf(stderr, "%s: Cache size = %d kB.\n", appName, totalSize);
}


