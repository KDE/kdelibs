#ifndef __kgenericfactory_h__
#define __kgenericfactory_h__

#include <klibloader.h>
#include <ktypelist.h>
#include <kgenericfactory.tcc>

template <class T>
class KGenericFactory : public KLibFactory
{
public:
    KGenericFactory() {}

protected:
    virtual QObject *createObject( QObject *parent, const char *name,
                                  const char *className, const QStringList &args )
    {   
        return KDEPrivate::ConcreteFactory<T>::create( 0, 0, parent, name, className, args );
    }
};

template <class T1, class T2>
class KGenericFactory< KTypeList<T1, T2> > : public KLibFactory
{
public:
    KGenericFactory() {}

protected:
    virtual QObject *createObject( QObject *parent, const char *name,
                                   const char *className, const QStringList &args )
    {
        return KDEPrivate::MultiFactory< KTypeList< T1, T2 > >::create( 0, 0, parent, name,
                                                                        className, args );
    }
};

/*
 * vim: et sw=4
 */

#endif

