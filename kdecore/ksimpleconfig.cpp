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
// Revision 1.8  1998/01/15 13:22:30  kalle
// Read-only mode for KSimpleConfig
// Added KCharset class - small changes in interface. I hope it is all source
// Revision 1.7  1998/01/11 13:41:42  kalle
// Write tag line for MIME detection even in KSimpleConfig
// Added Unicode maping for adobe-symbol fonts, but they are not well displayable yet.
// Revision 1.6  1997/12/18 20:51:34  kalle
// Some patches by Alex and me
// Revision 1.1.1.3  1997/12/11 07:19:14  jacek
// Imported sources from KDE CVS
//
// Revision 1.1.1.2  1997/12/10 07:08:33  jacek
// Imported sources from KDE CVS
//
// Revision 1.1.1.1  1997/12/09 22:02:46  jacek
// Imported sorces fromkde
//
// Revision 1.5  1997/10/21 20:44:52  kulow
// removed all NULLs and replaced it with 0L or "".
// There are some left in mediatool, but this is not C++
//
// Revision 1.4  1997/10/16 11:15:02  torben
// Kalle: Copyright headers
// kdoctoolbar removed
//
// Revision 1.3  1997/10/10 16:14:24  kulow
// removed one more default value from the implementation
//
// Revision 1.2  1997/10/08 19:28:53  kalle
// KSimpleConfig implemented
//
// Revision 1.1  1997/10/04 19:51:07  kalle
// new KConfig
//

#include <ksimpleconfig.h>
#include "ksimpleconfig.moc"

#include <qfileinf.h>

KSimpleConfig::KSimpleConfig( const char* pFile )
{
  if( pFile )
	{
	  // the file should exist in any case
	  QFileInfo info( pFile );
	  if( !info.exists() )
		{
		  QFile file( pFile );
		  file.open( IO_WriteOnly );

	  // we use the global app config file to save the filename 
  sync();

  data()->bReadOnly = bReadOnly;

  parseConfigFiles();
}

  aFile.open( IO_ReadWrite );
	sync();
}


void KSimpleConfig::parseConfigFiles()
{
  QFile aFile( data()->aGlobalAppFile );
  if( data()->bReadOnly )
	aFile.open( IO_ReadOnly );
  else
	aFile.open( IO_ReadWrite );
  parseOneConfigFile( aFile, 0L );
  aFile.close();
}


const QString KSimpleConfig::deleteEntry( const char* pKey, bool bLocalized )
{
  // retrieve the current group dictionary
  KEntryDict* pCurrentGroupDict = data()->aGroupDict[ data()->aGroup.data() ];
  
  if( pCurrentGroupDict )
    {
	  if( bLocalized )
		{	  
		  QString aLocalizedKey = QString( pKey );
		  aLocalizedKey += "[";
		  aLocalizedKey += data()->aLocaleString;
		  aLocalizedKey += "]";
		  // find the value for the key in the current group
		  KEntryDictEntry* pEntryData = (*pCurrentGroupDict)[ aLocalizedKey.data() ];
		  if( pEntryData )
			{
			  QString aValue = pEntryData->aValue;
			  pCurrentGroupDict->remove( pKey );
			  return aValue.copy();
			}
		  else
			return QString();
		}
	  else
		{
		  KEntryDictEntry* pEntryData = (*pCurrentGroupDict)[ pKey ];
		  if( pEntryData )
			{
			  QString aValue = pEntryData->aValue;
			  pCurrentGroupDict->remove( pKey );
			  return aValue.copy();
			}
		  else
			return QString();
		}
	}
  else 
	return QString();
}


bool KSimpleConfig::deleteGroup( const char* pGroup, bool bDeep )
{
  // retrieve the group dictionary
  KEntryDict* pGroupDict = data()->aGroupDict[ pGroup ];
  
  if( pGroupDict )
	{
	  if( pGroupDict->count() && !bDeep )
		// there are items which should not be deleted
		return false;
	  else
		{
		  // simply remove the group dictionary, since the main group
		  // dictionary is set to autoDelete, the items will be deleted,
		  // too
		  data()->aGroupDict.remove( pGroup );
		  return true;
	// no such group
	return false;
}

/** Write back the configuration data.
  */
bool KSimpleConfig::writeConfigFile( QFile& rFile, bool )
{
  if( data()->bReadOnly )
	return true; // fake that the data was written

  rFile.open( IO_Truncate | IO_WriteOnly );
  QTextStream* pStream = new QTextStream( &rFile );

  // write a magic cookie for Fritz' mime magic
  *pStream << "# KDE Config File\n";
  
  // write back -- start with the default group
  KEntryDict* pDefWriteGroup = data()->aGroupDict[ "<default>" ];
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
  
  QDictIterator<KEntryDict> aWriteIt( data()->aGroupDict );
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
  delete pStream;
  rFile.close();

  return true;
}

void KSimpleConfig::sync()
{
  if( data()->bReadOnly )
	return;

  QFile aFile( data()->aGlobalAppFile );
  writeConfigFile( aFile, false );
}
