#ifndef LOADER_H
#define LOADER_H

#include <qobject.h>
#include <qasciidict.h>
#include <qshared.h>
#include <qcstring.h>
#include <qstringlist.h>

class Factory : public QObject
{
    Q_OBJECT
public:
    Factory( QObject* parent = 0, const char* name = 0 );
    ~Factory();

    virtual QObject* create( QObject* parent = 0, const char* name = 0 ) = 0;
};

class Library : public QShared
{
public:
    Library( const char* libname, void* handel, Factory* factory );
    ~Library();

    const char* name() const;
    Factory* factory();

private:
    QCString m_libname;
    Factory* m_factory;
    void* m_handel;
};

class Loader : public QObject
{
    Q_OBJECT
public:
    Loader( QObject* parent = 0, const char* name = 0 );
    ~Loader();

    /**
     * Loads and initializes a library. Loading a library multiple times is
     * handled gracefully.
     *
     * The name of the library that is going to be loaded is constructed like this
     * <libname>.so.<major>.<minor>.0.
     *
     * @param libname is the name of the library to load, for example "libkspread" or
     *                "libkpresenter".
     * @param major is the major version number of the library "libname".
     * @param minor is the minor version number of the library "libname".
     * @param deps is the list of libraries on which the library "libname" depends.
     *             The loader will load these libraries before loading "libname".
     */
    Factory* factory( const char* libname, int major, int minor, const QStringList& deps );

    static Loader* self();

private:
    QAsciiDict<Library> m_libs;

    static Loader* s_self;
};

#endif
