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

