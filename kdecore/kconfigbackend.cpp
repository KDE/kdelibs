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

#include "config.h"

#include <qdir.h>
#include <qfileinfo.h>
#include <qtextstream.h>

#include <kapp.h>

#include "kconfigbackend.h"
#include "kconfigbase.h"
#include <kglobal.h>
#include <kstddirs.h>

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_TEST
#include <test.h>
#endif

/* translate escaped escape sequences to their actual values. */
static QString printableToString(const QString& s)
{
  if (!s.contains('\\'))
    return s;
  QString result;
  unsigned int i = 0;
  if (s.length()>1){ // remember: s.length() is unsigned....
    for (i=0;i<s.length()-1;i++){
      if (s[i] == '\\'){
        i++;
        if (s[i] == '\\')
          result.insert(result.length(), s[i]);
        else if (s[i] == 'n')
          result.append("\n");
        else if (s[i] == 't')
          result.append("\t");
        else if (s[i] == 'r')
          result.append("\r");
        else {
          result.append("\\");
          result.insert(result.length(), s[i]);
        }
      }
      else
        result.insert(result.length(), s[i]);
    }
  }
  if (i<s.length())
    result.insert(result.length(), s[i]);
  return result;
}

static QString stringToPrintable(const QString& s){
  QString result;
  unsigned int i;
  for (i=0;i<s.length();i++){
    if (s[i] == '\n')
      result.append("\\n");
    else if (s[i] == '\t')
      result.append("\\t");
    else if (s[i] == '\r')
      result.append("\\r");
    else if (s[i] == '\\')
      result.append("\\\\");
    else
      result.insert(result.length(), s[i]);
  }
  return result;
}

