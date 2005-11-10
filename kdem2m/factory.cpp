/*  This file is part of the KDE project
    Copyright (C) 2004-2005 Matthias Kretz <kretz@kde.org>

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

#include "factory.h"
#include "ifaces/audiopath.h"
#include "ifaces/audioeffect.h"
#include "ifaces/audiooutputbase.h"
#include "ifaces/audiooutput.h"
#include "ifaces/audiodataoutput.h"
#include "ifaces/audiofftoutput.h"
#include "ifaces/videopath.h"
#include "ifaces/videoeffect.h"
#include "ifaces/videooutputbase.h"
#include "ifaces/backend.h"
#include "ifaces/mediaobject.h"
#include "ifaces/soundcardcapture.h"
#include "ifaces/bytestream.h"

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

namespace Kdem2m
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
			KTrader::OfferList offers = KTrader::self()->query( "KDEMultimediaBackend", "Type == 'Service'" );
			KTrader::OfferListIterator it = offers.begin();
			KTrader::OfferListIterator end = offers.end();
			QStringList errormsg;
			for( ; it != end; ++it )
			{
				KService::Ptr ptr = *it;
				KLibFactory * factory = KLibLoader::self()->factory( QFile::encodeName( ptr->library() ) );
				if( factory )
				{
					backend = ( Ifaces::Backend* )factory->create( 0, "Multimedia Backend", "Kdem2m::Ifaces::Backend" );
					if( 0 == backend )
					{
						QString e = i18n( "create method returned 0" );
						errormsg.append( e );
						kdDebug( 600 ) << "Error getting backend from factory for " <<
							ptr->name() << ":\n" << e << endl;
					}
					else
					{
						service = ptr;
						kdDebug( 600 ) << "using backend: " << ptr->name() << endl;
						break;
					}
				}
				else
				{
					QString e = KLibLoader::self()->lastErrorMessage();
					errormsg.append( e );
					kdDebug( 600 ) << "Error getting factory for " << ptr->name() <<
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
};

static ::KStaticDeleter<Factory> sd;

Factory * Factory::m_self = 0;

Factory * Factory::self()
{
	if( ! m_self )
		sd.setObject( m_self, m_self = new Factory() );
	return m_self;
}

Factory::Factory()
	: DCOPObject( "Kdem2mFactory" )
	, d( new Private )
{
	connectDCOPSignal( 0, 0, "kdem2mBackendChanged()", "kdem2mBackendChanged()", false);
}

Factory::~Factory()
{
	kdDebug( 600 ) << k_funcinfo << endl;
	emit deleteYourObjects();
	foreach( QObject* o, d->objects )
	{
		kdDebug( 600 ) << "delete " << o << endl;
		delete o;
	}
	delete d->backend;
	delete d;
}

void Factory::kdem2mBackendChanged()
{
	if( d->backend )
	{
		emit deleteYourObjects();
		if( d->objects.size() > 0 )
		{
			kdWarning( 600 ) << "we were asked to change the backend but the application did\n"
				"not free all references to objects created by the factory. Therefore we can not\n"
				"change the backend without crashing. Now we have to wait for a restart to make\n"
				"backendswitching possible." << endl;
			// in case there were objects deleted give 'em a chance to recreate
			// them now
			emit recreateObjects();
			return;
		}
		delete d->backend;
		d->backend = 0;
	}
	d->createBackend();
	emit recreateObjects();
	emit backendChanged();
}

void Factory::objectDestroyed( QObject * obj )
{
	kdDebug( 600 ) << k_funcinfo << obj << endl;
	d->objects.removeAll( obj );
}

Ifaces::MediaObject* Factory::createMediaObject( QObject* parent )
{
	return d->backend ? registerObject( d->backend->createMediaObject( parent ) ) : 0;
}

Ifaces::SoundcardCapture* Factory::createSoundcardCapture( QObject* parent )
{
	return d->backend ? registerObject( d->backend->createSoundcardCapture( parent ) ) : 0;
}

Ifaces::ByteStream* Factory::createByteStream( QObject* parent )
{
	return d->backend ? registerObject( d->backend->createByteStream( parent ) ) : 0;
}

Ifaces::AudioPath* Factory::createAudioPath( QObject* parent )
{
	return d->backend ? registerObject( d->backend->createAudioPath( parent ) ) : 0;
}

Ifaces::AudioEffect* Factory::createAudioEffect( QObject* parent )
{
	return d->backend ? registerObject( d->backend->createAudioEffect( parent ) ) : 0;
}

Ifaces::AudioOutput* Factory::createAudioOutput( QObject* parent )
{
	return d->backend ? registerObject( d->backend->createAudioOutput( parent ) ) : 0;
}

Ifaces::AudioDataOutput* Factory::createAudioDataOutput( QObject* parent )
{
	return d->backend ? registerObject( d->backend->createAudioDataOutput( parent ) ) : 0;
}

Ifaces::AudioFftOutput* Factory::createAudioFftOutput( QObject* parent )
{
	return d->backend ? registerObject( d->backend->createAudioFftOutput( parent ) ) : 0;
}

Ifaces::VideoPath* Factory::createVideoPath( QObject* parent )
{
	return d->backend ? registerObject( d->backend->createVideoPath( parent ) ) : 0;
}

Ifaces::VideoEffect* Factory::createVideoEffect( QObject* parent )
{
	return d->backend ? registerObject( d->backend->createVideoEffect( parent ) ) : 0;
}

Ifaces::VideoOutputBase* Factory::createVideoOutputBase( QObject* parent )
{
	return d->backend ? registerObject( d->backend->createVideoOutputBase( parent ) ) : 0;
}

const Ifaces::Backend* Factory::backend() const
{
	return d->backend;
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

QString Factory::backendName() const
{
	if( d->service )
		return d->service->name();
	else
		return QString::null;
}

QString Factory::backendComment() const
{
	if( d->service )
		return d->service->comment();
	else
		return QString::null;
}

QString Factory::backendVersion() const
{
	if( d->service )
		return d->service->property( "X-KDE-MMBackendInfo-Version" ).toString();
	else
		return QString::null;
}

QString Factory::backendIcon() const
{
	if( d->service )
		return d->service->icon();
	else
		return QString::null;
}

QString Factory::backendWebsite() const
{
	if( d->service )
		return d->service->property( "X-KDE-MMBackendInfo-Website" ).toString();
	else
		return QString::null;
}
#endif

template<class T> inline T* Factory::registerObject( T* o )
{
	connect( o->qobject(), SIGNAL( destroyed( QObject* ) ), SLOT( objectDestroyed( QObject* ) ) );
	d->objects.append( o->qobject() );
	return o;
}

} //namespace Kdem2m

#include "factory.moc"

// vim: sw=4 ts=4 noet
