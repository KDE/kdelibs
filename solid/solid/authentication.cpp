#include "authentication.h"
#include <solid/ifaces/authentication.h>

namespace Solid
{

class Authentication::Private
{
    public:
        Ifaces::AuthenticationValidator * validator;
};

AuthenticationValidator::AuthenticationValidator()
{
    d = new Private;
}

AuthenticationValidator::~AuthenticationValidator()
{
    delete d;
}

bool AuthenticationValidator::validate( const Ifaces::Authentication * authentication )
{
    return d->validate( authentication );
}
