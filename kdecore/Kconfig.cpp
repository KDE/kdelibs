// $Id$
//
/* $Log$
 * Revision 1.19  1997/08/05 14:24:37  ettrich
 * Matthias: Final solution to the sync() problem. Sync() doesn't reparse,
 *   but a new method reparseConfiguration() will fullfill this task.
 *
 * Revision 1.18  1997/08/04 18:30:14  ettrich
 * Matthias: *sigh*
 *
 * Revision 1.17  1997/08/04 18:18:24  ettrich
 * Matthias: modified my tiny modifikation to do a deep rollback. Makes more sense.
 *
 * Revision 1.16  1997/08/04 17:51:29  ettrich
 * Matthias: again: sync() => no longer dirty entries
 *
 * Revision 1.15  1997/08/04 17:14:04  ettrich
 * Matthis: clear dirty flag after sync()
 *
 * Revision 1.14  1997/08/04 15:16:51  ettrich
 * Matthias: trivial fix for sync(): Now this works as _real_ sync, that
 *  means it also reparses the configuration files.
 *
 *  This is needed for applications with separate configuration tools like
 *  kwm.
 *
 *  A new method "reparse()" may be an alternative, but why make it
 *  even more complicate?
 *
 *  Hope it is ok like that (Kalle?)
 *
 * Revision 1.13  1997/07/31 19:53:14  kalle
 * Bug with defaults in readEntry() (reported and fixed by Stephan Kulow)
 *
 * Revision 1.12  1997/07/25 19:46:39  kalle
 * SGI changes
 *
 * Revision 1.11  1997/07/24 21:04:37  kalle
 * Kalle: Patches for SGI
 *
 * Revision 1.10  1997/07/17 18:43:17  kalle
 * Kalle: new stopsign.xpm
 * 			KConfig: environment variables are resolved in readEntry()
 * 			(meaning you can write() an entry with an environment
 * 			variable and read it back in, and the value gets properly
 * 			expanded).
 * 			Fixed three bugs in ksock.cpp
 * 			Added KFloater
 * 			Added KCombo
 * 			Added KLineEdit
 * 			New KToolbar
 * 			New KToplevelWidget
 *
 * Revision 1.9  1997/06/29 18:26:32  kalle
 * 29.06.97:	KConfig reads and writes string lists
 * 			Torben's patches to ktoolbar.*, kurl.h
 *
 * 22.06.97:	KApplications save and restore position and size of their top
 * (unstable)	level widget.
 *
 * Revision 1.8  1997/05/13 05:48:58  kalle
 * Kalle: Default arguments for KConfig::read*Entry()
 * app-specific config files don't start with a dot
 * Bufgix for the bugfix in htmlobj.cpp (FontManager)
 *
 * Revision 1.7  1997/05/09 15:10:09  kulow
 * Coolo: patched ltconfig for FreeBSD
 * removed some stupid warnings
 *
 * Revision 1.6  1997/04/23 16:45:14  kulow
 * fixed a bug in acinclude.m4 (Thanks to Paul)
 * solved some little problems with some gcc versions (no new code)
 *
 * Revision 1.5  1997/04/21 22:37:22  kalle
 * Bug in Kconfig gefixed (schrieb sein app-spezifisches File nicht mehr)
 * kcolordlg und kapp abgedated (von Martin Jones)
 *
 * Revision 1.4  1997/04/20 21:27:28  kalle
 * Bug fix: writeConfigFile legt neues Datei an, wenn möglich und notwendig
 *
 * Revision 1.3  1997/04/17 12:08:05  kalle
 * Bugfix in KConfig
 *
 * Revision 1.2  1997/04/15 20:01:53  kalle
 * Kalles changes for 0.8
 *
 * Revision 1.1.1.1  1997/04/13 14:42:41  cvsuser
 * Source imported
 *
 * Revision 1.1.1.1  1997/04/09 00:28:05  cvsuser
 * Sources imported
 *
 * Revision 1.14  1997/03/16 22:24:51  kalle
 * Patches from Nicolas Hadacek
 *
 * Revision 1.13  1997/03/09 17:45:10  kalle
 * Use a fresh copy for the value in writeEntry()
 *
 * Revision 1.12  1997/03/09 17:28:52  kalle
 * KTextStream -> QTextStream
 *
 * Revision 1.11  1996/12/14 12:58:26  kalle
 * read and write numerical config values
 *
 * Revision 1.10  1996/12/14 12:49:21  kalle
 * method names start with a small letter
 *
 * Revision 1.9  1996/12/01 10:30:41  kalle
 * bPersistent flag for WriteEntry
 *
 * Revision 1.8  1996/12/01 10:29:31  kalle
 * Rollback implemented
 *
 * Revision 1.7  1996/11/30 19:51:00  kalle
 * Only open default files (app-specific file must be opened by caller)
 *
 * Revision 1.6  1996/11/24 11:52:51  kalle
 * readLine instead of ReadLine
 *
 * Revision 1.5  1996/11/23 21:17:11  kalle
 * KTextStream instead of QTextStream
 *
 * Revision 1.4  1996/11/20 20:15:20  kalle
 * Write access added:
 *
 * KConfig::Sync(), KConfig::WriteConfigFile() and KConfig::WriteEntry() new
 * Destructor calls Sync() if KConfig object is dirty
 *
 * Revision 1.3  1996/11/17 09:51:05  kalle
 * adapted to new entry structore KEntryDictEntry instead of simple strings
 *
 * Revision 1.2  1996/11/15 18:12:57  kalle
 * SetGroup returns void
 *
 * Revision 1.1  1996/11/10 18:30:59  kalle
 * Initial revision
 *
 * Revision 1.9  1996/11/04 14:18:59  kalle
 * Evaluating environment variables
 *
 * Revision 1.8  1996/11/03 21:01:59  kalle
 * Change to cached config entries (in QDict's)
 *
 * Revision 1.7  1996/10/26 19:10:04  kalle
 * GetFileForKey implemented
 *
 * Revision 1.6  1996/10/26 18:56:09  kalle
 * HasKey() implemented
 *
 * Revision 1.5  1996/10/26 18:32:59  kalle
 * Include-Reihenfolge umgestellt
 *
 * Revision 1.4  1996/10/26 18:19:18  kalle
 * Change a leading tilde to user's home directory in 
 * config file name.  
 *
 * Revision 1.3  1996/10/26 18:08:06  kalle
 * Bugfixes, works for all the simple cases
 *
 * Revision 1.2  1996/10/25 18:26:35  kalle
 * first implementation of FindEntry, ReadEntry
 *
 * Revision 1.1  1996/10/25 17:15:17  kalle
 * Initial revision
 */

