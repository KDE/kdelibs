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
#include <stdlib.h>

#include <qfileinfo.h>
#include <qtextstream.h>

#include <kapp.h>

#include "kconfigbackend.h"
#include "kconfigbase.h"

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_TEST
#include <test.h>
#endif

static const char* globalConfigFileNames[] =
{
  // !!! If you add/remove pathnames here, update CONFIGFILECOUNT a few lines
  // below!!!
  "/etc/kderc",
  KDEDIR"/share/config/kderc",
  "/usr/lib/KDE/system.kderc",
  "/usr/local/lib/KDE/system.kderc",
  "~/.kderc",
};
 
const int CONFIGFILECOUNT = 5; // number of entries in globalConfigFileNames[]


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

KConfigBackEnd::KConfigBackEnd(KConfigBase *_config, const QString &_globalFileName,
			       const QString &_localFileName, bool _useKderc)
  : pConfig(_config), aGlobalFileName(_globalFileName), 
    aLocalFileName(_localFileName), useKderc(_useKderc)
{}

bool KConfigINIBackEnd::parseConfigFiles()
{
  // Parse all desired files from the least to the most specific.
 
  // Parse the general config files
  if (useKderc) {
    // only parse the system kderc files if we have been directed to.
    for (int i = 0; i < CONFIGFILECOUNT; i++) {
      QString aFileName = globalConfigFileNames[i];
      // replace a leading tilde with the home directory
      // is there a more portable way to find out the home directory?
      char* pHome = getenv( "HOME" );
      if( (aFileName[0] == '~') && pHome )
	aFileName.replace( 0, 1, pHome );
      
      QFile aConfigFile( aFileName );
      QFileInfo aInfo( aConfigFile );
      // only work with existing files currently
      if (!aInfo.exists())
	continue;
      aConfigFile.open( IO_ReadOnly );
      parseSingleConfigFile( aConfigFile, 0L, true );
      aConfigFile.close();
    }
  }
 
  // Parse app-specific config files if available
  if (!aGlobalFileName.isEmpty()) {
    QFile aConfigFile( aGlobalFileName );
    // we can already be sure that this file exists
    aConfigFile.open( IO_ReadOnly );
    parseSingleConfigFile( aConfigFile, 0L, false );
    aConfigFile.close();
  }

  if (!aLocalFileName.isEmpty()) {
    QFile aConfigFile(aLocalFileName );
    // we can't be sure that this file exists
    if (!aConfigFile.open( IO_ReadOnly)) {
      aLocalFileName =
	QString("%1/share/config/%2").arg(KApplication::localkdedir()).arg(aLocalFileName);
      aConfigFile.close();
      aConfigFile.setName(aLocalFileName);
      aConfigFile.open(IO_ReadOnly);
    }
    
    parseSingleConfigFile( aConfigFile, 0L, false );
    aConfigFile.close();
  }
  return true;
}

