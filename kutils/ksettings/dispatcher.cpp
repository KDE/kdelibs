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

void KCDDispatcher::registerInstance( const QCString & instanceName, QObject * recv, const char * slot )
{
	kdDebug( 701 ) << k_funcinfo << instanceName << endl;
	m_instanceName[ recv ] = instanceName;
	QSignal * sig = m_signals.contains( instanceName ) ?
		m_signals[ instanceName ] :
		m_signals[ instanceName ] = new QSignal( this, "signal dispatcher" );
	sig->connect( recv, slot );

	++m_instanceNameCount[ instanceName ];
	connect( recv, SIGNAL( destroyed( QObject * ) ), this, SLOT( unregisterInstance( QObject * ) ) );
}

void KCDDispatcher::registerInstance( const KInstance * instance, QObject * recv, const char * slot )
{
	registerInstance( instance->instanceName(), recv, slot );
}

QStrList KCDDispatcher::instanceNames() const
{
	kdDebug( 701 ) << k_funcinfo << endl;
	QStrList names;
	for( QMap<QCString, int>::const_iterator it = m_instanceNameCount.begin(); it != m_instanceNameCount.end(); ++it )
		if( *it > 0 )
			names.append( it.key() );
	return names;
}

void KCDDispatcher::reparseConfiguration( const QCString & instanceName )
{
	kdDebug( 701 ) << k_funcinfo << instanceName << endl;
	QSignal * sig = m_signals[ instanceName ];
	if( sig )
	{
		kdDebug( 701 ) << "emit signal to instance" << endl;
		sig->activate();
	}
}

void KCDDispatcher::unregisterInstance( QObject * obj )
{
	kdDebug( 701 ) << k_funcinfo << endl;
	QCString name = m_instanceName[ obj ];
	m_instanceName.remove( obj ); //obj will be destroyed when we return, so we better remove this entry
	--m_instanceNameCount[ name ];
	if( m_instanceNameCount[ name ] == 0 )
	{
		delete m_signals[ name ];
		m_signals.remove( name );
	}
}

#include "kcddispatcher.moc"

// vim: sw=4 ts=4 noet
