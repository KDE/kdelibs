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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

#include "ksettings/dispatcher.h"

#include <qsignal.h>

#include <kstaticdeleter.h>
#include <kdebug.h>
#include <kconfig.h>
#include <assert.h>

namespace KSettings
{

//class Dispatcher::DispatcherPrivate
//{
//};

static KStaticDeleter<Dispatcher> ksd_kpd;

Dispatcher * Dispatcher::m_self = 0;

Dispatcher * Dispatcher::self()
{
    kdDebug( 701 ) << k_funcinfo << endl;
    if( m_self == 0 )
        ksd_kpd.setObject( m_self, new Dispatcher() );
    return m_self;
}

Dispatcher::Dispatcher( QObject * parent, const char * name )
    : QObject( parent, name )
    //, d( 0 )
{
    kdDebug( 701 ) << k_funcinfo << endl;
}

Dispatcher::~Dispatcher()
{
    kdDebug( 701 ) << k_funcinfo << endl;
    //delete d;
}

void Dispatcher::registerInstance( KInstance * instance, QObject * recv, const char * slot )
{
    assert( instance != 0 );
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

KConfig * Dispatcher::configForInstanceName( const QCString & instanceName )
{
    kdDebug( 701 ) << k_funcinfo << endl;
    if( m_instanceInfo.contains( instanceName ) )
    {
        KInstance * inst = m_instanceInfo[ instanceName ].instance;
        if( inst )
            return inst->config();
    }
    //if( fallback )
        //return new KSimpleConfig( instanceName );
    return 0;
}

QStrList Dispatcher::instanceNames() const
{
    kdDebug( 701 ) << k_funcinfo << endl;
    QStrList names;
    for( QMap<QCString, InstanceInfo>::ConstIterator it = m_instanceInfo.begin(); it != m_instanceInfo.end(); ++it )
        if( ( *it ).count > 0 )
            names.append( it.key() );
    return names;
}

void Dispatcher::reparseConfiguration( const QCString & instanceName )
{
    kdDebug( 701 ) << k_funcinfo << instanceName << endl;
    // check if the instanceName is valid:
    if( ! m_instanceInfo.contains( instanceName ) )
        return;
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

void Dispatcher::syncConfiguration()
{
    for( QMap<QCString, InstanceInfo>::ConstIterator it = m_instanceInfo.begin(); it != m_instanceInfo.end(); ++it )
    {
        ( *it ).instance->config()->sync();
    }
}

void Dispatcher::unregisterInstance( QObject * obj )
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

//X KInstance * Dispatcher::instanceForName( const QCString & instanceName )
//X {
//X     return m_instanceInfo[ instanceName ].instance;
//X }

} //namespace

#include "dispatcher.moc"

// vim: sw=4 sts=4 et
