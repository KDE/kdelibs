/* This file is part of the KDE libraries
    Copyright (C) 1997 Matthias Kalle Dalheimer (kalle@kde.org)

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
// $Id$
//
// $Log$
// Revision 1.9  1997/12/27 22:57:26  kulow
// I was a little bit nerved by the QFile warnings caused by the KApplication
// constructor, so I investigated a little bit ;) Fixed now
// Added KCharset class - small changes in interface. I hope it is all source
// Revision 1.8  1997/12/18 20:51:27  kalle
// Some patches by Alex and me
// Revision 1.1.1.3  1997/12/11 07:19:11  jacek
// Imported sources from KDE CVS
//
// Revision 1.1.1.2  1997/12/10 07:08:29  jacek
// Imported sources from KDE CVS
//
// Revision 1.1.1.1  1997/12/09 22:02:45  jacek
// Imported sorces fromkde
//
// Revision 1.7  1997/11/20 08:44:54  kalle
// Whoever says A should also say B...
// (reading ~/.kderc works again)
//
// Revision 1.6  1997/11/18 21:40:51  kalle
// KApplication::localconfigdir()
// KApplication::localkdedir()
// KConfig searches in $KDEDIR/share/config/kderc
//
// Revision 1.5  1997/10/21 20:44:43  kulow
// removed all NULLs and replaced it with 0L or "".
// There are some left in mediatool, but this is not C++
//
// Revision 1.4  1997/10/16 11:35:25  kulow
// readded my yesterday bugfixes. I hope, I have not forgotten one.
// I'm not sure, why this have been removed, but I'm sure, they are
// needed.
//
// Revision 1.3  1997/10/16 11:14:29  torben
// Kalle: Copyright headers
// kdoctoolbar removed
//
// Revision 1.1  1997/10/04 19:50:58  kalle
// new KConfig
//

#include <kconfig.h>
#include "kconfig.moc"
#include <qfileinf.h>
#include <stdlib.h>

static char* aConfigFileName[] = 
	data()->aGlobalAppFile = pGlobalAppFile;

	data()->aLocalAppFile = pLocalAppFile;
		  file.close();
		}
	}
  if( pLocalAppFile )
	{
	  data()->aLocalAppFile = pLocalAppFile;
	  // the file should exist in any case
	  QFileInfo info( pLocalAppFile );
	  if( !info.exists() )
		{
		  QFile file( pLocalAppFile );
		  file.open( IO_WriteOnly );
		  file.close();
		}
	}

  parseConfigFiles();
}


KConfig::~KConfig()
{
  sync();
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
      parseOneConfigFile( aConfigFile, 0L, true ); 
	  aConfigFile.close();
	  parseOneConfigFile( aConfigFile, 0L, false );
	  aConfigFile.close();
	}
	  aConfigFile.open( IO_ReadOnly );
  if( !data()->aLocalAppFile.isEmpty() )
	{
	  QFile aConfigFile( data()->aLocalAppFile );
	  // we can already be sure that this file exists
 	  /* Actually, we can't: CHange by Alex */
 	  if (!aConfigFile.open( IO_ReadOnly)) {
 		data()->aLocalAppFile.sprintf("%s/.kde/share/config/%s",getenv("HOME"),data()->aLocalAppFile.data());
 		aConfigFile.close();
 		aConfigFile.setName(data()->aLocalAppFile.data());
 		aConfigFile.open(IO_ReadOnly);
 	  } 

	  parseOneConfigFile( aConfigFile, 0L, false );
	  aConfigFile.close();
	}
}


