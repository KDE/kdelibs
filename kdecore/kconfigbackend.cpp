/*
  This file is part of the KDE libraries
  Copyright (c) 1999 Preston Brown <pbrown@kde.org>
  Copyright (c) 1997-1999 Matthias Kalle Dalheimer <kalle@kde.org>

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
  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
  Boston, MA 02111-1307, USA.
*/

#include <config.h>

#include <unistd.h>
#include <ctype.h>
#ifdef HAVE_SYS_MMAN_H
#include <sys/mman.h>
#endif
#include <sys/types.h>
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#include <fcntl.h>
#include <signal.h>

#undef Unsorted
#include <qdir.h>
#include <qfileinfo.h>
#include <qtextcodec.h>
#include <qtextstream.h>

#include "kconfigbackend.h"
#include "kconfigbase.h"
#include <kglobal.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <ksavefile.h>
#include <kurl.h>

extern bool checkAccess(const QString& pathname, int mode);
/* translate escaped escape sequences to their actual values. */
static QCString printableToString(const char *str, int l)
{
  // Strip leading white-space.
  while((l>0) &&
        ((*str == ' ') || (*str == '\t') || (*str == '\r')))
  {
     str++; l--;
  }

  // Strip trailing white-space.
  while((l>0) &&
        ((str[l-1] == ' ') || (str[l-1] == '\t') || (str[l-1] == '\r')))
  {
     l--;
  }

  QCString result(l + 1);
  char *r = result.data();

  for(int i = 0; i < l;i++, str++)
  {
     if (*str == '\\')
     {
        i++, str++;
        if (i >= l) // End of line. (Line ends with single slash)
        {
           *r++ = '\\';
           break;
        }
        switch(*str)
        {
           case 's':
              *r++ = ' ';
              break;
           case 't':
              *r++ = '\t';
              break;
           case 'n':
              *r++ = '\n';
              break;
           case 'r':
              *r++ = '\r';
              break;
           case '\\':
              *r++ = '\\';
              break;
           default:
              *r++ = '\\';
              *r++ = *str;
        }
     }
     else
     {
        *r++ = *str;
     }
  }
  result.truncate(r-result.data());
  return result;
}

static QCString stringToPrintable(const QCString& str){
  QCString result(str.length()*2); // Maximum 2x as long as source string
  register char *r = result.data();
  register char *s = str.data();

  if (!s) return QCString("");

  // Escape leading space
  if (*s == ' ')
  {
     *r++ = '\\'; *r++ = 's';
     s++;
  }

  if (*s)
  {
   while(*s)
   {
    if (*s == '\n')
    {
      *r++ = '\\'; *r++ = 'n';
    }
    else if (*s == '\t')
    {
      *r++ = '\\'; *r++ = 't';
    }
    else if (*s == '\r')
    {
      *r++ = '\\'; *r++ = 'r';
    }
    else if (*s == '\\')
    {
      *r++ = '\\'; *r++ = '\\';
    }
    else
    {
      *r++ = *s;
    }
    s++;
   }
   // Escape trailing space
   if (*(r-1) == ' ')
   {
      *(r-1) = '\\'; *r++ = 's';
   }
  }

  result.truncate(r - result.data());
  return result;
}

void KConfigBackEnd::changeFileName(const QString &_fileName,
                                    const char * _resType,
                                    bool _useKDEGlobals)
{
   mfileName = _fileName;
   resType = _resType;
   useKDEGlobals = _useKDEGlobals;
   if (mfileName.isEmpty())
      mLocalFileName = QString::null;
   else if (mfileName[0] == '/')
      mLocalFileName = mfileName;
   else
      mLocalFileName = KGlobal::dirs()->saveLocation(resType) + mfileName;

   if (useKDEGlobals)
      mGlobalFileName = KGlobal::dirs()->saveLocation("config") +
	      QString::fromLatin1("kdeglobals");
   else
      mGlobalFileName = QString::null;
}