/* KConfig: configuration options for the Kool Desktop Environment
 *
 * KConfig.cpp
 *
 * (C) 1996 by Matthias Kalle Dalheimer
 */

#ifndef _KCONFIG_H
#include <Kconfig.h>
#endif
#ifndef _KCONFIGDATA_H
#include <Kconfigdata.h>
#endif

#include <qfile.h>
#include <qfileinf.h>
#include <qregexp.h>
#include <qstring.h>

#include <ctype.h>
#include <stdlib.h>

static char* aConfigFileName[] = 
{ 
  "/etc/kderc",
  "/usr/lib/KDE/system.kderc",
  "/usr/local/lib/KDE/system.kderc",
  "~/.kderc",
};

const int CONFIGFILECOUNT = 4; // number of entries in aConfigFileName[]


KConfig::KConfig( QTextStream* pStream )
{
  pData = new KConfigData( pStream );

  // setup a group entry for the default group
  KEntryDict* pDefGroup = new KEntryDict( 37, false );
  pDefGroup->setAutoDelete( true );
  pData->aGroupDict.insert( "<default>", pDefGroup );

  parseConfigFiles();
}

KConfig::~KConfig()
{
  // write back the dirty entries
  sync();
  
  delete pData;
}

void KConfig::parseConfigFiles()
{
  // Parse all desired files from the least to the most specific. This
  // gives the intended behaviour because the QDict returns the last
  // appropriate entry.

  // Parse the general config files
  for( int i = 0; i < CONFIGFILECOUNT; i++ )
    {
      QString aFileName = aConfigFileName[i];
     // replace a leading tilde with the home directory
      // is there a more portable way to find out the home directory?
      char* pHome = getenv( "HOME" );
      if( (aFileName[0] == '~') && pHome )
		aFileName.replace( 0, 1, pHome );

      QFile aConfigFile( aFileName );
      QFileInfo aInfo( aConfigFile );
      // only work with existing files currently
      if( !aInfo.exists() )
		continue;
      aConfigFile.open( IO_ReadOnly );
      QTextStream aStream( &aConfigFile );
      parseOneConfigFile( &aStream ); 
	  aConfigFile.close();
    }
  
  // Parse app-specific config file if available
  if( pData->pAppStream)
    parseOneConfigFile( pData->pAppStream );
}

