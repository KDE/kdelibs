/* This file is part of the KDE libraries
    Copyright (C) 2001,2002 Ellis Whitehead <ellis@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "kglobalaccel.h"
#ifdef Q_WS_X11
#include "kglobalaccel_x11.h"
#elif defined(Q_WS_WIN)
#include "kglobalaccel_win.h"
#elif defined(Q_WS_MACX)
#include "kglobalaccel_mac.h"
#else
#include "kglobalaccel_emb.h"
#endif

#include <qstring.h>
#include "kaccelbase.h"
#include <kdebug.h>
#include <kshortcut.h>
#include <klocale.h>

//----------------------------------------------------

KGlobalAccel::KGlobalAccel( QObject* pParent )
: QObject( pParent )
{
	kdDebug(125) << "KGlobalAccel(): this = " << this << endl;
	d = new KGlobalAccelPrivate();
}

KGlobalAccel::~KGlobalAccel()
{
	kdDebug(125) << "~KGlobalAccel(): this = " << this << endl;
	delete d;
}

/*
void KGlobalAccel::clear()
	{ d->clearActions(); }
*/
KAccelActions& KGlobalAccel::actions()
	{ return ( ( KAccelBase* ) d )->actions(); }

const KAccelActions& KGlobalAccel::actions() const
	{ return ( ( KAccelBase* ) d )->actions(); }

bool KGlobalAccel::isEnabled()
	{ return ((KAccelBase*)d)->isEnabled(); }

void KGlobalAccel::setEnabled( bool bEnabled )
	{ d->setEnabled( bEnabled ); }

void KGlobalAccel::blockShortcuts( bool block )
        { KGlobalAccelPrivate::blockShortcuts( block ); }

void KGlobalAccel::disableBlocking( bool disable )
        { d->disableBlocking( disable ); }

KAccelAction* KGlobalAccel::insert( const QString& sAction, const QString& sDesc, const QString& sHelp,
		const KShortcut& cutDef3, const KShortcut& cutDef4,
		const QObject* pObjSlot, const char* psMethodSlot,
		bool bConfigurable, bool bEnabled )
{
	return d->insert( sAction, sDesc, sHelp,
		cutDef3, cutDef4,
		pObjSlot, psMethodSlot,
		bConfigurable, bEnabled );
}

KAccelAction* KGlobalAccel::insert( const QString& sName, const QString& sDesc )
	{ return d->insert( sName, sDesc ); }
bool KGlobalAccel::updateConnections()
	{ return d->updateConnections(); }

bool KGlobalAccel::remove( const QString& sAction )
        { return d->remove( sAction ); }

const KShortcut& KGlobalAccel::shortcut( const QString& sAction ) const
{
	const KAccelAction* pAction = ( ( KAccelBase* ) d )->actions().actionPtr( sAction );
	return (pAction) ? pAction->shortcut() : KShortcut::null();
}

bool KGlobalAccel::setShortcut( const QString& sAction, const KShortcut& cut )
	{ return d->setShortcut( sAction, cut ); }
bool KGlobalAccel::setSlot( const QString& sAction, const QObject* pObjSlot, const char* psMethodSlot )
	{ return d->setActionSlot( sAction, pObjSlot, psMethodSlot ); }
QString KGlobalAccel::label( const QString& sAction ) const
{
	const KAccelAction* pAction = ( ( KAccelBase* ) d )->actions().actionPtr( sAction );
	return (pAction) ? pAction->label() : QString();
}
bool KGlobalAccel::setActionEnabled( const QString& sAction, bool bEnable )
{
        return d->setActionEnabled( sAction, bEnable );
}

const QString& KGlobalAccel::configGroup() const
	{ return d->configGroup(); }
// for kdemultimedia/kmix
void KGlobalAccel::setConfigGroup( const QString& s )
	{ d->setConfigGroup( s ); }

bool KGlobalAccel::readSettings( KConfigBase* pConfig )
	{ d->readSettings( pConfig ); return true; }
bool KGlobalAccel::writeSettings( KConfigBase* pConfig ) const
	{ d->writeSettings( pConfig ); return true; }
bool KGlobalAccel::writeSettings( KConfigBase* pConfig, bool bGlobal ) const
{
	d->setConfigGlobal( bGlobal );
	d->writeSettings( pConfig );
	return true;
}

void KGlobalAccel::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

#include "kglobalaccel.moc"
