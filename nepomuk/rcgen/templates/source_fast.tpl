
#include <soprano/soprano.h>

#include "NEPOMUK_RESOURCENAMELOWER.h"

NEPOMUK_INCLUDES

#include <QtCore/QDateTime>
#include <QtCore/QDate>
#include <QtCore/QTime>

NepomukFast::NEPOMUK_RESOURCENAME::NEPOMUK_RESOURCENAME( const QUrl &uri, const QUrl &graphUri )
  : NEPOMUK_PARENTRESOURCE( uri, graphUri, QUrl::fromEncoded("NEPOMUK_RESOURCETYPEURI", QUrl::StrictMode) )
{
}

NepomukFast::NEPOMUK_RESOURCENAME::NEPOMUK_RESOURCENAME( const QUrl& uri, const QUrl& graphUri, const QUrl& type )
  : NEPOMUK_PARENTRESOURCE( uri, graphUri, type )
{
}

NEPOMUK_METHODS