void KConfig::parseOneConfigFile( QTextStream* pStream, 
				  QDict<KEntryDict>* pWriteBackDict )
{
  QString aCurrentLine;
  QString aCurrentGroup = "";

  QDict<KEntryDict> *pDict;
  if( pWriteBackDict )
	// write back mode - don't mess up the normal dictionary
	pDict = pWriteBackDict;
  else
	// normal mode - use the normal dictionary
	pDict = &pData->aGroupDict;

  KEntryDict* pCurrentGroupDict = (*pDict)[ "<default>" ];
  
  // reset the stream's device
  pStream->device()->at(0);
  while( pStream->device()->isOpen() && !pStream->eof() )
    {
      aCurrentLine = pStream->readLine();

      // check for a group
      int nLeftBracket = aCurrentLine.find( '[' );
      int nRightBracket = aCurrentLine.find( ']', 1 );
      if( nLeftBracket == 0 && nRightBracket != -1 )
	{
	  // group found; get the group name by taking everything in  
	  // between the brackets  
	  aCurrentGroup = 
	    aCurrentLine.mid( 1, nRightBracket-1 );

	  // check if there already is such a group in the group
	  // dictionary
	  pCurrentGroupDict = (*pDict)[ aCurrentGroup ];
	  if( !pCurrentGroupDict )
	    {
	      // no such group -> create a new entry dictionary
	      KEntryDict* pNewDict = new KEntryDict( 37, false );
	      pNewDict->setAutoDelete( true );
	      (*pDict).insert( aCurrentGroup, pNewDict );

	      // this is now the current group
	      pCurrentGroupDict = pNewDict;
	    }
	  continue;
	};
      
      int nEqualsPos = aCurrentLine.find( '=' );
      if( nEqualsPos == -1 )
		// no equals sign: incorrect or empty line, skip it
		continue;
      
      // insert the key/value line into the current dictionary
	  KEntryDictEntry* pEntry = new KEntryDictEntry;
	  pEntry->aValue = 
		aCurrentLine.right( aCurrentLine.length()-nEqualsPos-1 );
	  pEntry->bDirty = false;


      pCurrentGroupDict->insert( aCurrentLine.left( nEqualsPos ),
								 pEntry );
    }
}

void KConfig::setGroup( const QString& rGroup )
{
  if( rGroup.isEmpty() )
    pData->aGroup = "<default>";
  else
    pData->aGroup = rGroup;
}

const QString& KConfig::getGroup() const
{
  static QString aEmptyStr = "";
  if( pData->aGroup == "<default>" )
    return aEmptyStr;
  else
    return pData->aGroup;
}

