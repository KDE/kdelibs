
#include <nepomuk/tools.h>
#include <nepomuk/variant.h>
#include <nepomuk/resourcemanager.h>
#include "NEPOMUK_RESOURCENAMELOWER.h"

NEPOMUK_INCLUDES

#include <QtCore/QDateTime>
#include <QtCore/QDate>
#include <QtCore/QTime>


Nepomuk::NEPOMUK_RESOURCENAME::NEPOMUK_RESOURCENAME()
  : NEPOMUK_PARENTRESOURCE( QUrl(), QUrl::fromEncoded("NEPOMUK_RESOURCETYPEURI") )
{
}


Nepomuk::NEPOMUK_RESOURCENAME::NEPOMUK_RESOURCENAME( ResourceManager* manager )
  : NEPOMUK_PARENTRESOURCE( QUrl(), QUrl::fromEncoded("NEPOMUK_RESOURCETYPEURI"), manager )
{
}


Nepomuk::NEPOMUK_RESOURCENAME::NEPOMUK_RESOURCENAME( const NEPOMUK_RESOURCENAME& res )
  : NEPOMUK_PARENTRESOURCE( res )
{
}


Nepomuk::NEPOMUK_RESOURCENAME::NEPOMUK_RESOURCENAME( const Nepomuk::Resource& res )
  : NEPOMUK_PARENTRESOURCE( res )
{
}


Nepomuk::NEPOMUK_RESOURCENAME::NEPOMUK_RESOURCENAME( const QString& uri )
  : NEPOMUK_PARENTRESOURCE( uri, QUrl::fromEncoded("NEPOMUK_RESOURCETYPEURI") )
{
}

Nepomuk::NEPOMUK_RESOURCENAME::NEPOMUK_RESOURCENAME( const QString& uri, ResourceManager* manager )
  : NEPOMUK_PARENTRESOURCE( uri, QUrl::fromEncoded("NEPOMUK_RESOURCETYPEURI"), manager )
{
}

Nepomuk::NEPOMUK_RESOURCENAME::NEPOMUK_RESOURCENAME( const QUrl& uri )
  : NEPOMUK_PARENTRESOURCE( uri, QUrl::fromEncoded("NEPOMUK_RESOURCETYPEURI") )
{
}

Nepomuk::NEPOMUK_RESOURCENAME::NEPOMUK_RESOURCENAME( const QUrl& uri, ResourceManager* manager )
  : NEPOMUK_PARENTRESOURCE( uri, QUrl::fromEncoded("NEPOMUK_RESOURCETYPEURI"), manager )
{
}

Nepomuk::NEPOMUK_RESOURCENAME::NEPOMUK_RESOURCENAME( const QString& uri, const QUrl& type )
  : NEPOMUK_PARENTRESOURCE( uri, type )
{
}

Nepomuk::NEPOMUK_RESOURCENAME::NEPOMUK_RESOURCENAME( const QString& uri, const QUrl& type, ResourceManager* manager )
  : NEPOMUK_PARENTRESOURCE( uri, type, manager )
{
}

Nepomuk::NEPOMUK_RESOURCENAME::NEPOMUK_RESOURCENAME( const QUrl& uri, const QUrl& type )
  : NEPOMUK_PARENTRESOURCE( uri, type )
{
}

Nepomuk::NEPOMUK_RESOURCENAME::NEPOMUK_RESOURCENAME( const QUrl& uri, const QUrl& type, ResourceManager* manager )
  : NEPOMUK_PARENTRESOURCE( uri, type, manager )
{
}

Nepomuk::NEPOMUK_RESOURCENAME::~NEPOMUK_RESOURCENAME()
{
}

Nepomuk::NEPOMUK_RESOURCENAME& Nepomuk::NEPOMUK_RESOURCENAME::operator=( const NEPOMUK_RESOURCENAME& res )
{
    Resource::operator=( res );
    return *this;
}

QString Nepomuk::NEPOMUK_RESOURCENAME::resourceTypeUri()
{
    return QLatin1String("NEPOMUK_RESOURCETYPEURI");
}

NEPOMUK_METHODS
