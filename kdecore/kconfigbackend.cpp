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

#undef Unsorted
#include <qdir.h>
#include <qfileinfo.h>
#include <qtextcodec.h>
#include <qtextstream.h>

#include "kconfigbackend.h"
#include "kconfigbase.h"
#include <kglobal.h>
#include <kstddirs.h>
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
  char *r = (char *) result.data();

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
  register char *r = (char *) result.data();
  register char *s = (char *) str.data();

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

KConfigBackEnd::KConfigBackEnd(KConfigBase *_config,
			       const QString &_fileName,
			       const char * _resType,
			       bool _useKDEGlobals)
  : pConfig(_config), fileName(_fileName),
    resType(_resType), useKDEGlobals(_useKDEGlobals)
{
}

bool KConfigINIBackEnd::parseConfigFiles()
{
  // Parse all desired files from the least to the most specific.

  // Parse the general config files
  if (useKDEGlobals) {

    QStringList kdercs = KGlobal::dirs()->
      findAllResources("config", QString::fromLatin1("kdeglobals"));

    if (checkAccess(QString::fromLatin1("/etc/kderc"), R_OK))
      kdercs += QString::fromLatin1("/etc/kderc");

    kdercs += KGlobal::dirs()->
      findAllResources("config", QString::fromLatin1("system.kdeglobals"));

    QStringList::ConstIterator it;

    for (it = kdercs.fromLast(); it != kdercs.end(); it--) {

      QFile aConfigFile( *it );
      if (!aConfigFile.open( IO_ReadOnly ))
	   continue;
      parseSingleConfigFile( aConfigFile, 0L, true );
      aConfigFile.close();
    }
  }

  if (!fileName.isEmpty()) {
    QStringList list = KGlobal::dirs()->
      findAllResources(resType, fileName);

    QStringList::ConstIterator it;

    for (it = list.fromLast(); it != list.end(); it--) {

      QFile aConfigFile( *it );
      // we can already be sure that this file exists
      aConfigFile.open( IO_ReadOnly );
      parseSingleConfigFile( aConfigFile, 0L, false );
      aConfigFile.close();
    }
  }

  return true;
}

KConfigBase::ConfigState KConfigINIBackEnd::getConfigState() const
{
    if (fileName.isEmpty())
	return KConfigBase::NoAccess;

    QString aLocalFileName = KGlobal::dirs()->saveLocation("config") +
      fileName;
    // Can we allow the write? We can, if the program
    // doesn't run SUID. But if it runs SUID, we must
    // check if the user would be allowed to write if
    // it wasn't SUID.
    if (checkAccess(aLocalFileName, W_OK|R_OK))
	return KConfigBase::ReadWrite;
    else
	if (checkAccess(aLocalFileName, R_OK))
	    return KConfigBase::ReadOnly;

    return KConfigBase::NoAccess;
}