KConfigBackEnd::KConfigBackEnd(KConfigBase *_config, 
			       const QString &_fileName, 
			       const QString &_resType,
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
      findAllResources("config", "kdeglobals");
    
    if (!access("/etc/kderc", R_OK)) 
      kdercs += "/etc/kderc";
    
    kdercs += KGlobal::dirs()->
      findAllResources("config", "system.kdeglobals");
    
    QStringList::ConstIterator it;
    
    for (it = kdercs.fromLast(); it != kdercs.end(); it--) {
      
      QFile aConfigFile( *it );
      aConfigFile.open( IO_ReadOnly );
      parseSingleConfigFile( aConfigFile, 0L, true );
      aConfigFile.close();
    }
  }
  
  if (!fileName.isEmpty()) {
    
    QStringList list = KGlobal::dirs()->
      findAllResources(resType, fileName, true);
    
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

    QString aLocalFileName = KGlobal::dirs()->getSaveLocation("config") + 
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

void KConfigINIBackEnd::parseSingleConfigFile(QFile &rFile,
					   KEntryMap *pWriteBackMap,
					   bool bGlobal)
{
  if (!rFile.isOpen()) // come back, if you have real work for us ;->
    return;

  QString aCurrentLine;
  QString aCurrentGroup("<default>");

  // reset the stream's device
  rFile.at(0);
  QTextStream aStream( &rFile );
  while (!aStream.atEnd()) {
    aCurrentLine = aStream.readLine();

    // check for a group
    int nLeftBracket = aCurrentLine.find( '[' );
    int nRightBracket = aCurrentLine.find( ']', 1 );
    if( nLeftBracket == 0 && nRightBracket != -1 ) {
      // group found; get the group name by taking everything in
      // between the brackets
      aCurrentGroup =
	aCurrentLine.mid( 1, nRightBracket-1 );

      if (pWriteBackMap) {
	// add the special group key indicator
	KEntryKey groupKey = { aCurrentGroup, QString() };
	pWriteBackMap->insert(groupKey, KEntry());
      }
      continue;
    };

    if( aCurrentLine[0] == '#' )
      // comment character in the first column, skip the line
      continue;

    int nEqualsPos = aCurrentLine.find( '=' );
    if( nEqualsPos == -1 )
      // no equals sign: incorrect or empty line, skip it
      continue;

    // insert the key/value line
    QString key = aCurrentLine.left(nEqualsPos).stripWhiteSpace();
    QString val = printableToString(aCurrentLine.right(aCurrentLine.length() - nEqualsPos - 1)).stripWhiteSpace();

    KEntryKey aEntryKey = { aCurrentGroup, key };
    KEntry aEntry;
    aEntry.aValue = val;
    aEntry.bGlobal = bGlobal;

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
}

void KConfigINIBackEnd::sync(bool bMerge)
{
  // write-sync is only necessary if there are dirty entries
  if (!pConfig->isDirty())
    return;

  bool bEntriesLeft = true;
  bool bLocalGood = false;

  
  // find out the file to write to (most specific writable file)
  // try local app-specific file first
  
  if (!fileName.isEmpty()) {
    QString aLocalFileName;
    if (fileName[0] == '/') {
       aLocalFileName = fileName;
    } else {
       aLocalFileName = KGlobal::dirs()->getSaveLocation("config") + fileName;
    }

    // Can we allow the write? We can, if the program
    // doesn't run SUID. But if it runs SUID, we must
    // check if the user would be allowed to write if
    // it wasn't SUID.
    if (checkAccess(aLocalFileName, W_OK)) {
      // is it writable?
      QFile aConfigFile(aLocalFileName);
      aConfigFile.open( IO_ReadWrite );
      // Set uid/gid (neccesary for SUID programs)
      chown(aConfigFile.name().ascii(), getuid(), getgid());
      bEntriesLeft = writeConfigFile( aConfigFile, false, bMerge );
      bLocalGood = true;
      aConfigFile.close();
    }
  }

  // only write out entries to the kdeglobals file if there are any
  // entries marked global (indicated by bEntriesLeft) and
  // the useKDEGlobals flag is set.
  if (bEntriesLeft && useKDEGlobals) {

    QString aFileName = KGlobal::dirs()->getSaveLocation("config") + 
      "kdeglobals";
    QFile aConfigFile( aFileName );
    
    // can we allow the write? (see above)
    if (checkAccess ( aFileName, W_OK )) {
      aConfigFile.open( IO_ReadWrite );
      // Set uid/gid (neccesary for SUID programs)
      chown(aConfigFile.name().ascii(), getuid(), getgid());
      writeConfigFile( aConfigFile, true, bMerge );
      aConfigFile.close();
    }
  }
  
}

bool KConfigINIBackEnd::writeConfigFile(QFile &rConfigFile, bool bGlobal,
					bool bMerge)
{
  KEntryMap aTempMap;
  bool bEntriesLeft = false;

  // is the config object read-only?
  if (pConfig->isReadOnly())
    return true; // pretend we wrote it


  QTextStream* pStream;
  if (bMerge) {
    // merge entries on disk

    pStream = new QTextStream( &rConfigFile );

    // fill the temporary structure with entries from the file
    parseSingleConfigFile( rConfigFile, &aTempMap, bGlobal );

    KEntryMap aMap = pConfig->internalEntryMap();

    // augment this structure with the dirty entries from the config object
    for (KEntryMapIterator aInnerIt = aMap.begin();
	 aInnerIt != aMap.end(); ++aInnerIt) {
      KEntry currentEntry = *aInnerIt;

      if (currentEntry.bDirty) {
	// only write back entries that have the same
	// "globality" as the file
	if (currentEntry.bGlobal == bGlobal) {
	  KEntryKey entryKey = aInnerIt.key();
	
	  // put this entry from the config object into the
	  // temporary map, possibly replacing an existing entry
	  if (aTempMap.contains(entryKey))
	    aTempMap.replace(entryKey, currentEntry);
	  else {
	    // add special group key and then the entry
	    KEntryKey groupKey = { entryKey.group, QString() };
	    if (!aTempMap.contains(groupKey))
	      aTempMap.insert(groupKey, KEntry());

	    aTempMap.insert(entryKey, currentEntry);
	  }
	} else
	  // wrong "globality" - might have to be saved later
	  bEntriesLeft = true;
      }
    } // loop

    // truncate file
    delete pStream;
  } else {
    // don't merge, just get the regular entry map and use that.
    aTempMap = pConfig->internalEntryMap();
    bEntriesLeft = true; // maybe not true, but we aren't sure
  }

  // OK now the temporary map should be full of ALL entries.
  // write it out to disk.
  rConfigFile.close();

  // Does program run SUID and user would not be allowed to write
  // to the file, if it doesn't run  SUID?
  if (!checkAccess(rConfigFile.name(), W_OK)) // write not allowed
    return false; // can't allow to write config data.


  rConfigFile.open(IO_Truncate | IO_WriteOnly);
  // Set uid/gid (neccesary for SUID programs)
  chown(rConfigFile.name().ascii(), getuid(), getgid());

  pStream = new QTextStream( &rConfigFile );

  // write back -- start with the default group
  KEntryKey groupKey = { "<default>", QString() };
  KEntryMapIterator aWriteIt = aTempMap.find(groupKey);

  for (; aWriteIt.key().group == "<default>" && aWriteIt != aTempMap.end();
       ++aWriteIt) {
    if (aWriteIt.key().key.isNull())
      // if group had no entries we may well now be pointing at another group,
      // we need to skip over the special group entry
      continue;
    if ( (*aWriteIt).bNLS &&
	aWriteIt.key().key.right(1) != "]")
      // not yet localized, but should be
      *pStream << aWriteIt.key().key << '['
	       << pConfig->locale() << ']' << "="
	       << stringToPrintable( (*aWriteIt).aValue) << '\n';
    else
      // need not be localized or already is
      *pStream << aWriteIt.key().key << "="
	       << stringToPrintable( (*aWriteIt).aValue) << '\n';

  } // for loop

  // now write out all other groups.
  for (aWriteIt = aTempMap.begin(); aWriteIt != aTempMap.end(); ++aWriteIt) {
    // check if it's not the default group (which has already been written)
    if (aWriteIt.key().group == "<default>")
      continue;

    if (aWriteIt.key().key.isNull()) {
      // we found a special group key, write it as such
      *pStream << '[' << aWriteIt.key().group << ']' << '\n';
    } else {
      // it is data for a group
      if ( (*aWriteIt).bNLS &&
	  aWriteIt.key().key.right(1) != "]")
	// not yet localized, but should be
	*pStream << aWriteIt.key().key << '['
		 << pConfig->locale() << ']' << "="
		 << stringToPrintable( (*aWriteIt).aValue) << '\n';
      else
	// need not be localized or already is
	*pStream << aWriteIt.key().key << "="
		 << stringToPrintable( (*aWriteIt).aValue) << '\n';
    }
  } // for loop

  // clean up
  delete pStream;
  rConfigFile.close();


  // Reopen the config file.

  // Can we allow the write? (see above)
  if( checkAccess( rConfigFile.name(), W_OK ) )
    // Yes, write OK
    rConfigFile.open( IO_ReadWrite );
  else
    rConfigFile.open( IO_ReadOnly );

  return bEntriesLeft;
}
