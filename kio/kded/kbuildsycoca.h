#ifndef __kbuildsycoca_h__
#define __kbuildsycoca_h__ 

#include <qobject.h>
#include <qstring.h>
#include <ksycoca.h>
#include <ksycocatype.h>
#include <sys/stat.h>

class QDataStream;
class KSycocaEntry;

// No need for this in libkio - apps only get readonly access
class KBuildSycoca : public KSycoca
{
  Q_OBJECT
public:
   KBuildSycoca();
   virtual ~KBuildSycoca();

   /**
    * process DCOP message.  Only calls to "recreate" are supported at
    * this time.
    */
   bool process(const QCString &fun, const QByteArray &data, 
		QCString &replyType, QByteArray &replyData);

   /**
    * Recreate the database file
    */
   void recreate();

protected:

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
    * @internal
    * @return true if building (i.e. if a KBuildSycoca);
    */
   virtual bool isBuilding() { return true; }
};

#endif