//#include <iostream.h>
void KConfigINIBackEnd::parseSingleConfigFile(QFile &rFile,
					      KEntryMap *pWriteBackMap,
					      bool bGlobal)
{
   if (!rFile.isOpen()) // come back, if you have real work for us ;->
      return;

   //using kdDebug() here leads to an infinite loop
   //remove this for the release, aleXXX
   //cout<<"******** parsing "<<rFile.name().latin1()<<endl;

   QCString aCurrentGroup("<default>");

   const char *s, *eof;
   QByteArray data;
#ifdef HAVE_MMAP
   const char *map = (const char *) 
	mmap(0, rFile.size(), PROT_READ, MAP_PRIVATE, rFile.handle(), 0);
   if (map)
   {
      s = map;
      eof = s + rFile.size();
   }
   else
#endif
   {
      rFile.at(0);
      data = rFile.readAll();
      s = data.data();
      eof = s + data.size();
   }

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
         while ((s < eof) && (*s != '\n')) s++; // Search till end of line / end of file
         const char *e = s - 1;
         while ((e > startLine) && isspace(*e)) e--;
         if (*e != ']')
         {
            fprintf(stderr, "Invalid group header at %s:%d\n", rFile.name().latin1(), line);
            continue;
         }
         // group found; get the group name by taking everything in
         // between the brackets
         aCurrentGroup = QCString(startLine + 1, e - startLine);
         //cout<<"found group ["<<aCurrentGroup<<"]"<<endl;

         // Backwards compatibility
         if (aCurrentGroup == "KDE Desktop Entry")
            aCurrentGroup = "Desktop Entry";

         if (pWriteBackMap)
         {
            // add the special group key indicator
            KEntryKey groupKey(aCurrentGroup, 0);
            pWriteBackMap->insert(groupKey, KEntry());
         }

         continue;
      }

      const char *endOfKey = 0, *locale = 0, *elocale = 0;
      for (; (s < eof) && (*s != '\n'); s++)
      {
         if (*s == '=') //find the equal sign
         {
	    if (!endOfKey)
        	endOfKey = s;
            goto haveeq;
	 }
	 if (*s == '[') //find the locale
	 {
	    if (locale) {
		fprintf(stderr, "Invalid entry (second locale!?) at %s:%d\n", rFile.name().latin1(), line);
		goto sktoeol;
	    }
	    endOfKey = s;
	    locale = ++s;
	    for (;; s++)
	    {
		if ((s >= eof) || (*s == '\n') || (*s == '=')) {
		    fprintf(stderr, "Invalid entry (missing ']') at %s:%d\n", rFile.name().latin1(), line);
		    goto sktoeol;
		}
		if (*s == ']')
		    break;
	    }
	    elocale = s;
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
	  uint ll = localeString.length();
          if ((ll != (uint) (elocale - locale)) || 
	      memcmp(locale, localeString.data(), ll))
          {
            //cout<<"mismatched locale '"<<QCString(locale, elocale-locale +1)<<"'"<<endl;
            // We can ignore this one
            if (!pWriteBackMap)
               continue; // We just ignore it
            // We just store it as is to be able to write it back later.
	    endOfKey = elocale;
            locale = 0;
          }
      }

      // insert the key/value line
      QCString key(startLine, endOfKey - startLine + 2);
      QCString val = printableToString(st, s - st);
      //cout<<"found key '"<<key<<"' with value '"<<val<<"'"<<endl;

      KEntryKey aEntryKey(aCurrentGroup, key);
      aEntryKey.bLocal = (locale != 0);

      KEntry aEntry;
      aEntry.mValue = val;
      aEntry.bGlobal = bGlobal;
      aEntry.bNLS = (locale != 0);

      if (pWriteBackMap) {
         // don't insert into the config object but into the temporary
         // scratchpad map
         pWriteBackMap->insert(aEntryKey, aEntry);
      } else {
         // directly insert value into config object
         // no need to specify localization; if the key we just
         // retrieved was localized already, no need to localize it again.
        pConfig->putData(aEntryKey, aEntry);
      }
   }
