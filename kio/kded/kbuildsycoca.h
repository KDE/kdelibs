#ifndef __kbuildsycoca_h__
#define __kbuildsycoca_h__ 

#include <qobject.h>
#include <qstring.h>
#include <ksycoca.h>
#include <ksycocatype.h>
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
   virtual ~KBuildSycoca();
   
public slots:

   /**
    * Recreate the database file
    */
   void recreate();

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

   /**
    * Clear the factories
    */
   void clear();

   /**
    * Saves the offer list 
    */
   void saveOfferList( KSycocaFactory * serviceFactory,
                       KSycocaFactory * servicetypeFactory );

   /**
    * Saves the mimetype patterns index
    * @param servicetypeFactory the name says it all
    * @param entrySize (output)
    * @param otherIndexOffset (output)
    */
   void saveMimeTypePattern( KSycocaFactory * servicetypeFactory, 
                             Q_INT32 & entrySize,
                             Q_INT32 & otherIndexOffset );
   
   /**
    * Scans dir for new files and new subdirectories.
    * Creates entries in factory.
    */
   void readDirectory(const QString& dir, KSycocaFactory * factory );
   
   /**
    * @internal
    * @return true if building (i.e. if a KBuildSycoca);
    */
   virtual bool isBuilding() { return true; }

protected:

   /**
    * Pointer to the dirwatch class which tells us, when some directories
    * changed.
    */
   KDirWatch* m_pDirWatch;
   /**
    * When a desktop file is updated, a timer is started (5 sec)
    * before rebuilding the binary - so that multiple updates result
    * in only one rebuilding.
    */
   QTimer* m_pTimer;
};

#endif
