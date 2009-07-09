
#ifndef _NEPOMUK_RESOURCENAMEUPPER_H_
#define _NEPOMUK_RESOURCENAMEUPPER_H_

class QDateTime;
class QDate;
class QTime;

namespace NepomukFast {
NEPOMUK_OTHERCLASSES
}

#include NEPOMUK_PARENT_INCLUDE

namespace NepomukFast {

NEPOMUK_RESOURCECOMMENT
    class NEPOMUK_RESOURCENAME : public NEPOMUK_PARENTRESOURCE
    {
    public:
        /**
         * Create a new empty and invalid NEPOMUK_RESOURCENAME instance
         */
        NEPOMUK_RESOURCENAME( const QUrl &uri = QUrl(), const QUrl &graphUri = QUrl() );

NEPOMUK_METHODS

    protected:
        NEPOMUK_RESOURCENAME( const QUrl& uri, const QUrl& graphUri, const QUrl& type );
    };
}

#endif
