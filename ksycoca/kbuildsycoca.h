#ifndef __kbuildsycoca_h__
#define __kbuildsycoca_h__ 

#include <qobject.h>
#include <qstring.h>
#include "ksycoca.h"
#include "ksycocatype.h"
#include <sys/stat.h>

class QDataStream;
class KDirWatch;
class KSycocaEntry;

// No need for this in libkio - apps only get readonly access
class KBuildSycoca : public KSycoca
{
  Q_OBJECT
public:
   KBuildSycoca();
   ~KBuildSycoca();
   
   /**
    * Add a factory for building, triggers directory parsing
    * (unlike KSycoca::registerFactory which is for read-only factories)
    */
   void addFactory( KSycocaFactory *);

   /**
    * Build the whole system cache, from .desktop files
    */
   void build();
   
   /**
    * Save the ksycoca file
    */
   void save();

protected:

  /**
   * Scans dir for new files and new subdirectories.
   * Creates entries in factory.
   */
  void readDirectory(const QString& dir, KSycocaFactory * factory );

  virtual bool _isBuilding() { return true; }

protected slots:

  /**
   * @internal Triggers rebuilding
   */
  void dirDeleted(const QString& path);
 
  /**
   * @internal Triggers rebuilding
   */
  void update (const QString& dir );

protected:

  /**
   * Pointer to the dirwatch class which tells us, when some directories
   * changed.
   */
  KDirWatch* m_pDirWatch;
};

#endif
