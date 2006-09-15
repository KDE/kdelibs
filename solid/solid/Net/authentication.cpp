#include "authentication.h"

namespace Solid
{
namespace Net
{
    class Authentication::Private
    {
        public:
            SecretMap secrets;
    };
    class AuthenticationWEP::Private
    {
        public:
            Ifaces::AuthenticationWEP::WEPMethod method;
            Ifaces::AuthenticationWEP::WEPType type;
    };
    class AuthenticationWPA::Private
    {
        public:
            Ifaces::AuthenticationWPA::WPAProtocol protocol;
            Ifaces::AuthenticationWPA::WPAVersion version;
    };
    class AuthenticationWPAEnterprise::Private
    {
        public:
            QString identity;
            QString anonIdentity;
            QString certClient;
            QString certCA;
            QString certPrivate;
            Ifaces::AuthenticationWPAEnterprise::EAPMethod method;
            QString idPasswordKey;
            QString certPrivatePasswordKey;
    };
}
}

Solid::Net::Authentication::Authentication()
{
    d = new Private();
}

Solid::Net::Authentication::~Authentication()
{
    delete d;
}

void Solid::Net::Authentication::setSecrets( const SecretMap& secrets )
{
    d->secrets = secrets;
}

SecretMap Solid::Net::Authentication::secrets() const
{
    return d->secrets;
}

bool Solid::Net::AuthenticationNone::isValid( const QString &essid )
{
    return true;
}

void Solid::Net::AuthenticationNone::setDefaults()
{
    return;
}

Solid::Net::AuthenticationWEP::AuthenticationWEP( Ifaces::AuthenticationWEP::WEPMethod method )
{
    d = new Private;
    d->method = method;
}

Solid::Net::AuthenticationWEP::~AuthenticationWEP()
{
    delete d;
}

void Solid::Net::AuthenticationWEP::setMethod( Ifaces::AuthenticationWEP::WEPMethod method )
{
    d->method = method;
}

Solid::Net::Ifaces::AuthenticationWEP::WEPMethod Solid::Net::AuthenticationWEP::method()
{
    return d->method;
}

void Solid::Net::AuthenticationWEP::setType( Ifaces::AuthenticationWEP::WEPType type )
{
    d->type = type;
}

Solid::Net::Ifaces::AuthenticationWEP::WEPType Solid::Net::AuthenticationWEP::type()
{
    return d->type;
}

Solid::Net::AuthenticationWPA::AuthenticationWPA()
{
    d = new Private;
}

Solid::Net::AuthenticationWPA::~AuthenticationWPA()
{
    delete d;
}

void Solid::Net::AuthenticationWPA::setProtocol( Ifaces::AuthenticationWPA::WPAProtocol protocol )
{
    d->protocol = protocol;
}

Solid::Net::Ifaces::AuthenticationWPA::WPAProtocol Solid::Net::AuthenticationWPA::protocol()
{
    return d->protocol;
}

void Solid::Net::AuthenticationWPA::setVersion( Ifaces::AuthenticationWPA::WPAVersion version )
{
    d->version = version;
}

Solid::Net::Ifaces::AuthenticationWPA::WPAVersion Solid::Net::AuthenticationWPA::version()
{
    return d->version;
}

Solid::Net::AuthenticationWPAEnterprise::AuthenticationWPAEnterprise()
{
    d = new Private;
}

Solid::Net::AuthenticationWPAEnterprise::~AuthenticationWPAEnterprise()
{
    delete d;
}

void Solid::Net::AuthenticationWPAEnterprise::setIdentity( const QString & identity )
{
    d->identity = identity;
}

QString Solid::Net::AuthenticationWPAEnterprise::identify() const
{
    return d->identity;
}

void Solid::Net::AuthenticationWPAEnterprise::setAnonIdentity( const QString & anonIdentity)
{
    d->anonIdentity = anonIdentity;
}

QString Solid::Net::AuthenticationWPAEnterprise::anonIdentity() const
{
    return d->anonIdentity;
}

void Solid::Net::AuthenticationWPAEnterprise::setCertClient( const QString & certClient )
{
    d->certClient = certClient;
}

QString Solid::Net::AuthenticationWPAEnterprise::certClient() const
{
    return d->certClient;
}

void Solid::Net::AuthenticationWPAEnterprise::setCertCA( const QString & certCA)
{
    d->certCA = certCA;
}

QString Solid::Net::AuthenticationWPAEnterprise::certCA() const
{
    return d->certCA;
}

void Solid::Net::AuthenticationWPAEnterprise::setCertPrivate( const QString & certPrivate)
{
    d->certPrivate = certPrivate;
}

QString Solid::Net::AuthenticationWPAEnterprise::certPrivate() const
{
    return d->certPrivate;
}


void Solid::Net::AuthenticationWPAEnterprise::setMethod( Ifaces::AuthenticationWPAEnterprise::EAPMethod method )
{
    d->method = method;
}

Solid::Net::Ifaces::AuthenticationWPAEnterprise::EAPMethod Solid::Net::AuthenticationWPAEnterprise::method() const
{
    return d->method;
}


QString Solid::Net::AuthenticationWPAEnterprise::idPasswordKey() const
{
    return d->idPasswordKey;
}

QString Solid::Net::AuthenticationWPAEnterprise::certPrivatePasswordKey() const
{
    return d->certPrivatePasswordKey;
}