KConfigBackEnd::KConfigBackEnd(KConfigBase *_config,
			       const QString &_fileName,
			       const char * _resType,
			       bool _useKDEGlobals)
  : pConfig(_config), bFileImmutable(false), mConfigState(KConfigBase::NoAccess), mFileMode(-1)
{
   changeFileName(_fileName, _resType, _useKDEGlobals);
}

void KConfigBackEnd::setFileWriteMode(int mode)
{
  mFileMode = mode;
}

bool KConfigINIBackEnd::parseConfigFiles()
{
  // Check if we can write to the local file.
  mConfigState = KConfigBase::ReadOnly;
  if (!mLocalFileName.isEmpty() && !pConfig->isReadOnly())
  {
     if (checkAccess(mLocalFileName, W_OK))
     {
        mConfigState = KConfigBase::ReadWrite;
     }
     else
     {
        // Create the containing dir, maybe it wasn't there
        KURL path;
        path.setPath(mLocalFileName);
        QString dir=path.directory();
        KStandardDirs::makeDir(dir);

        if (checkAccess(mLocalFileName, W_OK))
        {
           mConfigState = KConfigBase::ReadWrite;
        }
     }
  }

  // Parse all desired files from the least to the most specific.
  bFileImmutable = false;

  // Parse the general config files
  if (useKDEGlobals) {
    QStringList kdercs = KGlobal::dirs()->
      findAllResources("config", QString::fromLatin1("kdeglobals"));

    if (checkAccess(QString::fromLatin1("/etc/kderc"), R_OK))
      kdercs += QString::fromLatin1("/etc/kderc");

    kdercs += KGlobal::dirs()->
      findAllResources("config", QString::fromLatin1("system.kdeglobals"));

    QStringList::ConstIterator it;

    for (it = kdercs.fromLast(); it != kdercs.end(); --it) {

      QFile aConfigFile( *it );
      if (!aConfigFile.open( IO_ReadOnly ))
	   continue;
      parseSingleConfigFile( aConfigFile, 0L, true, (*it != mGlobalFileName) );
      aConfigFile.close();
      if (bFileImmutable)
         break;
    }
  }

  bool bReadFile = !mfileName.isEmpty();
  while(bReadFile) {
    bReadFile = false;
    QString bootLanguage;
    if (useKDEGlobals && localeString.isEmpty() && !KGlobal::_locale) {
       // Boot strap language
       bootLanguage = KLocale::_initLanguage(pConfig);
       setLocaleString(bootLanguage.utf8());
    }

    bFileImmutable = false;
    QStringList list = KGlobal::dirs()->
      findAllResources(resType, mfileName);

    QStringList::ConstIterator it;

    for (it = list.fromLast(); it != list.end(); --it) {

      QFile aConfigFile( *it );
      // we can already be sure that this file exists
      bool bIsLocal = (*it == mLocalFileName);
      if (aConfigFile.open( IO_ReadOnly )) {
         parseSingleConfigFile( aConfigFile, 0L, false, !bIsLocal );
         aConfigFile.close();
         if (bFileImmutable)
            break;
      }
    }
    QString currentLanguage;
    if (!bootLanguage.isEmpty())
    {
       currentLanguage = KLocale::_initLanguage(pConfig);
       // If the file changed the language, we need to read the file again
       // with the new language setting.
       if (bootLanguage != currentLanguage)
       {
          bReadFile = true;
          setLocaleString(currentLanguage.utf8());
       }
    }
  }
  if (bFileImmutable)
     mConfigState = KConfigBase::ReadOnly;

  return true;
}

#ifdef HAVE_MMAP
#ifdef SIGBUS
static const char **mmap_pEof;

static void mmap_sigbus_handler(int)
{
   *mmap_pEof = 0;
   write(2, "SIGBUS\n", 7);
   signal(SIGBUS, mmap_sigbus_handler);
}
#endif
#endif

