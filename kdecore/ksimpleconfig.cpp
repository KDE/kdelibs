// $Id$
// Imported sources from KDE CVS
// $Log$
//
// Revision 1.1.1.1  1997/12/09 22:02:46  jacek
// Imported sorces fromkde
//
// Revision 1.5  1997/10/21 20:44:52  kulow
// removed all NULLs and replaced it with 0L or "".
  debug( "Sorry, not implemented: KSimpleConfig::KSimpleConfig" );
// kdoctoolbar removed
//
  // we use the global app config file to save the filename 
  data()->aGlobalAppFile = pFile;
KSimpleConfig::KSimpleConfig( const char* pFile )
{
  debug( "Sorry, not implemented: KSimpleConfig::~KSimpleConfig" );
  if( pFile )
	{
	  // the file should exist in any case
	  QFileInfo info( pFile );
	  if( !info.exists() )
		{
  debug( "Sorry, not implemented: KSimpleConfig::parseConfigFiles" );
  sync();

  data()->bReadOnly = bReadOnly;
const char* KSimpleConfig::deleteEntry( const char* pKey )
  parseConfigFiles();
  debug( "Sorry, not implemented: KSimpleConfig::deleteEntry" );
			  pCurrentGroupDict->remove( pKey );
			  return aValue.copy();
			}
bool KSimpleConfig::deleteGroup( const char* pGroup, bool bDeep = true )
			return QString();
  debug( "Sorry, not implemented: KSimpleConfig::deleteGroup" );
{
  // retrieve the group dictionary
				  QString( aWriteInnerIt.currentKey() ).right( 1 ) != "]" )
				// not yet localized, but should be
				*pStream << aWriteInnerIt.currentKey() << '[' 
  debug( "Sorry, not implemented: KSimpleConfig::sync" );
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