void KConfigINIBackEnd::parseSingleConfigFile(QFile &rFile, 
					   KEntryMap *pWriteBackMap,
					   bool bGlobal)
{
  if (!rFile.isOpen()) // come back, if you have real work for us ;->
    return;
 
  QString aCurrentLine;
  QString aCurrentGroup;
  
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
	KEntryKey groupKey = { aCurrentGroup, QString::null };
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
    KEntry aEntry = { val, true, false, bGlobal };
    
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
  if (!aLocalFileName.isEmpty()) {
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
      if (aConfigFile.isWritable()) {
	bEntriesLeft = writeConfigFile( aConfigFile, false, bMerge );
	bLocalGood = true;
      }
      aConfigFile.close();
    }
  }
  
  // If we could not write to the local app-specific config file,
  // we can try the global app-specific one. This will only work
  // as root, but is worth a try.
  if (!bLocalGood && !aGlobalFileName.isEmpty()) {
    // Can we allow the write? (see above)
    if (checkAccess( aGlobalFileName, W_OK )) {
      // is it writable?
      QFile aConfigFile( aGlobalFileName );
      aConfigFile.open( IO_ReadWrite );
      if (aConfigFile.isWritable()) {
	bEntriesLeft = writeConfigFile( aConfigFile, false, bMerge );
	bLocalGood = true;
      }
      aConfigFile.close();
    }
  }
  
  // only write out entries to the kderc file if there are any
  // entries marked global (indicated by bEntriesLeft) and
  // the useKderc flag is set.
  if (bEntriesLeft && useKderc) {
    // If there are entries left, either something went wrong with
    // the app-specific files or there were global entries to write.
    // try other files
    for (int i = CONFIGFILECOUNT-1; i >= 0; i--) {
      QString aFileName = globalConfigFileNames[i];
      // replace a leading tilde with the home directory
      // is there a more portable way to find out the home directory?
      char* pHome = getenv( "HOME" );
      if ((aFileName[0] == '~') && pHome)
	aFileName.replace(0, 1, pHome);
      
      QFile aConfigFile( aFileName );
      QFileInfo aInfo( aConfigFile );
      // can we allow the write? (see above)
      if (checkAccess ( aFileName, W_OK )) {
	if (( aInfo.exists() && aInfo.isWritable()) ||
	    (!aInfo.exists() &&
	     QFileInfo(aInfo.dirPath(true)).isWritable())) {
	  aConfigFile.open( IO_ReadWrite );
	  // Set uid/gid (neccesary for SUID programs)
	  chown(aConfigFile.name().ascii(), getuid(), getgid());
	  writeConfigFile( aConfigFile, true, bMerge );
	  break;
	}
      }
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
	    KEntryKey groupKey = { entryKey.group, QString::null };
	    
	    // add special group key
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
    rConfigFile.close();
  } else {
    // don't merge, just get the regular entry map and use that.
    aTempMap = pConfig->internalEntryMap();
    bEntriesLeft = true; // maybe not true, but we aren't sure
  }

  // OK now the temporary map should be full of ALL entries.
  // write it out to disk.

  // Does program run SUID and user would not be allowed to write
  // to the file, if it doesn't run  SUID?
  if (!checkAccess(rConfigFile.name(), W_OK)) // write not allowed
    return false; // can't allow to write config data.
 
 
  rConfigFile.open(IO_Truncate | IO_WriteOnly);
  // Set uid/gid (neccesary for SUID programs)
  chown(rConfigFile.name().ascii(), getuid(), getgid());
 
  pStream = new QTextStream( &rConfigFile );
 
  // write back -- start with the default group
  KEntryKey groupKey = { "<default>", QString::null };
  KEntryMapIterator aWriteIt = aTempMap.find(groupKey);

  for (; aWriteIt.key().group == "<default>" && aWriteIt != aTempMap.end();
       ++aWriteIt) {
    if (aWriteIt.key().key.isNull())
      // if group had no entries we may well now be pointing at another group,
      // we need to skip over the special group entry
      continue;
    if (aWriteIt->bNLS &&
	aWriteIt.key().key.right(1) != "]")
      // not yet localized, but should be
      *pStream << aWriteIt.key().key << '['
	       << pConfig->locale() << ']' << "="
	       << stringToPrintable(aWriteIt->aValue) << '\n';
    else
      // need not be localized or already is
      *pStream << aWriteIt.key().key << "="
	       << stringToPrintable(aWriteIt->aValue) << '\n';
    
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
      if (aWriteIt->bNLS &&
	  aWriteIt.key().key.right(1) != "]")
	// not yet localized, but should be
	*pStream << aWriteIt.key().key << '['
		 << pConfig->locale() << ']' << "="
		 << stringToPrintable(aWriteIt->aValue) << '\n';
      else
	// need not be localized or already is
	*pStream << aWriteIt.key().key << "="
		 << stringToPrintable(aWriteIt->aValue) << '\n';
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
