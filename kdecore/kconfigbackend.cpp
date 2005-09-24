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
  the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
  Boston, MA 02110-1301, USA.
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
#include <setjmp.h>

#include <qdir.h>
#include <qfileinfo.h>
#include <qtextcodec.h>
#include <qtextstream.h>

#include "kconfigbackend.h"

#include "kapplication.h"
#include "kconfigbase.h"
#include "kconfigdata.h"
#include "kde_file.h"
#include "kglobal.h"
#include "klocale.h"
#include "kprocess.h"
#include "ksavefile.h"
#include "kstandarddirs.h"
#include "kurl.h"

extern bool checkAccess(const QString& pathname, int mode);
/* translate escaped escape sequences to their actual values. */
static QByteArray printableToString(const char *str, int l)
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

  QByteArray result(l + 1);
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

static QByteArray stringToPrintable(const QByteArray& str){
  QByteArray result(str.length()*2); // Maximum 2x as long as source string
  register char *r = result.data();
  register const char *s = str.data();

  if (!s) return QByteArray("");

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

static QByteArray decodeGroup(const char*s, int l)
{
  QByteArray result(l);
  register char *r = result.data();

  l--; // Correct for trailing \0
  while(l)
  {
    if ((*s == '[') && (l > 1))
    {
       if ((*(s+1) == '['))
       {
          l--;
          s++;
       }
    }
    if ((*s == ']') && (l > 1))
    {
       if ((*(s+1) == ']'))
       {
          l--;
          s++;
       }
    }
    *r++ = *s++;
    l--;
  }
  result.truncate(r - result.data());
  return result;
}

static QByteArray encodeGroup(const QByteArray &str)
{
  int l = str.length();
  QByteArray result(l*2+1);
  register char *r = result.data();
  register const char *s = str.data();
  while(l)
  {
    if ((*s == '[') || (*s == ']'))
       *r++ = *s;
    *r++ = *s++;
    l--;
  }
  result.truncate(r - result.data());
  return result;
}

class KConfigBackEnd::KConfigBackEndPrivate
{
public:
   QDateTime localLastModified;
   uint      localLastSize;
   KLockFile::Ptr localLockFile;
   KLockFile::Ptr globalLockFile;
};

void KConfigBackEnd::changeFileName(const QString &_fileName,
                                    const char * _resType,
                                    bool _useKDEGlobals)
{
   mfileName = _fileName;
   resType = _resType;
   useKDEGlobals = _useKDEGlobals;
   if (mfileName.isEmpty())
      mLocalFileName = QString::null;
   else if (!QDir::isRelativePath(mfileName))
      mLocalFileName = mfileName;
   else
      mLocalFileName = KGlobal::dirs()->saveLocation(resType) + mfileName;

   if (useKDEGlobals)
      mGlobalFileName = KGlobal::dirs()->saveLocation("config") +
	      QLatin1String("kdeglobals");
   else
      mGlobalFileName = QString::null;

   d->localLastModified = QDateTime();
   d->localLastSize = 0;
   d->localLockFile = 0;
   d->globalLockFile = 0;
}

KLockFile::Ptr KConfigBackEnd::lockFile(bool bGlobal)
{
   if (bGlobal)
   {
      if (d->globalLockFile)
         return d->globalLockFile;
      
      if (!mGlobalFileName.isEmpty())
      {
         d->globalLockFile = new KLockFile(mGlobalFileName+".lock");
         return d->globalLockFile;
      }
   }
   else
   {
      if (d->localLockFile)
         return d->localLockFile;
      
      if (!mLocalFileName.isEmpty())
      {
         d->localLockFile = new KLockFile(mLocalFileName+".lock");
         return d->localLockFile;
      }
   }
   return 0;
}

KConfigBackEnd::KConfigBackEnd(KConfigBase *_config,
			       const QString &_fileName,
			       const char * _resType,
			       bool _useKDEGlobals)
  : pConfig(_config), bFileImmutable(false), mConfigState(KConfigBase::NoAccess), mFileMode(-1)
{
   d = new KConfigBackEndPrivate;
   changeFileName(_fileName, _resType, _useKDEGlobals);
}

KConfigBackEnd::~KConfigBackEnd()
{
   delete d;
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
     QFileInfo info(mLocalFileName);
     d->localLastModified = info.lastModified();
     d->localLastSize = info.size();
  }

  // Parse all desired files from the least to the most specific.
  bFileImmutable = false;

  // Parse the general config files
  if (useKDEGlobals) {
    QStringList kdercs = KGlobal::dirs()->
      findAllResources("config", QLatin1String("kdeglobals"));

#ifdef Q_WS_WIN
    QString etc_kderc = QFile::decodeName( QByteArray(getenv("WINDIR")) + "\\kderc" );
#else
    QString etc_kderc = QLatin1String("/etc/kderc");
#endif

    if (checkAccess(etc_kderc, R_OK))
      kdercs += etc_kderc;

    kdercs += KGlobal::dirs()->
      findAllResources("config", QLatin1String("system.kdeglobals"));

    QListIterator<QString> it( kdercs );
    it.toBack();
    while (it.hasPrevious()) {
      QFile aConfigFile( it.previous() );
      if (!aConfigFile.open( QIODevice::ReadOnly ))
	   continue;
      parseSingleConfigFile( aConfigFile, 0L, true, 
                             (aConfigFile.fileName() != mGlobalFileName) );
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
       setLocaleString(bootLanguage.toUtf8());
    }

    bFileImmutable = false;
    QStringList list;
    if ( !QDir::isRelativePath(mfileName) )
       list << mfileName;
    else
       list = KGlobal::dirs()->findAllResources(resType, mfileName);

    QListIterator<QString> it( list );
    it.toBack();
    while (it.hasPrevious()) {
      QFile aConfigFile( it.previous() );
      // we can already be sure that this file exists
      bool bIsLocal = (aConfigFile.fileName() == mLocalFileName);
      if (aConfigFile.open( QIODevice::ReadOnly )) {
         parseSingleConfigFile( aConfigFile, 0L, false, !bIsLocal );
         aConfigFile.close();
         if (bFileImmutable)
            break;
      }
    }
    if (KGlobal::dirs()->isRestrictedResource(resType, mfileName))
       bFileImmutable = true;
    QString currentLanguage;
    if (!bootLanguage.isEmpty())
    {
       currentLanguage = KLocale::_initLanguage(pConfig);
       // If the file changed the language, we need to read the file again
       // with the new language setting.
       if (bootLanguage != currentLanguage)
       {
          bReadFile = true;
          setLocaleString(currentLanguage.toUtf8());
       }
    }
  }
  if (bFileImmutable)
     mConfigState = KConfigBase::ReadOnly;

  return true;
}