bool KConfig::writeConfigFile( QFile& rConfigFile, bool bGlobal )
{
  bool bEntriesLeft = false;

  QTextStream* pStream = new QTextStream( &rConfigFile );

  // create a temporary dictionary that represents the file to be written
  QDict<KEntryDict> aTempDict( 37, FALSE );
  aTempDict.setAutoDelete( true );
  
  // setup a group entry for the default group
  KEntryDict* pDefGroup = new KEntryDict( 37, false );
  pDefGroup->setAutoDelete( true );
  aTempDict.insert( "<default>", pDefGroup );
  
  // fill the temporary structure with entries from the file
  parseOneConfigFile( rConfigFile, &aTempDict, bGlobal );

  // augment this structure with the dirty entries from the normal structure
  QDictIterator<KEntryDict> aIt( data()->aGroupDict );

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
			  // only write back entries that have the same
			  // "globality" as the file
			  if( pCurrentEntry->bGlobal == bGlobal )
				{
				  // enter the
				  // *aInnerIt.currentKey()/pCurrentEntry->aValue pair
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
				  pNewEntry->bDirty = false;
				  pNewEntry->bGlobal = pCurrentEntry->bGlobal;
				  pNewEntry->bNLS = pCurrentEntry->bNLS;
				  pTempGroup->replace( aInnerIt.currentKey(), 
									   pNewEntry );
				}
			  else
				// wrong "globality" - might have to be saved later
				bEntriesLeft = true;
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
  
  // write a magic cookie for Fritz' mime magic
  *pStream << "# KDE Config File\n";

  // write back -- start with the default group
  KEntryDict* pDefWriteGroup = aTempDict[ "<default>" ];
  if( pDefWriteGroup )
	{
	  QDictIterator<KEntryDictEntry> aWriteInnerIt( *pDefWriteGroup );
	  while( aWriteInnerIt.current() )
		{
		  if( aWriteInnerIt.current()->bNLS && 
			  QString( aWriteInnerIt.currentKey() ).right( 1 ) != "]" )
			// not yet localized, but should be
			*pStream << aWriteInnerIt.currentKey() << '[' 
					 << data()->aLocaleString << ']' << "=" 
					 << aWriteInnerIt.current()->aValue << '\n';
		  else
			// need not be localized or already is
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
			  if( aWriteInnerIt.current()->bNLS && 
				  QString( aWriteInnerIt.currentKey() ).right( 1 ) != "]" )
				// not yet localized, but should be
				*pStream << aWriteInnerIt.currentKey() << '[' 
						 << data()->aLocaleString << ']' << "=" 
						 << aWriteInnerIt.current()->aValue << '\n';
			  else
				// need not be localized or already is
				*pStream << aWriteInnerIt.currentKey() << "="
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
  
  return bEntriesLeft;
}


void KConfig::sync()
{
  // write-sync is only necessary if there are dirty entries
  if( data()->bDirty ) 
	{
	  bool bEntriesLeft = false;
	  bool bLocalGood = false;

	  // find out the file to write to (most specific writable file)
	  // try local app-specific file first
	  if( !data()->aLocalAppFile.isEmpty() )
		{
		  // is it writable?
		  QFile aConfigFile( data()->aLocalAppFile );
		  aConfigFile.open( IO_ReadWrite );
		  if ( aConfigFile.isWritable() )
			{
			  bEntriesLeft = writeConfigFile( aConfigFile, false );   
			  bLocalGood = true;
			}
		  aConfigFile.close();
		}

	  // If we could not write to the local app-specific config file,
	  // we can try the global app-specific one. This will only work
	  // as root, but is worth a try.
	  if( !bLocalGood && !data()->aGlobalAppFile.isEmpty() )
		{
		  // is it writable?
		  QFile aConfigFile( data()->aGlobalAppFile );
		  aConfigFile.open( IO_ReadWrite );
		  if ( aConfigFile.isWritable() )
			{
			  bEntriesLeft = writeConfigFile( aConfigFile, false );   
			  bLocalGood = true;
			}
		  aConfigFile.close();
		}
	  

	  if( bEntriesLeft )
		// If there are entries left, either something went wrong with
		// the app-specific files or there were global entries to write.
		{
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
				  writeConfigFile( aConfigFile, true );
				  break;
				}
			}
		}
	}

  // no more dirty entries
  rollback();
}
