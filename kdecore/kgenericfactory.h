/* This file is part of the KDE project
 * Copyright (C) 2001 Simon Hausmann <hausmann@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */
#ifndef __kgenericfactory_h__
#define __kgenericfactory_h__

#include <klibloader.h>
#include <ktypelist.h>
#include <kgenericfactory.tcc>

/* @internal */
template <class T>
class KGenericFactoryBase 
{
public:
    KGenericFactoryBase( const char *instanceName )
        : m_instanceName( instanceName ) 
    {
        s_self = this;
    }
    virtual ~KGenericFactoryBase()
    {
        delete s_instance;
        s_instance = 0;
        s_self = 0;
    }

    static KInstance *instance();

protected:
    virtual KInstance *createInstance()
    {
        if ( !m_instanceName )
            return 0; // ### assert
        return new KInstance( m_instanceName );
    }

private:
    QCString m_instanceName;

    static KInstance *s_instance;
    static KGenericFactoryBase<T> *s_self;
};

/* @internal */
template <class T>
KInstance *KGenericFactoryBase<T>::s_instance = 0;

/* @internal */
template <class T>
KGenericFactoryBase<T> *KGenericFactoryBase<T>::s_self = 0;

/* @internal */
template <class T>
KInstance *KGenericFactoryBase<T>::instance()
{
    if ( !s_instance && s_self )
        s_instance = s_self->createInstance();
    return s_instance;
}

template <class T>
class KGenericFactory : public KLibFactory, public KGenericFactoryBase<T>
{
public:
    KGenericFactory( const char *instanceName = 0 )
        : KGenericFactoryBase<T>( instanceName ) 
    {}

protected:
    virtual QObject *createObject( QObject *parent, const char *name,
                                  const char *className, const QStringList &args )
    {   
        return KDEPrivate::ConcreteFactory<T>::create( 0, 0, parent, name, className, args );
    }
};

template <class T1, class T2>
class KGenericFactory< KTypeList<T1, T2> > : public KLibFactory,
                                             public KGenericFactoryBase< KTypeList<T1, T2> >
{
public:
    KGenericFactory( const char *instanceName  = 0 )
        : KGenericFactoryBase< KTypeList<T1, T2> >( instanceName )
    {}

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

