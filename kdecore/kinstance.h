#ifndef _KINSTANCE_H
#define _KINSTANCE_H

class KStandardDirs;
class KConfig;
class KIconLoader;
class KCharsets;
class QFont;

#include <qstring.h>


/**
 * Access to KDE global objects for use in shared libraries.
 *
 * @author Torben Weis
 * @version $Id$
 */
class KInstance
{
 public:
  /**
   *  Constructor
   *  @param instanceName the name of the instance
   *  @param aboutData data about this instance (see @ref KAboutData)
   **/
  KInstance( const QCString& instanceName,
	     const KAboutData * aboutData = 0L );
    /**
     * Destructor.
     */
    virtual ~KInstance();
    
    /**
     *  Retrieve the application standard dirs object.
     */
    KStandardDirs	*dirs() const;
    
    /**
     *  Retrieve the general config object.
     */
    KConfig *config() const;
    
    /**
     *  Retrieve an iconloader object.
     */
    KIconLoader	*iconLoader() const;
    
    KCharsets	     *charsets() const;
    
    QCString          instanceName() const;

protected:
    /**
     *  Copy Constructor is not allowed
     */
    KInstance( const KInstance& );
    
public:
    mutable KStandardDirs	*_dirs;
    
    mutable KConfig		*_config;
    mutable KIconLoader	        *_iconLoader;
    
    QCString                     _name;

};

#endif 

