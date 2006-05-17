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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

#include "factory.h"
#include "ifaces/videowidget.h"
#include "ifaces/backend.h"
#include "../factory.h"

#include <QFile>

#include <klibloader.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kdebug.h>

namespace Phonon
{
namespace Ui
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
			const char* lib = Phonon::Factory::self()->uiLibrary();
			if( lib == 0 )
			{
				KMessageBox::error( 0, i18n( "The current backend does not support any graphical user interface functionality. Choose a different backend if this is not what you want." ) ); //FIXME: Make this error message more useful: Offer the user a simple way to change the backend
				return;
			}

			QStringList errormsg;
			KLibFactory* factory = 0;
			const char* symbol = Phonon::Factory::self()->uiSymbol();
			if( symbol == 0 )
				factory = KLibLoader::self()->factory( lib );
			else
			{
				KLibrary* library = KLibLoader::self()->library( lib );
				if( library )
				{
					void* sym = library->symbol( symbol );
					if( sym )
					{
						typedef KLibFactory* ( *t_func )();
						t_func func = ( t_func )sym;
						factory = func();
						//connect( factory, SIGNAL( objectCreated( QObject* ) ),
								//library, SLOT( slotObjectCreated( QObject* ) ) );
					}
					else
						kWarning( 602 ) << "given symbol '" << symbol << "' not found" << endl;
				}
			}
			if( factory )
			{
				backend = static_cast<Ui::Ifaces::Backend*>( factory->create( 0, "Phonon::Ui::Ifaces::Backend" ) );
				if( 0 == backend )
				{
					QString e = i18n( "create method returned 0" );
					errormsg.append( e );
					kDebug( 602 ) << "Error getting ui backend from factory for " <<
						lib << ":\n" << e << endl;
				}
				else
					kDebug( 602 ) << "using ui backend: " << lib << endl;
			}
			else
			{
				QString e = KLibLoader::self()->lastErrorMessage();
				errormsg.append( e );
				kDebug( 602 ) << "Error getting factory for " << lib <<
					":\n" << e << endl;
			}
			if( 0 == backend )
				KMessageBox::error( 0, i18n( "Unable to use the UI part of the loaded Multimedia Backend" ) );
		}

		Ui::Ifaces::Backend * backend;
};

Factory* Factory::m_self = 0;

Factory* Factory::self()
{
	if( !m_self )
	{
		m_self = new Factory();
		Phonon::Factory* f = Phonon::Factory::self();
		connect( f, SIGNAL( deleteYourObjects() ), m_self, SIGNAL( deleteYourObjects() ) );
		connect( f, SIGNAL( recreateObjects() ), m_self, SIGNAL( recreateObjects() ) );
		connect( f, SIGNAL( backendChanged() ), m_self, SIGNAL( backendChanged() ) );
		connect( Phonon::Factory::self(), SIGNAL( destroyed( QObject* ) ), m_self, SLOT( deleteNow() ) );
	}
	return m_self;
}

Factory::Factory()
	: d( new Private )
{
}

Factory::~Factory()
{
	kDebug( 602 ) << k_funcinfo << endl;
	emit deleteYourObjects(); //FIXME: this is probably emitted twice: once through Phonon::Factory::~Factory, and the second one from here
	delete d->backend;
	delete d;
}

void Factory::deleteNow()
{
	delete this;
}

Ui::Ifaces::VideoWidget* Factory::createVideoWidget( QWidget* parent )
{
	return d->backend ? registerObject( d->backend->createVideoWidget( parent ) ) : 0;
}

const Ui::Ifaces::Backend* Factory::backend() const
{
	return d->backend;
}

template<class T> inline T* Factory::registerObject( T* o )
{
	Phonon::Factory::self()->registerQObject( o->qobject() );
	return o;
}

}} //namespace Phonon::Ui

#include "factory.moc"

// vim: sw=4 ts=4 noet
