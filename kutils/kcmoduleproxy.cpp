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

#include "kcmoduleproxy.h"
#include "kcmoduleloader.h"
#include "kcmoduleinfo.h"
#include <qlayout.h>
#include <qapplication.h>
#include <qcursor.h>

class KCModuleProxy::KCModuleProxyPrivate
{
	public:
		KCModuleProxyPrivate( const KCModuleInfo & info )
			: kcm( 0 )
			, modinfo( info )
			, changed( false )
			, failed( false )
		{}

		~KCModuleProxyPrivate()
		{
			delete kcm;
		}

		QStringList args;
		KCModule * kcm;
		KCModuleInfo modinfo;
		bool withfallback;
		bool changed;
		bool failed;
};

KCModuleProxy::KCModuleProxy( const KCModuleInfo & info, bool fallback,
		QWidget * parent, const char * name, const QStringList & args )
	: QWidget( parent, name )
	, d( new KCModuleProxyPrivate( info ) )
{
	d->args = args;
	d->withfallback = fallback;
}

KCModuleProxy::~KCModuleProxy()
{
	KCModuleLoader::unloadModule( d->modinfo );
	delete d;
}

void KCModuleProxy::load()
{
	if( d->kcm )
	{
		d->kcm->load();
		moduleChanged( false );
	}
}

void KCModuleProxy::save()
{
	if( d->kcm )
	{
		d->kcm->save();
		moduleChanged( false );
	}
}

void KCModuleProxy::defaults()
{
	if( d->kcm )
	{
		d->kcm->defaults();
	}
}

QString KCModuleProxy::quickHelp() const
{
	return realModule() ? realModule()->quickHelp() : QString::null;
}

const KAboutData * KCModuleProxy::aboutData() const
{
	return realModule() ? realModule()->aboutData() : 0;
}

int KCModuleProxy::buttons() const
{
	return realModule() ? realModule()->buttons() :
		KCModule::Help | KCModule::Default | KCModule::Apply ;
}

QString KCModuleProxy::rootOnlyMsg() const
{
	return realModule() ? realModule()->rootOnlyMsg() : QString::null;
}

bool KCModuleProxy::useRootOnlyMsg() const
{
	return realModule() ? realModule()->useRootOnlyMsg() : true;
}

KInstance * KCModuleProxy::instance() const
{
	return realModule() ? realModule()->instance() : 0;
}

bool KCModuleProxy::changed() const
{
	return d->changed;
}

KCModule * KCModuleProxy::realModule() const
{
	if( ! d->kcm && ! d->failed )
	{
		QApplication::setOverrideCursor(Qt::WaitCursor);
		KCModuleProxy * that = const_cast<KCModuleProxy*>( this );
		( new QHBoxLayout( that, 0, 0 ) )->setAutoAdd( true );
		d->kcm = KCModuleLoader::loadModule( d->modinfo, d->withfallback,
				that, name(), d->args );
		if( ! d->kcm )
		{
			QApplication::restoreOverrideCursor();
			KCModuleLoader::showLastLoaderError( that );
			d->failed = true;
			return 0;
		}
		connect( d->kcm, SIGNAL( changed( bool ) ),
				this, SLOT( moduleChanged( bool ) ) );
		connect( d->kcm, SIGNAL( destroyed() ),
				this, SLOT( moduleDestroyed() ) );
		QApplication::restoreOverrideCursor();
	}
	return d->kcm;
}

const KCModuleInfo & KCModuleProxy::moduleInfo() const
{
  return d->modinfo;
}

void KCModuleProxy::showEvent( QShowEvent * ev )
{
	( void )realModule();
	QWidget::showEvent( ev );
}

void KCModuleProxy::moduleChanged( bool c )
{
	if( d->changed != c )
	{
		d->changed = c;
		emit changed( c );
	}
}

void KCModuleProxy::moduleDestroyed()
{
	d->kcm = 0;
}

#include "kcmoduleproxy.moc"

// vim: sw=4 ts=4 noet
