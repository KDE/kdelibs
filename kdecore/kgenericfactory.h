#ifndef __kgenericfactory_h__
#define __kgenericfactory_h__

#include <klibloader.h>
#include <ktypelist.h>

/*
 * These global template functions here exist to get a faster and more lightweight
 * implementation of the createObject function in the KGenericFactory template. Well,
 * in fact it applies only to the KGF template that takes a typelist. We could aswell
 * make the Factory multiply inherit from a recursively defined template, like
 * ... : public BlahFactory<T1>, public BlahFactory<T2> , but that creates overhead
 * in terms of more code and data (vtbl) being generated than really necessary. All
 * we want is only one factory method actually, because the caller only knows about
 * that one anyway (through the KLibFactory base class) .
 * That's why we use inlined global functions here, which should result in basically
 * all essential code being generated in that one function, so that we really end up
 * with only one class being generated (with one vtbl for it) .
 * The overloading through the last argument and the static_cast for it is a bit ugly,
 * but I couldn't figure out how to do it better. And oh well, it's an implementation detail
 * the user (developer) shouldn't care about/see anyway. He writes KGenericFactory<MyComponent> 
 * or KGenericFactory< K_TYPELIST_2( MyComponent, MyOtherComponent) > and be happy ;)
 * Any comments appreciated!
 * (Simon)
 */ 

/** @internal */
inline QObject *KGenericFactoryFunction( QObject *, const char *, const char *, KDE::NullType * )
{ return 0; }

/** @internal */
template <class T>
inline QObject *KGenericFactoryFunction( QObject *parent, const char *name, const char *className,
                                         T *)
{
    QMetaObject *metaObject = T::staticMetaObject();
    while ( metaObject )
    {
        if ( !qstrcmp( className, metaObject->className() ) )
            return new T( parent, name );
        metaObject = metaObject->superClass();
    }
    return 0;
}

/** @internal */
template <class T1, class T2>
inline QObject *KGenericFactoryFunction( QObject *parent, const char *name, 
                                         const char *className, KTypeList<T1, T2> * )
{
    QObject *result = KGenericFactoryFunction( parent, name, className, static_cast<T1 *>( 0 ) );
    if ( !result )
        result = KGenericFactoryFunction( parent, name, className, static_cast<T2 *>( 0 ) );
    return result;
}

template <class T>
class KGenericFactory : public KLibFactory
{
public:
    KGenericFactory() {}

protected:
    virtual QObject *createObject( QObject *parent, const char *name,
                                  const char *className, const QStringList & )
    {   
        return KGenericFactoryFunction( parent, name, className, static_cast<T *>( 0 ) );
    }
};

template <class T1, class T2>
class KGenericFactory< KTypeList<T1, T2> > : public KLibFactory
{
public:
    KGenericFactory() {}

protected:
    virtual QObject *createObject( QObject *parent, const char *name,
                                   const char *className, const QStringList & )
    {
        return KGenericFactoryFunction( parent, name, className, static_cast<KTypeList<T1, T2> *>( 0 ) );
    }
};

/*
 * vim: et sw=4
 */

#endif

