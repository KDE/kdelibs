#ifndef KABC_EVOLUTION_DB_WRAPPER
#define KABC_EVOLUTION_DB_WRAPPER

#include <db.h>

#include <qstring.h>
#include <qpair.h>

namespace Evolution {

    class DBWrapper;
    class DBIterator {
        friend class DBWrapper;
    public:
        DBIterator( DBWrapper* = 0l );
        ~DBIterator();

        DBIterator( const DBIterator& );
        DBIterator &operator=( const DBIterator& );

        QString key()const;
        QString value()const;

        QString operator*();

        DBIterator &operator++();
        DBIterator &operator--();

        bool operator==( const DBIterator& ) const;
        bool operator!=( const DBIterator& ) const;
    private:
        struct Data;
        Data* data;
    };
    class DBWrapper {
    public:
        DBWrapper();
        ~DBWrapper();

        QString lastError()const;

        bool open( const QString& file, bool readOnly = false);
        bool save();
        DBIterator begin();
        DBIterator end();

        bool find( const QString& key, QString& value );
        bool add( const QString& key,  const QString& val );
        bool remove( const QString& key );
    private:
        //  DBT element( const QString& );
        struct Data;
        Data* data;

    };

}


#endif