void KConfigINIBackEnd::parseSingleConfigFile(QFile &rFile,
					      KEntryMap *pWriteBackMap,
					      bool bGlobal, bool bDefault)
{
   void (*old_sighandler)(int) = 0;

   if (!rFile.isOpen()) // come back, if you have real work for us ;->
      return;

   //using kdDebug() here leads to an infinite loop
   //remove this for the release, aleXXX
   //qWarning("Parsing %s, global = %s default = %s",
   //           rFile.name().latin1(), bGlobal ? "true" : "false", bDefault ? "true" : "false");

   QCString aCurrentGroup("<default>");

   const char *s, *eof;
   QByteArray data;

   unsigned int ll = localeString.length();

#ifdef HAVE_MMAP
   const char *map = ( const char* ) mmap(0, rFile.size(), PROT_READ, MAP_PRIVATE,
                                          rFile.handle(), 0);

   if (map)
   {
      s = map;
      eof = s + rFile.size();

#ifdef SIGBUS      
      mmap_pEof = &eof;
      old_sighandler = signal(SIGBUS, mmap_sigbus_handler);
#endif
   }
   else
#endif
   {
      rFile.at(0);
      data = rFile.readAll();
      s = data.data();
      eof = s + data.size();
   }

   bool fileOptionImmutable = false;
   bool groupOptionImmutable = false;
   bool groupSkip = false;

   int line = 0;
   for(; s < eof; s++)
   {
      line++;

      while((s < eof) && isspace(*s) && (*s != '\n'))
         s++; //skip leading whitespace, shouldn't happen too often

      //skip empty lines, lines starting with #
      if ((s < eof) && ((*s == '\n') || (*s == '#')))
      {
    sktoeol:	//skip till end-of-line
         while ((s < eof) && (*s != '\n'))
            s++;
         continue; // Empty or comment or no keyword
      }
      const char *startLine = s;

      if (*s == '[')  //group
      {
         while ((s < eof) && (*s != '\n') && (*s != ']')) s++; // Search till end of group
         const char *e = s;
         while ((s < eof) && (*s != '\n')) s++; // Search till end of line / end of file
         if ((e >= eof) || (*e != ']'))
         {
            fprintf(stderr, "Invalid group header at %s:%d\n", rFile.name().latin1(), line);
            continue;
         }
         // group found; get the group name by taking everything in
         // between the brackets
         if ((e-startLine == 3) &&
             (startLine[1] == '$') &&
             (startLine[2] == 'i'))
         {
            fileOptionImmutable = true;
            continue;
         }

         aCurrentGroup = QCString(startLine + 1, e - startLine);
         //cout<<"found group ["<<aCurrentGroup<<"]"<<endl;

         // Backwards compatibility
         if (aCurrentGroup == "KDE Desktop Entry")
            aCurrentGroup = "Desktop Entry";

         groupOptionImmutable = fileOptionImmutable;

         e++;
         if ((e+2 < eof) && (*e++ == '[') && (*e++ == '$')) // Option follows
         {
            if (*e == 'i')
            {
               groupOptionImmutable = true;
            }
         }

         KEntryKey groupKey(aCurrentGroup, 0);
         KEntry entry = pConfig->lookupData(groupKey);
         groupSkip = entry.bImmutable;

         if (groupSkip)
            continue;

         entry.bImmutable = groupOptionImmutable;
         pConfig->putData(groupKey, entry, false);

         if (pWriteBackMap)
         {
            // add the special group key indicator
            (*pWriteBackMap)[groupKey] = entry;
         }

         continue;
      }
      if (groupSkip)
        goto sktoeol; // Skip entry

      bool optionImmutable = groupOptionImmutable;
      bool optionDeleted = false;
      bool optionExpand = false;
      const char *endOfKey = 0, *locale = 0, *elocale = 0;
      for (; (s < eof) && (*s != '\n'); s++)
      {
         if (*s == '=') //find the equal sign
         {
	    if (!endOfKey)
        	endOfKey = s;
            goto haveeq;
	 }
	 if (*s == '[') //find the locale or options.
	 {
            const char *option;
            const char *eoption;
	    endOfKey = s;
	    option = ++s;
	    for (;; s++)
	    {
		if ((s >= eof) || (*s == '\n') || (*s == '=')) {
		    fprintf(stderr, "Invalid entry (missing ']') at %s:%d\n", rFile.name().latin1(), line);
		    goto sktoeol;
		}
		if (*s == ']')
		    break;
	    }
	    eoption = s;
            if (*option != '$')
            {
              // Locale
              if (locale) {
		fprintf(stderr, "Invalid entry (second locale!?) at %s:%d\n", rFile.name().latin1(), line);
		goto sktoeol;
              }
              locale = option;
              elocale = eoption;
            }
            else
            {
              // Option
              while (option < eoption)
              {
                 option++;
                 if (*option == 'i')
                    optionImmutable = true;
                 else if (*option == 'e')
                    optionExpand = true;
                 else if (*option == 'd')
                 {
                    optionDeleted = true;
                    goto haveeq;
                 }
		 else if (*option == ']')
		    break;
              }
            }
         }
      }
      fprintf(stderr, "Invalid entry (missing '=') at %s:%d\n", rFile.name().latin1(), line);
      continue;

   haveeq:
      for (endOfKey--; ; endOfKey--)
      {
	 if (endOfKey < startLine)
	 {
	   fprintf(stderr, "Invalid entry (empty key) at %s:%d\n", rFile.name().latin1(), line);
	   goto sktoeol;
	 }
	 if (!isspace(*endOfKey))
	    break;
      }

      const char *st = ++s;
      while ((s < eof) && (*s != '\n')) s++; // Search till end of line / end of file

      if (locale) {
          unsigned int cl = static_cast<unsigned int>(elocale - locale);
          if ((ll != cl) || memcmp(locale, localeString.data(), ll))
          {
              // backward compatibility. C == en_US
              if ( cl != 1 || ll != 5 || memcmp(locale, "C", 1) || memcmp(localeString.data(), "en_US", 5)) {
                  //cout<<"mismatched locale '"<<QCString(locale, elocale-locale +1)<<"'"<<endl;
                  // We can ignore this one
                  if (!pWriteBackMap)
                      continue; // We just ignore it
                  // We just store it as is to be able to write it back later.
                  endOfKey = elocale;
                  locale = 0;
              }
          }
      }

      // insert the key/value line
      QCString key(startLine, endOfKey - startLine + 2);
      QCString val = printableToString(st, s - st);
      //qDebug("found key '%s' with value '%s'", key.data(), val.data());

      KEntryKey aEntryKey(aCurrentGroup, key);
      aEntryKey.bLocal = (locale != 0);
      aEntryKey.bDefault = bDefault;

      KEntry aEntry;
      aEntry.mValue = val;
      aEntry.bGlobal = bGlobal;
      aEntry.bImmutable = optionImmutable;
      aEntry.bDeleted = optionDeleted;
      aEntry.bExpand = optionExpand;
      aEntry.bNLS = (locale != 0);

      if (pWriteBackMap) {
         // don't insert into the config object but into the temporary
         // scratchpad map
         pWriteBackMap->insert(aEntryKey, aEntry);
      } else {
         // directly insert value into config object
         // no need to specify localization; if the key we just
         // retrieved was localized already, no need to localize it again.
         pConfig->putData(aEntryKey, aEntry, false);
      }
   }
   if (fileOptionImmutable)
      bFileImmutable = true;

#ifdef HAVE_MMAP
   if (map)
   {
      munmap(( char* )map, rFile.size());
#ifdef SIGBUS      
      signal(SIGBUS, old_sighandler);
#endif
   }
#endif
}