#ifdef HAVE_MMAP
#ifdef SIGBUS
static sigjmp_buf mmap_jmpbuf;
struct sigaction mmap_old_sigact;

extern "C" {
   static void mmap_sigbus_handler(int)
   {
      siglongjmp (mmap_jmpbuf, 1);
   }
}
#endif
#endif

extern bool kde_kiosk_exception;

void KConfigINIBackEnd::parseSingleConfigFile(QFile &rFile,
					      KEntryMap *pWriteBackMap,
					      bool bGlobal, bool bDefault)
{
   const char *s; // May get clobbered by sigsetjump, but we don't use them afterwards.
   const char *eof; // May get clobbered by sigsetjump, but we don't use them afterwards.
   QByteArray data;

   if (!rFile.isOpen()) // come back, if you have real work for us ;->
      return;

   //using kdDebug() here leads to an infinite loop
   //remove this for the release, aleXXX
   //qWarning("Parsing %s, global = %s default = %s",
   //           rFile.name().latin1(), bGlobal ? "true" : "false", bDefault ? "true" : "false");

   QByteArray aCurrentGroup("<default>");

   unsigned int ll = localeString.length();

#ifdef HAVE_MMAP
   static volatile const char *map;
   map = ( const char* ) mmap(0, rFile.size(), PROT_READ, MAP_PRIVATE,
                                          rFile.handle(), 0);

   if ( map != MAP_FAILED )
   {
      s = (const char*) map;
      eof = s + rFile.size();

#ifdef SIGBUS
      struct sigaction act;
      act.sa_handler = mmap_sigbus_handler;
      sigemptyset( &act.sa_mask );
#ifdef SA_ONESHOT
      act.sa_flags = SA_ONESHOT;
#else
      act.sa_flags = SA_RESETHAND;
#endif      
      sigaction( SIGBUS, &act, &mmap_old_sigact );

      if (sigsetjmp (mmap_jmpbuf, 1))
      {
qWarning("SIGBUS while reading %s", rFile.fileName().latin1());
         munmap(( char* )map, rFile.size());
         sigaction (SIGBUS, &mmap_old_sigact, 0);
         return;
      }
#endif
   }
   else
#endif
   {
      rFile.seek(0);
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
         // In a group [[ and ]] have a special meaning
         while ((s < eof) && (*s != '\n')) 
         {
            if (*s == ']')
            {
               if ((s+1 < eof) && (*(s+1) == ']'))
                  s++; // Skip "]]"
               else
                  break;
            }

            s++; // Search till end of group
         }
         const char *e = s;
         while ((s < eof) && (*s != '\n')) s++; // Search till end of line / end of file
         if ((e >= eof) || (*e != ']'))
         {
            fprintf(stderr, "Invalid group header at %s:%d\n", rFile.fileName().latin1(), line);
            continue;
         }
         // group found; get the group name by taking everything in
         // between the brackets
         if ((e-startLine == 3) &&
             (startLine[1] == '$') &&
             (startLine[2] == 'i'))
         {
            if (!kde_kiosk_exception)
               fileOptionImmutable = true;
            continue;
         }

         aCurrentGroup = decodeGroup(startLine + 1, e - startLine);
         //cout<<"found group ["<<aCurrentGroup<<"]"<<endl;

         groupOptionImmutable = fileOptionImmutable;

         e++;
         if ((e+2 < eof) && (*e++ == '[') && (*e++ == '$')) // Option follows
         {
            if ((*e == 'i') && !kde_kiosk_exception)
            {
               groupOptionImmutable = true;
            }
         }

         KEntryKey groupKey(aCurrentGroup, 0);
         KEntry entry = pConfig->lookupData(groupKey);
         groupSkip = entry.bImmutable;

         if (groupSkip && !bDefault)
            continue;

         entry.bImmutable |= groupOptionImmutable;
         pConfig->putData(groupKey, entry, false);

         if (pWriteBackMap)
         {
            // add the special group key indicator
            (*pWriteBackMap)[groupKey] = entry;
         }

         continue;
      }
      if (groupSkip && !bDefault)
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
		    fprintf(stderr, "Invalid entry (missing ']') at %s:%d\n", rFile.fileName().latin1(), line);
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
		fprintf(stderr, "Invalid entry (second locale!?) at %s:%d\n", rFile.fileName().latin1(), line);
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
                 if ((*option == 'i') && !kde_kiosk_exception)
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
      fprintf(stderr, "Invalid entry (missing '=') at %s:%d\n", rFile.fileName().latin1(), line);
      continue;

   haveeq:
      for (endOfKey--; ; endOfKey--)
      {
	 if (endOfKey < startLine)
	 {
	   fprintf(stderr, "Invalid entry (empty key) at %s:%d\n", rFile.fileName().latin1(), line);
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
              if ( cl != 1 || ll != 5 || *locale != 'C' || memcmp(localeString.data(), "en_US", 5)) {
                  //cout<<"mismatched locale '"<<QByteArray(locale, elocale-locale +1)<<"'"<<endl;
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
      QByteArray key(startLine, endOfKey - startLine + 1);
      QByteArray val = printableToString(st, s - st);
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
      sigaction (SIGBUS, &mmap_old_sigact, 0);
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
    if ((resType!="config") && !QDir::isRelativePath(mLocalFileName))
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
      // File is writable
      KLockFile::Ptr lf;

      bool mergeLocalFile = bMerge;
      // Check if the file has been updated since.
      if (mergeLocalFile)
      {
         lf = lockFile(false); // Lock file for local file
         if (lf && lf->isLocked())
            lf = 0; // Already locked, we don't need to lock/unlock again

         if (lf) 
         {
            lf->lock( KLockFile::LockForce );
            // But what if the locking failed? Ignore it for now...
         }
         
         QFileInfo info(mLocalFileName);
         if ((d->localLastSize == info.size()) &&
             (d->localLastModified == info.lastModified()))
         {
            // Not changed, don't merge.
            mergeLocalFile = false;
         }
         else
         {
            // Changed...
            d->localLastModified = QDateTime();
            d->localLastSize = 0;
         }
      }

      bEntriesLeft = writeConfigFile( mLocalFileName, false, mergeLocalFile );
      
      // Only if we didn't have to merge anything can we use our in-memory state
      // the next time around. Otherwise the config-file may contain entries
      // that are different from our in-memory state which means we will have to 
      // do a merge from then on. 
      // We do not automatically update the in-memory state with the on-disk 
      // state when writing the config to disk. We only do so when 
      // KCOnfig::reparseConfiguration() is called.
      // For KDE 4.0 we may wish to reconsider that.
      if (!mergeLocalFile)
      {
         QFileInfo info(mLocalFileName);
         d->localLastModified = info.lastModified();
         d->localLastSize = info.size();
      }
      if (lf) lf->unlock();
    }
  }

  // only write out entries to the kdeglobals file if there are any
  // entries marked global (indicated by bEntriesLeft) and
  // the useKDEGlobals flag is set.
  if (bEntriesLeft && useKDEGlobals) {

    // can we allow the write? (see above)
    if (checkAccess ( mGlobalFileName, W_OK )) {
      KLockFile::Ptr lf = lockFile(true); // Lock file for global file
      if (lf && lf->isLocked())
         lf = 0; // Already locked, we don't need to lock/unlock again

      if (lf) 
      {
         lf->lock( KLockFile::LockForce );
         // But what if the locking failed? Ignore it for now...
      }
      writeConfigFile( mGlobalFileName, true, bMerge ); // Always merge
      if (lf) lf->unlock();
    }
  }

}

static void writeEntries(FILE *pStream, const KEntryMap& entryMap, bool defaultGroup, bool &firstEntry, const QByteArray &localeString)
{
  // now write out all other groups.
  QByteArray currentGroup;
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
	fprintf(pStream, "[%s]\n", encodeGroup(currentGroup).data());
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

bool KConfigINIBackEnd::getEntryMap(KEntryMap &aTempMap, bool bGlobal,
                                    QFile *mergeFile)
{
  bool bEntriesLeft = false;
  bFileImmutable = false;

  // Read entries from disk
  if (mergeFile && mergeFile->open(QIODevice::ReadOnly))
  {
     // fill the temporary structure with entries from the file
     parseSingleConfigFile(*mergeFile, &aTempMap, bGlobal, false );

     if (bFileImmutable) // File has become immutable on disk
        return bEntriesLeft;
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

    if (mergeFile && !currentEntry.bDirty)
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
    KEntryMapIterator aIt2 = aTempMap.find(aIt.key());
    if (aIt2 != aTempMap.end() && (*aIt2).bImmutable)
       continue; // Bail out if the on-disk entry is immutable

    aTempMap.insert(aIt.key(), currentEntry, true);
  } // loop

  return bEntriesLeft;
}

bool KConfigINIBackEnd::writeConfigFile(const QString &filename, bool bGlobal,
					bool bMerge)
{
  // is the config object read-only?
  if (pConfig->isReadOnly())
    return true; // pretend we wrote it

  KEntryMap aTempMap;
  QFile *mergeFile = (bMerge ? new QFile(filename) : 0);
  bool bEntriesLeft = getEntryMap(aTempMap, bGlobal, mergeFile);
  delete mergeFile;
  if (bFileImmutable)
    return true; // pretend we wrote it

  // OK now the temporary map should be full of ALL entries.
  // write it out to disk.

  // Check if file exists:
  int fileMode = -1;
  bool createNew = true;

  KDE_struct_stat buf;
  if (KDE_stat(QFile::encodeName(filename), &buf) == 0)
  {
     if (buf.st_uid == getuid())
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
     int fd = KDE_open( QFile::encodeName(filename), O_WRONLY | O_TRUNC );
     if (fd < 0)
     {
        return bEntriesLeft;
     }
     pStream = KDE_fdopen( fd, "w");
     if (!pStream)
     {
        close(fd);
        return bEntriesLeft;
     }
  }

  writeEntries(pStream, aTempMap);

  if (pConfigFile)
  {
     bool bEmptyFile = (ftell(pStream) == 0);
     if ( bEmptyFile && ((fileMode == -1) || (fileMode == 0600)) )
     {
        // File is empty and doesn't have special permissions: delete it.
        ::unlink(QFile::encodeName(filename));
        pConfigFile->abort();
     }
     else
     {
        // Normal case: Close the file
        pConfigFile->close();
     }
     delete pConfigFile;
  }
  else
  {
     fclose(pStream);
  }

  return bEntriesLeft;
}

void KConfigINIBackEnd::writeEntries(FILE *pStream, const KEntryMap &aTempMap)
{
  bool firstEntry = true;

  // Write default group
  ::writeEntries(pStream, aTempMap, true, firstEntry, localeString);

  // Write all other groups
  ::writeEntries(pStream, aTempMap, false, firstEntry, localeString);
}

void KConfigBackEnd::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

void KConfigINIBackEnd::virtual_hook( int id, void* data )
{ KConfigBackEnd::virtual_hook( id, data ); }

bool KConfigBackEnd::checkConfigFilesWritable(bool warnUser)
{
  // WARNING: Do NOT use the event loop as it may not exist at this time.
  bool allWritable = true;
  QString errorMsg;
  if ( !mLocalFileName.isEmpty() && !bFileImmutable && !checkAccess(mLocalFileName,W_OK) )
  {
    errorMsg = i18n("Will not save configuration.\n");
    allWritable = false;
    errorMsg += i18n("Configuration file \"%1\" not writable.\n").arg(mLocalFileName);
  }
  // We do not have an immutability flag for kdeglobals. However, making kdeglobals mutable while making
  // the local config file immutable is senseless.
  if ( !mGlobalFileName.isEmpty() && useKDEGlobals && !bFileImmutable && !checkAccess(mGlobalFileName,W_OK) )
  {
    if ( errorMsg.isEmpty() )
      errorMsg = i18n("Will not save configuration.\n");
    errorMsg += i18n("Configuration file \"%1\" not writable.\n").arg(mGlobalFileName);
    allWritable = false;
  }

  if (warnUser && !allWritable)
  {
    // Note: We don't ask the user if we should not ask this question again because we can't save the answer.
    errorMsg += i18n("Please contact your system administrator.");
    QString cmdToExec = KStandardDirs::findExe(QString("kdialog"));
    KApplication *app = kapp;
    if (!cmdToExec.isEmpty() && app)
    {
      KProcess lprocess;
      lprocess << cmdToExec << "--title" << app->instanceName() << "--msgbox" << errorMsg.toLocal8Bit();
      lprocess.start( KProcess::Block );
    }
  }
  return allWritable;
}
