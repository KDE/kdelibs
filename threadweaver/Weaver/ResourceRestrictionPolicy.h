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
