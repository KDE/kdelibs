/*  This file is part of the KDE project
    Copyright (C) 2003 Matthias Kretz <kretz@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

*/

#include "kcddispatcher.h"

#include <qstrlist.h>
#include <qcstring.h>
#include <qsignal.h>

#include <kstaticdeleter.h>
#include <kdebug.h>
#include <kinstance.h>
#include <kconfig.h>

//class KCDDispatcher::KCDDispatcherPrivate
//{
//};

static KStaticDeleter<KCDDispatcher> ksd_kpd;

KCDDispatcher * KCDDispatcher::m_self = 0;

KCDDispatcher * KCDDispatcher::self()
{
    kdDebug( 701 ) << k_funcinfo << endl;
    if( m_self == 0 )
        ksd_kpd.setObject( m_self, new KCDDispatcher() );
    return m_self;
}

KCDDispatcher::KCDDispatcher( QObject * parent, const char * name )
    : QObject( parent, name )
    //, d( 0 )
{
    kdDebug( 701 ) << k_funcinfo << endl;
}

KCDDispatcher::~KCDDispatcher()
{
    kdDebug( 701 ) << k_funcinfo << endl;
    //delete d;
}

void KCDDispatcher::registerInstance( KInstance * instance, QObject * recv, const char * slot )
{
    // keep the KInstance around and call
    // instance->config()->reparseConfiguration when the app should reparse
    QCString instanceName = instance->instanceName();
    kdDebug( 701 ) << k_funcinfo << instanceName << endl;
    m_instanceName[ recv ] = instanceName;
    QSignal * sig;
    if( m_instanceInfo.contains( instanceName ) )
    {
        sig = m_instanceInfo[ instanceName ].signal;
    }
    else
    {
        sig = new QSignal( this, "signal dispatcher" );
        m_instanceInfo[ instanceName ].signal = sig;
        m_instanceInfo[ instanceName ].instance = instance;
    }
    sig->connect( recv, slot );

    ++m_instanceInfo[ instanceName ].count;
    connect( recv, SIGNAL( destroyed( QObject * ) ), this, SLOT( unregisterInstance( QObject * ) ) );
}

QStrList KCDDispatcher::instanceNames() const
{
    kdDebug( 701 ) << k_funcinfo << endl;
    QStrList names;
    for( QMap<QCString, InstanceInfo>::ConstIterator it = m_instanceInfo.begin(); it != m_instanceInfo.end(); ++it )
        if( ( *it ).count > 0 )
            names.append( it.key() );
    return names;
}

void KCDDispatcher::reparseConfiguration( const QCString & instanceName )
{
    kdDebug( 701 ) << k_funcinfo << instanceName << endl;
    // first we reparse the config of the instance so that the KConfig object
    // will be up to date
    m_instanceInfo[ instanceName ].instance->config()->reparseConfiguration();
    QSignal * sig = m_instanceInfo[ instanceName ].signal;
    if( sig )
    {
        kdDebug( 701 ) << "emit signal to instance" << endl;
        sig->activate();
    }
}

void KCDDispatcher::syncConfiguration()
{
    for( QMap<QCString, InstanceInfo>::ConstIterator it = m_instanceInfo.begin(); it != m_instanceInfo.end(); ++it )
    {
        ( *it ).instance->config()->sync();
    }
}

void KCDDispatcher::unregisterInstance( QObject * obj )
{
    kdDebug( 701 ) << k_funcinfo << endl;
    QCString name = m_instanceName[ obj ];
    m_instanceName.remove( obj ); //obj will be destroyed when we return, so we better remove this entry
    --m_instanceInfo[ name ].count;
    if( m_instanceInfo[ name ].count == 0 )
    {
        delete m_instanceInfo[ name ].signal;
        m_instanceInfo.remove( name );
    }
}

//X KInstance * KCDDispatcher::instanceForName( const QCString & instanceName )
//X {
//X     return m_instanceInfo[ instanceName ].instance;
//X }

#include "kcddispatcher.moc"

// vim: sw=4 sts=4 et
