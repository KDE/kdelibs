#ifndef RESOURCE_MANAGER_IFACE_H
#define RESOURCE_MANAGER_IFACE_H

#include <dcopobject.h>

class ResourceManagerIface : virtual public DCOPObject
{
  K_DCOP

  k_dcop_signals:
    void signalResourceAdded( QString identifier );
    void signalResourceModified( QString identifier );
    void signalResourceDeleted( QString identifier );

  k_dcop:
    virtual ASYNC dcopResourceAdded( QString identifier ) = 0;
    virtual ASYNC dcopResourceModified( QString identifier ) = 0;
    virtual ASYNC dcopResourceDeleted( QString identifier ) = 0;
//    virtual QRect mySynchronousMethod() = 0;
};

#endif
