// $Id$
//
// $Log$
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
// kdoctoolbar removed
//
  // we use the global app config file to save the filename 
  data()->aGlobalAppFile = pFile;
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
  parseOneConfigFile( aFile, NULL );
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
bool KSimpleConfig::deleteGroup( const char* pGroup, bool bDeep = true )
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