QString KConfig::readEntry( const QString& rKey, 
							const QString* pDefault ) const 
{
  QString aValue;
  // retrieve the current group dictionary
  KEntryDict* pCurrentGroupDict = pData->aGroupDict[ pData->aGroup.data() ];
  
  if( pCurrentGroupDict )
    {
      // find the value for the key in the current group
      KEntryDictEntry* pData = (*pCurrentGroupDict)[ rKey.data() ];
      if( pData )
		aValue = pData->aValue;
	  else if( pDefault )
		aValue = *pDefault;
    }
  else if( pDefault )
	aValue = *pDefault;

  // check for environment variables and make necessary translations
  int nDollarPos = aValue.find( '$' );
  while( nDollarPos != -1 )
	{
	  // there is at least one $
	  if( (aValue)[nDollarPos+1] != '$' )
	    {
	      uint nEndPos = nDollarPos;
	      // the next character is no $
	      do
			{
			  nEndPos++;
			} while ( isalnum( (aValue)[nEndPos] ) || 
					  nEndPos > aValue.length() );
	      QString aVarName = aValue.mid( nDollarPos+1, 
										 nEndPos-nDollarPos-1 );
	      char* pEnv = getenv( aVarName );
	      if( pEnv )
			aValue.replace( nDollarPos, nEndPos-nDollarPos, pEnv );
	      else
			aValue.remove( nDollarPos, nEndPos-nDollarPos );
	    }
	  else
	    // remove one of the dollar signs
	    aValue.remove( nDollarPos, nDollarPos+1 );
	  nDollarPos = aValue.find( '$', nDollarPos+2 );
	};

  return aValue;
}

int KConfig::readListEntry ( const QString &rKey, QStrList &list, 
							 char sep  ) const
{
  if( !hasKey( rKey ) )
    return 0;
  QString str_list, value;
  str_list = readEntry(rKey);
  if(str_list.isEmpty())
    return 0; 
  list.clear();
  int i;
  int len = str_list.length();
  for( i = 0; i < len; i++ )
    {
      if( str_list[i] != sep && str_list[i] != '\\' )
	{
	  value += str_list[i];
	  continue;
	}
      if( str_list[i] == '\\' )
	{
	  i++;
	  value += str_list[i];
	  continue;
	}
      list.append(value);
      value.truncate(0);
    }
  list.append(value);
  return list.count();
}

int KConfig::readNumEntry( const QString& rKey, int nDefault ) const
{
  bool ok;
  int rc;

  QString aValue = readEntry( rKey );
  if( aValue.isNull() )
	return nDefault;
  else
	{
	  rc = aValue.toInt( &ok );
	  return( ok ? rc : 0 );
	}
}


QFont KConfig::readFontEntry( const QString& rKey, 
							  const QFont* pDefault ) const
{
  QFont aRetFont;

  QString aValue = readEntry( rKey );
  if( !aValue.isNull() )
	{
	  // find first part (font family)
	  int nIndex = aValue.find( ',' );
	  if( nIndex == -1 )
		return aRetFont;
	  aRetFont.setFamily( aValue.left( nIndex ) );
	  
	  // find second part (point size)
	  int nOldIndex = nIndex;
	  nIndex = aValue.find( ',', nOldIndex+1 );
	  if( nIndex == -1 )
		return aRetFont;
	  aRetFont.setPointSize( aValue.mid( nOldIndex+1, 
										 nIndex-nOldIndex-1 ).toInt() );

	  // find third part (style hint)
	  nOldIndex = nIndex;
	  nIndex = aValue.find( ',', nOldIndex+1 );
	  if( nIndex == -1 )
		return aRetFont;
	  aRetFont.setStyleHint( (QFont::StyleHint)aValue.mid( nOldIndex+1, 
													nIndex-nOldIndex-1 ).toUInt() );

	  // find fourth part (char set)
	  nOldIndex = nIndex;
	  nIndex = aValue.find( ',', nOldIndex+1 );
	  if( nIndex == -1 )
		return aRetFont;
	  aRetFont.setCharSet( (QFont::CharSet)aValue.mid( nOldIndex+1, 
									   nIndex-nOldIndex-1 ).toUInt() );

	  // find fifth part (weight)
	  nOldIndex = nIndex;
	  nIndex = aValue.find( ',', nOldIndex+1 );
	  if( nIndex == -1 )
		return aRetFont;
	  aRetFont.setWeight( aValue.mid( nOldIndex+1,
									  nIndex-nOldIndex-1 ).toUInt() );

	  // find sixth part (font bits)
	  uint nFontBits = aValue.right( aValue.length()-nIndex-1 ).toUInt();
	  if( nFontBits & 0x01 )
		aRetFont.setItalic( true );
	  if( nFontBits & 0x02 )
		aRetFont.setUnderline( true );
	  if( nFontBits & 0x04 )
		aRetFont.setStrikeOut( true );
	  if( nFontBits & 0x08 )
		aRetFont.setFixedPitch( true );
	  if( nFontBits & 0x20 )
		aRetFont.setRawMode( true );
	}
  else if( pDefault )
	aRetFont = *pDefault;

  return aRetFont;
}