#ifdef HAVE_MMAP
   if (map)
      munmap((char *)map, rFile.size());
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

  if (!fileName.isEmpty()) {
    QString aLocalFileName;
    if (fileName[0] == '/') {
       aLocalFileName = fileName;
    } else {
       aLocalFileName = KGlobal::dirs()->saveLocation(resType) + fileName;
    }

    // Create the containing dir if needed
    if ((resType!="config") && aLocalFileName[0]=='/')
    {
       KURL path;
       path.setPath(aLocalFileName);
       QString dir=path.directory();
       KStandardDirs::makeDir(dir);
    }

    // Can we allow the write? We can, if the program
    // doesn't run SUID. But if it runs SUID, we must
    // check if the user would be allowed to write if
    // it wasn't SUID.
    if (checkAccess(aLocalFileName, W_OK)) {
      // is it writable?
      bEntriesLeft = writeConfigFile( aLocalFileName, false, bMerge );
    }
  }

  // only write out entries to the kdeglobals file if there are any
  // entries marked global (indicated by bEntriesLeft) and
  // the useKDEGlobals flag is set.
  if (bEntriesLeft && useKDEGlobals) {

    QString aFileName = KGlobal::dirs()->saveLocation("config") +
      QString::fromLatin1("kdeglobals");

    // can we allow the write? (see above)
    if (checkAccess ( aFileName, W_OK )) {
      writeConfigFile( aFileName, true, bMerge );
    }
  }

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
       parseSingleConfigFile( rConfigFile, &aTempMap, bGlobal );
       rConfigFile.close();
    }

    KEntryMap aMap = pConfig->internalEntryMap();

    // augment this structure with the dirty entries from the config object
    for (KEntryMapIterator aInnerIt = aMap.begin();
          aInnerIt != aMap.end(); ++aInnerIt)
    {
      KEntry currentEntry = *aInnerIt;

      if (!currentEntry.bDirty)
         continue;

      // only write back entries that have the same
      // "globality" as the file
      if (currentEntry.bGlobal == bGlobal)
      {
        KEntryKey entryKey = aInnerIt.key();

        // put this entry from the config object into the
        // temporary map, possibly replacing an existing entry
        if (aTempMap.contains(entryKey))
        {
          aTempMap.replace(entryKey, currentEntry);
        }
	else
        {
	  // add special group key and then the entry
	  KEntryKey groupKey(entryKey.mGroup, 0);
	  if (!aTempMap.contains(groupKey))
	    aTempMap.insert(groupKey, KEntry());

	  aTempMap.insert(entryKey, currentEntry);
        }
      }
      else
      {
        // wrong "globality" - might have to be saved later
        bEntriesLeft = true;
      }
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

  KSaveFile rConfigFile( filename, 0600 );

  if (rConfigFile.status() != 0)
     return bEntriesLeft;

  FILE *pStream = rConfigFile.fstream();

  // write back -- start with the default group
  KEntryMapConstIterator aWriteIt;
  for (aWriteIt = aTempMap.begin(); aWriteIt != aTempMap.end(); ++aWriteIt) {

      if ( aWriteIt.key().mGroup == "<default>" && !aWriteIt.key().mKey.isEmpty() ) {
	  if ( (*aWriteIt).bNLS )
          {
              fprintf(pStream, "%s[%s]=%s\n",
                    aWriteIt.key().mKey.data(), localeString.data(),
                    stringToPrintable((*aWriteIt).mValue).data());
          }
	  else
          {
              // not localized
              fprintf(pStream, "%s=%s\n",
                    aWriteIt.key().mKey.data(),
                    stringToPrintable((*aWriteIt).mValue).data());
          }
      }
  } // for loop

  // now write out all other groups.
  QCString currentGroup;
  for (aWriteIt = aTempMap.begin(); aWriteIt != aTempMap.end(); ++aWriteIt) {
    // check if it's not the default group (which has already been written)
    if (aWriteIt.key().mGroup == "<default>")
      continue;

    if ( currentGroup != aWriteIt.key().mGroup ) {
	if (!currentGroup.isEmpty())
	    fprintf(pStream, "\n");
	currentGroup = aWriteIt.key().mGroup;
	fprintf(pStream, "[%s]\n", aWriteIt.key().mGroup.data());
    }

    if (aWriteIt.key().mKey.isEmpty()) {
      // we found a special group key, ignore it
    } else {
      // it is data for a group
      if ( (*aWriteIt).bNLS )
      {
          fprintf(pStream, "%s[%s]=%s\n",
                   aWriteIt.key().mKey.data(), localeString.data(),
                   stringToPrintable((*aWriteIt).mValue).data());
      }
      else
      {
          // not localized
          fprintf(pStream, "%s=%s\n",
                    aWriteIt.key().mKey.data(),
                    stringToPrintable((*aWriteIt).mValue).data());
      }
    }
  } // for loop

  rConfigFile.close();

  return bEntriesLeft;
}
