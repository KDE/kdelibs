#ifndef _KINSTANCE_H
#define _KINSTANCE_H

class KStandardDirs;
class KConfig;
class KLocale;
class KIconLoader;
class KCharsets;
class QFont;

#include <qstring.h>


/**
 * Accessors to KDE global objects for use in shared libraries.
 *
 * @author Torben Weis
 * @version $Id$
 */
class KInstance
{
 public:
    
    KInstance( const QString& libname );
    /**
     * Destructor
     */
    virtual ~KInstance();
    
    /**
     *  Retrieves the application standard dirs object.
     */
    KStandardDirs	*dirs() const;
    
    /**
     *  Retrieves the general config object.
     */
    KConfig *config() const;
    
    /**
     *  Retrieves an iconloader object.
     */
    KIconLoader	*iconLoader() const;
    
    KLocale          *locale() const;
    KCharsets	     *charsets() const;
    
    QFont            generalFont() const;
    QFont            fixedFont() const;
    
    int              dndEventDelay() const;
    
    QString          instanceName() const;

protected:
    /**
     *  Copy Constructor is not allowed
     */
    KInstance( const KInstance& );
    
public:
    mutable KStandardDirs	*_dirs;
    
    mutable KConfig		*_config;
    mutable KIconLoader	        *_iconLoader;
    
    QString                     _name;
};

#endif 

