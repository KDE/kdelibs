/*
 * Copyright (C) 2004 Frans Englich <frans.englich@telia.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License version 2 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include <qdatastream.h>
#include <qmessagebox.h>

#include <dcopclient.h>

#include <kapplication.h>
#include <kcmoduleproxy.h>
#include <kdebug.h>

#include "kcmoduleproxyIfaceImpl.h"

KCModuleProxyIfaceImpl::KCModuleProxyIfaceImpl( const QByteArray& name,
		KCModuleProxy* const client )
	: DCOPObject( name ), QObject( 0 ),
		p( const_cast<KCModuleProxy *>( client ))
{
    setObjectName( name );
	connect( p, SIGNAL( changed(bool)),
			SLOT( changedRelay(bool)));
	connect( p, SIGNAL( quickHelpChanged()),
			SLOT( quickHelpRelay()));
}

void KCModuleProxyIfaceImpl::save()
{
	kDebug(711) << k_funcinfo << endl;
	p->save();
}

void KCModuleProxyIfaceImpl::load()
{
	kDebug(711) << k_funcinfo << endl;
	p->load();
}

void KCModuleProxyIfaceImpl::defaults()
{
	kDebug(711) << k_funcinfo << endl;
	p->defaults();
}

QString KCModuleProxyIfaceImpl::applicationName()
{
	return kapp->caption();
}

QString KCModuleProxyIfaceImpl::quickHelp()
{
	return p->quickHelp();
}

bool KCModuleProxyIfaceImpl::changed()
{
	return p->changed();
}

void KCModuleProxyIfaceImpl::changedRelay( bool c )
{
	QByteArray data;
	QDataStream stream(&data, QIODevice::WriteOnly);
	stream << c;
	emitDCOPSignal( "changed(bool)", data );
}

void KCModuleProxyIfaceImpl::quickHelpRelay()
{
	QByteArray data;
	emitDCOPSignal( "quickHelpChanged()", data );
}

/***************************************************************/




/***************************************************************/
KCModuleProxyRootCommunicatorImpl::KCModuleProxyRootCommunicatorImpl
		( const QByteArray& name, KCModuleProxy* const client )
	: DCOPObject( name ), QObject( 0 ),
		p( const_cast<KCModuleProxy *>( client ))
{
    setObjectName( name );
	/*
	 * Connect kcmshell's KCModuleProxy's change signal
	 * to us, such that we act as a proxy for
	 * KCModuleProxy's API.
	 */

	/* Note, we don't use KCModuleProxy::d->dcopClient */
	KApplication::dcopClient()->connectDCOPSignal( 0, p->dcopName(),
			"changed(bool)", objId(), "changed(bool)", false );

	KApplication::dcopClient()->connectDCOPSignal( 0, p->dcopName(),
			"quickHelpChanged()", objId(), "quickHelpChanged()", false );
}

/* Reimplementations of DCOP members */
void KCModuleProxyRootCommunicatorImpl::changed( bool c )
{
	kDebug(711) << k_funcinfo << endl;
	p->moduleChanged( c );
}

void KCModuleProxyRootCommunicatorImpl::quickHelpChanged()
{
	kDebug(711) << k_funcinfo << endl;
	p->emitQuickHelpChanged();
}

#include "kcmoduleproxyIfaceImpl.moc"