QColor KConfig::readColorEntry( const QString& rKey,
								const QColor* pDefault ) const
{
  QColor aRetColor;
  int nRed = 0, nGreen = 0, nBlue = 0;

  QString aValue = readEntry( rKey );
  if( !aValue.isNull() )
	{
bool bOK;
	  // find first part (red)
	  int nIndex = aValue.find( ',' );
	  if( nIndex == -1 )
		return aRetColor;
	  nRed = aValue.left( nIndex ).toInt( &bOK );
	  
	  // find second part (green)
	  int nOldIndex = nIndex;
	  nIndex = aValue.find( ',', nOldIndex+1 );
	  if( nIndex == -1 )
		return aRetColor;
	  nGreen = aValue.mid( nOldIndex+1,
						   nIndex-nOldIndex-1 ).toInt( &bOK );

	  // find third part (blue)
	  nBlue = aValue.right( aValue.length()-nIndex-1 ).toInt( &bOK );

	  aRetColor.setRgb( nRed, nGreen, nBlue );
	}
  else if( pDefault )
	aRetColor = *pDefault;

  return aRetColor;
}


QString KConfig::writeEntry( const QString& rKey, const QString& rValue,
							 bool bPersistent )
{
  QString aValue;

  // retrieve the current group dictionary
  KEntryDict* pCurrentGroupDict = pData->aGroupDict[ pData->aGroup.data() ];

  if( !pCurrentGroupDict )
	{
	  // no such group -> create a new entry dictionary
	  KEntryDict* pNewDict = new KEntryDict( 37, false );
	  pNewDict->setAutoDelete( true );
	  pData->aGroupDict.insert( pData->aGroup.data(), pNewDict );
	  
	  // this is now the current group
	  pCurrentGroupDict = pNewDict;
	}

  // try to retrieve the current entry for this key
  KEntryDictEntry* pEntryData = (*pCurrentGroupDict)[ rKey.data() ];
  if( pEntryData )
	{
	  // there already is such a key
	  aValue = pEntryData->aValue; // save old key as return value
	  pEntryData->aValue = rValue.copy() ; // set new value
	  if( bPersistent )
		pEntryData->bDirty = TRUE;
	}
  else
	{
	  // the key currently does not exist
	  KEntryDictEntry* pEntry = new KEntryDictEntry;
	  pEntry->aValue = rValue.copy();
	  if( bPersistent )
		pEntry->bDirty = TRUE;

	  // insert the new entry into group dictionary
	  pCurrentGroupDict->insert( rKey.data(), pEntry );
	}

  // the KConfig object is dirty now
  if( bPersistent )
	pData->bDirty = true;
  return aValue;
}

void KConfig::writeEntry( const QString& rKey, QStrList& list, 
						  char sep, bool bPersistent )
{
  if( list.isEmpty() )
    {
      writeEntry(rKey, "", bPersistent );      
      return;
    }
  QString str_list, value;
  int i;
  for( value = list.first(); value != (char*)NULL; value = list.next() )
    {
      for( i = 0; i < (int) value.length(); i++ )
	{
	  if( value[i] == sep || value[i] == '\\' )
	    str_list += '\\';
	  str_list += value[i];
	}
      str_list += sep;
    }
  if( str_list.right(1) == sep )
    str_list.truncate(str_list.length()-1);
  writeEntry(rKey, str_list, bPersistent );
}