void KConfigINIBackEnd::sync(bool bMerge)
{
  // write-sync is only necessary if there are dirty entries
  if (!pConfig->isDirty())
    return;

  bool bEntriesLeft = true;

  // find out the file to write to (most specific writable file)
  // try local app-specific file first

  if (!mfileName.isEmpty()) {
    // Create the containing dir if needed
    if ((resType!="config") && mLocalFileName[0]=='/')
    {
       KURL path;
       path.setPath(mLocalFileName);
       QString dir=path.directory();
       KStandardDirs::makeDir(dir);
    }

    // Can we allow the write? We can, if the program
    // doesn't run SUID. But if it runs SUID, we must
    // check if the user would be allowed to write if
    // it wasn't SUID.
    if (checkAccess(mLocalFileName, W_OK)) {
      // is it writable?
      bEntriesLeft = writeConfigFile( mLocalFileName, false, bMerge );
    }
  }

  // only write out entries to the kdeglobals file if there are any
  // entries marked global (indicated by bEntriesLeft) and
  // the useKDEGlobals flag is set.
  if (bEntriesLeft && useKDEGlobals) {


    // can we allow the write? (see above)
    if (checkAccess ( mGlobalFileName, W_OK )) {
      writeConfigFile( mGlobalFileName, true, bMerge );
    }
  }

}

