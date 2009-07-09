
#ifndef _NEPOMUK_RESOURCENAMEUPPER_H_
#define _NEPOMUK_RESOURCENAMEUPPER_H_

class QDateTime;
class QDate;
class QTime;

namespace Nepomuk {
NEPOMUK_OTHERCLASSES
}

#include NEPOMUK_PARENT_INCLUDE
NEPOMUK_VISIBILITY_HEADER_INCLUDE

namespace Nepomuk {

NEPOMUK_RESOURCECOMMENT
    class NEPOMUK_VISIBILITY NEPOMUK_RESOURCENAME : public NEPOMUK_PARENTRESOURCE
    {
    public:
        /**
         * Create a new empty and invalid NEPOMUK_RESOURCENAME instance
         */
        NEPOMUK_RESOURCENAME();
        NEPOMUK_RESOURCENAME( ResourceManager* );

        /**
         * Default copy constructor
         */
        NEPOMUK_RESOURCENAME( const NEPOMUK_RESOURCENAME& );
        NEPOMUK_RESOURCENAME( const Resource& );

        /**
         * Create a new NEPOMUK_RESOURCENAME instance representing the resource
         * referenced by \a uriOrIdentifier.
         */
        NEPOMUK_RESOURCENAME( const QString& uriOrIdentifier );
        NEPOMUK_RESOURCENAME( const QString& uriOrIdentifier, ResourceManager* );

        /**
         * Create a new NEPOMUK_RESOURCENAME instance representing the resource
         * referenced by \a uri.
         */
        NEPOMUK_RESOURCENAME( const QUrl& uri );
        NEPOMUK_RESOURCENAME( const QUrl& uri, ResourceManager* );
        ~NEPOMUK_RESOURCENAME();

        NEPOMUK_RESOURCENAME& operator=( const NEPOMUK_RESOURCENAME& );

NEPOMUK_METHODS

        /**
         * \return The URI of the resource type that is used in NEPOMUK_RESOURCENAME instances.
         */
        static QString resourceTypeUri();

    protected:
       NEPOMUK_RESOURCENAME( const QString& uri, const QUrl& type );
       NEPOMUK_RESOURCENAME( const QUrl& uri, const QUrl& type );
       NEPOMUK_RESOURCENAME( const QString& uri, const QUrl& type, ResourceManager* );
       NEPOMUK_RESOURCENAME( const QUrl& uri, const QUrl& type, ResourceManager* );
    };
}

#endif
