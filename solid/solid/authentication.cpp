#include "soliddefs_p.h"
#include "authentication.h"
#include <solid/networkmanager.h>
#include <solid/ifaces/networkmanager.h>
#include <solid/ifaces/authentication.h>

namespace Solid
{
    class AuthenticationValidator::Private
    {
    public:
        QObject *backendObject;
    };
}

Solid::AuthenticationValidator::AuthenticationValidator()
    : d( new Private )
{
    Ifaces::NetworkManager *backend = qobject_cast<Ifaces::NetworkManager*>( NetworkManager::self().managerBackend() );

    if ( backend )
    {
        d->backendObject = backend->createAuthenticationValidator();
    }
}

Solid::AuthenticationValidator::~AuthenticationValidator()
{
    delete d->backendObject;
    delete d;
}

bool Solid::AuthenticationValidator::validate( const Ifaces::Authentication *authentication )
{
    return_SOLID_CALL( Ifaces::AuthenticationValidator*, d->backendObject, false, validate(authentication) );
}

