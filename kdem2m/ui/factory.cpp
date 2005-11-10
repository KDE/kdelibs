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
#include <kdem2m/ifaces/ui/videowidget.h>
#include <kdem2m/ifaces/ui/backend.h>
#include <kdem2m/factory.h>

#include <QFile>

#include <kservice.h>
#include <klibloader.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kdebug.h>

namespace Kdem2m
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
			KService::Ptr ptr = Kdem2m::Factory::self()->uiService();
			if( !ptr )
			{
				KMessageBox::error( 0, i18n( "Unable to find the UI part of the Multimedia Backend" ) );
				return;
			}

			QStringList errormsg;
			KLibFactory* factory = KLibLoader::self()->factory( QFile::encodeName( ptr->library() ) );
			if( factory )
			{
				backend = static_cast<Ifaces::Ui::Backend*>( factory->create( 0, "Multimedia Ui Backend", "Kdem2m::Ifaces::Ui::Backend" ) );
				if( 0 == backend )
				{
					QString e = i18n( "create method returned 0" );
					errormsg.append( e );
					kdDebug( 600 ) << "Error getting ui backend from factory for " <<
						ptr->name() << ":\n" << e << endl;
				}
				else
				{
					service = ptr;
					kdDebug( 600 ) << "using ui backend: " << ptr->name() << endl;
				}
			}
			else
			{
				QString e = KLibLoader::self()->lastErrorMessage();
				errormsg.append( e );
				kdDebug( 600 ) << "Error getting factory for " << ptr->name() <<
					":\n" << e << endl;
			}
			if( 0 == backend )
				KMessageBox::error( 0, i18n( "Unable to use the UI part of the loaded Multimedia Backend" ) );
		}

		Ifaces::Ui::Backend * backend;
		KService::Ptr service;
};

Factory* Factory::m_self = 0;

Factory* Factory::self()
{
	if( !m_self )
	{
		m_self = new Factory();
		Kdem2m::Factory* f = Kdem2m::Factory::self();
		connect( f, SIGNAL( deleteYourObjects() ), m_self, SIGNAL( deleteYourObjects() ) );
		connect( f, SIGNAL( recreateObjects() ), m_self, SIGNAL( recreateObjects() ) );
		connect( f, SIGNAL( backendChanged() ), m_self, SIGNAL( backendChanged() ) );
		connect( Kdem2m::Factory::self(), SIGNAL( destroyed( QObject* ) ), m_self, SLOT( deleteNow() ) );
	}
	return m_self;
}

Factory::Factory()
	: d( new Private )
{
}

Factory::~Factory()
{
	kdDebug( 600 ) << k_funcinfo << endl;
	emit deleteYourObjects(); //FIXME: this is probably emitted twice: once through Kdem2m::Factory::~Factory, and the second one from here
	delete d->backend;
	delete d;
}

void Factory::deleteNow()
{
	delete this;
}

Ifaces::Ui::VideoWidget* Factory::createVideoWidget( QWidget* parent )
{
	return d->backend ? registerObject( d->backend->createVideoWidget( parent ) ) : 0;
}

const Ifaces::Ui::Backend* Factory::backend() const
{
	return d->backend;
}

template<class T> inline T* Factory::registerObject( T* o )
{
	Kdem2m::Factory::self()->registerQObject( o->qobject() );
	return o;
}

}} //namespace Kdem2m::Ui

#include "factory.moc"

// vim: sw=4 ts=4 noet
