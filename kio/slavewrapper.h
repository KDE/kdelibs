#ifndef _ioslave_wrapper_h
#define _ioslave_wrapper_h

#include "kio/connection.h"

namespace KIO {
class SlaveBase;

class SlaveWrapper {
public:
    SlaveWrapper(SlaveBase *slave, int socket);
    ~SlaveWrapper();
    void dispatchLoop();

protected:
    void connectSlave(const QString& path);

private:
    Connection parconn, appconn;
    SlaveBase *slave;
};

};
#endif