QString KConfig::writeEntry( const QString& rKey, int nValue,
							 bool bPersistent )
{
  QString aValue;

  aValue.setNum( nValue );

  return writeEntry( rKey, aValue, bPersistent );
}

QString KConfig::writeEntry( const QString& rKey, const QFont& rFont,
							 bool bPersistent )
{
  QString aValue;
  UINT8 nFontBits = 0;
  // this mimics get_font_bits() from qfont.cpp
  if( rFont.italic() )
	nFontBits = nFontBits | 0x01;
  if( rFont.underline() )
	nFontBits = nFontBits | 0x02;
  if( rFont.strikeOut() )
	nFontBits = nFontBits | 0x04;
  if( rFont.fixedPitch() )
	nFontBits = nFontBits | 0x08;
  if( rFont.rawMode() )
	nFontBits = nFontBits | 0x20;

  aValue.sprintf( "%s,%d,%d,%d,%d,%d", rFont.family(), rFont.pointSize(),
				  rFont.styleHint(), rFont.charSet(), rFont.weight(),
				  nFontBits );

  return writeEntry( rKey, aValue, bPersistent );
}

QString KConfig::writeEntry( const QString& rKey, const QColor& rColor,
							 bool bPersistent )
{
  QString aValue;
  aValue.sprintf( "%d,%d,%d", rColor.red(), rColor.green(), rColor.blue() );

  return writeEntry( rKey, aValue, bPersistent );
}


void KConfig::rollback( bool bDeep )
{
  // clear the global bDirty flag in all cases
  pData->bDirty = false;

  // if bDeep is true, clear the bDirty flags of all the entries
  if( !bDeep )
	return;
  QDictIterator<KEntryDict> aIt( pData->aGroupDict );
  // loop over all the groups
  const char* pCurrentGroup;
  while( (pCurrentGroup = aIt.currentKey()) )
	{
	  QDictIterator<KEntryDictEntry> aInnerIt( *aIt.current() );
	  // loop over all the entries
	  KEntryDictEntry* pCurrentEntry;
	  while( (pCurrentEntry = aInnerIt.current()) )
		{
		  pCurrentEntry->bDirty = false;
		  ++aInnerIt;
		}
	  ++aIt;
	}
}



void KConfig::sync()
{
  // write-sync is only necessary if there are dirty entries
  if( pData->bDirty ) {
    // find out the file to write to (most specific writable file)

    // try app-specific file first
    if( pData->pAppStream ){
	// is it writable?
      if (pData->pAppStream->device()->isWritable() )
	{
	  writeConfigFile( *(QFile *)pData->pAppStream->device() );
	  pData->pAppStream->device()->close();
	  pData->pAppStream->device()->open( IO_ReadWrite );
	}
    }
    else {
      // try other files
      for( int i = CONFIGFILECOUNT-1; i >= 0; i-- )
	{
	  QString aFileName = aConfigFileName[i];
	  // replace a leading tilde with the home directory
	  // is there a more portable way to find out the home directory?
	  char* pHome = getenv( "HOME" );
	  if( (aFileName[0] == '~') && pHome )
	    aFileName.replace( 0, 1, pHome );
	  
	  QFile aConfigFile( aFileName );
	  QFileInfo aInfo( aConfigFile );
	  if( ( aInfo.exists() && aInfo.isWritable() ) ||
	      ( !aInfo.exists() && 
		QFileInfo( aInfo.dirPath( true ) ).isWritable() ) )
	    {
	      aConfigFile.open( IO_ReadWrite );
	      writeConfigFile( aConfigFile );
	      break;
	    }
	}
    }
  }

  // no more dirty entries
  rollback();

}


