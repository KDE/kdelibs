#ifndef __kbuildsycoca_h__
#define __kbuildsycoca_h__ 

#include "ksycoca.h"
#include "ksycocatype.h"

class QDataStream;

// No need for this in libkio - apps only get readonly access
class KBuildSycoca : public KSycoca
{
public:
   KBuildSycoca();
   ~KBuildSycoca();
   
   // Add a factory for building, triggers directory parsing
   // (unlike KSycoca::registerFactory which is for read-only factories)
   void addFactory( KSycocaFactory *);

   void save();
};

#endif
