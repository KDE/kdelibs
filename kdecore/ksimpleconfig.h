// $Id$
// Revision 1.1.1.2  1997/12/10 07:08:33  jacek
// $Log$
// Imported sources from KDE CVS
//
// Revision 1.1.1.1  1997/12/09 22:02:46  jacek
// Imported sorces fromkde
//
// Revision 1.3  1997/10/16 11:15:02  torben
// Kalle: Copyright headers
// kdoctoolbar removed
//
// Revision 1.2  1997/10/08 19:28:53  kalle
// KSimpleConfig implemented
//
// Revision 1.1  1997/10/04 19:51:07  kalle
// new KConfig
//

#ifndef _KSIMPLECONFIG_H
* @version $Id$

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <kconfigbase.h>
* @version $Id$
/** 
* KDE Configuration entries
*
* This is a trivial implementation of KConfigBase for applications
* that need only one configuration file and no default system.
*
* @author Kalle Dalheimer (kalle@kde.org)
* @version $Id$
* @see KApplication::getConfig KConfigBase KConfig
* @short KDE Configuration Management class
*/
class KSimpleConfig : public KConfigBase
{
  Q_OBJECT 

  // copy-construction and assignment are not allowed
	* Construct a KConfig object. 
  KSimpleConfig& operator= ( const KSimpleConfig& rConfig );
	* @param A file to parse in addition to the normally parsed files.
protected:
  /** Open all appropriate configuration files and pass them on to
	* parseOneConfigFile()
  KSimpleConfig( const char* pFile );

  /**
	* Construct a KSimpleConfig object and make it either read-write
	* or read-only.  
	*
	*
	* Writes back any dirty configuration entries.
	*/
	* Returns true if the object is read-only
	*
	*/
  bool isReadOnly() const { return data()->bReadOnly; }
   * Write back the cache.
   *
  const char* deleteEntry( const char* pKey );
  virtual void sync();

  /** Write back the configuration data.
	*/
  bool writeConfigFile( QFile& rFile, bool bGlobal = false );

  /**
	* Delete a configuration entry.
	*
	* @param pKey The key of the entry to delete
	* @param bLocalized Whether the localized or the non-localized key should
	*                    be deleted
	* @return The old value of that key.
	*/
  const QString deleteEntry( const char* pKey, bool bLocalized );

  /**
	* Delete a configuration entry group
	*
	* If the group is not empty and bDeep is false, nothing gets
	* deleted and false is returned.
	* If this group is the current group and it is deleted, the
	* current group is undefined and should be set with setGroup()
	* before the next operation on the configuration object.
	*
	* @param pGroup The name of the group
	* @param bDeep Whether non-empty groups should be completely
	* deleted (including their entries)
	* @return If the group does not exist or is not empty and bDeep is
	* false, deleteGroup returns false.
	*/
  bool deleteGroup( const char* pGroup, bool bDeep = true );
};
  

 
#endif