void KConfig::writeConfigFile( QFile& rConfigFile )
{
  QTextStream* pStream = new QTextStream( &rConfigFile );

  // create a temporary dictionary that represents the file to be written
  QDict<KEntryDict> aTempDict( 37, FALSE );
  aTempDict.setAutoDelete( true );
  
  // setup a group entry for the default group
  KEntryDict* pDefGroup = new KEntryDict( 37, false );
  pDefGroup->setAutoDelete( true );
  aTempDict.insert( "<default>", pDefGroup );
  
  // fill the temporary structure with entries from the file
  parseOneConfigFile( pStream, &aTempDict );

  // augment this structure with the dirty entries from the normal structure
  QDictIterator<KEntryDict> aIt( pData->aGroupDict );

  // loop over all the groups
  const char* pCurrentGroup;
  while( (pCurrentGroup = aIt.currentKey()) )
	{
	  QDictIterator<KEntryDictEntry> aInnerIt( *aIt.current() );
	  // loop over all the entries
	  KEntryDictEntry* pCurrentEntry;
	  while( (pCurrentEntry = aInnerIt.current()) )
		{
		  if( pCurrentEntry->bDirty )
			{
			  // enter the *aInnerIt.currentKey()/pCurrentEntry->aValue pair
			  // into group *pCurrentGroup in aTempDict
			  KEntryDict* pTempGroup;
			  if( !( pTempGroup = aTempDict[ pCurrentGroup ] ) )
				{
				  // group does not exist in aTempDict
				  pTempGroup = new KEntryDict( 37, false );
				  pTempGroup->setAutoDelete( true );
				  aTempDict.insert( pCurrentGroup, pTempGroup );
				}
			  KEntryDictEntry* pNewEntry = new KEntryDictEntry();
			  pNewEntry->aValue = pCurrentEntry->aValue;
			  pNewEntry->bDirty = FALSE;
			  pTempGroup->replace( aInnerIt.currentKey(), 
								   pNewEntry );
			}
		  ++aInnerIt;
		}
	  ++aIt;
	}

  // truncate file
  delete pStream;
  rConfigFile.close();
  rConfigFile.open( IO_Truncate | IO_WriteOnly );
  pStream = new QTextStream( &rConfigFile );

  // write back -- start with the default group
  KEntryDict* pDefWriteGroup = aTempDict[ "<default>" ];
  if( pDefWriteGroup )
	{
	  QDictIterator<KEntryDictEntry> aWriteInnerIt( *pDefWriteGroup );
	  while( aWriteInnerIt.current() )
		{
		  *pStream << aWriteInnerIt.currentKey() << "=" 
			   << aWriteInnerIt.current()->aValue << '\n';
		  ++aWriteInnerIt;
		}
	}

  QDictIterator<KEntryDict> aWriteIt( aTempDict );
  while( aWriteIt.current() )
	{
	  // check if it's not the default group (which has already been written)
	  if( strcmp (aWriteIt.currentKey(), "<default>" ) )
		{
		  *pStream << '[' << aWriteIt.currentKey() << ']' << '\n';
		  QDictIterator<KEntryDictEntry> aWriteInnerIt( *aWriteIt.current() );
		  while( aWriteInnerIt.current() )
			{
			  *pStream << aWriteInnerIt.currentKey() 
				   << "="
				   << aWriteInnerIt.current()->aValue << '\n';
			  ++aWriteInnerIt;
			}
		}
	  ++aWriteIt;
	}

  // clean up
  delete pStream;
  rConfigFile.close();
  rConfigFile.open( IO_ReadWrite );
}

bool KConfig::hasKey( const QString& rKey ) const
{
  QString aValue = readEntry( rKey );
  return !aValue.isNull();
}



void KConfig::reparseConfiguration()
{
  pData->aGroupDict.clear();

  // setup a group entry for the default group
  KEntryDict* pDefGroup = new KEntryDict( 37, false );
  pDefGroup->setAutoDelete( true );
  pData->aGroupDict.insert( "<default>", pDefGroup );

  // the next three lines are indeed important.
  pData->pAppStream->device()->close();
  if (!pData->pAppStream->device()->open( IO_ReadWrite ))
    pData->pAppStream->device()->open( IO_ReadOnly );

  parseConfigFiles();
}
