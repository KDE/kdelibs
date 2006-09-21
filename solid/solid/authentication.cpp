#include "authentication.h"
#include <solid/networkmanager.h>
#include <solid/ifaces/networkmanager.h>
#include <solid/ifaces/authentication.h>

namespace Solid
{

class AuthenticationValidator::Private
{
    public:
        Ifaces::AuthenticationValidator *iface;
};

}

Solid::AuthenticationValidator::AuthenticationValidator()
    : d( new Private )
{
    d->iface = NetworkManager::self().backend()->createAuthenticationValidator();
}

Solid::AuthenticationValidator::~AuthenticationValidator()
{
    delete d->iface;
    delete d;
}

bool Solid::AuthenticationValidator::validate( const Ifaces::Authentication * authentication )
{
    return d->iface->validate( authentication );
}