static void writeEntries(FILE *pStream, const KEntryMap& entryMap, bool defaultGroup, bool &firstEntry, const QCString &localeString)
{
  // now write out all other groups.
  QCString currentGroup;
  for (KEntryMapConstIterator aIt = entryMap.begin();
       aIt != entryMap.end(); ++aIt)
  {
     const KEntryKey &key = aIt.key();

     // Either proces the default group or all others
     if ((key.mGroup != "<default>") == defaultGroup)
        continue; // Skip

     // Skip default values and group headers.
     if ((key.bDefault) || key.mKey.isEmpty())
        continue; // Skip

     const KEntry &currentEntry = *aIt;

     KEntryMapConstIterator aTestIt = aIt;
     ++aTestIt;
     bool hasDefault = (aTestIt != entryMap.end());
     if (hasDefault)
     {
        const KEntryKey &defaultKey = aTestIt.key();
        if ((!defaultKey.bDefault) ||
            (defaultKey.mKey != key.mKey) ||
            (defaultKey.mGroup != key.mGroup) ||
            (defaultKey.bLocal != key.bLocal))
           hasDefault = false;
     }


     if (hasDefault)
     {
        // Entry had a default value
        if ((currentEntry.mValue == (*aTestIt).mValue) &&
            (currentEntry.bDeleted == (*aTestIt).bDeleted))
           continue; // Same as default, don't write.
     }
     else
     {
        // Entry had no default value.
        if (currentEntry.bDeleted)
           continue; // Don't write deleted entries if there is no default.
     }

     if (!defaultGroup && (currentGroup != key.mGroup)) {
	if (!firstEntry)
	    fprintf(pStream, "\n");
	currentGroup = key.mGroup;
	fprintf(pStream, "[%s]\n", currentGroup.data());
     }

     firstEntry = false;
     // it is data for a group
     fputs(key.mKey.data(), pStream); // Key

     if ( currentEntry.bNLS )
     {
        fputc('[', pStream);
        fputs(localeString.data(), pStream);
        fputc(']', pStream);
     }

     if (currentEntry.bDeleted)
     {
        fputs("[$d]\n", pStream); // Deleted
     }
     else
     {
        if (currentEntry.bImmutable || currentEntry.bExpand)
        {
           fputc('[', pStream);
           fputc('$', pStream);
           if (currentEntry.bImmutable)
              fputc('i', pStream);
           if (currentEntry.bExpand)
              fputc('e', pStream);
           
           fputc(']', pStream);
        }
        fputc('=', pStream);
        fputs(stringToPrintable(currentEntry.mValue).data(), pStream);
        fputc('\n', pStream);
     }
  } // for loop
}

