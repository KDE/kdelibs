#ifndef _KINSTANCE_H
#define _KINSTANCE_H

class KStandardDirs;
class KConfig;
class KIconLoader;
class KAboutData;
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

    /**
     *  Constructor
     *  @param instanceName the name of the instance
     *  @param aboutData data about this instance (see @ref KAboutData)
     */
    KInstance( const QCString& instanceName,
               const KAboutData * aboutData = 0L );

    /**
     *  Destructor
     */
    virtual ~KInstance();

    /**
     *  @return the application standard dirs object.
     */
    KStandardDirs *dirs() const;

    /**
     *  @return the general config object.
     */
    KConfig *config() const;

    /**
     *  @return an iconloader object.
     */
    KIconLoader	*iconLoader() const;

    KCharsets *charsets() const;

    /**
     *  @return the about data of this instance
     *  Warning, can be 0L
     */
    KAboutData *aboutData() const;

    /**
     * @return the instance name
     */
    QCString instanceName() const;

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
    const KAboutData            *_aboutData;
};

#endif

