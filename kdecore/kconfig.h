// $Id$
// Imported sources from KDE CVS
// $Log$
//
// Revision 1.1.1.2  1997/12/10 07:08:29  jacek
// Imported sources from KDE CVS
//
// Revision 1.1.1.1  1997/12/09 22:02:45  jacek
// Imported sorces fromkde
//
// Revision 1.3  1997/10/21 20:44:44  kulow
// removed all NULLs and replaced it with 0L or "".
// There are some left in mediatool, but this is not C++
//
// Revision 1.2  1997/10/16 11:14:30  torben
// Kalle: Copyright headers
// kdoctoolbar removed
//
// Revision 1.1  1997/10/04 19:50:58  kalle
* @version $Id$
//

#ifndef _KCONFIG_H
#define _KCONFIG_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

* @version $Id$

/** 
* KDE Configuration entries
*
* This class implements KDE's default-based configuration system.
*
* @author Kalle Dalheimer (kalle@kde.org)
* @version $Id$
* @see KApplication::getConfig KConfigBase KSimpleConfig
* @short KDE Configuration Management  class
*/
class KConfig : public KConfigBase
{
  bool writeConfigFile( QFile& rFile, bool bGlobal = false );

  // copy-construction and assignment are not allowed
  bool writeConfigFile( QFile& rFile, bool bGlobal = FALSE );
  KConfig& operator= ( const KConfig& rConfig );

protected:
  KConfig( const char* pGlobalAppFile = NULL, 
		   const char* pLocalAppFile = NULL );
	*/
  virtual void parseConfigFiles();

  /** Write back the configuration data.
	*/
  bool writeConfigFile( QFile& rFile, bool bGlobal = false );

public:
  /** 
	* Construct a KConfig object. 
	*
	* @param pGlobalAppFile A file to parse in addition to the
	*  normally parsed files.  
	* @param pLocalAppFile Another file to parse in addition to the
	* normally parsed files (has priority over pGlobalAppFile 
	*/
  KConfig( const char* pGlobalAppFile = 0L, 
		   const char* pLocalAppFile = 0L );

  /** 
	* Destructor. 
	*
	* Writes back any dirty configuration entries.
	*/
  virtual ~KConfig();

  /**
	* Write back the config cache.
	*/
  virtual void sync();
};
  


#endif
