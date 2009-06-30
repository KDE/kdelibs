#ifndef BACKENDS_MANAGER_H
#define BACKENDS_MANAGER_H

#include "AuthBackend.h"
#include "HelperProxy.h"

class BackendsManager
{
    static AuthBackend *auth;
    static HelperProxy *helper;
    
    BackendsManager() {}
public:
    static AuthBackend *authBackend();
    static HelperProxy *helperProxy();
    
private:
    static void init();
};

#endif
