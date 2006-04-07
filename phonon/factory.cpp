/*  This file is part of the KDE project
    Copyright (C) 2004-2006 Matthias Kretz <kretz@kde.org>

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

#include "factory.h"
#include "ifaces/audiopath.h"
#include "ifaces/audioeffect.h"
#include "ifaces/volumefadereffect.h"
#include "ifaces/audiooutput.h"
#include "ifaces/audiodataoutput.h"
#include "ifaces/videopath.h"
#include "ifaces/videoeffect.h"
#include "ifaces/backend.h"
#include "ifaces/mediaobject.h"
#include "ifaces/avcapture.h"
#include "ifaces/bytestream.h"
#include "base_p.h"

#include <ktrader.h>
#include <kservice.h>
#include <klibloader.h>
#include <kmessagebox.h>
#include <QFile>
#include <QList>
#include <klocale.h>
#include <kmimetype.h>
#include <kdebug.h>
#include <kstaticdeleter.h>

static KStaticDeleter<Phonon::Factory> sd;

#define PHONON_LOAD_BACKEND_GLOBAL 1

namespace Phonon
{
class Factory::Private
{
	public:
		Private()
			: backend( 0 )
		{
			createBackend();
		}

		void createBackend()
		{
			KTrader::OfferList offers = KTrader::self()->query( "PhononBackend",
					"Type == 'Service' and [X-KDE-PhononBackendInfo-InterfaceVersion] == 1" );
			KTrader::OfferListIterator it = offers.begin();
			KTrader::OfferListIterator end = offers.end();
			QStringList errormsg;
			for( ; it != end; ++it )
			{
				KService::Ptr ptr = *it;
#ifdef PHONON_LOAD_BACKEND_GLOBAL
				// This code is in here temporarily until NMM gets fixed.
				// Currently the NMM backend will fail with undefined symbols if
				// the backend is not loaded with global symbol resolution
				KLibrary* library = KLibLoader::self()->globalLibrary( QFile::encodeName( ptr->library() ) );
				KLibFactory * factory = library->factory();
#else
				KLibFactory * factory = KLibLoader::self()->factory( QFile::encodeName( ptr->library() ) );
#endif
				if( factory )
				{
					backend = ( Ifaces::Backend* )factory->create( 0, "Multimedia Backend", "Phonon::Ifaces::Backend" );
					if( 0 == backend )
					{
						QString e = i18n( "create method returned 0" );
						errormsg.append( e );
						kDebug( 600 ) << "Error getting backend from factory for " <<
							ptr->name() << ":\n" << e << endl;
					}
					else
					{
						service = ptr;
						kDebug( 600 ) << "using backend: " << ptr->name() << endl;
						break;
					}
				}
				else
				{
					QString e = KLibLoader::self()->lastErrorMessage();
					errormsg.append( e );
					kDebug( 600 ) << "Error getting factory for " << ptr->name() <<
						":\n" << e << endl;
				}
			}
			if( 0 == backend )
			{
				if( offers.size() == 0 )
					KMessageBox::error( 0, i18n( "Unable to find a Multimedia Backend" ) );
				else
				{
					QString details = "<qt><table>";
					QStringList::Iterator eit = errormsg.begin();
					QStringList::Iterator eend = errormsg.end();
					KTrader::OfferListIterator oit = offers.begin();
					KTrader::OfferListIterator oend = offers.end();
					for( ; eit != eend || oit != oend; ++eit, ++oit )
						details += QString( "<tr><td><b>%1</b></td><td>%2</td></tr>" )
							.arg( ( *oit )->name() ).arg( *eit );
					details += "</table></qt>";

					KMessageBox::detailedError( 0,
							i18n( "Unable to use any of the available Multimedia Backends" ), details );
				}
			}
		}

		Ifaces::Backend * backend;
		KService::Ptr service;

		QList<QObject*> objects;
		QList<BasePrivate*> basePrivateList;
};

Factory * Factory::m_self = 0;

Factory * Factory::self()
{
	if( ! m_self )
	{
		m_self = new Factory();
		::sd.setObject( m_self, m_self );
	}
	return m_self;
}

Factory::Factory()
	: DCOPObject( "PhononFactory" )
	, d( new Private )
{
	connectDCOPSignal( 0, 0, "phononBackendChanged()", "phononBackendChanged()", false);
}

Factory::~Factory()
{
	//kDebug( 600 ) << k_funcinfo << endl;
	emit deleteYourObjects();
	foreach( BasePrivate* bp, d->basePrivateList )
		bp->deleteIface();
	foreach( QObject* o, d->objects )
	{
		//kDebug( 600 ) << "delete " << o << endl;
		delete o;
	}
	delete d->backend;
	delete d;
}

void Factory::registerFrontendObject( BasePrivate* bp )
{
	d->basePrivateList.append( bp );
}

void Factory::deregisterFrontendObject( BasePrivate* bp )
{
	d->basePrivateList.removeAll( bp );
}

void Factory::phononBackendChanged()
{
	if( d->backend )
	{
		emit deleteYourObjects();
		foreach( BasePrivate* bp, d->basePrivateList )
			bp->deleteIface();
		if( d->objects.size() > 0 )
		{
			kWarning( 600 ) << "we were asked to change the backend but the application did\n"
				"not free all references to objects created by the factory. Therefore we can not\n"
				"change the backend without crashing. Now we have to wait for a restart to make\n"
				"backendswitching possible." << endl;
			// in case there were objects deleted give 'em a chance to recreate
			// them now
			emit recreateObjects();
			foreach( BasePrivate* bp, d->basePrivateList )
				bp->createIface();
			return;
		}
		delete d->backend;
		d->backend = 0;
	}
	d->createBackend();
	emit recreateObjects();
	foreach( BasePrivate* bp, d->basePrivateList )
		bp->createIface();
	emit backendChanged();
}

//X void Factory::freeSoundcardDevices()
//X {
//X 	if( d->backend )
//X 	{
//X 		d->backend->freeSoundcardDevices();
//X 	}
//X }

void Factory::objectDestroyed( QObject * obj )
{
	//kDebug( 600 ) << k_funcinfo << obj << endl;
	d->objects.removeAll( obj );
}

#define FACTORY_IMPL( classname ) \
Ifaces::classname* Factory::create ## classname( QObject* parent ) \
{ \
	return d->backend ? registerObject( d->backend->create ## classname( parent ) ) : 0; \
}
#define FACTORY_IMPL_1ARG( type1, classname ) \
Ifaces::classname* Factory::create ## classname( type1 name1, QObject* parent ) \
{ \
	return d->backend ? registerObject( d->backend->create ## classname( name1, parent ) ) : 0; \
}

FACTORY_IMPL( MediaObject )
FACTORY_IMPL( AvCapture )
FACTORY_IMPL( ByteStream )
FACTORY_IMPL( AudioPath )
FACTORY_IMPL_1ARG( int, AudioEffect )
FACTORY_IMPL( VolumeFaderEffect )
FACTORY_IMPL( AudioOutput )
FACTORY_IMPL( AudioDataOutput )
FACTORY_IMPL( VideoPath )
FACTORY_IMPL_1ARG( int, VideoEffect )

#undef FACTORY_IMPL

const Ifaces::Backend* Factory::backend() const
{
	return d->backend;
}

const char* Factory::uiLibrary() const
{
	if( !d->backend )
		return 0;
	return d->backend->uiLibrary();
}

const char* Factory::uiSymbol() const
{
	if( !d->backend )
		return 0;
	return d->backend->uiSymbol();
}

#if 0
bool Factory::isMimeTypePlayable( const QString & type ) const
{
	if( d->backend )
	{
		KMimeType::Ptr mimetype = KMimeType::mimeType( type );
		QStringList mimetypes = playableMimeTypes();
		for( QStringList::ConstIterator i=mimetypes.begin(); i!=mimetypes.end(); i++ )
			if( mimetype->is( *i ) )
				return true;
	}
	return false;
}
#endif

QString Factory::backendName() const
{
	if( d->service )
		return d->service->name();
	else
		return QString();
}

QString Factory::backendComment() const
{
	if( d->service )
		return d->service->comment();
	else
		return QString();
}

QString Factory::backendVersion() const
{
	if( d->service )
		return d->service->property( "X-KDE-PhononBackendInfo-Version" ).toString();
	else
		return QString();
}

QString Factory::backendIcon() const
{
	if( d->service )
		return d->service->icon();
	else
		return QString();
}

QString Factory::backendWebsite() const
{
	if( d->service )
		return d->service->property( "X-KDE-PhononBackendInfo-Website" ).toString();
	else
		return QString();
}

template<class T> inline T* Factory::registerObject( T* o )
{
	registerQObject( o->qobject() );
	return o;
}

void Factory::registerQObject( QObject* o )
{
	connect( o, SIGNAL( destroyed( QObject* ) ), SLOT( objectDestroyed( QObject* ) ) );
	d->objects.append( o );
}

} //namespace Phonon

#include "factory.moc"

// vim: sw=4 ts=4 noet
