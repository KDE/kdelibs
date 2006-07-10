/* -*- C++ -*-

   This file declares the ResourceRestrictionPolicy class.

   $ Author: Mirko Boehm $
   $ Copyright: (C) 2004, 2005 Mirko Boehm $
   $ Contact: mirko@kde.org
         http://www.kde.org
         http://www.hackerbuero.org $
   $ License: LGPL with the following explicit clarification:
         This code may be linked against any version of the Qt toolkit
         from Troll Tech, Norway. $

   $Id: Job.h 32 2005-08-17 08:38:01Z mirko $
*/

#ifndef RESOURCE_RESTRICTION_POLICY_H
#define RESOURCE_RESTRICTION_POLICY_H

#include "QueuePolicy.h"

namespace ThreadWeaver {

  class ResourceRestrictionPolicy : public QueuePolicy
    {
    public:
      explicit ResourceRestrictionPolicy ( int cap = 0);
      ~ResourceRestrictionPolicy();

      void setCap (int newcap);
      bool canRun( Job* );
      void free (Job*);
      void release (Job*);
      void destructed (Job*);

    private:
      class Private;
      Private* d;
    };

}

#endif // RESOURCE_RESTRICTION_POLICY_H
