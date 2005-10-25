/* This code is placed in the public domain */
/*               Waldo Bastian - 2001/04/01 */

#ifndef _TEST_H_
#define _TEST_H_

#include "kdedmodule.h"

class TestModule : public KDEDModule
{
   Q_OBJECT
   K_DCOP
public:
   TestModule(const DCOPCString &obj);

   void idle();

k_dcop:
   QString world();
   void registerMe(const DCOPCString &app);
};

#endif