bool KConfigINIBackEnd::writeConfigFile(QString filename, bool bGlobal,
					bool bMerge)
{
  KEntryMap aTempMap;
  bool bEntriesLeft = false;

  // is the config object read-only?
  if (pConfig->isReadOnly())
    return true; // pretend we wrote it

  if (bMerge)
  {
    // Read entries from disk
    QFile rConfigFile( filename );
    if (rConfigFile.open(IO_ReadOnly))
    {
       // fill the temporary structure with entries from the file
       parseSingleConfigFile( rConfigFile, &aTempMap, bGlobal, false );
       rConfigFile.close();
    }

    KEntryMap aMap = pConfig->internalEntryMap();

    // augment this structure with the dirty entries from the config object
    for (KEntryMapIterator aIt = aMap.begin();
          aIt != aMap.end(); ++aIt)
    {
      const KEntry &currentEntry = *aIt;
      if(aIt.key().bDefault)
      {
         aTempMap.replace(aIt.key(), currentEntry);
         continue;
      }

      if (!currentEntry.bDirty)
         continue;

      // only write back entries that have the same
      // "globality" as the file
      if (currentEntry.bGlobal != bGlobal)
      {
        // wrong "globality" - might have to be saved later
        bEntriesLeft = true;
        continue;
      }

      // put this entry from the config object into the
      // temporary map, possibly replacing an existing entry
      aTempMap.replace(aIt.key(), currentEntry);
    } // loop
  }
  else
  {
    // don't merge, just get the regular entry map and use that.
    aTempMap = pConfig->internalEntryMap();
    bEntriesLeft = true; // maybe not true, but we aren't sure
  }

  // OK now the temporary map should be full of ALL entries.
  // write it out to disk.

  // Check if file exists:
  int fileMode = -1;
  bool createNew = true;

  struct stat buf;
  if (lstat(QFile::encodeName(filename), &buf) == 0)
  {
     if (S_ISLNK(buf.st_mode))
     {
        // File is a symlink:
        if (stat(QFile::encodeName(filename), &buf) == 0)
        {
           // Don't create new file but write to existing file instead.
           createNew = false;
        }
     }
     else if (buf.st_uid == getuid())
     {
        // Preserve file mode if file exists and is owned by user.
        fileMode = buf.st_mode & 0777;
     }
     else
     {
        // File is not owned by user:
        // Don't create new file but write to existing file instead.
        createNew = false;
     }
  }

  KSaveFile *pConfigFile = 0;
  FILE *pStream = 0;

  if (createNew)
  {
     pConfigFile = new KSaveFile( filename, 0600 );

     if (pConfigFile->status() != 0)
     {
        delete pConfigFile;
        return bEntriesLeft;
     }

     if (!bGlobal && (fileMode == -1))
        fileMode = mFileMode;

     if (fileMode != -1)
     {
        fchmod(pConfigFile->handle(), fileMode);
     }

     pStream = pConfigFile->fstream();
  }
  else
  {
     // Open existing file.
     // We use open() to ensure that we call without O_CREAT.
     int fd = open( QFile::encodeName(filename), O_WRONLY | O_TRUNC);
     if (fd < 0)
        return bEntriesLeft;
     pStream = fdopen( fd, "w");
     if (!pStream)
     {
        close(fd);
        return bEntriesLeft;
     }
  }

  bool firstEntry = true;

  // Write default group
  writeEntries(pStream, aTempMap, true, firstEntry, localeString);

  // Write all other groups
  writeEntries(pStream, aTempMap, false, firstEntry, localeString);

  if (pConfigFile)
  {
     pConfigFile->close();
     delete pConfigFile;
  }
  else
  {
     fclose(pStream);
  }

  return bEntriesLeft;
}


void KConfigBackEnd::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

void KConfigINIBackEnd::virtual_hook( int id, void* data )
{ KConfigBackEnd::virtual_hook( id, data ); }

